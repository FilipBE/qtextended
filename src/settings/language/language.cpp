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

#include "languagesettings.h"
#include "langmodel.h"

#include <QtopiaApplication>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qtopiaipcenvelope.h>
#include <qsoftmenubar.h>
#include <QtopiaItemDelegate>
#include <QFile>
#include <QMessageBox>
#include <QListView>
#include <QCheckBox>
#include <QPushButton>
#include <QDir>
#include <QMenu>
#include <QKeyEvent>


using namespace Ui;

//copied from PkimMatcher in order to remove dependency on pkim
QStringList LanguageSettings::langs;

QStringList LanguageSettings::dictLanguages()
{
    if ( langs.isEmpty() ) {
        QString basename = Qtopia::qtopiaDir() + "/etc/dict/";
        QDir dir(basename);
        QStringList dftLangs = dir.entryList(QDir::Dirs);
        for (QStringList::ConstIterator it = dftLangs.begin(); it != dftLangs.end(); ++it){
            QString lang = *it;
            if (QFile::exists(basename+lang+"/words.dawg")) {
                langs.append(lang);
            }
        }
    }

    return langs;
}

class TouchScreenAcceptDlg : public QDialog
{
    Q_OBJECT
public:

    TouchScreenAcceptDlg(const QModelIndex& index, bool allowActivation,
        const QString lang, QWidget* parent = 0, Qt::WindowFlags flags = 0)
        : QDialog(parent, flags), change(false), dictionaryState(false)
    {
        setWindowTitle( tr("Options") );
        QVBoxLayout* vbox = new QVBoxLayout(this);
 
        checkbox = new QCheckBox(tr("Use for Input"));
        checkbox->setEnabled(LanguageSettings::dictLanguages().contains(lang));
        bool useForInput = index.data(Qt::UserRole).toBool();
        checkbox->setChecked(useForInput);
        dictionaryState = useForInput;
        vbox->addWidget(checkbox);

        checkbox->setEnabled(allowActivation); //the current lang is used as primary dictionary

        if (allowActivation) {
            QPushButton* btn = new QPushButton(tr("Activate"));
            vbox->addWidget(btn);
            connect(btn, SIGNAL(clicked()), this, SLOT(activateLanguage()));
        }
    }

    bool inputToggled() const
    {
        return dictionaryState != checkbox->isChecked();
    }

    bool languageActivated() const
    {
        return change;
    }

private slots:
    void activateLanguage()
    {
        change = true;
        accept();
    }

private:
    QCheckBox* checkbox;
    bool change;
    bool dictionaryState;

};

#include "langname.h"

LanguageSettings::LanguageSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    confirmChange(true), notifySystem(true), a_input(0)
{
    setupUi(this);
    setModal( true );

    reset();

    QList<FontedItem> itemList;
    QStringList qpepaths = Qtopia::installPaths();
    foreach(QString tfn, qpepaths) {
        tfn +="i18n/";
        QDir langDir = tfn;
        QStringList list = langDir.entryList(QStringList("*") );
        QStringList::Iterator it;
        for( it = list.begin(); it != list.end(); ++it ) {
            QString id = (*it);
            if ( !langAvail.contains(id) ) {
                QFileInfo desktopFile( tfn + "/" + id + "/.directory" );
                if( desktopFile.exists() ) {
                    langAvail.append(id);
                    QFont font("dejavu");
                    font.setPointSize( 8 );
                    //we need to start with dejavu or all names would be written
                    //in unifont if this application is using unifont
                    bool rightToLeft = false;
                    QString langName = languageName(id, &font, &rightToLeft);
                    qLog(I18n) << "Found language:" << langName
                        << id << "Font:" << font.family() << "Font size:"<< font.pointSize()<< "RTL:" << rightToLeft;
                    FontedItem item (langName, font,
                        dictLanguages().contains(id) && inputLanguages.contains(id),
                        id == chosenLanguage );
                    item.direction = rightToLeft ? Qt::RightToLeft : Qt::LeftToRight;
                    itemList.append(item);
                }
            }
        }
    }

    model = new LanguageModel(this, itemList);

    listView = new QListView(this);
    listView->setFrameStyle(QFrame::NoFrame);
    listView->setModel(model);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);
    listView->setAlternatingRowColors( true );
    LanguageDelegate * delegate = new LanguageDelegate( listView );
    listView->setItemDelegate( delegate );

    //set current language
    int currentLanguage = langAvail.indexOf(chosenLanguage);
    if (currentLanguage >= 0)
        listView->setCurrentIndex(model->index(currentLanguage));

    vboxLayout->addWidget(listView);

    QItemSelectionModel *selectionModel = listView->selectionModel();
    connect(selectionModel,SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(applyLanguage(QModelIndex)));
    connect(listView,SIGNAL(activated(QModelIndex)), this, SLOT(newLanguageSelected()));

    if (!Qtopia::mousePreferred()) {
        a_input = new QAction( tr("Use for input"), this );
        connect( a_input, SIGNAL(triggered(bool)), this, SLOT(inputToggled()) );
        a_input->setCheckable(true);
        updateActions(listView->currentIndex());

        QMenu *menu = QSoftMenuBar::menuFor(this);
        menu->addAction(a_input);
    }
}

LanguageSettings::~LanguageSettings()
{
}

void LanguageSettings::setConfirm(bool c)
{
    confirmChange = c;
}

void LanguageSettings::setNotify(bool notify)
{
    notifySystem = notify;
}

void LanguageSettings::inputToggled(const QModelIndex &index)
{
    QString lang = langAvail.at( index.row() );
    inputLanguages.removeAll(lang);
    bool selected = model->data(index,Qt::UserRole).toBool();
    if (!selected && dictLanguages().contains(lang)) {
        inputLanguages.append(lang);
        model->setData(index, QVariant(true), Qt::UserRole);
    } else {
        model->setData(index, QVariant(false), Qt::UserRole);
    }

    updateActions(index);
}

void LanguageSettings::updateActions(const QModelIndex& index)
{
    if (!index.isValid() || Qtopia::mousePreferred())
        return;
    QString lang = langAvail.at( listView->currentIndex().row() );
    bool dictLang = dictLanguages().contains(lang);

    QSettings config("Trolltech","locale");
    config.beginGroup( "Language" );
    QString clang = config.value( "Language" ).toString();

    a_input->setEnabled(dictLang && clang!=lang);
    a_input->setVisible(dictLang);
    a_input->setChecked(inputLanguages.contains(lang));
}

void LanguageSettings::inputToggled()
{
    inputToggled(listView->currentIndex());
}

void LanguageSettings::forceChosen()
{
    // For simplicity, make primary reading language also primary writing language.
    QString l = chosenLanguage;
    while (1) {
        if ( dictLanguages().contains(l) )
            break;
        int e = l.indexOf(QRegExp("[._]"));
        if ( e >= 0 ) {
            l = l.left(e);
        } else {
            // Give up.
            l = chosenLanguage;
            break;
        }
    }
    inputLanguages.removeAll(l);
    inputLanguages.prepend(l);
}

void LanguageSettings::newLanguageSelected()
{
    chosenLanguage = langAvail.at( listView->currentIndex().row() );
    QSettings config("Trolltech","locale");
    config.beginGroup( "Language" );
    QString lang = config.value( "Language" ).toString();

    if ( Qtopia::mousePreferred() )
    {
        TouchScreenAcceptDlg dlg(listView->currentIndex(), lang!=chosenLanguage, chosenLanguage, this);
        QtopiaApplication::execDialog( &dlg, true );
        if (dlg.result() == QDialog::Rejected) {
            return;
        } else {
            if (dlg.inputToggled())
                inputToggled();
            if (!dlg.languageActivated())
                return;
        }
    }


    if (lang == chosenLanguage) {
        if ( !Qtopia::mousePreferred() )
            accept();
        return;
    }

    if( lang != chosenLanguage && confirmChange
        && QMessageBox::warning( this, tr("Language Change"),
                                  tr("<qt>This will cause "
                                  "Qt Extended to restart, closing all applications."
                                  "<p>Change Language?</qt>"),
                                  QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) {
        return;
    }

    // because reading language is primary writing language (see forceChosen())
    // we have to remove previous language from inputLanguages if it does not
    // have a dictionary.
    if (!dictLanguages().contains(lang))
        inputLanguages.removeAll(lang);

    config.setValue( "Language", chosenLanguage );
    forceChosen();
    config.setValue("InputLanguages", inputLanguages);
    config.sync();

    qLog(I18n) << "New language:" << chosenLanguage;
    qLog(I18n) << "Using following dictionaries:" << inputLanguages;

    //we have to set the new default font for the new language

    QSettings sysConfig(QSettings::SystemScope, "Trolltech", "qpe");
    sysConfig.beginGroup("Font");
    QString fontFamily = sysConfig.value("FontFamily[]").toString();
    QString fontSize = sysConfig.value("FontSize[]").toString();

    //remove the current font selection and let the new language determine
    //what the font should be
    QSettings qpeconfig("Trolltech", "qpe");
    qpeconfig.beginGroup("Font");
    qpeconfig.remove("");

    //remove the current date format and let the new language determine
    //what the format should be
    qpeconfig.remove("Date/DateFormat");

    qpeconfig.sync();

    if (lang != chosenLanguage && notifySystem) {
        QtopiaIpcEnvelope e("QPE/System","language(QString)");
        e << chosenLanguage;
    }

    accept();
}

void LanguageSettings::accept()
{
    QSettings config("Trolltech","locale");
    config.beginGroup( "Language" );
    config.setValue("InputLanguages", inputLanguages);
    config.sync();

    QDialog::accept();
}

void LanguageSettings::applyLanguage(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;
    chosenLanguage = langAvail.at(idx.row());
    updateActions(idx);
}


void LanguageSettings::reject()
{
    reset();
    QDialog::reject();
}

void LanguageSettings::reset()
{
    QSettings config("Trolltech","locale");
    config.beginGroup( "Language" );
    QString l = getenv("LANG");
    l = config.value( "Language", l ).toString();
    if(l.isEmpty())
        l = "en_US"; // No tr
    else
    {
        int index = l.indexOf(QChar('.'));
        if (index > 0)
            l = l.left(index);
    }

    chosenLanguage = l;
    inputLanguages = config.value( "InputLanguages").toStringList();
    forceChosen();
}


void LanguageSettings::done(int r)
{
    QDialog::done(r);
    close();
}

#include "language.moc"
