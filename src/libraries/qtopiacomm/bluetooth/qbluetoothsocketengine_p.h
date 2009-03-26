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

#ifndef QBLUETOOTHSOCKETENGINE_P_H
#define QBLUETOOTHSOCKETENGINE_P_H

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

#include <qbluetoothabstractsocket.h>
#include <qbluetoothnamespace.h>

class QBluetoothLocalDevice;
class QByteArray;

class QBluetoothSocketEngine
{
public:
    enum SelectType { SelectRead = 0x1, SelectWrite = 0x2};
    enum SocketOption { NonBlockingOption };

    Q_DECLARE_FLAGS(SelectTypes, SelectType)

    QBluetoothSocketEngine();
    ~QBluetoothSocketEngine();

    bool waitFor(QBluetoothSocketEngine::SelectTypes types,
                 int fd, int timeout,
                 bool *timedOut,
                 bool *canRead = 0, bool *canWrite = 0);

    int select(QBluetoothSocketEngine::SelectTypes types, int fd, int timeout,
               bool *canRead = 0, bool *canWrite = 0) const;

    inline QBluetoothAbstractSocket::SocketError error() const
    {
        return m_error;
    }

    static QString getErrorString(QBluetoothAbstractSocket::SocketError error);

    void handleSocketError(int error);
    int scoSocket();
    int rfcommSocket();
    int l2capSocket();
    int l2capDgramSocket();

    int accept(int fd);

    QBluetoothAbstractSocket::SocketState connectSco(int fd,
            const QBluetoothAddress &local,
            const QBluetoothAddress &remote);
    QBluetoothAbstractSocket::SocketState connectL2Cap(int fd,
            const QBluetoothAddress &local,
            const QBluetoothAddress &remote, int psm,
            int incomingMtu, int outgoingMtu);
    QBluetoothAbstractSocket::SocketState connectRfcomm(int fd,
            const QBluetoothAddress &local,
            const QBluetoothAddress &remote, int channel);

    bool testConnected(int fd);
    bool listen(int fd, int backlog);

    void handleBindError(int error);
    bool rfcommBind(int fd, const QBluetoothAddress &device, int channel);
    bool scoBind(int fd, const QBluetoothAddress &device);
    bool l2capBind(int fd, const QBluetoothAddress &device, int psm, int mtu);

    qint64 writeToSocket(int fd, const char *data, qint64 len);
    qint64 readFromSocket(int fd, char *data, qint64 len);
    void close(int fd);

    qint64 bytesAvailable(int fd) const;

    bool setSocketOption(int fd, QBluetoothSocketEngine::SocketOption option);

    bool getScoMtu(int fd, int *mtu) const;
    bool getL2CapIncomingMtu(int fd, int *imtu) const;
    bool setL2CapIncomingMtu(int fd, int imtu);
    bool getL2CapOutgoingMtu(int fd, int *omtu) const;
    bool setL2CapOutgoingMtu(int fd, int omtu);

    bool getsocknameSco(int fd, QBluetoothAddress *address) const;
    bool getsocknameRfcomm(int fd, QBluetoothAddress *address, int *channel) const;
    bool getsocknameL2Cap(int fd, QBluetoothAddress *address, int *psm) const;

    bool getpeernameSco(int fd, QBluetoothAddress *address) const;
    bool getpeernameRfcomm(int fd, QBluetoothAddress *address, int *channel) const;
    bool getpeernameL2Cap(int fd, QBluetoothAddress *address, int *psm) const;

    qint64 recvfrom(int fd, char *data, qint64 maxSize,
                    QBluetoothAddress *address, int *psm);

private:
    void handleConnectError(int error);
    QBluetoothAbstractSocket::SocketError m_error;
};

#endif
