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

#ifndef QIRSOCKET_H
#define QIRSOCKET_H

#include <QIODevice>
#include <QtGlobal>

#include <qglobal.h>
#include <qirglobal.h>

class QIrSocketPrivate;
class QIR_EXPORT QIrSocket : public QIODevice
{
    Q_OBJECT
    friend class QIrSocketPrivate;

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
        BusyError,
        HostUnreachableError,
        ServiceUnavailableError,
        ConnectionRefused,
        NetworkError,
        TimeoutError,
        RemoteHostClosedError,
        UnsupportedOperationError,
        AddressInUseError,
        AddressNotAvailableError,
        UnknownError
    };

    explicit QIrSocket(QObject *parent = 0);
    ~QIrSocket();

    bool connect(const QByteArray &service, quint32 addr);
    quint32 remoteAddress() const;

    void abort();
    virtual bool disconnect();

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor,
                             QIrSocket::SocketState state,
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
    void stateChanged(QIrSocket::SocketState socketState);
    void error(QIrSocket::SocketError socketError);
    void disconnected();

protected:
    virtual qint64 readLineData(char *data, qint64 maxsize);
    virtual qint64 readData(char *data, qint64 maxsize);
    virtual qint64 writeData(const char *data, qint64 size);

private:
    QIrSocketPrivate *m_data;
    Q_DISABLE_COPY(QIrSocket)
};

#endif
