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

#include <qtopiacomm/qbluetoothaudiogateway.h>
#include "qbluetoothhfservice_p.h"
#include "qbluetoothhfagserver_p.h"
#include <qbluetoothrfcommserver.h>
#include <qbluetoothrfcommsocket.h>
#include <qbluetoothrfcommserialport.h>
#include <qbluetoothscosocket.h>
#include <qbluetoothscoserver.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothsdprecord.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <qtopialog.h>
#include <qbluetoothaddress.h>
#include <qvaluespace.h>
#include <qcommdevicesession.h>
#include <QStringList>

#include <bluetooth/bluetooth.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "scomisc_p.h"

class HandsfreeIpcAdaptor : public QtopiaIpcAdaptor
{
    Q_OBJECT
    friend class QBluetoothHandsfreeService;

public:
    HandsfreeIpcAdaptor(QBluetoothHandsfreeService *parent);

public slots:
    void speakerVolumeChanged(int volume);
    void microphoneVolumeChanged(int volume);

signals:
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);

private:
    QBluetoothHandsfreeService *m_parent;
};

HandsfreeIpcAdaptor::HandsfreeIpcAdaptor(QBluetoothHandsfreeService *parent)
    : QtopiaIpcAdaptor("QPE/BluetoothHandsfree", parent)
{
    m_parent = parent;
    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
}

void HandsfreeIpcAdaptor::speakerVolumeChanged(int volume)
{
    m_parent->updateSpeakerVolume(volume);
}

void HandsfreeIpcAdaptor::microphoneVolumeChanged(int volume)
{
    m_parent->updateMicrophoneVolume(volume);
}

class QBluetoothHandsfreeServicePrivate
{
public:
    QBluetoothRfcommServer *m_server;
    QBluetooth::SecurityOptions m_securityOptions;
    quint32 m_sdpRecordHandle;

    QBluetoothScoServer *m_scoServer;

    QBluetoothHandsfreeCommInterface *m_interface;

    QBluetoothScoSocket *m_scoSocket;
    int m_scofd;

    bool m_connectInProgress;
    QBluetoothRfcommSerialPort *m_activeClient;
    QBluetoothRfcommSocket *m_client;

    int m_speakerVolume;
    int m_microphoneVolume;
    HandsfreeIpcAdaptor *m_adaptor;

    QBluetoothAddress m_remotePeer;

    void *m_audioDev;

    QValueSpaceItem *m_serialPorts;

    QCommDeviceSession *m_session;
    QBluetoothAddress m_addr;
    int m_channel;
};

/*!
    \class QBluetoothHandsfreeService
    \inpublicgroup QtBluetoothModule
    \brief The QBluetoothHandsfreeService class implements Bluetooth Handsfree Audio Gateway profile.
    \ingroup QtopiaServer::Task::Bluetooth

    Bluetooth Handsfree profile provides mechanisms for basic
    phone call control and audio transfer between the Audio Gateway
    (phone) and the Bluetooth Audio headset.  This class
    implements the Bluetooth Handsfree Audio Gateway as defined
    in the Handsfree Bluetooth Profile specification.

    This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.
*/

/*!
    Construct a new Handsfree service with service name given by
    \a service.  The display string to use is given by \a displayName
    and QObject \a parent.
*/
QBluetoothHandsfreeService::QBluetoothHandsfreeService(const QString &service, const QString &displayName, QObject *parent)
    : QBluetoothAbstractService(service, displayName, parent)
{
    m_data = new QBluetoothHandsfreeServicePrivate();

    m_data->m_server = new QBluetoothRfcommServer(this);
    m_data->m_server->setMaxPendingConnections(1);
    m_data->m_securityOptions = 0;
    QObject::connect(m_data->m_server, SIGNAL(newConnection()),
                     this, SLOT(newConnection()));

    m_data->m_scoServer = new QBluetoothScoServer(this);
    m_data->m_scoServer->setMaxPendingConnections(1);
    QObject::connect(m_data->m_scoServer, SIGNAL(newConnection()),
                     this, SLOT(newScoConnection()));

    m_data->m_speakerVolume = 0;
    m_data->m_microphoneVolume = 0;

    m_data->m_scoSocket = 0;
    m_data->m_scofd = -1;

    m_data->m_client = 0;
    m_data->m_activeClient = 0;
    m_data->m_connectInProgress = false;

    m_data->m_adaptor = new HandsfreeIpcAdaptor(this);

    m_data->m_serialPorts = new QValueSpaceItem("/Communications/ModemEmulator");
    QObject::connect(m_data->m_serialPorts, SIGNAL(contentsChanged()),
                     this, SLOT(serialPortsChanged()));

    m_data->m_session = 0;
    m_data->m_interface = 0;
    m_data->m_audioDev = 0;
}

/*!
    Destructor.
*/
QBluetoothHandsfreeService::~QBluetoothHandsfreeService()
{
    if (m_data) {
        if (m_data->m_audioDev)
            bt_sco_close(m_data->m_audioDev);
        delete m_data->m_server;
        delete m_data->m_scoServer;
        delete m_data->m_scoSocket;
        delete m_data->m_client;
        delete m_data->m_activeClient;
        delete m_data->m_adaptor;
        delete m_data->m_serialPorts;
        delete m_data->m_session;

        delete m_data;
    }
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::serialPortsChanged()
{
    if (!m_data->m_activeClient)
        return;

    QStringList serialPorts = m_data->m_serialPorts->value("serialPorts").toStringList();
    qLog(Bluetooth) << "SerialPorts now: " << serialPorts;

    if (!serialPorts.contains(m_data->m_activeClient->device())) {
        doDisconnect();
    }
}

/*!
    \internal
  Attempts to initialize the audio gateway by registering with
  the BT SCO handler.
  */
void QBluetoothHandsfreeService::initializeAudioGateway()
{
    if ( m_data->m_interface )
        return;

    QByteArray audioDev = find_btsco_device("Handsfree");

    if (audioDev.isEmpty()) {
        qLog(Bluetooth) << "No handsfree audio devices available...";
    }
    else if (!bt_sco_open(&m_data->m_audioDev, audioDev.constData())) {
        qWarning("Unable to open audio device: %s", audioDev.constData());
    }

    m_data->m_interface = new QBluetoothHandsfreeCommInterface(audioDev, this);
    m_data->m_interface->initialize();
}

/*!
    \reimp
*/
void QBluetoothHandsfreeService::start()
{
    qLog(Bluetooth) << "QBluetoothHandsfreeService::start";

    if (m_data->m_server->isListening()) {
        emit started(true, tr("Handsfree Audio Gateway already running."));
        return;
    }

    m_data->m_sdpRecordHandle = 0;
    QBluetoothSdpRecord sdpRecord;

    // register the SDP service
    QFile sdpRecordFile(Qtopia::qtopiaDir() + "etc/bluetooth/sdp/hfag.xml");
    if (sdpRecordFile.open(QIODevice::ReadOnly)) {
        sdpRecord = QBluetoothSdpRecord::fromDevice(&sdpRecordFile);
        if (!sdpRecord.isNull())
            m_data->m_sdpRecordHandle = registerRecord(sdpRecord);
    }

    if (sdpRecord.isNull())
        qWarning() << "QBluetoothHandsfreeService: cannot read" << sdpRecordFile.fileName();

    if (m_data->m_sdpRecordHandle == 0) {
        emit started(true, tr("Error registering with SDP server"));
        return;
    }

    initializeAudioGateway();

    if (!m_data->m_server->listen(QBluetoothAddress::any,
                QBluetoothSdpRecord::rfcommChannel(sdpRecord))) {
        unregisterRecord(m_data->m_sdpRecordHandle);
        emit started(true, tr("Could not listen on channel."));
        return;
    }

    m_data->m_server->setSecurityOptions(m_data->m_securityOptions);

    if (!m_data->m_scoServer->listen(QBluetoothAddress::any)) {
        // This is not fatal, we can still establish client connections..
        qWarning("Handsfree Service - Could not listen on the SCO socket.");
    }

    emit started(false, QString());
}

/*!
    \reimp
*/
void QBluetoothHandsfreeService::stop()
{
    qLog(Bluetooth) << "QBluetoothHandsfreeService::stop";

    if (m_data->m_server->isListening()) {
        m_data->m_server->close();
        m_data->m_scoServer->close();

        disconnect();
    }

    if (!unregisterRecord(m_data->m_sdpRecordHandle))
        qLog(Bluetooth) << "QBluetoothHandsfreeService::stop() error unregistering SDP service";

    emit stopped();
}

/*!
    \reimp
*/
void QBluetoothHandsfreeService::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    m_data->m_securityOptions = options;
    if (m_data->m_server->isListening()) {
        m_data->m_server->setSecurityOptions(options);
    }
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::sessionOpen()
{
    if (m_data->m_connectInProgress) {
        QBluetoothRfcommSocket *socket = new QBluetoothRfcommSocket(this);
        hookupSocket(socket);

        bool ret = m_data->m_client->connect(QBluetoothAddress::any,
                                            m_data->m_addr, m_data->m_channel);

        if (!ret) {
            delete m_data->m_client;
            m_data->m_client = 0;
            m_data->m_session->endSession();
            m_data->m_connectInProgress = false;
            emit connectResult(false, tr("Connect failed."));
            return;
        }
    }
}

/*!
    \internal
            m_data->m_connectInProgress = false;
*/
void QBluetoothHandsfreeService::sessionFailed()
{
    if (m_data->m_connectInProgress) {
        m_data->m_connectInProgress = false;
        emit connectResult(false, tr("Bluetooth Session could not be started"));
        return;
    }
}

/*!
    This method is the concrete implementation of the
    QBluetoothAudioGateway interface method of the same name.
    It is called from the QBluetoothHandsfreeAudioGatewayServer
    class, which acts as a forwarding agent.

    The address and channel to connect to are given by \a addr
    and \a rfcomm_channel respectively.

    \sa QBluetoothHandsfreeAudioGatewayServer
*/
void QBluetoothHandsfreeService::connect(const QBluetoothAddress &addr,
                                         int rfcomm_channel)
{
    // If the service is stop, deny connect requests
    if (!m_data->m_server->isListening()) {
        emit connectResult(false, tr("Service not available."));
        return;
    }

    // If we're still connecting or disconnecting, return
    if (m_data->m_connectInProgress) {
        emit connectResult(false, tr("Connection in progress."));
        return;
    }

    // If we're connected but haven't created tty yet,
    // return, caller should call disconnect first
    if (m_data->m_client &&
        (m_data->m_client->state() == QBluetoothRfcommSocket::ConnectedState)) {
        emit connectResult(false, tr("Already connected."));
        return;
    }

    // If we're connected as a tty, return.
    if (m_data->m_activeClient) {
        emit connectResult(false, tr("Already connected."));
        return;
    }

    m_data->m_connectInProgress = true;
    m_data->m_addr = addr;
    m_data->m_channel = rfcomm_channel;

    qLog(Bluetooth) << "Starting Bluetooth session for handsfree..";

    if (!m_data->m_session) {
        qLog(Bluetooth) << "Lazy initializing a new QCommDeviceSession";
        QBluetoothLocalDevice localDevice;
        m_data->m_session = new QCommDeviceSession(localDevice.deviceName().toLatin1());
        QObject::connect(m_data->m_session, SIGNAL(sessionOpen()), this, SLOT(sessionOpen()));
        QObject::connect(m_data->m_session, SIGNAL(sessionFailed()), this, SLOT(sessionFailed()));
    }

    m_data->m_session->startSession();
}

/*!
    This method is the concrete implementation of the
    QBluetoothAudioGateway interface method of the same name.
    It is called from the QBluetoothHandsfreeAudioGatewayServer
    class, which acts as a forwarding agent.

    \sa QBluetoothHandsfreeAudioGatewayServer
*/
void QBluetoothHandsfreeService::disconnect()
{
    // We are connecting/have connected, but not setup the tty yet
    if (m_data->m_client &&
        (m_data->m_client->state() == QBluetoothRfcommSocket::ConnectedState ||
        m_data->m_client->state() == QBluetoothRfcommSocket::ConnectingState))
    {
        m_data->m_connectInProgress = false;
        m_data->m_client->disconnect();
        delete m_data->m_client;
        m_data->m_client = 0;
        m_data->m_session->endSession();
        emit disconnected();
        return;
    }

    if (!m_data->m_activeClient) {
        m_data->m_interface->setValue("IsConnected", false);
        m_data->m_remotePeer = QBluetoothAddress::invalid;
        m_data->m_interface->setValue("RemotePeer",
                                      QVariant::fromValue(m_data->m_remotePeer));
        return;
    }

    QtopiaServiceRequest req( "ModemEmulator", "removeSerialPort(QString)" );
    req << m_data->m_activeClient->device();
    if (!req.send()) {
        qWarning("QBluetoothHandsfreeService: unable to send removeSerialPort() request to ModemEmulator, ensure atinterface is available");
    }
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::doDisconnect()
{
    releaseAudio();
    qLog(Bluetooth) << "Ending Bluetooth Session for Handsfree.";
    m_data->m_session->endSession();

    delete m_data->m_activeClient;
    m_data->m_activeClient = 0;

    m_data->m_interface->setValue("IsConnected", false);
    m_data->m_remotePeer = QBluetoothAddress::invalid;
    m_data->m_interface->setValue("RemotePeer",
                                  QVariant::fromValue(m_data->m_remotePeer));
    emit disconnected();
}

/*!
    This method is the concrete implementation of the
    QBluetoothAudioGateway interface method of the same name.
    It is called from the QBluetoothHandsfreeAudioGatewayServer
    class, which acts as a forwarding agent.

    Instructs the Bluetooth device to set the speaker volume to
    \a volume.

    \sa QBluetoothHandsfreeAudioGatewayServer
*/
void QBluetoothHandsfreeService::setSpeakerVolume(int volume)
{
    if (!m_data->m_activeClient || volume == m_data->m_speakerVolume)
        return;

    emit m_data->m_adaptor->setSpeakerVolume(volume);
    updateSpeakerVolume(volume);
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::updateSpeakerVolume(int volume)
{
    m_data->m_speakerVolume = volume;
    m_data->m_interface->setValue("SpeakerVolume", volume);
    emit speakerVolumeChanged();
}

/*!
    This method is the concrete implementation of the
    QBluetoothAudioGateway interface method of the same name.
    It is called from the QBluetoothHandsfreeAudioGatewayServer
    class, which acts as a forwarding agent.

    Instructs the Bluetooth device to set the microphone volume to
    \a volume.

    \sa QBluetoothHandsfreeAudioGatewayServer
*/
void QBluetoothHandsfreeService::setMicrophoneVolume(int volume)
{
    if (!m_data->m_activeClient || volume == m_data->m_microphoneVolume)
        return;

    emit m_data->m_adaptor->setMicrophoneVolume(volume);
    updateMicrophoneVolume(volume);
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::updateMicrophoneVolume(int volume)
{
    m_data->m_microphoneVolume = volume;
    m_data->m_interface->setValue("MicrophoneVolume", volume);
    emit microphoneVolumeChanged();
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::scoStateChanged(QBluetoothAbstractSocket::SocketState socketState)
{
    switch (socketState) {
        case QBluetoothRfcommSocket::ConnectingState:
            break;
        case QBluetoothRfcommSocket::ConnectedState:
            scoConnectionEstablished(m_data->m_scoSocket);
            m_data->m_scoSocket = 0;
            break;
        case QBluetoothRfcommSocket::ClosingState:
            break;
        case QBluetoothRfcommSocket::UnconnectedState:
            // This happens on a failed connect
            if (m_data->m_audioDev)
                bt_sco_set_fd(m_data->m_audioDev, -1);

            delete m_data->m_scoSocket;
            m_data->m_scoSocket = 0;

            m_data->m_interface->setValue("AudioEnabled", false);
            emit audioStateChanged();
            break;
        default:
            break;
    };
}

/*!
    This method is the concrete implementation of the
    QBluetoothAudioGateway interface method of the same name.
    It is called from the QBluetoothHandsfreeAudioGatewayServer
    class, which acts as a forwarding agent.

    \sa QBluetoothHandsfreeAudioGatewayServer
*/
void QBluetoothHandsfreeService::releaseAudio()
{
    if (m_data->m_scoSocket) {
        QObject::disconnect(m_data->m_scoSocket,
                            SIGNAL(stateChanged(QBluetoothAbstractSocket::SocketState)),
                            this, SLOT(scoStateChanged(QBluetoothAbstractSocket::SocketState)));

        delete m_data->m_scoSocket;
        m_data->m_scoSocket = 0;
    }

    if (m_data->m_scofd == -1)
        return;

    if (m_data->m_audioDev)
        bt_sco_set_fd(m_data->m_audioDev, -1);
    close(m_data->m_scofd);
    m_data->m_scofd = -1;

    m_data->m_interface->setValue("AudioEnabled", false);
    emit audioStateChanged();
}

/*!
    This method is the concrete implementation of the
    QBluetoothAudioGateway interface method of the same name.
    It is called from the QBluetoothHandsfreeAudioGatewayServer
    class, which acts as a forwarding agent.

    \sa QBluetoothHandsfreeAudioGatewayServer
*/
void QBluetoothHandsfreeService::connectAudio()
{
    doConnectAudio();
}

/*!
    \internal
*/
bool QBluetoothHandsfreeService::doConnectAudio()
{
    if (!m_data->m_activeClient) {
        qWarning("No control connection, ignoring.");
        return false;
    }

    if (m_data->m_scofd != -1 || m_data->m_scoSocket != 0) {
        qWarning("Already connected.");
        return false;
    }

    if (!canConnectAudio()) {
        return false;
    }

    m_data->m_scoSocket = new QBluetoothScoSocket(this);
    QObject::connect(m_data->m_scoSocket,
                     SIGNAL(stateChanged(QBluetoothAbstractSocket::SocketState)),
                     this, SLOT(scoStateChanged(QBluetoothAbstractSocket::SocketState)));

    bool ret = m_data->m_scoSocket->connect(QBluetoothAddress::any,
                                            m_data->m_remotePeer);

    if (ret == false) {
        delete m_data->m_scoSocket;
        m_data->m_scoSocket = 0;
    }

    return ret;
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::newConnection()
{
    // New client has connected
    QBluetoothRfcommSocket *socket =
        static_cast<QBluetoothRfcommSocket *>(m_data->m_server->nextPendingConnection());

    if (m_data->m_activeClient || (m_data->m_client &&
        (m_data->m_connectInProgress ||
        (m_data->m_client->state() == QBluetoothRfcommSocket::ConnectedState)))) {
        socket->close();
        delete socket;
        return;
    }

    qLog(Bluetooth) << "Starting session for incoming Handsfree connection.";

    if (!m_data->m_session) {
        qLog(Bluetooth) << "Lazy initializing the QCommDeviceSession object";
        QBluetoothLocalDevice localDevice;
        m_data->m_session = new QCommDeviceSession(localDevice.deviceName().toLatin1());
        QObject::connect(m_data->m_session, SIGNAL(sessionOpen()), this, SLOT(sessionOpen()));
        QObject::connect(m_data->m_session, SIGNAL(sessionFailed()), this, SLOT(sessionFailed()));
    }

    setupTty(socket, true);

    if (m_data->m_activeClient)
        m_data->m_session->startSession();
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::error(QBluetoothAbstractSocket::SocketError)
{
    if (!m_data->m_connectInProgress) {
        qWarning("Unknown error occrred in handsfree service");
    }
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::stateChanged(QBluetoothAbstractSocket::SocketState socketState)
{
    switch (socketState) {
        case QBluetoothRfcommSocket::ConnectingState:
            break;

        case QBluetoothRfcommSocket::ConnectedState:
            setupTty(m_data->m_client, false);
            m_data->m_client = 0;
            if (!m_data->m_activeClient)
                m_data->m_session->endSession();
            break;

        case QBluetoothRfcommSocket::ClosingState:
            break;

        case QBluetoothRfcommSocket::UnconnectedState:
            // We only need to catch connection failures here
            if (m_data->m_connectInProgress) {
                m_data->m_connectInProgress = false;
                emit connectResult(false, "Connection failed.");
                m_data->m_client->deleteLater();
                m_data->m_client = 0;
                m_data->m_session->endSession();
            }
            break;

        default:
            break;
    };
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::hookupSocket(QBluetoothRfcommSocket *socket)
{
    m_data->m_client = socket;
    QObject::connect(m_data->m_client, SIGNAL(stateChanged(QBluetoothAbstractSocket::SocketState)),
                     this, SLOT(stateChanged(QBluetoothAbstractSocket::SocketState)));
    QObject::connect(m_data->m_client, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
                     this, SLOT(error(QBluetoothAbstractSocket::SocketError)));
}

/*!
    \internal
*/
bool QBluetoothHandsfreeService::setupTty(QBluetoothRfcommSocket *socket, bool incoming)
{
    m_data->m_connectInProgress = false;
    QBluetoothAddress remoteAddr = socket->remoteAddress();

    qLog(Bluetooth) << "Creating tty device";
    m_data->m_activeClient = new QBluetoothRfcommSerialPort( socket, 0, this );
    QString dev = m_data->m_activeClient->device();
    qLog(Bluetooth) << "CreateTty returned: " << dev;

    if ( !dev.isEmpty() ) {
        QBluetoothAddress socketAddress = socket->remoteAddress();

        // Need to ensure we close the socket before handing it off to the Modem Emulator
        delete socket;

        QtopiaServiceRequest req( "ModemEmulator", "addSerialPort(QString,QString)" );
        req << m_data->m_activeClient->device() << "handsfree,noecho";
        if (!req.send()) {
            qWarning("QBluetoothHandsfreeService: unable to send addSerialPort() request to ModemEmulator, ensure atinterface is available");
            return false;
        }

        qLog(Bluetooth) << "Handsfree connected to" << socketAddress.toString();

        m_data->m_interface->setValue("IsConnected", true);
        m_data->m_remotePeer = socketAddress;
        m_data->m_interface->setValue("RemotePeer",
                                      QVariant::fromValue(m_data->m_remotePeer));

        if (incoming)
            emit newConnection(socketAddress);
        else
            emit connectResult(true, QString());

        return true;
    }

    delete socket;
    delete m_data->m_activeClient;
    m_data->m_activeClient = 0;
    qWarning("Could not create tty device!!");

    if (!incoming)
        emit connectResult(false, "Unable to setup TTY port");

    return false;
}

/*!
    \internal
*/
bool QBluetoothHandsfreeService::canConnectAudio()
{
    return true;
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::scoConnectionEstablished(QBluetoothScoSocket *socket)
{
    // This is gonna be ugly...
    m_data->m_scofd = dup(socket->socketDescriptor());
    ::fcntl(m_data->m_scofd, F_SETFD, FD_CLOEXEC);

    // Disconnect the signals
    socket->QObject::disconnect();
    delete socket;

    if (m_data->m_audioDev)
        bt_sco_set_fd(m_data->m_audioDev, m_data->m_scofd);
    m_data->m_interface->setValue("AudioEnabled", true);
    emit audioStateChanged();
}

/*!
    \internal
*/
void QBluetoothHandsfreeService::newScoConnection()
{
    qLog(Bluetooth) << "QBluetoothHandsfreeService::New Sco Connection hint";

    // New client has connected
    QBluetoothScoSocket *socket =
        static_cast<QBluetoothScoSocket *>(m_data->m_scoServer->nextPendingConnection());

    if ((m_data->m_scofd != -1) || (m_data->m_scoSocket)) {
        qWarning("Voice connection already active, closing");
        delete socket;
        return;
    }

    if (!m_data->m_activeClient) {
        qWarning("No active Handsfree connections.  Voice connection invalid.");
        delete socket;
        return;
    }

    if (m_data->m_remotePeer != socket->remoteAddress()) {
        qWarning("SCO connection initiated from a client different than the currently active client");
        delete socket;
        return;
    }

    scoConnectionEstablished(socket);
}

/*!
    \fn void QBluetoothHandsfreeService::connectResult(bool success, const QString &msg);
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeService::newConnection(const QBluetoothAddress &addr);
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeService::disconnected();
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeService::speakerVolumeChanged();
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeService::microphoneVolumeChanged();
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeService::audioStateChanged();
    \internal
*/

//==========================================================

class QBluetoothHandsfreeCommInterfacePrivate
{
public:
    QBluetoothHandsfreeService *m_service;
    QBluetoothHandsfreeAudioGatewayServer *m_gatewayServer;
};

/*!
    \class QBluetoothHandsfreeCommInterface
    \inpublicgroup QtBluetoothModule
    \brief The QBluetoothHandsfreeCommInterface class provides a Qt Extended IPC interface to the Handsfree Audio Gateway profile implementation.

    The QBluetoothHandsfreeCommInterface extends
    the QAbstractIpcInterfaceGroup class.  It adds the an
    implementation of the QBluetoothAudioGateway interface
    which forwards all calls to the implementation object,
    which is passed in the constructor.

    This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.
*/

/*!
    Constructs a new Handsfree Interface group.  The audio
    device to use is given by \a audioDev.  The implementation
    object is given by \a parent.
*/
QBluetoothHandsfreeCommInterface::QBluetoothHandsfreeCommInterface(const QByteArray &audioDev, QBluetoothHandsfreeService *parent)
    : QAbstractIpcInterfaceGroup(parent->name(), parent),
      m_data(new QBluetoothHandsfreeCommInterfacePrivate)
{
    m_data->m_service = parent;
    m_data->m_gatewayServer = new QBluetoothHandsfreeAudioGatewayServer(this, audioDev,
            parent->name());

    QObject::connect(parent, SIGNAL(connectResult(bool,QString)),
                     SIGNAL(connectResult(bool,QString)));
    QObject::connect(parent, SIGNAL(newConnection(QBluetoothAddress)),
                     SIGNAL(newConnection(QBluetoothAddress)));
    QObject::connect(parent, SIGNAL(disconnected()),
                     SIGNAL(disconnected()));
    QObject::connect(parent, SIGNAL(speakerVolumeChanged()),
                     SIGNAL(speakerVolumeChanged()));
    QObject::connect(parent, SIGNAL(microphoneVolumeChanged()),
                     SIGNAL(microphoneVolumeChanged()));
    QObject::connect(parent, SIGNAL(audioStateChanged()),
                     SIGNAL(audioStateChanged()));
}

/*!
    Destructor.
*/
QBluetoothHandsfreeCommInterface::~QBluetoothHandsfreeCommInterface()
{
    delete m_data;
}

/*!
    \reimp
*/
void QBluetoothHandsfreeCommInterface::initialize()
{
    if ( !supports<QBluetoothAudioGateway>() )
        addInterface(m_data->m_gatewayServer);
}

/*!
    \internal
*/
void QBluetoothHandsfreeCommInterface::setValue(const QString &key, const QVariant &value)
{
    m_data->m_gatewayServer->setValue(key, value);
}

/*!
    \internal
*/
void QBluetoothHandsfreeCommInterface::connect(const QBluetoothAddress &addr, int rfcomm_channel)
{
    m_data->m_service->connect(addr, rfcomm_channel);
}

/*!
    \internal
*/
void QBluetoothHandsfreeCommInterface::disconnect()
{
    m_data->m_service->disconnect();
}

/*!
    \internal
*/
void QBluetoothHandsfreeCommInterface::setSpeakerVolume(int volume)
{
    m_data->m_service->setSpeakerVolume(volume);
}

/*!
    \internal
*/
void QBluetoothHandsfreeCommInterface::setMicrophoneVolume(int volume)
{
    m_data->m_service->setMicrophoneVolume(volume);
}

/*!
    \internal
*/
void QBluetoothHandsfreeCommInterface::releaseAudio()
{
    m_data->m_service->releaseAudio();
}

/*!
    \internal
*/
void QBluetoothHandsfreeCommInterface::connectAudio()
{
    m_data->m_service->connectAudio();
}

/*!
    \fn void QBluetoothHandsfreeCommInterface::connectResult(bool success, const QString &msg);
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeCommInterface::newConnection(const QBluetoothAddress &addr);
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeCommInterface::disconnected();
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeCommInterface::speakerVolumeChanged();
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeCommInterface::microphoneVolumeChanged();
    \internal
*/

/*!
    \fn void QBluetoothHandsfreeCommInterface::audioStateChanged();
    \internal
*/

#include "qbluetoothhfservice.moc"
