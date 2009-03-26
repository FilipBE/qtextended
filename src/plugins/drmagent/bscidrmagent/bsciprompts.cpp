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
#include "bsciprompts.h"
#include "bscidrm.h"
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <QtDebug>
#include <QTimer>
#include <QContentSet>
#include <QSettings>
#include <QDateTime>
#include <QMimeType>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QUrl>
#include <QMessageBox>
#include <QThread>

/*!
    \class BSciPrompts
    \inpublicgroup QtDrmModule
    \brief The BSciPrompts class provides OMA DRM activation messages.

    \internal
*/


/*!
    \enum BSciPrompts::ActivationReason

    Indicates the reason the content is being activated.  Some notifications use this to differ the message
    displayed based on the context of the activation.

    \value Open The user has attempted to open content with invalid rights.
    \value Expired  The rights for content expired while the user was rendering.
    \value SingleRemainingUse The user has finished rendering content following which the rights only allow for one more use.

    \internal
*/

/*!
    \enum BSciPrompts::DomainAction

   Indicates the type of action undertaken to alter a device's status as member of a domain.

    \value JoinDomain Add the device to a domain.
    \value UpgradeDomain Upgrade the device's membership in a domain.
    \value LeaveDomain Remove the device from a domain.
*/


SBSciCallbacks BSciPrompts::m_callbacks =
{
    &BSciPrompts::notify_sd_ro,
    &BSciPrompts::notify_v2_install,
    &BSciPrompts::allow_domain_join,
    &BSciPrompts::allow_domain_leave,
    &BSciPrompts::allow_register_agent,
    &BSciPrompts::allow_acquire_ro,
    &BSciPrompts::store_dcf,
    &BSciPrompts::browse_rights_issuer,
    &BSciPrompts::browse_join_domain,
    &BSciPrompts::browse_register,
    &BSciPrompts::allow_preview_download,
    &BSciPrompts::allow_silent_download,
    &BSciPrompts::allow_time_sync,
    &BSciPrompts::time_sync_status
};

/*!
    Constructs a new BSciPrompts object.
*/
BSciPrompts::BSciPrompts()
    : m_response( 0 )
    , m_silentRoap( "/OmaDrmAgent" )
{
    QSettings conf( "Trolltech", "OmaDrm" );

    m_silentRoapEnabled = conf.value( QLatin1String( "silentroap" ), false ).toBool();
}

/*!
    Returns a static instance of a BSciPrompts object.
*/
BSciPrompts *BSciPrompts::instance()
{
    static BSciPrompts instance;

    return &instance;
}

/*!
    Returns a struct containing callback functions for the Beep Science DRM agent.
*/
SBSciCallbacks *BSciPrompts::callbacks()
{
    return &m_callbacks;
}


/*!
    Displays a prompt indicating that a ROAP action completed successfully, the \a triggerStatus contains the
    details of the ROAP action.
*/
void BSciPrompts::notifySuccess( SBSciRoapStatus *triggerStatus ) const
{
    switch( triggerStatus->event )
    {
    case TT_Register:
        instance()->notifyRegistrationSuccess( triggerStatus->riAlias, triggerStatus->domainAlias );
        break;
    case TT_ROAcquisition:
    {
        QStringList fileNames = BSciDrm::convertTextArray( triggerStatus->contentRef );

        if( fileNames.isEmpty() )
            instance()->notifyRightsObjectsReceived( BSciDrm::convertTextArray( triggerStatus->roAliases ) );
        else
        {
            instance()->notifyContentAvailable( fileNames );
        }

        break;
    }
    case TT_JoinDomain:
        instance()->notifyDomainSuccess( triggerStatus->domainAlias, JoinDomain );
        break;
    case TT_LeaveDomain:
        instance()->notifyDomainSuccess( triggerStatus->domainAlias, LeaveDomain );
        break;
    default:
        break;
    }
}

/*!
    Displays a prompt indicating that a ROAP action failed and the \a error, the \a triggerStatus contains the
    details of the ROAP action.
*/
void BSciPrompts::notifyFailure( SBSciRoapStatus *triggerStatus, int error ) const
{
    switch( triggerStatus->event )
    {
    case TT_Register:
        instance()->notifyRegistrationFailure(
                triggerStatus->riAlias,
                triggerStatus->domainAlias,
                formatError( error ) );
        break;
    case TT_JoinDomain:
        instance()->notifyDomainFailure(
                triggerStatus->domainAlias,
                JoinDomain,
                formatError( error ) );
        break;
    case TT_LeaveDomain:
        instance()->notifyDomainFailure(
                triggerStatus->domainAlias,
                LeaveDomain,
                formatError( error ) );
        break;
    default:
        instance()->notifyError( BSciDrm::getError( error ) + QLatin1Char( '\n' ) + formatError( error ) );
    }
}


/*!
    Notifies the user that the given \a content does not have valid rights to render with the given \a permission
    and that the content cannot be reactivated.  If \a permission is \c{QDrmRights::Unrestricted)} the user will
    be informed that the content must be opened in an application that support DRM content.  The message displayed
    will differ based on the \a reason the user was prompted to acquire new rights.
*/
void BSciPrompts::notifyCannotActivate( const QContent &content, QDrmRights::Permission permission, ActivationReason reason ) const
{
    QString title;
    QString message;

    if( permission == QDrmRights::Unrestricted )
    {
        title   = tr( "Content Unsupported" );
        message = tr( "'%1' cannot be opened in an application that doesn't support licensed content.",
                      "%1 = name of content" );
    }
    else
    {
        title   = tr( "Cannot activate" );

        switch( reason )
        {
        case Open:
            message = tr( "'%2' has no %1 and cannot be reactivated.",
                        "%1 = play|display|execute|print|export license, %2 = name of content" )
                    .arg( permissionString( permission ) );
            break;
        case Expired:
            message = tr( "The %1 for '%2' has expired and cannot be reactivated.",
                          "%1 = play|display|execute|print|export license, %2 = name of content" )
                    .arg( permissionString( permission ) );
            break;
        default:
            return;
        }
    }

    message = QString( "<qt>%1</qt>" ).arg( message.arg( content.name() ) );

    information( title, message );
}

/*!
    Displays a prompt informing the user that the \a content cannot be rendered due to invalid rights and that
    a preview will be rendered instead.
*/
void BSciPrompts::notifyUseEmbeddedPreview( const QContent &content ) const
{
    QString title   = tr( "Preview" );
    QString message = tr( "'%1' is inaccessible, a preview version will be opened instead.", "%1 = name of content" )
            .arg( content.name() );

    message = QString( "<qt>%1</qt>" ).arg( message );

    information( title, message );
}

/*!
    Displays a prompt informing the user that \a permission render rights for \a content are installed but they
    will not become valid until after a future \a date.
*/
void BSciPrompts::notifyFutureRights( const QContent &content, const QDateTime &date, QDrmRights::Permission permission ) const
{
    QString title   = tr( "Content Unavailable" );
    QString message = tr( "The %1 for '%2' does not allow it to be accessed before %3.",
                          "%1 = play|display|execute|print|export license, "
                          "%2 = name of content, %3 = access date/time" )
            .arg( permissionString( permission ) )
            .arg( content.name() )
            .arg( date.toString() );

    message = QString( "<qt>%1</qt>" ).arg( message );

    information( title, message );
}

/*!
    Displays a prompt informing the user that a rights have been received for content with the file names \a fileNames.

    The notification is only displayed for content if the arrival of the rights was unexpected.

    If rights are received for multiple files agreeing to view the content will display a list of the content, if the
    rights are for just one file it will be opened in the default application for its mime type.

    \sa notifyRightsObjectsReceived()
*/
void BSciPrompts::notifyContentAvailable( const QStringList &fileNames ) const
{
    QContentList unexpected;

    QDateTime now = QDateTime::currentDateTime();

    foreach( QString fileName, fileNames )
    {
        QContent content( QContent::execToContent( fileName ) );

        if( content.isNull() )
            unexpected.append( QContent( fileName ) );
        else if( content.lastUpdated().secsTo( now ) < 25 )
            QContent( fileName ).commit();
        else if( content.isValid() )
            unexpected.append( content );
    }

    if( unexpected.count() == 1 )
    {
        QContent content = unexpected.first();

        QString title   = tr( "Content available" );
        QString message = tr( "Access to '%1' is now available.  View content now?", "%1 = name of content" )
                .arg( content.name() );

        message = QString( "<qt>%1</qt>" ).arg( message );

        if( question( title, message ) == QMessageBox::Yes )
            content.execute();
    }
    else if( unexpected.count() > 1 )
    {
        QContentSet content;

        foreach( QContent c, unexpected )
            content.add( c );

        QString title   = tr( "Content available" );
        QString message = tr( "Access to new content is now available.  View content now?" );

        message = QString( "<qt>%1</qt>" ).arg( message );

        if( question( title, message ) == QMessageBox::Yes )
        {
            QtopiaServiceRequest request( "ContentSetView", "showContentSet(QContentSet)" );
            request << content;
            request.send();
        }
    }
}

/*!
    Displays a dialog notifying the user that rights objects with the given \a aliases have been received.
    If the rights objects correspond to content already on the device notifyContentAvailable() should be called
    instead to allow the user to access the content.

    \sa notifyContentAvailable()
*/
void BSciPrompts::notifyRightsObjectsReceived( const QStringList &aliases ) const
{
    QString title   = tr( "Licenses Received" );
    QString message;


    if( aliases.isEmpty() )
        message = tr( "Licenses successfully received." );
    else
        message = tr( "Licenses successfully received: %1.", "%1 = comma seperated list of rights object aliases" )
                .arg( aliases.join( ", " ) );  //?

    message = QString( "<qt>%1</qt>" ).arg( message );

    information( title, message );
}

/*!
    Displays a dialog informing the user that they have successfully registered with the rights issuer with
    the given \a alias and \a domain.  If \a alias or \a domain is empty a more generic message will be
    displayed.
*/
void BSciPrompts::notifyRegistrationSuccess( const QString &alias, const QString domain ) const
{
    QString title   = tr( "Registration successful" );
    QString message;

    if( alias.isEmpty() && domain.isEmpty() )
        message = title;
    else if( !alias.isEmpty() && !domain.isEmpty() )
        message = tr( "Registration with %1 <%2> successful", "%1 = rights issuer alias, %2 = domain alias" )
                .arg( alias  )
                .arg( domain );
    else
        message = tr( "Registration with %1 successful" )
                .arg( domain.isEmpty() ? alias : domain );

    message = QString( "<qt>%1</qt>" ).arg( message );

    information( title, message );
}

/*!
    Displays a dialog informing the user that a domain \a action involving the domain with the given \a alias
    has completed successfully.
 */
void BSciPrompts::notifyDomainSuccess( const QString &alias, DomainAction action ) const
{
    QString title;
    QString message;

    switch( action )
    {
    case JoinDomain:
        message = tr( "Joined domain", "The device is added to a domain" );
        break;
    case UpgradeDomain:
        message = tr( "Upgraded domain", "The device's membership in a domain is upgraded" );
        break;
    case LeaveDomain:
        message = tr( "Left domain", "The device is removed from a domain" );
    }

    if( alias.isEmpty() )
    {
        switch( action )
        {
        case JoinDomain:
            message = tr( "Joined domain", "The device is added to a domain" );
            break;
        case UpgradeDomain:
            message = tr( "Upgraded domain successfully", "The device's membership in a domain is upgraded" );
            break;
        case LeaveDomain:
            message = tr( "Left domain successfully", "The device is removed from a domain" );
        }
    }
    else
    {
        switch( action )
        {
        case JoinDomain:
            message = tr( "Joined %1 successfully", "%1 = domain alias" )
                    .arg( alias );
            break;
        case UpgradeDomain:
            message = tr( "Upgraded %1 successfully", "%1 = domain alias" )
                    .arg( alias );
            break;
        case LeaveDomain:
            message = tr( "Left %1 successfully", "%1 = domain alias" )
                    .arg( alias );
        }
    }

    message = QString( "<qt>%1</qt>" ).arg( message );

    information( title, message );
}

/*!
    Displays a dialog informing the user that they have failed to register with the rights issuer with
    the given \a alias and \a domain.  If \a alias or \a domain is empty a more generic message will be
    displayed.  If \a error is non empty it will also be presented to the user.
 */
void BSciPrompts::notifyRegistrationFailure( const QString &alias, const QString domain, const QString &error ) const
{
    QString title   = tr( "Error" );
    QString message;

    if( alias.isEmpty() && domain.isEmpty() )
        message = tr( "Registration failed" );
    else if( !alias.isEmpty() && !domain.isEmpty() )
        message = tr( "Registration with %1 <%2> failed.", "%1 = rights issuer alias, %2 = domain alias" )
                .arg( alias  )
                .arg( domain );
    else
        message = tr( "Registration with %1 failed." )
                .arg( domain.isEmpty() ? alias : domain );

    if( !error.isEmpty() )
        message += QLatin1String( "<br/>" ) + error;

    message = QString( "<qt>%1</qt>" ).arg( message );

    information( title, message );
}

/*!
    Displays a dialog informing the user that a domain \a action involving the domain with the given \a alias
    has failed. If \a error is non empty it will also be presented to the user.
 */
void BSciPrompts::notifyDomainFailure( const QString &alias, DomainAction action, const QString &error ) const
{
    QString title   = tr( "Error" );
    QString message;

    if( alias.isEmpty() )
    {
        switch( action )
        {
            case JoinDomain:
                message = tr( "Failed to join domain." );
                break;
            case UpgradeDomain:
                message = tr( "Failed to upgrade domain." );
                break;
            case LeaveDomain:
                message = tr( "Failed to leave domain." );
        }
    }
    else
    {
        switch( action )
        {
            case JoinDomain:
                message = tr( "Joined %1 successfully.", "%1 = domain alias" )
                        .arg( alias );
                break;
            case UpgradeDomain:
                message = tr( "Upgraded %1 successfully.", "%1 = domain alias" )
                        .arg( alias );
                break;
            case LeaveDomain:
                message = tr( "Left %1 successfully.", "%1 = domain alias" )
                        .arg( alias );
        }
    }

    if( !error.isEmpty() )
        message += QLatin1String( "<br/>" ) + error;

    message = QString( "<qt>%1</qt>" ).arg( message );

    information( title, message );
}

/*!
    Displays a dialog informing the user that an \a error has occurred.
*/
void BSciPrompts::notifyError( const QString &error ) const
{
    QString title   = tr( "Error" );
    QString message = QString( "<qt>%1</qt>" ).arg( error );

    information( title, message );
}

/*!
    Prompts the user for permission to Join, Update or Leave the domain with the given \a alias as per \a action.

    Returns true if the user grants permission; false otherwise.
*/
bool BSciPrompts::requestDomainPermission( const QString &alias, DomainAction action ) const
{
    QString title;
    QString message;

    switch( action )
    {
    case JoinDomain:
        title = tr( "Join domain?" );
        message = tr( "Allow device to join domain %1?", "%1 = domain alias (id if alias empty)" );
    case UpgradeDomain:
        title = tr( "Upgrade domain?" );
        message = tr( "Allow device to upgrade domain %1?", "%1 = domain alias (id if alias empty)" );
    case LeaveDomain:
        title = tr( "Leave domain?" );
        message = tr( "Allow device to leave domain %1?", "%1 = domain alias (id if alias empty)" );
    }

    message = message.arg( alias );

    message = QString( "<qt>%1</qt>" ).arg( message );

    return question( title, message ) == QMessageBox::Yes;
}

/*!
    Prompts the user for permission to register with the rights issuer with the given \a alias and \a url.

    Returns true if the user grants permission; false otherwise.
*/
bool BSciPrompts::requestRegistrationPermission( const QString &alias, const QString &url ) const
{
    QString title = tr( "Register rights issuer" );
    QString message;

    if( !alias.isEmpty() )
        message = tr( "Allow device to register with %1 <%2>?",
                      "%1 = rights issuer alias %2 = rights issuer url" )
            .arg( alias )
            .arg( url );
    else
        message = tr( "Allow device to register with %1?",
                      "%1 = rights issuer url" )
                .arg( url );

    message = QString( "<qt>%1</qt>" ).arg( message );

    return question( title, message ) == QMessageBox::Yes;
}

/*!
    Requests permission from the user to install a rights object with the given \a alias from the given rights \a issuer.

    Returns true upon success; otherwise returns false.
*/
bool BSciPrompts::requestRightsObjectPermission( const QString &alias, const QString &issuer ) const
{
    QString title = tr( "Download license?" );
    QString message;

    if( !alias.isEmpty() && !issuer.isEmpty() )
        message = tr( "Download license for %1 from %2?", "%1 = rights object alias, %2 = rights issuer alias" )
                .arg( alias )
                .arg( issuer );
    else if( !alias.isEmpty() )
        message = tr( "Download license for %1?", "%1 = rights object alias" )
                .arg( alias );
    else if( !issuer.isEmpty() )
        message = tr( "Download license from %1?", "%1 = rights issuer alias" )
                .arg( issuer );
    else
        message = title;

    message = QString( "<qt>%1</qt>" ).arg( message );

    return question( title, message ) == QMessageBox::Yes;
}

/*!
    Informs the user that rights for the given \a content may be obtained and from a \a url and asks
    if they would like to open the URL. If the user agrees then the URL will be opened in a web browser
    and this return true, otherwise this will return false.
*/
bool BSciPrompts::requestOpenUrl( const QContent &content, const QString &url ) const
{
    QString title = tr( "Acquire license?" );

    QString message = tr( "Acquire license for '%1' from %2?",
                          "%1 = name of content, %2 = rights issuer url" )
            .arg( content.name() )
            .arg( QUrl( url ).host() );

    message = QString( "<qt>%1</qt>" ).arg( message );

    if( question( title, message ) == QMessageBox::Yes )
    {
        QtopiaServiceRequest e( "WebAccess", "openUrl(QString)" );
        e << url;
        e.send();

        return true;
    }
    else
        return false;
}

/*!
    Informs the user that the rights for to open \a content with the given \a permission are not valid or will
    soon be invalidated and that new rights may be obtained and from a \a url. If the user agrees then the URL
    will be opened in a web browser and this return true, otherwise this will return false.  The message displayed
    will differ based on the \a reason the user was prompted to acquire new rights.
 */
bool BSciPrompts::requestOpenUrl( const QContent &content, const QString &url, QDrmRights::Permission permission, ActivationReason reason ) const
{
    QString title = tr( "Acquire license?" );

    QString message;

    switch( reason )
    {
    case Open:
        message = tr( "'%1' has no %2. Acquire new license from %3 now?",
                      "%1 = name of content, "
                      "%2 = play|display|execute|print|export license, "
                      "%3 = rights issuer url"
                    );
        break;
    case Expired:
        message = tr( "The %2 for '%1' has expired.  Acquire new license from %3 now?",
                        "%1 = name of content, "
                                "%2 = play|display|execute|print|export license, "
                                "%3 = rights issuer url"
                    );
        break;
    case SingleRemainingUse:
        message = tr( "The %2 for '%1' has one use remaining.  Acquire new license from %3 now?",
                        "%1 = name of content, "
                        "%2 = play|display|execute|print|export license, "
                        "%3 = rights issuer url"
                    );
        break;
    }

    message = QString( "<qt>%1</qt>" )
        .arg( message
              .arg( content.name() )
              .arg( permissionString( permission ) )
              .arg( QUrl( url ).host() ) );

    if( question( title, message ) == QMessageBox::Yes )
    {
        QtopiaServiceRequest e( "WebAccess", "openUrl(QString)" );
        e << url;
        e.send();

        return true;
    }
    else
        return false;
}

/*!
    Requests permission from the user to download a preview license for the given \a content.

    Returns true upon success; otherwise returns false.
*/
bool BSciPrompts::requestPreviewPermission( const QContent &content ) const
{
    QString title = tr( "Download preview?" );

    QString message = tr( "A preview license for %1 may be available, download now?", "%1 = content name" );

    message = QString( "<qt>%1</qt>" ).arg( message.arg( content.name() ) );

    return question( title, message ) == QMessageBox::Yes;
}


/*!
    Requests permission from the user to navigate to a \a url in order to find out more about the domain required
    to view the protected content \a content.

    Launches a web browser and returns true if permission is granted; otherwise returns false.
 */
bool BSciPrompts::requestOpenDomainUrl( const QContent &content, const QString &url ) const
{
    QString title = tr( "Join domain?" );

    QString message = tr( "A license for '%1' is installed but is not currently accessible, "
                          "more information is available from %2.  View now?",
                          "%1 = name of content, %2 = rights issuer url" )
            .arg( content.name() )
            .arg( QUrl( url ).host() );

    message = QString( "<qt>%1</qt>" ).arg( message );

    if( question( title, message ) == QMessageBox::Yes )
    {
        QtopiaServiceRequest e( "WebAccess", "openUrl(QString)" );
        e << url;
        e.send();

        return true;
    }
    else
        return false;
}

/*!
    Requests permission from the user to navigate to a \a url in order to register a rights issuer for the
    the DRM protected content \a content.

    Launches a web browser and returns true if permission is granted; otherwise returns false.
*/
bool BSciPrompts::requestOpenRegistrationUrl( const QContent &content, const QString &url ) const 
{
    QString title = tr( "Join domain?" );

    QString message = tr( "The license available for '%1' was issued by an unknown rights issuer, "
                          "more information is available from %2.  View now?",
                          "%1 = name of content, %2 = rights issuer url" )
            .arg( content.name() )
            .arg( QUrl( url ).host() );

    message = QString( "<qt>%1</qt>" ).arg( message );

    if( question( title, message ) == QMessageBox::Yes )
    {
        QtopiaServiceRequest e( "WebAccess", "openUrl(QString)" );
        e << url;
        e.send();

        return true;
    }
    else
        return false;
}

/*!
    Requests permission from the user to download a free license for the DRM protected content \a content from the
    given \a url.

    Returns true if permission to download the license was granted; returns false otherwise.
*/
bool BSciPrompts::requestSilentROPermission( const QContent &content, const QString &url ) const
{
    QString title = tr( "Download license?" );

    QString message = tr( "A free license for %1 may be available from %2, download now?", "%1 = content name" );

    message = QString( "<qt>%1</qt>" ).arg( message.arg( content.name() ).arg( url ) );

    return question( title, message ) == QMessageBox::Yes;
}

/*!
    Requests permission from the user to download a preview license for the DRM protected content \a content from the
    given \a url.

    Returns true if permission to download the license was granted; returns false otherwise.
 */
bool BSciPrompts::requestPreviewROPermission( const QContent &content, const QString &url ) const
{
    QString title = tr( "Download preview?" );

    QString message = tr( "A preview license for %1 may be available from %2, download now?", "%1 = content name" );

    message = QString( "<qt>%1</qt>" ).arg( message.arg( content.name() ).arg( url ) );

    return question( title, message ) == QMessageBox::Yes;
}

/*!
    Returns true if silent roap is enabled.
*/
bool BSciPrompts::silentRoapEnabled() const
{
    return m_silentRoapEnabled;
}

/*!
    Return a user readable string describing the type of license for a given \a permission.
*/
QString BSciPrompts::permissionString( QDrmRights::Permission permission ) const
{
    switch( permission )
    {
    case QDrmRights::Play:
        return tr( "play license" );
    case QDrmRights::Display:
        return tr( "display license" );
    case QDrmRights::Execute:
        return tr( "execute license" );
    case QDrmRights::Print:
        return tr( "print license" );
    case QDrmRights::Export:
        return tr( "export license" );
    default:
        qWarning() << "Invalid permission";
        return QString();
    }
}

/*!
    Returns the \a error number as a positive hexadecimal value.
*/
QString BSciPrompts::formatError( int error )
{
    return tr( "Error: %1" ).arg( QString::number( -error, 16 ) );
}


void BSciPrompts::_question( const QString &title, const QString &message )
{
    QMessageBox prompt( title, message, QMessageBox::NoIcon, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton, 0 );

    m_response = prompt.exec();
}

void BSciPrompts::_information( const QString &title, const QString &message )
{
    QMessageBox prompt( title, message, QMessageBox::NoIcon, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, 0 );

    prompt.exec();
}

/*!
    Displays a question message box with the given \a title and \a message.

    Returns the message box response.
*/
int BSciPrompts::question( const QString &title, const QString &message ) const
{
    QMetaObject::invokeMethod(
            const_cast< BSciPrompts * >( this ),
            "_question",
            thread() == QThread::currentThread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection,
            Q_ARG( QString, title ),
            Q_ARG( QString, message ) );

    return m_response;
}

/*!
    Displays an information message box with the given \a title and \a message.
*/
void BSciPrompts::information( const QString &title, const QString &message ) const
{
    QMetaObject::invokeMethod(
            const_cast< BSciPrompts * >( this ),
            "_information",
            thread() == QThread::currentThread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection,
            Q_ARG(QString,title),
            Q_ARG(QString,message) );
}

/*!
    Updates the silent roap status in response to a change event.
*/
void BSciPrompts::silentRoapChanged()
{
    m_silentRoapEnabled = m_silentRoap.value( "SilentRoap", m_silentRoapEnabled ).toBool();
}

/*!
    Implementation of the notify_sd_ro( const char *\a {fileName}, SBSciRights * \a {rights}) DRM agent callback function.
*/
int BSciPrompts::notify_sd_ro(const char *fileName, SBSciRights *rights)
{
    Q_UNUSED( rights );
    Q_UNUSED( fileName );

    return 1;
}

/*!
    Implementation of the notify_v2_install( SBSciRoapStatus *\a {triggerStatus}, int \a {success}, int \a {silent} )
    DRM agent callback function.
*/
int BSciPrompts::notify_v2_install(SBSciRoapStatus *triggerStatus, int success, int silent)
{
    Q_UNUSED( triggerStatus );
    qLog(DRMAgent) << __PRETTY_FUNCTION__ << QString::number( -success, 16 ) << silent;

    return 0;
}

/*!
    Implementation of the allow_domain_join( const char *\a {filename}, const char *\a {domainID}, const char *\a {domainAlias} )
    DRM agent callback function.
*/
int BSciPrompts::allow_domain_join(const char *filename, const char *domainID, const char *domainAlias)
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__ << filename << domainID << domainAlias;

    return ( !instance()->silentRoapEnabled() || instance()->requestDomainPermission( domainAlias, JoinDomain ) ) ? 1 : 0;
}

/*!
    Implementation of the allow_domain_leave( const char *\a {domainID}, const char *\a {domainAlias} ) DRM agent callback function.
*/
int BSciPrompts::allow_domain_leave(const char *domainID, const char *domainAlias)
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__ << domainID << domainAlias;

    return instance()->requestDomainPermission( domainAlias, LeaveDomain ) ? 1 : 0;
}

/*!
    Implementation of the allow_register_agent( const char *\a {riID}, const char *\a {riAlias},  const char *\a {riUrl} ) DRM agent callback function.
*/
int BSciPrompts::allow_register_agent(const char *riID, const char *riAlias,  const char *riUrl)
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__ << riID << riAlias << riUrl;

    return instance()->requestRegistrationPermission( riAlias, riUrl ) ? 1 : 0;
}

/*!
    Implementation of the allow_acquire_ro( const char *\a {roID}, const char *\a {roAlias},  const char *\a {riAlias} ) DRM agent callback function.
*/
int BSciPrompts::allow_acquire_ro(const char *roID, const char *roAlias,  const char *riAlias)
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__ << roID << roAlias << riAlias;

    return instance()->requestRightsObjectPermission( roAlias, riAlias ) ? 1 : 0;

    return 1;
}

/*!
    Implementation of the store_dcf( const char *\a {tmp_filename}, char *\a {stored_filename} ) DRM agent callback function.
*/
int BSciPrompts::store_dcf(const char *tmp_filename, char *stored_filename)
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__ << tmp_filename << stored_filename;

    return 0;
}

/*!
    Implementation of the browse_rights_issuer( const char *\a {fileURI}, const char *\a {rightsIssuerURL}, ERightsStatus \a {status} )
    DRM agent callback function.
*/
int BSciPrompts::browse_rights_issuer( const char *fileURI, const char *rightsIssuerURL, ERightsStatus status )
{
    Q_UNUSED( status );

    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    return instance()->requestOpenUrl( QContent( fileURI ), rightsIssuerURL ) ? 1 : 0;
}

/*!
    Implementation of the browse_join_domain( const char *\a {fileURI}, const char *\a {domainID}, const char *\a {riURL}, ERightsStatus *\a {status} )
    DRM agent callback function.
*/
int BSciPrompts::browse_join_domain( const char *fileURI, const char *domainID, const char *riURL, ERightsStatus *status )
{
    Q_UNUSED( domainID );
    Q_UNUSED( status );

    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    return instance()->requestOpenDomainUrl( QContent( fileURI ), riURL ) ? 1 : 0;
}

/*!
    Implementation of the browse_register( const char *\a {fileURI}, const char *\a {riURL}, ERightsStatus \a {status} ) DRM agent callback function.
*/
int BSciPrompts::browse_register( const char *fileURI, const char *riURL, ERightsStatus status )
{
    Q_UNUSED( status );

    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    return instance()->requestOpenRegistrationUrl( QContent( fileURI ), riURL ) ? 1 : 0;
}

/*!
    Implementation of the allow_preview_download( const char *\a {fileURI}, const char *\a {previewURL} ) DRM agent callback function.
*/
int BSciPrompts::allow_preview_download( const char *fileURI, const char *previewURL )
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    return instance()->requestPreviewROPermission( QContent( fileURI ), previewURL ) ? 1 : 0;
}

/*!
    Implementation of the allow_silent_download( const char *\a {fileUri}, const char *\a {silentURL} ) DRM agent callback function.
*/
int BSciPrompts::allow_silent_download( const char *fileUri, const char *silentURL )
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    return instance()->requestSilentROPermission( QContent( fileUri ), silentURL ) ? 1 : 0;
}

/*!
    Implementation of the allow_time_sync( int \a {onRORITS}, SBSciTime *\a {currentDRMTime} ) DRM agent callback function.
*/
int BSciPrompts::allow_time_sync( int onRORITS, SBSciTime *currentDRMTime )
{
    Q_UNUSED( onRORITS );

    QDateTime current( 
            QDate( currentDRMTime->year, currentDRMTime->month, currentDRMTime->day ),
            QTime( currentDRMTime->hour, currentDRMTime->minute, currentDRMTime->second ) );

    qLog(DRMAgent) << __PRETTY_FUNCTION__ << current;

    return 1;
}

/*!
    Implementation of the time_sync_status( int \a {status} ) DRM agent callback function.
*/
void BSciPrompts::time_sync_status( int status )
{
    SBSciTime time;
    memset( &time, 0, sizeof(time) );
    BSCIGetDRMTime( BSciDrm::context, &time );

    QDateTime current( QDate( time.year, time.month, time.day ), QTime( time.hour, time.minute, time.second ) );

    qLog(DRMAgent) << __PRETTY_FUNCTION__ << status << current;
}

