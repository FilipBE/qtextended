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

#include "qtmailwindow.h"
#include "statusdisplay.h"

#include "emailservice.h"
#ifndef QTOPIA_NO_SMS
#include "smsservice.h"
#endif
#ifndef QTOPIA_NO_COLLECTIVE
#include "instantmessageservice.h"
#endif
#include "messagesservice.h"

#include <qtopiaipcadaptor.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <QDebug>
#include <QStackedWidget>
#include <QMailAccount>


QTMailWindow *QTMailWindow::self = 0;

QTMailWindow::QTMailWindow(QWidget *parent, Qt::WFlags fl)
    : QWidget(parent, fl), noShow(false)
{
    qLog(Messaging) << "QTMailWindow ctor begin";

    QtopiaApplication::loadTranslations("libqtopiamail");
    init();
}

void QTMailWindow::init()
{
    self = this;

    views = new QStackedWidget;
    views->setFrameStyle(QFrame::NoFrame);

    status = new StatusDisplay;

    QVBoxLayout* vboxLayout = new QVBoxLayout(this);
    vboxLayout->setContentsMargins( 0, 0, 0, 0 );
    vboxLayout->setSpacing( 0 );
    vboxLayout->addWidget( views );
    vboxLayout->addWidget( status );

    // Add the email client to our central widget stack
    emailClient = new EmailClient(views);

    // Connect the email client as the primary interface widget
    connect(emailClient, SIGNAL(raiseWidget(QWidget*,QString)),
            this, SLOT(raiseWidget(QWidget*,QString)) );
    connect(emailClient, SIGNAL(statusVisible(bool)),
            status, SLOT(showStatus(bool)) );
    connect(emailClient, SIGNAL(updateStatus(QString)),
            status, SLOT(displayStatus(QString)) );
    connect(emailClient, SIGNAL(updateProgress(uint,uint)),
            status, SLOT(displayProgress(uint,uint)) );
    connect(emailClient, SIGNAL(clearStatus()),
            status, SLOT(clearStatus()) );

    // Hook up the QCop service handlers
    QtopiaAbstractService* svc;

    svc = new EmailService( this );
    connect(svc, SIGNAL(viewInbox()), 
            emailClient, SLOT(viewEmails()));
    connect(svc, SIGNAL(message(QMailMessageId)), 
            emailClient, SLOT(displayMessage(QMailMessageId)));
    connect(svc, SIGNAL(write(QString,QString)), 
            emailClient, SLOT(writeMailAction(QString,QString)));
    connect(svc, SIGNAL(write(QString,QString,QStringList,QStringList)), 
            emailClient, SLOT(writeMailAction(QString,QString,QStringList,QStringList)));
    connect(svc, SIGNAL(vcard(QString,QString)), 
            emailClient, SLOT(emailVCard(QString,QString)));
    connect(svc, SIGNAL(vcard(QDSActionRequest)), 
            emailClient, SLOT(emailVCard(QDSActionRequest)));
    connect(svc, SIGNAL(cleanup(QDate,int)), 
            emailClient, SLOT(cleanupMessages(QDate,int)));

#ifndef QTOPIA_NO_SMS
    svc = new SMSService( this );
    connect(svc, SIGNAL(newMessages(bool)), 
            emailClient, SLOT(newMessages(bool)));
    connect(svc, SIGNAL(viewInbox()), 
            emailClient, SLOT(viewMessages()));
    connect(svc, SIGNAL(write(QString,QString,QString)), 
            emailClient, SLOT(writeSms(QString,QString,QString)));
    connect(svc, SIGNAL(vcard(QString,QString)), 
            emailClient, SLOT(smsVCard(QString,QString)));
    connect(svc, SIGNAL(vcard(QDSActionRequest)), 
            emailClient, SLOT(smsVCard(QDSActionRequest)));
#endif

#ifndef QTOPIA_NO_COLLECTIVE
    svc = new InstantMessageService(this);
    connect(svc, SIGNAL(write(QString)), 
            emailClient, SLOT(writeInstantMessage(QString)));
#endif

    svc = new MessagesService( this );
    connect(svc, SIGNAL(view()), 
            emailClient, SLOT(viewMessages()));
    connect(svc, SIGNAL(viewNew(bool)), 
            emailClient, SLOT(newMessages(bool)));
    connect(svc, SIGNAL(view(QMailMessageId)), 
            emailClient, SLOT(displayMessage(QMailMessageId)));
    connect(svc, SIGNAL(replyTo(QMailMessageId)), 
            emailClient, SLOT(replyToMessage(QMailMessageId)));
    connect(svc, SIGNAL(compose(QMailMessage::MessageType, 
                                const QMailAddressList&, 
                                const QString&, 
                                const QString&, 
                                const QContentList&, 
                                QMailMessage::AttachmentsAction)), 
            emailClient, SLOT(composeMessage(QMailMessage::MessageType, 
                                             const QMailAddressList&, 
                                             const QString&, 
                                             const QString&, 
                                             const QContentList&, 
                                             QMailMessage::AttachmentsAction)));
    connect(svc, SIGNAL(compose(QMailMessage)), 
            emailClient, SLOT(composeMessage(QMailMessage)));

    views->addWidget(emailClient);
    views->setCurrentWidget(emailClient);

    setWindowTitle( emailClient->windowTitle() );
}

QTMailWindow::~QTMailWindow()
{
    qLog(Messaging) << "QTMailWindow dtor end";
}

void QTMailWindow::closeEvent(QCloseEvent *e)
{
    if (emailClient->closeImmediately()) {
        e->accept();
    } else {
        e->ignore();
    }
}

void QTMailWindow::showEvent(QShowEvent *e)
{
    emailClient->raiseApplication();

    QWidget::showEvent(e);
}

void QTMailWindow::forceHidden(bool hidden)
{
    noShow = hidden;
}

void QTMailWindow::setVisible(bool visible)
{
    if (noShow && visible)
        return;

    QWidget::setVisible(visible);
}

void QTMailWindow::setDocument(const QString &address)
{
    emailClient->sendMessageTo(QMailAddress(address), QMailMessage::AnyType);
}

void QTMailWindow::raiseWidget(QWidget *w, const QString &caption)
{
    if (!isVisible())
        showMaximized();

    views->setCurrentWidget(w);
    if (!caption.isEmpty())
        setWindowTitle( caption );

    raise();
    activateWindow();

    // needed to work with context-help
    setObjectName( w->objectName() );
}

QWidget* QTMailWindow::currentWidget() const
{
    return views->currentWidget();
}

QTMailWindow* QTMailWindow::singleton()
{
    return self;
}

#include "qtmailwindow.moc"

