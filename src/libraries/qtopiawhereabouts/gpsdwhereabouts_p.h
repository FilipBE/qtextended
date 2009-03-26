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

#ifndef GPSDWHEREABOUTS_P_H
#define GPSDWHEREABOUTS_P_H

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

#include "qwhereabouts.h"
#include "qwhereaboutsupdate.h"

#include <QAbstractSocket>
#include <QHostAddress>
#include <QQueue>
#include <QMetaType>

class QByteArray;
class QTcpSocket;
class QBasicTimer;
class QTimerEvent;

class QGpsdWhereabouts : public QWhereabouts
{
    Q_OBJECT

public:
    enum ActionWhenConnected {
        PeriodicUpdates,
        SingleUpdate
    };

    explicit QGpsdWhereabouts(QObject *parent = 0,
                             const QHostAddress &addr = QHostAddress::LocalHost,
                             quint16 port = 2947);
    ~QGpsdWhereabouts();

public slots:
    virtual void startUpdates();
    virtual void stopUpdates();
    virtual void requestUpdate();

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void socketError(QAbstractSocket::SocketError error);
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketReadyRead();

private:
    void initialize();
    void parseDeviceStatus(const QByteArray &status);
    void parseFix(const QByteArray &fix);

    QHostAddress m_addr;
    quint16 m_port;
    QTcpSocket *m_sock;
    QBasicTimer *m_queryTimer;
    QQueue<ActionWhenConnected> m_actionsWhenConnected;

    Q_DISABLE_COPY(QGpsdWhereabouts)
};

Q_DECLARE_METATYPE(QGpsdWhereabouts::ActionWhenConnected)

#endif
