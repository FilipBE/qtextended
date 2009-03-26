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

#include "voipmanager.h"
#include <qvaluespace.h>

#include <QTimer>
#include <QContact>
#include <QCollectivePresence>
#include <QCollectivePresenceInfo>

class VoIPManagerPrivate {
public:
    VoIPManagerPrivate()
    : netReg(0),
    presenceProvider(0),
    serviceManager(0),
    status(0),
    voipHideMsgTimer(0),
    voipHideMsg(false)
    {}

    QNetworkRegistration *netReg;
    QCollectivePresence *presenceProvider;
    QCommServiceManager *serviceManager;
    QValueSpaceObject *status;
    QTimer *voipHideMsgTimer;
    bool voipHideMsg;
    QString preDndStatus;
    QString dndType;
    QString preAwayStatus;
    QString awayType;
};

/*!
    \class VoIPManager
    \inpublicgroup QtIPCommsModule
    \ingroup QtopiaServer::Telephony
    \brief The VoIPManager class maintains information about the active VoIP telephony service.

    This class provides access to some of the common facilities of the \c{voip} telephony
    service, if it is running within the system, to ease the implementation of VoIP-specific
    user interfaces in the server.  The telephony service itself is started by PhoneServer
    at system start up.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
    \sa PhoneServer
*/

/*!
    \reimp
*/
QString VoIPManager::callType() const
{
    return "VoIP";      // No tr
}

/*!
    \reimp
*/
QString VoIPManager::trCallType() const
{
    return tr("VoIP");
}

/*!
    \reimp
*/
QString VoIPManager::callTypeIcon() const
{
    return "sipsettings/SIP";   // No tr
}

/*!
    \reimp
*/
QAbstractCallPolicyManager::CallHandling VoIPManager::handling
        (const QString& number)
{
    // If no network registration, then cannot use this to dial.
    if ( registrationState() != QTelephony::RegistrationHome )
        return CannotHandle;

    // If at least one '@' sign, then assume that it is a VoIP URI.
    if ( number.contains(QChar('@')) )
        return CanHandle;

    // Probably an ordinary number, which may be gatewayable via VoIP.
    // TODO: may want to user to be able to choose to turn this on/off.
    return CanHandle;
}

/*!
    \reimp
*/
QString VoIPManager::registrationMessage() const
{
    // Don't display anything if we do not have a VoIP service active yet.
    if ( !d->netReg )
        return QString();

    QString voipMsg;
    QTelephony::RegistrationState v = registrationState();

    switch (v) {
    case QTelephony::RegistrationNone:
        if(!d->voipHideMsg)
            voipMsg = tr("No VoIP network");
        break;
    case QTelephony::RegistrationHome:
    case QTelephony::RegistrationUnknown:
    case QTelephony::RegistrationRoaming:
        break;
    case QTelephony::RegistrationSearching:
        voipMsg = tr("Searching VoIP network");
        break;
    case QTelephony::RegistrationDenied:
        voipMsg += tr("VoIP Authentication Failed");
        break;
    }

    // If no registration, we want to hide the message after
    // some time, because the VoIP service may not be configured.
    if(v == QTelephony::RegistrationNone) {
        if(!d->voipHideMsg && !d->voipHideMsgTimer->isActive())
            d->voipHideMsgTimer->start(7000);
    } else {
        d->voipHideMsgTimer->stop();
        d->voipHideMsg = false;
    }

    return voipMsg;
}

/*!
    \reimp
*/
QString VoIPManager::registrationIcon() const
{
    // No specific icon used for VoIP registration messages.
    return QString();
}

/*!
    \reimp
*/
QTelephony::RegistrationState VoIPManager::registrationState() const
{
    if ( d->netReg )
        return d->netReg->registrationState();
    else
        return QTelephony::RegistrationNone;
}

/*!
    Returns true if the VoIP user associated with \a uri is available for calling;
    false otherwise.
*/
bool VoIPManager::isAvailable( const QString &uri )
{
    Q_UNUSED(uri)
    // Presence should never actually bar you from calling someone
    return true;
}

/*!
    \reimp
*/
bool VoIPManager::supportsPresence() const
{
    return true;
}
/*!
    \reimp
    Toggles the status between Do not disturb and normal status.
    Returns true if Do Not Disturb state is activated, otherwise returns false.
*/
bool VoIPManager::doDnd()
{
    // Must be registered
    if ( !d->netReg || d->netReg->registrationState() != QTelephony::RegistrationHome )
        return false;

    if ( !d->presenceProvider )
        return false;

    QMap<QString, QCollectivePresenceInfo::PresenceType> types = d->presenceProvider->statusTypes();
    QCollectivePresenceInfo info = d->presenceProvider->localInfo();
    if (!d->preDndStatus.isEmpty()) {
        info.setPresence(d->preDndStatus, types.value(d->preDndStatus));
        d->presenceProvider->setLocalPresence(info);
        d->preDndStatus.clear();
        return false;
    }

    d->preDndStatus = info.presence();
    info.setPresence(d->dndType, types.value(d->dndType));
    d->presenceProvider->setLocalPresence(info);

    return true;
}

/*!
    If \a isOnThePhone is true, updates the local presence to away.
    Otherwise updates the local presence to online.
*/
void VoIPManager::updateOnThePhonePresence( bool isOnThePhone )
{
    // Must be registered
    if ( !d->netReg || d->netReg->registrationState() != QTelephony::RegistrationHome )
        return;

    if ( !d->presenceProvider || isOnDnd() )
        return;

    QMap<QString, QCollectivePresenceInfo::PresenceType> types = d->presenceProvider->statusTypes();
    QCollectivePresenceInfo info = d->presenceProvider->localInfo();
    if (isOnThePhone) {
        d->preAwayStatus = info.presence();
        info.setPresence(d->awayType, types.value(d->awayType));
    } else {
        info.setPresence(d->preAwayStatus, types.value(d->preAwayStatus));
    }

    d->presenceProvider->setLocalPresence(info);
}

/*!
    Returns true if the presence of local account is on Do Not Disturb status, otherwise returns false.
*/
bool VoIPManager::isOnDnd()
{
    QCollectivePresenceInfo info = d->presenceProvider->localInfo();

    if (info.presence() == d->dndType)
        return true;

    return false;
}

/*!
    Create a new VoIP telephony service manager and attach it to \a parent.
*/
VoIPManager::VoIPManager(QObject *parent)
    : QAbstractCallPolicyManager(parent)
{
    d = new VoIPManagerPrivate;
    d->status = new QValueSpaceObject("/Telephony/Status/VoIP", this);
    d->voipHideMsgTimer = new QTimer( this );
    d->voipHideMsgTimer->setSingleShot( true );
    connect( d->voipHideMsgTimer, SIGNAL(timeout()),
             this, SLOT(hideMessageTimeout()) );

    // The "voip" telephony handler may not have started yet.
    // Hook onto QCommServiceManager to watch for it.
    d->serviceManager = new QCommServiceManager( this );
    connect( d->serviceManager, SIGNAL(servicesChanged()),
             this, SLOT(servicesChanged()) );

    // Just in case it has already started.
    servicesChanged();
}

void VoIPManager::registrationStateChanged()
{
    if ( d->netReg ) {
        emit registrationChanged( d->netReg->registrationState() );
        if ( d->netReg->registrationState() == QTelephony::RegistrationHome ) {
            d->status->setAttribute("Registered", true);
        } else if ( d->netReg->registrationState() == QTelephony::RegistrationNone ) {
            d->status->setAttribute("Registered", false);
        }
    }
}

void VoIPManager::localPresenceChanged()
{
    if ( d->presenceProvider ) {
        QCollectivePresenceInfo info = d->presenceProvider->localInfo();
        QMap<QString, QCollectivePresenceInfo::PresenceType> types = d->presenceProvider->statusTypes();
        QCollectivePresenceInfo::PresenceType type = types[info.presence()];
        d->status->setAttribute("Presence/Local/Presence", static_cast<int>(type));
        d->status->setAttribute("Presence/Local/DisplayName", info.displayName());
        d->status->setAttribute("Presence/Local/Uri", info.uri());
    } else {
        d->status->setAttribute("Presence/Local/Presence", static_cast<int>(QCollectivePresenceInfo::None));
        d->status->setAttribute("Presence/Local/DisplayName", QString());
        d->status->setAttribute("Presence/Local/Uri", QString());
    }
}

void VoIPManager::servicesChanged()
{
    QStringList interfaces = d->serviceManager->interfaces("voip");

    if (!d->presenceProvider && interfaces.contains("QCollectivePresence"))
        QTimer::singleShot(0, this, SLOT(presenceProviderAvailable()));

    if (!d->netReg) {
        if (interfaces.contains("QNetworkRegistration"))
            serviceStarted();
    } else {
        if (!interfaces.contains("QNetworkRegistration"))
            serviceStopped();
    }
}

void VoIPManager::serviceStarted()
{
    // The "voip" handler has started up, so attach to the service.
    d->netReg = new QNetworkRegistration( "voip", this ); // No tr
    if ( !d->netReg->available() ) {
        delete d->netReg;
        d->netReg = 0;
    } else {
        connect( d->netReg, SIGNAL(registrationStateChanged()),
                 this, SLOT(registrationStateChanged()) );
        registrationStateChanged();
    }
}

void VoIPManager::presenceProviderAvailable()
{
    d->presenceProvider = new QCollectivePresence( "voip", this );
    if ( !d->presenceProvider->available() ) {
        delete d->presenceProvider;
        d->presenceProvider = 0;
        return;
    }

    connect(d->presenceProvider, SIGNAL(localPresenceChanged()),
            this, SLOT(localPresenceChanged()));
    connect(d->presenceProvider, SIGNAL(disconnected()),
            this, SLOT(presenceProviderDisconnected()));

    QMap<QString, QCollectivePresenceInfo::PresenceType> statusTypes = d->presenceProvider->statusTypes();
    QMap<QString, QCollectivePresenceInfo::PresenceType>::const_iterator it = statusTypes.constBegin();

    // Assume that at least away should be present, but use busy/extended
    // away when we can
    while (it != statusTypes.constEnd()) {
        if (((it.value() == QCollectivePresenceInfo::Busy) || (it.value() == QCollectivePresenceInfo::ExtendedAway))
              && d->dndType.isEmpty()) {
            d->dndType = it.key();
        }

        if (it.value() == QCollectivePresenceInfo::Away || it.value() == QCollectivePresenceInfo::ExtendedAway) {
            if (d->awayType.isEmpty())
                d->awayType = it.key();
        }

        ++it;
    }

    if (d->dndType.isEmpty())
        d->dndType = d->awayType;
}

void VoIPManager::presenceProviderDisconnected()
{
    delete d->presenceProvider;
    d->presenceProvider = 0;

    d->dndType.clear();
    d->awayType.clear();

    d->preAwayStatus.clear();
    d->preDndStatus.clear();

    localPresenceChanged();
}

void VoIPManager::serviceStopped()
{
    // The "voip" handler has shut down, so detach from the service.
    delete d->netReg;
    d->netReg = 0;
    registrationStateChanged();
}

void VoIPManager::hideMessageTimeout()
{
    // Mark the message as hidden and then fake a registration state change.
    d->voipHideMsg = true;
    emit registrationChanged( registrationState() );
}

/*!
    \reimp
*/
void VoIPManager::setCellLocation(const QString &)
{
    // Nothing to do here.
}

QTOPIA_TASK(VoIP, VoIPManager);
QTOPIA_TASK_PROVIDES(VoIP, QAbstractCallPolicyManager);
