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

#ifndef QBLUETOOTHABSTRACTSERVER_P_H
#define QBLUETOOTHABSTRACTSERVER_P_H

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

class QBluetoothSocketEngine;
class QBluetoothAbstractServer;
class QSocketNotifier;
class QBluetoothAbstractSocket;

class QBluetoothAbstractServerPrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractServerPrivate(QBluetoothAbstractServer *parent);
    ~QBluetoothAbstractServerPrivate();

    int m_fd;

    bool m_isListening;
    QString m_errorString;
    int m_maxConnections;
    QList<QBluetoothAbstractSocket *> m_pendingConnections;

    QBluetoothAbstractServer *m_parent;
    QSocketNotifier *m_readNotifier;
    QBluetoothSocketEngine *m_engine;

public slots:
    void incomingConnection();
};

#define SERVER_DATA(Class) Class##Private * const m_data = static_cast<Class##Private *>(QBluetoothAbstractServer::m_data)

#endif
