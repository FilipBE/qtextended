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
#include <qmediartpsession.h>
#include <qmediartpsessionengine.h>

#include <QHostAddress>
#include <QPluginManager>
#include <QStringList>
#include <QSize>

class QMediaRtpStreamPrivate
{
public:
    QMediaRtpStream::Type type;
    QMediaRtpStream::Direction direction;
    QHostAddress outboundAddress;
    qint16 outboundPort;
    QMediaRtpStream::State outboundState;
    QHostAddress inboundAddress;
    qint16 inboundPort;
    QMediaRtpStream::State inboundState;
    QMediaRtpPayload inboundPayload;
    QString outboundErrorString;
    QString inboundErrorString;
};

/*!
    \class QMediaRtpStream
    \inpublicgroup QtMediaModule
    \brief The QMediaRtpStream class manages a single stream in an RTP session.

    A QMediaRtpStream is composed of an inbound data stream, an outbound data stream or a combined
    inbound and outbound stream.  The inbound and outbound connections are established separately
    so it is possible for example to start a stream listening for incoming data and start sending
    sometime later.

    The payload of an outbound stream must be set with setOutboundPayload() before it can be
    connected.

    The payload of an inbound streams is determined from the stream data.  When the payload changes
    the stream will emit the inboundPayloadChanged() signal.

    RTP streams can be created using QMediaRtpSession.
*/

/*!
    \enum QMediaRtpStream::Type
    Identifies the type of an RTP stream.

    \value Audio The stream is an audio stream.
    \value Video The stream is a video stream.
*/

/*!
    \enum QMediaRtpStream::Direction
    Identifies the direction of an RTP stream.

    \value SendOnly The stream sends data.
    \value ReceiveOnly The stream receives data.
    \value SendReceive The stream both sends and receives data.
*/

/*!
    \enum QMediaRtpStream::State
     Identifies the current state of an RTP stream.

    \value Connecting The stream is in the process of establishing a connection.
    \value Connected The stream is connected.
    \value Disconnecting The stream is in the process of tearing down a connection.
    \value Disconnected The stream is disconnected.
*/

/*!
    Constructs a \a type RTP stream with the given \a parent which sends and/or receives data
    according to \a direction.
*/
QMediaRtpStream::QMediaRtpStream(Type type, Direction direction, QObject *parent)
    : QObject(parent)
    , d(new QMediaRtpStreamPrivate)
{
    d->type = type;
    d->direction = direction;
    d->outboundPort = 0;
    d->outboundState = Disconnected;
    d->inboundPort = 0;
    d->inboundState = Disconnected;
}

/*!
    Destroys a stream.
*/
QMediaRtpStream::~QMediaRtpStream()
{
    delete d;
}

/*!
    Returns the type of a stream.
*/
QMediaRtpStream::Type QMediaRtpStream::type() const
{
    return d->type;
}

/*!
    Returns the direction of a stream.
*/
QMediaRtpStream::Direction QMediaRtpStream::direction() const
{
    return d->direction;
}

/*!
    Returns the address the outbound stream is sending to.
*/
QHostAddress QMediaRtpStream::outboundAddress() const
{
    return d->outboundAddress;
}

/*!
    Returns the port number the outbound stream is sending to.
*/
qint16 QMediaRtpStream::outboundPort() const
{
    return d->outboundPort;
}

/*!
    Returns the current state of the outbound stream.
*/
QMediaRtpStream::State QMediaRtpStream::outboundState() const
{
    return d->outboundState;
}

/*!
    \fn QMediaRtpStream::connectOutbound(const QHostAddress &address, qint16 port)

    Starts the outbound stream sending data to a host with the given \a address and \a port.

    \sa outboundAddress(), outboundPort(), outboundState(), disconnectOutbound(), connectInbound()
*/

/*!
    \fn QMediaRtpStream::disconnectOutbound()

    Stops the outbound stream sending data.
*/

/*!
    Returns the address the inbound stream is receiving data at.
*/
QHostAddress QMediaRtpStream::inboundAddress() const
{
    return d->inboundAddress;
}

/*!
    Returns the port number the inbound stream is receiving data on.
*/
qint16 QMediaRtpStream::inboundPort() const
{
    return d->inboundPort;
}

/*!
    Returns the state of the inbound stream.
*/
QMediaRtpStream::State QMediaRtpStream::inboundState() const
{
    return d->inboundState;
}

/*!
    \fn QMediaRtpStream::connectInbound(const QHostAddress &address, qint16 port)

    Starts the inbound stream listening for data on the given \a address and \a port.

    \sa inboundAddress(), inboundPort(), inboundState(), disconnectInbound(), connectOutbound()
*/

/*!
    \fn QMediaRtpStream::disconnectInbound()

    Stops the inbound stream receiving data.
*/

/*!
    Sets the \a address the outbound stream sends data to.
*/
void QMediaRtpStream::setOutboundAddress(const QHostAddress &address)
{
    d->inboundAddress = address;
}

/*!
    Sets the \a port the outbound stream sends data to.
*/
void QMediaRtpStream::setOutboundPort(qint16 port)
{
    d->inboundPort = port;
}

/*!
    Sets the \a state of the outbound stream.

    This will emit the outboundStateChanged() signal.
*/
void QMediaRtpStream::setOutboundState(State state)
{
    if (d->outboundState != state) {
        d->outboundState = state;

        emit outboundStateChanged(state);

        switch (state) {
        case Connected:
            emit outboundConnected();
            break;
        case Disconnected:
            emit outboundDisconnected();
            break;
        default:
            break;
        }
    }
}

/*!
    Sets the \a address the inbound stream listens for data at.
*/
void QMediaRtpStream::setInboundAddress(const QHostAddress &address)
{
    d->inboundAddress = address;
}

/*!
    Sets the \a port the inbound stream listens for data on.
*/
void QMediaRtpStream::setInboundPort(qint16 port)
{
    d->inboundPort = port;
}

/*!
    Sets the current \a state of the inbound stream.

    This emits the inboundStateChanged() signal.
*/
void QMediaRtpStream::setInboundState(State state)
{
    if (d->inboundState != state) {
        d->inboundState = state;

        emit inboundStateChanged(state);

        switch (state) {
        case Connected:
            emit inboundConnected();
            break;
        case Disconnected:
            emit inboundDisconnected();
            break;
        default:
            break;
        }
    }
}

/*!
    Returns a string describing the last error to occur to the outbound stream.
*/
QString QMediaRtpStream::outboundErrorString() const
{
    return d->outboundErrorString;
}

/*!
    Sets a string describing the last \a error to occur the the outbound stream.
*/
void QMediaRtpStream::setOutboundError(const QString &error)
{
    d->inboundErrorString = error;

    if (!error.isNull())
        emit outboundError(error);
}

/*!
    \fn QMediaRtpStream::outboundError(const QString &error)

    Signals that an \a error occurred in the outbound stream.
*/

/*!
    Returns a string describing the last error to occur to the inbound stream.
*/
QString QMediaRtpStream::inboundErrorString() const
{
    return d->inboundErrorString;
}

/*!
    Sets a string describing the last \a error to the inbound stream.

    This will emit the inboundError() signal.
*/
void QMediaRtpStream::setInboundError(const QString &error)
{
    d->outboundErrorString = error;

    if (!error.isNull())
        emit inboundError(error);
}

/*!
    \fn QMediaRtpStream::inboundError(const QString &error)

    Signals that an \a error occurred in the inbound stream.
*/

/*!
    \fn QMediaRtpStream::outboundPayload() const

    Returns the payload used for outbound data.
*/

/*!
    \fn QMediaRtpStream::setOutboundPayload(const QMediaRtpPayload &payload)

    Sets the \a payload to used for outbound data.

    Returns true if the payload could be set and false otherwise.

    Setting the payload will fail if the session does not support any equivalent outbound
    payload.

    \sa QMediaRtpSession::supportedOutboundPayloads(), QMediaRtpPayload::isEquivalent()
*/

/*!
    Returns the payload of the inbound stream.
*/
QMediaRtpPayload QMediaRtpStream::inboundPayload() const
{
    return d->inboundPayload;
}

/*!
    Sets the \a payload of the inbound stream.
*/
void QMediaRtpStream::setInboundPayload(const QMediaRtpPayload &payload)
{
    d->inboundPayload = payload;

    emit inboundPayloadChanged(payload);
}

/*!
    \fn QMediaRtpStream::inboundPayloads() const

    Returns a list of payloads the inbound stream can expect to receive.
*/

/*!
    \fn QMediaRtpStream::setInboundPayloads(const QList<QMediaRtpPayload> &payloads)

    Sets a list of \a payloads the inbound stream can expect to receive.
*/

/*!
    \fn QMediaRtpStream::inboundPayloadChanged(const QMediaRtpPayload &payload)

    Signals that the \a payload of the incoming stream has changed.
*/

/*!
    \fn QMediaRtpStream::outboundStateChanged(QMediaRtpStream::State state)

    Signals that the \a state of the outbound stream has changed.
*/

/*!
    \fn QMediaRtpStream::outboundConnected()

    Signals that the outbound streamm has been connected.
*/

/*!
    \fn QMediaRtpStream::outboundDisconnected()

    Signals that the outbound stream has been disconnected.
*/

/*!
    \fn QMediaRtpStream::inboundStateChanged(QMediaRtpStream::State state)

    Signals that the \a state of the inbound stream has changed.
*/

/*!
    \fn QMediaRtpStream::inboundConnected()

    Signals that the inbound stream has been connected.
*/

/*!
    \fn QMediaRtpStream::inboundDisconnected()

    Signals that the inbound stream has been disconnected.
*/

/*!
    \class QMediaRtpAudioStream
    \inpublicgroup QtMediaModule
    \brief The QMediaRtpAudioStream class manages a single audio stream in an RTP session.

    DTMF events may be sent over an audio stream as either tones added to the audio signal, or as
    separate RTP packets.  To send DTMF events as RTP packets a valid payload with the encoding
    name \c telephone-event must be set with setTelephonyEventPayload().  If no such payload is set
    the DTMF tones will be added to the audio stream instead.  If a stream supports DTMF RTP
    packets then a \c telephone-event payload will be included in the list supported audio payloads
    given by the QMediaRtpSession that created it.
*/

/*!
    \enum QMediaRtpAudioStream::DtmfTone

    The event codes for DTMF tones.

    \value Dtmf_0
    \value Dtmf_1
    \value Dtmf_2
    \value Dtmf_3
    \value Dtmf_4
    \value Dtmf_5
    \value Dtmf_6
    \value Dtmf_7
    \value Dtmf_8
    \value Dtmf_9
    \value Dtmf_Star
    \value Dtmf_Pound
    \value Dtmf_A
    \value Dtmf_B
    \value Dtmf_C
    \value Dtmf_D
*/

/*!
    Contructs a new RTP audio stream with the given \a parent which sends and/or receives data
    according to \a direction.
*/
QMediaRtpAudioStream::QMediaRtpAudioStream(Direction direction, QObject *parent)
    : QMediaRtpStream(Audio, direction, parent)
{
}

/*!
    Destroys an RTP audio stream.
*/
QMediaRtpAudioStream::~QMediaRtpAudioStream()
{
}


/*!
    \fn QMediaRtpAudioStream::telephonyEventPayload() const

    Returns the payload used for sending telephony events.
*/

/*!
    \fn QMediaRtpAudioStream::setTelephonyEventPayload(const QMediaRtpPayload &payload)

    Sets the \a payload to be used for sending telephony events.

    Returns true if the payload was successfully changed and false otherwise.
*/

/*!
    \fn QMediaRtpAudioStream::startTelephonyEvent(int event, int volume)

    Starts sending a telephony \a event at the given \a volume on a stream.

    Note: Telephony events can only be sent on Audio streams.

    \sa stopTelephonyEvent()
*/

/*!
    \fn QMediaRtpAudioStream::stopTelephonyEvent()

    Stops sending a telephony event.

    \sa startTelephonyEvent()
*/

/*!
    \class QMediaRtpVideoStream
    \inpublicgroup QtMediaModule
    \brief The QMediaRtpVideoStream class manages a single video stream in an RTP session.

    Outbound video data is sourced from a QCameraDevice set with the setCamera() function.
    Depending on the payload the video stream may not be able to accept all video formats output
    by the camera, a list of formats the stream can accept for a payload is given by the
    supportedVideoFormats() function.

    Inbound video data is rendered to a QVideoSurface given by the videoSurface() function.
*/

/*!
    Contructs a new RTP video stream with the given \a parent which sends and/or receives data
    according to \a direction.
*/
QMediaRtpVideoStream::QMediaRtpVideoStream(Direction direction, QObject *parent)
    : QMediaRtpStream(Video, direction, parent)
{
}

/*!
    Destroys an RTP video stream.
*/
QMediaRtpVideoStream::~QMediaRtpVideoStream()
{
}

/*!
    \fn QMediaRtpVideoStream::supportedVideoFormats(const QMediaRtpPayload &payload) const

    Returns a list of video formats that an RTP stream can accept as input for a \a payload.
*/

/*!
    \fn QMediaRtpVideoStream::camera() const

    Returns the camera the RTP stream sources video data from.
*/


/*!
    \fn QMediaRtpVideoStream::setCamera(QCameraPreviewCapture *camera)

    Sets the \a camera the RTP stream sources video data from.
*/

/*!
    \fn QMediaRtpVideoStream::videoSurface() const

    Returns the video surface received video data is painted to.
*/

class QMediaRtpSessionPrivate
{
public:
    QMediaRtpSessionPrivate() : pluginManager("mediaengines"), engine(0){}

    QPluginManager pluginManager;
    QMediaRtpEngine *engine;
};

/*!
    \class QMediaRtpSession
    \inpublicgroup QtMediaModule
    \brief The QMediaRtpSession class provides a means establishing remote media connections.

    An RTP session manages a set of related RTP streams, it provides functions for creating streams
    and managing their lifetimes.

    The RTP session also provides information about the payloads supported by the stream provider.
    A list of payloads that can be received for a stream type is given by supportedInboundPayloads()
    and a similar list for payloads that can be sent is given by supportedOutboundPayloads().
*/

/*!
    Creates new RTP session with the given \a parent.
*/
QMediaRtpSession::QMediaRtpSession(QObject *parent)
    : QObject(parent)
    , d(new QMediaRtpSessionPrivate)
{
    foreach (QString plugin, d->pluginManager.list()) {
        QObject *instance = d->pluginManager.instance(plugin);

        if(QMediaRtpEngineFactory *factory = qobject_cast<QMediaRtpEngineFactory *>(instance)) {
            d->engine = factory->createRtpEngine();

            break;
        }
        delete instance;
    }
}

/*!
    Destroys a session.
*/
QMediaRtpSession::~QMediaRtpSession()
{
    delete d->engine;
    delete d;
}

/*!
    Returns a list of \a type payloads a session is able to receive.
*/
QList<QMediaRtpPayload> QMediaRtpSession::supportedInboundPayloads(QMediaRtpStream::Type type) const
{
    return d->engine
            ? d->engine->supportedInboundPayloads(type)
            : QList<QMediaRtpPayload>();
}

/*!
    Returns a list of \a type payloads a session is able receive.
*/
QList<QMediaRtpPayload> QMediaRtpSession::supportedOutboundPayloads(QMediaRtpStream::Type type) const
{
    return d->engine
            ? d->engine->supportedOutboundPayloads(type)
            : QList<QMediaRtpPayload>();
}

/*!
    Returns the number of streams in the session.
*/
int QMediaRtpSession::streamCount() const
{
    return d->engine
        ? d->engine->streamCount()
        : 0;
}

/*!
    Returns the stream at \a index.
*/
QMediaRtpStream *QMediaRtpSession::stream(int index) const
{
    return d->engine
            ? d->engine->stream(index)
            : 0;
}

/*!
    Adds a new \a type stream to the session which sends and/or receives data according to
    \a direction.

    Returns the new stream, or a null pointer if the stream could not be created.
*/
QMediaRtpStream *QMediaRtpSession::addStream(QMediaRtpStream::Type type, QMediaRtpStream::Direction direction)
{
    return d->engine
            ? d->engine->addStream(type, direction)
            : 0;
}

/*!
    Adds an new audio stream to the session which sends and/or receives data according to
    \a direction.

    Returns the new stream, or a null pointer if the stream could not be created.
*/
QMediaRtpAudioStream *QMediaRtpSession::addAudioStream(QMediaRtpStream::Direction direction)
{
    return static_cast<QMediaRtpAudioStream *>(addStream(QMediaRtpStream::Audio, direction));
}

/*!
    Adds an new video stream to the session which sends and/or receives data according to
    \a direction.

    Returns the new stream, or a null pointer if the stream could not be created.
*/
QMediaRtpVideoStream *QMediaRtpSession::addVideoStream(QMediaRtpStream::Direction direction)
{
    return static_cast<QMediaRtpVideoStream *>(addStream(QMediaRtpStream::Video, direction));
}

/*!
    Removes a \a stream from the session.
*/
void QMediaRtpSession::removeStream(QMediaRtpStream *stream)
{
    if (d->engine)
        d->engine->removeStream(stream);
}

class QMediaRtpPayloadPrivate : public QSharedData
{
public:
    QMediaRtpPayloadPrivate()
        : id(-1)
        , type(QMediaRtpStream::Audio)
        , clockRate(0)
        , channels(0)
    {
        ref.ref();
    }

    QMediaRtpPayloadPrivate(
            int id_, QMediaRtpStream::Type type_, const QString &encodingName_, int clockRate_,
            int channels_, const QMap< QString, QString > &parameters_)
        : id(id_)
        , type(type_)
        , encodingName(encodingName_)
        , clockRate(clockRate_)
        , channels(channels_)
        , parameters(parameters_)
    {
    }

    int id;
    QMediaRtpStream::Type type;
    QString encodingName;
    int clockRate;
    int channels;
    QMap< QString, QString > parameters;
};

Q_GLOBAL_STATIC(QMediaRtpPayloadPrivate,qMediaRtpPayloadPrivateSharedNull);

/*!
    \class QMediaRtpPayload
    \inpublicgroup QtMediaModule
    \brief The QMediaRtpPayload class describes an RTP payload type.

    Payloads describe the encoding of RTP streams.  They can be divided into two categories; well
    know payloads with IDs between 0 and 95, and undefined payloads with IDs between 96 and 127.

    The encoding information associated with well known payload IDs is pre-defined and often no
    more information is given.  If the ID is supported by an RTP provider it already knows the
    encoding and so the payload can be identified on the basis of the ID alone.

    Undefined payloads are allocated their IDs dynamically and so the ID alone cannot be used to
    identify the payload.  In this case the full details of the payload must be shared and the
    payload ID is simply used in the RTP stream to identify a shared payload.  The isEquivalent()
    function can be used to determine if two payloads with different IDs refer to the same
    encoding.
*/

/*!
    Constructs a null RTP payload descriptor.
*/
QMediaRtpPayload::QMediaRtpPayload()
    : d(qMediaRtpPayloadPrivateSharedNull())
{
}

/*!
    Constructs an RTP payload descriptor for a well known payload with the given \a id and stream
    \a type.

    The \a id of a well known type must be between 0 and 95.
*/
QMediaRtpPayload::QMediaRtpPayload(int id, QMediaRtpStream::Type type)
    : d(new QMediaRtpPayloadPrivate(id, type, QString(), 0, 0, QMap<QString, QString>()))
{
}

/*!
    Constructs an RTP payload descriptor for a payload with a given \a id, stream \a type,
    \a encodingName, \a clockRate, and number of \a channels.
*/
QMediaRtpPayload::QMediaRtpPayload(
        int id, QMediaRtpStream::Type type, const QString &encodingName, int clockRate, int channels)
    : d(new QMediaRtpPayloadPrivate(id, type, encodingName, clockRate, channels, 
                                    QMap<QString, QString>()))
{
}

/*!
    Constructs an RTP payload descriptor for a payload with a given \a id, stream \a type,
    \a encodingName, \a clockRate, number of \a channels and a list of type specific \a parameters.
*/
QMediaRtpPayload::QMediaRtpPayload(
        int id, QMediaRtpStream::Type type, const QString &encodingName, int clockRate, int channels,
        const QMap< QString, QString > &parameters)
    : d(new QMediaRtpPayloadPrivate(id, type, encodingName, clockRate, channels, parameters))
{
}

/*!
    Constructs a copy of the the RTP payload descriptor \a other.
*/
QMediaRtpPayload::QMediaRtpPayload(const QMediaRtpPayload &other)
    : d(other.d)
{
}

/*!
    Assigns the value of \a other to an RTP payload descriptor.
*/
QMediaRtpPayload &QMediaRtpPayload::operator =(const QMediaRtpPayload &other)
{
    d = other.d;

    return *this;
}

/*!
    Destroys an RTP payload descriptor.
*/
QMediaRtpPayload::~QMediaRtpPayload()
{
}

/*!
    Compares \a other to an RTP payload descriptor and return true if they are equal and false
    otherwise.

    This may fail for descriptors representing the same payload but with conflicting fields, ie
    comparison between well known payloads with incomplete and complete sets of parameters, or
    comparison of dynamic payloads with different payload ids).  To determine if two descriptors
    represent the same payload use isEquivalent() instead.

    \sa isEquivalent()
*/
bool QMediaRtpPayload::operator ==(const QMediaRtpPayload &other) const
{
    return d == other.d || (d->id == other.d->id
        && d->type == other.d->type
        && d->encodingName.compare(other.d->encodingName, Qt::CaseInsensitive) == 0
        && d->clockRate == other.d->clockRate
        && d->channels == other.d->channels
        && d->parameters == other.d->parameters);
}

/*!
    Compares \a other to an RTP payload descriptor and return true if they are not equal and false
    otherwise.

    This may fail for descriptors representing the same payload but with conflicting fields, ie
    comparison between well known payloads with incomplete and complete sets of parameters, or
    comparison of dynamic payloads with different payload ids).  To determine if two descriptors
    represent the same payload use isEquivalent() instead.

    \sa isEquivalent()
*/
bool QMediaRtpPayload::operator !=(const QMediaRtpPayload &other) const
{
    return d != other.d && (d->id != other.d->id
        || d->type != other.d->type
        && d->encodingName.compare(other.d->encodingName, Qt::CaseInsensitive) != 0
        || d->clockRate != other.d->clockRate
        || d->channels != other.d->channels
        || d->parameters != other.d->parameters);
}

/*!
    Compares \a other to an RTP payload descriptor and return true if they represent the same 
    payload and false otherwise.

    Two descriptors represent the same payload of they share a common well known payload id()
    (<= 95) or both have a dynamic payload id() (>95) and all their parameters are equal.
*/
bool QMediaRtpPayload::isEquivalent(const QMediaRtpPayload &other) const
{
    return d == other.d || (d->id < 96
        ? (d->id           == other.d->id
        && d->type         == other.d->type)
        : (d->type         == other.d->type
        && d->encodingName.compare(other.d->encodingName, Qt::CaseInsensitive) == 0
        && d->clockRate    == other.d->clockRate
        && d->channels     == other.d->channels
        && d->parameters   == other.d->parameters));
}

/*!
    Returns true if the payload is null and false otherwise.
*/
bool QMediaRtpPayload::isNull() const
{
    return d == qMediaRtpPayloadPrivateSharedNull();
}

/*!
    Returns the id of the payload.
*/
int QMediaRtpPayload::id() const
{
    return d->id;
}

/*!
    Returns the stream type of the payload.
*/
QMediaRtpStream::Type QMediaRtpPayload::type() const
{
    return d->type;
}

/*!
    Returns the encoding name of the payload.
*/
QString QMediaRtpPayload::encodingName() const
{
    return d->encodingName;
}

/*!
    Returns the clock rate of the payload.
*/
int QMediaRtpPayload::clockRate() const
{
    return d->clockRate;
}

/*!
    Returns the number of channels of the payload or 0 if none has been specified.
*/
int QMediaRtpPayload::channels() const
{
    return d->channels;
}

/*!
    Returns a list of the payload's additional parameter keys.
*/
QList< QString > QMediaRtpPayload::parametersKeys() const
{
    return d->parameters.keys();
}

/*!
    Returns the additional payload parameter value associated with the given \a key.
*/
QString QMediaRtpPayload::parameter(const QString &key) const
{
    return d->parameters.value(key);
}

/*!
    Returns all the additional payload parameters.
*/
QMap< QString, QString > QMediaRtpPayload::parameters() const
{
    return d->parameters;
}

/*!
    Returns a list of payload descriptors that are common to both \a localPayloads and
    \a remotePayloads.

    Where a two payloads are equivalent but not equal the remote payload is returned.
*/
QList<QMediaRtpPayload> QMediaRtpPayload::commonPayloads(
        const QList<QMediaRtpPayload> &localPayloads, const QList<QMediaRtpPayload> &remotePayloads)
{
    QList<QMediaRtpPayload> commonPayloads;

    foreach (QMediaRtpPayload remotePayload, remotePayloads) {
        foreach (QMediaRtpPayload localPayload, localPayloads) {
            if (localPayload.isEquivalent(remotePayload)) {
                commonPayloads.append( remotePayload );

                break;
            }
        }
    }
    return commonPayloads;
}

/*!
    \fn QMediaRtpPayload::serialize(Stream &stream) const
    \internal
*/
template <typename Stream> void QMediaRtpPayload::serialize(Stream &stream) const
{
    stream
        << d->id
        << d->type
        << d->encodingName
        << d->clockRate
        << d->channels
        << d->parameters;
}

/*!
    \fn QMediaRtpPayload::deserialize(Stream &stream)
    \internal
*/
template <typename Stream> void QMediaRtpPayload::deserialize(Stream &stream)
{
    int id;
    QMediaRtpStream::Type type;
    QString encodingName;
    int clockRate;
    int channels;
    QMap< QString, QString > parameters;

    stream >> id
           >> type
           >> encodingName
           >> clockRate
           >> channels
           >> parameters;

    d = id != -1
        ? new QMediaRtpPayloadPrivate(id, type, encodingName, clockRate, channels, parameters)
        : qMediaRtpPayloadPrivateSharedNull();
}

/*!
    \internal
*/
QDebug operator <<(QDebug debug, const QMediaRtpPayload &payload)
{
    debug << '('
          << "ID:" << payload.id()
          << "Type:" << (payload.type() == QMediaRtpStream::Audio ? "audio" : "video")
          << "Encoding Name:" << payload.encodingName()
          << "Clock Rate:" << payload.clockRate()
          << "Channels:" << payload.channels()
          << "Parameters:" << payload.parameters()
          << ')';

    return debug;
}


Q_IMPLEMENT_USER_METATYPE(QMediaRtpPayload)
Q_IMPLEMENT_USER_METATYPE_ENUM(QMediaRtpStream::Type)
