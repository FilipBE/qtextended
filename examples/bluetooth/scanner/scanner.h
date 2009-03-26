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

#ifndef SCANNER_H
#define SCANNER_H

#include <QMainWindow>
#include <QString>
#include <QBluetoothRemoteDevice>

class QAction;
class QBluetoothLocalDevice;
class QBluetoothRemoteDevice;
class QListWidget;
class QListWidgetItem;

class Scanner : public QMainWindow
{
    Q_OBJECT

public:
    Scanner(QWidget *parent = 0, Qt::WFlags f = 0);
    ~Scanner();

private slots:
    void discoveryStarted();
    void discoveryComplete();
    void remoteDeviceFound(const QBluetoothRemoteDevice &dev);
    void itemActivated(QListWidgetItem *item);

private:
    QAction *startScan;
    QBluetoothLocalDevice *btDevice;
    QListWidget *deviceList;
    QMap<QString, QBluetoothRemoteDevice> deviceInfo;
};

#endif
