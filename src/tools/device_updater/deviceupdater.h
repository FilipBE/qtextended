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

#ifndef DEVICEUPDATER_H
#define DEVICEUPDATER_H

#include "ui_deviceupdaterbase.h"

#include <QDialog>

class LocalSocketListener;
class PackageScanner;
class DeviceUpdaterBase_ui;
class Configure;
class ConfigureData;

class DeviceUpdaterWidget : public QWidget, public Ui::DeviceUpdaterBase
{
    Q_OBJECT
public:
    DeviceUpdaterWidget( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~DeviceUpdaterWidget();
    void connectLocalSocket( LocalSocketListener * ) const;
    void connectScanner( PackageScanner * ) const;
signals:
    void sendPackage(const QModelIndex & );
    void done(int);
public slots:
    void sendPackage(const QString & );
    void sendButtonClicked();
private slots:
    void progressWake();
    void quitWidget();
    void showConfigure();
    void scannerUpdated();
    void handleCommand( const QString & );
    void serverStopped();
    void serverStarted();
    void serverButtonToggled();
};

#endif
