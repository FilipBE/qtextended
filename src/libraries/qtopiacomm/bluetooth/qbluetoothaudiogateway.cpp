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

#include <qbluetoothaudiogateway.h>
#include <qbluetoothaddress.h>
#include <QVariant>
#include <QString>

/*!
    \class QBluetoothAudioGateway
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothAudioGateway class provides an interface to a Bluetooth Audio Gateway.
    \ingroup qtopiabluetooth

    The Bluetooth Audio Gateway usually has an audio device associated with it.  Clients can also control speaker and microphone volume of
    the device, connect and disconnect remote clients and find status
    information of the audio gateway.

    The QBluetoothAudioGateway is used to control the Headset and Handsfree
    profile implementations in Qtopia.  Both use a control RFCOMM channel
    and a SCO voice data channel.  This class should be used by client
    applications that wish to control the state of Handsfree / Headset
    implementations (e.g. a Bluetooth Audio settings application.)

    \sa QCommInterface
 */

/*!
    Construct a new audio gateway object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports bluetooth audio gateway.  If there is more
    than one service that supports bluetooth audio gateway, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QBluetoothAudioGateway objects for each.

    \sa QAbstractIpcInterfaceGroup::supports()
 */

QBluetoothAudioGateway::QBluetoothAudioGateway(const QString &service,
QObject *parent,
QAbstractIpcInterface::Mode mode) : QCommInterface("QBluetoothAudioGateway", service, parent, mode)
{
    proxyAll( staticMetaObject );
}

/*!
    Destroys the audio gateway.
*/
QBluetoothAudioGateway::~QBluetoothAudioGateway()
{
}

/*!
    Returns the current speaker volume of the device.  The volume can be between
    0 and 15.

    \sa setSpeakerVolume()
*/
int QBluetoothAudioGateway::speakerVolume() const
{
    return value("SpeakerVolume", 0).toInt();
}

/*!
    Returns the current microphone volume of the device.  The volume can be between
    0 and 15.

    \sa setMicrophoneVolume()
*/
int QBluetoothAudioGateway::microphoneVolume() const
{
    return value("MicrophoneVolume", 0).toInt();
}

/*!
    Returns whether the voice data information is being passed to the device.
    This is true once an SCO connection has been established between the remote
    device (headset) and the audio gateway.  All audio information should now
    be routed over the device associated with this audio gateway.

    \sa isConnected(), remotePeer()
*/
bool QBluetoothAudioGateway::audioEnabled() const
{
    return value("AudioEnabled", false).toBool();
}

/*!
    Returns true if a client is connected.  This is true once an RFCOMM
    control connection has been established between the remote device
    and the audio gateway.

    \sa audioEnabled(), remotePeer()
*/
bool QBluetoothAudioGateway::isConnected() const
{
    return value("IsConnected", false).toBool();
}

/*!
    Returns the address of the connected remote device.  If there is no connection
    an invalid QBluetoothAddress is returned.

    \sa isConnected()
*/
QBluetoothAddress QBluetoothAudioGateway::remotePeer() const
{
    return value("RemotePeer").value<QBluetoothAddress>();
}

/*!
    Attempts to establish an Audio Gateway initiated RFCOMM control connection
    to the headset.  The \a addr contains the address of the remote device
    and \a rfcomm_channel contains the service channel to connect on.

    The connectResult() signal will be sent once the connection succeeds or fails.

    \sa connectResult()
*/
void QBluetoothAudioGateway::connect(const QBluetoothAddress &addr,
                                     int rfcomm_channel)
{
    invoke( SLOT(connect(QBluetoothAddress,int)),
            qVariantFromValue( addr ),
            qVariantFromValue( rfcomm_channel ));
}

/*!
    Disconnect the currently active client from the Audio Gateway.  The
    headsetDisconnected() signal will be sent once the headset is disconnected().

    \sa headsetDisconnected()
*/
void QBluetoothAudioGateway::disconnect()
{
    invoke( SLOT(disconnect()) );
}

/*!
    Requests the Audio Gateway to notify the remote device to update
    its microphone volume to \a volume.  The volume range must be between
    0 and 15.

    \sa microphoneVolume()
*/
void QBluetoothAudioGateway::setMicrophoneVolume(int volume)
{
    invoke( SLOT(setMicrophoneVolume(int)),
            qVariantFromValue(volume));
}

/*!
    Requests the Audio Gateway to notify the remote device to update
    its speaker volume to \a volume.  The volume range must be between
    0 and 15.

    \sa speakerVolume()
 */
void QBluetoothAudioGateway::setSpeakerVolume(int volume)
{
    invoke( SLOT(setSpeakerVolume(int)),
            qVariantFromValue(volume) );
}

/*!
    Asks the Audio Gateway to release the SCO audio data connection.  No audio
    should be routed through the associated audio device.

    \sa connectAudio()
*/
void QBluetoothAudioGateway::releaseAudio()
{
    invoke( SLOT(releaseAudio()) );
}

/*!
    Asks the Audio Gateway to establish an SCO data connection with the currently
    connected peer.  All audio data should now be routed to the associated audio
    device.

    \sa releaseAudio()
*/
void QBluetoothAudioGateway::connectAudio()
{
    invoke( SLOT(connectAudio()) );
}

/*!
    \fn void QBluetoothAudioGateway::connectResult(bool success, const QString &msg)

    This signal is sent after the connect method has been called on the Audio Gateway
    object.  The \a success parameter is true if the connection succeeded, and false
    otherwise.  If the connection failed, the \a msg parameter holds the error
    string.

    \sa connect()
*/

/*!
    \fn void QBluetoothAudioGateway::newConnection(const QBluetoothAddress &addr)

    This signal is sent whenever a client has connected to the Audio Gateway.
    It is only sent on remote device initiated connections.  The \a addr
    parameter holds the address of the connected remote device.

    \sa connect()
*/

/*!
    \fn void QBluetoothAudioGateway::headsetDisconnected()

    This signal is sent whenever a headset has disconnected from the audio gateway.

    \sa disconnect()
*/

/*!
    \fn void QBluetoothAudioGateway::speakerVolumeChanged()

    This signal is sent whenever the microphone volume of the remote device has been
    changed.

    \sa speakerVolume(), setSpeakerVolume()
*/

/*!
    \fn void QBluetoothAudioGateway::microphoneVolumeChanged()

    This signal is sent whenever the microphone volume of the remote device has been
    changed.

    \sa microphoneVolume(), setMicrophoneVolume()
 */

/*!
    \fn void QBluetoothAudioGateway::audioStateChanged()

    This signal is emitted whenever the state of the audio data stream has changed.
    E.g. the audio stream (SCO connection) has been disconnected or connected.  Use audioEnabled() to find out the state of the
    audio stream.

    \sa audioEnabled()
 */
