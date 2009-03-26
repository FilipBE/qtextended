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

#include "qmimetype.h"
#include <qtopiaglobal.h>
#include <qcontent.h>
#include <qcontentset.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#else
#include <QApplication>
#include <qtopianamespace.h>
#endif

#include <QHash>
#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>
#include <QPixmapCache>
#include <QDir>
#include <QMutexLocker>
#include <QStyle>
#include <QSettings>
#include <QtGlobal>
#include <qtopialog.h>
#include "qmimetypedata_p.h"
#include "qcontentstore_p.h"

#include "drmcontent_p.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef QHash<QString, QString> typeForType;
Q_GLOBAL_STATIC(typeForType, typeFor);
typedef QHash<QString, QStringList> extForType;
Q_GLOBAL_STATIC(extForType, extFor);
typedef QHash<QString, QDateTime> loadedTimesType;
Q_GLOBAL_STATIC(loadedTimesType, loadedTimes);

QMutex QMimeType::staticsGuardMutex(QMutex::Recursive);

typedef QCache<QString,QMimeTypeData> QMimeTypeDataCache;

Q_GLOBAL_STATIC(QMimeTypeDataCache,mimeTypeDataCache);

/*!
    \class QMimeType
    \inpublicgroup QtBaseModule

    \brief The QMimeType class provides MIME type information.

    A QMimeType object is a light-weight value which
    provides MIME type information.

    \ingroup io
*/

/*!
    \enum QMimeType::IconType
    IconType describes the properties of the content an icon represents.

    \value Default The icon is displayed for content with no special properties.
    \value DrmValid The icon is displayed for drm protected content with current valid rights.
    \value DrmInvalid The icon is displayed for drm protected content with no current valid rights.
*/

/*!
    Constructs a null QMimeType.
*/
QMimeType::QMimeType()
{
}

/*!
    Constructs a QMimeType object. If the parameter \a ext_or_id is a MIME type, it
    is interpreted as a filename if all of the following conditions hold:
    \list
        \o it is not in the known list of MIME types
        \o if either QFile( \a ext_or_id ).exists() is true, or if ext_or_id starts with a '/'
            or contains no internal '/'
    \endlist

    Otherwise, if it is interpreted as a file name, the file extension (ie, for /foo/bar.txt this will
    be .txt) is used to determine the MIME type. If it does not appear to be a registered file
    extension, or a valid file, then it is treated as an unknown MIME type.
*/
QMimeType::QMimeType( const QString& ext_or_id )
{
    init(ext_or_id);
}

/*!
    Constructs a QMimeType from the \l {QContent::}{type()} of \a lnk.
*/
QMimeType::QMimeType( const QContent& lnk )
{
    init(lnk.type());
}

/*!
    Constructs a copy of the QMimeType \a other.
*/
QMimeType::QMimeType( const QMimeType &other )
    : mimeId( other.mimeId )
{
}

/*!
    Assigns the value of the QMimeType \a other to a mime type.
*/
QMimeType &QMimeType::operator =( const QMimeType &other )
{
    mimeId = other.mimeId;

    return *this;
}

/*!
    Compares the mime type with the QMimeType \a other and returns true if they are equal.
*/
bool QMimeType::operator ==( const QMimeType &other ) const
{
    return mimeId == other.mimeId;
}

/*!
    Constructs a QMimeType from the mime type ID string \a mimeId.

    This does not perform any validation of the mime type ID, if validation is required consider using the QMimeType() constructor
    instead.
*/
QMimeType QMimeType::fromId( const QString &mimeId )
{
    QMimeType type;

    type.mimeId = mimeId;

    return type;
}

/*!
    Constructs a QMimeType from a file \a extension.

    To note, do not include the period.
*/
QMimeType QMimeType::fromExtension( const QString &extension )
{
    if(loadedTimes()->count() == 0)
        loadExtensions();

    QMimeType type;

    type.mimeId = typeFor()->value( extension.toLower() );

    return type;
}

/*!
    Constructs a QMimeType from a \a fileName.
*/
QMimeType QMimeType::fromFileName( const QString &fileName )
{
    QMimeType type;

    int slashIndex = fileName.lastIndexOf( QLatin1Char( '/' ) );

    int dotIndex = fileName.lastIndexOf( QLatin1Char( '.' ) );

    if( dotIndex > slashIndex )
    {
        if(loadedTimes()->count() == 0)
            loadExtensions();
        QString extension = fileName.mid( dotIndex + 1 ).toLower();

        type.mimeId = typeFor()->value( extension );
    }

    if( type.mimeId.isEmpty() )
    {
        const char elfMagic[] = { '\177', 'E', 'L', 'F', '\0' };

        QFile ef( fileName );

        if ( ef.size() > 5 && ef.open( QIODevice::ReadOnly ) && ef.peek(5) == elfMagic)  // try to find from magic
            type.mimeId = QLatin1String("application/x-executable");  // could be a shared library or an exe
        else
            type.mimeId = QLatin1String("application/octet-stream");
    }

    return type;
}

/*!
    Returns true if the QMimeType is null.
*/
bool QMimeType::isNull() const
{
    return mimeId.isEmpty();
}

/*!
    Returns the MIME type identifier. eg. image/jpeg
*/
QString QMimeType::id() const
{
    return mimeId;
}

/*!
    Returns a description of the MIME Type. This is usually based
    on the application() associated with the type.
*/
QString QMimeType::description() const
{
#ifndef QTOPIA_CONTENT_INSTALLER
    return QtopiaApplication::tr("%1 document").arg( data(mimeId).defaultApplication().name() );
#else
    return QString();
#endif
}

/*!
    Returns an \a iconType icon appropriate for the MIME type.
*/
QIcon QMimeType::icon( IconType iconType ) const
{
    Q_UNUSED( iconType );

    QMimeTypeData mtd = data(mimeId);

    switch( iconType )
    {
    case Default:
        return mtd.icon( mtd.defaultApplication() );
    case DrmValid:
        return mtd.validDrmIcon( mtd.defaultApplication() );
    case DrmInvalid:
        return mtd.invalidDrmIcon( mtd.defaultApplication() );
    default:
        return QIcon();
    }
}


/*!
    Returns the first extension associated with the MIME type.
*/
QString QMimeType::extension() const
{
    QStringList exts = extensions();
    if (exts.count())
        return extensions().first();
    return QString();
}

/*!
    Returns the list of file extensions associated with the MIME type.
*/
QStringList QMimeType::extensions() const
{
    if(loadedTimes()->count() == 0)
        loadExtensions();

    return extFor()->value(mimeId);
}

/*!
    Returns a list of QContent objects of the applications associated
    with this MIME type, or an empty list if none is associated.
*/
QContentList QMimeType::applications() const
{
    return applicationsFor(*this);
}

/*!
    Returns the QContent of the default application associated
    with this MIME type, or an invalid QContent if none is associated.

    \sa QtopiaService::binding()
*/
QContent QMimeType::application() const
{
    return defaultApplicationFor(*this);
}

/*!
    Returns the permission type used by the associated application to open
    a file of this MIME type.  If no application is associated or the application
    does not support DRM, then QDrmRights::Unrestricted is returned.
*/
QDrmRights::Permission QMimeType::permission() const
{
    QMimeTypeData mtd = data(mimeId);

    return mtd.permission( mtd.defaultApplication() );
}

/*!
    Returns the permissions used by the associated applications.
    Permissions are mapped one to one with the list returned by applications().
*/
QList< QDrmRights::Permission > QMimeType::permissions() const
{
    return QList< QDrmRights::Permission >();
}

/*!
    \internal
    Clears out the internal structures, to force a reload of the information.
*/
void QMimeType::clear()
{
    QMutexLocker staticsGuard(&staticsGuardMutex);
    extFor()->clear();
    typeFor()->clear();
    loadedTimes()->clear();
    mimeTypeDataCache()->clear();
}

/*!
    \internal
    Loads the information from /etc/mime.types from all installpaths, and the root directory
*/
void QMimeType::loadExtensions()
{
    QStringList paths;
    paths << QLatin1String("/") << Qtopia::installPaths();
    const int CheckIntervalSeconds = 60 * 10;

    const QString etcMimeTypesConst = QLatin1String("etc/mime.types");

    QFileInfo fi;
    foreach(QString path, paths)
    {
        QString file = QDir::cleanPath(path+etcMimeTypesConst);
        if(!loadedTimes()->contains(path) || loadedTimes()->value(path).secsTo(QDateTime::currentDateTime()) > CheckIntervalSeconds)
        {
            fi.setFile(file);
            if(fi.exists() && fi.created() > loadedTimes()->value(path))
            {
                loadExtensions(file);
                loadedTimes()->insert(path, fi.created());
            }
        }
    }
}

/*!
    \internal
    Returns the next word, given the input and starting position.
*/
static QString nextString( const char *line, int& posn )
{
    if ( line[posn] == '\0' )
        return QString::null;
    int end = posn;
    char ch;
    for (;;) {
        ch = line[end];
        if ( ch == '\0' || ch == ' ' || ch == '\t' ||
             ch == '\r' || ch == '\n' ) {
            break;
        }
        ++end;
    }
    const char *result = line + posn;
    int resultLen = end - posn;
    for (;;) {
        ch = line[end];
        if ( ch == '\0' )
            break;
        if ( ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' )
            break;
        ++end;
    }
    posn = end;
    return QString::fromLocal8Bit(result, resultLen);
}

/*!
    \internal
    Loads the information from the /etc/mime.types passed as \a filename.
*/
void QMimeType::loadExtensions(const QString& filename)
{
    QMutexLocker staticsGuard(&staticsGuardMutex);
    QFile file(filename);
    if ( file.open(QIODevice::ReadOnly) ) {
        char line[1024];

        while (file.readLine(line, sizeof(line)) > 0) {
            if (line[0] == '\0' || line[0] == '#')
                continue;
            int posn = 0;
            QString id = nextString(line, posn);
            if ( id.isEmpty() )
                continue;
            id = id.toLower();

            QStringList exts = extFor()->value(id);

            for( QString ext = nextString( line, posn ); !ext.isEmpty(); ext = nextString(line, posn).toLower() )
            {
                if( !exts.contains( ext ) )
                {
                    exts.append( ext );

                    typeFor()->insert(ext, id);
                }
            }
            (*extFor())[ id ] = exts;
        }
        // todo: update the database view, and send out a signal about notifying the doc system of mimetype updates.
    }
}

/*!
    \internal
    Offload common initialisation work for the given file extension or filename passed via \a ext_or_id.
*/

void QMimeType::init( const QString& ext_or_id )
{
    if (ext_or_id.isEmpty())
        return;

    if(loadedTimes()->count() == 0)
        loadExtensions();
    QString mime_sep = QLatin1String("/");
    // either it doesnt have exactly one mime-separator, or it has
    // a path separator at the beginning
    bool doesntLookLikeMimeString =
        ext_or_id.count( mime_sep ) != 1 ||
        ext_or_id[0] == QDir::separator();

    // do a case insensitive search for a known mime type.
    QString lwrExtOrId = ext_or_id.toLower();
    QHash<QString,QStringList>::const_iterator it = extFor()->find(lwrExtOrId);
    if ( it != extFor()->end() ) {
        mimeId = lwrExtOrId;
    } else if( ext_or_id.startsWith( "Service/" ) || ext_or_id.startsWith( "Ipc/" ) || ext_or_id.startsWith( "Folder/" ) ) {
        mimeId = ext_or_id;
    } else if ( doesntLookLikeMimeString || QFile(ext_or_id).exists() ) {
        QFile ef(ext_or_id);
        int dot = ext_or_id.lastIndexOf('.');
        QString ext = dot >= 0 ? ext_or_id.mid(dot+1) : ext_or_id;
        mimeId = typeFor()->value(ext.toLower());
        const char elfMagic[] = { '\177', 'E', 'L', 'F', '\0' };
        if ( mimeId.isNull() && ef.exists() && ef.size() > 5 && ef.open(QIODevice::ReadOnly) && ef.peek(5) == elfMagic)  // try to find from magic
            mimeId = QLatin1String("application/x-executable");  // could be a shared library or an exe
        if ( mimeId.isNull() )
            mimeId = QLatin1String("application/octet-stream");
    }
    else  // could be something like application/vnd.oma.rights+object
    {
        mimeId = lwrExtOrId;
    }
}

/*!
    \internal
    Accessor for the internal singleton, returns the QMimeTypeData associated with the MIME type \a id.
*/
QMimeTypeData QMimeType::data(const QString& id)
{
    QMutexLocker staticsGuard(&staticsGuardMutex);

    QMimeTypeData data;

    if( mimeTypeDataCache()->contains( id ) )
    {
        data = *mimeTypeDataCache()->object( id );
    }
    else
    {
        data = QContentStore::instance()->mimeTypeFromId( id );

        if( !data.id().isEmpty() )
            mimeTypeDataCache()->insert( id, new QMimeTypeData( data ) );
    }

    if( data.applications().count() == 0 )
    {
        QString majorType = id.left( id.indexOf( '/' ) ) + QLatin1String( "/*" );

        if( mimeTypeDataCache()->contains( majorType ) )
        {
            data = *mimeTypeDataCache()->object( majorType );
        }
        else
        {
            data = QContentStore::instance()->mimeTypeFromId( majorType );

            if( !data.id().isEmpty() )
                mimeTypeDataCache()->insert( majorType, new QMimeTypeData( data ) );
        }
    }

    return data;
}

/*!
    \internal
    Reloads application definitions.
*/
void QMimeType::updateApplications()
{
    QMutexLocker staticsGuard(&staticsGuardMutex);

    mimeTypeDataCache()->clear();
}

/*!
    Adds an association to the system between a \a mimeType and an \a application, with \a icon and \a permission.
*/
void QMimeType::addAssociation(const QString& mimeType, const QString& application, const QString& icon, QDrmRights::Permission permission)
{
    QContentStore::instance()->addAssociation( mimeType, application, icon, permission );
    if( mimeTypeDataCache()->contains( mimeType ) )
        mimeTypeDataCache()->remove( mimeType );
}

/*!
    Removes an association from the system between a \a mimeType and an \a application.
*/
void QMimeType::removeAssociation(const QString& mimeType, const QString& application)
{
    QContentStore::instance()->removeAssociation( mimeType, application );
    if( mimeTypeDataCache()->contains( mimeType ) )
        mimeTypeDataCache()->remove( mimeType );
}

/*!
    Returns a list of applications that have registered associations with the mimetype of the \a content object.
*/
QContentList QMimeType::applicationsFor(const QContent& content)
{
    QContentList result(data(content.type()).applications());
    if(content.type().contains('/')  && !content.type().contains("/*"))
        result += data(content.type().section('/', 0, 0) + "/*").applications();
    return result;
}

/*!
    Returns a list of applications that have registered associations with the \a mimeType.
*/
QContentList QMimeType::applicationsFor(const QMimeType& mimeType)
{
    QContentList result(data(mimeType.id()).applications());
    if(mimeType.id().contains('/') && !mimeType.id().contains("/*"))
        result += data(mimeType.id().section('/', 0, 0) + "/*").applications();
    return result;
}

/*!
    Returns the system marked default from the list of applications that have registered associations with the mimetype of the \a content object.
*/
QContent QMimeType::defaultApplicationFor(const QContent& content)
{
    if(data(content.type()).defaultApplication().id() == QContent::InvalidId && applicationsFor(content).count() != 0)
        setDefaultApplicationFor(content.type(), applicationsFor(content).at(0));
    return data(content.type()).defaultApplication();
}

/*!
    Returns the system marked default from the list of applications that have registered associations with the \a mimeType.
*/
QContent QMimeType::defaultApplicationFor(const QMimeType& mimeType)
{
    if(data(mimeType.id()).defaultApplication().id() == QContent::InvalidId && applicationsFor(mimeType).count() != 0)
        setDefaultApplicationFor(mimeType.id(), applicationsFor(mimeType).at(0));
    return data(mimeType.id()).defaultApplication();
}

/*!
    Set the system marked default from the list of applications that have registered associations with the \a mimeType to the \a content object.
*/
void QMimeType::setDefaultApplicationFor(const QString& mimeType, const QContent& content)
{
    QContentStore::instance()->setDefaultApplicationFor( mimeType, content.fileName() );
    if( mimeTypeDataCache()->contains( mimeType ) )
        mimeTypeDataCache()->remove( mimeType );
}

#include "qmimetype.moc"
