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

#include "searchview.h"

#include <QtopiaApplication>
#include <QSoftMenuBar>
#include <QMenu>
#include <QDesktopWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QApplication>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QMailFolder>
#include <QMailMessage>
#include <QContactSelector>

SearchView::SearchView(QWidget* parent)
    : QDialog(parent)
{
    setupUi( this );

    delete layout();
    QGridLayout *g = new QGridLayout(this);
    sv = new QScrollArea(this);
    sv->setFocusPolicy(Qt::NoFocus);
    sv->setWidgetResizable( true );
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidget(searchFrame);
    sv->setFocusPolicy(Qt::NoFocus);
    g->addWidget(sv, 0, 0);
    int dw = QApplication::desktop()->availableGeometry().width();
    searchFrame->setMaximumWidth(dw - qApp->style()->pixelMetric(QStyle::PM_SliderLength) + 4 );
    sv->setMaximumWidth(dw);
    setWindowTitle( tr("Search") );
    QMenu *searchContext = QSoftMenuBar::menuFor(this);
    connect(searchContext, SIGNAL(aboutToShow()), this, SLOT(updateActions()));
    QIcon abicon(":icon/addressbook/AddressBook");
    pickAddressAction = new QAction(abicon,
                                    tr("From contacts",
                                       "Find email address "
                                       "from Contacts application"),
                                    this);
    searchContext->addAction(pickAddressAction);
    connect(pickAddressAction, SIGNAL(triggered()), this, SLOT(editRecipients()));
    init();
}

SearchView::~SearchView()
{
}

void SearchView::init()
{
    dateAfter = QDate::currentDate();
    dateAfterButton->setDate( dateAfter );
    dateBefore = QDate::currentDate();
    dateBeforeButton->setDate( dateBefore );
    setTabOrder(mailbox, status);
    setTabOrder(status, fromLine);
    setTabOrder(fromLine, toLine);
    setTabOrder(toLine,subjectLine);
    setTabOrder(subjectLine,bodyLine);
    setTabOrder(dateAfterBox, dateAfterButton);
    setTabOrder(dateAfterButton, dateBeforeBox);
    setTabOrder(dateBeforeBox, dateBeforeButton);
}

QMailMessageKey SearchView::searchKey() const
{
    QMailMessageKey resultKey;

    static QMailMessageKey inboxFolderKey(QMailMessageKey::ParentFolderId,QMailFolderId(QMailFolder::InboxFolder));
    static QMailMessageKey outboxFolderKey(QMailMessageKey::ParentFolderId,QMailFolderId(QMailFolder::OutboxFolder));
    static QMailMessageKey draftsFolderKey(QMailMessageKey::ParentFolderId,QMailFolderId(QMailFolder::DraftsFolder));
    static QMailMessageKey sentFolderKey(QMailMessageKey::ParentFolderId,QMailFolderId(QMailFolder::SentFolder));
    static QMailMessageKey trashFolderKey(QMailMessageKey::ParentFolderId,QMailFolderId(QMailFolder::TrashFolder));
    static QMailMessageKey inboxWithImapKey(inboxFolderKey | ~(outboxFolderKey | draftsFolderKey | sentFolderKey | trashFolderKey)); 

    //folder

    switch(mailbox->currentIndex())
    {
        case 1: //inbox with imap subfolders 
            resultKey &= inboxWithImapKey; 
            break;
        case 2: //outbox
            resultKey &= outboxFolderKey;
            break;
        case 3: //drafts
            resultKey &= draftsFolderKey; 
            break;
        case 4: //sent
            resultKey &= sentFolderKey; 
            break;
        case 5: //trash
            resultKey &= trashFolderKey; 
            break;
    }

    //status

    switch(status->currentIndex())
    {
         case 1: //read
            resultKey &= (QMailMessageKey(QMailMessageKey::Status,QMailMessage::Read,QMailDataComparator::Includes) |
                          QMailMessageKey(QMailMessageKey::Status,QMailMessage::ReadElsewhere,QMailDataComparator::Includes));
            break;
        case 2: //unread
            resultKey &= (~QMailMessageKey(QMailMessageKey::Status,QMailMessage::Read,QMailDataComparator::Includes) &
                          ~QMailMessageKey(QMailMessageKey::Status,QMailMessage::ReadElsewhere,QMailDataComparator::Includes));
            break;
        case 3: //replied
            resultKey &= ~QMailMessageKey(QMailMessageKey::Status,QMailMessage::Replied,QMailDataComparator::Includes);
            break;
        case 4: //replied
            resultKey &= QMailMessageKey(QMailMessageKey::Status,QMailMessage::Removed,QMailDataComparator::Includes);
            break;
    }

    if(!fromLine->text().isEmpty())
        resultKey &= QMailMessageKey(QMailMessageKey::Sender,fromLine->text(),QMailDataComparator::Includes);

    if(!toLine->text().isEmpty())
        resultKey &= QMailMessageKey(QMailMessageKey::Recipients,toLine->text(),QMailDataComparator::Includes);

    if(!subjectLine->text().isEmpty())
        resultKey &= QMailMessageKey(QMailMessageKey::Subject,subjectLine->text(),QMailDataComparator::Includes);

    //dates

    if(dateAfterBox->isChecked())
    {
        QDate afterDate= dateAfterButton->date();
        resultKey &= QMailMessageKey(QMailMessageKey::TimeStamp,afterDate,QMailDataComparator::GreaterThan);
    }

    if(dateBeforeBox->isChecked())
    {
        QDate beforeDate = dateBeforeButton->date();
        resultKey &= QMailMessageKey(QMailMessageKey::TimeStamp,beforeDate,QMailDataComparator::LessThan);
    }

    return resultKey;
}

QString SearchView::bodyText() const
{
    return bodyLine->text();
}

void SearchView::reset() 
{
    mailbox->setCurrentIndex(0);
    status->setCurrentIndex(0);
    fromLine->clear();
    toLine->clear();
    subjectLine->clear();
    bodyLine->clear();
    dateAfterBox->setChecked(false);
    dateBeforeBox->setChecked(false); 

    mailbox->setFocus();
}

void SearchView::updateActions()
{
    pickAddressAction->setVisible(fromLine->hasFocus() || toLine->hasFocus());
}

void SearchView::editRecipients()
{
    QString txt;
    QLineEdit *edit = fromLine;
    if (toLine->hasFocus())
        edit = toLine;
    QContactSelector selector;
    selector.setObjectName("select-contact");
    
    QContactModel model(&selector);
    
    QSettings config( "Trolltech", "Contacts" );
    config.beginGroup( "default" );
    if (config.contains("SelectedSources/size")) {
        int count = config.beginReadArray("SelectedSources");
        QSet<QPimSource> set;
        for(int i = 0; i < count; ++i) {
            config.setArrayIndex(i);
            QPimSource s;
            s.context = QUuid(config.value("context").toString());
            s.identity = config.value("identity").toString();
            set.insert(s);
        }
        config.endArray();
        model.setVisibleSources(set);
    }
    
    selector.setModel(&model);
    selector.setAcceptTextEnabled(false);
    
    if (QtopiaApplication::execDialog(&selector) == QDialog::Accepted) {
        QContact contact(selector.selectedContact());
        edit->setText(contact.defaultEmail());
    }
}
