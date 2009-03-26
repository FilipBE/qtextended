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

#ifndef SETTINGSDISPLAY_H
#define SETTINGSDISPLAY_H

#include <QWidget>
#include <QBluetoothLocalDevice>
#include <QCommDeviceController>

class LocalServicesDialog;
class QAction;
class QGroupBox;
class QCheckBox;
class QSpinBox;
class QLineEdit;

class SettingsDisplay : public QWidget
{
    Q_OBJECT
public:
    SettingsDisplay(QBluetoothLocalDevice *local, QCommDeviceController *controller, QWidget *parent = 0);
    ~SettingsDisplay();

private slots:
    void toggleLocalPowerState(bool enable);
    void toggleLocalVisibility(bool visible);
    void nameChanged(const QString &name);
    void nameEditingFinished();
    void timeoutEditingFinished();

    void powerStateChanged(QCommDeviceController::PowerState state);
    void deviceStateChanged(QBluetoothLocalDevice::State state);

    void showDetailsDialog();
    void showMyServices();
    void init();

private:
    int getTimeout();
    void setInteractive(bool interactive);

    QBluetoothLocalDevice *m_local;
    QCommDeviceController *m_deviceController;
    LocalServicesDialog *m_localServicesDialog;

    QGroupBox *m_optionsGroupBox;
    QCheckBox *m_powerCheckBox;
    QCheckBox *m_visibilityCheckBox;
    QSpinBox  *m_timeoutSpinBox;
    QLineEdit *m_nameEdit;

    QAction *m_servicesAction;
    QAction *m_detailsAction;

    int m_lastTimeout;
};

#endif
