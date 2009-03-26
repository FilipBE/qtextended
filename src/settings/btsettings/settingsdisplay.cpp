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

#include "settingsdisplay.h"
#include "localservicesdialog.h"

#include <qbluetoothaddress.h>

#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <QPhoneProfileManager>

#include <QDialog>
#include <QAction>
#include <QList>
#include <QMessageBox>
#include <QTimer>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>


SettingsDisplay::SettingsDisplay(QBluetoothLocalDevice *local, QCommDeviceController *controller, QWidget *parent)
    : QWidget(parent),
      m_local(local),
      m_deviceController(controller),
      m_localServicesDialog(0)
{
    // Create widgets
    m_powerCheckBox = new QCheckBox(tr("Enable Bluetooth"));
    m_visibilityCheckBox = new QCheckBox(tr("Visible to others"));
    m_timeoutSpinBox = new QSpinBox;
    m_timeoutSpinBox->setSuffix(tr("min", "short for minutes"));
    m_timeoutSpinBox->setRange(0, 10);
    m_timeoutSpinBox->setSpecialValueText(tr("Off"));
    m_nameEdit = new QLineEdit;

    QHBoxLayout *timeoutLayout = new QHBoxLayout;
    timeoutLayout->setAlignment(Qt::AlignLeft);
    timeoutLayout->addItem(new QSpacerItem(30, 1));
    QLabel *timeout = new QLabel(tr("Timeout:"));
    timeout->setBuddy(m_timeoutSpinBox);
    timeoutLayout->addWidget(timeout);
    timeoutLayout->addWidget(m_timeoutSpinBox);

    m_optionsGroupBox = new QGroupBox(tr("Options"));
    QGridLayout *optionsLayout = new QGridLayout;
    optionsLayout->addWidget(m_visibilityCheckBox, 0, 0, 1, 2);
    optionsLayout->addLayout(timeoutLayout, 1, 0, 1, 2, Qt::AlignLeft);
    QLabel *name = new QLabel(tr("Name:"));
    name->setBuddy(m_nameEdit);
    optionsLayout->addWidget(name, 2, 0);
    optionsLayout->addWidget(m_nameEdit, 2, 1);
    optionsLayout->setRowStretch(3, 1);
    m_optionsGroupBox->setLayout(optionsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_powerCheckBox);
    mainLayout->addWidget(m_optionsGroupBox);
    setLayout(mainLayout);

    // Create actions
    m_detailsAction = new QAction(tr("Other details"), this);
    addAction(m_detailsAction);
    m_servicesAction = new QAction(tr("My services..."), this);
    addAction(m_servicesAction);

    // Set initial states
    bool discoverable = m_local->discoverable();
    m_visibilityCheckBox->setChecked(discoverable);

    m_lastTimeout = m_local->discoverableTimeout();
    m_timeoutSpinBox->setValue(m_lastTimeout / 60);
    m_timeoutSpinBox->setEnabled(discoverable);

    m_nameEdit->setText(m_local->name());

    powerStateChanged(m_deviceController->powerState());

    QTimer::singleShot(0, this, SLOT(init()));
}

SettingsDisplay::~SettingsDisplay()
{
}

void SettingsDisplay::toggleLocalPowerState(bool enable)
{
    if (enable) {
        QPhoneProfileManager mgr;
        if (mgr.planeMode()) {
            QMessageBox::warning(this,
                    QObject::tr("Bluetooth Error"),
                    QObject::tr("<P>Cannot turn on Bluetooth while in Airplane mode."));
            m_powerCheckBox->setChecked(false);
            return;
        }

        m_deviceController->bringUp();
    } else {
        if (m_deviceController->sessionsActive()) {
            int result = QMessageBox::question(this,
                    QObject::tr("Turn off Bluetooth?"),
                    QObject::tr("<P>There are applications using the bluetooth device.  Are you sure you want to turn it off?"),
                    QMessageBox::Yes|QMessageBox::No);

            if (result == QMessageBox::No) {
                m_powerCheckBox->setChecked(true);
                return;
            }
        }

        setInteractive(false);
        m_deviceController->bringDown();
    }
}

void SettingsDisplay::toggleLocalVisibility(bool visible)
{
    if (visible) {
        m_local->setDiscoverable(getTimeout());
    } else {
        m_timeoutSpinBox->setEnabled(false);
        m_local->setConnectable();
    }
}

void SettingsDisplay::nameChanged(const QString &name)
{
    m_nameEdit->setText(name);
}

void SettingsDisplay::nameEditingFinished()
{
    QString newName = m_nameEdit->text();
    if (newName.trimmed().isEmpty())
        // revert to stored local name
        m_nameEdit->setText(m_local->name());
    else
        m_local->setName(newName);
}

void SettingsDisplay::timeoutEditingFinished()
{
    int timeout = getTimeout();
    if (timeout != m_lastTimeout && m_local->setDiscoverable(timeout))
        m_lastTimeout = timeout;
}

int SettingsDisplay::getTimeout()
{
    // timeouts are shown in minutes, setDiscoverable() needs them in seconds
    return m_timeoutSpinBox->value() * 60;
}

void SettingsDisplay::powerStateChanged(QCommDeviceController::PowerState state)
{
    // call setChecked() in case call didn't work and checkbox state needs
    // to be reverted
    bool enabled = (state != QCommDeviceController::Off);
    m_powerCheckBox->setChecked(enabled);
    setInteractive(enabled);

    // if bluetooth is turned off, the discoverable timeout is lost,
    // so need to set it again
    if (enabled && m_visibilityCheckBox->isChecked())
        m_local->setDiscoverable(getTimeout());
}

void SettingsDisplay::deviceStateChanged(QBluetoothLocalDevice::State state)
{
    m_timeoutSpinBox->setEnabled(state == QBluetoothLocalDevice::Discoverable);
}

void SettingsDisplay::setInteractive(bool interactive)
{
    // show/hide actions in the context menu
    QList<QAction *> actionList = actions();
    for (int i = 0; i < actionList.size(); ++i)
        actionList.at(i)->setVisible(interactive);

    m_optionsGroupBox->setEnabled(interactive);
}

// similar to details display for remote devices (in remotedeviceinfodialog.cpp)
void SettingsDisplay::showDetailsDialog()
{
    QDialog dlg;
    QFormLayout form(&dlg);

    form.addRow(tr("Address:"), new QLabel(m_local->address().toString()));
    form.addRow(tr("Version:"), new QLabel(m_local->version()));
    form.addRow(tr("Vendor:"), new QLabel(m_local->manufacturer()));
    form.addRow(tr("Company:"), new QLabel(m_local->company()));

    for (int i=0; i<form.count(); i++) {
        QLabel *label = qobject_cast<QLabel*>(form.itemAt(i)->widget());
        if (label) {
            label->setWordWrap(true);
            if (label->text() == "(null)")  // returned from hcid
                label->setText("");
        }
    }

    dlg.setWindowTitle(tr("Other details"));
    QtopiaApplication::execDialog(&dlg);
}

void SettingsDisplay::showMyServices()
{
    if (!m_localServicesDialog)
        m_localServicesDialog = new LocalServicesDialog(this);
    m_localServicesDialog->start();
}

void SettingsDisplay::init()
{
    // action signals
    connect(m_detailsAction, SIGNAL(triggered()), SLOT(showDetailsDialog()));
    connect(m_servicesAction, SIGNAL(triggered()), SLOT(showMyServices()));

    // for bluetooth on/off
    connect(m_powerCheckBox, SIGNAL(clicked(bool)), SLOT(toggleLocalPowerState(bool)));
    connect(m_deviceController, SIGNAL(powerStateChanged(QCommDeviceController::PowerState)),
            SLOT(powerStateChanged(QCommDeviceController::PowerState)));

    // for device visibility
    connect(m_visibilityCheckBox, SIGNAL(clicked(bool)), SLOT(toggleLocalVisibility(bool)));
    connect(m_local, SIGNAL(stateChanged(QBluetoothLocalDevice::State)),
            SLOT(deviceStateChanged(QBluetoothLocalDevice::State)));
    connect(m_timeoutSpinBox, SIGNAL(editingFinished()), SLOT(timeoutEditingFinished()));

    // for device name
    connect(m_local, SIGNAL(nameChanged(QString)), SLOT(nameChanged(QString)));
    connect(m_nameEdit, SIGNAL(editingFinished()), SLOT(nameEditingFinished()));
}

#include "settingsdisplay.moc"
