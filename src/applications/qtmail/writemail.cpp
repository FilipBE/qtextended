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

#include "writemail.h"
#include "selectcomposerwidget.h"
#include "qtmailwindow.h"

#include <private/accountconfiguration_p.h>

#include <qtopialog.h>
#include <QMailAccount>
#include <QMailComposerInterface>
#include <QSoftMenuBar>
#include <QStackedWidget>
#include <QDrmContentPlugin>

WriteMail::WriteMail(QWidget* parent)
    :
#ifdef QTOPIA_HOMEUI
    QDialog(parent),
#else
    QMainWindow(parent),
#endif
    m_composerInterface(0),
    m_cancelAction(0),
    m_draftAction(0),
    m_widgetStack(0),
    _detailsOnly(false),
    m_selectComposerWidget(0)
{
    /*
      because of the way QtMail has been structured, even though
      WriteMail is a main window, setting its caption has no effect
      because its parent is also a main window. have to set it through
      the real main window.
    */
    m_mainWindow = QTMailWindow::singleton();

    init();
}

WriteMail::~WriteMail()
{
    delete m_composerInterface;
    m_composerInterface = 0;
}

void WriteMail::setDefaultAccount(const QMailAccountId& defaultId)
{
    defaultAccountId = defaultId;
    if(m_composerInterface)
        m_composerInterface->setDefaultAccount(defaultId);
}

void WriteMail::init()
{
    m_widgetStack = new QStackedWidget(this);

    QDrmContentPlugin::initialize();

    m_draftAction = new QAction(QIcon(":icon/draft"),tr("Save in drafts"),this);
    connect( m_draftAction, SIGNAL(triggered()), this, SLOT(draft()) );
    m_draftAction->setWhatsThis( tr("Save this message as a draft.") );
    addAction(m_draftAction);

    m_cancelAction = new QAction(QIcon(":icon/cancel"),tr("Cancel"),this);
    connect( m_cancelAction, SIGNAL(triggered()), this, SLOT(discard()) );
    addAction(m_cancelAction);

    m_selectComposerWidget = new SelectComposerWidget(this);
    m_selectComposerWidget->setObjectName("selectComposer");
    connect(m_selectComposerWidget, SIGNAL(selected(QPair<QString,QMailMessage::MessageType>)), 
            this, SLOT(composerSelected(QPair<QString,QMailMessage::MessageType>)));
    connect(m_selectComposerWidget, SIGNAL(cancel()), this, SLOT(discard()));
    m_widgetStack->addWidget(m_selectComposerWidget);

#ifdef QTOPIA_HOMEUI
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_widgetStack);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    QDialog::setLayout(layout);
#else
    setCentralWidget(m_widgetStack);
    QSoftMenuBar::setLabel(m_selectComposerWidget, Qt::Key_Back, QSoftMenuBar::Cancel);
#endif

}

bool WriteMail::sendStage()
{
    if (!isComplete()) {
        // The user must either complete the message, save as draft or explicitly cancel
        return false;
    }

    if (buildMail()) {
        if (largeAttachments()) {
            //prompt for action
            QMessageBox::StandardButton result;
            result = QMessageBox::question(qApp->activeWindow(),
                    tr("Large attachments"),
                    tr("The message has large attachments. Send now?"),
                    QMessageBox::Yes | QMessageBox::No);
            if(result == QMessageBox::No)
            {
                draft();
                QMessageBox::warning(qApp->activeWindow(),
                        tr("Message saved"),
                        tr("The message has been saved in the Drafts folder"),
                        tr("OK") );

                return true;
            }
        }

        emit enqueueMail(mail);
        emitFinished();
    } else {
        qLog(Messaging) << "Unable to build mail for transmission!";
    }

    // Prevent double deletion of composer textedit that leads to crash on exit
    reset();

    return true;
}

bool WriteMail::isComplete() const
{
    if (m_composerInterface && m_composerInterface->isReadyToSend()) {
        if (changed() || (QMessageBox::question(qApp->activeWindow(), 
                                                tr("Empty message"), 
                                                tr("The message is currently empty. Do you wish to send an empty message?"), 
                                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)) {
            return true;
        }
    } else {
        QMessageBox::warning(qApp->activeWindow(),
                             tr("Incomplete message"),
                             tr("The message cannot be sent until at least one recipient has been entered."),
                             QMessageBox::Ok);
    }

    return false;
}

bool WriteMail::saveChangesOnRequest()
{
    // ideally, you'd also check to see if the message is the same as it was
    // when we started working on it
    // dont bother with a prompt for empty composers. Not likely to want to save empty draft.
    if (hasMessageChanged && hasContent() &&
        QMessageBox::warning(this,
                             tr("Save to drafts"),
                             tr("Do you wish to save the message to drafts?"),
                             QMessageBox::Yes,
                             QMessageBox::No) == QMessageBox::Yes) {
        draft();
    } else {
        discard();
    }

    return true;
}

bool WriteMail::buildMail()
{
    //retain the old mail id since it may have been a draft
    QMailMessageId existingId = mail.id();

    // Ensure the signature of the selected account is used
    m_composerInterface->setSignature(signature());

    // Extract the message from the composer
    mail = m_composerInterface->message();

    mail.setDate( QMailTimeStamp::currentDateTime() );
    mail.setId(existingId);
    mail.setStatus( QMailMessage::Outgoing, true);
    mail.setStatus( QMailMessage::Downloaded, true);
    mail.setStatus( QMailMessage::Read,true);
    return true;
}

/*  TODO: connect this method to the input widgets so we can actually know whether
    the mail was changed or not */
bool WriteMail::changed() const
{
    if (!m_composerInterface || m_composerInterface->isEmpty())
        return false;

    return true;
}

void WriteMail::attach( const QContent &dl, QMailMessage::AttachmentsAction action )
{
    if (m_composerInterface) {
        m_composerInterface->attach( dl, action );
    } else {
        qWarning("WriteMail::attach called with no composer interface present.");
    }
}

void WriteMail::reply(const QMailMessage& replyMail, int action)
{
    newMail(replyMail.messageType());
    if (composer().isEmpty())
        return;

    m_composerInterface->reply(replyMail,action);
    hasMessageChanged = true;
}

void WriteMail::modify(const QMailMessage& previousMessage)
{
    QString recipients = "";

    newMail(previousMessage.messageType());
    if (composer().isEmpty())
        return;

    mail.setId( previousMessage.id() );
    mail.setFrom( previousMessage.from() );
    m_composerInterface->setSignature( signature() );
    m_composerInterface->setMessage( previousMessage );

    // ugh. we need to do this everywhere
    hasMessageChanged = false;
}

void WriteMail::setRecipient(const QString &recipient)
{
    if (m_composerInterface) {
        m_composerInterface->setTo( recipient );
    } else {
        qWarning("WriteMail::setRecipient called with no composer interface present.");
    }
}

void WriteMail::setSubject(const QString &subject)
{
    if (m_composerInterface) {
        m_composerInterface->setSubject( subject );
    } else {
        qWarning("WriteMail::setRecipient called with no composer interface present.");
    }
}

void WriteMail::setBody(const QString &text, const QString &type)
{
    if (m_composerInterface) {
        m_composerInterface->setBody( text, type );
    } else {
        qWarning("WriteMail::setBody called with no composer interface present.");
    }
}

bool WriteMail::hasContent()
{
    // Be conservative when returning false, which means the message can
    // be discarded without user confirmation.
    if (!m_composerInterface)
        return true;
    return !m_composerInterface->isEmpty();
}

#ifndef QTOPIA_NO_SMS
void WriteMail::setSmsRecipient(const QString &recipient)
{
    m_composerInterface->setTo(recipient);
}
#endif

void WriteMail::setRecipients(const QString &emails, const QString & numbers)
{
    QString to;
    to += emails;
    to = to.trimmed();
    if (to.right( 1 ) != "," && !numbers.isEmpty()
        && !numbers.trimmed().startsWith( "," ))
        to += ", ";
    to +=  numbers;

 if (m_composerInterface) {
        m_composerInterface->setTo( to );
    } else {
        qWarning("WriteMail::setRecipients called with no composer interface present.");
    }
}

void WriteMail::reset()
{
    mail = QMailMessage();

    if (m_composerInterface) {
        // Remove any associated widgets
        m_widgetStack->removeWidget(m_composerInterface);
        // Remove and delete any existing details page
        m_composerInterface->deleteLater();
        m_composerInterface = 0;
    }

    hasMessageChanged = false;
}

bool WriteMail::newMail(QMailMessage::MessageType type, bool detailsOnly)
{
    bool success = false;

    reset();

    _detailsOnly = detailsOnly;

    if (type == QMailMessage::AnyType) {
        bool displaySelector(true);

        // If we have no options, prompt to set up an account
        if (m_selectComposerWidget->availableTypes().isEmpty()) {
            displaySelector = false;

            if (QMessageBox::question(qApp->activeWindow(), 
                                      tr("No accounts configured"), 
                                      tr("No accounts are configured to send messages with. Do you wish to configure one now?"), 
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                emit editAccounts();
            }
        }

#ifndef QTOPIA_HOMEUI
        // If there's just one composer option, select it immediately
        QString key = m_selectComposerWidget->singularKey();
        if (!key.isEmpty()) {
            displaySelector = false;
            success = composerSelected(qMakePair(key, QMailComposerFactory::messageTypes(key).first()));
        }
#endif

        if (displaySelector) {
            m_selectComposerWidget->setSelected(composer(), QMailMessage::AnyType);
            setTitle( tr("Select type") );
            m_widgetStack->setCurrentWidget(m_selectComposerWidget);
            success = true;
        }
    } else {
        QString key = QMailComposerFactory::defaultKey(type);
        if (!key.isEmpty()) {
            success = composerSelected(qMakePair(key, type));
        } else {
            qWarning() << "Cannot edit message of type:" << type;
        }
    }

    return success;
}

void WriteMail::discard()
{
    reset();

    emit discardMail();
    emitFinished();
}

bool WriteMail::draft()
{
    bool result(false);

    if (changed()) {
        if (!buildMail()) {
            qWarning() << "draft() - Unable to buildMail for saveAsDraft!";
        } else {
            emit saveAsDraft( mail );
            emitFinished();
        }
        result = true;
    }

    // Since the existing details page may have hijacked the focus, we need to reset
    reset();

    return result;
}

void WriteMail::contextChanged()
{
    setTitle(m_composerInterface->contextTitle());
}

bool WriteMail::composerSelected(const QPair<QString, QMailMessage::MessageType> &selection)
{
    // We need to ensure that we can send for this composer
    QMailMessage::MessageType selectedType = selection.second;

    if (!m_selectComposerWidget->availableTypes().contains(selectedType)) {
        // See if the user wants to add/edit an account to resolve this
        QString type(QMailComposerFactory::displayName(selection.first, selectedType));
        if (QMessageBox::question(qApp->activeWindow(), 
                                  tr("No accounts configured"), 
                                  tr("No accounts are configured to send %1.\nDo you wish to configure one now?").arg(type), 
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            emit editAccounts();
        }
    }
    if (!m_selectComposerWidget->availableTypes().contains(selectedType)) {
        // If still not possible, then just fail
        emit noSendAccount(selectedType);
        return false;
    }

    setComposer(selection.first);

    m_composerInterface->setMessageType( selectedType );
    m_composerInterface->setDefaultAccount( defaultAccountId );
    m_composerInterface->setSignature( signature() );
    m_composerInterface->setEditFocus(true);
    m_composerInterface->setDetailsOnlyMode(_detailsOnly);

    setTitle(m_composerInterface->contextTitle());
    m_widgetStack->setCurrentWidget(m_composerInterface);

    // Can't save as draft until there has been a change
    m_draftAction->setVisible(false);
    return true;
}

void WriteMail::setComposer( const QString &key )
{
    if (m_composerInterface && m_composerInterface->key() == key) {
        // We can reuse the existing composer object
        m_composerInterface->clear();
        return;
    }

    if (m_composerInterface) {
        // Remove any associated widgets
        m_widgetStack->removeWidget(m_composerInterface);
        delete m_composerInterface;
        m_composerInterface = 0;
    }

    m_composerInterface = QMailComposerFactory::create( key, this );
    if (!m_composerInterface)
        return;

    connect( m_composerInterface, SIGNAL(changed()), this, SLOT(messageModified()) );
    connect( m_composerInterface, SIGNAL(sendMessage()), this, SLOT(sendStage()));
    connect( m_composerInterface, SIGNAL(cancel()), this, SLOT(saveChangesOnRequest()));
    connect( m_composerInterface, SIGNAL(contextChanged()), this, SLOT(contextChanged()));

    m_widgetStack->addWidget(m_composerInterface);
}

void WriteMail::setTitle(const QString& title)
{
#ifdef QTOPIA_HOMEUI
    setWindowTitle(title);
#else
    m_mainWindow->setWindowTitle(title);
#endif
}

void WriteMail::emitFinished()
{
#ifdef QTOPIA_HOMEUI
    QDialog::reject();
#else
    emit finished();
#endif
}

QString WriteMail::composer() const
{
    QString key;
    if (m_composerInterface)
        key = m_composerInterface->key();
    return key;
}

void WriteMail::messageModified()
{
    hasMessageChanged = true;

    if ( m_composerInterface )
        m_draftAction->setVisible( hasContent() );
}

bool WriteMail::forcedClosure()
{
    if (draft())
        return true;

    discard();
    return false;
}

bool WriteMail::largeAttachments()
{
    //determine if the message attachments exceed our
    //limits

    quint64 totalAttachmentKB = 0;

    for(unsigned int i = 0; i < mail.partCount(); ++i)
    {
        const QMailMessagePart part = mail.partAt(i);
        if(!part.attachmentPath().isEmpty())
        {
            QFileInfo fi(part.attachmentPath());
            if(fi.exists())
                totalAttachmentKB += fi.size();
        }
    }

    totalAttachmentKB /= 1024;

    return (totalAttachmentKB > largeAttachmentsLimit());
}

uint WriteMail::largeAttachmentsLimit() const
{
    static uint limit = 0;

    if(limit == 0)
    {
        const QString key("emailattachlimitkb");
        QSettings mailconf("Trolltech","qtmail");

        mailconf.beginGroup("qtmailglobal");
        if (mailconf.contains(key))
            limit = mailconf.value(key).value<uint>();
        else
            limit = 2048; //default to 2MB
        mailconf.endGroup();
    }
    return limit;
}

QString WriteMail::signature() const
{
    if (m_composerInterface)
    {
        QMailAccount account = m_composerInterface->fromAccount();
        if (account.id().isValid()) {
            AccountConfiguration config(account.id());
            if (config.useSignature())
                return config.signature();
        }
    }
    return QString();
}

#ifdef QTOPIA_HOMEUI
void WriteMail::reject()
{
    QWidget *curPage = m_widgetStack->currentWidget();

    if(curPage != m_selectComposerWidget) {
        if (hasContent()) {
            saveChangesOnRequest();
        } else {
            discard();
        }
    }

    QDialog::reject();
}

void WriteMail::accept()
{
    QWidget *curPage = m_widgetStack->currentWidget();
    if (curPage == m_selectComposerWidget) {
        if(m_selectComposerWidget->availableTypes().isEmpty()) {
            discard();
        } else {
            QPair<QString, QMailMessage::MessageType> selection(m_selectComposerWidget->currentSelection());
            if (selection.first.isEmpty()) {
                QMessageBox::warning(qApp->activeWindow(),
                                    tr("No type selected"),
                                    tr("Please select a message type to compose."),
                                    QMessageBox::Ok);
            } else {
                composerSelected(m_selectComposerWidget->currentSelection());
            }
        }
    } else if(curPage == m_composerInterface) {
        if (sendStage()) {
            QDialog::accept();
        }
    }
}

void WriteMail::changeEvent(QEvent* e)
{
    if(e->type() == QEvent::ActivationChange)
        if(isActiveWindow())
            raise();
}
#endif
