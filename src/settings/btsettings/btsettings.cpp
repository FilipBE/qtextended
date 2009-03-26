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
#include "btsettings.h"
#include "btsettings_p.h"
#include "settingsdisplay.h"
#include "mydevicesdisplay.h"

#include <qbluetoothlocaldevice.h>

#include <QTabWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QSoftMenuBar>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <QScrollArea>

BTSettingsMainWindow::BTSettingsMainWindow(QWidget *parent, Qt::WFlags fl)
    : QMainWindow(parent, fl), m_localDevice(new QBluetoothLocalDevice(this)),
      m_controller(0)
{
    if (!m_localDevice->isValid()) {
        QLabel *label = new QLabel(tr("(Bluetooth not available.)"));
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        label->setWordWrap(true);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setCentralWidget(label);
        return;
    }

    QScrollArea* scroll = new QScrollArea();
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    scroll->setFrameStyle(QFrame::NoFrame);

    m_menu = QSoftMenuBar::menuFor(this);
    m_tabs = new QTabWidget();

    m_controller =
            new QCommDeviceController(m_localDevice->deviceName().toLatin1(), this);

    SettingsDisplay *settings = new SettingsDisplay(m_localDevice, m_controller);
    scroll->setWidget(settings);
    scroll->setFocusProxy(settings);
    m_tabs->addTab(scroll, tr("Settings"));

    // Delay initialization of tabs other than the first
    m_tabs->addTab(new QWidget, tr("Paired Devices"));
    m_tabs->setTabEnabled(1, false);

    m_tabs->setCurrentIndex(0);

    // change the context menu when the tab changes
    connect(m_tabs, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));

    // set the current context menu
    tabChanged(m_tabs->currentIndex());

    setCentralWidget(m_tabs);
    setWindowTitle(tr("Bluetooth"));

    QTimer::singleShot(0, this, SLOT(init()));
}

void BTSettingsMainWindow::init()
{
    MyDevicesDisplay *pairedDevices = new MyDevicesDisplay(m_localDevice);

    m_tabs->setUpdatesEnabled(false);
    QString title = m_tabs->tabText(1);
    delete m_tabs->widget(1);
    m_tabs->insertTab(1, pairedDevices, title);
    m_tabs->setUpdatesEnabled(true);

    connect(m_controller, SIGNAL(powerStateChanged(QCommDeviceController::PowerState)),
            SLOT(setTabsEnabled(QCommDeviceController::PowerState)));
    setTabsEnabled(m_controller->powerState());

    QTimer::singleShot(200, pairedDevices, SLOT(populateDeviceList()));
}

BTSettingsMainWindow::~BTSettingsMainWindow()
{
}

void BTSettingsMainWindow::tabChanged(int /*index*/)
{
    m_menu->clear();
    QWidget *w = m_tabs->currentWidget();
    if (QScrollArea *scroll = qobject_cast<QScrollArea *>(w))
        w = scroll->widget();
    m_menu->addActions(w->actions());
}

void BTSettingsMainWindow::setTabsEnabled(QCommDeviceController::PowerState state)
{
    // turn off all tabs except the first one if device is turned off.
    bool bluetoothOn = (state != QCommDeviceController::Off);
    for (int i=1; i<m_tabs->count(); i++)
        m_tabs->setTabEnabled(i, bluetoothOn);
}


//====================================================================

void BtSettings::setAudioProfileChannel(const QBluetoothAddress &addr, QBluetooth::SDPProfile profile, int channel)
{
    QSettings settings("Trolltech", "BluetoothKnownHeadsets");
    settings.setValue(addr.toString() + "/" + QString::number(int(profile)),
                      channel);
}

int BtSettings::audioProfileChannel(const QBluetoothAddress &addr, QBluetooth::SDPProfile profile)
{
    QSettings settings("Trolltech", "BluetoothKnownHeadsets");
    return settings.value(addr.toString() + "/" + QString::number(int(profile)), -1).toInt();
}
