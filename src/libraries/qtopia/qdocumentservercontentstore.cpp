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
#include "qdocumentservercontentstore_p.h"
#include "qdocumentserverchannel_p.h"
#include "qdocumentservercontentsetengine_p.h"
#include "qmimetypedata_p.h"
#include "qdocumentservercontentengine_p.h"
#include "qcontentengine_p.h"
#include <qtopialog.h>

#include <QCache>
#include <QContent>

class QDocumentServerContentStorePrivate : public QDocumentServerClient
{
public:
    QDocumentServerContentStorePrivate( QObject *parent = 0 )
        : QDocumentServerClient( "QContentStoreServer", parent )
    {
    }

    QMap< int, QDocumentServerContentSetEngine * > contentSets;

protected:
    virtual void invokeSignal( const QDocumentServerMessage &message );
};

void QDocumentServerContentStorePrivate::invokeSignal( const QDocumentServerMessage &message )
{
    const QByteArray signature = message.signature();
    const QVariantList arguments = message.arguments();

    if( signature == "insertContentIntoSet(int,int,int)" )
    {
        Q_ASSERT( arguments.count() == 3 );

        QDocumentServerContentSetEngine *contentSet = contentSets.value( arguments[ 0 ].toInt(), 0 );

        if( contentSet )
        {
            contentSet->insertContent( arguments[ 1 ].toInt(), arguments[ 2 ].toInt() );
        }
    }
    else if( signature == "removeContentFromSet(int,int,int)" )
    {
        Q_ASSERT( arguments.count() == 3 );

        QDocumentServerContentSetEngine *contentSet = contentSets.value( arguments[ 0 ].toInt(), 0 );

        if( contentSet )
        {
            contentSet->removeContent( arguments[ 1 ].toInt(), arguments[ 2 ].toInt() );
        }
    }
    else if( signature == "contentSetChanged(int,int,int)" )
    {
        Q_ASSERT( arguments.count() == 3 );

        QDocumentServerContentSetEngine *contentSet = contentSets.value( arguments[ 0 ].toInt(), 0 );

        if( contentSet )
        {
            contentSet->refreshContent( arguments[ 1 ].toInt(), arguments[ 2 ].toInt() );
        }
    }
    else if( signature == "contentSetUpdateStarted(int)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        QDocumentServerContentSetEngine *contentSet = contentSets.value( arguments[ 0 ].toInt(), 0 );

        if( contentSet )
        {
            contentSet->updateStarted();
        }
    }
    else if( signature == "contentSetUpdateFinished(int)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        QDocumentServerContentSetEngine *contentSet = contentSets.value( arguments.at( 0 ).toInt(), 0 );

        if( contentSet )
        {
            contentSet->updateFinished();
        }
    }
}

QDocumentServerContentStore::QDocumentServerContentStore( QObject *parent )
    : QContentStore( parent )
{
    d = new QDocumentServerContentStorePrivate;

    for( int i = 0; i < 5 && !d->connect(); i++ )
        qWarning() << "Could not connect to content store server";
}

QDocumentServerContentStore::~QDocumentServerContentStore()
{
    delete d;
}

QContent QDocumentServerContentStore::contentFromId( QContentId contentId )
{
    if( contentId == QContent::InvalidId )
        return QContent();

    QContent content = QContentCache::instance()->lookup( contentId );

    if( content.isNull() )
    {
        QDocumentServerMessage response = d->callWithArgumentList( "contentFromId(QContentId)",
                QVariantList() << QVariant::fromValue( contentId ) );

        if( response.type() == QDocumentServerMessage::ReplyMessage )
        {
            Q_ASSERT( response.arguments().count() == 1 );

            content = qvariant_cast< QContent >( response.arguments().first() );

            QContentCache::instance()->cache( content );
        }
    }

    return content;
}

QContent QDocumentServerContentStore::contentFromFileName( const QString &fileName, LookupFlags lookup )
{
    if( fileName.isEmpty() )
        return QContent();

    QByteArray signature = "contentFromFileName(QString," + QByteArray::number( lookup, 2 ).rightJustified( 3, '0' ) + ')';

    QDocumentServerMessage response = d->callWithArgumentList( signature.constData(),
            QVariantList() << fileName );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        QContent content = qvariant_cast< QContent >( response.arguments().first() );

        QContentCache::instance()->cache( content );

        return content;
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QContent();
}

QContent QDocumentServerContentStore::contentFromEngineType( const QString &engineType )
{
    return QContent( new QDocumentServerContentEngine( engineType ) );
}

bool QDocumentServerContentStore::commitContent( QContent *content )
{
    QDocumentServerMessage response = d->callWithArgumentList( "commitContent(QContent)",
            QVariantList() << QVariant::fromValue( *content ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        *content = qvariant_cast< QContent >( response.arguments().first() );

        QContentCache::instance()->cache( *content );

        return true;
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerContentStore::removeContent( QContent *content )
{
    QDocumentServerMessage response = d->callWithArgumentList( "removeContent(QContentId)",
            QVariantList() << QVariant::fromValue( content->id() ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        if( qvariant_cast< bool >( response.arguments().first() ) )
        {
            *content = QContent();

            return true;
        }
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerContentStore::uninstallContent( QContentId contentId )
{
    if( contentId == QContent::InvalidId )
        return false;

    QDocumentServerMessage response = d->callWithArgumentList( "uninstallContent(QContentId)",
            QVariantList() << QVariant::fromValue( contentId ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

void QDocumentServerContentStore::batchCommitContent( const QContentList &content )
{
    d->callSlotWithArgumentList( "batchCommitContent(QContentList)",
                                 QVariantList() << QVariant::fromValue( content ) );
}

void QDocumentServerContentStore::batchUninstallContent( const QContentIdList &content )
{
    d->callSlotWithArgumentList( "batchUninstallContent(QContentIdList)",
                                 QVariantList() << QVariant::fromValue( content ) );
}

bool QDocumentServerContentStore::moveContentTo( QContent *content, const QString &newFileName )
{
    QDocumentServerMessage response = d->callWithArgumentList( "moveContentTo(QContentId,QString)",
            QVariantList() << QVariant::fromValue( content->id() ) << newFileName );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        *content = qvariant_cast< QContent >( response.arguments().first() );

        QContentCache::instance()->cache( *content );

        return true;
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerContentStore::copyContentTo( QContent *content, const QString &newFileName )
{
    QDocumentServerMessage response = d->callWithArgumentList( "copyContentTo(QContentId,QString)",
            QVariantList() << QVariant::fromValue( content->id() ) << newFileName );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerContentStore::renameContent(QContent *content, const QString &name)
{
    QDocumentServerMessage response = d->callWithArgumentList("renameContent(QContentId,QString)",
            QVariantList() << QVariant::fromValue(content->id()) << name);

    if (response.type() == QDocumentServerMessage::ReplyMessage) {
        Q_ASSERT(response.arguments().count() == 1);

        *content = qvariant_cast<QContent>(response.arguments().first());

        QContentCache::instance()->cache(*content);

        return true;
    } else if (response.type() == QDocumentServerMessage::ErrorMessage) {
        Q_ASSERT(response.arguments().count() == 1);

        setErrorString(qvariant_cast<QString>(response.arguments().first()));
    }
    return false;
}

QIODevice *QDocumentServerContentStore::openContent( QContent *content, QIODevice::OpenMode mode )
{
    QByteArray signature = "openContent(QContent,QIODevice::OpenMode," + QByteArray::number( mode & QIODevice::ReadWrite, 2 ).rightJustified( 2, '0' ) + ')';

    QDocumentServerMessage response = d->callWithArgumentList( signature.constData(),
            QVariantList() << QVariant::fromValue( *content ) << int( mode ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        *content = qvariant_cast< QContent >( response.arguments().first() );

        QContentCache::instance()->cache( *content );

        return contentEngine( content )->open( mode );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return 0;
}

QStringList QDocumentServerContentStore::contentCategories( QContentId contentId )
{
    if( contentId == QContent::InvalidId )
        return QStringList();

    QDocumentServerMessage response = d->callWithArgumentList( "contentCategories(QContentId)",
            QVariantList() << QVariant::fromValue( contentId ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QStringList >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QStringList();
}

QContentEnginePropertyCache QDocumentServerContentStore::contentProperties( QContentId contentId )
{
    if( contentId == QContent::InvalidId )
        return QContentEnginePropertyCache();

    QDocumentServerMessage response = d->callWithArgumentList( "contentProperties(QContentId)",
            QVariantList() << QVariant::fromValue( contentId ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QContentEnginePropertyCache >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QContentEnginePropertyCache();
}

QStringList QDocumentServerContentStore::contentMimeTypes( QContentId contentId )
{
    if( contentId == QContent::InvalidId )
        return QStringList();

    QDocumentServerMessage response = d->callWithArgumentList( "contentMimeTypes(QContentId)",
            QVariantList() << QVariant::fromValue( contentId ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QStringList >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

        return QStringList();
}

QMimeTypeData QDocumentServerContentStore::mimeTypeFromId( const QString &mimeId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "mimeTypeFromId(QString)",
            QVariantList() << mimeId );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QMimeTypeData >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

        return QMimeTypeData();
}

/*!
    \reimp
*/
QContentSetEngine *QDocumentServerContentStore::contentSet( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode )
{
    QDocumentServerMessage response = d->callWithArgumentList(
            "createContentSet(QContentFilter,QContentSortCriteria,QContentSet::UpdateMode)",
            QVariantList()
                    << QVariant::fromValue(filter)
                    << QVariant::fromValue(order)
                    << QVariant::fromValue(mode));

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        int setId = response.arguments().first().toInt();

        QDocumentServerContentSetEngine *contentSet = new QDocumentServerContentSetEngine( filter, order, mode, setId, this );

        d->contentSets.insert( setId, contentSet );

        connect( contentSet, SIGNAL(releaseContentSet(int)), this, SLOT(releaseContentSet(int)) );

        return contentSet;
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

        return 0;
}

QContentFilterSetEngine *QDocumentServerContentStore::contentFilterSet( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType )
{
    Q_UNUSED( filter );
    Q_UNUSED( type );
    Q_UNUSED( subType );

    return 0;
}

QStringList QDocumentServerContentStore::filterMatches( const QContentFilter &filter, QContentFilter::FilterType type, const QString &subType )
{
    QDocumentServerMessage response = d->callWithArgumentList( "filterMatches(QContentFilter,QContentFilter::FilterType,QString)",
            QVariantList() << QVariant::fromValue( filter ) << QVariant::fromValue( type ) << subType );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QStringList >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QStringList();
}

int QDocumentServerContentStore::contentCount( const QContentFilter &filter )
{
    if( !filter.isValid() )
        return 0;

    QDocumentServerMessage response = d->callWithArgumentList( "contentCount(QContentFilter)",
            QVariantList() << QVariant::fromValue( filter ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< int >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return 0;
}

void QDocumentServerContentStore::releaseContentSet( int setId )
{
    d->contentSets.remove( setId );

    d->callSlotWithArgumentList( "releaseContentSet(int)", QVariantList() << setId );
}

int QDocumentServerContentStore::contentSetCount( int setId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "contentSetCount(int)", QVariantList() << setId );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< int >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return 0;
}

QContentList QDocumentServerContentStore::contentSetFrame( int setId, int start, int count )
{
    QDocumentServerMessage response = d->callWithArgumentList( "contentSetFrame(int,int,int)",
            QVariantList() << setId << start << count );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        QContentList content = qvariant_cast< QContentList >( response.arguments().first() );

        foreach( const QContent &c, content )
            QContentCache::instance()->cache( c );

        for( int i = content.count(); i < count; i++ )
            content.append( QContent() );

        return content;
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QContentList();
}

void QDocumentServerContentStore::setContentSetFilter( int setId, const QContentFilter &filter )
{
    d->callSlotWithArgumentList( "setContentSetFilter(int,QContentFilter)", 
                                 QVariantList() << setId << QVariant::fromValue( filter ) );
}

void QDocumentServerContentStore::setContentSetSortOrder( int setId, const QContentSortCriteria &sortOrder )
{
    d->callSlotWithArgumentList( "setContentSetSortOrder(int,QContentSortCriteria)",
                                 QVariantList() << setId << QVariant::fromValue( sortOrder ) );
}

void QDocumentServerContentStore::setContentSetCriteria( int setId, const QContentFilter &filter, const QContentSortCriteria &sortOrder )
{
    d->callSlotWithArgumentList( "setContentSetCriteria(int,QContentFilter,QContentSortCriteria)",
                                 QVariantList() << setId << QVariant::fromValue( filter ) << QVariant::fromValue( sortOrder ) );
}


void QDocumentServerContentStore::insertContentIntoSet( int setId, const QContent &content )
{
    d->callSlotWithArgumentList( "insertContentIntoSet(int,QContent)",
                                 QVariantList() << setId << QVariant::fromValue( content ) );
}

void QDocumentServerContentStore::removeContentFromSet( int setId, const QContent &content )
{
    d->callSlotWithArgumentList( "removeContentFromSet(int,QContent)",
                                 QVariantList() << setId << QVariant::fromValue( content ) );
}

void QDocumentServerContentStore::clearContentSet(int setId)
{
    d->callSlotWithArgumentList("clearContentSet(int)", QVariantList() << setId);
}

void QDocumentServerContentStore::commitContentSet(int setId)
{
    d->callSlotWithArgumentList("commitContentSet(int)", QVariantList() << setId);
}

bool QDocumentServerContentStore::contentSetContains( int setId, const QContent &content )
{
    QDocumentServerMessage response = d->callWithArgumentList( "contentSetContains(int,QContent)",
            QVariantList() << setId << QVariant::fromValue( content ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerContentStore::contentValid( QContentId contentId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "contentValid(QContentId)",
            QVariantList() << QVariant::fromValue( contentId ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

QDrmRights QDocumentServerContentStore::contentRights( QContentId contentId, QDrmRights::Permission permission )
{
    QDocumentServerMessage response = d->callWithArgumentList( "contentRights(QContentId,QDrmRights::Permission)",
            QVariantList() << QVariant::fromValue( contentId ) << QVariant::fromValue( permission ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QDrmRights >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QDrmRights();
}

bool QDocumentServerContentStore::executeContent( QContentId contentId, const QStringList &arguments )
{
    QDocumentServerMessage response = d->callWithArgumentList( "executeContent(QContentId,QStringList)",
            QVariantList() << QVariant::fromValue( contentId ) << arguments );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

QDrmRights::Permissions QDocumentServerContentStore::contentPermissions( QContentId contentId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "contentPermissions(QContentId)",
            QVariantList() << QVariant::fromValue( contentId ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QDrmRights::Permissions >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QDrmRights::Permissions();
}

qint64 QDocumentServerContentStore::contentSize( QContentId contentId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "contentSize(QContentId)",
            QVariantList() << QVariant::fromValue( contentId ) );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< qint64 >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return 0;
}

void QDocumentServerContentStore::addAssociation(const QString& mimeType, const QString& application, const QString& icon, QDrmRights::Permission permission)
{
    d->callSlotWithArgumentList( "addAssociation(QString,QString,QString,QDrmRights::Permission)",
                                 QVariantList() << mimeType << application << icon << permission );
}

void QDocumentServerContentStore::removeAssociation(const QString& mimeType, const QString& application)
{
    d->callSlotWithArgumentList( "removeAssociation(QString,QString)",
                                 QVariantList() << mimeType << application );
}

void QDocumentServerContentStore::setDefaultApplicationFor(const QString& mimeType, const QString& application)
{
    d->callSlotWithArgumentList( "setDefaultApplicationFor(QString,QString)",
                                 QVariantList() << mimeType << application );
}

QList<QMimeEngineData> QDocumentServerContentStore::associationsForApplication(const QString& application )
{
    QDocumentServerMessage response = d->callWithArgumentList( "associationsForApplication(QString)",
            QVariantList() << application );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QList<QMimeEngineData> >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }
    return QList<QMimeEngineData>();
}

QList<QMimeEngineData> QDocumentServerContentStore::associationsForMimeType(const QString& mimeType )
{
    QDocumentServerMessage response = d->callWithArgumentList( "associationsForMimeType(QString)",
            QVariantList() << mimeType );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QList<QMimeEngineData> >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }
    return QList<QMimeEngineData>();
}
