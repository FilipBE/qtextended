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

#ifndef MYDEVICESDISPLAY_H
#define MYDEVICESDISPLAY_H

#include <qbluetoothaddress.h>
#include <qbluetoothnamespace.h>

#include <QWidget>
#include <QList>

class QBluetoothRemoteDeviceSelector;
class QBluetoothLocalDevice;
class QAction;
class QWaitWidget;
class PairingAgent;
class QBluetoothRemoteDeviceDialog;
class QBluetoothSdpQuery;
class QBluetoothSdpQueryResult;
class RemoteDeviceInfoDialog;
class QLabel;

class MyDevicesDisplay : public QWidget
{
    Q_OBJECT
public:
    MyDevicesDisplay(QBluetoothLocalDevice *local, QWidget *parent = 0);
    ~MyDevicesDisplay();

protected:
    void showEvent(QShowEvent *e);

public slots:
    void populateDeviceList();

private slots:
    void refreshNextDevice();
    void setAlias();
    void deviceSelectionChanged();
    void deviceActivated(const QBluetoothAddress &addr);

    void newPairing();
    void selectedPairingTarget();
    void pairingAgentDone(bool error);
    void pairingCreated(const QBluetoothAddress &addr);

    void deleteDevice();
    void pairingRemoved(const QBluetoothAddress &addr);

    void foundServices(const QBluetoothSdpQueryResult &);

    void init();

private:
    void resetDisplay();
    void createPairing(const QBluetoothAddress &addr);
    void doneAddDevice(bool error, const QString &errorString);

    QBluetoothLocalDevice *m_local;
    QBluetoothRemoteDeviceSelector *m_browser;
    QLabel *m_devicesPlaceholderLabel;
    QList<QBluetoothAddress> m_initialDevices;
    int m_refreshIndex;

    PairingAgent *m_pairingAgent;
    QWaitWidget *m_pairingWaitWidget;

    QBluetoothRemoteDeviceDialog *m_pairingDeviceDialog;
    QBluetoothSdpQuery *m_sdpQuery;

    RemoteDeviceInfoDialog *m_deviceInfoDialog;

    QList<QAction *> m_deviceActions;
};

#endif
