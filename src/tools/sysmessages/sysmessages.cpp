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

#include "sysmessages.h"
#include <qmessagebox.h>
#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>

/*!
    \service SysMessagesService SystemMessages
    \inpublicgroup QtPkgManagementModule
    \brief The SysMessagesService class provides the SystemMessages service.

    The \i SystemMessages service enables the sending of
    system related messages as both warning dialogs and as synthetic messages which will end up in the users inbox.
    Client applications can request the \i SystemMessages service
    to send a message with the following code:
    \code
        QtopiaServiceRequest req( "SystemMessages", "QString, QString" );
        req << subject;
        req << text;
        req.send();
    \endcode
*/
/**
   Implementation Notes:
   The SystemMessage service will immediately display any dialogs, but
   will queue up any system sms messages (but still provide immediate
   notification that mail has arrived)

   Qtmail, upon startup, should request and subsequently receive
   these queued up messages and send back acknowledgements; at which
   point the internally queued up messages may be deleted.
*/

/*!
 Constructs the service as a child of \a parent
 */
SysMessagesService::SysMessagesService( QObject *parent )
    : QtopiaAbstractService( "SystemMessages", parent )
{
#ifdef MAIL_EXISTS
    keepRunning = false;
    maxMessageId = 0;
    sysMessagesChannel = new QtopiaChannel( "QPE/SysMessages", this);
    connect( sysMessagesChannel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(received(QString,QByteArray)));
#endif
    publishAll();
}

/*!
  \internal
  Destroys the service
*/
SysMessagesService::~SysMessagesService()
{
#ifdef MAIL_EXISTS
    if ( sysMessagesChannel )
        delete sysMessagesChannel;
#endif
}

/*!
  Displays a dialog to the user with the specified \a title and \a text
*/
void SysMessagesService::showDialog( const QString &title, const QString &text )
{

    QMessageBox::warning( 0, title, text );

#ifdef MAIL_EXISTS
    //closing message box unregisters the running task so re-register the task
    //if it is to be kept running
    if ( keepRunning )
       ( qobject_cast<QtopiaApplication *>(qApp) )->registerRunningTask( "QtopiaSysMessages", this );
#endif
}
#ifdef MAIL_EXISTS
/*!
  Attempts to pass on the queued system message corresponding to the specified
  \a messageId to qtmail.
  If qtmail is running, it should receive the message and send back an acknowledgement.
  If qtmail is not running, nothing should happen.
*/
void SysMessagesService::passOnMessage( int messageId )
{
    SysMail sysMail = (SysMail)sysMailMap.value( messageId );
    QtopiaIpcEnvelope e ( "QPE/SysMessages", "postMessage(int,QDateTime,QString,QString)" );
    e << messageId;
    e << sysMail.time();
    e << sysMail.subject();
    e << sysMail.text();
}

/*!
  Creates synthetic system message with the specified \a subject and \a text
*/
void SysMessagesService::sendMessage( const QString &subject, const QString &text )
{
    SysMail sysMail( QDateTime::currentDateTime(), subject, text );
    sysMailMap.insert( maxMessageId, sysMail );
    passOnMessage( maxMessageId );
    maxMessageId++;

    QtopiaApplication *qtopiaApp = qobject_cast<QtopiaApplication *>(qApp);

    //can't use qtopiaApp->willKeepRunning() since it will return true
    //if a message box is showing 
    if ( !keepRunning )
    {
        qtopiaApp->registerRunningTask( "QtopiaSysMessages", this );
        keepRunning = true;
    }
}

/*!
  \internal
  Acknowledgement that the message corresponding to \a messageId has
  been received by qtmail.  The message can be deleted from the
  internal queue.
 */
void SysMessagesService::ackMessage( int messageId )
{
    if ( sysMailMap.remove( messageId ) != 1 )
        qWarning() << "SysMessagesService: message corresponding to messageId, "
                   << messageId << ", could not be removed";

    if ( sysMailMap.isEmpty() )
    {
        ( qobject_cast<QtopiaApplication *>(qApp) )->unregisterRunningTask( "QtopiaSysMessages" );
        keepRunning = false;

        // Report that all messages have been processed
        QtopiaIpcEnvelope e( "QPE/SysMessages", "processed()" );
    }
}


/*!
  Sends out all system messages on channel QPE/SysMessages
  on startup to retrieve all pending system sms messages.
 */
void SysMessagesService::collectMessages()
{
    if (!sysMailMap.isEmpty()) {
        QMap<int, SysMail>::const_iterator it;
        for ( it = sysMailMap.begin(); it != sysMailMap.end(); ++it )
            passOnMessage( it.key() );
    } else {
        // Report that all messages have been processed
        QtopiaIpcEnvelope e( "QPE/SysMessages", "processed()" );
    }
}

/*!
  \internal
  This slot listens to messages received on QPE/SysMessages channel and takes
  appropriate action depending on the \a message and \a data.
*/
void SysMessagesService::received(const QString &message, const QByteArray &data)
{
    if ( message == "collectMessages()" ) {
        collectMessages();
    } else if ( message == "ackMessage(int)" ) {
        int messageId;
        QDataStream ds( data );
        ds >> messageId;
        ackMessage( messageId );
    }
}
#endif

