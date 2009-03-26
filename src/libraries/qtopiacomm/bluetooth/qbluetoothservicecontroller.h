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

#ifndef QBLUETOOTHSERVICECONTROLLER_H
#define QBLUETOOTHSERVICECONTROLLER_H

#include <QObject>
#include <QStringList>
#include <qbluetoothnamespace.h>

class QBluetoothServiceControllerPrivate;
class QBLUETOOTH_EXPORT QBluetoothServiceController : public QObject
{
    friend class QBluetoothServiceControllerPrivate;
    Q_OBJECT

public:
    enum ServiceState {
        NotRunning,
        Starting,
        Running
    };

    explicit QBluetoothServiceController(QObject *parent = 0);
    ~QBluetoothServiceController();

    void start(const QString &name);
    void stop(const QString &name);
    QBluetoothServiceController::ServiceState state(const QString &name) const;

    void setSecurityOptions(const QString &name,
                            QBluetooth::SecurityOptions options);
    QBluetooth::SecurityOptions securityOptions(const QString &name) const;

    QString displayName(const QString &name) const;

    QStringList services() const;

signals:
    void started(const QString &name, bool error, const QString &description);
    void stopped(const QString &name);

private:
    QBluetoothServiceControllerPrivate *m_data;
};

#endif
