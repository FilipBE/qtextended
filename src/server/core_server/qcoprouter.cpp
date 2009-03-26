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
#if !defined(QT_NO_COP)
#include "qcoprouter.h"
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include "applicationlauncher.h"

#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "qcopfile.h"

/*!
  \class QCopRouter
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::AppLaunch
  \brief The QCopRouter class provides an application ipc router for the QCop transport.

  QCopRouter provides a Qt Extended Server Task.  Qt Extended Server Tasks are
  documented in full in the QtopiaServerApplication class documentation.

  \table
  \row \o Task Name \o IpcRouter
  \row \o Interfaces \o ApplicationIpcRouter
  \row \o Services \o None
  \endtable

  The QCopRouter class provides an implementation of the ApplicationIpcRouter
  for the QCop transport.  
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

QTOPIA_TASK(IpcRouter, QCopRouter);
QTOPIA_TASK_PROVIDES(IpcRouter, ApplicationIpcRouter);

/*!  \internal */
QCopRouter::QCopRouter()
{
    // cleanup old messages
    QDir dir( Qtopia::tempDir(), "qcop-msg-*" );

    if (!dir.exists())
        return;
    QStringList stale = dir.entryList();
    QStringList::Iterator it;
    for ( it = stale.begin(); it != stale.end(); ++it ) {
        dir.remove( *it );
    }

    QCopChannel *channel;

    channel = new QCopChannel( "QPE/Application/*", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(applicationMessage(QString,QByteArray)) );

    channel = new QCopChannel( "QPE/Service/*", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(serviceMessage(QString,QByteArray)) );
}

/*!  \internal */
QCopRouter::~QCopRouter()
{
    // Nothing to do here at present.
}

// Process all messages to QPE/Application/*.
/*!  \internal */
void QCopRouter::applicationMessage
            ( const QString& msg, const QByteArray& data )
{
    if ( msg == QLatin1String("forwardedMessage(QString,QString,QByteArray)") ) {

        QDataStream stream( data );
        QString channel, message;
        QByteArray newData;
        stream >> channel;
        stream >> message;
        stream >> newData;

        QString app = channel.mid(16 /* ::strlen("QPE/Application/") */);
        routeMessage(app, message, newData);
    }
}

// Process all messages to QPE/Service/*.
/*!  \internal */
void QCopRouter::serviceMessage( const QString& msg, const QByteArray& data )
{
    if ( msg == QLatin1String("forwardedMessage(QString,QString,QByteArray)") ) {

        QDataStream stream( data );
        QString channel, message;
        QByteArray newData;
        stream >> channel;
        stream >> message;
        stream >> newData;

        // Bail out if it doesn't look like a valid service request.
        if ( !channel.startsWith( QLatin1String( "QPE/Service/" ) ) )
            return;

        // Look up the application channel that handles the service.
        QString appChannel = QtopiaService::channel( channel.mid(12) );
        if ( appChannel.isEmpty() ) {
            qWarning( "No service mapping for %s, cannot forward %s",
                      channel.toLatin1().constData(),
                      message.toLatin1().constData() );
            return;
        }

        QString app = appChannel.mid(16 /* ::strlen("QPE/Application/") */);
        routeMessage(app, message, newData);
    }
}

/*!  \internal */
void QCopRouter::routeMessage(const QString &dest,
                              const QString &message,
                              const QByteArray &data)
{
    if(dest.isEmpty())
        return;

    // Launch route
    ApplicationLauncher *l = qtopiaTask<ApplicationLauncher>();
    Q_ASSERT(m_cDest.isEmpty());

    m_cDest = dest;
    m_cMessage = message;
    m_cData = data;

    if(l)
        l->launch(dest);

    QMultiMap<QString, RouteDestination *>::Iterator iter = m_routes.find(dest);
    while(iter != m_routes.end() && iter.key() == dest) {
        if(!m_cRouted.contains(*iter))
            (*iter)->routeMessage(dest, message, data);
        ++iter;
    }

    m_cDest.clear();
    m_cMessage.clear();
    m_cData.clear();
    m_cRouted.clear();
}

/*!  \internal */
void QCopRouter::addRoute(const QString &app, RouteDestination *dest)
{
    Q_ASSERT(dest);
    Q_ASSERT(!app.isEmpty());
    m_routes.insert(app, dest);
    if(app == m_cDest) {
        dest->routeMessage(m_cDest, m_cMessage, m_cData);
        m_cRouted.insert(dest);
    }
}

/*!  \internal */
void QCopRouter::remRoute(const QString &app, RouteDestination *dest)
{
    Q_ASSERT(dest);
    QMultiMap<QString, RouteDestination *>::Iterator iter = m_routes.find(app);
    while(iter != m_routes.end() && iter.key() == app) {
        if(iter.value() == dest) {
            m_routes.erase(iter);
            return;
        }
        ++iter;
    }
}
#endif

