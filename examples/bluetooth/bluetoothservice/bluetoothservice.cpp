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

#include "bluetoothservice.h"

#include <QBluetoothAbstractService>
#include <QBluetoothRfcommSocket>
#include <QBluetoothRfcommServer>
#include <QBluetoothSdpRecord>
#include <QBluetoothLocalDevice>
#include <QBluetoothAddress>
#include <QBluetoothServiceController>
#include <Qtopia>
#include <QtopiaApplication>

#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QDebug>


class BluetoothSerialPortExampleService : public QBluetoothAbstractService
{
    Q_OBJECT

public:
    BluetoothSerialPortExampleService(QObject *parent = 0);
    virtual ~BluetoothSerialPortExampleService();

    virtual void start();
    virtual void stop();
    virtual void setSecurityOptions(QBluetooth::SecurityOptions options);

private slots:
    void newRfcommConnection();

private:
    bool startRfcommServer(int rfcommChannel);

    QBluetooth::SecurityOptions m_securityOptions;
    quint32 m_sdpRecordHandle;
    QBluetoothRfcommServer *m_rfcommServer;
};

BluetoothSerialPortExampleService::BluetoothSerialPortExampleService(QObject *parent)
    : QBluetoothAbstractService("SerialPortExampleService", tr("Serial Port Example"), parent)
{
    m_securityOptions = 0;
    m_sdpRecordHandle = 0;
    m_rfcommServer = 0;
}

BluetoothSerialPortExampleService::~BluetoothSerialPortExampleService()
{
}

void BluetoothSerialPortExampleService::start()
{
    if (!m_rfcommServer)
        m_rfcommServer = new QBluetoothRfcommServer(this);

    // Read the SDP record that we want to register for this service.
    QFile file(Qtopia::qtopiaDir() + "/etc/bluetooth/sdp/SerialPortSDPRecord.xml");
    if (!file.exists()) {
        emit started(true, tr("Cannot find SDP record file"));
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        emit started(true, tr("Error reading SDP record file"));
        return;
    }
    QBluetoothSdpRecord record = QBluetoothSdpRecord::fromDevice(&file);
    file.close();

    // QBluetoothAbstractService::registerRecord() registers a SDP record and
    // returns the service record handle for the newly registered service.
    m_sdpRecordHandle = registerRecord(record);
    if (m_sdpRecordHandle == 0) {
        emit started(true, tr("Error registering the SDP service"));
        return;
    }

    int rfcommChannel = QBluetoothSdpRecord::rfcommChannel(record);

    if (!startRfcommServer(rfcommChannel)) {
        // The server could not be started, so clean up and unregister the
        // SDP record using QBluetoothAbstractService::unregisterRecord().
        unregisterRecord(m_sdpRecordHandle);
        m_sdpRecordHandle = 0;

        emit started(true, tr("Error starting RFCOMM server"));
        return;
    }

    qDebug() << "Started example Bluetooth service";
    emit started(false, QString());
}

void BluetoothSerialPortExampleService::stop()
{
    if (m_rfcommServer) {
        m_rfcommServer->close();
        delete m_rfcommServer;
        m_rfcommServer = 0;
    }

    if (!unregisterRecord(m_sdpRecordHandle))
        qDebug() << "Error unregistering the SDP service";
    m_sdpRecordHandle = 0;

    qDebug() << "Stopped example Bluetooth service";
    emit stopped();
}

void BluetoothSerialPortExampleService::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    if (m_rfcommServer)
        m_rfcommServer->setSecurityOptions(options);
    m_securityOptions = options;
}

bool BluetoothSerialPortExampleService::startRfcommServer(int rfcommChannel)
{
    if (!m_rfcommServer->setSecurityOptions(m_securityOptions)) {
        qDebug() << "Error setting security options";
        return false;
    }

    QBluetoothLocalDevice localDevice;
    if (!m_rfcommServer->listen(localDevice.address(), rfcommChannel)) {
        qDebug() << "Error listening on server";
        return false;
    }

    connect(m_rfcommServer, SIGNAL(newConnection()),
            this, SLOT(newRfcommConnection()));
    return true;
}

void BluetoothSerialPortExampleService::newRfcommConnection()
{
    QBluetoothRfcommSocket *socket =
            qobject_cast<QBluetoothRfcommSocket *>(m_rfcommServer->nextPendingConnection());
    if (socket) {
        QByteArray greeting("hello, world!");
        socket->write(greeting);
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        socket->disconnect();
    }
}


//=================================================================


/*
    Normally, Bluetooth services in Qtopia are created as server tasks using
    the QTOPIA_TASK macro from QtopiaServerApplication, so that the service
    will run as a background task.

    However, since this example is running from an application instead of as
    a server task, we will use QtopiaApplication::registerRunningTask() to
    register the service as a task, so that it will continue to run after
    you exit the example application.
*/
BluetoothService::BluetoothService(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f)
{
    // This is the string used as the service name in the
    // BluetoothSerialPortExampleService constructor.
    QString serviceName = "SerialPortExampleService";

    // Create the service if it has not already been created (if this
    // application has previously been launched).
    QBluetoothServiceController controller;
    if (!controller.services().contains(serviceName)) {

        BluetoothSerialPortExampleService *service = new BluetoothSerialPortExampleService;
        Q_UNUSED(service);   // silence compiler warning

        // Register the service as a task so it will continue to run after the
        // application is closed.
        QtopiaApplication::instance()->registerRunningTask(
                "BluetoothServiceExample", this);
    }

    QLabel *label = new QLabel(tr("The <b>%1</b> service has been created.<P>Go to Settings -> Bluetooth, open the context menu and click 'My services' to modify the settings for this service.").arg(controller.displayName(serviceName)));

    label->setWordWrap(true);
    setCentralWidget(label);
}

BluetoothService::~BluetoothService()
{
}

#include "bluetoothservice.moc"
