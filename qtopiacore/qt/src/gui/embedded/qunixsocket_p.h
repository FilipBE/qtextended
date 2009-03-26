/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QUNIXSOCKET_P_H
#define QUNIXSOCKET_P_H

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

#include <QtNetwork/qabstractsocket.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>

extern "C" {
#include <sys/types.h>
};

QT_BEGIN_NAMESPACE

class QUnixSocketRights;
class QUnixSocketRightsPrivate;
class QUnixSocketPrivate;
class QUnixSocketMessagePrivate;
struct iovec;

class Q_GUI_EXPORT QUnixSocketRights {
public:
    QUnixSocketRights(int);
    ~QUnixSocketRights();

    QUnixSocketRights(const QUnixSocketRights &);
    QUnixSocketRights & operator=(const QUnixSocketRights &);

    bool isValid() const;

    int dupFd() const;
    int peekFd() const;

private:
    friend class QUnixSocket;
    QUnixSocketRights(int,int);
    QSharedDataPointer<QUnixSocketRightsPrivate> d;
};

class Q_GUI_EXPORT QUnixSocketMessage {
public:
    QUnixSocketMessage();
    QUnixSocketMessage(const QByteArray &);
    QUnixSocketMessage(const QByteArray &, const QList<QUnixSocketRights> &);
    QUnixSocketMessage(const QUnixSocketMessage &);
    QUnixSocketMessage(const iovec*, int);
    QUnixSocketMessage & operator=(const QUnixSocketMessage &);
    ~QUnixSocketMessage();

    void setBytes(const QByteArray &);
    void setRights(const QList<QUnixSocketRights> &);

    const QList<QUnixSocketRights> & rights() const;
    bool rightsWereTruncated() const;

    const QByteArray & bytes() const;

    pid_t processId() const;
    uid_t userId() const;
    gid_t groupId() const;

    void setProcessId(pid_t);
    void setUserId(uid_t);
    void setGroupId(gid_t);

    bool isValid() const;
private:
    friend class QUnixSocket;
    friend class QUnixSocketPrivate;
    QSharedDataPointer<QUnixSocketMessagePrivate> d;
};

class Q_GUI_EXPORT QUnixSocket : public QIODevice
{
    Q_OBJECT
public:
    QUnixSocket(QObject * = 0);
    QUnixSocket(qint64, qint64, QObject * = 0);
    virtual ~QUnixSocket();

    enum SocketState {
        UnconnectedState = QAbstractSocket::UnconnectedState,
        HostLookupState = QAbstractSocket::HostLookupState,
        ConnectingState = QAbstractSocket::ConnectingState,
        ConnectedState = QAbstractSocket::ConnectedState,
        BoundState = QAbstractSocket::BoundState,
        ClosingState = QAbstractSocket::ClosingState,
        ListeningState = QAbstractSocket::ListeningState,
    };

    enum SocketError { NoError, InvalidPath, ResourceError,
                       NonexistentPath, ConnectionRefused, UnknownError,
                       ReadFailure, WriteFailure };

    bool connect(const QByteArray & path);
    bool setSocketDescriptor(int socketDescriptor);
    int socketDescriptor() const;
    void abort();
    void close();

    bool flush();

    SocketError error() const;

    SocketState state() const;
    QByteArray address() const;

    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;

    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);
    qint64 rightsBufferSize() const;
    void setRightsBufferSize(qint64 size);

    bool canReadLine() const;

    qint64 write(const char * data, qint64 maxSize)
    { return QIODevice::write(data, maxSize); }
    qint64 write(const QByteArray & byteArray)
    { return QIODevice::write(byteArray); }
    qint64 read(char * data, qint64 maxSize)
    { return QIODevice::read(data, maxSize); }
    QByteArray read(qint64 maxSize)
    { return QIODevice::read(maxSize); }

    qint64 write(const QUnixSocketMessage &);
    QUnixSocketMessage read();

    virtual bool isSequential() const;
    virtual bool waitForReadyRead(int msec = 300);
    virtual bool waitForBytesWritten(int msec = 300);

Q_SIGNALS:
    void stateChanged(SocketState socketState);

protected:
    virtual qint64 readData(char * data, qint64 maxSize);
    virtual qint64 writeData (const char * data, qint64 maxSize);

private:
    QUnixSocket(const QUnixSocket &);
    QUnixSocket & operator=(const QUnixSocket &);

    QUnixSocketPrivate * d;
};

QT_END_NAMESPACE

#endif // QUNIXSOCKET_P_H
