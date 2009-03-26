/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "words.h"

#include <pkimmatcher.h>
#include <qtopianamespace.h>
#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <QLineEdit>
#include <QLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextDocument>
#include <QKeyEvent>
#include <QPainter>
#include <QMenu>
#include <QSet>
#include "../language/langname.h"

class WordListItem : public QListWidgetItem
{
public:
    enum Roles {
        LanguageRole = Qt::UserRole + 1,
        LanguageFontRole,
        MaxLengthRole,
    };

    WordListItem(const QString &text, QListWidget * parent, int len, const QString& language, const QFont* f) :
        QListWidgetItem(text, parent)
    {
        setData(MaxLengthRole, len);
        setData(LanguageRole, language);
        if (f) {
            QFont fnt(*f);
            setData(LanguageFontRole, QVariant(fnt));
        }
    }

    virtual ~WordListItem() {}
};

WordListDelegate::WordListDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void WordListDelegate::paint(QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
{
    if (!index.isValid()) { //QItemDelegate can handle this
        QItemDelegate::paint(painter, option, index);
        return;
    }
    static QRect emptyRect;

    QVariant word = index.model()->data(index, Qt::DisplayRole);
    QVariant lang = index.model()->data(index, WordListItem::LanguageRole);
    QVariant font = index.model()->data(index, WordListItem::LanguageFontRole);
    int maxLength = index.model()->data(index, WordListItem::MaxLengthRole).toInt();

    QString label;
    QFont f;
    if (word.isValid()) {
        f = painter->font();
        f.setBold(true);
        QFontMetrics fm(f);
        label = word.toString();
        label = "<b>" + label.left(maxLength) + "</b>"
                + label.mid(maxLength);
    } else { //in case sth went wrong
        QItemDelegate::paint(painter, option, index);
        return;
    }

    QString langString = lang.toString();
    if (!langString.isNull())
        label += QString(" ("+langString+")");

    // Layout text
    QRect textRect;
    doLayout(option, &emptyRect, &emptyRect, &textRect, false);

    // draw the item
    QPen pen = painter->pen();
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(textRect, option.palette.color(cg, QPalette::Highlight));
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    if (option.state & QStyle::State_Editing) {
        painter->save();
        painter->setPen(option.palette.color(cg, QPalette::Text));
        painter->drawRect(textRect.adjusted(0, 0, -1, -1));
        painter->restore();
    }

    QTextDocument doc;
    doc.setHtml(label);
    painter->translate(textRect.topLeft());
    doc.drawContents(painter);
    painter->translate(-textRect.topLeft());
}

QSize WordListDelegate::sizeHint(const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    static QRect emptyRect;
    if (!index.isValid())
        return QSize();

    QVariant value = index.model()->data(index, Qt::FontRole);
    QFont fnt = value.isValid() ? qvariant_cast<QFont>(value) : option.font;

    QString text = index.model()->data(index, Qt::DisplayRole).toString();

    value = index.model()->data(index, WordListItem::LanguageRole);
    QString lang = value.isValid() ? value.toString() : QString();

    int maxLength = index.model()->data(index, WordListItem::MaxLengthRole).toInt();

    // bold part
    fnt.setBold(true);
    QSize result = QFontMetrics(fnt).size(0, text.left(maxLength));

    // normal part
    fnt.setBold(false);
    QSize workSize = QFontMetrics(fnt).size(0, text.right(text.length() - maxLength));
    result = QSize(result.width() + workSize.width(), qMax(workSize.height(), result.height()));

    // italics part
    fnt.setItalic(true);
    if (!lang.isNull())
    {
        QSize workSize = QFontMetrics(fnt).size(0, " ("+lang+")");
        result = QSize(result.width() + workSize.width(), qMax(workSize.height(), result.height()));
    }

    result += QSize(2,2);

    return result;
}

Words::Words( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setWindowTitle(tr("Words"));

    QVBoxLayout *centralLayout = new QVBoxLayout(this);

    line = new QLineEdit(this);
    centralLayout->addWidget(line);

    box = new QListWidget(this);
    box->setFrameStyle(QFrame::NoFrame);
    centralLayout->addWidget(box);
    box->setFocusPolicy(Qt::NoFocus);
    QPalette pal = box->palette();
    pal.setBrush(QPalette::Background, pal.base());
    box->setPalette(pal);

    WordListDelegate *delegate = new WordListDelegate(box);
    box->setItemDelegate(delegate);

    a_add = new QAction( QIcon(":icon/add"), tr("Add"), this );
    connect( a_add, SIGNAL(triggered()), this, SLOT(addWord()) );
    a_del = new QAction( QIcon(":icon/trash"), QString(), this );
    connect( a_del, SIGNAL(triggered()), this, SLOT(deleteWord()) );
    a_prefer = new QAction( QIcon(":icon/prefer"), tr("Prefer"), this );
    connect( a_prefer, SIGNAL(triggered()), this, SLOT(preferWord()) );

    QActionGroup *mode = new QActionGroup(this);
    mode->setExclusive(true);

    a_alllocal = new QAction( QIcon(":icon/addedwords"),
            tr("All Added"), mode );
    connect( a_alllocal, SIGNAL(triggered()), this, SLOT(showAddedDict()) );
    a_alllocal->setCheckable(true);
    a_allpref = new QAction( QIcon(":icon/preferredwords"),
            tr("All Preferred"), mode );
    connect( a_allpref, SIGNAL(triggered()), this, SLOT(showPreferredDict()) );
    a_allpref->setCheckable(true);
    a_alldel = new QAction( QIcon(":icon/deletedwords"),
            tr("All Deleted"), mode );
    connect( a_alldel, SIGNAL(triggered()), this, SLOT(showDeletedDict()) );
    a_alldel->setCheckable(true);

    a_pkim = new QAction( tr("Number Match", "enter a number and app looks up matching T9 words"), mode );
    a_word = new QAction( QIcon(":icon/find"), tr("Word Lookup/Add"), mode );
    a_pkim->setCheckable(true);
    a_word->setCheckable(true);
    connect( mode, SIGNAL(triggered(QAction*)), this, SLOT(modeChanged()) );

    tooltip = new QLabel(this);
    tooltip->setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
    tooltip->setAlignment(Qt::AlignCenter);
    tooltip->setBackgroundRole(QPalette::Button);
    //tooltip->setAutoResize(true);
    tooltip->setWordWrap(true);
    tooltip->setMaximumWidth(150);
    tooltip->setMargin(6);
    tooltip->hide();

    QMenu *menu = QSoftMenuBar::menuFor(this);

    // XXX Need to turn off cut/paste menu for line,
    // XXX so the above is used. For now, duplicated it...
    QMenu *linemenu = QSoftMenuBar::menuFor(line);
    for (int i=0; i<=1; i++) {
        // XXX see linemenu above
        QMenu *m = i ? menu : linemenu;
        m->addAction(a_add);
        m->addAction(a_del);
        m->addAction(a_prefer);
        m->addSeparator();
        if ( !Qtopia::mousePreferred() )
            m->addAction(a_pkim);
        m->addAction(a_word);
        QMenu *subMenu = m->addMenu( tr("Show only...", "the context is \"Show only All Added/All Preferred/All Deleted\"") );
        subMenu->addAction(a_alllocal);
        if ( !Qtopia::mousePreferred() )
            subMenu->addAction(a_allpref);
        subMenu->addAction(a_alldel);
    }
    if ( Qtopia::mousePreferred() )
        a_word->setChecked(true);
    else
        a_pkim->setChecked(true);

    line->installEventFilter(this);

    connect(line,SIGNAL(textChanged(QString)), this, SLOT(lookup()));
    connect(box,SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(updateActions()));

    modeChanged();
    updateActions();
}

Words::~Words()
{
}

bool Words::eventFilter(QObject* watched, QEvent* e)
{
    Q_UNUSED(watched);
    Q_UNUSED(e);
    if(e->type() == QEvent::InputMethod)
    {
        QInputMethodEvent* ime =  (QInputMethodEvent*)e;

        if(ime->commitString().isEmpty())
            lookup (line->text().insert(line->cursorPosition(),ime->preeditString()));
    }
    return false;
};

void Words::updateActions()
{
    bool addEnabled =
            a_word->isChecked()
            && line->text().length() > 0
            && (!box->item(0) || box->item(0)->text() != line->text())
         || a_alldel->isChecked()
            && box->currentItem()!=0;
    a_add->setEnabled( addEnabled );
    a_add->setVisible( addEnabled );

    bool delEnabled = !a_alldel->isChecked() && !dummy && box->currentItem()!=0;
    a_del->setEnabled( delEnabled );
    a_del->setVisible( delEnabled );
    a_del->setText(
            a_allpref->isChecked() ? tr("Unprefer")
            : a_alldel->isChecked() ? tr("Undelete")
            : tr("Delete Word"));

    bool preferEnabled =
            a_pkim->isChecked()
            && box->currentItem()!=0
            && box->row(box->currentItem())>=1 //ie. not already preferred
            && box->currentItem()->text().length() == line->text().length() // not a prefix
            && box->count()>1
            && box->item(1)->text().length() == line->text().length(); // ie. more than one choice
    a_prefer->setEnabled( preferEnabled );
    a_prefer->setVisible( preferEnabled );
}

void Words::search()
{
    search(line->text());
}

void Words::search(const QString& pattern)
{
    // Find out what language is being used for the UI.
    QString uiLang;
    {
        QStringList langList = Qtopia::languageList();
        if (langList.size() > 0)
            uiLang = langList.at(0);
    }
    QString uiLangName = languageName(uiLang,0,0);

    QStringList langs = InputMatcher::chosenLanguages();
    QSet<QString> seen;
    // If the pattern is four or less characters, only search for exact matches,
    // otherwise search for both exact matches and predicted matches.
    for (int prefix=0; prefix<=(pattern.length()>4?1:0); prefix++) {
        // For each input language, search for matching words
        foreach(QString lang, langs) {
            InputMatcher matcher("dict-"+lang);
            if (matcher.isValid()) {
                bool ok=true;
                if (a_word->isChecked())
                    matcher.matchSet(pattern);
                else
                    ok=matcher.match(pattern);
                if (ok) {
                    QStringList matches = matcher.choices(false, prefix);

                    QFont f = font();
                    // Show the names of languages that are not the current UI language
                    QString langName;
                    if (lang != uiLang) {
                        langName = languageName(lang, &f, 0);
                        // Where the current language and search language have the same base, show only
                        // the variant of the language, e.g. if the current language is "English (U.S.)"
                        // then show "British" instead of "English (British)".
                        int paren = langName.indexOf('(');
                        if (paren >= 0 && langName.left(paren) == uiLangName.left(paren))
                            langName = langName.mid(paren+1, langName.length()-paren-2);
                    }
                    QFont *langFont = f==font() ? 0 : &f;

                    // Add matches to the list
                    foreach(QString word, matches) {
                        if ( !prefix || word.length() != pattern.length() ) {
                            if ( !seen.contains(word) ) {
                                seen.insert(word);
                                new WordListItem(word, box, pattern.length(), langName, langFont);
                            }
                        }
                    }
                }
            }
        }
    }
}

void Words::lookup()
{
    lookup(line->text());
}

void Words::lookup(const QString &in)
{
    QString pattern = in;
    for (int i=0; i<(int)pattern.length(); ++i) {
        if ( pattern[i] <= ' ' )
            pattern.remove(i,1);
    }

    int cur = 0;
    dummy = false;
    if (a_word->isChecked()) {
        /* user has type a specific word, and wants to know if it
           is in the dictinary */
        box->clear();
        if ( pattern.length() > 0 ) {
            search(pattern);
            tooltip->hide();
        }
        if ( pattern.length() <= 3 && box->count() == 0 ) {
            tooltip->setText(tr("Type letters to match."));
            dummy = true;
        }
    } else if ( a_pkim->isChecked() ) {
        /* user has done what would have been dict lookup,
           find out what the word was */
        box->clear();
        if ( pattern.length() > 0 ) {
            search(pattern);
            tooltip->hide();
        }
        if (box->count() == 0) {
            QString msg;
            if ( pattern.length() <= 1) {
                msg = tr("Type numbers to match.");
            } else if (pattern.length() < 4) {
                msg = tr("No exact match.");
            } else {
                msg = tr("No matches.");
            }
            tooltip->setText(msg);
            dummy = true;
        }
    } else {
        /* the user has selected a list (like added words) and wants
           to jump to a point in it */
        /* note the lack of call to search? its because no list is made */
        cur = -1;
        for (int i=0; i<(int)box->count(); i++) {
            QString t = box->item(i)->text();
            // is this dictionary depended?
            // no, just set dependent.
            InputMatcher matcher("ext");
            if ( matcher.match(t,0,pattern,pattern.length()) == (int)pattern.length() ) {
                cur = i;
                break;
            }
        }
        if ( box->count() > 0 && cur == -1 ) {
            tooltip->setText(tr("No match."));
            dummy = true;
        }
    }
    if ( !dummy && box->count() == 0 ) {
        tooltip->setText(tr("No matching words."));
        dummy = true;
    }
    if (dummy) {
        tooltip->resize(tooltip->maximumWidth(), tooltip->heightForWidth(tooltip->maximumWidth()));
        tooltip->raise();
        tooltip->show();
    }
    if ( !dummy && box->count() && cur>= 0) {
        QListWidgetItem *curItem = box->item(cur);
        box->setCurrentItem(curItem);
        if ( curItem ) {
            QModelIndex mIndex = box->currentIndex();
            box->selectionModel()->select(mIndex, QItemSelectionModel::ClearAndSelect);
        } else
            box->clearSelection();
    }
    updateActions();
}

void Words::modeChanged()
{
    lookup();
    if ( a_word->isChecked()) {
        QtopiaApplication::setInputMethodHint(line, "text noautocapitalization");
    } else {
        QtopiaApplication::setInputMethodHint(line,QtopiaApplication::Number);
    }
    line->clear(); // Old text not useful.
}

QSize Words::sizeHint() const
{
    return QSize(-1,400);
}

void Words::resizeEvent(QResizeEvent *resizeEvent)
{
    QWidget::resizeEvent(resizeEvent);
    tooltip->move((width()-tooltip->width())/2, (height()-tooltip->height())/3*2);
}

void Words::addWord()
{
    addWord(line->text());
}

void Words::addWord(const QString& word)
{
    QListWidgetItem* curItem = box->currentItem();
    if ( a_alldel->isChecked() && curItem ) {
        Qtopia::addWords(QStringList(box->currentItem()->text())); // add it back
        showDeletedDict();
    } else {
        Qtopia::addWords(QStringList(word));
    }
    lookup(word);
}

void Words::deleteWord()
{
    QListWidgetItem *item = box->currentItem();
    QString w = item->text();
    int i = box->row(item);
    box->takeItem(i);
    delete item;
    Qtopia::removeWords("preferred",QStringList(w)); // No tr
    if ( !a_allpref->isChecked() )
        Qtopia::removeWords(QStringList(w));

    if (!i && box->count() == i) {
        return;
    }
    if ( i == box->count() && i)
        i--;
    box->setCurrentItem(box->item(i));
    QModelIndex mIndex = box->currentIndex();
    box->selectionModel()->select(mIndex, QItemSelectionModel::ClearAndSelect);
}

void Words::preferWord()
{
    QStringList nopref;
    for (int i=0; i<(int)box->count(); i++) {
        if ( box->item(i)!=box->currentItem() )
            nopref.append(box->item(i)->text());
    }
    Qtopia::removeWords("preferred",nopref);
    Qtopia::addWords("preferred",QStringList(box->currentItem()->text()));
    lookup();
}

void Words::showDict(const char* dname)
{
    QString name = dname;
    box->clear();
    dummy = false;
    tooltip->hide();
    box->insertItems(-1, Qtopia::dawg(name).allWords());
    if (box->count()) {
        box->setCurrentItem(box->item(0));
        QModelIndex mIndex = box->currentIndex();
        box->selectionModel()->select(mIndex, QItemSelectionModel::ClearAndSelect);
    } else {
        if (name=="local")
            tooltip->setText(tr("You have added no words."));
        else if (name=="preferred")
            tooltip->setText(tr("You have made no preferences."));
        else if (name=="deleted")
            tooltip->setText(tr("You have deleted no standard words."));
        else
            tooltip->setText(tr("No matching words."));
        tooltip->resize(tooltip->maximumWidth(), tooltip->heightForWidth(tooltip->maximumWidth()));
        tooltip->raise();
        tooltip->show();
        dummy = true;
    }
    updateActions();
}

void Words::showAddedDict()
{
    showDict("local"); // No tr
}

void Words::showPreferredDict()
{
    showDict("preferred"); // No tr
}

void Words::showDeletedDict()
{
    showDict("deleted"); // No tr
}

void Words::keyPressEvent(QKeyEvent* e)
{
    if ( e->type()==QEvent::KeyPress ) {
        int ch=0;
        switch ( e->key() ) {
            case Qt::Key_Up:
                ch = -1;
                break;
            case Qt::Key_Down:
                ch = 1;
                break;
        }
        if ( box->currentItem() != 0 && ch ) {
            int i = (box->row(box->currentItem())+ch+box->count())%box->count();
            box->setCurrentItem(box->item(i));
            QModelIndex mIndex = box->currentIndex();
            box->selectionModel()->select(mIndex, QItemSelectionModel::ClearAndSelect);
            e->accept();
            return;
        }
    }
    QDialog::keyPressEvent(e);
}

