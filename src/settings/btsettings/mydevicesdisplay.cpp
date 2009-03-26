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
#include "mydevicesdisplay.h"
#include "pairingagent.h"
#include "remotedeviceinfodialog.h"
#include "btsettings_p.h"

#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothremotedevice.h>
#include <qbluetoothremotedevicedialog.h>
#include <qbluetoothsdpquery.h>
#include <qbluetoothsdprecord.h>
#include <qtopiacomm/private/qbluetoothremotedeviceselector_p.h>

#include <qtopiaapplication.h>
#include <qwaitwidget.h>
#include <qsoftmenubar.h>

#include <QList>
#include <QVBoxLayout>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>


class DeviceDialogFilter : public QBluetoothRemoteDeviceDialogFilter 
{
public:
    DeviceDialogFilter(const QList<QBluetoothAddress> &devices);
    virtual bool filterAcceptsDevice(const QBluetoothRemoteDevice &device);

    QList<QBluetoothAddress> m_pairedDevices;
};

DeviceDialogFilter::DeviceDialogFilter(const QList<QBluetoothAddress> &devices)
    : m_pairedDevices(devices)
{
}

bool DeviceDialogFilter::filterAcceptsDevice(const QBluetoothRemoteDevice &device)
{
    if (m_pairedDevices.contains(device.address()))
        return false;
    return true;
}


//=======================================================

MyDevicesDisplay::MyDevicesDisplay(QBluetoothLocalDevice *local, QWidget *parent)
    : QWidget(parent),
      m_local(local),
      m_browser(0),
      m_devicesPlaceholderLabel(0),
      m_pairingAgent(0),
      m_pairingWaitWidget(0),
      m_pairingDeviceDialog(0),
      m_sdpQuery(0),
      m_deviceInfoDialog(0)
{
    // don't show pairing status icon since all device in the list are paired
    m_browser = new QBluetoothRemoteDeviceSelector(
            QBluetoothRemoteDeviceSelector::DeviceIcon |
            QBluetoothRemoteDeviceSelector::Name |
            QBluetoothRemoteDeviceSelector::Alias |
            QBluetoothRemoteDeviceSelector::ConnectionStatus, m_local, this);

    QVBoxLayout *l = new QVBoxLayout;
    l->addWidget(m_browser);
    setLayout(l);

    QTimer::singleShot(0, this, SLOT(init()));
}

MyDevicesDisplay::~MyDevicesDisplay()
{
}

void MyDevicesDisplay::init()
{
    // set up some signals and slots:

    connect(m_browser, SIGNAL(selectionChanged()),
            SLOT(deviceSelectionChanged()));
    connect(m_browser, SIGNAL(activated(QBluetoothAddress)),
            SLOT(deviceActivated(QBluetoothAddress)));

    connect(m_local, SIGNAL(pairingCreated(QBluetoothAddress)),
            SLOT(pairingCreated(QBluetoothAddress)));
    connect(m_local, SIGNAL(pairingRemoved(QBluetoothAddress)),
            SLOT(pairingRemoved(QBluetoothAddress)));


    // now set up actions:

    // this action is always visible
    QAction *pairAction = new QAction(QIcon(":image/bluetooth/paired"),
            tr("Pair with new device..."), this);
    connect(pairAction, SIGNAL(triggered()), SLOT(newPairing()));
    addAction(pairAction);

    // Actions in the m_deviceActions list are hidden if no devices are
    // displayed, and disabled if no devices are selected.
    // They are also shown in the remote device info dialog that shows
    // up when a device in the list is activated.
    QAction *action;

    action = new QAction(this);
    action->setSeparator(true);
    m_deviceActions.append(action);

    action = new QAction(tr("Set nickname..."), this);
    connect(action, SIGNAL(triggered()), SLOT(setAlias()));
    m_deviceActions.append(action);

    action = new QAction(tr("Remove pairing"), this);
    connect(action, SIGNAL(triggered()), SLOT(deleteDevice()));
    m_deviceActions.append(action);

    // add all device actions to own list of actions
    addActions(m_deviceActions);
}

void MyDevicesDisplay::showEvent(QShowEvent *e)
{
    populateDeviceList();
    QWidget::showEvent(e);
}

/*
    This must be called to properly set up the list of devices.
*/
void MyDevicesDisplay::populateDeviceList()
{
    if (!m_initialDevices.isEmpty())
        return;     // already initialised

    // get paired devices
    m_initialDevices = m_local->pairedDevices();

    // add paired devices to browser
    if (m_initialDevices.size() > 0) {
        for (int i=0; i<m_initialDevices.size(); i++)
            m_browser->insert(QBluetoothRemoteDevice(m_initialDevices[i]));
        m_browser->selectDevice(m_initialDevices[0]);
    }

    if (m_initialDevices.size() > 0) {
        m_refreshIndex = 0;
        QTimer::singleShot(10, this, SLOT(refreshNextDevice()));
    } else {
        resetDisplay();
    }
}

/*
    If there are no devices to display, then show a blank screen
    with a "no devices" label instead of a blank list.
*/
void MyDevicesDisplay::resetDisplay()
{
    if (m_browser->count() > 0) {
        for (int i=0; i<m_deviceActions.size(); i++)
            m_deviceActions[i]->setVisible(true);
        m_browser->setVisible(true);

        if (m_devicesPlaceholderLabel)
            m_devicesPlaceholderLabel->setVisible(false);

    } else {
        if (!m_devicesPlaceholderLabel) {
            m_devicesPlaceholderLabel = new QLabel(tr("(No paired devices.)"));
            m_devicesPlaceholderLabel->setAlignment(Qt::AlignCenter);
            m_devicesPlaceholderLabel->setMargin(10);
            m_devicesPlaceholderLabel->setWordWrap(true);
            layout()->addWidget(m_devicesPlaceholderLabel);
        }
        m_devicesPlaceholderLabel->setVisible(true);

        for (int i=0; i<m_deviceActions.size(); i++)
            m_deviceActions[i]->setVisible(false);
        m_browser->setVisible(false);
    }
}

void MyDevicesDisplay::refreshNextDevice()
{
    QBluetoothRemoteDevice device(m_initialDevices[m_refreshIndex]);
    m_local->updateRemoteDevice(device);
    m_browser->update(device);

    m_refreshIndex++;
    if (m_refreshIndex != m_initialDevices.size())
        QTimer::singleShot(10, this, SLOT(refreshNextDevice()));
}

void MyDevicesDisplay::newPairing()
{
    if (m_pairingDeviceDialog) {
        delete m_pairingDeviceDialog;
        m_pairingDeviceDialog = 0;
    }

    m_pairingDeviceDialog = new QBluetoothRemoteDeviceDialog(m_local, this);
    m_pairingDeviceDialog->addFilter(new DeviceDialogFilter(m_local->pairedDevices()));
    m_pairingDeviceDialog->setFilterSelectionEnabled(false);
    connect(m_pairingDeviceDialog, SIGNAL(accepted()),
            SLOT(selectedPairingTarget()));
    QtopiaApplication::showDialog(m_pairingDeviceDialog);
}

void MyDevicesDisplay::selectedPairingTarget()
{
    createPairing(m_pairingDeviceDialog->selectedDevice());
}

void MyDevicesDisplay::createPairing(const QBluetoothAddress &addr)
{
    if (!m_pairingAgent) {
        m_pairingAgent = new PairingAgent(m_local, this);
        connect(m_pairingAgent, SIGNAL(done(bool)),
                SLOT(pairingAgentDone(bool)));
    }
    if (!m_pairingWaitWidget) {
        m_pairingWaitWidget = new QWaitWidget(this);
        connect(m_pairingWaitWidget, SIGNAL(cancelled()),
                m_pairingAgent, SLOT(cancel()));
    }

    m_pairingWaitWidget->setText(tr("Pairing..."));
    m_pairingWaitWidget->setCancelEnabled(true);
    m_pairingWaitWidget->show();
    m_pairingAgent->start(addr);
}

void MyDevicesDisplay::setAlias()
{
    QBluetoothAddress addr = m_browser->selectedDevice();

    if (addr.isValid()) {
        QString text = m_local->remoteAlias(addr);

        QDialog dlg(this);
        dlg.setWindowTitle(tr("Change nickname"));
        QVBoxLayout layout;
        layout.setMargin(9);
        layout.setSpacing(6);
        QLabel label(tr("Enter a nickname:"));
        layout.addWidget(&label);
        QLineEdit lineEdit(text);
        layout.addWidget(&lineEdit);
        dlg.setLayout(&layout);

        if (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted) {
            QString newAlias = lineEdit.text();
            if (newAlias.trimmed().isEmpty()) {
                m_local->removeRemoteAlias(addr);
            } else if (newAlias != text) {
                m_local->setRemoteAlias(addr, newAlias);
            }
        }
    }
}

void MyDevicesDisplay::deviceSelectionChanged()
{
    bool valid = m_browser->selectedDevice().isValid();
    for (int i=0; i<m_deviceActions.size(); i++)
        m_deviceActions[i]->setEnabled(valid);
}

void MyDevicesDisplay::deviceActivated(const QBluetoothAddress &addr)
{
    if (!m_deviceInfoDialog) {
        m_deviceInfoDialog = new RemoteDeviceInfoDialog(m_local, this);
        QMenu *menu = QSoftMenuBar::menuFor(m_deviceInfoDialog);
        menu->addActions(m_deviceActions);
    }

    m_deviceInfoDialog->setRemoteDevice(addr);
    m_deviceInfoDialog->adjustSize();
    QtopiaApplication::execDialog(m_deviceInfoDialog);
}

void MyDevicesDisplay::pairingAgentDone(bool error)
{
    if (m_pairingAgent->wasCanceled()) {
        // Show the device dialog again if the pairing was cancelled - user
        // might have selected the wrong device.
        QtopiaApplication::showDialog(m_pairingDeviceDialog);
    } else {
        if (error) {
            doneAddDevice(true, tr("<P>Unable to create pairing"));
        } else {
            if (!m_sdpQuery) {
                m_sdpQuery = new QBluetoothSdpQuery(this);
                connect(m_sdpQuery, SIGNAL(searchComplete(QBluetoothSdpQueryResult)),
                        SLOT(foundServices(QBluetoothSdpQueryResult)));
            }

            // don't allow cancelling of SDP query, too messy (and it should
            // be quick since we've just paired)
            m_pairingWaitWidget->setCancelEnabled(false);
            m_pairingWaitWidget->setText(tr("Checking services..."));
            if (!m_sdpQuery->searchServices(m_pairingAgent->remoteAddress(), *m_local,
                                       QBluetoothSdpUuid::L2cap)) {
                doneAddDevice(true, tr("<P>Unable to request device services"));
            }
        }
    }
}

void MyDevicesDisplay::foundServices(const QBluetoothSdpQueryResult &result)
{
    if (!result.isValid()) {
        doneAddDevice(true, tr("<P>Unable to request device services"));
    } else {
        const QList<QBluetoothSdpRecord> &services = result.services();
        for (int i=0; i<services.size(); i++) {
            if (services[i].isInstance(QBluetooth::HeadsetProfile)) {
                BtSettings::setAudioProfileChannel(
                        m_pairingAgent->remoteAddress(),
                        QBluetooth::HeadsetProfile,
                        QBluetoothSdpRecord::rfcommChannel(services[i]));

            } else if (services[i].isInstance(QBluetooth::HandsFreeProfile)) {
                BtSettings::setAudioProfileChannel(
                        m_pairingAgent->remoteAddress(),
                        QBluetooth::HandsFreeProfile,
                        QBluetoothSdpRecord::rfcommChannel(services[i]));
            }
        }

        doneAddDevice(false, QLatin1String(""));
    }
}

void MyDevicesDisplay::doneAddDevice(bool error, const QString &errorString)
{
    m_pairingWaitWidget->hide();
    if (error) {
        QMessageBox::warning(this, tr("Pairing Error"), errorString);
    }
}

void MyDevicesDisplay::pairingCreated(const QBluetoothAddress &addr)
{
    qLog(Bluetooth) << "MyDevicesDisplay::pairingCreated()" << addr.toString();

    QBluetoothRemoteDevice device(addr);
    if (m_browser->insert(device)) {
        if (m_browser->count() == 1)    // this is the 1st added device
            resetDisplay();

        // must update name display since a QBluetoothRemoteDevice
        // won't be created with it by default
        m_local->updateRemoteDevice(device);

        m_browser->update(device, QBluetoothRemoteDeviceSelector::Name |
                                  QBluetoothRemoteDeviceSelector::DeviceIcon);

        // did we initiate this pairing through the pairing agent?
        if (m_pairingAgent && m_pairingAgent->remoteAddress() == addr)
            m_browser->selectDevice(addr);
    }
}

void MyDevicesDisplay::deleteDevice()
{
    if (QMessageBox::question(this, tr("Confirm"),
            tr("Remove the pairing with this device?"),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {

        QBluetoothAddress addr = m_browser->selectedDevice();
        if (addr.isValid()) {
            if (!m_local->removePairing(addr)) {
                QMessageBox::warning(this, tr("Pairing Error"),
                                        tr("Unable to remove pairing"));
            }
        }
    }
}

void MyDevicesDisplay::pairingRemoved(const QBluetoothAddress &addr)
{
    qLog(Bluetooth) << "MyDevicesDisplay::pairingRemoved()" << addr.toString();

    if (m_browser->remove(addr)) {
        if (m_browser->count() == 0)
            resetDisplay();
        else
            m_browser->selectDevice(m_browser->devices().first());

        if (m_deviceInfoDialog && m_deviceInfoDialog->isVisible())
            m_deviceInfoDialog->hide();

        // remove any alias that's been set, or else user can't change the
        // the alias anymore once the device has been removed from the list
        m_local->removeRemoteAlias(addr);
    }
}
