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

#include "qdrmcontentplugin.h"
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qthumbnail.h>
#include "drmcontent_p.h"
#include <QThreadStorage>
/*
    Generic QAbstractFileEngine implementation providing access to content using
    QDrmContentPlugin::createDecoder().
*/
class DefaultDrmFileEngine : public QFSFileEngine
{
public:
    DefaultDrmFileEngine( const QString &file, QDrmContentPlugin *plugin )
        : QFSFileEngine( file )
        , plugin( plugin )
        , io( 0 )
    {
    }

    virtual ~DefaultDrmFileEngine()
    {
        if( io )
            delete io;
    }

    virtual bool close()
    {
        if( io && io->isOpen() )
        {
            io->close();

            return true;
        }
        else
            return false;
    }

    virtual bool copy( const QString & newName )
    {
        close();

        bool error = false;
        if(!QFSFileEngine::open(QFile::ReadOnly)) {
            QString errorMessage = QLatin1String("Cannot open %1 for input");
            setError(QFile::CopyError, errorMessage.arg(QFSFileEngine::fileName(DefaultName)));
            error = true;
        } else {
            QString fn = Qtopia::tempName(newName);
            QFile out( fn );
            if(!out.open( QIODevice::WriteOnly )) {
                close();
                setError(QFile::CopyError, QLatin1String("Cannot open for output"));
                error = true;
            } else {
                char block[1024];
                while(true) {
                    qint64 in = QFSFileEngine::read(block, 1024);
                    if(in <= 0)
                        break;
                    if(in != out.write(block, in)) {
                        setError(QFile::CopyError, QLatin1String("Failure to write block"));
                        error = true;
                        break;
                    }
                }
                QFSFileEngine::close();
                out.close();

                if( error || !out.rename(newName) ) {
                    out.remove();
                    QString errorMessage = QLatin1String("Cannot create %1 for output");
                    setError(QFile::CopyError, errorMessage.arg(newName));
                    error = true;
                }
            }
        }
        if(!error) {
            int flags = QFSFileEngine::fileFlags(FileInfoAll) & PermsMask;
            QFile::setPermissions(newName, QFile::Permissions( flags ) );
            return true;
        }
        return false;
    }

    virtual bool mkdir( const QString & dirName, bool createParentDirectories ) const
    {
        Q_UNUSED( dirName );
        Q_UNUSED( createParentDirectories );

        return false;
    }

    virtual bool open( QIODevice::OpenMode mode )
    {
        if( plugin && !io )
        {
            QString absoluteName = fileName( AbsoluteName );

            QDrmContentLicense *license = plugin->license( absoluteName );

            io = license ? plugin->createDecoder( absoluteName, license->permission() ) : 0;

            return io ? io->open( mode | QIODevice::Unbuffered ) : false;
        }
        else if( io && !io->isOpen() )
            return io->open( mode | QIODevice::Unbuffered );
        else
            return false;
    }

    virtual qint64 pos() const
    {
        return io ? io->pos() : -1;
    }

    virtual qint64 read( char * data, qint64 maxlen )
    {
        return io ? io->read( data, maxlen ) : -1;
    }

    virtual bool seek ( qint64 offset )
    {
        return io ? io->seek( offset ) : false;
    }

    virtual bool setPermissions( uint permissions )
    {
        Q_UNUSED( permissions );

        return false;
    }

    virtual bool setSize( qint64 size )
    {
        Q_UNUSED( size );

        return false;
    }

    virtual qint64 size() const
    {
        return io ? io->size() : QFSFileEngine::size();
    }

    virtual qint64 write( const char *data, qint64 len )
    {
        Q_UNUSED( data );
        Q_UNUSED( len );

        return -1;
    }

private:
    QDrmContentPlugin *plugin;
    QIODevice *io;
};


/*!
    \class QDrmContentLicense
    \inpublicgroup QtBaseModule

    \brief The QDrmContentLicense class provides an interface for DRM agents to monitor the consumption of DRM content.

    A DRM content license provides the interface through which a consuming application will notify a DRM agent
    of when consumption of an item of content begins, ends, and pauses.

    Before opening a DRM protected file an application will request an instance of QDrmContentLicense from a DRM
    agent. If a license is sucessfully granted the application will attempt to open the file and begin making calls to
    \l QDrmContentLicense::renderStateChanged() reflecting the consumption of the content.  If the content's rights
    expire while the content is being rendered the content license will emit the \l QDrmContentLicense::rightsExpired()
    signal.

    Applications should not request a content license directly, instead licenses should be requested using \l QDrmContent.

    \sa QDrmContent
    \sa QDrmContentPlugin

    \ingroup drm
 */

/*!
    Constructs a ContentLicense object for an item of DRM protected content.
 */
QDrmContentLicense::QDrmContentLicense()
{
    qLog(DRMAgent) << "QDrmContentLicense created" << qApp->applicationName();
}

/*!
    Destroys a QDrmContentLicense object. If the paired monitor is
    still consuming the content it is sent a message to release the content.
*/
QDrmContentLicense::~QDrmContentLicense()
{
    qLog(DRMAgent) << "QDrmContentLicense destroyed" << qApp->applicationName();
}

/*!
    \fn QDrmContentLicense::renderStateChanged( const QContent &content, QDrmContent::RenderState state )

    Notifies the drm agent that the render state of  \a content has changed to \a state and the
    rights should be updated accordingly.

   \sa renderState()
*/

/*!
    \fn QDrmContentLicense::rightsExpired( const QContent &content, QDrmRights::Permission permission )

    Signals that the license to render \a content with the given \a permission has expired.
*/

/*!
    \fn QDrmContentLicense::content() const

    Returns the content the license has been granted for.
*/

/*!
    \fn QDrmContentLicense::permission() const

    Returns the usage permission the license has been granted for.
*/

/*!
    \fn QDrmContentLicense::renderState() const

    Returns the current render state of the content the license has been granted for.

    \sa renderStateChanged()
*/

class QDrmContentPluginLicenseStore : public QObject
{
    Q_OBJECT
public:
    QDrmContentPluginLicenseStore( QObject *parent = 0 );

    void insertLicense( QDrmContentLicense *license );
    QDrmContentLicense *license( const QString &fileName );

private slots:
    void licenseDestroyed( QObject *object );

private:
    QHash< QString, QDrmContentLicense * > m_licenses;
};

QDrmContentPluginLicenseStore::QDrmContentPluginLicenseStore( QObject *parent )
    : QObject( parent )
{
}

void QDrmContentPluginLicenseStore::insertLicense( QDrmContentLicense *license )
{
    m_licenses.insert( license->content().fileName(), license );

    connect( license, SIGNAL(destroyed(QObject*)), this, SLOT(licenseDestroyed(QObject*)) );
}

QDrmContentLicense *QDrmContentPluginLicenseStore::license( const QString &fileName )
{
    return m_licenses.value( fileName );
}

void QDrmContentPluginLicenseStore::licenseDestroyed( QObject *object )
{
    QHash< QString, QDrmContentLicense * >::iterator it;
    for( it = m_licenses.begin(); it != m_licenses.end(); it++ )
    {
        if( it.value() == object )
        {
            m_licenses.erase( it );

            return;
        }
    }
}

class QDrmContentPluginPrivate
{
public:
    QThreadStorage< QDrmContentPluginLicenseStore * > licenses;
};

/*!
    \class QDrmContentPlugin
    \inpublicgroup QtBaseModule

    \brief The QDrmContentPlugin class is the primary interface between DRM agents and the Qt Extended document system.

    QDrmContentPlugin allows the document system to present the license state and meta-info of DRM protected files,
    provide activation options and provide metered access to the plain-text content of those files.

    \section1 DRM Content Recognition and Installation

    When the document system discovers new content on a device it will attempt to determine if it is DRM protected by
    calling \l installContent() for each DRM content plugin installed. If a plugin recognizes the content as DRM protected
    it should populate the passed QContent with meta-data from the files and return true which will mark the content as being
    DRM protected.

    Once a QContent has been identified as being DRM protected the document system will identify the plugin by looking up the
    file extension in the list returned by \l keys().  If a file passed to \l installContent() is DRM protected but does not have
    a correct extension it should be renamed by the plugin when it is installed.  Likewise if a DRM format has separate transport
    and storage formats the plugin should convert from the transport format to the storage format when it is installed.

    \section1 DRM File Access

    The document system requires applications to request a license through QDrmContent before they can access the plain-text
    content of a DRM protected file.  When a license is requested the document system first checks the current permissions of
    the file, if permission the license is requested for is not valid the document system will attempt to acquire rights for
    the file by calling \l activate().  If the file permissions were already valid or the activation was successful the document
    system will then attempt to acquire an instance of QDrmContentLicense by calling \l requestContentLicense() which will be
    used to update the files license constraints.  Content licenses can be registered by calling \l registerLicense() and once a
    license has been registered it can be retrieved using \l license(), deleted licenses are automatically unregistered.
    QDrmContentPlugin inherits from QAbstractFileEngine and its implementation  of \l create() constructs a file engine which 
    will allow application to open a DRM protected file in read-only mode if a content license for that file has been registered
    with the plugin, the file will be opened using a QIODevice created by \l createDecoder() with the permission the license 
    was requested for.  When a license is released the document system will call \l reactivate() irregardless of the current
    permissions of the file, if rights have expired or will expire in the near future a plugin may prompt the user to acquire
    new rights.

    \sa QDrmContent

    \ingroup drm
    \ingroup plugins
*/

/*!
    Constructs a new drm content plug-in with the given \a parent.
*/
QDrmContentPlugin::QDrmContentPlugin( QObject *parent )
    : QObject( parent )
{
    d = new QDrmContentPluginPrivate;
}


/*!
    Destroys a QDrmContentPlugin object.
*/
QDrmContentPlugin::~QDrmContentPlugin()
{
    delete d;

    qLog(DRMAgent) << "QDrmContentPlugin destroyed" << qApp->applicationName();
}

/*!
    \fn QDrmContentPlugin::keys() const

    Returns the file extensions of all DRM file formats supported.  This is used to identify the DRM plugin that should
    handle a file type.
*/

/*!
    \fn QDrmContentPlugin::types() const

    Returns the mime types of all DRM file formats supported by the plugin.  These types are set in the accept headers
    of DRM enabled web applications.
*/

/*!
    Returns a list of HTTP header key/value pairs DRM enabled web applications should advertise to indicate support for the DRM
    content handled by the plugin.
*/
QList< QPair< QString, QString > > QDrmContentPlugin::httpHeaders() const
{
    return QList< QPair< QString, QString > >();
}

/*!
    \fn QDrmContentPlugin::isProtected( const QString &fileName ) const

    Returns true if the file with the file name \a fileName is DRM protected.
*/

/*!
    \fn QDrmContentPlugin::permissions( const QString &fileName )

    Returns the current valid permissions of the DRM protected content with the file name \a fileName.
 */

/*!
    \fn QDrmContentPlugin::getRights( const QString &fileName, QDrmRights::Permission permission )

    Returns the rights associated with the drm protected file with the file name \a fileName for the given \a permission.
 */

/*!
    Requests a content license for rendering \a content with the given \a permission.  If the content has valid and
    current rights a pointer to a new content license is returned otherwise a null pointer is returned.  The initial
    state of the content license is determined by the license \a options.

    Implementing classes should register new content licenses with the plugin using \l registerLicense(), the content licenses
    are unregistered automatically when destroyed.  Once a license has been granted and registered with the plugin access to
    the unencrypted content of the DRM protected file will become available through the Qt file API for the duration of the
    license's lifetime.
 */
QDrmContentLicense *QDrmContentPlugin::requestContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    Q_UNUSED( content    );
    Q_UNUSED( permission );
    Q_UNUSED( options    );

    return 0;
}

/*!
    Returns a new QIODevice which provides access to the decrypted content of the DRM file with the file name \a fileName. If
    the file cannot be opened for rendering with the given \a permission a null pointer will be returned instead.

    The returned QIODevice will not update any license constraints.  In order to update the constraints with the correct
    usage a content license should be requested using \l requestContentLicense() and explicitly updated with the usage.

    \sa requestContentLicense()
 */
QIODevice *QDrmContentPlugin::createDecoder( const QString &fileName, QDrmRights::Permission permission )
{
    Q_UNUSED( fileName   );
    Q_UNUSED( permission );

    return 0;
}

/*!
    \fn QDrmContentPlugin::canActivate( const QString &fileName )

    Indicates whether the user should be given a menu option to activate the DRM protected content with the file name
    \a fileName.

    Returns true if the content with the file name \a fileName can be activated.

    \sa activate()
*/

/*!
    \fn QDrmContentPlugin::activate( const QContent &content, QWidget *focus )

    Initiates an attempt to acquire rights for the given DRM protected \a content in response to the user selecting an
    activation menu option or similar action.

    Any dialogs displayed will be parented off the given \a focus widget.

    \sa canActivate()
*/

/*!
    \fn QDrmContentPlugin::deleteFile( const QString &fileName )

    Deletes the DRM protected file with the file name \a fileName.

    Returns true if the file was sucessfully deleted.
*/

/*!
    \fn QDrmContentPlugin::unencryptedSize( const QString &fileName )

    Returns the size in bytes of the plaintext content of a DRM protected file with the file name \a fileName.
*/

/*!
    \fn QDrmContentPlugin::installContent( const QString &fileName, QContent *content )

    Populates a newly created QContent \a content with meta-data from the file with the file name \a fileName.

    This at a minimum sets the name and type of the QContent, but may also set other attributes such
    as an icon or any number of property values.  In some cases the file may be converted to an localized
    format and renamed, one such instance is OMA DRM message (application/vnd.oma.drm.message) files which
    are converted to the content format (application/vnd.oma.drm.content) when installed.

    Returns true if the file is a valid DRM file.

    \sa QContent
*/

/*!
    Refreshes the content data of \a content following a change to the file it references.

    Returns true if the content data was out of date, false otherwise.
 */
bool QDrmContentPlugin::updateContent( QContent *content )
{
    Q_UNUSED( content );

    return false;
}

/*!
    \fn QDrmContentPlugin::reactivate( const QContent &content, QDrmRights::Permission permission, QWidget *focus )

    Checks if the rights to render \a content with the given \a permission have expired or will expire
    in the near future.  If either condition is true and there is a valid activation mechanism for the
    content the user is informed and prompted to reactivate the content.

    Any dialogs displayed will be parented off the given \a focus widget.
*/


/*!
    \fn QDrmContentPlugin::activate( const QContent &content, QDrmRights::Permission permission, QWidget *focus )

    Checks if there are valid rights to render the DRM protected content \a content with the given \a permission.
    If there are currently no valid rights an attempt to acquire new rights is initiated, which may be done by
    downloading the rights directly or by prompting the user to visit a web site.

    If the content had valid rights when this method was called or new rights could be acquired directly this will
    return true. Otherwise if the rights are invalid and new rights can not be obtained or the rights acquisition
    process requires external action such as visiting a web page this will return false.

    Any dialogs displayed will be parented off the given \a focus widget.

    This method will block until the rights activation process has been completed which may take significant time.
*/

/*!
    Creates a preview thumbnail image of the content with the file name \a fileName scaled to the given \a size.
    Aspect ratio is maintained according to \a mode.
*/
QPixmap QDrmContentPlugin::thumbnail( const QString &fileName, const QSize &size, Qt::AspectRatioMode mode )
{
    QPixmap thumb;

    QDrmRights::Permissions p = permissions( fileName );

    QIODevice *io = 0;

    if( p & QDrmRights::Display )
        io = createDecoder( fileName, QDrmRights::Display );
    if( !io && p & QDrmRights::Preview )
        io = createDecoder( fileName, QDrmRights::Preview );

    if( io )
    {
        if( io->open( QIODevice::ReadOnly ) )
        {
            QThumbnail thumbnail( io );

            thumb = thumbnail.pixmap( size, mode );

            io->close();
        }

        delete io;
    }

    return thumb;
}

/*!
    \reimp
*/
QAbstractFileEngine *QDrmContentPlugin::create( const QString &file ) const
{
    foreach( QString key, keys() )
        if( file.endsWith( '.' + key ) && isProtected( file ) )
            return new DefaultDrmFileEngine( file, const_cast< QDrmContentPlugin * >( this ) );

    return 0;
}

/*!
    Checks if a license for the content with the file name \a fileName is registered with the
    plugin.  A pointer to a registered license is returned if found otherwise a null pointer is
    returned.

    \sa registerLicense(), requestContentLicense()
*/
QDrmContentLicense *QDrmContentPlugin::license( const QString &fileName ) const
{
    return d->licenses.hasLocalData() ? d->licenses.localData()->license( fileName ) : 0;
}

/*!
    Registers a new content \a license with the plugin.

    \sa license(), requestContentLicense()
*/
void QDrmContentPlugin::registerLicense( QDrmContentLicense *license )
{
    if( !d->licenses.hasLocalData() )
        d->licenses.setLocalData( new QDrmContentPluginLicenseStore );

    d->licenses.localData()->insertLicense( license );
}

/*!
    Unregisters a content \a license when it is destroyed.
*/
void QDrmContentPlugin::unregisterLicense( QObject *license )
{
    Q_UNUSED(license);
}

/*!
    Ensures all DRM plugins have been instantiated.  This should be called before scanning directories for content
    to ensure the file engines for drm content are loaded and content contained within multipart files can be found.
*/
void QDrmContentPlugin::initialize()
{
    DrmContentPluginManager::instance()->load();
}

/*!
    \class QDrmAgentPlugin
    \inpublicgroup QtBaseModule

    \brief The QDrmAgentPlugin class initializes the various services provided by a DRM agent.

    In addition to integration with the Qt Extended document system DRM agents may need to provide application services and
    license management facilities.  QDrmAgentPlugin allows agents to implement a single plugin that instantiates a Qt Extended service for rights acquisition and user notification services, a series of widgets for a DRM management application
    and a regular QDrmContentPlugin for use in DRM enabled applications.

    \ingroup drm
*/

/*!
    \internal
*/
QDrmAgentPlugin::~QDrmAgentPlugin()
{
}

/*!
    \fn QDrmAgentPlugin::createDrmContentPlugin()

    Returns an instance of a QDrmContentPlugin.
*/

/*!
    \fn QDrmAgentPlugin::createService( QObject *parent )

    Returns a new instance of a qtopia service with the given \a parent to provide rights acquisition and
    user notification services.
*/

/*!
    \fn QDrmAgentPlugin::createManagerWidgets( QWidget *parent )

    Constructs a series of widgets with the given \a parent to be utilised within a DRM management application.

    Typical widgets used are lists of installed rights and settings pages.

    Each widget is displayed in a seperate tab page.
*/

#include "qdrmcontentplugin.moc"
