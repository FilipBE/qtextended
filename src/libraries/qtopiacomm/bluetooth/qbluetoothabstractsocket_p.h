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

#ifndef QBLUETOOTHABSTRACTSOCKET_P_H
#define QBLUETOOTHABSTRACTSOCKET_P_H

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

#include <QObject>

#include <private/qringbuffer_p.h>
#include <qbluetoothabstractsocket.h>

class QSocketNotifier;
class QTimer;
class QBluetoothSocketEngine;

class QBluetoothAbstractSocketPrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractSocketPrivate(bool isBuffered);
    ~QBluetoothAbstractSocketPrivate();

    bool initiateDisconnect();

    bool flush();

    bool readData();

    void resetNotifiers();
    void setupNotifiers();

    qint64 bytesAvailable() const;

public slots:
    void testConnected();
    void abortConnectionAttempt();
    bool readActivated();
    bool writeActivated();

public:
    QBluetoothAbstractSocket *m_parent;
    QBluetoothAbstractSocket::SocketError m_error;
    QBluetoothAbstractSocket::SocketState m_state;
    int m_fd;
    QSocketNotifier *m_readNotifier;
    QSocketNotifier *m_writeNotifier;
    QTimer *m_timer;

    QRingBuffer m_writeBuffer;
    QRingBuffer m_readBuffer;

    bool m_readSocketNotifierCalled;
    bool m_readSocketNotifierState;
    bool m_readSocketNotifierStateSet;
    bool m_emittedReadyRead;
    bool m_emittedBytesWritten;
    qint64 m_readBufferCapacity;

    bool m_isBuffered;

    int m_readMtu;
    int m_writeMtu;

    QBluetoothSocketEngine *m_engine;
};

#define SOCKET_DATA(Class) Class##Private * const m_data = static_cast<Class##Private *>(QBluetoothAbstractSocket::m_data)

#endif
