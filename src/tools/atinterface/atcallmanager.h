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

#ifndef ATCALLMANAGER_H
#define ATCALLMANAGER_H

#include <qphonecallmanager.h>
#include <qatresult.h>

class AtCallManagerPrivate;
class AtCommands;

class AtCallManager : public QObject
{
    Q_OBJECT
public:
    AtCallManager( QObject *parent = 0 );
    ~AtCallManager();

    // Broken out call states for reporting to higher layers.
    enum CallState
    {
        CallIdle,
        CallCalling,
        CallConnecting,
        CallActive,
        CallHold,
        CallWaiting,
        CallAlerting,
        CallBusy
    };

    // Result code that indicates that a response will be deferred.
    static const QAtResult::ResultCode Defer = (QAtResult::ResultCode)(-100);

    void setHandler( AtCommands *handler );

    QAtResult::ResultCode dial( const QString& dialString );
    QAtResult::ResultCode accept();
    QAtResult::ResultCode online();
    QAtResult::ResultCode hangup();
    QAtResult::ResultCode hangup( int callID );
    QAtResult::ResultCode hangupIncomingCall();
    QAtResult::ResultCode hangupHeldCalls();
    QAtResult::ResultCode activateHeldCalls();
    QAtResult::ResultCode activate( int callID );
    QAtResult::ResultCode join();
    QAtResult::ResultCode transfer();
    QAtResult::ResultCode transferIncoming( const QString& number );
    QAtResult::ResultCode tone( const QString& value );

    QStringList formatCallList();

    bool ringing() const;
    bool callInProgress() const;

    static int clccCallType( const QString& callType );
    static int numCallType( const QString& callType );
    static QString strCallType( const QString& callType );

    void notifyCallStates();

    enum CallSetup
    {
        NoCallSetup,
        IncomingCallSetup,
        OutgoingCallSetup,
        AlertingCallSetup
    };

    enum CallHoldState
    {
        NoCallsHeld,
        CallsActiveAndHeld,
        CallsHeldOnly
    };

signals:
    void stateChanged( int callID, AtCallManager::CallState state,
                       const QString& number, const QString& type );
    void callStateInitialized();
    void deferredResult( AtCommands *handler, QAtResult::ResultCode result );
    void ring( const QString& number, const QString& type );
    void dialingOut( bool asynchronous, bool transparent, bool GPRS );
    void outgoingConnected( const QString& number );
    void callWaiting( const QString& number, const QString& type );
    void noCarrier();
    void setOnCall( bool value );
    void setCallSetup( AtCallManager::CallSetup callSetup );
    void setCallHold( AtCallManager::CallHoldState callHold );

private slots:
    void newCall( const QPhoneCall &call );
    void callStateChanged( const QPhoneCall& call );
    void repeatRing();
    void callManagerStatesChanged();

private:
    AtCallManagerPrivate *d;
};

#endif
