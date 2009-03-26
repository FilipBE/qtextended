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

#include "qmodempppdmanager_p.h"
#include <qserialiodevicemultiplexer.h>
#include <qserialiodevice.h>
#include <qatchatscript.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qtopiachannel.h>
#include <qtopiaipcenvelope.h>
#include <qtimer.h>

/*
    The QModemPPPdManager class provides helper methods for starting pppd on
    data channels.  This is intended to be used by telephony service
    implementations.  Client applications should use
    QPhoneCallManager::create() with a call type of \c IP, and then
    call QPhoneCall::dial() to set up a pppd session.
*/

/*
    Create a new pppd manager for the data channel in \a mux and
    attach it to \a parent.
*/
QModemPPPdManager::QModemPPPdManager( QSerialIODeviceMultiplexer *mux, QObject *parent )
    : QObject( parent )
{
    this->mux = mux;
    this->process = 0;
    this->active = false;
    this->terminated = false;

    // Create the QCop channel that we use to listen for pppd messages.
    pppdChannel = new QtopiaChannel( "QPE/pppd", this );
    QObject::connect
        ( pppdChannel, SIGNAL(received(QString,QByteArray)),
          this, SLOT(pppdListen(QString,QByteArray)) );
}

/*
    Destroy this pppd manager.
*/
QModemPPPdManager::~QModemPPPdManager()
{
    // Kill any pppd processes that are left over.
    if ( process ) {
        process->terminate();
        process->deleteLater();
    }
}

/*
    Determine if there is a session already active in this pppd manager.
*/
bool QModemPPPdManager::isActive() const
{
    return active;
}

/*
    Determine if data calls are currently possible using this pppd manager
    because the data channel is available and not open.
*/
bool QModemPPPdManager::dataCallsPossible() const
{
    QSerialIODevice *channel = mux->channel( "data" );
    return ( channel != 0 && !channel->isOpen() );
}

/*
    Start pppd on this manager using the supplied dial \a options.
*/
bool QModemPPPdManager::start( const QDialOptions& options )
{
    QStringList args;
    QString connectScript;
    QString disconnectScript;

    // Bail out if there is already a pppd process running.
    if ( process ) {
        emit dataStateUpdate( QPhoneCall::PPPdFailed );
        return false;
    }

    // Save the options for later.
    this->options = options;

    // Determine if data setup happens on the command channel.
    bool cmdSetup = ( mux->channel( "datasetup" ) != mux->channel( "data" ) );

    // Build the full list of command-line arguments.
    args << options.ipProgramName();
    args << options.ipArgs();
    if ( cmdSetup ) {
        // Tell pppd to call our script for connect and disconnect.
        // This script talks to us via QCop to do the dial/hangup.
        args << "connect"; // No tr
        args << Qtopia::qtopiaDir() +
                "bin/qtopia-pppd-internal dial"; // No tr
        args << "disconnect"; // No tr
        args << Qtopia::qtopiaDir() +
                "bin/qtopia-pppd-internal stop"; // No tr
    } else {
        // We mostly let pppd do the work of connecting and disconnecting,
        // but we do want to know when it happens so we can track when
        // the call is active in demand-dialing mode.
        connectScript = options.ipConnectScript();
        disconnectScript = options.ipDisconnectScript();
        args << "connect";
        if ( !connectScript.isEmpty() ) {
            args << Qtopia::qtopiaDir() +
                    "bin/qtopia-pppd-internal active " +   // No tr
                    connectScript;
        } else {
            args << Qtopia::qtopiaDir() +
                    "bin/qtopia-pppd-internal active";      // No tr
        }
        args << "disconnect";
        if ( !disconnectScript.isEmpty() ) {
            args << Qtopia::qtopiaDir() +
                    "bin/qtopia-pppd-internal inactive " + // No tr
                    disconnectScript;
        } else {
            args << Qtopia::qtopiaDir() +
                    "bin/qtopia-pppd-internal inactive";   // No tr
        }
    }

    // Log the full command-line that we are about to execute.
    qLog(Network) << "starting pppd"
                  << ( options.ipDemandDialing() ? "(demand)" : "(non-demand)" )
                  << ":" << args.join(" ");

    // Run pppd on the data channel and make it insert the device name
    // at the start of the command-line arguments.
    terminated = false;
    process = mux->channel( "data" )->run( args, true );
    if ( !process ) {
        emit dataStateUpdate( QPhoneCall::PPPdFailed );
        return false;
    }
    connect( process, SIGNAL(stateChanged(QProcess::ProcessState)),
             this, SLOT(pppdStateChanged(QProcess::ProcessState)) );
    changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataInactive );
    return true;
}

/*
    Stop the current pppd session.
*/
void QModemPPPdManager::stop()
{
    if ( process ) {
        // Send SIGTERM to the protocol module to request that it shut down.
        qLog(Network) << "Stopping pppd";
        terminated = true;
        process->terminate();
    }
}

void QModemPPPdManager::dataCallStop()
{
    QSerialIODevice *data = mux->channel( "data" );
    if ( data ) {
        // Drop the DTR signal on the channel, which should cause
        // the GPRS session to hangup and return to command mode.
        data->setDtr( false );

        // Start a timer to raise DTR again after half a second.
        QTimer::singleShot( 500, this, SLOT(raiseDtrAndHangup()) );
    }
}

void QModemPPPdManager::raiseDtr()
{
    QSerialIODevice *data = mux->channel( "data" );
    if ( data )
        data->setDtr( true );
}

void QModemPPPdManager::runChatFile( const QString& filename, const char *slot )
{
    QAtChatScript *script = new QAtChatScript
        ( mux->channel( "datasetup" )->atchat(), this );
    connect( script, SIGNAL(done(bool,QAtResult)), this, slot );
    connect( script, SIGNAL(done(bool,QAtResult)),
             script, SLOT(deleteLater()) );
    script->runChatFile( filename );
}

void QModemPPPdManager::changeState( int value )
{
    // Handle race conditions where a state change due to QCop messages
    // happens after pppd has already terminated.
    if ( process )
        emit dataStateUpdate( (QPhoneCall::DataState)value );
}

void QModemPPPdManager::raiseDtrAndHangup()
{
    // Raise the DTR signal.
    raiseDtr();

    // Run the disconnect script if one is available.
    QString disconnectScript = options.ipDisconnectScript();
    if ( !disconnectScript.isEmpty() ) {
        runChatFile( disconnectScript, SLOT(disconnected()) );
    } else {
        disconnected();
    }
}

void QModemPPPdManager::pppdListen( const QString& msg, const QByteArray& )
{
    qLog(Network) << "QModemPPPdManager::pppdListen(" << msg << ")";
    if ( msg == "dial()" ) {

        // The qtopia-pppd-internal script is telling us that we
        // need to initiate a dialing sequence.  Used when data
        // setup is on the command channel.

        // Bail out if we already have a data call proceeding.
        if ( active ) {
            QtopiaIpcEnvelope env( "QPE/pppd", "dialResult(bool)" );
            env << (int)1;
            return;
        }

        // Make sure that DTR is raised on the data channel, just in
        // case we just did a hangup and DTR isn't back up again.
        raiseDtr();

        // Notify higher layers of the change in state.
        changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataInactive |
                     QPhoneCall::Connecting );

        // Run the connect chat script.
        runChatFile( options.ipConnectScript(), SLOT(connected(bool)) );

    } else if ( msg == "stop()" ) {

        // The qtopia-pppd-internal script is telling us that we
        // need to initiate a disconnect sequence.  Used when data
        // setup is on the command channel.
        dataCallStop();

    } else if ( msg == "active()" ) {

        // The qtopia-pppd-internal script is telling us that a
        // new data call is active.  Used when data setup is on
        // the data channel.
        active = true;
        emit dataCallActive();
        changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataActive );

    } else if ( msg == "inactive()" ) {

        // The qtopia-stop-internal script is telling us that a
        // data call is no longer active.  Used when data setup
        // is on the data channel.
        active = false;
        emit dataCallInactive();
        if ( options.ipDemandDialing() ) {
            changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataInactive );
        }

    } else if ( msg == "connecting()" ) {

        // The qtopia-pppd-internal script is telling us that the
        // chat script is starting when data setup on data channel.

        changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataInactive |
                     QPhoneCall::Connecting );

    } else if ( msg == "connectFailed()" ) {

        // The qtopia-pppd-internal script is telling us that the
        // chat script failed when data setup is on the data channel.
        changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataInactive |
                     QPhoneCall::ConnectFailed );

    }
}

void QModemPPPdManager::connected( bool ok )
{
    if ( !process ) { //no reason to go on if pppd stopped already
        active = false; //just to be sure
        emit dataCallInactive();
        return;
    }
    // Tell the "qtopia-pppd-internal" script the result of the dial.
    QtopiaIpcEnvelope env( "QPE/pppd", "dialResult(bool)" );
    env << (int)ok;
    if ( !ok ) {
        changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataInactive |
                     QPhoneCall::ConnectFailed );
        return;
    }

    // Let the rest of the system know that there is a data call active.
    active = true;
    emit dataCallActive();
    changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataActive );
}

void QModemPPPdManager::disconnected()
{
    active = false;
    emit dataCallInactive();
    changeState( QPhoneCall::PPPdStarted | QPhoneCall::DataInactive );
}

void QModemPPPdManager::pppdStateChanged( QProcess::ProcessState state )
{
    if ( process && state == QProcess::NotRunning ) {
        process->deleteLater();
        process = 0;
        if ( active ) {
            active = false;
            emit dataCallInactive();
        }
        if ( terminated )
            emit dataStateUpdate( QPhoneCall::PPPdStopped );
        else
            emit dataStateUpdate( QPhoneCall::PPPdFailed );
    }
}

/*
    void QModemPPPdManager::dataCallActive()

    Signal that is emitted once a pppd session is active on this manager.
*/

/*
    void QModemPPPdManager::dataCallInactive()

    Signal that is emitted once a pppd session becomes inactive.
*/

/*
    void QModemPPPdManager::dataStateUpdate( QPhoneCall::DataState state )

    Signal that is emitted to advertise a change in the pppd \a state.
*/
