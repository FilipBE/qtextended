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

#include "pensettingswidget.h"
#include "charsetedit.h"
#include "uniselect.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtopiaapplication.h>

QIMPenInputCharDlg::QIMPenInputCharDlg( QWidget *parent, const char *name,
        bool modal, bool isFS, Qt::WFlags f )
    : QDialog( parent, f )
{
    setWindowTitle( tr("New character") );
    setObjectName( name );
    setModal( modal );
    uni = 0;

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin(4);
    vb->setSpacing(4);

    QHBoxLayout *hb = new QHBoxLayout();
    hb->setMargin(4);
    hb->setSpacing(4);

    QLabel *label = new QLabel( tr("Character"), this );
    hb->addWidget( label );

    QLineEdit *currentChar = new QLineEdit(this);
    currentChar->setAlignment( Qt::AlignHCenter );
    currentChar->setReadOnly(true);
    hb->addWidget( currentChar );
    QtopiaApplication::setInputMethodHint(currentChar, QtopiaApplication::AlwaysOff);

    vb->addItem(hb);

    u = new UniSelect(this);
    vb->addWidget(u);

    connect(u, SIGNAL(selected(QString)),
            currentChar, SLOT(setText(QString)));
    connect(u, SIGNAL(selected(uint)),
            this, SLOT(setCharacter(uint)));

    u->setFocus();
    addSpecial( isFS );
    setCharacter(u->character());
    currentChar->setText(u->text());
}

// own map...
const int comboSel[] = {
    Qt::Key_Escape,
    Qt::Key_Tab,
    Qt::Key_Backspace,
    Qt::Key_Return,
    QIMPenChar::Caps,
    Qt::Key_unknown
};

const int popupComboSel[] = {
    QIMPenChar::CapsLock, // popup only
    QIMPenChar::Punctuation,
    QIMPenChar::Symbol, // popup only
    Qt::Key_unknown
};

const int fsComboSel[] = {
    //Qt::Key_Left,
    //Qt::Key_Right,
    //Qt::Key_Up,
    //Qt::Key_Down,
    QIMPenChar::Punctuation,
    QIMPenChar::NextWord, // fs only
    QIMPenChar::WordPopup, // fs only
    QIMPenChar::SymbolPopup, // fs only
    QIMPenChar::ModePopup, // fs only
    Qt::Key_unknown
};


void QIMPenInputCharDlg::addSpecial( bool isFS )
{
    int i = 0;
    while ( comboSel[i] != Qt::Key_unknown ) {
        QIMPenChar c;
        c.setKey(comboSel[i]);
        u->addSpecial(c.key(), c.name());
        i++;
    }
    const int *extraSel;
    if (isFS)
        extraSel = fsComboSel;
    else
        extraSel = popupComboSel;
    i = 0;
    while ( extraSel[i] != Qt::Key_unknown ) {
        QIMPenChar c;
        c.setKey(extraSel[i]);
        u->addSpecial(c.key(), c.name());
        i++;
    }
}

void QIMPenInputCharDlg::setCharacter( uint sp )
{
    uni = sp;
}

CharSetDlg::CharSetDlg( QWidget *parent, const char *name,
            bool modal, Qt::WFlags f ): QDialog(parent, f )
{
    setObjectName( name );
    setModal( modal );
    setWindowTitle(tr("Handwriting"));
    QVBoxLayout *vl = new QVBoxLayout(this);

    edit = new CharSetEdit(this);

    vl->addWidget(edit);
}

CharSetDlg::~CharSetDlg() {}

void CharSetDlg::accept()
{
    edit->checkStoreMatch();
    QDialog::accept();
}

void CharSetDlg::reject()
{
    edit->clearMatch();
    QDialog::reject();
}

void CharSetDlg::setCharSet( QIMPenCharSet *c )
{
    edit->setCharSet(c);
}

QIMPenCharSet *CharSetDlg::charSet() const
{
    return edit->charSet();
}


CharSetEdit::CharSetEdit( QWidget *parent, const char *name )
    : QFrame( parent ), currentSet(0),
            lastCs(1), lastCh(0), addFlag(false), mIsFS(false)
{
    setupUi( this );
    setObjectName( name );
    init();
}

CharSetEdit::CharSetEdit( QIMPenCharSet *c, QWidget *parent,
                const char *name )
    : QFrame( parent ), currentSet(0),
            lastCs(1), lastCh(0), addFlag(false)
{
    setupUi( this );
    setObjectName( name );
    init();
    setCharSet(c);
}

void CharSetEdit::init()
{
    pw->setReadOnly(true);
    currentChar = 0;
    matchCount = 0;
    matchIndex = 0;
    inputChar = new QIMPenChar();
    
    newCharBtn->setIcon(QIcon(":icon/new_character"));
    delCharBtn->setIcon(QIcon(":icon/trash"));
    resetCharBtn->setIcon(QIcon(":icon/systemowned"));
    prevBtn->setIcon(QIcon(":icon/up"));
    nextBtn->setIcon(QIcon(":icon/down"));
    addBtn->setIcon(QIcon(":icon/new_stroke"));
    removeBtn->setIcon(QIcon(":icon/trash"));

    connect( newCharBtn, SIGNAL(clicked()), this, SLOT(addChar())) ;
    connect( delCharBtn, SIGNAL(clicked()), this, SLOT(removeChar())) ;
    connect( resetCharBtn, SIGNAL(clicked()), this, SLOT(resetMatches())) ;

    connect( charList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            SLOT(selectItemCode(QListWidgetItem*,QListWidgetItem*)) );

    pw->setFixedHeight( 75 );
    connect( pw, SIGNAL(stroke(QIMPenStroke*)),
                 SLOT(newStroke(QIMPenStroke*)) );

    connect( prevBtn, SIGNAL(clicked()), this, SLOT(prevMatch()));
    connect( nextBtn, SIGNAL(clicked()), this, SLOT(nextMatch()));
    connect( addBtn, SIGNAL(clicked()), this, SLOT(addMatch()) );
    connect( removeBtn, SIGNAL(clicked()), this, SLOT(removeMatch()) );

    charList->setFocus();

    enableButtons();
}

void CharSetEdit::setCharSet( QIMPenCharSet *c )
{
    if ( currentSet )
        pw->removeCharSet( 0 );
    currentSet = c;
    fillCharList();
    pw->insertCharSet( currentSet );
    inputChar->clear();
    if ( charList->count() ) {
        charList->setItemSelected( charList->item(0), true );
        charList->setCurrentItem( charList->item(0) );
        selectCode(0);
    }
}

QIMPenCharSet *CharSetEdit::charSet() const
{
    return currentSet;
}

class CharListItem : public QListWidgetItem
{
public:
    CharListItem( const QString &text, const QIMPenChar &c )
        : QListWidgetItem( text )
    {
        setChar(c);
    }

    void setChar(const QIMPenChar &ch) {
        _code = ch.key();
        _char = ch.repCharacter();
    }

    bool testChar(const QIMPenChar &ch) const {
        return ch.key() == _code && ch.repCharacter() == _char;
    }

    bool testEquivalent(const CharListItem *other) const {
        return other->key() == _code && other->character() == _char;
    }

    uint key() const { return _code; }
    QChar character() const { return _char; }

protected:
    uint _code;
    QChar _char;
};

/*!
  Fill the character list box with the characters.  Duplicates are not
  inserted.
*/
void CharSetEdit::fillCharList()
{
    charList->clear();
    QIMPenCharIterator it = currentSet->characters().begin();
    CharListItem *li = 0;
    for ( ; it != currentSet->characters().end(); ++it ) {
        QIMPenChar *p = (*it);
        QString n = p->name();
        if ( (n != " " && n.trimmed().isEmpty()) || !n[0].isPrint() ) continue;

        QList<QListWidgetItem *>items = charList->findItems( n, Qt::MatchExactly );
        // duplicates, cannot ever be more than 1, we dont add if found
        if (( items.count() > 0 )
                && ((CharListItem *)(items[0]))->testChar(*p))
        {
            continue;
        }
        li = new CharListItem( n, *p );
        charList->addItem( li );
    }
    currentChar = 0;
}

void CharSetEdit::enableButtons()
{
    addBtn->setEnabled(!addFlag);

    bool haveSystem = false;
    bool haveChanged = addFlag;
    bool haveChar = addFlag;
    CharListItem *ch = (CharListItem *)charList->currentItem();
    if(!ch)
    {
        resetCharBtn->setEnabled(false);
        return;
    };
    QIMPenCharIterator it = currentSet->characters().begin();
    for ( ; it != currentSet->characters().end() && !(haveSystem && haveChanged && haveChar); ++it ) {
        if(ch->testChar(**it))
        {
            if ((*it)->testFlag(QIMPenChar::System))
            {
                haveSystem = true;
                if((*it)->testFlag(QIMPenChar::Deleted))
                    haveChanged = true;
                else
                    haveChar = true;
            } else if (!(*it)->testFlag(QIMPenChar::Deleted))
            {
                haveChanged = true;
                haveChar = true;
            }
        }
    }

    removeBtn->setEnabled(haveChar || addFlag);
    resetCharBtn->setEnabled(haveSystem && haveChanged);
    updateLabel(); // this is probably redundant most of the time
}

/*!
  Find the previous character with the same code as the current one.
  returns 0 if there is no previous character.
*/
QIMPenChar *CharSetEdit::findPrev()
{
    if ( !currentChar )
        return 0;
    QIMPenCharIterator it = currentSet->characters().end();
    CharListItem *ch = (CharListItem *)charList->currentItem();
    bool found = false;
    do  // probably should use reverse iterator here
    {
        --it;
        if ( !found && (*it) == currentChar )
        {
            found = true;
        }
        else if ( found && ch->testChar(*(*it)) &&
                !(*it)->testFlag( QIMPenChar::Deleted ) )
        {
            return (*it);
        }
    } while ( it != currentSet->characters().begin() );

    return 0;
}

/*!
  Find the next character with the same code as the current one.
  returns 0 if there is no next character.
*/
QIMPenChar *CharSetEdit::findNext()
{
    if ( !currentChar )
        return 0;
    QIMPenCharIterator it = currentSet->characters().begin();
    CharListItem *ch = (CharListItem *)charList->currentItem();
    bool found = false;
    for ( ; it != currentSet->characters().end(); ++it ) {
        if ( !found && (*it) == currentChar )
            found = true;
        else if ( found && ch->testChar(*(*it)) &&
                    !(*it)->testFlag( QIMPenChar::Deleted ) ) {
            return (*it);
        }
    }

    return 0;
}

void CharSetEdit::setCurrentChar( QIMPenChar *pc )
{
    CharListItem *ch = (CharListItem *)charList->currentItem();
    currentChar = pc;
    pw->showCharacter( currentChar );
    if ( currentChar ) {
        if (currentChar->testFlag(QIMPenChar::System)) {
            delCharBtn->setEnabled(false);
            resetCharBtn->setEnabled(false);

            bool haveMissing = false;
            QIMPenCharIterator it = currentSet->characters().begin();
            for ( ; it != currentSet->characters().end(); ++it ) {
                if ( ch && ch->testChar(*(*it)) &&
                        (*it)->testFlag( QIMPenChar::Deleted ) ) {
                    haveMissing = true;
                    break;
                }
            }
            resetCharBtn->setEnabled(haveMissing);
        } else {
            bool haveSystem = false;
            QIMPenCharIterator it = currentSet->characters().begin();
            for ( ; it != currentSet->characters().end(); ++it ) {
                if ( ch && ch->testChar(*(*it)) &&
                        (*it)->testFlag( QIMPenChar::System ) ) {
                    haveSystem = true;
                    break;
                }
            }
            delCharBtn->setEnabled(!haveSystem);
            resetCharBtn->setEnabled(haveSystem);
        }

    }
}

void CharSetEdit::prevMatch()
{
    // if not adding, or adding and something to add.
    if (!addFlag || !inputChar->isEmpty()) {
        if (addFlag) {
            appendMatch();
            pw->setReadOnly(true);
            addFlag = false;
        }
        QIMPenChar *pc = findPrev();
        if ( pc ) {
            setCurrentChar( pc );
            --matchIndex;
        }
    } else if (addFlag) {
        // adding and something to add, (or would have met prev)
        matchCount--;
        matchIndex = matchCount;
        pw->showCharacter(currentChar);
        pw->setReadOnly(true);
        addFlag = false;
        addBtn->setEnabled(true);
    }
    updateLabel();
    enableButtons();
}

void CharSetEdit::nextMatch()
{
    QIMPenChar *pc = findNext();
    if ( pc ) {
        setCurrentChar( pc );
        ++matchIndex;
        updateLabel();
    }
    enableButtons();
}

void CharSetEdit::firstMatch()
{
    CharListItem *ch = (CharListItem *)charList->currentItem();
    QIMPenCharIterator it = currentSet->characters().begin();
    for ( ; it != currentSet->characters().end(); ++it ) {
        if ( ch->testChar(*(*it))) {
            if ( *it != currentChar)
                setCurrentChar( *it );
            return;
        }
    }
}

void CharSetEdit::lastMatch()
{
    CharListItem *ch = (CharListItem *)charList->currentItem();
    QIMPenCharIterator it = currentSet->characters().begin();
    QIMPenChar *lastFound = 0;
    for ( ; it != currentSet->characters().end(); ++it ) {
        if ( ch->testChar(*(*it))) {
            lastFound = *it;
        }
    }
    if (lastFound && lastFound != currentChar)
        setCurrentChar(lastFound);
    else
        setCurrentChar(lastFound);
}

void CharSetEdit::clearMatch()
{
    inputChar->clear();
    pw->clear();
    addFlag = false;
    enableButtons();
}

void CharSetEdit::selectItemCode( QListWidgetItem *item, QListWidgetItem *previous )
{
    if (addFlag && !inputChar->isEmpty()) {
        // adding and something to add,
        appendMatch(previous);
    };
    if (addFlag){
        pw->setReadOnly(true);
        addFlag = false;
    };

    if ( item == NULL ) return;
    int index = charList->row( item );
    if ( index >= 0 )
        selectCode( index );
}

void CharSetEdit::selectCode( int i )
{
    checkStoreMatch();
    currentChar = 0;
    CharListItem *ch = (CharListItem *)charList->item(i);
    QIMPenCharIterator it = currentSet->characters().begin();
    matchCount = 0;
    matchIndex = 0;
    for ( ; it != currentSet->characters().end(); ++it ) {
        if ( ch->testChar(*(*it)) &&
             !(*it)->testFlag( QIMPenChar::Deleted ) ) {
            if (matchCount == 0) {
                setCurrentChar( *it );
                matchIndex = 1;
            }
            matchCount++;
        }
    }
    updateLabel();
    if ( matchCount == 0 )
        setCurrentChar( 0 );
    inputChar->clear();
    enableButtons();
    lastCh = i;
}

void CharSetEdit::updateLabel()
{
    QString itemText = tr("%1/%2", "way %1 of drawing character out of a total %2 ways of drawing character");
    itemDisplay->setText(itemText.arg(matchIndex).arg(matchCount));
    prevBtn->setEnabled( matchIndex > 1 );
    nextBtn->setEnabled( matchIndex < matchCount );

    removeBtn->setEnabled( matchCount > 0 );
}

/*
   Action should be:
   clearMatch,
   flag that what is drawn will be a new match
*/
void CharSetEdit::addMatch()
{
    checkStoreMatch();
    lastMatch();
    pw->setReadOnly(false);
    matchCount++;
    matchIndex = matchCount;
    updateLabel();
    clearMatch();
    addFlag = true;
    addBtn->setEnabled(false);
    pw->greyStroke();
}

void CharSetEdit::checkStoreMatch()
{
    if (addFlag) {
        addFlag = false;
        appendMatch();
        pw->setReadOnly(true);
    }
}

void CharSetEdit::appendMatch(QListWidgetItem *item)
{
    CharListItem *ch;
    if (item)
        ch = static_cast<CharListItem*>(item);
    else
        ch = (CharListItem *)charList->currentItem();

    // should be more of an assert.
    if ( !inputChar->isEmpty() ) {
        QIMPenChar *pc = new QIMPenChar( *inputChar );
        pc->setRepCharacter( ch->character() );
        pc->setKey( ch->key() );

        // User characters override all matching system characters.
        // Copy and mark deleted identical system characters.
        QIMPenCharIterator it = currentSet->characters().begin();
        QIMPenChar *sc = 0;
        while ( it != currentSet->characters().end() )
        {
            sc = *it++;
            if ( ch->testChar(*sc) &&
                 sc->testFlag( QIMPenChar::System ) &&
                 !sc->testFlag( QIMPenChar::Deleted ) )
            {
                QIMPenChar *cc = new QIMPenChar( *sc );
                cc->clearFlag( QIMPenChar::System );
                currentSet->addChar( cc );
                sc->setFlag( QIMPenChar::Deleted );
            }
        }

        currentSet->addChar( pc );
        setCurrentChar( pc );
        inputChar->clear();
        enableButtons();
    }
}

/* adds a whole new code */
void CharSetEdit::addChar()
{
    checkStoreMatch();

    //if ( !inputChar->isEmpty() ) {
    QIMPenInputCharDlg dlg( 0, "newchar", true , mIsFS );
    if (QtopiaApplication::execDialog(&dlg)) {

        uint key = dlg.unicode();
        // if its a special...
        QChar c;
        if (!(key & 0xffff0000))
            c = key;

        QIMPenChar currentCode;
        currentCode.setKey(key);
        currentCode.setRepCharacter(c);

        // update combo now?  disable combo?
        // if added an existing char, do a new 'match'
        // if new code, then add code, set to current, add new match.
        bool foundMatch = false;
        for (int i = 0; i < charList->count(); ++i) {
            if (((CharListItem *)charList->item(i))->testChar(currentCode)) {
                foundMatch = true;
                charList->setCurrentItem(charList->item(i));
                break;
            }
        }
        if (!foundMatch) {
            // create a blank one.
            QIMPenChar *pc = new QIMPenChar( *inputChar );
            pc->setKey( currentCode.key() );
            pc->setRepCharacter( currentCode.repCharacter() );
            CharListItem *cli = new CharListItem( pc->name(), *pc);
            charList->addItem(cli);
            charList->setCurrentItem(cli);
        }
        addMatch();
        updateLabel();
    }
}

/* removes a user added char */
void CharSetEdit::removeChar()
{
    addFlag = false; // if was adding, just removed the char... can't add now
    pw->setReadOnly(true);

    int idx = 0;
    CharListItem *ch = (CharListItem *)charList->currentItem();
    // Can't use the normal iterator, since the list is const
    while ( idx < currentSet->characters().count() ) {
        QIMPenChar *pc = currentSet->characters()[idx];
        if ( ch->testChar(*pc) && !pc->testFlag( QIMPenChar::System ) )
            currentSet->removeChar( pc );
        else
            ++idx;
    }

    for (int i = 0; i < charList->count();++i) {
        if (((CharListItem *)charList->item(i))->testEquivalent(ch)) {
            delete charList->takeItem(i);
            break;
        }
    }
    updateLabel();
}

void CharSetEdit::removeMatch()
{
    if (addFlag) {
        // assume user meant cancel add match
        /// same as match prev when not added...
        //matchCount--;
        //matchIndex = matchCount;
        pw->showCharacter(currentChar);
        pw->setReadOnly(true);
        addFlag = false;
    } else {
        if ( currentChar ) {
            QIMPenChar *pc = findPrev();
            if ( !pc ) pc = findNext();
            else matchIndex--;
            if ( currentChar->testFlag( QIMPenChar::System ) )
                currentChar->setFlag( QIMPenChar::Deleted );
            else
                currentSet->removeChar( currentChar );
            setCurrentChar( pc );
        }
    }
    if (matchIndex == matchCount)
        matchIndex--;
    matchCount--;
    updateLabel();
    enableButtons();
}

void CharSetEdit::resetMatches()
{
    if (addFlag) {
        addFlag = false;
        pw->setReadOnly(true);
    }
    CharListItem *ch = (CharListItem *)charList->currentItem();
    if ( ch ) {
        currentChar = 0;
        bool haveSystem = false;
        QIMPenCharIterator it = currentSet->characters().begin();
        for ( ; it != currentSet->characters().end(); ++it ) {
            if ( ch->testChar(*(*it)) &&
                 (*it)->testFlag( QIMPenChar::System ) ) {
                haveSystem = true;
                break;
            }
        }
        if ( haveSystem ) {
            // Can't use an iterator since the list is const
            int idx = 0;
            while ( idx < currentSet->characters().count() ) {
                QIMPenChar *pc = currentSet->characters()[idx];
                if ( ch->testChar(*pc) ) {
                    if ( pc->testFlag( QIMPenChar::System ) ) {
                        pc->clearFlag( QIMPenChar::Deleted );
                        if ( !currentChar )
                            currentChar = pc;
                        ++idx;
                    } else {
                        currentSet->removeChar( pc );
                    }
                } else {
                    ++idx;
                }
            }
            setCurrentChar( currentChar );
        }
    }
    selectCode( charList->row( charList->currentItem() ));
    addBtn->setEnabled(true);
}

void CharSetEdit::newStroke( QIMPenStroke *st )
{
    inputChar->addStroke( st );

    CharListItem *ch = (CharListItem *)charList->currentItem();
    addBtn->setEnabled(true);
    if ( currentChar ) {
        bool haveSystem = false;
        QIMPenCharIterator it = currentSet->characters().begin();
        for ( ; it != currentSet->characters().end(); ++it ) {
            if ( ch->testChar(*(*it)) &&
                    (*it)->testFlag( QIMPenChar::System ) ) {
                haveSystem = true;
                break;
            }
        }
        delCharBtn->setEnabled(!haveSystem);
        resetCharBtn->setEnabled(haveSystem);
    } else {
        // new Stroke on a new character....
        // can delete and delets troke and add match etc.
        delCharBtn->setEnabled(true);
        resetCharBtn->setEnabled(false);
    }
}

