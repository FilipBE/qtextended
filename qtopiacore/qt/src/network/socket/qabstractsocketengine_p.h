/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QABSTRACTSOCKETENGINE_P_H
#define QABSTRACTSOCKETENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtNetwork/qhostaddress.h"
#include "QtNetwork/qabstractsocket.h"
#include "private/qobject_p.h"

QT_BEGIN_NAMESPACE

class QAuthenticator;
class QAbstractSocketEnginePrivate;

class QAbstractSocketEngineReceiver {
public:
    virtual ~QAbstractSocketEngineReceiver(){}
    virtual void readNotification()= 0;
    virtual void writeNotification()= 0;
    virtual void exceptionNotification()= 0;
#ifndef QT_NO_NETWORKPROXY
    virtual void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)= 0;
#endif
};

class Q_AUTOTEST_EXPORT QAbstractSocketEngine : public QObject
{
    Q_OBJECT
public:

    static QAbstractSocketEngine *createSocketEngine(const QHostAddress &address, QAbstractSocket::SocketType socketType, QObject *parent);
    static QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent);

    QAbstractSocketEngine(QObject *parent = 0);

    enum SocketOption {
        NonBlockingSocketOption,
        BroadcastSocketOption,
        ReceiveBufferSocketOption,
        SendBufferSocketOption,
        AddressReusable,
        BindExclusively,
        ReceiveOutOfBandData
    };

    virtual bool initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol) = 0;

    virtual bool initialize(int socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState) = 0;

    virtual int socketDescriptor() const = 0;

    virtual bool isValid() const = 0;

    virtual bool connectToHost(const QHostAddress &address, quint16 port) = 0;
    virtual bool bind(const QHostAddress &address, quint16 port) = 0;
    virtual bool listen() = 0;
    virtual int accept() = 0;
    virtual void close() = 0;

    virtual qint64 bytesAvailable() const = 0;

    virtual qint64 read(char *data, qint64 maxlen) = 0;
    virtual qint64 write(const char *data, qint64 len) = 0;

#ifndef QT_NO_UDPSOCKET
    virtual qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *addr = 0,
                                quint16 *port = 0) = 0;
    virtual qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &addr,
                                 quint16 port) = 0;
    virtual bool hasPendingDatagrams() const = 0;
    virtual qint64 pendingDatagramSize() const = 0;
#endif

    virtual int option(SocketOption option) const = 0;
    virtual bool setOption(SocketOption option, int value) = 0;

    virtual bool waitForRead(int msecs = 30000, bool *timedOut = 0) const = 0;
    virtual bool waitForWrite(int msecs = 30000, bool *timedOut = 0) const = 0;
    virtual bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
			    bool checkRead, bool checkWrite,
			    int msecs = 30000, bool *timedOut = 0) const = 0;

    QAbstractSocket::SocketError error() const;
    QString errorString() const;
    QAbstractSocket::SocketState state() const;
    QAbstractSocket::SocketType socketType() const;
    QAbstractSocket::NetworkLayerProtocol protocol() const;

    QHostAddress localAddress() const;
    quint16 localPort() const;
    QHostAddress peerAddress() const;
    quint16 peerPort() const;

    virtual bool isReadNotificationEnabled() const = 0;
    virtual void setReadNotificationEnabled(bool enable) = 0;
    virtual bool isWriteNotificationEnabled() const = 0;
    virtual void setWriteNotificationEnabled(bool enable) = 0;
    virtual bool isExceptionNotificationEnabled() const = 0;
    virtual void setExceptionNotificationEnabled(bool enable) = 0;

public Q_SLOTS:
    void readNotification();
    void writeNotification();
    void exceptionNotification();
#ifndef QT_NO_NETWORKPROXY
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
#endif

public:
    void setReceiver(QAbstractSocketEngineReceiver *receiver);
protected:
    QAbstractSocketEngine(QAbstractSocketEnginePrivate &dd, QObject* parent = 0);

    void setError(QAbstractSocket::SocketError error, const QString &errorString) const;
    void setState(QAbstractSocket::SocketState state);
    void setSocketType(QAbstractSocket::SocketType socketType);
    void setProtocol(QAbstractSocket::NetworkLayerProtocol protocol);
    void setLocalAddress(const QHostAddress &address);
    void setLocalPort(quint16 port);
    void setPeerAddress(const QHostAddress &address);
    void setPeerPort(quint16 port);

private:
    Q_DECLARE_PRIVATE(QAbstractSocketEngine)
    Q_DISABLE_COPY(QAbstractSocketEngine)
};

class QAbstractSocketEnginePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractSocketEngine)
public:
    QAbstractSocketEnginePrivate();

    mutable QAbstractSocket::SocketError socketError;
    mutable bool hasSetSocketError;
    mutable QString socketErrorString;
    QAbstractSocket::SocketState socketState;
    QAbstractSocket::SocketType socketType;
    QAbstractSocket::NetworkLayerProtocol socketProtocol;
    QHostAddress localAddress;
    quint16 localPort;
    QHostAddress peerAddress;
    quint16 peerPort;
    QAbstractSocketEngineReceiver *receiver;
};


class Q_AUTOTEST_EXPORT QSocketEngineHandler
{
protected:
    QSocketEngineHandler();
    virtual ~QSocketEngineHandler();
    virtual QAbstractSocketEngine *createSocketEngine(const QHostAddress &address, QAbstractSocket::SocketType socketType, QObject *parent) = 0;
    virtual QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent) = 0;

private:
    friend class QAbstractSocketEngine;
};

QT_END_NAMESPACE

#endif // QABSTRACTSOCKETENGINE_P_H
