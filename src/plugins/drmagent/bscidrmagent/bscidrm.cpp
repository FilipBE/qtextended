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

#include "bscidrm.h"
#include <qtopianamespace.h>
#include <QUrl>
#include <QDateTime>
#include <QtDebug>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <QTimeString>

#include <BSciTransformer.h>
#include <BSciTransformer.c>
#include <stdlib.h>
#include <custom.h>

void *BSciDrm::context = 0;

void BSciDrm::initialiseAgent( const QString &id, SBSciCallbacks *callbacks )
{
    qLog(DRMAgent) << "BSciDrm::initialiseAgent()";

    uchar authToken[ id.size() + 1 ];
    uchar dateBuffer[ 9 ];

    QString dateString = QDateTime::currentDateTime().toUTC().toString( "ddMMyyyy" );

    qstrcpy( reinterpret_cast< char * >( dateBuffer ), dateString.toLocal8Bit() );

    BSCITransform( reinterpret_cast< const uchar * >( id.toLocal8Bit().constData() ), dateBuffer, authToken, id.size() );

#ifdef BSCI_DATABASE_PATH
    QString databasePath = QLatin1String( BSCI_DATABASE_PATH );
#else
    QString databasePath;

    const char *envDbPath = getenv( "BSCI_DATABASE_PATH" );

    if( envDbPath )
        databasePath =  envDbPath;
    else
        databasePath = Qtopia::homePath() + QLatin1String( "/Applications/Qtopia/DRM" );
#endif
    bool databaseExists = QFile::exists( databasePath + QLatin1String( "/bscidrm2.crt" ) );

    if( !databaseExists )
    {
        QString certificatePath = Qtopia::qtopiaDir() + QLatin1String( "etc/bscidrm/bscidrm2.crt" );

        QDir::root().mkdir( databasePath );
        QFile::copy( certificatePath, databasePath + QLatin1String( "/bscidrm2.crt" ) );
    }

    int error = 0;
    int flags = 0;

    {
        QSettings conf( "Trolltech", "OmaDrm" );

        if( conf.value( "transactiontracking", false ).toBool() )
            flags |= INIT_TRANTRACK;
    }

    context = BSCICreateContext(
            0,
            reinterpret_cast< const uchar * >( id.toLocal8Bit().constData() ),
            authToken,
            id.size(),
            reinterpret_cast< const uchar * >( databasePath.toLocal8Bit().constData() ),
            callbacks,
            &error );

    if( error != BSCI_NO_ERROR )
    {
        releaseAgent();

        printError( error, "BSciDrm::initaliseAgent()" );
    }

    if( !databaseExists && context )
    {
        QString keyPath = Qtopia::qtopiaDir() + QLatin1String( "etc/bscidrm/BSCI_Device_1033.pem.key" );
        QString certificatePath = Qtopia::qtopiaDir() + QLatin1String( "etc/bscidrm/BSCI_DeviceChain.pem.crt" );

        error = BSCIImportDeviceKeyAndCertList(
                context,
                keyPath.toLocal8Bit().constData(),
                certificatePath.toLocal8Bit().constData() );

        if( error != BSCI_NO_ERROR )
            printError( error, "BSciDrm::initaliseAgent() import key" );
    }


    qLog(DRMAgent) << "Agent initialised" << qApp->applicationName();
}

void BSciDrm::releaseAgent()
{
    qLog(DRMAgent) << "BSciDrm::releaseAgent()";

    if( context )
    {
        BSCIFreeContext( context );

        context = 0;

        qLog(DRMAgent) << "Agent released" << qApp->applicationName();
    }
}

/*!
    Converts the Qt Extended QDrmRights::Permission \a permissionType to the equivalent beep science
    EPermission value
*/
EPermission BSciDrm::transformPermission( QDrmRights::Permission permissionType )
{
    switch( permissionType )
    {
        case QDrmRights::Play:    return PT_Play;
        case QDrmRights::Display: return PT_Display;
        case QDrmRights::Execute: return PT_Execute;
        case QDrmRights::Print:   return PT_Print;
        case QDrmRights::Export:  return PT_Export;
        default:                  return PT_LAST;
    }
}

/*!
    Formats a string with interval data
*/
QString BSciDrm::formatInterval( const SBSciDuration &duration )
{
    QString date;
    QString time;

    if( duration.year || duration.month || duration.day )
        date = QObject::tr( "%1-%2-%3", "Date interval, %1 = number of years, %2 = number of months, %3 = number of days" )
                .arg( duration.year  )
                .arg( duration.month )
                .arg( duration.day   );

    if( duration.hour || duration.minute || duration.second )
        time = QTime( duration.hour, duration.minute, duration.second ).toString();

    if( !date.isEmpty() && !time.isEmpty() )
        return QObject::tr( "%1 %2", "%1 = date interval string, %2 = time interval string" )
                .arg( date )
                .arg( time );
    else if( !date.isEmpty() )
        return date;
    else if( !time.isEmpty() )
        return time;
    else
        return QTime().toString();
}

/*!
    Adds the constraints from \a constraints to \a rights.
*/
QDrmRights BSciDrm::constraints( QDrmRights::Permission permission, ERightsStatus status, SBSciConstraints *constraints )
{
    QDrmRights::Status stat = QDrmRights::Invalid;

    switch( status )
    {
        case RS_NoRights:
            stat = QDrmRights::Invalid;
            break;
        case RS_TrialOnly:
        case RS_Valid:
            stat = QDrmRights::Valid;
            break;
        case RS_RightsInFuture:
            stat = QDrmRights::ValidInFuture;
            break;
        case RS_NotProtected:
        default:
            return QDrmRights();
    }

    QDrmRights::ConstraintList con;

    if( constraints->count >= 0 )
    {
        con.append( QDrmRights::Constraint( QObject::tr( "Uses" ), constraints->count ) );
    }

    if( constraints->timedCount >= 0 )
    {
        QList< QPair< QString, QVariant > > attributes;

        attributes.append( QPair< QString, QVariant >( QObject::tr( "Time (seconds)" ), constraints->timer ) );

        con.append( QDrmRights::Constraint( QObject::tr( "Timed uses" ), constraints->timedCount, attributes ) );
    }

    if( IS_DURATION_SPECIFIED( constraints->intervalDuration ) )
    {
        QList< QPair< QString, QVariant > > attributes;

        if( constraints->intervalDuration.year )
            attributes.append( QPair< QString, QVariant >( QObject::tr( "Years" ), constraints->intervalDuration.year ) );
        if( constraints->intervalDuration.month )
            attributes.append( QPair< QString, QVariant >( QObject::tr( "Months" ), constraints->intervalDuration.month ) );
        if( constraints->intervalDuration.day )
            attributes.append( QPair< QString, QVariant >( QObject::tr( "Days" ), constraints->intervalDuration.day ) );
        if( constraints->intervalDuration.hour )
            attributes.append( QPair< QString, QVariant >( QObject::tr( "Hours" ), constraints->intervalDuration.hour ) );
        if( constraints->intervalDuration.minute )
            attributes.append( QPair< QString, QVariant >( QObject::tr( "Minutes" ), constraints->intervalDuration.minute ) );
        if( constraints->intervalDuration.second )
            attributes.append( QPair< QString, QVariant >( QObject::tr( "Seconds" ), constraints->intervalDuration.second ) );

        con.append( QDrmRights::Constraint( QObject::tr( "Interval" ), QVariant(), attributes ) );
    }

    // Start DateTime Rights
    if( IS_TIME_SPECIFIED( constraints->startDateTime ) )
    {
        QDateTime start(
                QDate( constraints->startDateTime.year, constraints->startDateTime.month, constraints->startDateTime.day ),
                QTime( constraints->startDateTime.hour, constraints->startDateTime.hour, constraints->startDateTime.second ) );

        con.append( QDrmRights::Constraint( QObject::tr( "Valid after" ), QTimeString::localYMDHMS( start ) ) );
    }

    // End DateTime Rights
    if( IS_TIME_SPECIFIED( constraints->endDateTime ) )
    {
        QDateTime end(
                QDate( constraints->endDateTime.year, constraints->endDateTime.month, constraints->endDateTime.day ),
                QTime( constraints->endDateTime.hour, constraints->endDateTime.hour, constraints->endDateTime.second ) );

        con.append( QDrmRights::Constraint( QObject::tr( "Valid before" ), QTimeString::localYMDHMS( end ) ) );
    }

    if( constraints->accumDuration != -1 )
    {
        con.append( QDrmRights::Constraint( QObject::tr( "Usage (seconds)" ), constraints->accumDuration ) );
    }

    if( constraints->isIndividual )
    {
        con.append( QDrmRights::Constraint( QObject::tr( "Individual" ), QVariant() ) );
    }

    for( unsigned int i = 0; i < constraints->systemCnt; i++ )
    {
        SBSciSystem *sys = &constraints->systems[ i ];

        QList< QPair< QString, QVariant > > attributes;

        for( unsigned int j = 0; j < sys->count; j++ )
            attributes.append( QPair< QString, QVariant >( QString( sys->systemName[ j ] ), sys->systemVersion[ j ] ) );

        con.append( QDrmRights::Constraint( QObject::tr( "System" ), QVariant(), attributes ) );
    }

    if( IS_TIME_SPECIFIED( constraints->noConsumeAfter ) )
    {
        QDateTime expiry(
                QDate( constraints->noConsumeAfter.year, constraints->noConsumeAfter.month, constraints->noConsumeAfter.day ),
                QTime( constraints->noConsumeAfter.hour, constraints->noConsumeAfter.hour, constraints->noConsumeAfter.second ) );

        con.append( QDrmRights::Constraint( QObject::tr( "Domain expires", "domain expires on <date>" ),
                    QTimeString::localYMDHMS( expiry ) ) );
    }

    return QDrmRights( permission, stat, con );
}

bool BSciDrm::isStateful( const SBSciConstraints &constraints )
{
    return constraints.count         != -1 ||
           constraints.timedCount    != -1 ||
           constraints.accumDuration != -1 ||
           IS_TIME_SPECIFIED( constraints.endDateTime    ) ||
           IS_TIME_SPECIFIED( constraints.noConsumeAfter ) ||
           IS_DURATION_SPECIFIED( constraints.intervalDuration );
}

/*!
    Maps the drm agent error codes to their string equivalents for debug
    output.
*/
const char *BSciDrm::getError( int error )
{
    switch( error )
    {
    case BSCI_NO_ERROR                   : return "Success";

    // Common errors 0x -- Those common values defined must not change!
    case BSCI_NOT_FOUND                  : return "Unable to find the specified object";
    case BSCI_CANCEL                     : return "The operation was cancelled";
    case BSCI_NO_MEMORY                  : return "There is not enough memory to complete operation.";
    case BSCI_NOT_SUPPORTED              : return "The operation requested is not supported";
    case BSCI_INVALID_PARAMETER          : return "Invalid parameter";
    case BSCI_NOT_AUTHORISED             : return "Application not authenticated";
    case BSCI_OVERFLOW                   : return "Overflow";
    case BSCI_UNDERFLOW                  : return "Negative array index specified";
    case BSCI_RESULT_TRUNCATED           : return "The returned buffer is truncated";
    case BSCI_INSUFFICIENT_DATA          : return "The input buffer is too small";
    case BSCI_NOT_ENOUGH_DATA            : return "There is not enough data to response a request.";
    case BSCI_INVALID_OPERATION_HANDLE   : return "The operation is running or no such operation handle";
    case BSCI_CHAR_ENCODING_ERROR        : return "Wrongly encoded character or string.";

    // Cryptographic errors 1x
    case BSCI_INVALID_RSA_KEY            : return "RESERVED for Game Extension";
    case BSCI_INVALID_CEK                : return "Invalid CEK in RO";
    case BSCI_SIZE_MISMATCH              : return "Size mismatch";
    case BSCI_INVALID_REK                : return "Invalid REK|MAC in RO or Domain";
    case BSCI_ASN1_ERROR                 : return "ASN1 parser error";
    case BSCI_KEY_NOT_FOUND              : return "Key Not Found in the store";
    case BSCI_CERT_ROOT_NOT_FOUND        : return "Root certificate not found";
    case BSCI_CERT_PARENT_NOT_FOUND      : return "Parent certificate not found";
    case BSCI_CERT_INVALID_DIGEST        : return "Certificate digests doesn't match";
    case BSCI_INVALID_CERT_CHAIN         : return "Certificate chain is invalid";
    case BSCI_CORRUPT                    : return "Corrupt cryptographic data; e.g. invalid key.";
    case BSCI_CERT_EXPIRED               : return "Certificate expired";
    case BSCI_OCSP_CERT_EXPIRED          : return "OCSP Certificate expired";
    case BSCI_OCSP_EXPIRED               : return "OCSP response expired (ocsp next update)";
    case BSCI_INVALID_CERT_STATUS        : return "Certificate status is invalid (revoked)";

    // Rights Object Acquisition (ROAP) errors 2x -- V2 DRM only
    case BSCI_ROAP_FAILURE               : return "General ROAP failure";
    case BSCI_PARSE_ERROR                : return "XML Parsing error (malformed)";
    case BSCI_INVALID_PERMISSION         : return "An unknown permission is specified";
    case BSCI_INVALID_RO                 : return "Invalid Rights Object";
    case BSCI_NOT_IN_DOMAIN              : return "Not a member of the domain";
    case BSCI_NOT_REGISTERED             : return "Not registered with the RI";
    case BSCI_REGISTRATION_FAILED        : return "RI Registration failure";
    case BSCI_JOIN_DOMAIN_FAILED         : return "General Join Domain failure";
    case BSCI_LEAVE_DOMAIN_FAILED        : return "General Leave Domain failure";
    case BSCI_DOMAIN_EXPIRED             : return "Domain Membership has expired";
    case BSCI_SIGNATURE_ERROR            : return "Signature error";
    case BSCI_NEED_DOMAIN_UPGD           : return "The domain generations are different, domain upgrade is needed";
    case BSCI_ROAP_RIGHTS_EXPIRED        : return "ROAP RigthsExpired status code, used in preview";
    case BSCI_REPLAY_FAILED              : return "the RO is rejected because of replay protection";
    case BSCI_INVALID_DRMTIME            : return "The RI reported that Device DRM Time is invalid";

    // File errors 3x, returned by ferror() and other operations on files.
    case BSCI_FILE_OP_ERROR              : return "Unspecified file error";
    case BSCI_FILE_OPEN_ERROR            : return "File open error";
    case BSCI_FILE_NOT_FOUND             : return "File not found";
    case BSCI_FILE_LOCKED                : return "File is locked";
    case BSCI_FILE_BADHANDLE             : return "Bad File handle";
    case BSCI_FILE_NUMFILES              : return "Max number open files reached";
    case BSCI_FILE_BADPATH               : return "Invalid path or filename";
    case BSCI_FILE_NOSPACE               : return "Out of disk space";
    case BSCI_FILE_ACCESS                : return "Access to the file/path is denied";
    case BSCI_FILE_BAD_FILEURI           : return "Bad reference to an object in a multipart DCF";

    // Http errors 4x -- V2 DRM only
    case BSCI_NO_CONNECTION              : return "The socket is not connected";
    case BSCI_HOST_NOT_FOUND             : return "Host not found by name";
    case BSCI_SOCKET_ERROR               : return "Socket Error";
    case BSCI_INVALID_SOCKET             : return "Invalid Socket";
    case BSCI_HTTP_ERROR                 : return "Not Expected Server Response";
    case BSCI_INVALID_CONTENT_TYPE       : return "Not Expected Content Type";
    case BSCI_HTTP_BADREQUEST            : return "HTTP 400 response";
    case BSCI_HTTP_FORBIDDEN             : return "HTTP 403 response";
    case BSCI_HTTP_NOT_FOUND             : return "HTTP 404 response";
    case BSCI_HTTP_SERVER_ERR            : return "HTTP 500 response";
    case BSCI_HTTP_REDIRECT              : return "HTTP redirect response";

    // Database errors 5x
    case BSCI_DB_GENERAL_ERROR           : return "Unknown database error occured";
    case BSCI_DBCREATE_FAILED            : return "Unable to create the database";
    case BSCI_DBOPEN_FAILED              : return "Unable to open a database table";
    case BSCI_DBWRITE_FAILED             : return "Unable to write a record to the database";
    case BSCI_DBREAD_FAILED              : return "Unable to read the database";
    case BSCI_DB_ROLLBACK                : return "Database rollback occured";

    // (P)DCF errors 6x
    case BSCI_ODCF_NOT_FOUND             : return "Invalid DCF file branding";
    case BSCI_ODRM_NOT_FOUND             : return "No OMA DRM container Box: invalid DCF";
    case BSCI_ODDA_NOT_FOUND             : return "No Content Object Box: invalid DCF";
    case BSCI_OHDR_NOT_FOUND             : return "No Common Headers Box: invalid DCF";
    case BSCI_TEXT_HEADER_NOT_FOUND      : return "No text header with that name";
    case BSCI_META_FIELD_NOT_FOUND       : return "No Meta field of that type";
    case BSCI_NO_ACTIVE_READER           : return "No active reader (DCF File Error)";
    case BSCI_FILE_SIZE_MISMATCH         : return "File Size is incorrect";
    case BSCI_INVALID_FILE_FORMAT        : return "Corrupt DCF file format.";
    case BSCI_NOT_SUPPORTED_ENCRYPT      : return "Unsupported encrypt method in DCF";
    case BSCI_BAD_DCFHASH                : return "Invalid DCF Hash - maybe corrupt file";
    case BSCI_NO_RIGHTS                  : return "Decrypt operation failed due to lack of rights";
    case BSCI_NOT_EXPECTED_BOX           : return "Unexpected box type or version not supported";

    // Activation errors 7x
    case BSCI_UNABLE_TO_ACTIVATE         : return "No option to activate or an error occured";
    case BSCI_IN_PROGRESS                : return "Activation is in progress";

    // Backup-Restore system errors
    case BSCI_BACKUP_NOT_FOUND           : return "Backup file not found";
    case BSCI_BACKUP_CORRUPT             : return "Backup file is corrupted";
    case BSCI_BACKUP_BADVERSION          : return "Backup file has bad version";

    // Stream errors
    case BSCI_STREAM_CONTENT_END         : return "End of the content has been reached.";
    case BSCI_STREAM_CONTENT_NEW         : return "New content found.";

    // General errors Fx
    case BSCI_MUTEX_ERROR                : return "Failure Locking or Releasing a mutex";
    case BSCI_NOT_INITIALIZED            : return "The library has not been initialized";
    case BSCI_RESOURCE_ERROR             : return "Failed to read the library resources";
    case BSCI_CRYPTO_ERROR               : return "General failure in Crypto functions";
    case BSCI_TIME_UNTRUSTED             : return "The agent couldn't do time synchronization or the device DRM Time is not trusted";
    case BSCI_EVALUATION                 : return "Returned if the function is unavailable in the evaluation version";
    case BSCI_ENVIRONMENT_ERROR          : return "The Graphical environment cannot be loaded.";
    case BSCI_UNHANDLED_EXCEPTION        : return "Unhandled exception has occured";
    case BSCI_GENERAL_ERROR              : return "General error.";
    default                              : return "Unknown error";
    }
}

/*!
    Outputs a drm agent error message on qWarning.
*/
void BSciDrm::printError( int error, const QString &method, const QString &filePath )
{
    qWarning() << "Error in method " << method << " on file " << filePath;
    qWarning() << QString::number( -error, 16 ) << ": " << getError( error );
}

/*!
    Outputs a drm agent error message on qWarning.
*/
void BSciDrm::printError( int error, const QString &method )
{
    qWarning() << "Error in method " << method;
    qWarning() << QString::number( -error, 16 ) << ": " << getError( error );
}

/*!
    Returns true if the dcf file at \a dcfFilePath has current valid rights of type \a permission.
*/
bool BSciDrm::hasRights( const QString &dcfFilePath, QDrmRights::Permission permission )
{
    EPermission  mode = transformPermission( permission );

    ERightsStatus status;

    BSCIGetLicenseInfo( context, formatPath( dcfFilePath ), mode, &status, 0, 0 );

    return status == RS_NotProtected || status == RS_Valid;
}

/*
    Gets the meta information string of type \a item from the drm protected file
    at \a filePath.
*/
QString BSciDrm::getMetaData( const QString &filePath, enum EMetaData item )
{
    const QByteArray localPath = formatPath( filePath );

    size_t bufferSize = 0;

    int error = BSCIGetMetaData( context, localPath, item, &bufferSize, 0 );

    if( error == BSCI_META_FIELD_NOT_FOUND || bufferSize <= 0 )
        return QString();

    char *buffer = new char[ bufferSize + 1 ];

    error = BSCIGetMetaData( context, localPath, item, &bufferSize, buffer );

    if( error == BSCI_RESULT_TRUNCATED )
        qWarning( "MetaData Truncated" );

    QString metaData( buffer );

    delete buffer;

    return metaData.remove( '"' ).trimmed();
}

/*!
    Returns the size in bytes of the plaintext content of the dcf file located at \a dcfFilePath.
*/
qint64 BSciDrm::getContentSize( const QString &dcfFilePath )
{
    qint64 size = -1;

    // In order to get the size of the content the file needs to be opened.
    // Find if there are any rights to the content and if so open it using
    // those rights and get the size.
    SBSciContentAccess *ops = BSCIGetDrmContentAccess( context );

    if( ops != 0 )
    {
        EPermission  mode = PT_LAST;

        if     ( hasRights( dcfFilePath, QDrmRights::Play    ) ) mode = PT_Play;
        else if( hasRights( dcfFilePath, QDrmRights::Display ) ) mode = PT_Display;
        else if( hasRights( dcfFilePath, QDrmRights::Execute ) ) mode = PT_Execute;
        else if( hasRights( dcfFilePath, QDrmRights::Print   ) ) mode = PT_Print;
        else if( hasRights( dcfFilePath, QDrmRights::Export  ) ) mode = PT_Export;

        if( mode != PT_LAST )
        {
            TFileHandle file = (*ops->bsci_fopen)( context, formatPath( dcfFilePath ), "rb", mode, 0 );

            size = (*ops->bsci_getContentSize)( file );

            (*ops->bsci_close)( file );
        }
    }

    return size;
};

QString BSciDrm::getPreviewUri( const QString &dcfFilePath )
{
    QString uri;

    SBSciFileInfo fileInfo;

    memset( &fileInfo, 0, sizeof(fileInfo) );

    const QByteArray localPath = formatPath( dcfFilePath );

    if( BSCIGetFileInfo( context, localPath, 0, &fileInfo ) != BSCI_NO_ERROR )
    {
        if( fileInfo.numObjects > 0 && fileInfo.previewURIs[ 0 ] )
        {
            uri = QString( fileInfo.previewURIs[ 0 ] );
        }

        BSCIReleaseFileInfo( context, &fileInfo );
    }

    return uri;
}

QStringList BSciDrm::convertTextArray( const SBSciTextArray &text )
{
    QStringList list;

    for( uint i = 0; i < text.count; i++ )
        list.append( QString( text.text[ i ] ) );

    return list;
}

const QByteArray BSciDrm::formatPath( const QString &filePath )
{
    QByteArray formattedPath = filePath.toLocal8Bit();

    formattedPath.replace( ".odf/OBJECT" , ".odf;OBJECT" );

    return formattedPath;
}
