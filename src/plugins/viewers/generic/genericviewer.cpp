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

#include "genericviewer.h"
#include "attachmentoptions.h"
#include "browser.h"

#include <qtopiaservices.h>

#include <QAction>
#include <QGridLayout>
#include <QKeyEvent>
#include <QMailMessage>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QtopiaApplication>

#ifdef QTOPIA_HOMEUI

#include <qtopialog.h>

class RecipientsDialog : public QDialog
{
    Q_OBJECT

public:
    RecipientsDialog(QWidget *parent = 0, QString str = QString::null)
        : QDialog(parent)
    {
        setContentsMargins(0, 0, 0, 0);
        setObjectName("HomeRecipientsDialog");
        QVBoxLayout *vbl = new QVBoxLayout(this);
        vbl->setMargin(0);
        vbl->setSpacing(0);
        QTextBrowser *browser = new QTextBrowser;
        QPalette pal = browser->palette();
        pal.setBrush(QPalette::Text, pal.base());
        pal.setBrush(QPalette::Base, palette().window());
        browser->setPalette(pal);
        browser->setFrameStyle(QFrame::NoFrame);
        browser->setText(str);
        vbl->addWidget(browser);
        QHBoxLayout *hbl = new QHBoxLayout;
        hbl->setMargin(0);
        hbl->setSpacing(0);
        hbl->addStretch(1);
        HomeActionButton *closeButton;
        closeButton = new HomeActionButton(tr("Close"), QtopiaHome::Red);
        connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
        hbl->addWidget(closeButton);
        vbl->addLayout(hbl);
        QtopiaHome::setPopupDialogStyle(this);
    }
};
#endif


GenericViewer::GenericViewer(QWidget* parent)
    : QMailViewerInterface(parent),
      browser(new Browser(parent)),
#ifdef QTOPIA_HOMEUI
      mainWidget(new QWidget(parent)),
      fromButton(new HomeContactButton(tr("From:"), sizer)),
      toButton(new HomeFieldButton(tr("To:"), sizer)),
      replyButton(new HomeActionButton(tr("Reply", "Reply to selected msg"), QtopiaHome::Green)),
      deleteButton(new HomeActionButton(tr("Delete", "Delete selected msg" ), QtopiaHome::Red)),
      backButton(new HomeActionButton(tr("Back"), mainWidget->palette().color(QPalette::Button), mainWidget->palette().color(QPalette::ButtonText))),
#endif
      message(0),
      plainTextMode(false),
      containsNumbers(false)
{
    connect(browser, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
    connect(browser, SIGNAL(highlighted(QUrl)), this, SLOT(linkHighlighted(QUrl)));

    plainTextModeAction = new QAction(QIcon(":icon/txt"), tr("Plain text"), this);
    plainTextModeAction->setVisible(!plainTextMode);
    plainTextModeAction->setWhatsThis(tr("Display the message contents in Plain text format."));

    richTextModeAction = new QAction(QIcon(":icon/txt"), tr("Rich text"), this);
    richTextModeAction->setVisible(plainTextMode);
    richTextModeAction->setWhatsThis(tr("Display the message contents in Rich text format."));

    printAction = new QAction(QIcon(":icon/print"), tr("Print"), this);
    printAction->setVisible(false);
    printAction->setWhatsThis(tr("Print the message contents."));

    dialAction = new QAction(this);
    dialAction->setVisible(false);

    messageAction = new QAction(this);
    messageAction->setVisible(false);

    storeAction = new QAction(this);
    storeAction->setVisible(false);

    contactAction = new QAction(this);
    contactAction->setVisible(false);

#ifdef QTOPIA_HOMEUI
    QSizePolicy expandingPolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSizePolicy minimumPolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    fromButton->setSizePolicy(expandingPolicy);
    connect(fromButton, SIGNAL(clicked()), this, SLOT(senderActivated()));

    toButton->setSizePolicy(expandingPolicy);
    connect(toButton, SIGNAL(clicked()), this, SLOT(recipientsActivated()));

    replyButton->setSizePolicy(minimumPolicy);
    connect(replyButton, SIGNAL(clicked()), this, SLOT(replyActivated()));

    deleteButton->setSizePolicy(minimumPolicy);
    connect(deleteButton, SIGNAL(clicked()), this, SIGNAL(deleteMessage()));

    backButton->setSizePolicy(minimumPolicy);
    connect(backButton, SIGNAL(clicked()), this, SIGNAL(finished()));

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->addWidget(fromButton, 0, 0);
    grid->addWidget(toButton, 1, 0, 1, 2);
    grid->addWidget(replyButton, 0, 1);
    grid->addWidget(backButton, 0, 2);
    grid->addWidget(deleteButton, 1, 2);

    QVBoxLayout* vb = new QVBoxLayout(mainWidget);
    vb->setSpacing(0);
    vb->setContentsMargins(0, 0, 0, 0);
    vb->addLayout(grid);
    vb->addWidget(browser);
#endif

    widget()->installEventFilter(this);
}

GenericViewer::~GenericViewer()
{
}

void GenericViewer::scrollToAnchor(const QString& a)
{
    browser->scrollToAnchor(a);
}

QWidget* GenericViewer::widget() const
{
#ifdef QTOPIA_HOMEUI
    return mainWidget;
#else
    return browser;
#endif
}

void GenericViewer::addActions(QMenu* menu) const
{
    // Ensure we don't receive menu events multiple times
    disconnect(menu, SIGNAL(triggered(QAction*)),
               this, SLOT(action(QAction*)));

    menu->addAction(plainTextModeAction);
    menu->addAction(richTextModeAction);
    menu->addAction(printAction);

    if (containsNumbers) {
        menu->addSeparator();
        menu->addAction(dialAction);
        menu->addAction(messageAction);
        menu->addAction(storeAction);
        menu->addAction(contactAction);
    }

    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(action(QAction*)));
}

#ifdef QTOPIA_HOMEUI
QString GenericViewer::prettyName(QMailAddress address)
{
    QString namePart, addressPart;

    if (address.matchesExistingContact()) {
        namePart = address.matchContact().label();
    } else if (address.name() != address.address()) {
        namePart = address.name();
    }

    addressPart = address.address();
    if (address.isChatAddress()) {
        // Just show the identifier
        addressPart = address.chatIdentifier();
    }
    
    return QMailAddress(namePart, addressPart).toString();
}

QString GenericViewer::recipients()
{
    QList<QMailAddress> recipients = message->to() + message->cc();
    QString recipientsStr = prettyName(recipients.takeFirst());
    foreach(QMailAddress address, recipients)
        recipientsStr += ", " + prettyName(address);
    return recipientsStr;
}
#endif

bool GenericViewer::setMessage(const QMailMessage& mail)
{
    message = &mail;

    setPlainTextMode(plainTextMode);
    printAction->setVisible(true);

    containsNumbers = !browser->embeddedNumbers().isEmpty() || mail.from().isPhoneNumber();
    dialAction->setVisible(false);
    messageAction->setVisible(false);
    storeAction->setVisible(false);
    contactAction->setVisible(false);

#ifdef QTOPIA_HOMEUI
    contact = message->from().matchContact();
    
    if (!contact.uid().isNull()) {
        fromButton->setValues(contact, contact.label());
    } else {
        fromButton->setValues(contact, message->from().displayName());
    }

    toButton->setField(recipients());
#endif

    return true;
}

void GenericViewer::setResource(const QUrl& name, QVariant var)
{
    browser->setResource(name, var);
}

void GenericViewer::clear()
{
    plainTextMode = false;
    contact = QContact();

    browser->setPlainText("");
    browser->clearResources();
}

void GenericViewer::action(QAction* action)
{
    if (action == plainTextModeAction) {
        setPlainTextMode(true);
    } else if (action == richTextModeAction) {
        setPlainTextMode(false);
    } else if (action == printAction) {
        print();
    } else if ((action == dialAction) ||
               (action == messageAction) ||
               (action == storeAction) ||
               (action == contactAction)) {
        emit anchorClicked(action->data().toString());
    }
}

void GenericViewer::setPlainTextMode(bool plainTextMode)
{
    this->plainTextMode = plainTextMode;

    browser->setMessage(*message, plainTextMode);

    plainTextModeAction->setVisible(!plainTextMode && message->messageType() != QMailMessage::Mms);
    richTextModeAction->setVisible(plainTextMode);
}

void GenericViewer::print() const
{
    QtopiaServiceRequest srv2( "Print", "printHtml(QString)" );
    srv2 << browser->toHtml();
    srv2.send();
}

void GenericViewer::linkClicked(const QUrl& link)
{
    QString command = link.toString();

    if (command.startsWith("attachment")) {
        QRegExp splitter("attachment;([^;]+)(?:;(\\d*))?");
        if (splitter.exactMatch(command)) {
            QString cmd = splitter.cap(1);
            QString number = splitter.cap(2);
            if (!number.isEmpty()) {
                uint partNumber = number.toUInt();

                // Show the attachment dialog
                AttachmentOptions options(widget());

                // TODO: either part-detaching is permitted on a const part, or we will
                // have to accept only non-const messages to the viewer...
                QMailMessagePart& part = const_cast<QMailMessagePart&>(message->partAt(partNumber));
                options.setAttachment(part);
                QtopiaApplication::execDialog(&options);
                return;
            }
        }
    } else if (command == "download") {
        emit completeMessage();
    }

    emit anchorClicked(link);
}

void GenericViewer::linkHighlighted(const QUrl& link)
{
    QString command = link.toString();

    QString number;
    if (command.startsWith("dial;")) {
        number = command.mid(5);
    } else if (command.startsWith("mailto:")) {
        QMailAddress address(command.mid(7));
        if (address.isPhoneNumber())
            number = address.address();
    }

    if (!number.isEmpty()) {
        dialAction->setText(tr("Dial %1", "%1=number").arg(number));
        dialAction->setData(QVariant(QString("dial;%1").arg(number)));
        dialAction->setVisible(true);

        messageAction->setText(tr("Message %1", "%1=number").arg(number));
        messageAction->setData(QVariant(QString("message;%1").arg(number)));
        messageAction->setVisible(true);

        QContact matching(QMailAddress(number).matchContact());
        if (!matching.uid().isNull()) {
            contactAction->setText(tr("View %1", "%1=contact label").arg(matching.label()));
            contactAction->setData(QVariant(QString("contact;%1").arg(matching.uid().toString())));
            contactAction->setVisible(true);
        } else {
            storeAction->setText(tr("Save %1", "%1=number").arg(number));
            storeAction->setData(QVariant(QString("store;%1").arg(number)));
            storeAction->setVisible(true);
        }
    } else {
        dialAction->setVisible(false);
        messageAction->setVisible(false);
        storeAction->setVisible(false);
        contactAction->setVisible(false);
    }
}

#ifdef QTOPIA_HOMEUI
void GenericViewer::replyActivated()
{
    emit replyToSender();
}

void GenericViewer::senderActivated()
{
    if (!contact.uid().isNull()) {
        emit contactDetails(contact);
    } else {
        emit saveSender();
    }
}

void GenericViewer::recipientsActivated()
{
    RecipientsDialog recipientsDialog(widget(), recipients());
    QtopiaApplication::execDialog(&recipientsDialog);
}
#endif

bool GenericViewer::eventFilter(QObject*, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        if (QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event)) {
            if (keyEvent->key() == Qt::Key_Back) {
                emit finished();
                return true;
            }
        }
    }

    return false;
}


QTOPIA_EXPORT_PLUGIN( GenericViewerPlugin )

GenericViewerPlugin::GenericViewerPlugin()
    : QMailViewerPlugin()
{
}

QString GenericViewerPlugin::key() const
{
    return "GenericViewer";
}

static QList<QMailMessage::ContentType> supportedTypes() 
{
    QList<QMailMessage::ContentType> types;

    types << QMailMessage::PlainTextContent
          << QMailMessage::RichTextContent
          << QMailMessage::ImageContent
          << QMailMessage::AudioContent
          << QMailMessage::VideoContent
          << QMailMessage::MultipartContent
#ifndef QTOPIA_HOMEUI
          << QMailMessage::VoicemailContent
#endif
          << QMailMessage::HtmlContent         // temporary...
          << QMailMessage::VCardContent        // temporary...
          << QMailMessage::VCalendarContent    // temporary...
          << QMailMessage::ICalendarContent;   // temporary...

    return types;
}

bool GenericViewerPlugin::isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const
{
    if ((pres != QMailViewerFactory::AnyPresentation) && (pres != QMailViewerFactory::StandardPresentation))
        return false;

    static QList<QMailMessage::ContentType> types(supportedTypes());
    return types.contains(type);
}

QMailViewerInterface *GenericViewerPlugin::create(QWidget *parent)
{
    return new GenericViewer(parent);
}

#ifdef QTOPIA_HOMEUI
#include "genericviewer.moc"
#endif
