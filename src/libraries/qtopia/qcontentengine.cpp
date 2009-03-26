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

#include "qcontentengine_p.h"
#include "qcontentstore_p.h"
#include "contentpluginmanager_p.h"
#include <qtopianamespace.h>
#include <QMutex>
#include <QMimeType>
#include <QFileSystem>
#include <QtDebug>

Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QContentEnginePropertyCache,QContentEnginePropertyCache);
Q_IMPLEMENT_USER_METATYPE(QMimeEngineData);
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QMimeEngineData>);

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, contentEngineHandlerMutex, (QMutex::Recursive))
static bool qt_contentenginehandlerlist_shutDown = false;
class QContentEngineHandlerList : public QList<QContentEngineHandler *>
{
public:
    ~QContentEngineHandlerList()
    {
        QMutexLocker locker(contentEngineHandlerMutex());
        qt_contentenginehandlerlist_shutDown = true;
    }
};
Q_GLOBAL_STATIC(QContentEngineHandlerList, contentEngineHandlers)

/*!
    \class QContentEngine
    \inpublicgroup QtBaseModule

    \brief The QContentEngine class is an abstract base class which represents an item of content in the documents system.

    \internal
*/

/*!
    \class QContentEngineData
    \inpublicgroup QtBaseModule

    \brief The QContentEngineData class is the base data class for QContentEngine.

    \internal
*/

/*!
    \class QContentEnginePrivate
    \inpublicgroup QtBaseModule

    \brief The QContentEnginePrivate class is the private class for QContentEngine.

    \internal
*/

/*!
    \enum QContentEngine::Attribute
    Flags indexing the attributes of a QContentEngine

    \value Categories \l categories()
    \value FileName \l fileName()
    \value Icon \l icon()
    \value Media \l media()
    \value Name \l name()
    \value Role \l role()
    \value MimeType \l mimeType()
    \value Properties \l property()
    \value Permissions \l permissions()
    \value Size \l size()
    \value ExecutableMimeTypes \l executableMimeTypes()
*/

/*!
    \typedef QContentEngine::Attributes
    Flags indexing the attributes of a QContentEngine.
*/

/*!
    Constructs a new QContentEngine with the d pointer \a dd and the engine mime type \a engineType.
*/
QContentEngine::QContentEngine( QContentEnginePrivate &dd, const QString &engineType )
    : d_ptr( &dd )
{
    d_ptr->engineType = engineType;
}

/*!
    Constructs a new QContentEngine with the engine mime type \a engineType.
*/
QContentEngine::QContentEngine( const QString &engineType )
    : d_ptr( new QContentEnginePrivate )
{
    d_ptr->engineType = engineType;
}

/*!
    Destroys a QContentEngine.
*/
QContentEngine::~QContentEngine()
{
    delete d_ptr;
}

/*!
    Populates a content engine with data from the content engine \a other.
*/
void QContentEngine::copy( const QContentEngine &other )
{
    Q_D(QContentEngine);

    d->name                = other.d_func()->name;
    d->fileName            = other.d_func()->fileName;
    d->mimeType            = other.d_func()->mimeType;
    d->media               = other.d_func()->media;
    d->iconName            = other.d_func()->iconName;
    d->role                = other.d_func()->role;
    d->drmState            = other.d_func()->drmState;
    d->lastUpdated         = other.d_func()->lastUpdated;
    d->loadedAttributes    = other.d_func()->loadedAttributes;
    d->dirtyAttributes     = other.d_func()->dirtyAttributes;
    d->size                = other.d_func()->size;
    d->icon                = other.d_func()->icon;
    d->categories          = other.d_func()->categories;
    d->propertyCache       = other.d_func()->propertyCache;
    d->permissions         = other.d_func()->permissions;
    d->translatedName      = other.d_func()->translatedName;
    d->mimeEngineData      = other.d_func()->mimeEngineData;
    d->isValid             = other.d_func()->isValid;

    d->error = false;
    d->errorString = QString();

}

/*!
    \fn QContentEngine::serialize(Stream &stream) const

    Writes the content engine data to a data \a stream.
*/
template <typename Stream> void QContentEngine::serialize(Stream &stream) const
{
    stream << d_func()->id;
    stream << d_func()->name;
    stream << d_func()->fileName;
    stream << d_func()->mimeType.id();
    stream << d_func()->media;
    stream << d_func()->iconName;
    stream << d_func()->role;
    stream << d_func()->drmState;
    stream << d_func()->lastUpdated;
    stream << (d_func()->loadedAttributes & ~(Icon | ExecutableMimeTypes | Validity));
    stream << d_func()->dirtyAttributes;
    stream << d_func()->size;
    stream << d_func()->categories;
    stream << d_func()->propertyCache;
    stream << d_func()->permissions;
    stream << d_func()->translatedName;
}

/*!
    \fn QContentEngine::deserialize(Stream &stream)

    Populates the content engine with data from a data \a stream.
*/
template <typename Stream> void QContentEngine::deserialize(Stream &stream)
{
    Q_D(QContentEngine);

    QString mimeId;

    stream >> d->id;
    stream >> d->name;
    stream >> d->fileName;
    stream >> mimeId; d->mimeType = QMimeType::fromId( mimeId );
    stream >> d->media;
    stream >> d->iconName;
    stream >> d->role;
    stream >> d->drmState;
    stream >> d->lastUpdated;
    stream >> d->loadedAttributes;
    stream >> d->dirtyAttributes;
    stream >> d->size;
    stream >> d->categories;
    stream >> d->propertyCache;
    stream >> d->permissions;
    stream >> d->translatedName;

    d->loadedAttributes &= ~(Icon | ExecutableMimeTypes | Validity);
    d->error = false;
    d->errorString = QString();
}

/*!
    \fn QContentEngine::id() const

    Returns the content id of the content.
*/

/*!
    \fn QContentEngine::engineType() const

    Returns the engine mime type of the content.
 */

/*!
    \fn QContentEngine::name() const

    Returns the name of the content.
 */

/*!
    \fn QContentEngine::fileName() const

    Returns the file name of the content.
 */

/*!
    \fn QContentEngine::mimeType() const

    Returns the mime type of the content.
 */

/*!
    \fn QContentEngine::iconName() const

    Returns the file name of the content's icon.
 */

/*!
    \fn QContentEngine::role() const

    Returns the data role of the content.
 */

/*!
    \fn QContentEngine::drmState() const

    Returns the DRM state of the represented content.
*/

/*!
    \fn QContentEngine::lastUpdated() const

    Returns the date and time the content was last updated.
*/

/*!
    Returns true if the content engine contains valid data.

    The results of validity checks are cached, if \a force is true the cached validity will be ignored and a new check run.
*/
bool QContentEngine::isValid( bool force ) const
{
    if( force || !(d_func()->loadedAttributes & Validity) )
    {
        QContentEnginePrivate *d = const_cast< QContentEnginePrivate * >( d_func() );

        d->isValid = const_cast< QContentEngine * >( this )->queryValidity();

        d->loadedAttributes |= Validity;
    }

    return d_func()->isValid;
}

/*!
    \fn QContentEngine::createCopy() const

    Constructs a copy of the content engine.

    This is used by the copy on write functionality of QContent.  All implementing classes must implement this individually.
*/

/*!
    \fn QContentEngine::isOutOfDate() const

    Returns true if the content meta-data stored in the database has potentially become out of sync with the
    contents of the backing file.
*/

/*!
    Returns the translated name of a QContent.
*/
const QString &QContentEngine::translatedName() const
{
    if( !(d_func()->loadedAttributes & TranslatedName) )
        const_cast< QContentEngine * >( this )->loadTranslatedName();

    return d_func()->translatedName;
}

/*!
    Sets the content id value of the content to \a id.
*/
void QContentEngine::setId( QContentId id )
{
    d_ptr->id = id;
}

/*!
    Sets the name value of the content to \a name.
*/
void QContentEngine::setName( const QString &name )
{
    Q_D(QContentEngine);

    d->name = name;

    d->dirtyAttributes |= Name;
    d->loadedAttributes &= ~TranslatedName;
}

/*!
    Sets the media value of the content to \a media.
*/
void QContentEngine::setMedia( const QString &media )
{
    Q_D(QContentEngine);

    d->media = media;

    d->dirtyAttributes |= Media;
}

/*!
    Sets the mime type value of the content to \a mimeType.
*/
void QContentEngine::setMimeType( const QMimeType &mimeType )
{
    Q_D(QContentEngine);

    d->mimeType = mimeType;

    d->dirtyAttributes |= MimeType;
}

void QContentEngine::setMimeTypes( const QStringList &mimeTypes, const QStringList& mimeTypeIcons, const QList< QDrmRights::Permission > permissions)
{
    Q_D(QContentEngine);

    d->mimeEngineData.clear();
    
    QMimeEngineData data;
    for(int i=0;i<mimeTypes.count();i++)
    {
        data.type=mimeTypes.at(i);
        data.icon= mimeTypeIcons.count() > i ? mimeTypeIcons.at(i) : d->iconName;
        data.application = d->fileName;
        data.permission = permissions.count() > i ? permissions.at(i) : QDrmRights::Unrestricted;
        d->mimeEngineData.append(data);
    }

    d->dirtyAttributes |= ExecutableMimeTypes;
}


/*!
    Sets the file name value of the the content to \a fileName.
*/
void QContentEngine::setFileName( const QString &fileName )
{
    Q_D(QContentEngine);

    d->fileName = fileName;

    d->dirtyAttributes |= FileName;
}

/*!
    Sets the DRM state value of the content to \a state.
*/
void QContentEngine::setDrmState( QContent::DrmState state )
{
    Q_D(QContentEngine);

    d->drmState = state;
}

/*!
    Sets the QContent role value of the content to \a role.
*/
void QContentEngine::setRole( QContent::Role role )
{
    Q_D(QContentEngine);

    d->role = role;

    d->dirtyAttributes |= Role;
}

/*!
    Sets the icon file name value of the content to \a name.
*/
void QContentEngine::setIconName( const QString &name )
{
    Q_D(QContentEngine);

    d->icon     = QIcon();
    d->iconName = name;

    d->dirtyAttributes |= Icon;
    d->loadedAttributes &= ~Icon;
}

/*!
    Sets the categories value of the content to \a categories.
*/
void QContentEngine::setCategories( const QStringList &categories )
{
    Q_D(QContentEngine);

    d->categories = categories;

    d->dirtyAttributes |= Categories;
    d->loadedAttributes |= Categories;
}

/*!
    Sets the \a date and time the content was last updated.
*/
void QContentEngine::setLastUpdated( const QDateTime &date )
{
    Q_D(QContentEngine);

    d->lastUpdated = date;
}

/*!
    Sets an error string \a errorString describing the last error to occur.
*/
void QContentEngine::setError( const QString &errorString )
{
    Q_D(QContentEngine);

    d->error = true;
    d->errorString = errorString;
}

/*!
    Returns a string describing the last error that occured.
*/
const QString &QContentEngine::errorString() const
{
    return d_func()->errorString;
}

/*!
    Returns the icon for the content.
*/
const QIcon &QContentEngine::icon() const
{
    if( !(d_func()->loadedAttributes & Icon) )
        const_cast< QContentEngine * >( this )->loadIcon();

    return d_func()->icon;
}

/*!
    Returns the ids of all categories the content belongs to.
*/
const QStringList &QContentEngine::categories() const
{
    if( !(d_func()->loadedAttributes & Categories) )
        const_cast< QContentEngine * >( this )->loadCategories();

    return d_func()->categories;
}

/*!
    Returns a list of all mime types the content can execute.
*/
const QList< QMimeEngineData > &QContentEngine::executableMimeTypes() const
{
    if( !(d_func()->loadedAttributes & ExecutableMimeTypes) && !(d_func()->dirtyAttributes & ExecutableMimeTypes) )
          const_cast< QContentEngine * >( this )->loadExecutableMimeTypes();

    return d_func()->mimeEngineData;
}

/*!
    Returns the valid DRM permissions for the content.

    Permissions are retrieved the first time this is called and subsequent calls will return a cached value, if \a force
    is true the cached value will be updated.
*/
QDrmRights::Permissions QContentEngine::permissions( bool force ) const
{
    if( force || !(d_func()->loadedAttributes & Permissions) )
    {
        QContentEnginePrivate *d = const_cast< QContentEnginePrivate * >( d_func() );

        d->permissions = const_cast< QContentEngine * >( this )->queryPermissions();

        d->loadedAttributes |= Permissions;
    }

    return d_func()->permissions;
}

/*!
    Returns the current rights for the rendering the content with the permission \a permission.
*/
QDrmRights QContentEngine::rights( QDrmRights::Permission permission ) const
{
    Q_UNUSED( permission );

    return QDrmRights();
}

/*!
    Returns the size of the content in bytes.

    The size is typically only retreived once with a cached value returned on subsequent calls., if \a force is true
    then the cached value will not be used.
*/
qint64 QContentEngine::size( bool force ) const
{
    if( force || !(d_func()->loadedAttributes & Size) )
    {
        QContentEnginePrivate *d = const_cast< QContentEnginePrivate * >( d_func() );

        d->size = const_cast< QContentEngine * >( this )->querySize();

        d->loadedAttributes |= Size;
    }

    return d_func()->size;
}

/*!
    \fn QContentEngine::open( QIODevice::OpenMode mode )

    Returns a pointer to a QIODevice opened in the IO mode \a mode which provides access the content data.

    If the device could not be opened a null pointer is returned.
*/

/*!
    \fn QContentEngine::moveTo( const QString &path )

    Moves the content engine's file to a new \a path.

    Note: Moving a content engine may invalidate its id.
*/

/*!
    \fn QContentEngine::copyTo( const QString &path )

    Creates a copy of the content engine's file at the file path \a path.
*/

/*!
    \fn QContentEngine::rename(const QString &name)

    Changes the \a name of the content and renames the associated file accordingly.
*/

/*!
    \fn QContentEngine::execute( const QStringList &arguments ) const

    If the content engine refers to an application executes the application with the given \a arguments.

    If the engine refers to a document launches the document in the default application responsible for handling it with
    the given \a arguments.
 */

/*!
    Returns true if the content is DRM protected and a license acquistion process can be initiated for it.
*/
bool QContentEngine::canActivate() const
{
    return false;
}

/*!
    Attempts to acquire a license to render DRM protected content with the permission \a permission, prior to rendering the
    content.

    Any dialogs presented to the user when attempting to acquire the license will be parented off the \a parent widget.

    This method will return true if there are valid rights to render with the given \a permission on completion, if the
    content is not DRM protected or already has valid rights this will return immediately.

    \sa reactivate(), permissions()
*/
bool QContentEngine::activate( QDrmRights::Permission permission, QWidget *parent )
{
    Q_UNUSED( permission );
    Q_UNUSED( parent );

    return true;
}

/*!
    Attempts to reacquire a license to render DRM protected content with the permission \a permission after rendering the
    content.

    Any dialogs presented to the user when attempting to acquire the license will be parented off the \a parent widget.

    This method will return true if there are valid rights to render with the given \a permission on completion, if the
    content is not DRM protected or already has valid rights this will return immediately.

    Unlike \l activate() which is called to ensure there are rights to content prior to rendering, reactivate() is called
    to notify the user that they will no longer be able to render expired or soon to expire content unless they acquire
    a new license.

    \sa activate(), permissions()
 */
bool QContentEngine::reactivate( QDrmRights::Permission permission, QWidget *parent )
{
    Q_UNUSED( permission );
    Q_UNUSED( parent );

    return true;
}

/*!
    Requests a content license to render DRM protected content with the permission \a permission.

    A content license must be requested and successfully granted before any read access will be given to
    the DRM protected data.  The content license is used meter the usage of DRM content and enforce any
    constraints applied.

    The license \a options determine the initial state of the content license.

    \sa activate(), reactivate(), permissions()
*/
QDrmContentLicense *QContentEngine::requestLicense( QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    Q_UNUSED( permission );
    Q_UNUSED( options );

    return 0;
}

/*!
    Deletes the represented content.
*/
bool QContentEngine::remove()
{
    return false;
}

/*!
    Returns the value of the property with the key \a key belonging to the group \a group.

    \sa setProperty(), propertyGroups(), propertyKeys()
*/
QString QContentEngine::property( const QString &group, const QString &key ) const
{
    if( !( d_func()->loadedAttributes & Properties ) )
        const_cast< QContentEngine * >( this )->loadProperties();

    return d_func()->propertyCache.value( group ).value( key );
}

/*!
    Returns a list of all property groups the content has properties for.

    \sa propertyKeys(), property()
*/
QStringList QContentEngine::propertyGroups() const
{
    if( !( d_func()->loadedAttributes & Properties ) )
        const_cast< QContentEngine * >( this )->loadProperties();

    return d_func()->propertyCache.keys();
}

/*!
    Returns a list of all property keys the content has properties for in the property group \a group.

    \sa propertyGroups(), property()
*/
QStringList QContentEngine::propertyKeys( const QString &group ) const
{
    if( !( d_func()->loadedAttributes & Properties ) )
        const_cast< QContentEngine * >( this )->loadProperties();

    return d_func()->propertyCache.value( group ).keys();
}

/*!
    Sets the value of the property with the key \a key belonging to the group \a group to \a value.
*/
void QContentEngine::setProperty( const QString &group, const QString &key, const QString &value )
{
    Q_D(QContentEngine);

    if( !(d->loadedAttributes & Properties ) )
        loadProperties();

    d->dirtyAttributes |= Properties;

    d->propertyCache[ group ][ key ] = value;
}

/*!
    \fn QContentEngine::querySize()

    Queries the size of the content in bytes.
 */

/*!
    Loads the icon for the represented content.
*/
void QContentEngine::loadIcon()
{
    Q_D(QContentEngine);

    d->icon = !iconName().isEmpty() ? QIcon( QLatin1String( ":icon/" ) + iconName() ) : QIcon();

    // Check if a valid icon was actually loaded.
    if( !d->icon.actualSize( QSize( 32, 32 ) ).isValid() )
        d->icon = QIcon();

    d->loadedAttributes |= Icon;
}

/*!
    Loads the ids of the categories the represented content belongs to from the database.
*/
void QContentEngine::loadCategories()
{
    Q_D(QContentEngine);

    d->categories = QContentStore::instance()->contentCategories( id() );

    d->loadedAttributes |= Categories;
}

/*!
    Loads the properties of the represented content from the database.
*/
void QContentEngine::loadProperties()
{
    Q_D(QContentEngine);

    QMapIterator< QString, QContentEngineGroupCache > i(QContentStore::instance()->contentProperties( id() ));
    while (i.hasNext()) {
        i.next();
        d->propertyCache.insert( i.key(), i.value() );
    }

    d->loadedAttributes |= Properties;
}

/*!
    Loads a translation of the the content name in the current language.
*/
void QContentEngine::loadTranslatedName()
{
    Q_D(QContentEngine);

    if( d->role == QContent::Application || d->role == QContent::Folder )
    {
        QString trFile = property( QLatin1String( "Translation" ), QLatin1String( "File" ) );
        QString trContext = property( QLatin1String( "Translation" ), QLatin1String( "Context" ) );

        if( !trFile.isEmpty() && !trContext.isEmpty() )
            d->translatedName = Qtopia::translate( trFile, trContext, d->name );
        else
            d->translatedName = d->name;
    }
    else
        d->translatedName = d->name;
}

/*!
    Loads a list of the mime types the content can execute from the database.
*/
void QContentEngine::loadExecutableMimeTypes()
{
    Q_D(QContentEngine);

    d->mimeEngineData = QContentStore::instance()->associationsForApplication(fileName());

    d->loadedAttributes |= ExecutableMimeTypes;

}

/*!
    Queries the current permissions for the content.
*/
QDrmRights::Permissions QContentEngine::queryPermissions()
{
    return QDrmRights::Unrestricted;
}

/*!
    Returns the attributes that have changed since the content was last commited.

    \sa clearDirtyAttributes()
*/
QContentEngine::Attributes QContentEngine::dirtyAttributes() const
{
    return d_func()->dirtyAttributes;
}

/*!
    Clears the dirty attributes flags for the content.
*/
void QContentEngine::clearDirtyAttributes()
{
    Q_D(QContentEngine);

    d->dirtyAttributes = Attributes();
}

/*!
    Constructs a new content engine for a file with the file name \a fileName.

    If no content engine handler identifies with the file a null pointer will be returned.
*/
QContentEngine *QContentEngine::createEngine( const QString &fileName )
{
    QContentEngine *engine = 0;
    {
        QMutexLocker locker(contentEngineHandlerMutex());

        for( int i = 0; i < contentEngineHandlers()->size(); i++ )
        {
            if( (engine = contentEngineHandlers()->at( i )->create( fileName )) )
                break;
        }
    }
    return engine;
}

/*!
    Constructs a new content engine from a template content engine \a content.

    If no content engine handler identifies with the content a null pointer will be returned.

    \sa QContentEngineHandler
*/
QContentEngine *QContentEngine::createEngine( const QContent &content )
{
    QContentEngine *engine = 0;
    {
        QMutexLocker locker(contentEngineHandlerMutex());

        for( int i = 0; i < contentEngineHandlers()->size(); i++ )
        {
            if( (engine = contentEngineHandlers()->at( i )->create( content )) )
                break;
        }
    }
    return engine;
}

/*!
    Returns the storage media of the content.
 */
const QString &QContentEngine::media() const
{
    if( d_ptr->media.isEmpty() )
    {
        d_ptr->media = QFileSystem::fromFileName( d_ptr->fileName ).path();
    }
    return d_ptr->media;
}

/*!
    Writes a description of the content engine \a engine to a \a debug stream.

    \internal
*/
QDebug operator <<( QDebug debug, const QContentEngine &engine )
{
    return debug
            << "QContentEngine("
            << "Id:" << QString( "%1|%2" )
                .arg( engine.id().first, 4, 16, QLatin1Char( '0' ) )
                .arg( engine.id().second, 8, 16, QLatin1Char( '0' ) )
            << "Name:" << engine.name()
            << "FileName:" << engine.fileName()
            << "MimeType:" << engine.mimeType().id()
            << "DrmState:" << engine.drmState()
            << "Role:" << engine.role()
            << "IconName:" << engine.iconName()
            << "LastUpdated:" << engine.lastUpdated()
            << ")";
}

/*!
    Serializes a content \a engine to a data \a stream.

    \internal
*/
QDataStream &operator <<( QDataStream &stream, const QContentEngine &engine )
{
    engine.serialize( stream );

    return stream;
}

/*!
    Deserializes a content \a engine from a data \a stream.

    \internal
*/
QDataStream &operator >>( QDataStream &stream, QContentEngine &engine )
{
    engine.deserialize( stream );

    return stream;
}

#if defined(QTOPIA_DBUS_IPC) && !defined(QTOPIA_HOST)

/*!
    Serializes a content \a engine to a data \a stream.

    \internal
*/
QDBusArgument &operator <<( QDBusArgument &stream, const QContentEngine &engine )
{
    engine.serialize( stream );

    return stream;
}

/*!
    Deserializes a content \a engine from a data \a stream.

    \internal
*/
const QDBusArgument &operator >>( const QDBusArgument &stream, QContentEngine &engine )
{
    engine.deserialize( stream );

    return stream;
}

#endif

Q_IMPLEMENT_USER_METATYPE_ENUM(QContentEngine::Attributes);

/*!
    \class QContentEngineHandler
    \inpublicgroup QtBaseModule

    \brief The QContentEngineHandler class is the base class for classes that create QContentEngine instance.

    \sa QContentEngine

    \internal
*/

/*!
    Constructs a new QContentEngineHandler and registers it with the list of active engine handlers.
*/
QContentEngineHandler::QContentEngineHandler()
{
    QMutexLocker locker(contentEngineHandlerMutex());
    contentEngineHandlers()->prepend(this);
}

/*!
    Destroys and unregisters a QContentEngineHandler.
*/
QContentEngineHandler::~QContentEngineHandler()
{
    QMutexLocker locker(contentEngineHandlerMutex());

    if (!qt_contentenginehandlerlist_shutDown)
        contentEngineHandlers()->removeAll(this);
}

/*!
    \fn QContentEngineHandler::create( const QString &fileName ) const

    Constructs and populates a new QContentEngine for the file with the file name \a fileName.
*/

/*!
    \fn QContentEngineHandler::create( const QContent &content ) const

    Constructs and populates a new QContentEngine based on the contents of the template \a content.
*/

/*!
    \fn void QMimeEngineData::serialize(Stream &stream) const

    Serializes QMimeEngineData to a data \a stream.

    \internal
*/
template <typename Stream> void QMimeEngineData::serialize(Stream &stream) const
{
    stream << type << icon << application << permission;
}

/*!
    \fn void QMimeEngineData::deserialize(Stream &stream)

    Deserializes QMimeEngineData from a data \a stream.

    \internal
*/
template <typename Stream> void QMimeEngineData::deserialize(Stream &stream)
{
    stream >> type >> icon >> application >> permission;
}
