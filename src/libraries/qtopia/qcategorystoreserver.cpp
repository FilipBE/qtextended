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
#include "qcategorystoreserver_p.h"
#include "qcategorystore_p.h"
#include <qtopianamespace.h>
#include <QtDebug>

QCategoryStoreServer::QCategoryStoreServer( QObject *parent )
    : QDocumentServerHost( "QCategoryStoreServer", parent )
{
    QObject::connect( QCategoryStore::instance(), SIGNAL(categoriesChanged()), this, SLOT(categoriesChanged()) );
    QObject::connect( this, SIGNAL(disconnected()), this, SLOT(deleteLater()) );
}

QDocumentServerMessage QCategoryStoreServer::invokeMethod( const QDocumentServerMessage &message )
{
    const QByteArray signature = message.signature();
    const QVariantList arguments = message.arguments();

    if( signature == "addCategory(QString,QString,QString,QString,bool)" )
    {
        QVariantList arguments = message.arguments();

        Q_ASSERT( arguments.count() == 5 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->addCategory(
                qvariant_cast< QString >( arguments[ 0 ] ),
                qvariant_cast< QString >( arguments[ 1 ] ),
                qvariant_cast< QString >( arguments[ 2 ] ),
                qvariant_cast< QString >( arguments[ 3 ] ),
                qvariant_cast< bool    >( arguments[ 4 ] ) ) ) );
    }
    else if( signature == "categoryExists(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->categoryExists(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "categoryFromId(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->categoryFromId(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "scopeCategories(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->scopeCategories(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "removeCategory(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->removeCategory(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "setCategoryScope(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setCategoryScope(
                qvariant_cast< QString >( arguments[ 0 ] ),
                qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if( signature == "setCategoryIcon(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setCategoryIcon(
                qvariant_cast< QString >( arguments[ 0 ] ),
                qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if ( signature == "setCategoryRingTone(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );
        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setCategoryRingTone(
                qvariant_cast< QString >( arguments[ 0 ] ),
                qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if( signature == "setCategoryLabel(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setCategoryLabel(
                qvariant_cast< QString >( arguments[ 0 ] ),
        qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if( signature == "setSystemCategory(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setSystemCategory(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else
    {
        qWarning() << "Tried to invoke unknown message";

        Q_ASSERT( false );

        return message.createError( "Unknown method" );
    }
}

void QCategoryStoreServer::categoriesChanged()
{
    emitSignalWithArgumentList( "categoriesChanged()", QVariantList() );
}

QCategoryStoreSocketServer::QCategoryStoreSocketServer( QObject *parent )
    : QUnixSocketServer( parent )
{
    QByteArray socketPath = (Qtopia::tempDir() + QLatin1String( "QCategoryStoreServer" )).toLocal8Bit();

    listen( socketPath );
}

void QCategoryStoreSocketServer::incomingConnection( int socketDescriptor )
{
    QCategoryStoreServer *server = new QCategoryStoreServer( this );

    server->setSocketDescriptor( socketDescriptor );
}

QCategoryStoreServerTask::QCategoryStoreServerTask( QObject *parent )
    : QThread( parent )
{
    start();
}

void QCategoryStoreServerTask::run()
{
    QCategoryStoreSocketServer socketServer;

    exec();
}
