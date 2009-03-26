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

#include "readmail.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <qsoftmenubar.h>
#include <qthumbnail.h>

#include <qcontact.h>
#include <qcontactmodel.h>

#include <qmailviewer.h>

#include <qlabel.h>
#include <qimage.h>
#include <qaction.h>
#include <qfile.h>
#include <qtextbrowser.h>
#include <qtextstream.h>

#include <qcursor.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qregexp.h>
#include <qstackedwidget.h>
#include <qmessagebox.h>
#include <qboxlayout.h>
#include <qevent.h>
#include <qimagereader.h>
#include <qalgorithms.h>
#include <qtmailwindow.h>
#include <QContactSelector>
#include <QContactFieldDefinition>
#include <QMailAccount>
#include <QMailComposerFactory>
#include <QMailFolder>
#include <QMailStore>
#include <QDrmContentPlugin>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QMenuBar>

#ifdef QTOPIA_HOMEUI
#include <private/homewidgets_p.h>
#endif


class SaveContactDialog : public QDialog
{
    Q_OBJECT

public:
    enum Selection { None = 0, Create, Existing };

    SaveContactDialog(const QMailAddress &address, QWidget *parent = 0)
        : QDialog(parent),
          sel(None),
#ifdef QTOPIA_HOMEUI
          createButton(new HomeActionButton(tr("Create new contact"), QtopiaHome::Green)),
          existingButton(new HomeActionButton(tr("Add to existing"), QtopiaHome::Green)),
          cancelButton(new HomeActionButton(tr("Cancel"), QtopiaHome::Red))
#else
          createButton(new QPushButton(tr("Create new contact"))),
          existingButton(new QPushButton(tr("Add to existing")))
#endif
    {
        setObjectName("SaveContactDialog");
        setModal(true);

        setWindowTitle(tr("Save"));

        connect(createButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
        connect(existingButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
#ifdef QTOPIA_HOMEUI
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
#endif

        QString text = tr("Saving '%1' to Contacts.", "%1=name/address/number").arg(address.address());
        text += "\n";
        text += tr("Create new contact or add to an existing contact?");

        QPlainTextEdit* textEdit = new QPlainTextEdit(text, this);
        textEdit->setReadOnly(true);
        textEdit->setFocusPolicy(Qt::NoFocus);
        textEdit->setFrameStyle(QFrame::NoFrame);
        textEdit->viewport()->setBackgroundRole(QPalette::Window);

        QVBoxLayout *vbl = new QVBoxLayout;
        vbl->setSpacing(4);
        vbl->setContentsMargins(0, 0, 0, 0);
        vbl->addWidget(textEdit);
#ifdef QTOPIA_HOMEUI
        QGridLayout* grid = new QGridLayout;
        grid->addWidget(createButton, 0, 0);
        grid->addWidget(existingButton, 0, 1);
        grid->addWidget(cancelButton, 1, 1);
        vbl->addLayout(grid);
#else
        vbl->addWidget(createButton);
        vbl->addWidget(existingButton);
#endif
        setLayout(vbl);

#ifdef QTOPIA_HOMEUI
        QtopiaHome::setPopupDialogStyle(this);
#endif
    }

    Selection selection() const { return sel; }

protected slots:
    void buttonClicked()
    {
        if (sender() == createButton) {
            sel = Create;
        } else if (sender() == existingButton) {
            sel = Existing;
        } else {
            reject();
            return;
        }

        accept();
    }

private:
    Selection sel;
#ifdef QTOPIA_HOMEUI
    HomeActionButton *createButton;
    HomeActionButton *existingButton;
    HomeActionButton *cancelButton;
#else
    QPushButton *createButton;
    QPushButton *existingButton;
#endif
};


ReadMail::ReadMail( QWidget* parent, Qt::WFlags fl )
    : QMainWindow(parent, fl),
      sending(false),
      receiving(false),
      firstRead(false),
      hasNext(false),
      hasPrevious(false),
      initialized(false),
      contactModel(0),
      modelUpdatePending(false)
{
    init();
}

ReadMail::~ReadMail()
{
    delete contactModel;
}

void ReadMail::init()
{
    QDrmContentPlugin::initialize();

    getThisMailButton = new QAction( QIcon(":icon/getmail"), tr("Get message"), this );
    connect(getThisMailButton, SIGNAL(triggered()), this, SLOT(getThisMail()) );
    getThisMailButton->setWhatsThis( tr("Retrieve this message from the server.  You can use this option to retrieve individual messages that would normally not be automatically downloaded.") );

    sendThisMailButton = new QAction( QIcon(":icon/sendmail"), tr("Send message"), this );
    connect(sendThisMailButton, SIGNAL(triggered()), this, SLOT(sendThisMail()));
    sendThisMailButton->setWhatsThis(  tr("Send this message.  This option will not send any other messages in your outbox.") );

    replyButton = new QAction( QIcon(":icon/reply"), tr("Reply"), this );
    connect(replyButton, SIGNAL(triggered()), this, SLOT(reply()));
    replyButton->setWhatsThis( tr("Reply to sender only.  Select Reply all from the menu if you want to reply to all recipients.") );

    replyAllAction = new QAction( QIcon(":icon/replytoall"), tr("Reply all"), this );
    connect(replyAllAction, SIGNAL(triggered()), this, SLOT(replyAll()));

    forwardAction = new QAction(tr("Forward"), this );
    connect(forwardAction, SIGNAL(triggered()), this, SLOT(forward()));

    modifyButton = new QAction( QIcon(":icon/edit"), tr("Modify"), this );
    connect(modifyButton, SIGNAL(triggered()), this, SLOT(modify()));
    modifyButton->setWhatsThis( tr("Opens this message in the composer so that you can make modifications to it.") );

    previousButton = new QAction( QIcon( ":icon/up" ), tr( "Previous" ), this );
    connect( previousButton, SIGNAL(triggered()), this, SIGNAL(viewPrevious()) );
    previousButton->setWhatsThis( tr("Read the previous message in the folder.") );

    nextButton = new QAction( QIcon( ":icon/down" ), tr( "Next" ), this );
    connect( nextButton, SIGNAL(triggered()), this, SIGNAL(viewNext()) );
    nextButton->setWhatsThis( tr("Read the next message in the folder.") );

    attachmentsButton = new QAction( QIcon( ":icon/attach" ), tr( "Attachments" ), this );
    connect( attachmentsButton, SIGNAL(triggered()), this,
            SLOT(viewAttachments()) );
    attachmentsButton->setWhatsThis( tr("View the attachments in the message.") );

    deleteButton = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
    connect( deleteButton, SIGNAL(triggered()), this, SLOT(deleteItem()) );
    deleteButton->setWhatsThis( tr("Move this message to the trash folder.  If the message is already in the trash folder it will be deleted. ") );

    storeButton = new QAction( QIcon( ":icon/save" ), tr( "Save Sender" ), this );
    connect( storeButton, SIGNAL(triggered()), this, SLOT(storeContact()) );
    
    views = new QStackedWidget(this);
    setCentralWidget(views);

    // Update the view if the displayed message changes
    connect(QMailStore::instance(), SIGNAL(messagesUpdated(QMailMessageIdList)), 
            this, SLOT(messagesUpdated(QMailMessageIdList)));
}

QMailMessageId ReadMail::displayedMessage() const 
{
    return mail.id();
}

bool ReadMail::handleIncomingMessages(const QMailMessageIdList &list) const
{
    if (QMailViewerInterface* viewer = currentViewer()) {
        if (viewer->handleIncomingMessages(list)) {
            // Mark each of these messages as read
            quint64 status(QMailMessage::Read);
            QMailStore::instance()->updateMessagesMetaData(QMailMessageKey(list), status, true);

            return true;
        }
    }

    return false;
}

bool ReadMail::handleOutgoingMessages(const QMailMessageIdList &list) const
{
    if (QMailViewerInterface* viewer = currentViewer())
        return viewer->handleOutgoingMessages(list);

    return false;
}

/*  We need to be careful here. Don't allow clicking on any links
    to automatically install anything.  If we want that, we need to
    make sure that the mail doesn't contain mailicious link encoding
*/
void ReadMail::linkClicked(const QUrl &lnk)
{
    QString str = lnk.toString();
    QRegExp commandPattern("(\\w+);(.+)");

    if (commandPattern.exactMatch(str)) {
        QString command = commandPattern.cap(1);
        QString param = commandPattern.cap(2);

        if (command == "attachment") {
            if (param == "view") { // No tr
                viewAttachments();
            } else if (param.startsWith("scrollto;")) {
                if (QMailViewerInterface* viewer = currentViewer())
                    viewer->scrollToAnchor(param.mid(9));
            } else if (param == "play") {
                if (isMms)
                    viewMms();
            }
        } else if (command == "dial") {
            dialNumber(param);
        } else if (command == "message") {
            emit sendMessageTo(QMailAddress(param), QMailMessage::Sms);
        } else if (command == "store") {
            storeContact(QMailAddress(param), mail.messageType());
        } else if (command == "contact") {
            displayContact(QUniqueId(param));
        }
    } else if (str.startsWith("mailto:")) {
        // strip leading 'mailto:'
        emit sendMessageTo( QMailAddress(str.mid(7)), mail.messageType() );
    } else if (str.startsWith("http://")) {
        QtopiaServiceRequest e( "WebAccess", "openURL(QString)" );
        e << str;
        e.send();
    } else if (mail.messageType() == QMailMessage::System && str.startsWith(QLatin1String("qtopiaservice:"))) {
        int commandPos  = str.indexOf( QLatin1String( "::" ) ) + 2;
        int argPos      = str.indexOf( '?' ) + 1;
        QString service = str.mid( 14, commandPos - 16 );
        QString command;
        QStringList args;

        if (argPos > 0) {
            command = str.mid( commandPos, argPos - commandPos - 1 );
            args    = str.mid( argPos ).split( ',' );
        } else {
            command = str.mid( commandPos );
        }

        QtopiaServiceRequest e( service, command );
        foreach( const QString &arg, args )
            e << arg;
        e.send();
    }
}

QString ReadMail::displayName(const QMailMessage& mail)
{
    const bool outgoing(mail.status() & QMailMessage::Outgoing);

    QString name;
    if (outgoing) {
        if (!mail.to().isEmpty())
            name = mail.to().first().displayName();
    } else {
        name = mail.from().displayName();
    }

    if (name.isEmpty()) {
        // Use the type of this message as the title
        QString key(QMailComposerFactory::defaultKey(mail.messageType()));
        if (!key.isEmpty())
            name = QMailComposerFactory::displayName(key, mail.messageType());
        else 
            name = tr("Message");

        if (!name.isEmpty())
            name[0] = name[0].toUpper();
    }

    if (outgoing)
        name.prepend(tr("To:") + ' ');

    return name;
}

void ReadMail::updateView(QMailViewerFactory::PresentationType type)
{
    if ( !mail.id().isValid() )
        return;

    if (type == QMailViewerFactory::AnyPresentation) {
        type = QMailViewerFactory::StandardPresentation;
        if (mail.messageType() == QMailMessage::Instant) {
            type = QMailViewerFactory::ConversationPresentation;
        }
    }

    QMailMessage::ContentType content(mail.content());
    if ((content == QMailMessage::NoContent) || (content == QMailMessage::UnknownContent)) {
        // Attempt to show the message as text, from the subject if necessary
        content = QMailMessage::PlainTextContent;
    }

    QMailViewerInterface* view = viewer(content, type);
    if (!view) {
        qLog(Messaging) << "Unable to view message" << mail.id() << "with content:" << content;
        return;
    }

    // Mark message as read before showing viewer, to avoid reload on change notification
    updateReadStatus();

    view->clear();

    if (!isSmil && (mail.messageType() != QMailMessage::System)) {
        initImages(view);
    }

    view->setMessage(mail);
    context = QSoftMenuBar::menuFor(view->widget());
    context->clear();
    context->addAction(getThisMailButton);
    context->addAction(sendThisMailButton);
#ifndef QTOPIA_HOMEUI
    context->addAction(replyButton);
    context->addAction(replyAllAction);
    context->addAction(forwardAction);
#endif
    context->addAction(modifyButton);
#ifndef QTOPIA_HOMEUI
    context->addAction(deleteButton);
#endif
    context->addAction(storeButton);

    context->addSeparator();
    view->addActions(context);

#ifndef QTOPIA_NO_MMS
    if(mail.messageType() == QMailMessage::Mms)
        QtopiaApplication::setPowerConstraint(QtopiaApplication::Disable);
#endif

    switchView(view, displayName(mail));
}

void ReadMail::keyPressEvent(QKeyEvent *e)
{
    switch( e->key() ) {
        case Qt::Key_A:
            if ( attachmentsButton->isEnabled() )
                viewAttachments();
            break;
        case Qt::Key_P:
            if ( previousButton->isEnabled() )
                emit viewPrevious();
            break;
        case Qt::Key_N:
            if ( nextButton->isEnabled() )
                emit viewNext();
            break;
        case Qt::Key_Delete:
            deleteItem();
            break;
        case Qt::Key_R:
            reply();
            break;
        case Qt::Key_F:
            forward();
            break;
        case Qt::Key_E:
            if ( modifyButton->isEnabled() )
                modify();
        default:
            QMainWindow::keyPressEvent( e );
    }
}

//update view with current EmailListItem (item)
void ReadMail::displayMessage(const QMailMessageId& id, QMailViewerFactory::PresentationType type, bool nextAvailable, bool previousAvailable)
{
    if (!id.isValid())
        return;
       
    hasNext = nextAvailable;
    hasPrevious = previousAvailable;

    showMessage(id, type);

    updateButtons();

    //report currently viewed mail so that it will be
    //placed first in the queue of new mails to download.
    emit viewingMail(mail);
}

void ReadMail::buildMenu(const QString &mailbox)
{
    Q_UNUSED(mailbox);
}

void ReadMail::messagesUpdated(const QMailMessageIdList& list)
{
    if (!mail.id().isValid())
        return;

    if (list.contains(mail.id())) {
        loadMessage(mail.id());
        if (QMailViewerInterface* viewer = currentViewer())
            viewer->setMessage(mail);
        return;
    }
    
    updateButtons();
}

void ReadMail::showMessage(const QMailMessageId& id, QMailViewerFactory::PresentationType type)
{
    loadMessage(id);

    updateView(type);
}

void ReadMail::messageChanged(const QMailMessageId &id)
{
    // Ignore updates from viewers that aren't currently top of the stack
    if (sender() == static_cast<QObject*>(currentViewer())) {
        loadMessage(id);
    }
}

void ReadMail::loadMessage(const QMailMessageId &id)
{
    mail = QMailMessage(id);

    isMms = false;
    isSmil = false;

#ifndef QTOPIA_NO_MMS
    if (mail.messageType() == QMailMessage::Mms) {
        QString mmsType = mail.headerFieldText("X-Mms-Message-Type");
        if (mmsType.contains("m-retrieve-conf") || mmsType.contains("m-send-req")) {
            isMms = true;
            if (mail.contentType().content().toLower() == "multipart/related")
                isSmil = true;
        }
    }
#endif

    updateButtons();
}

void ReadMail::viewFinished()
{
    //check for read reply flag
#ifndef QTOPIA_NO_MMS

    if(mail.messageType() == QMailMessage::Mms)
        QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    QString mmsType = mail.headerFieldText("X-Mms-Message-Type");
    QString msgClass = mail.headerFieldText("X-Mms-Message-Class");
    QString readReply = mail.headerFieldText("X-Mms-Read-Reply");

    if (mmsType.contains("m-retrieve-conf") && 
        !msgClass.contains("Auto") && 
        readReply.contains("Yes") && 
        firstRead) {
        emit readReplyRequested(mail);
    }
#endif

    closeView();
}

void ReadMail::closeView()
{
    if (currentViewer()) {
        if ((mail.messageType() == QMailMessage::Sms) && (mail.headerFieldText("X-Sms-Class") == "0")) {
            // This is a 'flash' message just displayed - we should delete it after viewing
            QMailMessageId deleteId(mail.id());

            mail = QMailMessage();
            emit removeMessage(deleteId, false);
        }

        currentView.pop();
        if (currentView.isEmpty()) {
            // Clear the current message record
            mail = QMailMessage();
            emit cancelView();
        } else {
            QTMailWindow::singleton()->setWindowTitle(currentView.top().second);
            views->setCurrentWidget(currentView.top().first->widget());
        }
    }
}

//deletes item, tries bringing up next or previous, exits if unsucessful
void ReadMail::deleteItem()
{
    if (deleteButton->isVisible()) {
        QMailMessageId deleteId(mail.id());

        if (deleteId.isValid()) {
            // After deletion, we're finished viewing
            viewFinished();

            // Clear the current message, otherwise we will think it is updated by the deletion event
            mail = QMailMessage();

            emit removeMessage(deleteId, true);
        }
    }
}

void ReadMail::updateButtons()
{
    static const QMailFolder trashFolder(QMailFolder::TrashFolder);
    static const QMailFolder draftsFolder(QMailFolder::DraftsFolder);

    if (!mail.id().isValid())
        return;

    bool incoming(mail.status() & QMailMessage::Incoming);
    bool sent(mail.status() & QMailMessage::Sent);
    bool outgoing(mail.status() & QMailMessage::Outgoing);
    bool downloaded(mail.status() & QMailMessage::Downloaded);
    bool removed(mail.status() & QMailMessage::Removed);
    bool system(mail.messageType() == QMailMessage::System);
    bool messageSent(sent || sending);
    bool messageReceived(downloaded || receiving);

    sendThisMailButton->setVisible( !messageSent && outgoing && mail.hasRecipients() );
    modifyButton->setVisible( !messageSent && outgoing && (mail.parentFolderId() == draftsFolder.id()) );

    getThisMailButton->setVisible( !messageReceived && !removed && incoming );

    if (!downloaded || system) {
        // We can't really forward/reply/reply-to-all without the message content
        replyButton->setVisible(false);
        replyAllAction->setVisible(false);
        forwardAction->setVisible(false);
    } else {
        bool otherReplyTarget(!mail.cc().isEmpty() || mail.to().count() > 1);

        // TODO: handle cases where: a) a Mail-Followup-To is specified, and 
        // b) a singular To address is not our own address, and is probably a mailing list...

        replyButton->setVisible(incoming);
        replyAllAction->setVisible(incoming && otherReplyTarget);

        forwardAction->setVisible(true);
    }

    attachmentsButton->setVisible( mail.partCount() );

    nextButton->setVisible(hasNext);
    previousButton->setVisible(hasPrevious);

    deleteButton->setText( mail.parentFolderId() == trashFolder.id() ? tr("Delete") : tr("Move to Trash") );

    // Show the 'Save Sender' action if we don't have a matching contact
    QMailAddress fromAddress(mail.from());
    bool unknownContact = !fromAddress.matchesExistingContact();
    storeButton->setVisible(!fromAddress.isNull() && unknownContact);
}

void ReadMail::viewAttachments()
{
    ViewAtt dlg(&mail, (mail.status() & QMailMessage::Incoming));
    QtopiaApplication::execDialog(&dlg);
    QMailStore::instance()->updateMessage(&mail);
}

void ReadMail::viewMms()
{
#ifndef QTOPIA_NO_MMS
    QMailViewerInterface* smilView = viewer(QMailMessage::SmilContent);
    smilView->setObjectName( "smilView" );

    if (smilView->setMessage(mail)) {
        updateReadStatus();
        switchView(smilView, tr("MMS"));
    } else {
        QMessageBox::warning(this, tr("Cannot view MMS"),
            tr("<qt>Cannot play improperly formatted MMS.</qt>"), QMessageBox::Ok, QMessageBox::NoButton);
    }
#endif
}

void ReadMail::reply()
{
#ifdef QTOPIA_HOMEUI
    int replyType = 0;
    if (mail.messageType() == QMailMessage::Instant) {
        // Neither Reply-to-all nor Forward are sensible options
        replyType = Reply;

        if (mail.status() & QMailMessage::Outgoing) {
            // Although we're technically replying to a message we sent, we should
            // interpret this as a reply to the recipient of the message
            emit sendMessageTo(mail.to().first(), mail.messageType());
            return;
        }
    }

    emit resendRequested(mail, replyType);
#else
    if (replyButton->isVisible())
    {
        emit cancelView();
        emit resendRequested(mail, Reply);
        mail = QMailMessage();
    }
#endif    
}

void ReadMail::replyAll()
{
    if (replyAllAction->isVisible())
    {
        emit cancelView();
        emit resendRequested(mail, ReplyToAll);
        mail = QMailMessage();
    }
}

void ReadMail::forward()
{
    if (forwardAction->isVisible())
    {
        emit cancelView();
        emit resendRequested(mail, Forward);
        mail = QMailMessage();
    }
}

void ReadMail::setStatus(int id)
{
    quint64 prevStatus(mail.status());
    quint64 newStatus(prevStatus);

    switch( id ) {
        case 1:
            newStatus &= ~( QMailMessage::Replied | QMailMessage::RepliedAll | QMailMessage::Forwarded | QMailMessage::Read );
            break;

        case 2:
            newStatus &= ~( QMailMessage::RepliedAll | QMailMessage::Forwarded );
            newStatus |= QMailMessage::Replied;
            break;

        case 3:
            newStatus &= ~( QMailMessage::Replied | QMailMessage::RepliedAll );
            newStatus |= QMailMessage::Forwarded;
            break;

        case 4: 
            newStatus |= QMailMessage::Sent;
            break;

        case 5: 
            newStatus &= ~QMailMessage::Sent;
            break;
    }

    if ( newStatus != prevStatus) {
        mail.setStatus( newStatus );
        QMailStore::instance()->updateMessage(&mail);
    }

    updateButtons();
}

void ReadMail::modify()
{
    if (modifyButton->isVisible())
        emit modifyRequested(mail);
}

void ReadMail::getThisMail()
{
    if (getThisMailButton->isVisible())
        emit getMailRequested(mail);
}

void ReadMail::sendThisMail()
{
    if (sendThisMailButton->isVisible())
        emit sendMailRequested(mail);
}

void ReadMail::setSendingInProgress(bool on)
{
    sending = on;
    if ( isVisible() )
        updateButtons();
}

void ReadMail::setRetrievalInProgress(bool on)
{
    receiving = on;
    if ( isVisible() )
        updateButtons();
}

void ReadMail::initImages(QMailViewerInterface* view)
{
    static QMap<QUrl, QVariant> resourceMap;

    if (!initialized) {
    // Add the predefined smiley images for EMS messages.
        resourceMap.insert( QUrl( "x-sms-predefined:ironic" ),
                            QVariant( QImage( ":image/smiley/ironic" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:glad" ),
                            QVariant( QImage( ":image/smiley/glad" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:skeptical" ),
                            QVariant( QImage( ":image/smiley/skeptical" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:sad" ),
                            QVariant( QImage( ":image/smiley/sad" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:wow" ),
                            QVariant( QImage( ":image/smiley/wow" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:crying" ),
                            QVariant( QImage( ":image/smiley/crying" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:winking" ),
                            QVariant( QImage( ":image/smiley/winking" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:laughing" ),
                            QVariant( QImage( ":image/smiley/laughing" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:indifferent" ),
                            QVariant( QImage( ":image/smiley/indifferent" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:kissing" ),
                            QVariant( QImage( ":image/smiley/kissing" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:confused" ),
                            QVariant( QImage( ":image/smiley/confused" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:tongue" ),
                            QVariant( QImage( ":image/smiley/tongue" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:angry" ),
                            QVariant( QImage( ":image/smiley/angry" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:glasses" ),
                            QVariant( QImage( ":image/smiley/glasses" ) ) );
        resourceMap.insert( QUrl( "x-sms-predefined:devil" ),
                            QVariant( QImage( ":image/smiley/devil" ) ) );
        initialized = true;
    }

    QMap<QUrl, QVariant>::iterator it = resourceMap.begin(), end = resourceMap.end();
    for ( ; it != end; ++it)
        view->setResource( it.key(), it.value() );
}

void ReadMail::dialNumber(const QString& number)
{
    if ( !number.isEmpty() ) {
        QtopiaServiceRequest req( "Dialer", "showDialer(QString)" );
        req << number;
        req.send();
    }
}

void ReadMail::switchView(QMailViewerInterface* viewer, const QString& title)
{
    if (currentViewer() == viewer)
        currentView.pop();

    lastTitle = title;
    updateWindowTitle();

    views->setCurrentWidget(viewer->widget());

    currentView.push(qMakePair(viewer, title));
}

void ReadMail::updateWindowTitle() const
{
    QTMailWindow::singleton()->setWindowTitle(lastTitle);
}

QMailViewerInterface* ReadMail::currentViewer() const 
{
    if (currentView.isEmpty())
        return 0;

    return currentView.top().first;
}

static void addContactAddress(QContact &contact, QMailMessage::MessageType type, const QString &address)
{
    if (type == QMailMessage::Instant) {
        // Treat this address as a jabber URI
        QStringList chatFields = QContactFieldDefinition::fields("chat");
        if (!chatFields.isEmpty()) {
            // Just use the first type; if there are multiple ones, we need to prevail 
            // upon PIM to provide services that delegate this stuff to addressbook...
            QContactFieldDefinition field(chatFields.first());
            field.setValue(contact, field.id() + ':' + address);
        }
    } else {
        contact.insertEmail(address);
    }
}

void ReadMail::storeContact(const QMailAddress &address, QMailMessage::MessageType type)
{
    if (!address.isPhoneNumber() && !address.isEmailAddress()) {
        qWarning() << "Unable to store unknown address type:" << address.toString();
    } else {
        SaveContactDialog dialog(address);

        if ((QtopiaApplication::execDialog(&dialog) == QDialog::Accepted) &&
            (dialog.selection() != SaveContactDialog::None)) {
            bool newContact = (dialog.selection() == SaveContactDialog::Create);

            if (!contactModel) {
                // Once we have registered the new contact, we need to update our message display
                contactModel = new QContactModel();
                connect(contactModel, SIGNAL(modelReset()), this, SLOT(contactModelReset()) );
            }

            modelUpdatePending = true;

            if (address.isPhoneNumber()) {
                QtopiaServiceRequest req( "Contacts", (newContact ? "createNewContact(QString)"
                                                                  : "addPhoneNumberToContact(QString)") );
                req << address.toString();
                req.send();
            } else {
                // The Contacts app doesn't provide email address services at this time
                if (newContact) {
                    QContact contact;
                    addContactAddress(contact, type, address.address());

                    QtopiaServiceRequest req( "Contacts", "addAndEditContact(QContact)" );
                    req << contact;
                    req.send();
                } else {
                    // For now, we need to do this ourselves
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

                    selector.showMaximized();
                    if (QtopiaApplication::execDialog(&selector) == QDialog::Accepted) {
                        QContact contact(selector.selectedContact());
                        addContactAddress(contact, type, address.address());

                        if (!model.updateContact(contact)) {
                            qWarning() << "Unable to update contact:" << contact.label();
                        }
                    }
                }
            }
        }
    }
}

void ReadMail::storeContact()
{
    if (storeButton->isVisible()) {
        // Store the address of the correspondent
        if (mail.status() & QMailMessage::Incoming)
            storeContact(mail.from(), mail.messageType());
        else
            storeContact(mail.to().first(), mail.messageType());
    }
}

void ReadMail::contactModelReset()
{
    if (modelUpdatePending) {
        // TODO: In fact, we can't ignore unrequested reset events, because the first reset after 
        // our update may not include the change we requested...
        //modelUpdatePending = false;

        if (QMailViewerInterface *view = currentViewer()) {
            // Update the view to reflect any changes in details from the contact model
            view->setMessage(mail);

            if (view->objectName() != "smilView") {
                // Update the window title also
                QTMailWindow::singleton()->setWindowTitle(displayName(mail));
            }
        }

        updateButtons();
    }
}

QMailViewerInterface* ReadMail::viewer(QMailMessage::ContentType content, QMailViewerFactory::PresentationType type)
{
    ViewerMap::iterator it = contentViews.find(qMakePair(content, type));
    if (it == contentViews.end()) {
        QString key = QMailViewerFactory::defaultKey(content, type);
        if (key.isEmpty())
            return 0;

        QMailViewerInterface* view = QMailViewerFactory::create(key, views);

        view->setObjectName("read-message");
        view->widget()->setWhatsThis(tr("This view displays the contents of the message."));

        connect(view, SIGNAL(replyToSender()), replyButton, SLOT(trigger()));
        connect(view, SIGNAL(replyToAll()), replyAllAction, SLOT(trigger()));
        connect(view, SIGNAL(completeMessage()), getThisMailButton, SLOT(trigger()));
        connect(view, SIGNAL(forwardMessage()), forwardAction, SLOT(trigger()));
        connect(view, SIGNAL(deleteMessage()), deleteButton, SLOT(trigger()));
        connect(view, SIGNAL(saveSender()), storeButton, SLOT(trigger()));
        connect(view, SIGNAL(contactDetails(QContact)), this, SLOT(displayContact(QContact)));
        connect(view, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
        connect(view, SIGNAL(messageChanged(QMailMessageId)), this, SLOT(messageChanged(QMailMessageId)));
        connect(view, SIGNAL(viewMessage(QMailMessageId,QMailViewerFactory::PresentationType)), this, SIGNAL(viewMessage(QMailMessageId,QMailViewerFactory::PresentationType)));
        connect(view, SIGNAL(sendMessage(QMailMessage)), this, SIGNAL(sendMessage(QMailMessage)));

        // Note: this connection must be queued to prevent spurious emission of close events:
        connect(view, SIGNAL(finished()), this, SLOT(viewFinished()), Qt::QueuedConnection);

        QWidget* viewWidget = view->widget();
        viewWidget->setGeometry(geometry());
        views->addWidget(viewWidget);

        QSoftMenuBar::clearLabel(viewWidget, QSoftMenuBar::menuKey());
        QSoftMenuBar::clearLabel(viewWidget, Qt::Key_Select);

        it = contentViews.insert(qMakePair(content, type), view);
    }

    return it.value();
}

void ReadMail::displayContact(const QContact &contact)
{
    displayContact(contact.uid());
}

void ReadMail::displayContact(const QUniqueId &uid)
{
    QtopiaServiceRequest req("Contacts", "showContact(QUniqueId)");
    req << uid;
    req.send();
}

void ReadMail::updateReadStatus()
{
    if (mail.id().isValid()) {
        bool newMessage(mail.status() & QMailMessage::New);

        // This message is no longer new
        if (newMessage)
            mail.setStatus(QMailMessage::New, false);

        // Do not mark as read unless it has been downloaded
        if (mail.status() & QMailMessage::Downloaded) {
            firstRead = !(mail.status() & QMailMessage::Read);
            if (firstRead)
                mail.setStatus(QMailMessage::Read, true);
        }

        if (newMessage || firstRead) {
            QMailStore::instance()->updateMessage(&mail);
        }
    }
}

#include "readmail.moc"
