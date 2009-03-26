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

#ifndef QBLUETOOTHABSTRACTSOCKET_H
#define QBLUETOOTHABSTRACTSOCKET_H

#include <QIODevice>
#include <QtGlobal>

#include <qbluetoothglobal.h>
#include <qbluetoothnamespace.h>

class QBluetoothAddress;
struct sockaddr;

class QBluetoothAbstractSocketPrivate;
class QBLUETOOTH_EXPORT QBluetoothAbstractSocket : public QIODevice
{
    Q_OBJECT
    friend class QBluetoothAbstractSocketPrivate;

public:
    enum SocketState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        BoundState,
        ClosingState,
    };

    enum SocketError {
        NoError,
        AccessError,
        ResourceError,
        BindError,
        ConnectionRefused,
        HostDownError,
        NetworkError,
        TimeoutError,
        RemoteHostClosedError,
        BusyError,
        HostUnreachableError,
        UnsupportedOperationError,
        AddressInUseError,
        AddressNotAvailableError,
        UnknownError
    };

    ~QBluetoothAbstractSocket();

    void abort();
    virtual bool disconnect();

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor,
                             QBluetoothAbstractSocket::SocketState state,
                             QIODevice::OpenMode openMode = QIODevice::ReadWrite);

    SocketError error() const;
    SocketState state() const;

    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);

    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

// from QIODevice
    virtual void close();
    virtual bool isSequential() const;
    virtual bool atEnd() const;
    virtual bool flush();
    virtual bool waitForReadyRead(int msecs);
    virtual bool waitForBytesWritten(int msecs);
    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool canReadLine() const;

signals:
    void connected();
    void stateChanged(QBluetoothAbstractSocket::SocketState socketState);
    void error(QBluetoothAbstractSocket::SocketError socketError);
    void disconnected();

protected:
    explicit QBluetoothAbstractSocket(QBluetoothAbstractSocketPrivate *data, QObject *parent = 0);

    virtual qint64 readLineData(char *data, qint64 maxsize);
    virtual qint64 readData(char *data, qint64 maxsize);
    virtual qint64 writeData(const char *data, qint64 size);

    bool handleConnect(int socket, QBluetoothAbstractSocket::SocketState state);
    virtual bool readSocketParameters(int sockfd) = 0;
    virtual void resetSocketParameters();
    void setError(QBluetoothAbstractSocket::SocketError error);

    void setReadMtu(int mtu);
    int readMtu() const;
    void setWriteMtu(int mtu);
    int writeMtu() const;

    QBluetoothAbstractSocketPrivate *m_data;

private:
    Q_DISABLE_COPY(QBluetoothAbstractSocket)
};

#endif
