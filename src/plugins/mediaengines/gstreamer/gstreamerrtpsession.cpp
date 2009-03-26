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

#include "gstreamerrtpsession.h"

#include "gstreamerqtopiacamerasource.h"
#include <QCameraDevice>
#include <QCameraDeviceLoader>
#include <QHostAddress>
#include <QTimerEvent>
#include <QSettings>
#include <QtDebug>
#include <qtopialog.h>
#include <qcamera.h>

namespace gstreamer
{

static GstElement *createGstElement(GstElement *bin, GstElementFactory *factory)
{
    GstElement *element = gst_element_factory_create(factory, 0);

    if (element && !gst_bin_add(GST_BIN(bin), element)) {
        gst_object_unref(GST_OBJECT(element));

        element = 0;
    }
    return element;
}

static GstElement *createGstElement(GstElement *bin, const char *name)
{
    GstElement *element = gst_element_factory_make(name, 0);

    if (element && !gst_bin_add(GST_BIN(bin), element)) {
        gst_object_unref(GST_OBJECT(element));

        element = 0;
    }
    return element;
}

static GstElement *createGstElement(GstElement *bin, GType type)
{
    GstElement *element = GST_ELEMENT(g_object_new(type, 0));

    if (element && !gst_bin_add(GST_BIN(bin), element)) {
        gst_object_unref(GST_OBJECT(element));

        element = 0;
    }
    return element;
}

class OutboundPayloadFactory
{
    Q_DISABLE_COPY(OutboundPayloadFactory)
public:
    OutboundPayloadFactory(GstElementFactory *encoderFactory, GstElementFactory *payloaderFactory, const QMediaRtpPayload &payload)
        : m_encoderFactory(encoderFactory)
        , m_payloaderFactory(payloaderFactory)
        , m_payload(payload)
    {
    }

    ~OutboundPayloadFactory()
    {
        gst_object_unref(GST_OBJECT(m_encoderFactory));
        gst_object_unref(GST_OBJECT(m_payloaderFactory));
    }

    GstElementFactory *encoderFactory(){ return m_encoderFactory; }
    GstElementFactory *payloaderFactory(){ return m_payloaderFactory; }
    QMediaRtpPayload payload() const{ return m_payload; }

    GstElement *createBin(GstElement *pipeline, int payloadId);

    static OutboundPayloadFactory *createFactory(const QString &key, QSettings *conf);

private:
    GstElementFactory *m_encoderFactory;
    GstElementFactory *m_payloaderFactory;
    QMediaRtpPayload m_payload;
};


class InboundPayloadFactory
{
    Q_DISABLE_COPY(InboundPayloadFactory)
public:
    InboundPayloadFactory(GstElementFactory *decoderFactory, GstElementFactory *depayloaderFactory, const QMediaRtpPayload &payload)
        : m_decoderFactory(decoderFactory)
        , m_depayloaderFactory(depayloaderFactory)
        , m_payload(payload)
    {
    }

    ~InboundPayloadFactory()
    {
        gst_object_unref(GST_OBJECT(m_decoderFactory));
        gst_object_unref(GST_OBJECT(m_depayloaderFactory));
    }

    GstElementFactory *decoderFactory(){ return m_decoderFactory; }
    GstElementFactory *depayloaderFactory(){ return m_depayloaderFactory; }
    QMediaRtpPayload payload() const{ return m_payload; }

    GstElement *createBin(GstElement *pipeline);

    static InboundPayloadFactory *createFactory(const QString &key, QSettings *conf);

private:
    GstElementFactory *m_decoderFactory;
    GstElementFactory *m_depayloaderFactory;
    QMediaRtpPayload m_payload;
};


GstElement *OutboundPayloadFactory::createBin(GstElement *pipeline, int payloadId)
{
    GstElement *bin = gst_bin_new(0);

    GstElement *encoder = 0;
    GstElement *payloader = 0;

    if (!gst_bin_add(GST_BIN(pipeline), bin)) {
        gst_object_unref(GST_OBJECT(bin));
    } else {
        if (!(encoder = createGstElement(bin, m_encoderFactory))) {
            qWarning()
                    << "Failed to create factory element"
                    << gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(m_encoderFactory));
        } else if (!(payloader = createGstElement(bin, m_payloaderFactory))) {
            qWarning()
                    << "Failed to create factory element"
                    << gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(m_payloaderFactory));
        } else if (!gst_element_link(encoder, payloader)) {
            qWarning()
                    << "Failed to link elements"
                    << gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(m_encoderFactory))
                    << gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(m_payloaderFactory));
        } else {
            g_object_set(payloader, "pt", payloadId, NULL);

            gst_element_add_pad(
                    bin, gst_ghost_pad_new("sink", gst_element_get_pad(encoder, "sink")));
            gst_element_add_pad(
                    bin, gst_ghost_pad_new("src", gst_element_get_pad(payloader, "src")));

            return bin;
        }
        gst_bin_remove(GST_BIN(pipeline), bin);
    }
    return 0;
}

OutboundPayloadFactory *OutboundPayloadFactory::createFactory(const QString &key, QSettings *conf)
{
    bool ok = false;

    int id = key.toInt(&ok);

    if (!ok)
        return 0;

    QString typeString;
    QString encodingName;
    int clockRate = 0;
    int channels = 0;
    QMap<QString,QString> parameters;
    QString encoderName;
    QString payloaderName;

    foreach (QString key, conf->childKeys()) {
        if (key == QLatin1String("Type"))
            typeString = conf->value(key).toString();
        else if (key == QLatin1String("EncodingName"))
            encodingName = conf->value(key).toString();
        else if (key == QLatin1String("ClockRate"))
            clockRate = conf->value(key).toInt();
        else if (key == QLatin1String("Channels"))
            channels = conf->value(key).toInt();
        else if (key == QLatin1String("Encoder"))
            encoderName = conf->value(key).toString();
        else if (key == QLatin1String("Payloader"))
            payloaderName = conf->value(key).toString();
        else
            parameters.insert(key, conf->value(key).toString());
    }

    if (encoderName.isEmpty() || payloaderName.isEmpty())
        return 0;

    QMediaRtpStream::Type type;

    if (typeString == QLatin1String("Audio"))
        type = QMediaRtpStream::Audio;
    else if (typeString == QLatin1String("Video"))
        type = QMediaRtpStream::Video;
    else
        return 0;

    GstElementFactory *encoder = gst_element_factory_find(encoderName.toLatin1().constData());

    if (!encoder)
        return 0;

    GstElementFactory *payloader = gst_element_factory_find(payloaderName.toLatin1().constData());

    if (payloader) {
        return new OutboundPayloadFactory(
            encoder,
            payloader,
            QMediaRtpPayload(id, type, encodingName, clockRate, channels, parameters));
    } else {
        gst_object_unref( GST_OBJECT(encoder) );
    }

    return 0;
}

GstElement *InboundPayloadFactory::createBin(GstElement *pipeline)
{
    GstElement *bin = gst_bin_new(0);

    GstElement *decoder = 0;
    GstElement *depayloader = 0;

    if (!gst_bin_add(GST_BIN(pipeline), bin)) {
        gst_object_unref(GST_OBJECT(bin));
    } else {
        if (!(decoder = createGstElement(bin, m_decoderFactory))) {
            qWarning()
                    << "Failed to create factory element"
                    << gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(m_decoderFactory));
        } else if (!(depayloader = createGstElement(bin, m_depayloaderFactory))) {
            qWarning()
                    << "Failed to create factory element"
                    << gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(m_depayloaderFactory));
        } else if (!gst_element_link(depayloader, decoder)) {
            qWarning()
                    << "Failed to link elements"
                    << gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(m_depayloaderFactory))
                    << gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(m_decoderFactory));
        } else {
            gst_element_add_pad(
                    bin, gst_ghost_pad_new("sink", gst_element_get_pad(depayloader, "sink")));
            gst_element_add_pad(
                    bin, gst_ghost_pad_new("src", gst_element_get_pad( decoder, "src")));

            return bin;
        }
        gst_bin_remove(GST_BIN(pipeline), bin);
    }
    return 0;
}

InboundPayloadFactory *InboundPayloadFactory::createFactory(const QString &key, QSettings *conf)
{
    bool ok = false;

    int id = key.toInt(&ok);

    if (!ok)
        return 0;

    QString typeString;
    QString encodingName;
    int clockRate = 0;
    int channels = 0;
    QMap<QString,QString> parameters;
    QString decoderName;
    QString depayloaderName;

    foreach (QString key, conf->childKeys()) {
        if (key == QLatin1String("Type"))
            typeString = conf->value(key).toString();
        else if (key == QLatin1String("EncodingName"))
            encodingName = conf->value(key).toString();
        else if (key == QLatin1String("ClockRate"))
            clockRate = conf->value(key).toInt();
        else if (key == QLatin1String("Channels"))
            channels = conf->value(key).toInt();
        else if (key == QLatin1String("Decoder"))
            decoderName = conf->value(key).toString();
        else if (key == QLatin1String("Depayloader"))
            depayloaderName = conf->value(key).toString();
        else
            parameters.insert(key, conf->value(key).toString());
    }

    if (decoderName.isEmpty() || depayloaderName.isEmpty())
        return 0;

    QMediaRtpStream::Type type;

    if (typeString == QLatin1String("Audio"))
        type = QMediaRtpStream::Audio;
    else if (typeString == QLatin1String("Video"))
        type = QMediaRtpStream::Video;
    else
        return 0;

    GstElementFactory *decoder = gst_element_factory_find(decoderName.toLatin1().constData());

    if (!decoder)
        return 0;

    GstElementFactory *depayloader = gst_element_factory_find(depayloaderName.toLatin1().constData());

    if (depayloader) {
        return new InboundPayloadFactory(
            decoder,
            depayloader,
            QMediaRtpPayload(id, type, encodingName, clockRate, channels, parameters));
    } else {
        gst_object_unref( GST_OBJECT(decoder) );
    }

    return 0;
}

RtpStream::RtpStream()
{
}

RtpStream::~RtpStream()
{
}

void RtpStream::setInboundFactories(const QList<InboundPayloadFactory *> &factories)
{
    m_inboundFactories = factories;
}

void RtpStream::setOutboundFactories(const QList<OutboundPayloadFactory *> &factories)
{
    m_outboundFactories = factories;
}

GstCaps *RtpStream::requestPtMap(GstElement *demux, guint pt, QMediaRtpStream *stream)
{
    Q_UNUSED(demux);

    foreach (QMediaRtpPayload payload, stream->inboundPayloads())
        if (payload.id() == int(pt))
            return createCaps(payload);

    return 0;
}

GstCaps *RtpStream::createCaps(const QMediaRtpPayload &payload)
{
    GstStructure *structure = gst_structure_new(
            "application/x-rtp",
            "pt", G_TYPE_INT, payload.id(),
            "media", G_TYPE_STRING, (payload.type() == QMediaRtpStream::Audio ? "audio" : "video"),
            "encoding-name",  G_TYPE_STRING, payload.encodingName().toLatin1().constData(),
            NULL );

    if (payload.clockRate()) {
        gst_structure_set(
                structure,
                "clock-rate",  G_TYPE_INT, payload.clockRate(),
                NULL);
    }

    if (payload.channels()) {
        gst_structure_set(
                structure,
                "encoding-params", G_TYPE_STRING, QByteArray::number(payload.channels(), 10).constData(),
                NULL);
    }

    QMap<QString, QString> parameters = payload.parameters();

    for (QMap<QString, QString>::const_iterator it = parameters.constBegin();
         it != parameters.constEnd(); ++it) {
        gst_structure_set(
                structure,
                it.key().toLatin1().constData(), G_TYPE_STRING, it.value().toLatin1().constData(),
                NULL );
    }

    return gst_caps_new_full( structure, NULL );
}

InboundPayloadFactory *RtpStream::inboundFactory(int payloadId)
{
    foreach (InboundPayloadFactory *factory, m_inboundFactories)
        if (factory->payload().id() == payloadId)
            return factory;

    return 0;
}

OutboundPayloadFactory *RtpStream::outboundFactory(const QMediaRtpPayload &payload)
{
    foreach (OutboundPayloadFactory *factory, m_outboundFactories)
        if (factory->payload().isEquivalent(payload))
            return factory;

    return 0;
}

/*!
    \class gstreamer::AudioRtpStream
    \internal
*/

AudioRtpStream::AudioRtpStream(Direction direction)
    : QMediaRtpAudioStream(direction)
    , m_source(0)
    , m_dtmfToneSource(0)
    , m_outboundToneAdder(0)
    , m_encoder(0)
    , m_payloader(0)
    , m_dtmfPacketSource(0)
    , m_dtmfMuxer(0)
    , m_udpSink(0)
    , m_outboundBus(0)
    , m_udpSource(0)
    , m_jitterBuffer(0)
    , m_demuxer(0)
    , m_switch(0)
    , m_sink(0)
    , m_dtmfDecoder(0)
    , m_dtmfSink(0)
    , m_inboundBus(0)
    , m_busTimerId(-1)
{
}

AudioRtpStream::~AudioRtpStream()
{
    if (m_inboundBin) {
        gst_element_set_state(m_inboundBin, GST_STATE_NULL);
        gst_element_get_state(m_inboundBin, 0, 0, GST_CLOCK_TIME_NONE);

        gst_object_unref(GST_OBJECT(m_inboundBin));
    }

    if (m_outboundBin) {
        gst_element_set_state(m_outboundBin, GST_STATE_NULL);
        gst_element_get_state(m_outboundBin, 0, 0, GST_CLOCK_TIME_NONE);

        gst_object_unref(GST_OBJECT(m_outboundBin));
    }
}

QList<QMediaRtpPayload> AudioRtpStream::inboundPayloads() const
{
    return m_inboundPayloads;
}

void AudioRtpStream::setInboundPayloads(const QList<QMediaRtpPayload> &payloads)
{
    m_inboundPayloads = payloads;

    foreach (m_inboundDtmfPayload, payloads) {
        if (m_inboundDtmfPayload.encodingName().compare(
            QLatin1String("TELEPHONE-EVENT"), Qt::CaseInsensitive) == 0) {
            return;
        }
    }

    m_inboundDtmfPayload = QMediaRtpPayload();
}

QMediaRtpPayload AudioRtpStream::outboundPayload() const
{
    return m_outboundPayload;
}

void AudioRtpStream::setOutboundPayload(const QMediaRtpPayload &payload)
{
    if (!m_outboundBin || outboundState() != Disconnected || m_outboundPayload == payload)
        return;

    if (OutboundPayloadFactory *factory = outboundFactory(payload)) {
        GstElement *encoder = 0;

        if (!(encoder = createGstElement(m_outboundBin, factory->encoderFactory()))) {
            qWarning() << "Failed to create encoder" << __LINE__;
        } else {
            GstElement *payloader = 0;

            if (!(payloader = createGstElement(m_outboundBin, factory->payloaderFactory()))){
                qWarning() << "Failed to create payloader" << __LINE__;
            } else {
                if (m_encoder) {
                    gst_element_set_state(m_encoder, GST_STATE_NULL);
                    gst_element_set_state(m_payloader, GST_STATE_NULL);

                    if (m_dtmfMuxer) {
                        if (GstPad *sourcePad = gst_element_get_static_pad(m_payloader, "src")) {
                            if (GstPad *sinkPad = gst_pad_get_peer(sourcePad)) {
                                gst_element_release_request_pad(m_dtmfMuxer, sinkPad);
                            }
                            gst_object_unref(GST_OBJECT(sourcePad));
                        }
                    }
                    gst_bin_remove(GST_BIN(m_outboundBin), m_encoder);
                    gst_bin_remove(GST_BIN(m_outboundBin), m_payloader);
                }

                GstElement *source = m_outboundToneAdder
                        ? m_outboundToneAdder
                        : m_source;

                GstElement *sink = m_dtmfMuxer
                        ? m_dtmfMuxer
                        : m_udpSink;

                if (!gst_element_link(source, encoder)) {
                    qWarning() << "Failed to audio source to encoder" << __LINE__;
                } else if (!gst_element_link(encoder, payloader)) {
                    qWarning() << "Failed to link encoder to payloader" << __LINE__;
                } else if (!gst_element_link(payloader, sink)) {
                    qWarning() << "Failed to link payloader to sink" << __LINE__;
                } else {
                    m_encoder = encoder;
                    m_payloader = payloader;

                    m_outboundPayload = payload;

                    return;
                }
                gst_bin_remove(GST_BIN(m_outboundBin), payloader);

                m_outboundPayload = QMediaRtpPayload();
            }
            gst_bin_remove(GST_BIN(m_outboundBin), encoder);
        }
    } else {
        qWarning() << "Invalid payload" << payload;
    }
}

QMediaRtpPayload AudioRtpStream::telephonyEventPayload() const
{
    return m_telephonyEventPayload;
}

void AudioRtpStream::setTelephonyEventPayload(const QMediaRtpPayload &payload)
{
    if (m_dtmfPacketSource && m_telephonyEventPayload != payload && outboundState() == Disconnected) {
        if (m_telephonyEventPayload.isNull()) {
            gst_element_set_locked_state(m_dtmfPacketSource, FALSE);

            if (m_dtmfToneSource)
                gst_element_set_locked_state(m_dtmfToneSource, TRUE);

            g_object_set(G_OBJECT(m_dtmfPacketSource), "pt", payload.id(), NULL);
        } else if (payload.isNull()) {
            gst_element_set_locked_state(m_dtmfPacketSource, TRUE);

            if (m_dtmfToneSource)
                gst_element_set_locked_state(m_dtmfToneSource, FALSE);
        } else {
            g_object_set(G_OBJECT(m_dtmfPacketSource), "pt", payload.id(), NULL);
        }

        m_telephonyEventPayload = payload;
    }
}


void AudioRtpStream::connectOutbound(const QHostAddress &address, qint16 port)
{
    if (m_outboundBin && outboundState() == Disconnected) {
        setOutboundAddress(address);
        setOutboundPort(port);
        setOutboundState(Connecting);

        if (m_encoder && m_payloader) {
            g_object_set(G_OBJECT(m_udpSink), "host", address.toString().toLatin1().constData(), NULL);
            g_object_set(G_OBJECT(m_udpSink), "port", port, NULL);

            m_outboundBus = gst_pipeline_get_bus(GST_PIPELINE(m_outboundBin));

            if (m_busTimerId == -1)
                m_busTimerId = startTimer(500);

            gst_element_set_state(m_outboundBin, GST_STATE_PLAYING);
        } else {
            qWarning() << "No outbound audio payload set";

            setOutboundState(Disconnected);
        }
    }
}

void AudioRtpStream::disconnectOutbound()
{
    switch (outboundState()) {
    case Connecting:
    case Connected:
        setOutboundState(Disconnecting);

        gst_element_set_state(m_outboundBin, GST_STATE_READY);
    default:
        break;
    }
}

void AudioRtpStream::connectInbound(const QHostAddress &address, qint16 port)
{
    if (m_inboundBin && inboundState() == Disconnected) {
        setInboundAddress(address);
        setInboundPort(port);
        setInboundState(Connecting);

        g_object_set(
                G_OBJECT(m_udpSource),
                "multicast-group",
                address.toString().toLatin1().constData(),
                NULL);

        g_object_set(G_OBJECT(m_udpSource), "port", port, NULL);

        m_inboundBus = gst_pipeline_get_bus(GST_PIPELINE(m_inboundBin));

        if (m_busTimerId == -1)
            m_busTimerId = startTimer(500);

        gst_element_set_state(m_inboundBin, GST_STATE_PLAYING);
    }
}

void AudioRtpStream::disconnectInbound()
{
    switch (inboundState()) {
    case Connecting:
    case Connected:
        setInboundState(Disconnecting);

        gst_element_set_state(m_inboundBin, GST_STATE_READY);
    default:
        break;
    }
}

void AudioRtpStream::startTelephonyEvent(int event, int volume)
{
    if (m_outboundBin) {
        gst_element_send_event(m_outboundBin, gst_event_new_custom(
                GST_EVENT_CUSTOM_UPSTREAM,
                gst_structure_new(
                        "dtmf-event",
                        "type", G_TYPE_INT, 1,
                        "number", G_TYPE_INT, event,
                        "volume", G_TYPE_INT, volume,
                        "start", G_TYPE_BOOLEAN, TRUE,
                        NULL)));
    }
}

void AudioRtpStream::stopTelephonyEvent()
{
    if (m_outboundBin) {
        gst_element_send_event(m_outboundBin, gst_event_new_custom(
                GST_EVENT_CUSTOM_UPSTREAM,
                gst_structure_new(
                        "dtmf-event",
                        "type", G_TYPE_INT, 1,
                        "start", G_TYPE_BOOLEAN, FALSE,
                        NULL)));
    }
}

void AudioRtpStream::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_busTimerId) {
        event->accept();

        GstMessage *message = 0;

        while (m_inboundBus && (message = gst_bus_poll(m_inboundBus, GST_MESSAGE_ANY, 0))) {
            inboundBusMessage(message);

            gst_message_unref(message);
        }

        while (m_outboundBus && (message = gst_bus_poll(m_outboundBus, GST_MESSAGE_ANY, 0))) {
            outboundBusMessage(message);

            gst_message_unref(message);
        }
    } else {
        QMediaRtpAudioStream::timerEvent(event);
    }
}

bool AudioRtpStream::prepareInboundStream()
{
    if (!(m_inboundBin = gst_pipeline_new(0))) {
        qWarning() << "Failed to create bin" << __LINE__;
    } else {
        if (!(m_udpSource = createGstElement(m_inboundBin, "udpsrc"))) {
            qWarning() << "Failed to create updsrc element" << __LINE__;
        } else if (!(m_jitterBuffer = createGstElement(m_inboundBin, "gstrtpjitterbuffer"))) {
            qWarning() << "Failed to create gstrtpjitterbuffer element" << __LINE__;
        } else if (!(m_demuxer = createGstElement(m_inboundBin, "gstrtpptdemux"))) {
            qWarning() << "Failed to create gstrtpptdemux element" << __LINE__;
        } else if (!(m_switch = createGstElement(m_inboundBin, "input-selector"))) {
            qWarning() << "Failed to create input-selector element" << __LINE__;
        } else if (!(m_sink = createGstElement(m_inboundBin, "alsasink"))
            && !(m_sink = createGstElement(m_inboundBin, "osssink"))) {
            qWarning() << "Failed to create alsasink or osssink gstreamer element";
        } else if (!gst_element_link(m_udpSource, m_jitterBuffer)) {
            qWarning() << "Failed to link updsrc and gstrtpjitterbuffer elements" << __LINE__;
        } else if (!gst_element_link(m_jitterBuffer, m_demuxer)) {
            qWarning() << "Failed to link gstrtpjitterbuffer and gstrtpdemuxer elements" << __LINE__;
        } else {
            if (!gst_element_link(m_switch, m_sink)) {
                qWarning() << "Failed to link input-selector gstreamer element to audio sink";
            } else {
                prepareDtmfDecoder();

                if (GstClock *clock = gst_system_clock_obtain()) {
                    gst_pipeline_use_clock(GST_PIPELINE(m_outboundBin), clock);

                    gst_object_unref(GST_OBJECT(clock));
                }

                g_signal_connect(
                        G_OBJECT(m_demuxer), "new-payload-type", G_CALLBACK(newPayloadType), this);
                g_signal_connect(
                        G_OBJECT(m_demuxer), "payload-type-change", G_CALLBACK(payloadTypeChange), this);
                g_signal_connect(
                        G_OBJECT(m_demuxer), "request-pt-map", G_CALLBACK(requestPtMap), this);
                g_signal_connect(
                        G_OBJECT(m_jitterBuffer), "request-pt-map", G_CALLBACK(requestPtMap), this);

                g_object_set(m_sink, "async", FALSE, NULL);
                g_object_set(m_sink, "sync", FALSE, NULL);

                return true;
            }
        }
        gst_object_unref(GST_OBJECT(m_inboundBin));

        m_udpSource = 0;
        m_jitterBuffer = 0;
        m_demuxer = 0;
    }

    return false;
}

void AudioRtpStream::prepareDtmfDecoder()
{
    if (!(m_dtmfDecoder = createGstElement(m_inboundBin, "rtpdtmfdepay"))) {
        qLog(Media) << "Failed to create rtpdtmfdepay gstreamer element";
    } else {
        if (!(m_dtmfSink = createGstElement(m_inboundBin, "alsasink"))
            && !(m_dtmfSink = createGstElement(m_inboundBin, "osssink"))) {
            qLog(Media) << "Failed to create alsasink or osssink gstreamer elements for dtmf output";
        } else {
            if (!gst_element_link(m_dtmfDecoder, m_dtmfSink)) {
                qLog(Media) << "Failed to link rtpdtmfdepay and liveadder gstreamer elements";
            } else {
                g_object_set(m_dtmfSink, "async", FALSE, NULL);
                g_object_set(m_dtmfSink, "sync", FALSE, NULL);

                return;
            }

            gst_bin_remove(GST_BIN(m_inboundBin), m_dtmfSink);
            m_dtmfSink = 0;
        }
        gst_bin_remove(GST_BIN(m_inboundBin), m_dtmfDecoder);
        m_dtmfDecoder = 0;
    }
    qLog(Media) << "DMTF packets received by this stream cannot be decoded";
}

bool AudioRtpStream::prepareOutboundStream()
{
    if (!(m_outboundBin = gst_pipeline_new(0))) {
        qWarning() << "Failed to create outbound audio bin";
    } else {
        if (!(m_source = createGstElement(m_outboundBin, "alsasrc"))
            && !(m_source = createGstElement(m_outboundBin, "osssrc"))) {
            qWarning() << "Failed to create alsasrc or osssrc elements";
        } else if (!(m_udpSink = createGstElement(m_outboundBin, "udpsink"))) {
            qWarning() << "Failed to create udp sink";
        } else {
            prepareDtmfToneSource();
            prepareDtmfPacketSource();

            if (GstClock *clock = gst_system_clock_obtain()) {
                gst_pipeline_use_clock(GST_PIPELINE(m_outboundBin), clock);

                gst_object_unref(GST_OBJECT(clock));
            }

            g_object_set(G_OBJECT(m_udpSink), "async", FALSE, NULL);
            g_object_set(G_OBJECT(m_udpSink), "sync", FALSE, NULL);

            return true;
        }
        gst_object_unref(GST_OBJECT(m_outboundBin));

        m_source = 0;
        m_udpSink = 0;
    }

    return false;
}

void AudioRtpStream::prepareDtmfToneSource()
{
    if (!(m_dtmfToneSource = createGstElement(m_outboundBin, "dtmfsrc"))) {
        qLog(Media) << "Failed to create dtmfsrc gstreamer element";
    } else {
        if (!(m_outboundToneAdder = createGstElement(m_outboundBin, "liveadder"))) {
            qLog(Media) << "Failed to create liveadder gstreamer element";
        } else {
            if (!gst_element_link(m_dtmfToneSource, m_outboundToneAdder)) {
                qLog(Media) << "Failed to link dtmfsrc and liveadder gstreamer elements";
            } else if (!gst_element_link(m_source, m_outboundToneAdder)) {
                qLog(Media) << "Failed to link audio source to liveadder gstreamer element";
            } else {
                return;
            }
            gst_bin_remove(GST_BIN(m_outboundBin), m_outboundToneAdder);
            m_outboundToneAdder = 0;
        }
        gst_bin_remove(GST_BIN(m_outboundBin), m_dtmfToneSource);
        m_dtmfToneSource = 0;
    }
    qLog(Media) << "DTMF tones can not be generated on this outbound audio stream";
}

void AudioRtpStream::prepareDtmfPacketSource()
{
    if (!(m_dtmfPacketSource = createGstElement(m_outboundBin, "rtpdtmfsrc"))) {
        qLog(Media) << "Failed to create rtpdtmfsrc gstreamer element";
    } else {
        if (!(m_dtmfMuxer = createGstElement(m_outboundBin, "rtpdtmfmux"))) {
            qLog(Media) << "Failed to create rtpdtmfmux gstreamer element";
        } else {
            if (!gst_element_link(m_dtmfPacketSource, m_dtmfMuxer)) {
                qLog(Media) << "Failed to link rtpdtmfsrc and rtpdtmfmux gstreamer elements";
            } else if (!(gst_element_link(m_dtmfMuxer, m_udpSink))) {
                qLog(Media) << "Failed to link rtpdtmfmux and udpsink gstreamer elements";
            } else {
                gst_element_set_locked_state(m_dtmfPacketSource, TRUE);

                return;
            }
            gst_bin_remove(GST_BIN(m_outboundBin), m_dtmfMuxer);
            m_dtmfMuxer = 0;
        }
        gst_bin_remove(GST_BIN(m_outboundBin), m_dtmfPacketSource);
        m_dtmfPacketSource = 0;
    }
    qLog(Media) << "DTMF packets can not be generated on this outbound audio stream";
}

void AudioRtpStream::inboundBusMessage(GstMessage *message)
{
    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_STATE_CHANGED:
        if (GST_MESSAGE_SRC(message) == GST_OBJECT_CAST(m_inboundBin)) {
            GstState oldState;
            GstState newState;
            GstState pending;

            gst_message_parse_state_changed(message, &oldState, &newState, &pending);

            switch (newState) {
            case GST_STATE_READY:
                if (oldState == GST_STATE_PAUSED) {
                    g_signal_emit_by_name(m_demuxer, "clear-pt-map", NULL);

                    gst_object_unref(GST_OBJECT(m_inboundBus));
                    m_inboundBus = 0;

                    if (outboundState() == Disconnected) {
                       killTimer(m_busTimerId);

                       m_busTimerId = -1;
                    }

                    setInboundState(Disconnected);
                }
                break;
            case GST_STATE_PLAYING:
                if (oldState == GST_STATE_PAUSED)
                    setInboundState(Connected);
                break;
            default:
                break;
            }
        }
        break;
    case GST_MESSAGE_ERROR:
        {
            GError *error;
            gst_message_parse_error(message, &error, 0);
            setInboundError(QString::fromLatin1(error->message));
            g_error_free(error);
        }
        disconnectInbound();
        break;
    default:
        break;
    }
}

void AudioRtpStream::outboundBusMessage(GstMessage *message)
{
    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_STATE_CHANGED:
        if (GST_MESSAGE_SRC(message) == GST_OBJECT_CAST(m_outboundBin)) {
            GstState oldState;
            GstState newState;
            GstState pending;

            gst_message_parse_state_changed(message, &oldState, &newState, &pending);

            switch (newState) {
            case GST_STATE_READY:
                if (oldState == GST_STATE_PAUSED) {
                    gst_object_unref(GST_OBJECT(m_outboundBus));
                    m_outboundBus = 0;

                    if (inboundState() == Disconnected) {
                       killTimer(m_busTimerId);

                       m_busTimerId = -1;
                    }

                    setOutboundState(Disconnected);
                }
                break;
            case GST_STATE_PLAYING:
                if (oldState == GST_STATE_PAUSED)
                    setOutboundState(Connected);
                break;
            default:
                break;
            }
        }
        break;
    case GST_MESSAGE_ERROR:
        {
            GError *error;
            gst_message_parse_error(message, &error, 0);
            setOutboundError(QString::fromLatin1(error->message));
            g_error_free(error);
        }
        disconnectOutbound();
        break;
    default:
        break;
    }
}

void AudioRtpStream::newPayloadType(GstElement *element, guint pt, GstPad *pad, AudioRtpStream *stream)
{
    Q_UNUSED(element);

    qLog(Media) << "New audio payload" << pt;

    if (InboundPayloadFactory *factory = stream->inboundFactory(pt)) {
        if (GstElement *payloadBin = factory->createBin(stream->m_inboundBin)) {
            if (GstPad *sinkPad = gst_element_get_static_pad(payloadBin, "sink")) {
                if (gst_pad_link(pad, sinkPad) != GST_PAD_LINK_OK) {
                    qWarning() << "Failed to link demuxer pad to payload bin";
                } else if (!gst_element_link(payloadBin, stream->m_switch)) {
                    qWarning() << "Failed to link payload bin to sink";
                } else {
                    gst_object_unref(GST_OBJECT(sinkPad));
                    gst_element_set_state(payloadBin, GST_STATE_PLAYING);

                    stream->m_inboundPayloadBins.insert(pt, payloadBin);

                    return;
                }
                gst_object_unref(GST_OBJECT(sinkPad));
            }
            gst_element_set_state(payloadBin, GST_STATE_NULL);
            gst_bin_remove(GST_BIN(stream->m_inboundBin), payloadBin);
        }
    } else if (stream->m_dtmfDecoder && stream->inboundDtmfPayload().id() == int(pt)) {
        if (GstPad *sinkPad = gst_element_get_static_pad(stream->m_dtmfDecoder, "sink")) {
            if (gst_pad_link(pad, sinkPad) != GST_PAD_LINK_OK) {
                qWarning() << "Failed to link demuxer pad to dtmf decoder";
            }
            gst_object_unref(GST_OBJECT(sinkPad));
        }
    }
}

void AudioRtpStream::payloadTypeChange(GstElement *element, guint pt, AudioRtpStream *stream)
{
    Q_UNUSED(element);

    qLog(Media) << "Audio payload changed" << pt;

    if (GstElement *payloadBin = stream->m_inboundPayloadBins.value(pt)) {
        if (GstPad *sourcePad = gst_element_get_static_pad(payloadBin, "src")) {
            if (GstPad *sinkPad = gst_pad_get_peer(sourcePad)) {
                g_object_set(G_OBJECT(stream->m_switch), "active-pad", sinkPad, NULL);

                gst_object_unref(GST_OBJECT(sinkPad));

                foreach (QMediaRtpPayload payload, stream->inboundPayloads()) {
                    if (payload.id() == int(pt)) {
                        stream->setInboundPayload(payload);
                        break;
                    }
                }
            }
            gst_object_unref(GST_OBJECT(sourcePad));
        }
    }
}

/*!
    \class gstreamer::VideoRtpStream
    \internal
*/

VideoRtpStream::VideoRtpStream(Direction direction)
    : QMediaRtpVideoStream(direction)
    , m_source(0)
    , m_queue(0)
    , m_inputColorSpace(0)
    , m_encoder(0)
    , m_payloader(0)
    , m_udpSink(0)
    , m_outboundBus(0)
    , m_udpSource(0)
    , m_jitterBuffer(0)
    , m_demuxer(0)
    , m_switch(0)
    , m_outputColorSpace(0)
    , m_sink(0)
    , m_inboundBus(0)
    , m_camera(0)
    , m_painter(0)
    , m_busTimerId(-1)
{
}

VideoRtpStream::~VideoRtpStream()
{
    if (m_inboundBin) {
        gst_element_set_state(m_inboundBin, GST_STATE_NULL);
        gst_element_get_state(m_inboundBin, 0, 0, GST_CLOCK_TIME_NONE);

        gst_object_unref(GST_OBJECT(m_inboundBin));
    }

    if (m_outboundBin) {
        gst_element_set_state(m_outboundBin, GST_STATE_NULL);
        gst_element_get_state(m_outboundBin, 0, 0, GST_CLOCK_TIME_NONE);

        gst_object_unref(GST_OBJECT(m_outboundBin));
    }

    delete m_painter;
}

QList<QMediaRtpPayload> VideoRtpStream::inboundPayloads() const
{
    return m_inboundPayloads;
}

void VideoRtpStream::setInboundPayloads(const QList<QMediaRtpPayload> &payloads)
{
    m_inboundPayloads = payloads;
}

QMediaRtpPayload VideoRtpStream::outboundPayload() const
{
    return m_outboundPayload;
}

void VideoRtpStream::setOutboundPayload(const QMediaRtpPayload &payload)
{
    if (outboundState() != Disconnected)
        return;

    if (OutboundPayloadFactory *factory = outboundFactory(payload)) {
        GstElement *encoder = 0;

        if (!(encoder = createGstElement(m_outboundBin, factory->encoderFactory()))) {
            qWarning() << "Failed to create encoder" << __LINE__;
        } else {
            GstElement *payloader = 0;

            if (!(payloader = createGstElement(m_outboundBin, factory->payloaderFactory()))){
                qWarning() << "Failed to create payloader" << __LINE__;
            } else {
                if (m_encoder) {
                    gst_bin_remove(GST_BIN(m_outboundBin), m_encoder);
                    gst_bin_remove(GST_BIN(m_outboundBin), m_payloader);
                }
                if (!gst_element_link(m_inputColorSpace, encoder)) {
                    qWarning() << "Failed to link video input to encoder" << __LINE__;
                } else if (!gst_element_link(encoder, payloader)) {
                    qWarning() << "Failed to link encoder to payloader" << __LINE__;
                } else if (!gst_element_link(payloader, m_udpSink)) {
                    qWarning() << "Failed to link payloader to upd sink" << __LINE__;
                } else {
                    qLog(Media) << "Outbound payload changed" << payload;

                    m_encoder = encoder;
                    m_payloader = payloader;

                    m_outboundPayload = payload;

                    return;
                }
                m_outboundPayload = QMediaRtpPayload();

                gst_bin_remove(GST_BIN(m_outboundBin), payloader);
            }
            gst_bin_remove(GST_BIN(m_outboundBin), encoder);
        }
    } else {
        qWarning() << "Invalid payload" << payload;
    }
}

void VideoRtpStream::connectOutbound(const QHostAddress &address, qint16 port)
{
    if (outboundState() == Disconnected) {
        setOutboundAddress(address);
        setOutboundPort(port);
        setOutboundState(Connecting);

        if (m_encoder && m_payloader) {
            g_object_set(G_OBJECT(m_udpSink), "host", address.toString().toLatin1().constData(), NULL);
            g_object_set(G_OBJECT(m_udpSink), "port", port, NULL);

            m_outboundBus = gst_pipeline_get_bus(GST_PIPELINE(m_outboundBin));

            if (m_busTimerId == -1)
                m_busTimerId = startTimer(500);

            gst_element_set_state(m_outboundBin, GST_STATE_PLAYING);
        } else {
            qWarning() << "No outbound video payload set";

            setOutboundState(Disconnected);
        }
    }
}

void VideoRtpStream::disconnectOutbound()
{
    switch (outboundState()) {
    case Connecting:
    case Connected:
        setOutboundState(Disconnecting);

        gst_element_set_state(m_outboundBin, GST_STATE_READY);
    default:
        break;
    }
}

void VideoRtpStream::connectInbound(const QHostAddress &address, qint16 port)
{
    if (inboundState() == Disconnected) {
        setInboundAddress(address);
        setInboundPort(port);
        setInboundState(Connecting);

        g_object_set(
                G_OBJECT(m_udpSource),
                "multicast-group",
                address.toString().toLatin1().constData(),
                NULL);

        g_object_set(G_OBJECT(m_udpSource), "port", port, NULL);

        m_inboundBus = gst_pipeline_get_bus(GST_PIPELINE(m_inboundBin));

        if (m_busTimerId == -1)
            m_busTimerId = startTimer(500);

        gst_element_set_state(m_inboundBin, GST_STATE_PLAYING);
    }
}

void VideoRtpStream::disconnectInbound()
{
    switch (inboundState()) {
    case Connecting:
    case Connected:
        setInboundState(Disconnecting);

        gst_element_set_state(m_inboundBin, GST_STATE_READY);
    default:
        break;
    }
}

void VideoRtpStream::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_busTimerId) {
        event->accept();

        GstMessage *message = 0;

        while (m_inboundBus && (message = gst_bus_poll(m_inboundBus, GST_MESSAGE_ANY, 0))) {
            inboundBusMessage(message);

            gst_message_unref(message);
        }

        while (m_outboundBus && (message = gst_bus_poll(m_outboundBus, GST_MESSAGE_ANY, 0))) {
            outboundBusMessage(message);

            gst_message_unref(message);
        }
    } else {
        QMediaRtpVideoStream::timerEvent(event);
    }
}


void VideoRtpStream::frameReady(const QVideoFrame &frame)
{
    reinterpret_cast<QtopiaCameraSource *>(m_source)->frameReady(frame, m_camera->framerate());
}

QList<uint> VideoRtpStream::supportedVideoFormats(const QMediaRtpPayload &payload) const
{
    Q_UNUSED(payload);

    return QList<uint>()
            << QtopiaCamera::RGB32
            << QtopiaCamera::RGB24
            << QtopiaCamera::RGB565
            << QtopiaCamera::RGB555
            << QtopiaCamera::YUYV
            << QtopiaCamera::UYVY;
}

QCameraPreviewCapture *VideoRtpStream::camera() const
{
    return m_camera;
}

void VideoRtpStream::setCamera(QCameraPreviewCapture *camera)
{
    if (m_source) {
        if (m_camera)
            disconnect(m_camera, SIGNAL(frameReady(QVideoFrame)), this, SLOT(frameReady(QVideoFrame)));

        m_camera = camera;

        if (m_camera)
            connect(m_camera, SIGNAL(frameReady(QVideoFrame)), this, SLOT(frameReady(QVideoFrame)));
    }
}

QVideoSurface *VideoRtpStream::videoSurface() const
{
    return m_painter
            ? m_painter->videoSurface()
            : 0;
}

bool VideoRtpStream::prepareOutboundStream()
{
    if (!(m_outboundBin = gst_pipeline_new(0))) {
        qWarning() << "Failed to create outbound video bin";
    } else {
        if (!(m_source = createGstElement(m_outboundBin, QtopiaCameraSourceClass::get_type()))) {
            qWarning() << "Failed to create camera source element" << __LINE__;
        } else if (!(m_queue = createGstElement(m_outboundBin, "queue"))) {
            qWarning() << "Failed to create queue element";
        } else if (!(m_inputColorSpace = createGstElement(m_outboundBin, "ffmpegcolorspace"))) {
            qWarning() << "Failed to create ffmpegcolorspace element" << __LINE__;
        } else if (!(m_udpSink = createGstElement(m_outboundBin, "udpsink"))) {
            qWarning() << "Failed to create udp sink";
        } else if (!gst_element_link(m_source, m_queue)) {
            qWarning() << "Failed to link video source to queue";
        } else if (!gst_element_link(m_queue, m_inputColorSpace)) {
            qWarning() << "Failed to link queue to color space converter";
        } else {
            g_object_set(m_udpSink, "async", FALSE, NULL);
            g_object_set(m_udpSink, "sync", FALSE, NULL);

            return true;
        }
        gst_object_unref(GST_OBJECT(m_outboundBin));

        m_source = 0;
        m_queue = 0;
        m_inputColorSpace = 0;
        m_udpSink = 0;
    }

    return false;
}

bool VideoRtpStream::prepareInboundStream()
{
    if (!(m_inboundBin = gst_pipeline_new(0))) {
        qWarning() << "Failed to create bin" << __LINE__;
    } else {
        m_painter = new gstreamer::VideoWidget;

        if (!(m_sink = m_painter->element())) {
            qWarning() << "Failed to create video sink";
        } else if (!(gst_bin_add(GST_BIN(m_inboundBin), m_sink))) {
            qWarning() << "Failed to add video sink to inbound video bin";
        } else if (!(m_udpSource = createGstElement(m_inboundBin, "udpsrc"))) {
            qWarning() << "Failed to create updsrc element" << __LINE__;
        } else if (!(m_jitterBuffer = createGstElement(m_inboundBin, "gstrtpjitterbuffer"))) {
            qWarning() << "Failed to create gstrtpjitterbuffer element" << __LINE__;
        } else if (!(m_demuxer = createGstElement(m_inboundBin, "gstrtpptdemux"))) {
            qWarning() << "Failed to create gstrtpptdemux element" << __LINE__;
        } else if (!(m_switch = createGstElement(m_inboundBin, "input-selector"))) {
            qWarning() << "Failed to create switch element" << __LINE__;
        } else if (!(m_outputColorSpace = createGstElement(m_inboundBin, "ffmpegcolorspace"))) {
            qWarning() << "Failed to create ffmpegcolorspace element" << __LINE__;
        } else if (!gst_element_link(m_udpSource, m_jitterBuffer)) {
            qWarning() << "Failed to link updsrc and gstrtpjitterbuffer elements" << __LINE__;
        } else if (!gst_element_link(m_jitterBuffer, m_demuxer)) {
            qWarning() << "Failed to link gstrtpjitterbuffer and gstrtpdemuxer elements" << __LINE__;
        } else if (!gst_element_link(m_switch, m_outputColorSpace)) {
            qWarning() << "Failed to link switch and ffmpegcolorspace elements" << __LINE__;
        } else if (!gst_element_link(m_outputColorSpace, m_sink)) {
            qWarning() << "Failed to link ffmpegcolorspace and video sink elements" << __LINE__;
        } else {
            g_object_set(m_sink, "async", FALSE, NULL);
            g_object_set(m_sink, "sync", FALSE, NULL);

            g_signal_connect(
                    G_OBJECT(m_demuxer), "new-payload-type", G_CALLBACK(newPayloadType), this);
            g_signal_connect(
                    G_OBJECT(m_demuxer), "payload-type-change", G_CALLBACK(payloadTypeChange), this);
            g_signal_connect(
                    G_OBJECT(m_demuxer), "request-pt-map", G_CALLBACK(requestPtMap), this);
            g_signal_connect(
                    G_OBJECT(m_jitterBuffer), "request-pt-map", G_CALLBACK(requestPtMap), this);

            return true;
        }
        gst_object_unref(GST_OBJECT(m_inboundBin));

        m_udpSource = 0;
        m_jitterBuffer = 0;
        m_demuxer = 0;
        m_outputColorSpace = 0;
        m_sink = 0;
        delete m_painter;
        m_painter = 0;
    }

    return false;
}

void VideoRtpStream::inboundBusMessage(GstMessage *message)
{
    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_STATE_CHANGED:
        if (GST_MESSAGE_SRC(message) == GST_OBJECT_CAST(m_inboundBin)) {
            GstState oldState;
            GstState newState;
            GstState pending;

            gst_message_parse_state_changed(message, &oldState, &newState, &pending);

            switch (newState)
            {
            case GST_STATE_READY:
                if (oldState == GST_STATE_PAUSED) {
                    g_signal_emit_by_name(m_demuxer, "clear-pt-map", NULL);

                    setInboundState(Disconnected);
                }
                break;
            case GST_STATE_PLAYING:
                if (oldState == GST_STATE_PAUSED)
                    setInboundState(Connected);
                break;
            default:
                break;
            }
        }
        break;
    case GST_MESSAGE_ERROR:
        {
            GError *error;
            gst_message_parse_error(message, &error, 0);
            setInboundError(QString::fromLatin1(error->message));
            g_error_free(error);
        }
        disconnectInbound();
        break;
    default:
        break;
    }
}

void VideoRtpStream::outboundBusMessage(GstMessage *message)
{
    switch (GST_MESSAGE_TYPE(message)) {
    case GST_MESSAGE_STATE_CHANGED:
        if (GST_MESSAGE_SRC(message) == GST_OBJECT_CAST(m_outboundBin)) {
            GstState oldState;
            GstState newState;
            GstState pending;

            gst_message_parse_state_changed(message, &oldState, &newState, &pending);

            switch (newState)
            {
            case GST_STATE_READY:
                if (oldState == GST_STATE_PAUSED)
                    setOutboundState(Disconnected);
                break;
            case GST_STATE_PLAYING:
                if (oldState == GST_STATE_PAUSED)
                    setOutboundState(Connected);
                break;
            default:
                break;
            }
        }
        break;
    case GST_MESSAGE_ERROR:
        {
            GError *error;
            gst_message_parse_error(message, &error, 0);
            setOutboundError(QString::fromLatin1(error->message));
            g_error_free(error);
        }
        disconnectOutbound();
        break;
    default:
        break;
    }
}

void VideoRtpStream::newPayloadType(GstElement *element, guint pt, GstPad *pad, VideoRtpStream *stream)
{
    Q_UNUSED(element);

    qLog(Media) << "New video payload" << pt;

    if (InboundPayloadFactory *factory = stream->inboundFactory(pt)) {
        if (GstElement *payloadBin = factory->createBin(stream->m_inboundBin)) {
            if (GstPad *sinkPad = gst_element_get_static_pad(payloadBin, "sink")) {
                if (gst_pad_link(pad, sinkPad) != GST_PAD_LINK_OK) {
                    qWarning() << "Failed to link demuxer pad to payload bin";
                } else if (!gst_element_link(payloadBin, stream->m_switch)) {
                    qWarning() << "Failed to link payload bin to sink";
                } else {
                    gst_object_unref(GST_OBJECT(sinkPad));

                    gst_element_set_state(payloadBin, GST_STATE_PLAYING);

                    stream->m_inboundPayloadBins.insert(pt, payloadBin);

                    return;
                }
                gst_object_unref(GST_OBJECT(sinkPad));
            }
            gst_element_set_state(payloadBin, GST_STATE_NULL);
            gst_bin_remove(GST_BIN(stream->m_inboundBin), payloadBin);
        }
    }
}

void VideoRtpStream::payloadTypeChange(GstElement *element, guint pt, VideoRtpStream *stream)
{
    Q_UNUSED(element);

    qLog(Media) << "Video payload changed" << pt;

    if (GstElement *payloadBin = stream->m_inboundPayloadBins.value(pt)) {
        if (GstPad *sourcePad = gst_element_get_static_pad(payloadBin, "src")) {
            if (GstPad *sinkPad = gst_pad_get_peer(sourcePad)) {
                g_object_set(G_OBJECT(stream->m_switch), "active-pad", sinkPad, NULL);

                gst_object_unref(GST_OBJECT(sinkPad));

                foreach (QMediaRtpPayload payload, stream->inboundPayloads()) {
                    if (payload.id() == int(pt)) {
                        stream->setInboundPayload(payload);
                        break;
                    }
                }
            }
            gst_object_unref(GST_OBJECT(sourcePad));
        }
    }
}

/*!
    \class gstreamer::RtpSession
    \internal
*/

RtpSession::RtpSession()
{
    gst_init(NULL, NULL);

    QSettings conf(QLatin1String("Trolltech"), QLatin1String("gstreamer"));

    conf.beginGroup(QLatin1String("OutboundPayloads"));
    foreach (QString group, conf.childGroups()) {
        conf.beginGroup(group);
        if (OutboundPayloadFactory *factory = OutboundPayloadFactory::createFactory(group, &conf)) {
            QMediaRtpPayload payload = factory->payload();

            m_outboundFactories[payload.type()].append(factory);
            m_outboundPayloads[payload.type()].append(payload);
        } else if (conf.value(QLatin1String("EncodingName")).toString().compare(
            QLatin1String("TELEPHONE-EVENT"), Qt::CaseInsensitive) == 0) {
            m_outboundPayloads[QMediaRtpStream::Audio].append(QMediaRtpPayload(
                    group.toInt(),
                    QMediaRtpStream::Audio,
                    QLatin1String("TELEPHONE-EVENT"),
                    conf.value(QLatin1String("ClockRate"), 8000).toInt(),
                    0));
        }
        conf.endGroup();
    }
    conf.endGroup();

    conf.beginGroup(QLatin1String("InboundPayloads"));
    foreach (QString group, conf.childGroups()) {
        conf.beginGroup(group);
        if (InboundPayloadFactory *factory = InboundPayloadFactory::createFactory(group, &conf)) {
            QMediaRtpPayload payload = factory->payload();

            m_inboundFactories[payload.type()].append(factory);
            m_inboundPayloads[payload.type()].append(payload);
        } else if (conf.value(QLatin1String("EncodingName")).toString().compare(
            QLatin1String("TELEPHONE-EVENT"), Qt::CaseInsensitive) == 0) {
            m_inboundPayloads[QMediaRtpStream::Audio].append(QMediaRtpPayload(
                    group.toInt(),
                    QMediaRtpStream::Audio,
                    QLatin1String("TELEPHONE-EVENT"),
                    conf.value(QLatin1String("ClockRate"), 8000).toInt(),
                    0));
        }
        conf.endGroup();
    }
    conf.endGroup();
}

RtpSession::~RtpSession()
{
    qDeleteAll(m_audioStreams);
    qDeleteAll(m_videoStreams);
}

QList<QMediaRtpPayload> RtpSession::supportedInboundPayloads(QMediaRtpStream::Type type)
{
    return m_inboundPayloads[type];
}

QList<QMediaRtpPayload> RtpSession::supportedOutboundPayloads(QMediaRtpStream::Type type)
{
    return m_outboundPayloads[type];
}

int RtpSession::streamCount() const
{
    return m_audioStreams.count() + m_videoStreams.count();
}

QMediaRtpStream *RtpSession::stream(int index) const
{
    if (index < m_audioStreams.count())
        return m_audioStreams.at(index);
    else
        return m_videoStreams.value(index - m_audioStreams.count());
}

QMediaRtpStream *RtpSession::addStream(QMediaRtpStream::Type type, QMediaRtpStream::Direction direction)
{
    if (type == QMediaRtpStream::Audio) {
        AudioRtpStream *stream = new AudioRtpStream(direction);

        if (direction & QMediaRtpStream::SendOnly && !stream->prepareOutboundStream()) {
            qWarning() << "Failed to construct outbound audio stream";
        } else if (direction & QMediaRtpStream::ReceiveOnly && !stream->prepareInboundStream()) {
            qWarning() << "Failed to construct inbound audio stream";
        } else {
            stream->setInboundPayloads(m_inboundPayloads[QMediaRtpStream::Audio]);
            stream->setInboundFactories(m_inboundFactories[QMediaRtpStream::Audio]);
            stream->setOutboundFactories(m_outboundFactories[QMediaRtpStream::Audio]);

            m_audioStreams.append(stream);

            return stream;
        }

        delete stream;
    } else if (type == QMediaRtpStream::Video) {
        VideoRtpStream *stream = new VideoRtpStream(direction);

        if (direction & QMediaRtpStream::SendOnly && !stream->prepareOutboundStream()) {
            qWarning() << "Failed to construct outbound video stream";
        } else if (direction & QMediaRtpStream::ReceiveOnly && !stream->prepareInboundStream()) {
            qWarning() << "Failed to construct inbound video stream";
        } else {
            stream->setInboundPayloads(m_inboundPayloads[QMediaRtpStream::Video]);
            stream->setInboundFactories(m_inboundFactories[QMediaRtpStream::Video]);
            stream->setOutboundFactories(m_outboundFactories[QMediaRtpStream::Video]);


            m_videoStreams.append(stream);

            return stream;
        }

        delete stream;
    }

    return 0;
}

void RtpSession::removeStream(QMediaRtpStream *stream)
{
    switch (stream->type()) {
    case QMediaRtpStream::Audio:
        delete stream;

        m_audioStreams.removeAll(static_cast<AudioRtpStream *>(stream));

        break;
    case QMediaRtpStream::Video:
        delete stream;

        m_videoStreams.removeAll(static_cast<VideoRtpStream *>(stream));

        break;
    default:
        break;
    }
}

}   // ns gstreamer
