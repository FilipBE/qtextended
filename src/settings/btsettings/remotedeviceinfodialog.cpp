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
#include "remotedeviceinfodialog.h"
#include "ui_remotedeviceinfo.h"
#include "btsettings_p.h"

#include <qbluetoothlocaldevice.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qbluetoothsdpquery.h>
#include <qbluetoothaudiogateway.h>
#include <qbluetoothaddress.h>

#include <qformlayout.h>
#include <qwaitwidget.h>
#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qtopialog.h>

#include <QMenu>
#include <QSoftMenuBar>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QStringListModel>
#include <QTimer>


//===================================================================

class ServicesDisplay : public QObject
{
    Q_OBJECT
public:
    ServicesDisplay(QObject *parent = 0);
    ~ServicesDisplay();
    void setRemoteDevice(const QBluetoothAddress &address, const QBluetoothLocalDevice &local);

private slots:
    void foundServices(const QBluetoothSdpQueryResult &result);

private:
    QBluetoothSdpQuery *m_sdpQuery;
    QWaitWidget *m_wait;
    QStringListModel *m_serviceModel;
    QBluetoothAddress m_address;
};

ServicesDisplay::ServicesDisplay(QObject *parent)
    : QObject(parent),
      m_sdpQuery(new QBluetoothSdpQuery(this)),
      m_wait(new QWaitWidget(0)),
      m_serviceModel(new QStringListModel(this))
{
    connect(m_sdpQuery, SIGNAL(searchComplete(QBluetoothSdpQueryResult)),
            SLOT(foundServices(QBluetoothSdpQueryResult)));

    m_wait->setText(tr("Querying services..."));
    m_wait->setCancelEnabled(true);
    connect(m_wait, SIGNAL(cancelled()), m_sdpQuery, SLOT(cancelSearch()));
}

ServicesDisplay::~ServicesDisplay()
{
    delete m_wait;
}

void ServicesDisplay::setRemoteDevice(const QBluetoothAddress &address, const QBluetoothLocalDevice &local)
{
    if (!m_sdpQuery->searchServices(address, local, QBluetoothSdpUuid::L2cap)) {
        QMessageBox::warning(0, tr("Services Query Error"),
                             tr("<P>Unable to request device services"));
        return;
    }
    m_address = address;
    m_serviceModel->removeRows(0, m_serviceModel->rowCount());
    m_wait->show();
}

void ServicesDisplay::foundServices(const QBluetoothSdpQueryResult &result)
{
    if (!result.isValid()) {
        QMessageBox::warning(0, tr("Services Query Error"),
                             tr("<P>Unable to request device services"));
        m_wait->hide();
        return;
    }

    QStringList serviceNames;
    const QList<QBluetoothSdpRecord> &services = result.services();
    for (int i=0; i<services.size(); i++) {
        serviceNames << (services[i].serviceName());

        // save any Headset or Handsfree channel info
        if (services[i].isInstance(QBluetooth::HeadsetProfile)) {
            BtSettings::setAudioProfileChannel(
                    m_address,
                    QBluetooth::HeadsetProfile,
                    QBluetoothSdpRecord::rfcommChannel(services[i]));

        } else if (services[i].isInstance(QBluetooth::HandsFreeProfile)) {
            BtSettings::setAudioProfileChannel(
                    m_address,
                    QBluetooth::HandsFreeProfile,
                    QBluetoothSdpRecord::rfcommChannel(services[i]));
        }
    }
    m_serviceModel->setStringList(serviceNames);

    QDialog *dlg = new QDialog;
    dlg->setWindowTitle(tr("Supported services"));
    QVBoxLayout *layout = new QVBoxLayout;
    if (m_serviceModel->rowCount() > 0) {
        QListView *view = new QListView;
        view->setAlternatingRowColors(true);
        view->setFrameShape(QFrame::NoFrame);
        view->setModel(m_serviceModel);
        view->setCurrentIndex(m_serviceModel->index(0, 0));
        layout->addWidget(view);
        QSoftMenuBar::setLabel(view, Qt::Key_Select, QSoftMenuBar::NoLabel);
    } else {
        QLabel *label = new QLabel(tr("(No available services.)"));
        layout->setMargin(9);
        layout->setSpacing(6);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        layout->addWidget(label);
    }
    dlg->setLayout(layout);
    m_wait->hide();

    QtopiaApplication::execDialog(dlg);
    delete dlg;
}


//==========================================================================


class DeviceConnectionStatus : public QWidget
{
    Q_OBJECT
public:
    DeviceConnectionStatus(QBluetoothLocalDevice *local, QWidget *parent = 0);
    virtual void setRemoteDevice(const QBluetoothAddress &addr);

    const QBluetoothAddress &address() const;
    QBluetoothLocalDevice *localDevice() const;
    bool isTransportConnected() const;

    virtual void updateConnectionStatus() = 0;

protected slots:
    virtual void remoteDeviceConnected(const QBluetoothAddress &addr);
    virtual void remoteDeviceDisconnected(const QBluetoothAddress &addr);

private:
    QBluetoothAddress m_address;
    QBluetoothLocalDevice *m_local;
    bool m_connected;
};

DeviceConnectionStatus::DeviceConnectionStatus(QBluetoothLocalDevice *local, QWidget *parent)
    : QWidget(parent),
      m_local(local),
      m_connected(false)
{
    connect(local, SIGNAL(remoteDeviceConnected(QBluetoothAddress)),
            SLOT(remoteDeviceConnected(QBluetoothAddress)));
    connect(local, SIGNAL(remoteDeviceDisconnected(QBluetoothAddress)),
            SLOT(remoteDeviceDisconnected(QBluetoothAddress)));
}

void DeviceConnectionStatus::setRemoteDevice(const QBluetoothAddress &addr)
{
    m_address = addr;
    m_connected = m_local->isConnected(m_address);
}

const QBluetoothAddress &DeviceConnectionStatus::address() const
{
    return m_address;
}

QBluetoothLocalDevice *DeviceConnectionStatus::localDevice() const
{
    return m_local;
}

bool DeviceConnectionStatus::isTransportConnected() const
{
    return m_connected;
}

void DeviceConnectionStatus::remoteDeviceConnected(const QBluetoothAddress &addr)
{
    qLog(Bluetooth) << "remoteDeviceConnected:" << addr.toString();
    if (addr == m_address) {
        m_connected = true;
        updateConnectionStatus();
    }
}

void DeviceConnectionStatus::remoteDeviceDisconnected(const QBluetoothAddress &addr)
{
    qLog(Bluetooth) << "remoteDeviceDisconnected:" << addr.toString();
    if (addr == m_address) {
        m_connected = false;
        updateConnectionStatus();
    }
}

//===================================================================

/*
    Displays information about whether the local device has a transport
    connection to a remote device. Allows user to disconnect the transport
    if it is connected.
*/
class GenericDeviceConnectionStatus : public DeviceConnectionStatus
{
    Q_OBJECT
public:
    GenericDeviceConnectionStatus(QBluetoothLocalDevice *local, QWidget *parent = 0);
    virtual void updateConnectionStatus();

private slots:
    void clickedDisconnect();

private:
    QLabel *m_connectionStatusLabel;
    QPushButton *m_disconnectButton;
};

GenericDeviceConnectionStatus::GenericDeviceConnectionStatus(QBluetoothLocalDevice *local, QWidget *parent)
    : DeviceConnectionStatus(local, parent),
      m_connectionStatusLabel(new QLabel),
      m_disconnectButton(new QPushButton(QObject::tr("Disconnect")))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_connectionStatusLabel);
    layout->addWidget(m_disconnectButton);

    connect(m_disconnectButton, SIGNAL(clicked()),
        SLOT(clickedDisconnect()));
}

void GenericDeviceConnectionStatus::clickedDisconnect()
{
    localDevice()->disconnectRemoteDevice(address());
}

void GenericDeviceConnectionStatus::updateConnectionStatus()
{
    if (isTransportConnected()) {
        QFont f = m_connectionStatusLabel->font();
        f.setBold(true);
        m_connectionStatusLabel->setFont(f);
        m_connectionStatusLabel->setText(QObject::tr("Connected."));
        m_disconnectButton->show();
    } else {
        m_connectionStatusLabel->setFont(QFont());
        m_connectionStatusLabel->setText(QObject::tr("Not connected."));
        m_disconnectButton->hide();
    }
}


//===================================================================

/*
    Displays information about whether the local device has an audio
    (i.e. headset/handsfree) connection to a remote device. Allows user to
    disconnect the audio connection if it is connected, and also disconnect
    the transport connection if it is connected.
*/
class AudioDeviceConnectionStatus : public DeviceConnectionStatus
{
    Q_OBJECT
public:
    AudioDeviceConnectionStatus(QBluetoothLocalDevice *local, QWidget *parent = 0);
    virtual void setRemoteDevice(const QBluetoothAddress &addr);
    virtual void updateConnectionStatus();

protected slots:
    virtual void remoteDeviceDisconnected(const QBluetoothAddress &addr);

private slots:
    void clickedConnectHeadset();
    void clickedConnectHandsFree();
    void clickedDisconnect();

    void audioGatewayConnected(bool success, const QString &msg);
    void remoteAudioDeviceConnected(const QBluetoothAddress &addr);
    void headsetDisconnected();

    void connectAudioGateway(QBluetoothAudioGateway *gateway, int channel);

private:
    QBluetoothAudioGateway *connectedGateway() const;

    QBluetoothAudioGateway *m_headsetGateway;
    QBluetoothAudioGateway *m_handsFreeGateway;

    int m_headsetChannel;
    int m_handsFreeChannel;

    QLabel *m_connectionStatusLabel;
    QPushButton *m_connectHeadsetButton;
    QPushButton *m_connectHandsFreeButton;
    QPushButton *m_disconnectButton;
    QWaitWidget *m_waitWidget;
};

AudioDeviceConnectionStatus::AudioDeviceConnectionStatus(QBluetoothLocalDevice *local, QWidget *parent)
    : DeviceConnectionStatus(local, parent),
      m_headsetGateway(new QBluetoothAudioGateway("BluetoothHeadset", this)),
      m_handsFreeGateway(new QBluetoothAudioGateway("BluetoothHandsfree", this)),
      m_connectionStatusLabel(new QLabel),
      m_connectHeadsetButton(new QPushButton(tr("Connect headset"))),
      m_connectHandsFreeButton(new QPushButton(tr("Connect handsfree unit"))),
      m_disconnectButton(new QPushButton(tr("Disconnect"))),
      m_waitWidget(new QWaitWidget(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(6);
    layout->addWidget(m_connectionStatusLabel);
    layout->addWidget(m_connectHeadsetButton);
    layout->addWidget(m_connectHandsFreeButton);
    layout->addWidget(m_disconnectButton);

    m_connectionStatusLabel->setWordWrap(true);

    connect(m_headsetGateway, SIGNAL(connectResult(bool,QString)),
            SLOT(audioGatewayConnected(bool,QString)));
    connect(m_connectHeadsetButton, SIGNAL(clicked()),
            SLOT(clickedConnectHeadset()));
    connect(m_headsetGateway, SIGNAL(newConnection(QBluetoothAddress)),
            SLOT(remoteAudioDeviceConnected(QBluetoothAddress)));
    connect(m_headsetGateway, SIGNAL(headsetDisconnected()),
            SLOT(headsetDisconnected()));

    connect(m_handsFreeGateway, SIGNAL(connectResult(bool,QString)),
            SLOT(audioGatewayConnected(bool,QString)));
    connect(m_connectHandsFreeButton, SIGNAL(clicked()),
        SLOT(clickedConnectHandsFree()));
    connect(m_handsFreeGateway, SIGNAL(newConnection(QBluetoothAddress)),
            SLOT(remoteAudioDeviceConnected(QBluetoothAddress)));
    connect(m_handsFreeGateway, SIGNAL(headsetDisconnected()),
            SLOT(headsetDisconnected()));

    connect(m_disconnectButton, SIGNAL(clicked()), SLOT(clickedDisconnect()));
}

void AudioDeviceConnectionStatus::setRemoteDevice(const QBluetoothAddress &addr)
{
    DeviceConnectionStatus::setRemoteDevice(addr);

    m_headsetChannel = BtSettings::audioProfileChannel(address(),
            QBluetooth::HeadsetProfile);
    m_handsFreeChannel = BtSettings::audioProfileChannel(address(),
            QBluetooth::HandsFreeProfile);
}

void AudioDeviceConnectionStatus::updateConnectionStatus()
{
    QBluetoothAudioGateway *gateway = connectedGateway();
    if (gateway) {
        if (gateway == m_headsetGateway) {
            m_connectionStatusLabel->setText(tr("Connected to headset."));
        } else if (gateway == m_handsFreeGateway) {
            m_connectionStatusLabel->setText(tr("Connected to handsfree unit."));
        }
        QFont f = m_connectionStatusLabel->font();
        f.setBold(true);
        m_connectionStatusLabel->setFont(f);

        m_connectHeadsetButton->hide();
        m_connectHandsFreeButton->hide();
        m_disconnectButton->show();

    } else {

        // Can't allow connect if another device is using the audio gateway.
        if ( (m_headsetGateway->isConnected() && m_headsetGateway->remotePeer() != address()) ||
              (m_handsFreeGateway->isConnected() && m_handsFreeGateway->remotePeer() != address()) ) {
            m_connectionStatusLabel->setText(QObject::tr(
                    "Connectivity not available. Another device is using Bluetooth audio."));
            m_connectHeadsetButton->hide();
            m_connectHandsFreeButton->hide();
        } else {
            m_connectHeadsetButton->setVisible(m_headsetChannel != -1);
            m_connectHandsFreeButton->setVisible(m_handsFreeChannel != -1);
            m_connectionStatusLabel->setText(QObject::tr("Not connected."));
        }

        m_connectionStatusLabel->setFont(QFont());
        m_disconnectButton->hide();
    }
}

void AudioDeviceConnectionStatus::remoteDeviceDisconnected(const QBluetoothAddress &addr)
{
    DeviceConnectionStatus::remoteDeviceDisconnected(addr);
    m_waitWidget->hide();
}

void AudioDeviceConnectionStatus::clickedConnectHeadset()
{
    connectAudioGateway(m_headsetGateway, m_headsetChannel);
}

void AudioDeviceConnectionStatus::clickedConnectHandsFree()
{
    connectAudioGateway(m_handsFreeGateway, m_handsFreeChannel);
}

void AudioDeviceConnectionStatus::connectAudioGateway(QBluetoothAudioGateway *gateway, int channel)
{
    m_waitWidget->setText(tr("Connecting..."));
    m_waitWidget->setCancelEnabled(true);
    connect(m_waitWidget, SIGNAL(cancelled()), gateway, SLOT(disconnect()));
    m_waitWidget->show();

    gateway->connect(address(), channel);

    qLog(Bluetooth) << "AudioDeviceConnectionStatus: Connecting to audio gateway on"
            << address().toString() << channel;
}

void AudioDeviceConnectionStatus::audioGatewayConnected(bool success, const QString &msg)
{
    qLog(Bluetooth) << "AudioDeviceConnectionStatus: gateway connected" << success << msg;
    updateConnectionStatus();

    if (m_waitWidget->isVisible()) {
        m_waitWidget->hide();
        QBluetoothAudioGateway *gateway = qobject_cast<QBluetoothAudioGateway *>(sender());
        if (gateway) {
            disconnect(m_waitWidget, SIGNAL(cancelled()),
                       gateway, SLOT(disconnect()));
        }

        if (!success) {
            qLog(Bluetooth) << "AudioDeviceConnectionStatus: Headset/Handsfree connection failed:"
                    << msg;
            QMessageBox::warning(this, tr("Error"),
                tr("<qt>Connection failed.</qt>"), QMessageBox::Ok);
        }
    }
}

void AudioDeviceConnectionStatus::clickedDisconnect()
{
    qLog(Bluetooth) << "AudioDeviceConnectionStatus::clickedDisconnect()";

    QBluetoothAudioGateway *gateway = connectedGateway();
    if (gateway) {
        m_waitWidget->setText(tr("Disconnecting..."));
        m_waitWidget->setCancelEnabled(false);
        m_waitWidget->show();
        gateway->disconnect();
    }
}

void AudioDeviceConnectionStatus::remoteAudioDeviceConnected(const QBluetoothAddress &addr)
{
    qLog(Bluetooth) << "AudioDeviceConnectionStatus::remoteAudioDeviceConnected()" << addr.toString();

    // Need to change display even if it wasn't this device that was connected,
    // otherwise connect buttons are enabled even if another device is now
    // using the gateway.
    updateConnectionStatus();
    if (addr == address())
        m_waitWidget->hide();
}

void AudioDeviceConnectionStatus::headsetDisconnected()
{
    qLog(Bluetooth) << "AudioDeviceConnectionStatus::headsetDisconnected()";

    // This is called when any headset is disconnected, not necessarily the
    // one we're looking at.

    // Update status regardless of the device that has disconnected - if some
    // other headset disconnected, then the audio gateway is now available,
    // and the status display needs to change.
    updateConnectionStatus();

    m_waitWidget->hide();
}

QBluetoothAudioGateway *AudioDeviceConnectionStatus::connectedGateway() const
{
    // assuming you won't be connected to more than one gateway at a time
    if (m_headsetGateway->remotePeer() == address())
        return m_headsetGateway;
    if (m_handsFreeGateway->remotePeer() == address())
        return m_handsFreeGateway;
    return 0;
}



//==========================================================================


RemoteDeviceInfoDialog::RemoteDeviceInfoDialog(QBluetoothLocalDevice *local, QWidget *parent, Qt::WFlags flags)
    : QDialog(parent, flags),
      m_ui(0),
      m_local(local),
      m_device(QBluetoothAddress()),
      m_servicesDisplay(0),
      m_audioDeviceConnStatus(0),
      m_genericDeviceConnStatus(0)
{
    m_ui = new Ui::RemoteDeviceInfo();
    m_ui->setupUi(this);

    connect(m_local, SIGNAL(remoteAliasChanged(QBluetoothAddress,QString)),
              SLOT(setTitle()));
    connect(m_local, SIGNAL(remoteAliasRemoved(QBluetoothAddress)),
              SLOT(setTitle()));

    QMenu *menu = QSoftMenuBar::menuFor(this);
    menu->addAction(tr("Other details"), this, SLOT(showMoreInfo()));
    menu->addAction(tr("Supported services"), this, SLOT(showServices()));

    setObjectName("remote_device_info");
    setWindowTitle(tr("Details"));
}

RemoteDeviceInfoDialog::~RemoteDeviceInfoDialog()
{
    delete m_ui;
}

void RemoteDeviceInfoDialog::setRemoteDevice(const QBluetoothAddress &addr)
{
    QBluetoothRemoteDevice device(addr);
    m_local->updateRemoteDevice(device);
    setRemoteDevice(device);
}

static void getDeviceMinorString(const QBluetoothRemoteDevice &device, QString *string)
{
    if (!string)
        return;
    quint8 minor = device.deviceMinor();

    // Return empty string if the minor class is 'Uncategorized'.
    // (not all majors have an 'Uncategorized' minor category)
    switch (device.deviceMajor()) {
        case QBluetooth::Computer:
        case QBluetooth::Phone:
        case QBluetooth::AudioVideo:
        case QBluetooth::Wearable:
        case QBluetooth::Toy:
            if ((minor & 0x3F) == 0)
                return;
            break;
        case QBluetooth::Imaging:
            if (((minor >> 2) & 0xF) == 0)
                return;
            break;
        default:
            break;
    }
    *string = device.deviceMinorAsString();
}

void RemoteDeviceInfoDialog::setRemoteDevice(const QBluetoothRemoteDevice &device)
{
    // set some instance variables
    m_device = device;
    m_address = device.address();
    m_name = device.name();
    if (m_name.isEmpty()) {
        m_nameFontVariant.clear();
    } else {
        m_nameFontVariant = Qtopia::findDisplayFont(m_name);
    }

    // set title
    setTitle();

    // set icon
    QIcon icon = find_device_icon(device);
    if (icon.isNull()) {
        m_ui->icon->hide();
    } else {
        m_ui->icon->setPixmap(icon.pixmap(22));
        m_ui->icon->show();
    }

    // set address
    m_ui->addressEdit->setText(m_address.toString());

    // set type
    QString minor;
    getDeviceMinorString(device, &minor);

    if (minor.isEmpty()) {
        m_ui->typeEdit->setText(device.deviceMajorAsString());
    } else {
        if (minor.startsWith("(")) {
            m_ui->typeEdit->setText(device.deviceMajorAsString() +
                    " " + minor);
        } else {
            m_ui->typeEdit->setText(device.deviceMajorAsString() +
                    " (" + minor + ")");
        }
    }

    // set up connection status box
    int headsetChannel = BtSettings::audioProfileChannel(m_address,
            QBluetooth::HeadsetProfile);
    int handsFreeChannel = BtSettings::audioProfileChannel(m_address,
            QBluetooth::HandsFreeProfile);

    DeviceConnectionStatus *statusWidget;
    if (headsetChannel != -1 || handsFreeChannel != -1) {
        if (!m_audioDeviceConnStatus)
            m_audioDeviceConnStatus = new AudioDeviceConnectionStatus(m_local);
        statusWidget = m_audioDeviceConnStatus;
        if (m_genericDeviceConnStatus)
            m_genericDeviceConnStatus->hide();
    } else {
        if (!m_genericDeviceConnStatus)
            m_genericDeviceConnStatus = new GenericDeviceConnectionStatus(m_local);
        statusWidget = m_genericDeviceConnStatus;
        if (m_audioDeviceConnStatus)
            m_audioDeviceConnStatus->hide();
    }
    statusWidget->setRemoteDevice(m_address);
    statusWidget->updateConnectionStatus();

    if (m_ui->connectionStatus->layout()->indexOf(statusWidget) == -1)
        m_ui->connectionStatus->layout()->addWidget(statusWidget);
    statusWidget->show();
}

void RemoteDeviceInfoDialog::setTitle()
{
    QString alias = m_local->remoteAlias(m_address);
    if (!alias.isEmpty()) {
        m_ui->title->setText(alias);
        return;
    }

    if (m_nameFontVariant.isValid()) {
        QFont f = m_nameFontVariant.value<QFont>();
        f.setBold(true);
        m_ui->title->setFont(f);
        m_ui->title->setText(m_name);
        return;
    }

    m_ui->title->setText(m_address.toString());
}

// similar to details display for local device (in settingdisplay.cpp)
void RemoteDeviceInfoDialog::showMoreInfo()
{
    QDialog *dlg = new QDialog;
    QFormLayout *form = new QFormLayout(dlg);

    QLabel *name = new QLabel(m_name);
    name->setFont(m_nameFontVariant.value<QFont>());
    form->addRow(tr("Name:"), name);
    form->addRow(tr("Version:"), new QLabel(m_device.version()));
    form->addRow(tr("Vendor:"), new QLabel(m_device.manufacturer()));
    form->addRow(tr("Company:"), new QLabel(m_device.company()));
    //form->addWidget(tr("Services:"),
    //               new QLabel(m_device.serviceClassesAsString().join(", ")));

    for (int i=0; i<form->count(); i++) {
        QLabel *label = qobject_cast<QLabel*>(form->itemAt(i)->widget());
        if (label) {
            label->setWordWrap(true);
            if (label->text() == "(null)")  // returned from hcid
                label->setText("");
        }
    }

    dlg->setWindowTitle(tr("Other details"));
    QtopiaApplication::execDialog(dlg);
    delete dlg;
}

void RemoteDeviceInfoDialog::showServices()
{
    if (!m_servicesDisplay)
        m_servicesDisplay = new ServicesDisplay(this);
    m_servicesDisplay->setRemoteDevice(m_address, *m_local);
}

#include "remotedeviceinfodialog.moc"
