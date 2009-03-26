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

#ifndef REMOTEDEVICEINFODIALOG_H
#define REMOTEDEVICEINFODIALOG_H

#include <qbluetoothaddress.h>
#include <qbluetoothremotedevice.h>

#include <QDialog>
#include <QVariant>

namespace Ui {
    class RemoteDeviceInfo;
};

class QBluetoothAddress;
class QBluetoothLocalDevice;
class QBluetoothRemoteDevice;
class ServicesDisplay;
class AudioDeviceConnectionStatus;
class GenericDeviceConnectionStatus;

class RemoteDeviceInfoDialog : public QDialog
{
    Q_OBJECT

public:
    RemoteDeviceInfoDialog(QBluetoothLocalDevice *local, QWidget *parent, Qt::WFlags flags = 0);
    ~RemoteDeviceInfoDialog();

    void setRemoteDevice(const QBluetoothAddress &addr);
    void setRemoteDevice(const QBluetoothRemoteDevice &device);

private slots:
    void showMoreInfo();
    void showServices();
    void setTitle();

private:
    Ui::RemoteDeviceInfo *m_ui;
    QBluetoothLocalDevice *m_local;

    QBluetoothRemoteDevice m_device;
    QBluetoothAddress m_address;
    QString m_name;
    QVariant m_nameFontVariant;

    ServicesDisplay *m_servicesDisplay;
    AudioDeviceConnectionStatus *m_audioDeviceConnStatus;
    GenericDeviceConnectionStatus *m_genericDeviceConnStatus;
};

#endif
