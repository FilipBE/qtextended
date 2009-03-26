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

#ifndef QBLUETOOTHREMOTEDEVICESELECTOR_P_H
#define QBLUETOOTHREMOTEDEVICESELECTOR_P_H

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

#include <qbluetoothaddress.h>

#include <QWidget>

class QBluetoothRemoteDevice;
class QBluetoothRemoteDeviceSelectorPrivate;

class QBLUETOOTH_EXPORT QBluetoothRemoteDeviceSelector : public QWidget
{
    Q_OBJECT
public:
    enum DisplayFlag {
        DeviceIcon = 0x1,
        Name = 0x2,
        Alias = 0x4,
        PairingStatus = 0x8,
        ConnectionStatus = 0x10
    };
    Q_DECLARE_FLAGS(DisplayFlags, DisplayFlag)

    QBluetoothRemoteDeviceSelector(DisplayFlags displayFlags,
                                   QBluetoothLocalDevice *local,
                                   QWidget *parent = 0,
                                   Qt::WFlags flags = 0);
    QBluetoothRemoteDeviceSelector(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~QBluetoothRemoteDeviceSelector();

    DisplayFlags flags() const;

    bool insert(const QBluetoothRemoteDevice &device);
    void update(const QBluetoothRemoteDevice &device);
    void update(const QBluetoothRemoteDevice &device, DisplayFlags flags);
    bool remove(const QBluetoothAddress &address);

    bool isDeviceHidden(const QBluetoothAddress &address) const;
    bool contains(const QBluetoothAddress &address) const;
    int count() const;

    QList<QBluetoothAddress> devices() const;
    QBluetoothAddress selectedDevice() const;

public slots:
    void clear();
    void selectDevice(const QBluetoothAddress &address);

    void hideDevice(const QBluetoothAddress &address);
    void showDevice(const QBluetoothAddress &address);

signals:
    void selectionChanged();
    void activated(const QBluetoothAddress &addr);

private:
    friend class QBluetoothRemoteDeviceSelectorPrivate;
    QBluetoothRemoteDeviceSelectorPrivate *m_data;
    Q_DISABLE_COPY(QBluetoothRemoteDeviceSelector)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QBluetoothRemoteDeviceSelector::DisplayFlags)
Q_DECLARE_USER_METATYPE_ENUM(QBluetoothRemoteDeviceSelector::DisplayFlag)

#endif
