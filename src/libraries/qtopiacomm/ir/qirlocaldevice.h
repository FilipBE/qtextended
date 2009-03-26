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

#ifndef QIRLOCALDEVICE_H
#define QIRLOCALDEVICE_H

#include <qobject.h>
#include <qglobal.h>
#include <QString>

#include <qirglobal.h>
#include <qirnamespace.h>

class QIrIasAttribute;
class QIrLocalDevice_Private;
class QIrRemoteDevice;

class QIR_EXPORT QIrLocalDevice : public QObject
{
    Q_OBJECT

    friend class QIrLocalDevice_Private;

public:
    explicit QIrLocalDevice(const QString &device);
    ~QIrLocalDevice();

    const QString &deviceName() const;

    bool isUp() const;
    bool bringUp();
    bool bringDown();

    static QStringList devices();

    QVariant queryRemoteAttribute(const QIrRemoteDevice &remote,
                                  const QString &className,
                                  const QString &attribName);

public slots:
    bool discoverRemoteDevices(QIr::DeviceClasses classes = QIr::All);

signals:
    void discoveryStarted();
    void remoteDeviceFound(const QIrRemoteDevice &device);
    void remoteDevicesFound(const QList<QIrRemoteDevice> &devices);
    void discoveryCompleted();

private:
    QIrLocalDevice_Private *m_data;
    Q_DISABLE_COPY(QIrLocalDevice)
};

#endif
