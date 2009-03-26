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

#include "qdrmcontent.h"
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>
#include <QThread>
#include <qtopiaservices.h>
#include "drmcontent_p.h"
#include <qmimetype.h>
#include <QUniqueId>
#include <QPointer>
#include "qcontentengine_p.h"

#include <QAbstractFileEngineHandler>

class QDrmContentPrivate : public QObject
{
    Q_OBJECT
public:
    QDrmContent *c;
    QPointer< QDrmContentLicense > license;
    QDrmRights::Permission permission;
    QDrmContent::LicenseOptions options;
    QWidget *focus;
    QContent content;


signals:
    void rightsExpired( const QDrmContent &content );

public slots:
    void rightsExpired( const QContent &content, QDrmRights::Permission permissionType );
};

/*!
    Receives a DrmContentLicense rightsExpired signal and emits a QDrmContent
    rightsExpired signal for the same content.
*/
void QDrmContentPrivate::rightsExpired( const QContent &content, QDrmRights::Permission permission )
{
    Q_UNUSED( content    );
    Q_UNUSED( permission );

    if( license )
        emit rightsExpired( *c );
}

/*!
    \class QDrmContent
    \inpublicgroup QtBaseModule

    \brief The QDrmContent class provides applications with access to DRM protected content.

    In order to enforce the constraints placed on DRM protected content it is neccessary to validate the content's
    permissions before rendering, and to meter usage and revalidate permissions during rendering.  QDrmContent provides
    the interface through which applications can request access to DRM protected content and DRM agents can monitor content
    usage.  An individual QDrmContent object allows access to a single item of content at a time, concurrent access to
    multiple content items requires a QDrmContent object for each item of content.

    File access to DRM content is requested  using the the \l requestLicense() method.  If requestLicense() returns
    true access to the content has been granted and it is possible to access the content using the standard Qt file
    API.  Calling \l releaseLicense() will relinquish access rights to the content, QDrmContent will automatically
    release any already held licenses when a new license is requested but a license should be released as soon as
    possible once done with a file to close access to the file.

    To enforce time and use based rights the DRM agent needs to notified of the number of uses and the duration of
    each use.  The renderStarted, renderPaused and renderStopped slots track this information and should be called
    as appropriate when usage state changes.

    QDrmContent will emit a \l rightsExpired() signal if the rights to the content expires while being rendered.
    If a DRM protected file is open when this signal is emitted it will remain open, however it will not be possible
    to reopen the file once it is closed.

    An implementation for a simple media player would be:
    \code
    class MediaPlayer : public QWidget
    {
    public:
        MediaPlayer( QWidget *parent = 0 );
    public slots:
        bool open( const QContent &content );
        void play();
        void pause();
        void stop();

    signals:
        void playing();
        void paused();
        void stopped();

    private:
        QDrmContent openContent;
    };

    MediaPlayer::MediaPlayer( QWidget *parent = 0 )
        : QWidget( parent )
    {
        openContent.setPermission( QDrmRights::Play );

        // Connect status signals and slots.
        connect( this, SIGNAL(playing()), &openContent, SLOT(renderStarted()));
        connect( this, SIGNAL(paused()), &openContent, SLOT(renderPaused()));
        connect( this, SIGNAL(stopped()), &openContent, SLOT(renderStopped()));

        // Stop playback when the rights have expired.
        connect( &openContent, SIGNAL(rightsExpired(QDrmContent)), this, SLOT(stop()) );

        // Perform initialisation.
    }

    bool MediaPlayer::open( const QContent &content )
    {
        if( openContent.requestLicense( content ) )
        {
            // Open file as normal.

            return true;
        }
        else
            return false;
    }

    void MediaPlayer::close()
    {
        // Close file as normal.

        openContent.releaseLicense();
    }

    \endcode

    By default if a license is requested for content without any current rights QDrmContent will prompt the user
    to activate the content which if successful will result in a license being granted.  QDrmContent will also
    prompt the user to reactivate content which has expired or will in expire in the immediate future when a
    license for it is released.  Both prompts can be disabled by clearing the \l QDrmContent::Activate and
    \l QDrmContent::Reactivate license options respectively.

    \ingroup drm
*/

/*!
    \enum QDrmContent::RenderState
    Represents the current state of a render or usage of content.

    \value Started Content is currently being rendered.  The time in which the content is in this state
    counts towards any timed usage allowance.
    \value Paused Rendering of the content has started but been interrupted.  Continuing the render
    will not count as an additional use and time does not count towards timed usage allowance.
    \value Stopped The content is not currently being rendered.  Starting a render from this state
    is considered a use.
*/

/*!
    \enum QDrmContent::LicenseOption
    Specifies options for content licenses.

    \value NoLicenseOptions Do not display any notifications and set the inital render state to \c Stopped.
    \value Activate Attempt to activate content if it has no valid rights when requesting a license.
    \value Reactivate When a license is no longer valid or will soon be invalidated when released, prompt the user to reactivate the license.
    \value Handover Set the inital render state to \a Paused. This allows the updating of rights for a single use to be spread accross multiple licenses.
    \value Default Activate content with invalid rights when requesting or releasing a content license.
*/

/*!
    \typedef QDrmContent::LicenseOptions

    Synonym for QFlags< QDrmContent::LicenseOption >
*/

/*!
    \fn QDrmContent::renderStateChanged( const QDrmContent &content )

    Signals that the render state of \a content has changed.
*/

/*!
    \fn QDrmContent::rightsExpired( const QDrmContent &content )

    Signals that the rights to \a content have expired.
 */

/*!
    \fn QDrmContent::licenseGranted( const QContent &content )

    Signals that a call to \l {QDrmContent::requestLicense()} to request a license to render \a content  succeeded.

    \sa requestLicense(), licenseDenied()
*/

/*!
    \fn QDrmContent::licenseDenied( const QContent &content )

    Signals that a call to \l {QDrmContent::requestLicense()} to request a license to render \a content  failed.

    \sa requestLicense(), licenseGranted()
*/

/*!
    Creates a new QDrmContent object for accessing content using the given \a permission and license \a options with the parent \a parent.
*/
QDrmContent::QDrmContent( QDrmRights::Permission permission, LicenseOptions options, QObject *parent )
    : QObject( parent )
{
    init( permission, options );
}

/*!
    Destroys a QDrmContent object.
*/
QDrmContent::~QDrmContent()
{
    releaseLicense();

    delete d;
}

void QDrmContent::init( QDrmRights::Permission permission, LicenseOptions options )
{
    d = new QDrmContentPrivate;

    d->permission  = permission;
    d->options = options;
    d->c = this;
    d->focus = 0;

    connect( d   , SIGNAL(rightsExpired(QDrmContent)),
             this, SIGNAL(rightsExpired(QDrmContent)) );
}

/*!
    Returns the current render state.
*/
QDrmContent::RenderState QDrmContent::renderState() const
{
    return d->license ? d->license->renderState() : Stopped;
}

/*!
    Returns the default permission type or the permission type of the last requested license.
*/
QDrmRights::Permission QDrmContent::permission() const
{
    return d->permission;
}

/*!
    Sets the default render \a permission.

    This will only succeed if no license is currently held.
*/
void QDrmContent::setPermission( QDrmRights::Permission permission )
{
    if( !d->license )
        d->permission = permission;
}

/*!
    Returns the license options indicating how the user should be prompted if the content has
    invalid rights and the type of license that should be requested.

    \sa enableLicenseOptions(), disableLicenseOptions()
*/
QDrmContent::LicenseOptions QDrmContent::licenseOptions() const
{
    return d->options;
}

/*!
    Sets the license \a options indicating how the user should be prompted if the content has
    invalid rights and the type of license that should be requested.

    \sa enableLicenseOptions(), disableLicenseOptions()
 */
void QDrmContent::setLicenseOptions( LicenseOptions options )
{
    d->options = options;
}

/*!
    Enables the given license \a options.

    \sa licenseOptions(), setLicenseOptions(), disableLicenseOptions()
*/
void QDrmContent::enableLicenseOptions( LicenseOptions options )
{
    d->options |= options;
}

/*!
    Disables the given license \a options.

    \sa licenseOptions(), setLicenseOptions(), enableLicenseOptions()
 */
void QDrmContent::disableLicenseOptions( LicenseOptions options )
{
    d->options &= ~options;
}

/*!
    Sets a \a widget that will parent any dialogs displayed when activating or reactivating content when
    requesting and releasing licenses.
*/
void QDrmContent::setFocusWidget( QWidget *widget )
{
    d->focus = widget;
}

/*!
    Returns the widget that parents any dialogs displayed when activating or reactivating content when
    requesting and releasing licenses.
 */
QWidget *QDrmContent::focusWidget() const
{
    return d->focus;
}

/*!
    Returns the currently licensed QContent.
*/
QContent QDrmContent::content() const
{
    return d->content;
}

/*!
    Requests a license to render DRM \a content using the currently assigned permission type.

    If there are no current permissions to render the content the user will be prompted to acquire new permissions
    or informed that the content can not be rendered.  If the permissions are valid or the user succeeds in acquiring
    new permissions the licenseGranted() signal will be emitted and true returned.  If the permissions are invalid and
    user cannnot acquire new permissions, or must go through an external agent such as a web browser to acquire new
    permissions the licenseDenied() signal will be emitted and false returned.

    Activatation can be turned off by clearing the Activate license option using disableLicenceOptions().

    \sa licenseGranted(), licenseDenied(), permission()
 */
bool QDrmContent::requestLicense( const QContent &content )
{
    qLog(DRMAgent) << "QDrmContent::requestLicense()" << content;

    if( content.id() == QContent::InvalidId || content.drmState() == QContent::Unprotected ||
        ( content.id() == d->content.id() && d->license ) )
    {
        d->content = content;

        emit licenseGranted( content );

        return true;
    }

    if( !(d->options & Activate) )
        return getLicense( content, d->permission );

    QContent c = content;

    if( c.d->activate( d->permission, d->focus ) )
    {
        return getLicense( c, d->permission );
    }
    else
    {
        emit licenseDenied( c );

        return false;
    }
}

/*!
    Requests a content license for the given \a content.  This should only be called after the validity of the content's
    rights has been verified or the content has been successfully activated.  If no license can be obtained this will return
    false and emit the \c{QDrmContent::licenseDenied(const QContent&)} signal otherwise this will return true and emit the
    \c{QDrmContent::licenseGranted(const QContent&)} signal.
*/
bool QDrmContent::getLicense( const QContent &content, QDrmRights::Permission permission )
{
    QContent c = content;

    QDrmContentLicense *license = c.d->requestLicense( permission, d->options );

    if( !license )
    {
        emit licenseDenied( c );

        return false;
    }

    releaseLicense();

    d->content = c;

    d->license = license;

    connect( d->license, SIGNAL(rightsExpired(QContent,QDrmRights::Permission)),
             d         , SLOT  (rightsExpired(QContent,QDrmRights::Permission)) );

    qLog(DRMAgent) << "License Granted";

    emit licenseGranted( d->content );

    return true;
}

/*!
    Releases the currently held content license. No further file access will be possible
    after this unless a new license is requested.

    If the permissions have expired or are about to expire the user may be prompted to acquire new permissions, this
    prompt can be disabled by clearing the Reactivate license option using disableLicenseOptions().

    Requesting a new license will automatically release a previously held one.

    \sa requestLicense()
*/
void QDrmContent::releaseLicense()
{
    if( d->license )
    {
        QDrmContentLicense *license = d->license;

        d->license = 0;

        if( license->renderState() != Stopped )
            license->renderStateChanged( d->content, Stopped );

        if( d->options & Reactivate )
            d->content.d->reactivate( license->permission(), d->focus );

        license->deleteLater();
    }
}

/*!
    Notifies the DRM agent that rendering of the content has started.

    \sa renderStopped(), renderPaused()
*/
void QDrmContent::renderStarted()
{
    if( d->license && d->license->renderState() != Started )
    {
        d->license->renderStateChanged( d->content, Started );

        emit renderStateChanged( *this );
    }
}

/*!
    Notifies the DRM agent that rendering of the content has ended.

    \sa renderStarted(), renderPaused()
*/
void QDrmContent::renderStopped()
{
    if( d->license && d->license->renderState() != Stopped )
    {
        d->license->renderStateChanged( d->content, Stopped );

        emit renderStateChanged( *this );
    }
}

/*!
    Notifies the DRM agent that rendering of the content has been paused.

    \sa renderStarted(), renderStopped()
 */
void QDrmContent::renderPaused()
{
    if( d->license && d->license->renderState() == Started )
    {
        d->license->renderStateChanged( d->content, Paused );

        emit renderStateChanged( *this );
    }
}

/*!
    Initiates an attempt to retrieve rights for the given \a content.

    Returns true if the content has a valid option for activating the content.

    Content activation is typically performed asynchronously (i.e. opening a web page where the user may purchase rights.)
    so rights may not be available until some time after this method is called.

    Any dialogs displayed will be parented off the given \a focus widget.

    \sa canActivate()
*/
bool QDrmContent::activate( const QContent &content, QWidget *focus )
{
    QContent c = content;

    c.d->activate( QDrmRights::Unrestricted, focus );

    return true;
}

/*!
    Returns true if options exist to activate the given \a content; otherwise returns false.

    \sa activate()
*/
bool QDrmContent::canActivate( const QContent &content )
{
    return QMimeType::fromId( content.type() ).application().isValid()
        ? content.d->canActivate()
        : false;
}

/*!
    Returns a list of all supported DRM types. Applications should use this list when they need to advertise
    support for DRM types, for example HTTP accept headers.
*/
QStringList QDrmContent::supportedTypes()
{
    QStringList types;

    foreach( QDrmContentPlugin *plugin, DrmContentPluginManager::instance()->plugins() )
        types << plugin->types();

    return types;
}

/*!
    Returns a list of HTTP header name/value pairs web applications should use in requests to identify installed DRM agents
*/
QList< QPair< QString, QString > > QDrmContent::httpHeaders()
{
    QList< QPair< QString, QString > > headers;

    foreach( QDrmContentPlugin *plugin, DrmContentPluginManager::instance()->plugins() )
        headers += plugin->httpHeaders();

    return headers;
}

#include "qdrmcontent.moc"
