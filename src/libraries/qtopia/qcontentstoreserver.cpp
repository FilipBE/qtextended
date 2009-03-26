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
#include "qcontentstoreserver_p.h"
#include "qcontentstore_p.h"
#include "qmimetypedata_p.h"
#include "qsqlcontentstore_p.h"
#include "qcontentsetengine_p.h"
#include <qtopianamespace.h>
#include <QtDebug>

class QContentStoreServerSet : public QObject
{
    Q_OBJECT
public:
    QContentStoreServerSet( int setId, QContentSetEngine *set, QObject *parent );

    int setId() const;

    QContentList contentFrame( int start, int end ) const;

    QContentSetEngine *set();

    const QContentSetEngine *set() const;

signals:
    void removeContent( int setId, int start, int end );

    void insertContent( int setId, int start, int end );

    void contentChanged( int setId, int start, int end );

    void updateStarted( int setId );
    void updateFinished( int setId );

private slots:
    void contentAboutToBeRemoved( int start, int end );
    void contentRemoved();

    void contentAboutToBeInserted( int start, int end );
    void contentInserted();

    void contentChanged( int start, int end );

    void updateStarted();
    void updateFinished();

private:
    int m_setId;

    QContentSetEngine *m_set;

    int m_pendingStart;
    int m_pendingEnd;

};


QContentStoreServerSet::QContentStoreServerSet( int setId, QContentSetEngine *set, QObject *parent )
    : QObject( parent )
    , m_setId( setId )
    , m_set( set )
    , m_pendingStart( -1 )
    , m_pendingEnd( -1 )
{
    m_set->setParent( this );

    connect( set, SIGNAL(contentAboutToBeRemoved(int,int)), this, SLOT(contentAboutToBeRemoved(int,int)) );
    connect( set, SIGNAL(contentAboutToBeInserted(int,int)), this, SLOT(contentAboutToBeInserted(int,int)) );
    connect( set, SIGNAL(contentRemoved()), this, SLOT(contentRemoved()) );
    connect( set, SIGNAL(contentInserted()), this, SLOT(contentInserted()) );
    connect( set, SIGNAL(contentChanged(int,int)), this, SLOT(contentChanged(int,int)) );
    connect( set, SIGNAL(updateStarted()), this, SLOT(updateStarted()) );
    connect( set, SIGNAL(updateFinished()), this, SLOT(updateFinished()) );

    connect( this, SIGNAL(removeContent(int,int,int)), parent, SLOT(removeContent(int,int,int)) );
    connect( this, SIGNAL(insertContent(int,int,int)), parent, SLOT(insertContent(int,int,int)) );
    connect( this, SIGNAL(contentChanged(int,int,int)), parent, SLOT(contentChanged(int,int,int)) );
    connect( this, SIGNAL(updateStarted(int)), parent, SLOT(updateStarted(int)) );
    connect( this, SIGNAL(updateFinished(int)), parent, SLOT(updateFinished(int)) );
}

int QContentStoreServerSet::setId() const
{
    return m_setId;
}

QContentList QContentStoreServerSet::contentFrame( int start, int end ) const
{
    QContentList list;

    for( int i = start; i <= end &&  i < m_set->count(); i++ )
        list.append( m_set->content( i ) );

    return list;
}

QContentSetEngine *QContentStoreServerSet::set()
{
    return m_set;
}

const QContentSetEngine *QContentStoreServerSet::set() const
{
    return m_set;
}

void QContentStoreServerSet::contentAboutToBeRemoved( int start, int end )
{
    m_pendingStart = start;
    m_pendingEnd = end;
}

void QContentStoreServerSet::contentRemoved()
{
    emit removeContent( m_setId, m_pendingStart, m_pendingEnd );
}

void QContentStoreServerSet::contentAboutToBeInserted( int start, int end )
{
    m_pendingStart = start;
    m_pendingEnd = end;
}

void QContentStoreServerSet::contentInserted()
{
    emit insertContent( m_setId, m_pendingStart, m_pendingEnd );
}

void QContentStoreServerSet::contentChanged( int start, int end )
{
    emit contentChanged( m_setId, start, end );
}

void QContentStoreServerSet::updateStarted()
{
    emit updateStarted( m_setId );
}

void QContentStoreServerSet::updateFinished()
{
    emit updateFinished( m_setId );
}

QContentStoreServer::QContentStoreServer( QObject *parent )
    : QDocumentServerHost( "QContentStoreServer", parent )
{
    QObject::connect( this, SIGNAL(disconnected()), this, SLOT(deleteLater()) );
}

QDocumentServerMessage QContentStoreServer::invokeMethod( const QDocumentServerMessage &message )
{
    const QByteArray signature = message.signature();
    const QVariantList arguments = message.arguments();

    if( signature == "contentFromId(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->contentFromId(
                qvariant_cast< QContentId >( arguments[ 0 ] ) ) ) );
    }
    else if( signature.startsWith( "contentFromFileName(QString," ) )
    {
        Q_ASSERT( arguments.count() == 1 );

        bool ok = false;

        int flags = signature.mid( 28, 3 ).toInt( &ok );

        if( ok )
        {
            return message.createReply( QVariant::fromValue( QContentStore::instance()->contentFromFileName(
                    qvariant_cast< QString >( arguments[ 0 ] ), static_cast< QContentStore::LookupFlag >( flags ) ) ) );
        }
        else
            return message.createError( "Malformed signature" );
    }
    else if( signature == "commitContent(QContent)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        QContent content = qvariant_cast< QContent >( arguments[ 0 ] );

        if( QContentStore::instance()->commitContent( &content ) )
        {
            return message.createReply( QVariant::fromValue( content ) );
        }
        else
        {
            QString errorString = QContentStore::instance()->errorString();

            if( errorString.isEmpty() )
                errorString = "Content commit failed for an unknown reason";

            return message.createError( errorString );
        }
    }
    else if( signature == "removeContent(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        QContent content( qvariant_cast< QContentId >( arguments[ 0 ] ) );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->removeContent(
                &content ) ) );
    }
    else if( signature == "uninstallContent(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->uninstallContent(
                qvariant_cast< QContentId >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "moveContentTo(QContentId,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        QContent content( qvariant_cast< QContentId >( arguments[ 0 ] ) );

        if( QContentStore::instance()->moveContentTo( &content, qvariant_cast< QString >( arguments[ 1 ] ) ) )
        {
            return message.createReply( QVariant::fromValue( content ) );
        }
        else
        {
            QString errorString = QContentStore::instance()->errorString();

            if( errorString.isEmpty() )
                errorString = "Content move failed for an unknown reason";

            return message.createError( errorString );
        }
    }
    else if( signature == "copyContentTo(QContentId,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        QContent content( qvariant_cast< QContentId >( arguments[ 0 ] ) );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->copyContentTo(
                &content, qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if (signature == "renameContent(QContentId,QString)")
    {
        Q_ASSERT(arguments.count() == 2);

        QContent content(qvariant_cast<QContentId>(arguments[0]));

        if (QContentStore::instance()->renameContent(&content, qvariant_cast<QString>(arguments[1]))) {
            return message.createReply(QVariant::fromValue(content));
        } else {
            QString errorString = QContentStore::instance()->errorString();

            if (errorString.isEmpty())
                errorString == "Content rename failed for an unknown reason";

            return message.createError(errorString);
        }
    }
    else if( signature.startsWith( "openContent(QContent,QIODevice::OpenMode," ) )
    {
        Q_ASSERT( arguments.count() == 2 );

        bool ok = false;

        int flags = signature.mid( 41, 2 ).toInt( &ok );

        if( !ok )
            return message.createError( "Malformed signature" );

        flags |= arguments[ 1 ].toInt() & ~QIODevice::ReadWrite;

        QContent content = qvariant_cast< QContent >( arguments[ 0 ] );

        QIODevice *device = content.open( static_cast< QIODevice::OpenModeFlag >( flags ) );

        if( device )
        {
            device->close();

            delete device;

            return message.createReply( QVariant::fromValue( content ) );
        }
        else
        {
            QString errorString = content.errorString();

            if( errorString.isEmpty() )
                errorString = "Failed to open content for unknown reason";

            return message.createError( errorString );
        }
    }
    else if( signature == "contentCategories(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->contentCategories(
                qvariant_cast< QContentId >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "contentProperties(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->contentProperties(
                qvariant_cast< QContentId >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "contentMimeTypes(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->contentMimeTypes(
                qvariant_cast< QContentId >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "mimeTypeFromId(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->mimeTypeFromId(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "filterMatches(QContentFilter,QContentFilter::FilterType,QString)" )
    {
        Q_ASSERT( arguments.count() == 3 );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->filterMatches(
                qvariant_cast< QContentFilter >( arguments[ 0 ] ),
                qvariant_cast< QContentFilter::FilterType >( arguments[ 1 ] ),
                qvariant_cast< QString >( arguments[ 2 ] ) ) ) );
    }
    else if( signature == "contentCount(QContentFilter)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QContentStore::instance()->contentCount(
                qvariant_cast< QContentFilter >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "createContentSet(QContentFilter,QContentSortCriteria,QContentSet::UpdateMode)" )
    {
        Q_ASSERT( arguments.count() == 3 );

        static int setIds = 0;

        int setId = setIds++;

        QContentSetEngine *contentSet = QContentStore::instance()->contentSet(
                qvariant_cast<QContentFilter>(arguments[0]),
                qvariant_cast<QContentSortCriteria>(arguments[1]),
                qvariant_cast<QContentSet::UpdateMode>(arguments[2]));

        if( contentSet )
        {
            m_contentSets.insert( setId, new QContentStoreServerSet( setId, contentSet, this ) );

            return message.createReply( QVariant::fromValue( setId ) );
        }
        else
        {
            QString errorString = QContentStore::instance()->errorString();

            if( errorString.isEmpty() )
                errorString = "Failed to construct content set for unknown reason";

            return message.createError( errorString );
        }
    }
    else if( signature == "contentSetCount(int)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        int setId = arguments[ 0 ].toInt();

        QContentStoreServerSet *contentSet = m_contentSets.value( setId, 0 );

        if( contentSet )
            return message.createReply( QVariant::fromValue( contentSet->set()->count() ) );
        else
            return message.createError( "Content set doesn't exist" );
    }
    else if( signature == "contentSetFrame(int,int,int)" )
    {
        Q_ASSERT( arguments.count() == 3 );

        int setId = arguments[ 0 ].toInt();

        QContentStoreServerSet *serverSet = m_contentSets.value( setId, 0 );

        if( serverSet )
        {
            QContentSetEngine *contentSet = serverSet->set();

            QContentList content;

            int start = arguments[ 1 ].toInt();
            int end = arguments[ 2 ].toInt() + start;

            for( int i = start; i < end && i < contentSet->count(); i++ )
                content.append( contentSet->content( i ) );

            return message.createReply( QVariant::fromValue( content ) );
        }
        else
            return message.createError( "Content set doesn't exist" );
    }
    else if( signature == "contentSetContains(int,QContent)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        int setId = arguments[ 0 ].toInt();

        QContentStoreServerSet *serverSet = m_contentSets.value( setId, 0 );

        if( serverSet )
        {
            return message.createReply( QVariant::fromValue(
                    serverSet->set()->contains( qvariant_cast< QContent >( arguments[ 1 ] ) ) ) );
        }
        else
            return message.createError( "Content set doesn't exist" );
    }
    else if( signature == "contentValid(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QContent( qvariant_cast< QContentId >( arguments[ 0 ] ) ).isValid( true ) ) );
    }
    else if( signature == "contentRights(QContentId,QDrmRights::Permission)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        QContent content( qvariant_cast< QContentId >( arguments[ 0 ] ) );

        return message.createReply( QVariant::fromValue(
                content.rights( qvariant_cast< QDrmRights::Permission >( arguments[ 1 ] ) ) ) );
    }
    else if( signature == "executeContent(QContentId,QStringList)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        QContent content( qvariant_cast< QContentId >( arguments[ 0 ] ) );

        content.execute( qvariant_cast< QStringList >( arguments[ 1 ] ) );

        return message.createReply( QVariant::fromValue( true ) );
    }
    else if( signature == "contentPermissions(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        QContent content( qvariant_cast< QContentId >( arguments[ 0 ] ) );

        return message.createReply( QVariant::fromValue(
                content.permissions() ) );
    }
    else if( signature == "contentSize(QContentId)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        QContent content( qvariant_cast< QContentId >( arguments[ 0 ] ) );

        return message.createReply( QVariant::fromValue(
                content.size() ) );
    }
    else if( signature == "associationsForApplication(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

       return message.createReply( QVariant::fromValue( QContentStore::instance()->associationsForApplication(qvariant_cast<QString>( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "associationsForMimeType(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

       return message.createReply( QVariant::fromValue( QContentStore::instance()->associationsForMimeType(qvariant_cast<QString>( arguments[ 0 ] ) ) ) );
    }
    else
    {
        qWarning() << "Tried to invoke unknown message" << signature;

        Q_ASSERT( false );

        return message.createError( "Unknown method" );
    }
}

void QContentStoreServer::invokeSlot( const QDocumentServerMessage &message )
{
    const QByteArray signature = message.signature();
    const QVariantList arguments = message.arguments();

    if( signature == "releaseContentSet(int)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        int setId = arguments.first().toInt();

        if( m_contentSets.contains( setId ) )
            delete m_contentSets.take( setId );
    }
    else if( signature == "setContentSetFilter(int,QContentFilter)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        int setId = arguments[ 0 ].toInt();

        QContentStoreServerSet *contentSet = m_contentSets.value( setId, 0 );

        if( contentSet )
            contentSet->set()->setFilter( qvariant_cast< QContentFilter >( arguments[ 1 ] ) );
    }
    else if( signature == "setContentSetSortOrder(int,QContentSortCriteria)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        int setId = arguments[ 0 ].toInt();

        QContentStoreServerSet *contentSet = m_contentSets.value( setId, 0 );

        if( contentSet )
            contentSet->set()->setSortCriteria( qvariant_cast< QContentSortCriteria >( arguments[ 1 ] ) );
    }
    else if( signature == "setContentSetCriteria(int,QContentFilter,QContentSortCriteria)" )
    {
        Q_ASSERT( arguments.count() == 3 );

        int setId = arguments[ 0 ].toInt();

        QContentStoreServerSet *contentSet = m_contentSets.value( setId, 0 );

        if( contentSet )
        {
            contentSet->set()->setFilter( qvariant_cast< QContentFilter >( arguments[ 1 ] ) );
            contentSet->set()->setSortCriteria( qvariant_cast< QContentSortCriteria >( arguments[ 2 ] ) );
        }
    }
    else if( signature == "insertContentIntoSet(int,QContent)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        int setId = arguments[ 0 ].toInt();

        QContentStoreServerSet *contentSet = m_contentSets.value( setId, 0 );

        if( contentSet )
            contentSet->set()->insertContent( qvariant_cast< QContent >( arguments[ 1 ] ) );
    }
    else if( signature == "removeContentFromSet(int,QContent)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        int setId = arguments[ 0 ].toInt();

        QContentStoreServerSet *contentSet = m_contentSets.value( setId, 0 );

        if( contentSet )
            contentSet->set()->removeContent( qvariant_cast< QContent >( arguments[ 1 ] ) );
    }
    else if (signature == "clearContentSet(int)")
    {
        Q_ASSERT(arguments.count() == 1);

        int setId = arguments[0].toInt();

        if (QContentStoreServerSet *contentSet = m_contentSets.value(setId, 0))
            contentSet->set()->clear();
    }
    else if (signature == "commitContentSet(int)" )
    {
        Q_ASSERT(arguments.count() == 1);

        int setId = arguments[0].toInt();

        if (QContentStoreServerSet *contentSet = m_contentSets.value(setId, 0))
            contentSet->set()->commitChanges();
    }
    else if( signature == "batchCommitContent(QContentList)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        QContentStore::instance()->batchCommitContent( qvariant_cast< QContentList >( arguments[ 0 ] ) );
    }
    else if( signature == "batchUninstallContent(QContentIdList)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        QContentStore::instance()->batchUninstallContent( qvariant_cast< QContentIdList >( arguments[ 0 ] ) );
    }
    else if( signature == "addAssociation(QString,QString,QString,QDrmRights::Permission)" )
    {
        Q_ASSERT( arguments.count() == 4 );

        QContentStore::instance()->addAssociation( qvariant_cast< QString >( arguments[ 0 ] ), qvariant_cast< QString >( arguments[ 1 ] ),
                                                   qvariant_cast< QString >( arguments[ 2 ] ), qvariant_cast< QDrmRights::Permission >( arguments[ 3 ] ));
    }
    else if( signature == "removeAssociation(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        QContentStore::instance()->removeAssociation( qvariant_cast< QString >( arguments[ 0 ] ), qvariant_cast< QString >( arguments[ 1 ] ));
    }
    else if( signature == "setDefaultApplicationFor(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        QContentStore::instance()->setDefaultApplicationFor( qvariant_cast< QString >( arguments[ 0 ] ), qvariant_cast< QString >( arguments[ 1 ] ));
    }
    else
    {
        qWarning() << "Tried to invoke unknown message" << signature;

        Q_ASSERT( false );
    }
}

void QContentStoreServer::removeContent( int setId, int start, int end )
{
    emitSignalWithArgumentList( "removeContentFromSet(int,int,int)", QVariantList() << setId << start << end );
}

void QContentStoreServer::insertContent( int setId, int start, int end )
{
    emitSignalWithArgumentList( "insertContentIntoSet(int,int,int)", QVariantList() << setId << start << end );
}

void QContentStoreServer::contentChanged( int setId, int start, int end )
{
    emitSignalWithArgumentList( "contentSetChanged(int,int,int)", QVariantList() << setId << start << end );
}

void QContentStoreServer::updateStarted( int setId )
{
    emitSignalWithArgumentList( "contentSetUpdateStarted(int)",
                                QVariantList() << setId );
}

void QContentStoreServer::updateFinished( int setId )
{
    emitSignalWithArgumentList( "contentSetUpdateFinished(int)",
                                QVariantList() << setId );
}

QContentStoreSocketServer::QContentStoreSocketServer( QObject *parent )
    : QUnixSocketServer( parent )
{
    QByteArray socketPath = (Qtopia::tempDir() + QLatin1String( "QContentStoreServer" )).toLocal8Bit();

    listen( socketPath );
}

void QContentStoreSocketServer::incomingConnection( int socketDescriptor )
{
    QContentStoreServer *server = new QContentStoreServer( this );

    server->setSocketDescriptor( socketDescriptor );
}

QContentStoreServerTask::QContentStoreServerTask( QObject *parent )
    : QThread( parent )
{
    start();
}

void QContentStoreServerTask::run()
{
    QContentStoreSocketServer socketServer;

    exec();
}

#include "qcontentstoreserver.moc"
