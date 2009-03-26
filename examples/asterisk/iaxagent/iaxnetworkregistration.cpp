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

#include "iaxnetworkregistration.h"
#include "iaxtelephonyservice.h"
#include "iaxclient.h"
#include <qtopialog.h>
#include <QSettings>
#include <QTimer>

IaxNetworkRegistration::IaxNetworkRegistration( IaxTelephonyService *service )
    : QNetworkRegistrationServer( service->service(), service )
{
    this->service = service;
    pendingSetCurrentOperator = false;
    registrationId = -1;

    // Wait 1 second for the rest of iaxagent to start up and then
    // automatically register if specified by Asterisk.conf.
    // TODO: wait for the Qtopia network to become available.
    QTimer::singleShot( 1000, this, SLOT(autoRegisterToServer()) );
}

IaxNetworkRegistration::~IaxNetworkRegistration()
{
    deregisterFromServer();
}

QString IaxNetworkRegistration::callUri() const
{
    // Get the URI to prepend to a number to make a complete IAX
    // dialing sequence.  i.e. "username:password@host".
    if ( registrationState() == QTelephony::RegistrationHome )
        return callUriValue;
    else
        return QString();
}

void IaxNetworkRegistration::setCurrentOperator
        ( QTelephony::OperatorMode mode, const QString& /*id*/,
          const QString& /*technology*/ )
{
    // The mode is the only parameter that makes sense for IAX.
    // We use it to register to or deregister from the network.
    if ( mode == QTelephony::OperatorModeDeregister ) {
        pendingSetCurrentOperator = true;
        deregisterFromServer();
    } else {
        pendingSetCurrentOperator = true;
        registerToServer();
    }
}

void IaxNetworkRegistration::requestAvailableOperators()
{
    // "Available network operators" doesn't make sense for IAX,
    // so return an empty list.
    QList<QNetworkRegistration::AvailableOperator> list;
    emit availableOperators( list );
}

void IaxNetworkRegistration::registrationEvent( int eventType )
{
    // Process an IAX registration event.
    if ( eventType == IAXC_REGISTRATION_REPLY_ACK ||
         eventType == 65535 ) { // IAX_EVENT_NULL, which means already reg'd.
        // Registration is now valid.
        qLog(VoIP) << "IAX registration set to Home";
        updateRegistrationState( QTelephony::RegistrationHome );
        if ( pendingSetCurrentOperator ) {
            emit setCurrentOperatorResult( QTelephony::OK );
            pendingSetCurrentOperator = false;
        }
    } else if ( eventType == IAXC_REGISTRATION_REPLY_REJ ) {
        // Registration authentication failed.
        qLog(VoIP) << "IAX registration set to Denied";
        updateRegistrationState( QTelephony::RegistrationDenied );
        if ( pendingSetCurrentOperator ) {
            emit setCurrentOperatorResult( QTelephony::NetworkNotAllowed );
            pendingSetCurrentOperator = false;
        }
        if ( registrationId != -1 ) {
            iaxc_unregister( registrationId );
            registrationId = -1;
        }
    } else {
        // Registration has failed for some other reason (timeout probably).
        qLog(VoIP) << "IAX registration set to None, event type" << eventType;
        updateRegistrationState( QTelephony::RegistrationNone );
        if ( pendingSetCurrentOperator ) {
            emit setCurrentOperatorResult( QTelephony::Error );
            pendingSetCurrentOperator = false;
        }
        if ( registrationId != -1 ) {
            iaxc_unregister( registrationId );
            registrationId = -1;
        }
    }
}

void IaxNetworkRegistration::autoRegisterToServer()
{
    QSettings config( "Trolltech", "Asterisk" );
    config.beginGroup( "Registration" );
    if ( config.value( "AutoRegister", false ).toBool() )
        registerToServer();
}

void IaxNetworkRegistration::registerToServer()
{
    qLog(VoIP) << "IaxNetworkRegistration::registerToServer()";
    if ( registrationState() == QTelephony::RegistrationHome ) {
        if ( pendingSetCurrentOperator ) {
            emit setCurrentOperatorResult( QTelephony::OK );
            pendingSetCurrentOperator = false;
        }
    } else if ( registrationId == -1 ) {
        // We are now searching for a server to register to.
        qLog(VoIP) << "IAX registration set to Searching";
        updateRegistrationState( QTelephony::RegistrationSearching );

        // Read the registration settings to use from Asterisk.conf.
        QSettings config( "Trolltech", "Asterisk" );
        config.beginGroup( "Registration" );
        QString userid = config.value( "UserId" ).toString();
        QString password = config.value( "Password" ).toString();
        QString server = config.value( "Server" ).toString();
        int port = config.value( "Port", -1 ).toInt();
        if ( password.startsWith( ":" ) ) {
            // The password has been base64-encoded.
            password = QString::fromUtf8
                ( QByteArray::fromBase64( password.mid(1).toLatin1() ) );
        }
        config.endGroup();      // Registration

        // If no server, then assume that we cannot register at all.
        if ( server.isEmpty() ) {
            qLog(VoIP) << "IAX registration set to None - no server configured";
            updateRegistrationState( QTelephony::RegistrationNone );
            if ( pendingSetCurrentOperator ) {
                emit setCurrentOperatorResult( QTelephony::Error );
                pendingSetCurrentOperator = false;
            }
            return;
        }

        // Record the call URI for later.  TODO: how to encode special chars?
        if ( password.isEmpty() )
            callUriValue = userid + "@" + server;
        else
            callUriValue = userid + ":" + password + "@" + server;

        // Start the registration process.  TODO: async host lookup.
        if ( port != -1 && port != 0 )
            server += ":" + QString::number( port );
        registrationId = iaxc_register( userid.toUtf8().data(),
                                        password.toUtf8().data(),
                                        server.toUtf8().data() );

        // Handle any pending events or timeouts caused by the registration.
        service->serviceIaxClient();
    }
}

void IaxNetworkRegistration::deregisterFromServer()
{
    qLog(VoIP) << "IaxNetworkRegistration::deregisterFromServer()";
    callUriValue = QString();
    if ( registrationId != -1 ) {
        iaxc_unregister( registrationId );
        registrationId = -1;

        // Handle any pending events or timeouts caused by the unregistration.
        service->serviceIaxClient();
    }
    qLog(VoIP) << "IAX registration set to None";
    updateRegistrationState( QTelephony::RegistrationNone );
    if ( pendingSetCurrentOperator ) {
        emit setCurrentOperatorResult( QTelephony::OK );
        pendingSetCurrentOperator = false;
    }
}

void IaxNetworkRegistration::updateRegistrationConfig()
{
    qLog(VoIP) << "IaxNetworkRegistration::updateRegistrationConfig()";

    // If we are currently registered, or registering, then shut the
    // registration down and start it up again.  If we are not registered,
    // then wait for the next setCurrentOperator() call to read the config.
    if ( registrationId != -1 ) {
        deregisterFromServer();
        registerToServer();
    }
}
