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
#ifndef QPHONECALL_H
#define QPHONECALL_H

#include <qtopiaipcmarshal.h>
#include <qdialoptions.h>

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qdatetime.h>

class QPhoneCall;
class QPhoneCallPrivate;
class QIODevice;

class QTOPIAPHONE_EXPORT QPhoneCall
{
    friend class QPhoneCallPrivate;
    friend class QPhoneCallManagerPrivate;
    friend bool operator==( const QPhoneCall& call1,
                            const QPhoneCall& call2 );
    friend bool operator!=( const QPhoneCall& call1,
                            const QPhoneCall& call2 );

private:
    QPhoneCall( QPhoneCallPrivate *priv );

public:
    QPhoneCall();
    QPhoneCall( const QPhoneCall& call );
    ~QPhoneCall();

    // Call states.
    enum State
    {
        Idle,               // New outgoing call, not dialed yet.
        Incoming,           // Incoming connection from remote party.
        Dialing,            // Dialing, but not yet connected.
        Alerting,           // Alerting other party during outgoing dial.
        Connected,          // Connected to the other party.
        Hold,               // Connected, but currently on hold.
        HangupLocal,        // Local side hung up the call.
        HangupRemote,       // Remote side hung up the call, or call lost.
        Missed,             // Incoming call that was missed.
        NetworkFailure,     // Network has failed in some way.
        OtherFailure,       // Something else went wrong.
        ServiceHangup       // Hangup due to supplementary service request.
    };

    // Scope of an operation.
    enum Scope
    {
        CallOnly,           // Only the referenced call.
        Group               // All calls in the same active/held group.
    };

    // Request types, for reporting failures.
    enum Request
    {
        HoldFailed,
        JoinFailed,
        TransferFailed,
        ActivateFailed
    };

    // Notification types for phone call properties.
    enum Notification
    {
        CallingName,
        DataStateUpdate,
        RemoteHold,
        ConnectedLineId
    };

    enum DataState
    {
        PPPdStarted         = 0x0001,
        PPPdStopped         = 0x0002,
        PPPdFailed          = 0x0004,
        DataActive          = 0x0008,
        DataInactive        = 0x0010,
        Connecting          = 0x0020,
        ConnectFailed       = 0x0040
    };

    static QPhoneCall::DataState parseDataState( const QString& value );

    QPhoneCall& operator=( const QPhoneCall& call );

    QString identifier() const;
    int modemIdentifier() const;

    QString fullNumber() const;

    QString number() const;
    QUniqueId contact() const;

    State state() const;

    QString callType() const;

    QDateTime startTime() const;
    QDateTime connectTime() const;
    QDateTime endTime() const;
    bool hasBeenConnected() const;

    // Convenience functions for detecting interesting states.
    bool idle() const { return ( state() == Idle ); }
    bool incoming() const { return ( state() == Incoming ); }
    bool dialing() const
        { return ( state() == Dialing || state() == Alerting ); }
    bool established() const
        { return ( state() == Connected || state() == Hold ); }
    bool connected() const { return ( state() == Connected ); }
    bool onHold() const { return ( state() == Hold ); }
    bool dropped() const { return ( state() >= HangupLocal ); }
    bool missed() const { return ( state() == Missed ); }

    bool dialed() const;

    QString pendingTones() const;

    void dial( const QString& number, bool callerid, const QUniqueId& contact = QUniqueId() );
    void dial( const QDialOptions& options );

    void accept();

    void hangup( QPhoneCall::Scope scope = Group );

    void hold();

    void activate( QPhoneCall::Scope scope = Group );

    void join( bool detachSubscriber = false );

    void tone( const QString& tones );

    void transfer( const QString& number );

    void requestFloor( int secs = -1 );
    void releaseFloor();
    bool haveFloor() const;
    bool floorAvailable() const;

    bool canAccept() const;
    bool canHold() const;
    bool canActivate( QPhoneCall::Scope scope = Group ) const;
    bool canJoin( bool detachSubscriber = false ) const;
    bool canTone() const;
    bool canTransfer() const;

    void connectPendingTonesChanged( QObject *, const char * ) const;
    void connectStateChanged( QObject *, const char * ) const;
    void disconnectStateChanged( QObject *, const char * ) const;

    void connectRequestFailed( QObject *, const char * ) const;
    void disconnectRequestFailed( QObject *, const char * ) const;

    void connectNotification( QObject *, const char * ) const;
    void disconnectNotification( QObject *, const char * ) const;

    void connectFloorChanged( QObject *, const char * ) const;
    void disconnectFloorChanged( QObject *, const char * ) const;

    QIODevice *device() const;

    bool isNull() const;

private:
    QPhoneCallPrivate *d;
};

inline bool operator==( const QPhoneCall& call1,
                        const QPhoneCall& call2 )
{
    return (call1.d == call2.d);
}

inline bool operator!=( const QPhoneCall& call1,
                        const QPhoneCall& call2 )
{
    return (call1.d != call2.d);
}

Q_DECLARE_USER_METATYPE_ENUM(QPhoneCall::State)
Q_DECLARE_USER_METATYPE_ENUM(QPhoneCall::Scope)
Q_DECLARE_USER_METATYPE_ENUM(QPhoneCall::Request)
Q_DECLARE_USER_METATYPE_ENUM(QPhoneCall::Notification)

#endif
