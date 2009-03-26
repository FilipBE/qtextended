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

#ifndef QPHONECALL_P_H
#define QPHONECALL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qphonecall.h>
#include <qphonecallprovider.h>

class QPhoneCallManagerPrivate;
class QtopiaIpcAdaptor;
class QTimer;

class QPhoneCallPrivate : public QObject
{
    Q_OBJECT
public:
    QPhoneCallPrivate( QPhoneCallManagerPrivate *manager,
                       const QString& service, const QString& type,
                       const QString& identifier );
    ~QPhoneCallPrivate();

    void ref() { ++count; }
    bool deref() { return !--count; }

public:
    // Emit a "stateChanged" signal for this call.
    void emitStateChanged();

    // Emit a "requestFailed" signal for this call.
    void emitRequestFailed( QPhoneCall::Request request );

    // Emit a "notification" signal for this call.
    void emitNotification( QPhoneCall::Notification type, const QString& value );

    void addPendingTones( const QString &tones );

public slots:
    void abortCall();

private slots:
    void callStateChanged( const QString& identifier, QPhoneCall::State state,
                           const QString& number, const QString& service,
                           const QString& callType, int actions );
    void callRequestFailed
        ( const QString& identifier, QPhoneCall::Request request );
    void callNotification
        ( const QString& identifier, QPhoneCall::Notification type,
          const QString& value );
    void callFloorChanged
        ( const QString& identifier, bool haveFloor, bool floorAvailable );
    void callDataPort( const QString& identifier, int port );
    void sendPendingTones();

signals:
    void stateChanged( const QPhoneCall& conv );
    void floorChanged( const QPhoneCall& conv );
    void requestFailed( const QPhoneCall& conv, QPhoneCall::Request request );
    void pendingTonesChanged( const QPhoneCall &call );
    void notification( const QPhoneCall& conv, QPhoneCall::Notification type, const QString& value );

public:
    QPhoneCallManagerPrivate *manager;
    QString identifier;

    QTimer *toneTimer;

    QDateTime startTime, connectTime, endTime;
    QString fullNumber;
    QString number;
    QPhoneCall::State state;
    QPhoneCallImpl::Actions actions;
    QString service;
    QString callType;
    QUniqueId contact;

    QString pendingTones;
    bool hasBeenConnected;
    bool dialedCall;
    bool sentAllTones;
    bool haveFloor;
    bool floorAvailable;

    QIODevice *device;
    int dataPort;

    QtopiaIpcAdaptor *request;
    QtopiaIpcAdaptor *response;

    uint count;
};

#endif
