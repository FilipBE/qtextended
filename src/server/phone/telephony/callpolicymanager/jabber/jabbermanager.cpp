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

#include "jabbermanager.h"
#include <qvaluespace.h>

#include <QTimer>
#include <QContact>
#include <QCollectivePresence>
#include <QCollectivePresenceInfo>

class JabberManagerPrivate {
public:
    JabberManagerPrivate()
    : netReg(0),
    presenceProvider(0),
    serviceManager(0),
    status(0),
    jabberHideMsgTimer(0),
    jabberHideMsg(false)
    {}

    QNetworkRegistration *netReg;
    QCollectivePresence *presenceProvider;
    QCommServiceManager *serviceManager;
    QValueSpaceObject *status;
    QTimer *jabberHideMsgTimer;
    bool jabberHideMsg;
    QString preDndStatus;
    QString dndType;
    QString preAwayStatus;
    QString awayType;
};

/*!
    \class JabberManager
    \inpublicgroup QtIPCommsModule
    \ingroup QtopiaServer::Telephony
    \brief The JabberManager class maintains information about the active Jabber telephony service.

    This class provides access to some of the common facilities of the \c{jabber} telephony
    service, if it is running within the system, to ease the implementation of Jabber-specific
    user interfaces in the server.  The telephony service itself is started by PhoneServer
    at system start up.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
    \sa PhoneServer
*/

/*!
    \reimp
*/
QString JabberManager::callType() const
{
    return "Jabber";      // No tr
}

/*!
    \reimp
*/
QString JabberManager::trCallType() const
{
    return tr("Jabber");
}

/*!
    \reimp
*/
QString JabberManager::callTypeIcon() const
{
    return "gtalksettings/gtalk";   // No tr
}

/*!
    \reimp
*/
QAbstractCallPolicyManager::CallHandling JabberManager::handling
        (const QString& number)
{
    // If no network registration, then cannot use this to dial.
    if ( registrationState() != QTelephony::RegistrationHome )
        return CannotHandle;

    // If at least one '@' sign, then assume that it is a Jabber URI.
    //XXX Enable when Jabber voice calls are supported
#ifdef ENABLE_WHEN_JABBER_VOICE_CALLS_SUPPORTED
    if ( number.contains(QChar('@')) )
        return CanHandle;
#else
    Q_UNUSED(number);
#endif

    return CannotHandle;
}

/*!
    \reimp
*/
QString JabberManager::registrationMessage() const
{
    // Don't display anything if we do not have a Jabber service active yet.
    if ( !d->netReg )
        return QString();

    QString jabberMsg;
    QTelephony::RegistrationState v = registrationState();

    switch (v) {
    case QTelephony::RegistrationNone:
        if(!d->jabberHideMsg)
            jabberMsg = tr("No Jabber network");
        break;
    case QTelephony::RegistrationHome:
    case QTelephony::RegistrationUnknown:
    case QTelephony::RegistrationRoaming:
        break;
    case QTelephony::RegistrationSearching:
        jabberMsg = tr("Searching Jabber network");
        break;
    case QTelephony::RegistrationDenied:
        jabberMsg += tr("Jabber Authentication Failed");
        break;
    }

    // If no registration, we want to hide the message after
    // some time, because the Jabber service may not be configured.
    if(v == QTelephony::RegistrationNone) {
        if(!d->jabberHideMsg && !d->jabberHideMsgTimer->isActive())
            d->jabberHideMsgTimer->start(7000);
    } else {
        d->jabberHideMsgTimer->stop();
        d->jabberHideMsg = false;
    }

    return jabberMsg;
}

/*!
    \reimp
*/
QString JabberManager::registrationIcon() const
{
    // No specific icon used for Jabber registration messages.
    return QString();
}

/*!
    \reimp
*/
QTelephony::RegistrationState JabberManager::registrationState() const
{
    if ( d->netReg )
        return d->netReg->registrationState();
    else
        return QTelephony::RegistrationNone;
}

/*!
    Returns true if the Jabber user associated with \a uri is available for calling;
    false otherwise.
*/
bool JabberManager::isAvailable( const QString &uri )
{
    Q_UNUSED(uri)
    // Presence should never actually bar you from calling someone
    return true;
}

/*!
    \reimp
*/
bool JabberManager::supportsPresence() const
{
    return true;
}
/*!
    \reimp
    Toggles the status between Do not disturb and normal status.
    Returns true if Do Not Disturb state is activated, otherwise returns false.
*/
bool JabberManager::doDnd()
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
void JabberManager::updateOnThePhonePresence( bool isOnThePhone )
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
bool JabberManager::isOnDnd()
{
    QCollectivePresenceInfo info = d->presenceProvider->localInfo();

    if (info.presence() == d->dndType)
        return true;

    return false;
}

/*!
    Create a new Jabber telephony service manager and attach it to \a parent.
*/
JabberManager::JabberManager(QObject *parent)
    : QAbstractCallPolicyManager(parent)
{
    d = new JabberManagerPrivate;
    d->status = new QValueSpaceObject("/Telephony/Status/Jabber", this);
    d->jabberHideMsgTimer = new QTimer( this );
    d->jabberHideMsgTimer->setSingleShot( true );
    connect( d->jabberHideMsgTimer, SIGNAL(timeout()),
             this, SLOT(hideMessageTimeout()) );

    // The "jabber" telephony handler may not have started yet.
    // Hook onto QCommServiceManager to watch for it.
    d->serviceManager = new QCommServiceManager( this );
    connect( d->serviceManager, SIGNAL(servicesChanged()),
             this, SLOT(servicesChanged()) );

    // Just in case it has already started.
    servicesChanged();
}

void JabberManager::registrationStateChanged()
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

void JabberManager::localPresenceChanged()
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

void JabberManager::servicesChanged()
{
    QStringList interfaces = d->serviceManager->interfaces("jabber");

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

void JabberManager::serviceStarted()
{
    // The "jabber" handler has started up, so attach to the service.
    d->netReg = new QNetworkRegistration( "jabber", this ); // No tr
    if ( !d->netReg->available() ) {
        delete d->netReg;
        d->netReg = 0;
    } else {
        connect( d->netReg, SIGNAL(registrationStateChanged()),
                 this, SLOT(registrationStateChanged()) );
        registrationStateChanged();
    }
}

void JabberManager::presenceProviderAvailable()
{
    d->presenceProvider = new QCollectivePresence( "jabber", this );
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

void JabberManager::presenceProviderDisconnected()
{
    delete d->presenceProvider;
    d->presenceProvider = 0;

    d->dndType.clear();
    d->awayType.clear();

    d->preAwayStatus.clear();
    d->preDndStatus.clear();

    localPresenceChanged();
}

void JabberManager::serviceStopped()
{
    // The "jabber" handler has shut down, so detach from the service.
    delete d->netReg;
    d->netReg = 0;
    registrationStateChanged();
}

void JabberManager::hideMessageTimeout()
{
    // Mark the message as hidden and then fake a registration state change.
    d->jabberHideMsg = true;
    emit registrationChanged( registrationState() );
}

/*!
    \reimp
*/
void JabberManager::setCellLocation(const QString &)
{
    // Nothing to do here.
}

QTOPIA_TASK(Jabber, JabberManager);
QTOPIA_TASK_PROVIDES(Jabber, QAbstractCallPolicyManager);
