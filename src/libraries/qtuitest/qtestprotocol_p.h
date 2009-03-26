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

#ifndef QTESTPROTOCOL_P_H
#define QTESTPROTOCOL_P_H

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

#include <QTimer>
#include <QTime>
#include <QObject>
#include <QString>
#include <QFile>
#include <QVariant>
#include <QMap>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <qtuitestglobal.h>

#include "qelapsedtimer_p.h"

#define REMOTE_CONNECT_ERROR 99

class QTUITEST_EXPORT QTestMessage
{
public:
    QTestMessage(QString const &event = QString(), QVariantMap const &map = QVariantMap() );
    QTestMessage(QString const &event, const QTestMessage &other );
    QTestMessage(QString const &event, QString const &queryApp, QString const &queryPath = QString() );
    QTestMessage(const QTestMessage &other);
    virtual ~QTestMessage();

    QTestMessage& operator=(const QTestMessage &other);

    QString event() const;
    quint16 msgId() const;

    QVariant const operator[](QString const &key) const;
    QVariant &operator[](QString const &key);
    bool contains(QString const &key) const;
    QList<QString> keys() const;

    QString toString() const;

    bool statusOK() const;
    bool isNull() const;

protected:
    uint          m_phase;
    quint32       m_len;

    quint16       m_msg_id;
    QString       m_event;

    QVariantMap m_map;

    friend class QTestProtocol;
};
Q_DECLARE_METATYPE( QTestMessage )
Q_DECLARE_METATYPE( QTestMessage* )
Q_DECLARE_METATYPE( const QTestMessage* )

class QTUITEST_EXPORT QTestServerSocket : public QTcpServer
{
    Q_OBJECT
public:
    QTestServerSocket( quint16 port, int backlog = 1 );
    virtual ~QTestServerSocket();

    bool ok() const;
    quint16 port() const;
    QString address() const;

    virtual void onNewConnection( int socket ) = 0;

private:
    virtual void incomingConnection( int socket );
};

class QTUITEST_EXPORT QTestProtocol : public QTcpSocket
{
    Q_OBJECT
public:

    QTestProtocol();
    virtual ~QTestProtocol();

    virtual void setSocket( int socket );

    void enableReconnect( bool enable, uint reconnectInterval = 5000 );

    void connect( const QString& hostname, int port );
    void disconnect( bool disableReconnect = true );
    bool isConnected();
    virtual bool waitForConnected( int timeout = 10000 );

    virtual uint postMessage( QTestMessage const &message );

    virtual bool sendMessage( QTestMessage const &message, QTestMessage &reply, int timeout );
    virtual void replyMessage( QTestMessage *originalMsg, QTestMessage const &message );

    bool lastDataReceived();
    bool rxBusy();
    virtual void onReplyReceived( QTestMessage *reply );

    QString errorStr();

    virtual void onConnectionFailed( const QString &reason );
    virtual void onConnected() {}; // re-implement in derived class
    
    QString uniqueId();
    void enableDebug( bool debugOn );

public slots:
    void reconnect();
    void disableDebug();

protected:
    virtual void processMessage( QTestMessage *msg ) = 0;

    void send( QTestMessage const &message );
    static void sendPreamble( QDataStream *ds, quint16 msgId, const QString &event );

    bool receive( QTestMessage *msg, bool &syncError );

signals:
    void connectionClosed( QTestProtocol *socket );
    void connectionFailed( QTestProtocol *socket, const QString &reason );
    void replyReceived(int id = -1, QTestMessage const *message = 0);
    void replyConfirmed();

protected slots:
    void onData();
    void onSocketConnected();
    void onSocketClosed();
    void connectTimeout();
    void pingTimeout();
    void emitConnectionClosed();
    void processMessages();
    void testConnection();

private:
    void enablePing( bool enable );

    quint16         tx_msg_id;
    QString         host;
    int             port;
    bool            onData_busy;
    bool            enable_reconnect;
    uint            reconnect_interval;
    QTimer          connect_timer;
    QElapsedTimer   rx_timer;
    bool            last_data_received;
    bool            rx_busy;
    bool            connection_valid;

    QList<QTestMessage*> send_msg_replies;
    QList<QTestMessage*> unhandled_msg;
    QTestMessage   *cur_message;

    bool            ping_enabled;
    uint            ping_interval;
    QTimer          ping_timer;
    uint            ping_count;
    bool            ping_timeout_warning_issued;
    QString         last_send_cmd;
    QString         unique_id;
    bool            debug_on;
};

#define TAB_BAR_ALIAS ">@TAB_BAR@<"
#define OPTIONS_MENU_ALIAS ">@OPTIONS_MENU@<"
#define LAUNCHER_MENU_ALIAS ">@LAUNCHER_MENU@<" // launcher menu _can_ be a grid menu, but also a wheel menu, etc
#define SOFT_MENU_ALIAS ">@SOFT_MENU@<"
#define PROGRESS_BAR_ALIAS ">@PROGRESS_BAR@<"
#define CALL_ACCEPT_ALIAS ">@CALL_ACCEPT@<"
#define CALL_HANGUP_ALIAS ">@CALL_HANGUP@<"

#endif

