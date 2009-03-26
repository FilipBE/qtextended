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

#ifndef CALLMANAGER_H
#define CALLMANAGER_H

#include "phonesim.h"

enum CallState
{
    CallState_Active        = 0,
    CallState_Held          = 1,
    CallState_Dialing       = 2,
    CallState_Alerting      = 3,
    CallState_Incoming      = 4,
    CallState_Waiting       = 5,
    CallState_Hangup        = 6,      // Call about to go away.
    CallState_SwapDummy     = 7       // Used internally in phonesim only.
};

struct CallInfo
{
    int         id;
    CallState   state;
    QString     number;
    bool        incoming;
    bool        dialBack;
};

class QSimControlEvent;

class CallManager : public QObject
{
    Q_OBJECT
public:
    CallManager( QObject *parent = 0 );
    ~CallManager();

    // Process an AT command.  Returns false if not a call-related command.
    bool command( const QString& cmd );

    // Get the active call list.
    QList<CallInfo> calls() const { return callList; }

    // Hangup calls.
    void hangupAll();
    void hangupConnected();
    void hangupHeld();
    void hangupConnectedAndHeld();
    void hangupCall( int id );

    // Accept the incoming call.
    bool acceptCall();

    // Primitive GSM 22.030 call operations with AT+CHLD.
    bool chld0();
    bool chld1();
    bool chld1x( int x );
    bool chld2();
    bool chld2x( int x );
    bool chld3();
    bool chld4();

    // Adjust whether various commands will fail even if they would normally succeed.
    bool holdWillFail() const { return _holdWillFail; }
    void setHoldWillFail( bool value ) { _holdWillFail = value; }
    bool activateWillFail() const { return _activateWillFail; }
    void setActivateWillFail( bool value ) { _activateWillFail = value; }
    bool joinWillFail() const { return _joinWillFail; }
    void setJoinWillFail( bool value ) { _joinWillFail = value; }
    bool deflectWillFail() const { return _deflectWillFail; }
    void setDeflectWillFail( bool value ) { _deflectWillFail = value; }

    // Get or set the maximum number of participants in a multiparty (-1 means unlimited).
    int multipartyLimit() const { return _multipartyLimit; }
    void setMultipartyLimit( int value ) { _multipartyLimit = value; }

public slots:
    // Start an incoming call simulation.
    void startIncomingCall( const QString& number, bool dialBack );
    void startIncomingCall( const QString& number );

signals:
    // Send a response to a command.
    void send( const QString& line );

    // Send an unsolicited notification related to calls.
    void unsolicited( const QString& line );

    // Verify a phone number to see if dialing is allowed.
    // Set "ok" to false if the number should be disallowed.
    void dialCheck( const QString& number, bool& ok );

    // Send a call control event.
    void controlEvent( const QSimControlEvent& event );

private slots:
    // Transition the active dialing or alerting call to connected.
    void dialingToConnected();

    // Transition the active dialing call to alerting.
    void dialingToAlerting();

    // Handle dial-backs.
    void dialBack();
    void dialBackWithHangup5();
    void dialBackWithHangup4();

    // Hangup after a dial-back timeout.
    void hangupTimeout();

    // Send the next RING indication for incoming calls.
    void sendNextRing();

private:
    QList<CallInfo> callList;
    QTimer *connectTimer;
    QTimer *alertingTimer;
    QTimer *hangupTimer;
    QTimer *ringTimer;
    bool _holdWillFail;
    bool _activateWillFail;
    bool _joinWillFail;
    bool _deflectWillFail;
    int _multipartyLimit;
    int numRings;

    int newId();
    int idForDialing();
    int idForIncoming();
    int idForState( CallState state );
    int countForState( CallState state );
    int indexForId( int id );
    bool hasCall( CallState state );
    void changeGroup( CallState oldState, CallState newState );
    void sendState( const CallInfo& info );
};

#endif
