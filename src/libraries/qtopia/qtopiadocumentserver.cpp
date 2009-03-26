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
#include <QtopiaDocumentServer>
#include "qcategorystoreserver_p.h"
#include "qcontentstoreserver_p.h"
#include "qdocumentselectorsocketserver_p.h"

class QtopiaDocumentServerPrivate
{
public:
    QContentStoreServerTask contentServer;
    QCategoryStoreServerTask categoryServer;
    QDocumentSelectorSocketServer documentSelectorServer;
};

/*!
    \class QtopiaDocumentServer
    \inpublicgroup QtBaseModule

    \brief The QtopiaDocumentServer class provides an instance of the Qt Extended Document Server.

    The Qt Extended Document Server provides applications with a mechanism to interact with the
    \l{Document System}{Qt Extended Document System} without having direct access to the document databases or
    even the file system.  A single instance of the document server runs in a server process, and client
    applications connect to this server in order to gain access to the functionality provided by the document
    system API.

    The document server runs multiple threads and should be shut down before being destroyed in order to allow
    the threads to exit cleanly.  A shut down can be initiated with the shutdown() slot, and the shutdownComplete()
    signal will be emitted when this is complete.
*/

/*!
    Constructs a new Qt Extended document server with the parent \a parent.

    Only one instance of the document server should ever exist at once across all Qt Extended applications.
*/
QtopiaDocumentServer::QtopiaDocumentServer( QObject *parent )
    : QObject( parent )
{
    d = new QtopiaDocumentServerPrivate;

    connect( &d->contentServer , SIGNAL(finished()), this, SLOT(threadFinished()) );
    connect( &d->categoryServer, SIGNAL(finished()), this, SLOT(threadFinished()) );
}

/*!
    Destroys the document server object.
*/
QtopiaDocumentServer::~QtopiaDocumentServer()
{
    delete d;
}

/*!
    \fn QtopiaDocumentServer::shutdownComplete()

    Emitted by the server when the server has successfully been shutdown in response to a call to shutdown().

    \sa shutdown()
*/

/*!
    Initiates a shutdown of the document server.  The shutdownComplete() signal will be emitted when all the
    document server threads have exited.

    \sa shutdownComplete()
*/
void QtopiaDocumentServer::shutdown()
{
    d->contentServer.quit();
    d->categoryServer.quit();
}

/*!
    Called when one of the document server threads has exited.
*/
void QtopiaDocumentServer::threadFinished()
{
    if( d->contentServer.isFinished() && d->categoryServer.isFinished() )
        emit shutdownComplete();
}
