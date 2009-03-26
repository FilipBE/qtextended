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

#include "bscidrmcontentplugin.h"
#include "bscifileengine.h"
#include "bscidrm.h"
#include "bsciprompts.h"
#include <qtopiaservices.h>
#include <QtDebug>
#include <QTimerEvent>
#include <qtopialog.h>
#include <QContentSet>
#include <QThread>
#include <QEventLoop>
#include <QWaitWidget>
#include <QMessageBox>

/*!
    \class BSciContentLicense
    \inpublicgroup QtDrmModule

    \internal
*/


/*!
    Constructs a new content license to permit renderering \a content with the given \a permission with notifications
    displayed in accordance with the given \a options.

    Constraints are updated using the given file \a operations on the open file handle \a f.
*/
BSciContentLicense::BSciContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options, TFileHandle f, SBSciContentAccess *operations )
    : m_content( content )
    , m_permission( permission )
    , m_renderState( options & QDrmContent::Handover ? QDrmContent::Paused : QDrmContent::Stopped )
    , m_options( options )
    , file( f )
    , fileOps( operations )
    , timerId( 0 )
{
}

/*!
    \internal
 */
BSciContentLicense::~BSciContentLicense()
{
    if( fileOps )
        (fileOps->bsci_close)( file );
}

/*!
    \reimp
*/
QContent BSciContentLicense::content() const
{
    return m_content;
}

/*!
    \reimp
*/
QDrmRights::Permission BSciContentLicense::permission() const
{
    return m_permission;
}

/*!
    \reimp
*/
QDrmContent::RenderState BSciContentLicense::renderState() const
{
    return m_renderState;
}

/*!
    \reimp
*/
void BSciContentLicense::renderStateChanged( const QContent &content, QDrmContent::RenderState state )
{
    if( content.id() != m_content.id() )
        return;

    switch( state )
    {
    case QDrmContent::Started:
        startConstraintUpdates();
        break;
    case QDrmContent::Paused:
        pauseConstraintUpdates();
        break;
    case QDrmContent::Stopped:
        stopConstraintUpdates();
    }
}

/*!
    Responds to a timer \a event to update the rights constraints at a regular interval.
    If the rights have expired a rightsExpired signal is emitted and constraints updates are stopped.
 */
void BSciContentLicense::timerEvent( QTimerEvent * event )
{
    QObject::timerEvent( event );

    if( fileOps && event->timerId() == timerId )
    {
        QDateTime currentTime = QDateTime::currentDateTime();

        if( (fileOps->bsci_meterUsage)( file, lastUpdate.secsTo( currentTime ) ) != RS_Valid )
        {
            (fileOps->bsci_close)( file );

            fileOps = 0;
            file = 0;

            emit rightsExpired( m_content, m_permission );

            m_renderState = QDrmContent::Stopped;
        }

        lastUpdate = currentTime;
    }
}

/*!
    Starts the rights metering timer.

    If the render state is stopped this will count as a use of the content.
 */
void BSciContentLicense::startConstraintUpdates()
{
    if( fileOps && m_renderState != QDrmContent::Started )
    {
        qLog(DRMAgent) << "Render started" << m_content.name();
        ERightsStatus status = RS_LAST;

        if( (fileOps->bsci_getLicenseInfo)( file, &status, 0, 0 ) == BSCI_NO_ERROR && status == RS_Valid )
        {
            if( m_renderState == QDrmContent::Stopped )
                (fileOps->bsci_executeIntent)( file );

            m_renderState = QDrmContent::Started;

            lastUpdate = QDateTime::currentDateTime();

            timerId = startTimer( 5000 );
        }
        else
            expireLicense();
    }
}

/*!
    Pauses the rights metering timer.  Resuming rendering will not count as a use.
 */
void BSciContentLicense::pauseConstraintUpdates()
{
    if( fileOps && m_renderState == QDrmContent::Started )
    {
        qLog(DRMAgent) << "Render paused" << m_content.name();

        killTimer( timerId );

        timerId = 0;

        if( (fileOps->bsci_meterUsage)( file, lastUpdate.secsTo( QDateTime::currentDateTime() ) ) != RS_Valid )
            expireLicense();
        else
            m_renderState = QDrmContent::Paused;
    }
}


/*!
    Stops the rights metering timer.  Restarting rendering will count as a use.
 */
void BSciContentLicense::stopConstraintUpdates()
{
    if( m_renderState == QDrmContent::Started )
    {
        pauseConstraintUpdates();
    }

    if( fileOps && m_renderState == QDrmContent::Paused )
    {
        qLog(DRMAgent) << "Render stopped" << m_content.name();

        m_renderState = QDrmContent::Stopped;

        ERightsStatus status = RS_LAST;

        if( (fileOps->bsci_getLicenseInfo)( file, &status, 0, 0 ) != BSCI_NO_ERROR || status != RS_Valid )
            expireLicense();
    }
}

/*!
    Closes the license and emits the rightsExpired() signal.
*/
void BSciContentLicense::expireLicense()
{
    qLog(DRMAgent) << "Rights Expired";

    m_renderState = QDrmContent::Stopped;

    (fileOps->bsci_close)( file );

    fileOps = 0;
    file = 0;

    emit rightsExpired( m_content, m_permission );
}


/*!
    \class BSciPreviewLicense
    \inpublicgroup QtDrmModule

    License for rendering an embedded preview of protected content.  Because embedded preview
    don't have any constraints associated with their use this license will not perform any
    constraints updated.

    \internal
*/

/*!
    Constructs a new preview license for the given \a content.
*/
BSciPreviewLicense::BSciPreviewLicense( const QContent &content )
    : m_content( content )
    , m_renderState( QDrmContent::Stopped )
{
}

/*!
    \internal
*/
BSciPreviewLicense::~BSciPreviewLicense()
{
}

/*!
    \reimp
*/
QContent BSciPreviewLicense::content() const
{
    return m_content;
}

/*!
    \reimp
*/
QDrmRights::Permission BSciPreviewLicense::permission() const
{
    return QDrmRights::Preview;
}

/*!
    \reimp
*/
QDrmContent::RenderState BSciPreviewLicense::renderState() const
{
    return m_renderState;
}

/*!
    \reimp
*/
void BSciPreviewLicense::renderStateChanged( const QContent &content, QDrmContent::RenderState state )
{
    if( content.id() == m_content.id() )
        m_renderState = state;
}

/*!
    \class BSciReadDevice
    \inpublicgroup QtDrmModule

    \internal
*/

/*!
    Constructs an io device which provides access to the plaintext content of the drm protected file located at
    the given \a path to be rendered with the the given \a permission.
 */
BSciReadDevice::BSciReadDevice( const QString &path, QDrmRights::Permission permission )
    : QIODevice()
    , fileName( permission == QDrmRights::Preview ? BSciDrm::getPreviewUri( path ).toLocal8Bit() : BSciDrm::formatPath( path ) )
    , permission( permission )
    , file( 0 )
    , fileOps( 0 )
{
    fileOps = BSCIGetDrmContentAccess( BSciDrm::context );
}

/*!
    \internal
 */
BSciReadDevice::~BSciReadDevice()
{
    if( isOpen() )
        close();
}

/*!
    \reimp
*/
void BSciReadDevice::close ()
{
    QIODevice::close();

    int error = (*fileOps->bsci_close)( file );

    if( error != BSCI_NO_ERROR )
        BSciDrm::printError( error, "BSciReadDevice::close()" );

    file = 0;
}


/*!
    Opens the device in the specified \a mode.  Returns true if
    the open was successful; false otherwise.
 */
bool BSciReadDevice::open( QIODevice::OpenMode mode )
{
    if( mode & WriteOnly || !( mode & ReadOnly ) )
        return false;

    EPermission renderMode = BSciDrm::transformPermission( permission );

    int error = BSCI_NO_ERROR;

    file = (*fileOps->bsci_fopen)( BSciDrm::context, fileName, "rb", renderMode, &error );

    if( error == BSCI_NO_ERROR && file && QIODevice::open( mode ) )
        return true;

    (*fileOps->bsci_close)( file );

    file = 0;

    return false;
}


/*!
    \reimp
*/
qint64 BSciReadDevice::readData( char *data, qint64 maxlen )
{
    return (*fileOps->fread)( static_cast< void * >( data ), 1, maxlen, file );
}

/*!
    \reimp
*/
bool BSciReadDevice::seek( qint64 offset )
{
    QIODevice::seek( offset );
    return (*fileOps->fseek)( file, offset, SEEK_SET ) == 0;
}

/*!
    \reimp
*/
qint64 BSciReadDevice::size () const
{
    return (*fileOps->bsci_getContentSize)( file );
}

/*!
    \reimp
 */
qint64 BSciReadDevice::writeData( const char *data, qint64 len )
{
    Q_UNUSED( data );
    Q_UNUSED( len );

    return -1;
}

class BSciActivationThread : public QThread
{
    Q_OBJECT
public:
    BSciActivationThread( QObject *parent = 0 );
    virtual ~BSciActivationThread();

    int activate( const QByteArray &fileName, EPermission permission );

    void run();

public slots:
    void cancel();

private:
    int m_operationHandle;
    QByteArray m_fileName;
    EPermission m_permission;
    int m_result;
};

BSciActivationThread::BSciActivationThread( QObject *parent )
    : QThread( parent )
    , m_operationHandle( BSCICreateOperationHandle( BSciDrm::context ) )
    , m_result( BSCI_NO_ERROR )
{
}

BSciActivationThread::~BSciActivationThread()
{
}

int BSciActivationThread::activate( const QByteArray &fileName, EPermission permission )
{
    m_fileName = fileName;
    m_permission = permission;

    QEventLoop eventLoop;

    connect( this, SIGNAL(finished()), &eventLoop, SLOT(quit()), Qt::QueuedConnection );

    start();

    eventLoop.exec();

    return m_result;
}

void BSciActivationThread::run()
{
    m_result = BSCIActivateContent( BSciDrm::context, m_operationHandle, m_fileName, m_permission );
}

void BSciActivationThread::cancel()
{
    BSCICancel( BSciDrm::context, m_operationHandle );
}


/*!
    \class BSciDrmContentPlugin
    \inpublicgroup QtDrmModule

    \internal
*/

/*!
    \enum BSciDrmContentPlugin::MetaData
    Represents the meta-data values read from DCF files.

    \value ContentType The mime type of the content embedded in the DCF.
    \value ContentUrl A URL where the content can be acquired.
    \value ContentVersion The version of the content.
    \value Title The title of the content.
    \value Description A description of the content.
    \value Copyright A copyright notice for the content.
    \value Author The author of the content.
    \value IconUri A URI where an icon for the content can be acquired.
    \value InfoUrl A URL where information about the content can be acquired.
    \value RightsIssuerUrl A URL from which rights to the content can be acquired.
*/

/*!
    Constructs a new Beep Science drm content plugin with the given \a parent.
*/
BSciDrmContentPlugin::BSciDrmContentPlugin( QObject *parent )
    : QDrmContentPlugin( parent )
    , m_transactionTracking( "/OmaDrmAgent" )
{
    connect( &m_transactionTracking, SIGNAL(contentsChanged()), this, SLOT(transactionTrackingChanged()) );
}

/*!
    \internal
*/
BSciDrmContentPlugin::~BSciDrmContentPlugin()
{
}

/*!
    \reimp
*/
QStringList BSciDrmContentPlugin::keys() const
{
    return QStringList()
            << QLatin1String( "dcf" )
            << QLatin1String( "odf" )
            << QLatin1String( "dm" );
}

/*!
    \reimp
*/
QStringList BSciDrmContentPlugin::types() const
{
    return QStringList()
            << QLatin1String( "application/vnd.oma.drm.content" )
            << QLatin1String( "application/vnd.oma.drm.message" )
            << QLatin1String( "application/vnd.oma.drm.dcf" );
}

/*!
    \reimp
*/
bool BSciDrmContentPlugin::isProtected( const QString &filePath ) const
{
    QByteArray formattedPath = BSciDrm::formatPath( filePath );

    formattedPath = formattedPath.mid( 0, formattedPath.indexOf( ";cid:" ) );

    return BSCIGetFileInfo( BSciDrm::context, formattedPath, 0, 0 ) > 0;
}

/*!
    \reimp
*/
QList< QPair< QString, QString > > BSciDrmContentPlugin::httpHeaders() const
{
    int version = BSCIGetDrmVersion( BSciDrm::context );

    return QList< QPair< QString, QString > >() << QPair< QString, QString >(
            QLatin1String( "DRM-Version" ),
            QString( "%1.%2" )
                    .arg( (version & 0x0000FF00) >> 8 )
                    .arg(  version & 0x000000FF ) );
}

/*!
    \reimp
*/
QDrmRights::Permissions BSciDrmContentPlugin::permissions( const QString &filePath )
{
    const QByteArray localPath = BSciDrm::formatPath( filePath );

    QDrmRights::Permissions permissions = QDrmRights::NoPermissions;

    if( !filePath.contains( QLatin1String( ".odf/" ) ) && BSCIAllowForward( BSciDrm::context, localPath, 0 ) > 0 )
        permissions |= QDrmRights::Distribute;

    if( filePath.endsWith( QLatin1String( ".odf" ) ) &&
        BSCIGetFileInfo( BSciDrm::context, localPath, 0, 0 ) > 1 )
    {
        permissions |= QDrmRights::BrowseContents;

        return permissions;
    }

    bool stateful = false;

    const char *fileUris[] = {
        localPath.constData(), localPath.constData(), localPath.constData(),
        localPath.constData(), localPath.constData() };

    EPermission bsciPermissions[] = { PT_Play, PT_Display, PT_Execute, PT_Print, PT_Export };

    SBSciConstraints constraints[ 5 ];
    EDrmMethod methods[ 5 ];
    ERightsStatus status[ 5 ];

    memset( &constraints, 0, sizeof(constraints) );
    memset( &methods, 0, sizeof(methods) );
    memset( &status, 0, sizeof(status) );

    int error = BSCIGetLicenseInfoArray( BSciDrm::context, 0, 5, fileUris, bsciPermissions, constraints, methods, status );

    if( error != BSCI_NO_ERROR )
    {
        qWarning() << "Error" << error;
        return QDrmRights::Permissions();
    }

    if( status[ 0 ] == RS_Valid ){ permissions |= QDrmRights::Play   ; stateful |= BSciDrm::isStateful( constraints[ 0 ] ); }
    if( status[ 1 ] == RS_Valid ){ permissions |= QDrmRights::Display; stateful |= BSciDrm::isStateful( constraints[ 1 ] ); }
    if( status[ 2 ] == RS_Valid ){ permissions |= QDrmRights::Execute; stateful |= BSciDrm::isStateful( constraints[ 2 ] ); }
    if( status[ 3 ] == RS_Valid ){ permissions |= QDrmRights::Print  ; stateful |= BSciDrm::isStateful( constraints[ 3 ] ); }
    if( status[ 4 ] == RS_Valid ){ permissions |= QDrmRights::Export ; stateful |= BSciDrm::isStateful( constraints[ 4 ] ); }

    if( !stateful )
        permissions |= QDrmRights::Automated;

    return permissions;
}

/*!
    \reimp
 */
bool BSciDrmContentPlugin::activate( const QContent &content, QDrmRights::Permission permission, QWidget *focus )
{
    Q_UNUSED( focus );

    qLog(DRMAgent) << __PRETTY_FUNCTION__ << content.name() << permission;

    if( (content.permissions( true ) & permission) == permission )
        return true;

    switch( permission )
    {
    case QDrmRights::Play:
    case QDrmRights::Display:
    case QDrmRights::Execute:
    case QDrmRights::Print:
    case QDrmRights::Export:
        break;
    case QDrmRights::Unrestricted:
        QMessageBox::warning(
                focus,
                tr( "Document protected" ),
                tr( "%1 cannot be opened in an application that doesn't support protected content.", "%1 = document name" ).arg( content.name() ) );
        return false;
    default:
        qWarning() <<  "Trying to activate content with an invalid permission" << permission;
        return false;
    }

    QWaitWidget waitWidget( focus );

    waitWidget.setCancelEnabled ( true );

    BSciActivationThread thread;

    connect( &waitWidget, SIGNAL(cancelled()), &thread, SLOT(cancel()) );
    connect( &thread, SIGNAL(finished()), &waitWidget, SLOT(hide()) );

    waitWidget.show();

    int error = thread.activate( BSciDrm::formatPath( content.fileName() ), BSciDrm::transformPermission( permission ) );

    switch( error )
    {
    case BSCI_NO_ERROR:
        return true;
    case BSCI_UNABLE_TO_ACTIVATE:
        BSciPrompts::instance()->notifyCannotActivate( content, permission, BSciPrompts::Open );
    case BSCI_CANCEL:
    case BSCI_IN_PROGRESS:
        return false;
    default:
        BSciPrompts::instance()->notifyError( QString( "%1 could not be activated due to an error.\nError number: %2\n%3" )
                .arg( content.name() )
                .arg( QString::number( -error, 16 ) )
                .arg( BSciDrm::getError( error ) ) );
        return false;
    }
}

/*!
    \reimp
 */
void BSciDrmContentPlugin::activate( const QContent &content, QWidget *focus )
{
    Q_UNUSED( content );
    Q_UNUSED( focus );
}

/*!
    \reimp
*/
void BSciDrmContentPlugin::reactivate( const QContent &content, QDrmRights::Permission permission, QWidget *focus )
{
    Q_UNUSED( focus );

    qLog(DRMAgent) << __PRETTY_FUNCTION__ << content.name() << permission;

    const QByteArray localPath = BSciDrm::formatPath( content.fileName() );

    EPermission mode = BSciDrm::transformPermission( permission );

    if( mode != PT_LAST )
    {
        ERightsStatus status = RS_LAST;

        if( BSCIGetLicenseInfo( BSciDrm::context, localPath, mode ,&status, 0, 0 ) == BSCI_NO_ERROR && status == RS_NoRights )
        {
            QString url = BSciDrm::getMetaData( content.fileName(), MD_RightsIssuerURL );

            if( !url.isEmpty() )
                BSciPrompts::instance()->requestOpenUrl( content, url, permission, BSciPrompts::Expired );
            else
                BSciPrompts::instance()->notifyCannotActivate( content, permission, BSciPrompts::Expired );
        }
    }
}

/*!
    \reimp
*/
QDrmRights BSciDrmContentPlugin::getRights( const QString &filePath, QDrmRights::Permission permission )
{
    const QByteArray localPath = BSciDrm::formatPath( filePath );

    if( permission == QDrmRights::Unrestricted )
        return QDrmRights( permission, isProtected( filePath ) ? QDrmRights::Invalid : QDrmRights::Valid );
    else if( permission == QDrmRights::Distribute )
        return QDrmRights( permission,
                           BSCIAllowForward( BSciDrm::context, localPath, 0 ) > 0
                                   ? QDrmRights::Valid
                                   : QDrmRights::Invalid );

    SBSciConstraints constraints;

    memset( &constraints, 0, sizeof(constraints) );

    EPermission mode = BSciDrm::transformPermission( permission );

    ERightsStatus status;

    if( BSCIGetLicenseInfo( BSciDrm::context, localPath, mode, &status, &constraints, 0 ) == BSCI_NO_ERROR )
        return BSciDrm::constraints( permission, status, &constraints );
    else
        return QDrmRights();
}

/*!
    \reimp
*/
QDrmContentLicense *BSciDrmContentPlugin::requestContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    switch( permission )
    {
    case QDrmRights::Play:
    case QDrmRights::Display:
    case QDrmRights::Execute:
    case QDrmRights::Print:
    case QDrmRights::Export:
        {
            if( content.permissions() & permission )
                break;

            permission = QDrmRights::Preview;
        }
    case QDrmRights::Preview:
        {
            if( content.permissions() & permission )
            {
                QDrmContentLicense *license = new BSciPreviewLicense( content );

                registerLicense( license );

                return license;
            }
        }
    default:
            return 0;
    }

    SBSciContentAccess *ops = BSCIGetDrmContentAccess( BSciDrm::context );

    EPermission renderMode = BSciDrm::transformPermission( permission );

    TFileHandle file = (*ops->bsci_fopen)( BSciDrm::context, BSciDrm::formatPath( content.fileName() ), "rb", renderMode, 0 );

    QDrmContentLicense *license = new BSciContentLicense( content, permission, options, file, ops );

    registerLicense( license );

    return license;
}

/*!
    \reimp
*/
QIODevice *BSciDrmContentPlugin::createDecoder( const QString &filePath, QDrmRights::Permission permission )
{
    return new BSciReadDevice( filePath, permission );
}

/*!
    \reimp
*/
bool BSciDrmContentPlugin::canActivate( const QString &filePath )
{
    // Don't support global activation option.
    Q_UNUSED(filePath);

    return false;
}

/*!
    \reimp
*/
qint64 BSciDrmContentPlugin::unencryptedSize( const QString &filePath )
{
    qint64 size = -1;

    // In order to get the size of the content the file needs to be opened.
    // Find if there are any rights to the content and if so open it using
    // those rights and get the size.
    SBSciContentAccess *ops = BSCIGetDrmContentAccess( BSciDrm::context );

    if( ops != 0 )
    {
        EPermission  mode = PT_LAST;

        if     ( BSciDrm::hasRights( filePath, QDrmRights::Play    ) ) mode = PT_Play;
        else if( BSciDrm::hasRights( filePath, QDrmRights::Display ) ) mode = PT_Display;
        else if( BSciDrm::hasRights( filePath, QDrmRights::Execute ) ) mode = PT_Execute;
        else if( BSciDrm::hasRights( filePath, QDrmRights::Print   ) ) mode = PT_Print;
        else if( BSciDrm::hasRights( filePath, QDrmRights::Export  ) ) mode = PT_Export;

        if( mode != PT_LAST )
        {
            TFileHandle file = (*ops->bsci_fopen)( BSciDrm::context, BSciDrm::formatPath( filePath ), "rb", mode, 0 );

            size = (*ops->bsci_getContentSize)( file );

            (*ops->bsci_close)( file );
        }
    }

    if( size == -1 )
        size = QFileInfo( filePath ).size();

    return size;
}

/*!
    \reimp
*/
QAbstractFileEngine *BSciDrmContentPlugin::create( const QString &fileName ) const
{
    int pos = -1;
    static QString odfExtn(QLatin1String(".odf/"));

    while( ( pos = fileName.indexOf( odfExtn, pos + 1 ) ) != -1 )
    {
        QString rootFile = fileName.left( pos + 4 );
        QString baseName = fileName.mid( pos + 5 );

        if( isProtected( rootFile ) )
            return new BSciDrmFileEngine( const_cast< BSciDrmContentPlugin * >( this ), fileName, rootFile, baseName );
    }

    if( fileName.endsWith( QLatin1String(".odf") ) || fileName.endsWith( QLatin1String(".dcf") ) )
        if( isProtected( fileName ) )
            return new BSciDrmFileEngine( const_cast< BSciDrmContentPlugin * >( this ), fileName );

    return 0;
}


/*!
    \reimp
 */
bool BSciDrmContentPlugin::deleteFile( const QString &filePath )
{
    return QFile::remove( filePath );
}

/*!
    \reimp
 */
bool BSciDrmContentPlugin::installContent( const QString &filePath, QContent *content )
{
    qLog(DRMAgent) << "QOmaDrmContentPlugin::installContent()" << filePath;

    QString dcfFilePath;

    if( filePath.endsWith( QLatin1String( ".dm" ) ) )
    {
        dcfFilePath = filePath;

        dcfFilePath.chop( 2 );
        dcfFilePath += QLatin1String( "dcf" );

        {
            QFile file( filePath );

            if( file.size() <= 0 || !file.rename( dcfFilePath ) )
            {    // Empty file with .dm extension: have to assume drm message.
                content->setType( QLatin1String( "application/vnd.oma.drm.message" ) );

                return true;
            }
        }

        if( !installMessageFile( dcfFilePath ) )
        {
            QFile::rename( dcfFilePath, filePath );

            qLog(DRMAgent) << "Message install failed" << filePath;

            return false;
        }
    }
    else
    {
        dcfFilePath = filePath;

        registerFile( dcfFilePath );
    }

    if( !isProtected( dcfFilePath ) )
        return false;

    content->setFile( dcfFilePath );

    content->setRole( QContent::Document );

    QMap< MetaData, QString > metaData = getMetaData( dcfFilePath );

    if( metaData.contains( Title ) )
        content->setName( metaData[ Title ] );
    else if( content->name().isEmpty() )
    {
        int cid = filePath.indexOf( QLatin1String( "/cid:" ) );

        if( cid != -1 )
            content->setName( filePath.mid( cid + 5 ) );
        else
            content->setName( QFileInfo( filePath ).completeBaseName() );
    }

    if( !QFileInfo( dcfFilePath ).isDir() )
    {
        if( metaData.contains( ContentType ) )
            content->setType( metaData[ ContentType ] );
        else
            qLog(DRMAgent) << "Protected file has no mime type" << dcfFilePath;
    }
    else
        QContentSet::scan( dcfFilePath );

    if( metaData.contains( IconUri ) )
        content->setIcon( metaData[ IconUri ] );

    if( metaData.contains( ContentUrl ) )
        content->setProperty( QContent::ContentUrl, metaData[ ContentUrl ] );

    if( metaData.contains( ContentVersion ) )
        content->setProperty( QContent::Version, metaData[ ContentVersion ] );

    if( metaData.contains( Description ) )
        content->setProperty( QContent::Description, metaData[ Description ] );

    if( metaData.contains( Author ) )
        content->setProperty( QContent::Author, metaData[ Author ] );

    if( metaData.contains( InfoUrl ) )
        content->setProperty( QContent::InformationUrl, metaData[ InfoUrl ] );

    if( metaData.contains( RightsIssuerUrl ) )
        content->setProperty( QContent::RightsIssuerUrl, metaData[ RightsIssuerUrl ] );

    return true;
}

/*!
    \reimp
*/
bool BSciDrmContentPlugin::updateContent( QContent *content )
{
    qLog(DRMAgent) << "QOmaDrmContentPlugin::updateContent()" << content->fileName();

    return installContent( content->fileName(), content );
}

/*!
    Converts the DRM message file with the file name \a fileName to the DCF format and stores the rights in the rights database.

    Returns true upon success; otherwise returns false.
*/
bool BSciDrmContentPlugin::installMessageFile( const QString &fileName )
{
    QFile file( fileName );

    if( !file.open( QIODevice::ReadOnly ) )
        return false;

    QByteArray data = file.readAll();

    file.close();

    QByteArray boundary = data.left( data.indexOf( '\r' ) );

    int error = BSCIHandleDrmMessage( BSciDrm::context, 0, data.constData(), data.size(), boundary.constData(), fileName.toLocal8Bit().constData() );

    switch( error )
    {
    case BSCI_NO_ERROR:
        return true;
    case BSCI_INVALID_RO:
    case BSCI_INVALID_PERMISSION:
        QFile::remove( fileName );
    default:
        qWarning() << "Install failure" << error;
        return false;
    }
}

/*!
    Registers the DCF file with the file name \a fileName with the rights database.

    Returns true upon success; otherwise returns false.
*/
bool BSciDrmContentPlugin::registerFile( const QString &fileName )
{
    int error = BSCIRegisterDCF( BSciDrm::context, 0, BSciDrm::formatPath( fileName ), 1 );

    if( error == BSCI_NO_ERROR )
        return true;

    return false;
}

#define INSERT_META_DATA( qtopiaField, bsciField ) \
    { \
        QString temp = BSciDrm::getMetaData( fileName, bsciField ); \
        \
        if( !temp.isEmpty() ) \
            data.insert( qtopiaField, temp ); \
    }

/*!
    Returns all available meta-data from the headers of the DCF file with the file name \a fileName.
*/
QMap< BSciDrmContentPlugin::MetaData, QString > BSciDrmContentPlugin::getMetaData( const QString &fileName )
{
    QMap< MetaData, QString > data;

    EDrmMethod method = DM_NotProtected;

    BSCIAllowForward( BSciDrm::context, BSciDrm::formatPath( fileName ), &method );

    if( method == DM_NotProtected )
        return data;
    else if( method != DM_FL && method != DM_CD )
        INSERT_META_DATA( Title, MD_Title );

    INSERT_META_DATA( ContentType    , MD_ContentType     );
    INSERT_META_DATA( ContentUrl     , MD_ContentURL      );
    INSERT_META_DATA( ContentVersion , MD_ContentVersion  );
    INSERT_META_DATA( Description    , MD_Description     );
    INSERT_META_DATA( Copyright      , MD_Copyright       );
    INSERT_META_DATA( Author         , MD_Author          );
    INSERT_META_DATA( IconUri        , MD_IconURI         );
    INSERT_META_DATA( InfoUrl        , MD_InfoURL         );
    INSERT_META_DATA( RightsIssuerUrl, MD_RightsIssuerURL );

    return data;
}

#undef INSERT_META_DATA

void BSciDrmContentPlugin::transactionTrackingChanged()
{
    BSCIEnableTransactionTracking( BSciDrm::context, m_transactionTracking.value( "TransactionTracking", false ).toBool() ? 1 : 0 );
}

#include "bscidrmcontentplugin.moc"
