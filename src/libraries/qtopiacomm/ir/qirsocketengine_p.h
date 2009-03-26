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

#ifndef QIRSOCKETENGINE_P_H
#define QIRSOCKETENGINE_P_H

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

#include "qirsocket.h"
#include <qirnamespace.h>

class QIrSocketEngine
{
public:
    enum SelectType { SelectRead = 0x1, SelectWrite = 0x2};
    enum SocketOption { NonBlockingOption };

    Q_DECLARE_FLAGS(SelectTypes, SelectType)

    QIrSocketEngine();
    ~QIrSocketEngine();

    bool waitFor(QIrSocketEngine::SelectTypes types,
                 int fd, int timeout,
                 bool *timedOut,
                 bool *canRead = 0, bool *canWrite = 0);

    int select(QIrSocketEngine::SelectTypes types, int fd, int timeout,
               bool *canRead = 0, bool *canWrite = 0) const;

    inline QIrSocket::SocketError error() const
    {
        return m_error;
    }

    static QString getErrorString(QIrSocket::SocketError error);

    int socket();
    int accept(int fd);
    QIrSocket::SocketState connect(int fd, const QByteArray &service, quint32 remote);
    bool testConnected(int fd);
    bool listen(int fd, int backlog);
    bool bind(int fd, const QByteArray &service);
    qint64 writeToSocket(int fd, const char *data, qint64 len);
    qint64 readFromSocket(int fd, char *data, qint64 len);
    void close(int fd);

    qint64 bytesAvailable(int fd) const;
    void readSocketParameters(int fd, quint32 *remote) const;

    bool setSocketOption(int fd, QIrSocketEngine::SocketOption option);
    bool setServiceHints(int fd, QIr::DeviceClasses classes);

private:
    void handleConnectError(int error);

    QIrSocket::SocketError m_error;
};

#endif
