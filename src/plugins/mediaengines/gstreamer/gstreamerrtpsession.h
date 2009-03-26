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
#ifndef GSTREAMERRTPSESSION_H
#define GSTREAMERRTPSESSION_H

#include <gst/gst.h>

#include <qmediartpsessionengine.h>

#include "gstreamervideowidget.h"


class QCameraDevice;
class QCameraPreviewCapture;

namespace gstreamer
{

class InboundPayloadFactory;
class OutboundPayloadFactory;

class RtpStream
{
public:
    RtpStream();
    virtual ~RtpStream();

    void setInboundFactories(const QList<InboundPayloadFactory *> &factories);
    void setOutboundFactories(const QList<OutboundPayloadFactory *> &factories);

protected:
    InboundPayloadFactory *inboundFactory(int payloadId);
    OutboundPayloadFactory *outboundFactory(const QMediaRtpPayload &payload);

    static GstCaps *requestPtMap(GstElement *demux, guint pt, QMediaRtpStream *stream);
private:
    static GstCaps *createCaps(const QMediaRtpPayload &payload);

    QList<InboundPayloadFactory *> m_inboundFactories;
    QList<OutboundPayloadFactory *> m_outboundFactories;
};

class AudioRtpStream : public QMediaRtpAudioStream, public RtpStream
{
    Q_OBJECT
public:
    AudioRtpStream(Direction direction);
    ~AudioRtpStream();

    QMediaRtpPayload inboundDtmfPayload() const { return m_inboundDtmfPayload; }

    QList<QMediaRtpPayload> inboundPayloads() const;
    void setInboundPayloads(const QList<QMediaRtpPayload> &payloads);

    QMediaRtpPayload outboundPayload() const;
    void setOutboundPayload(const QMediaRtpPayload &payload);

    QMediaRtpPayload telephonyEventPayload() const;
    void setTelephonyEventPayload(const QMediaRtpPayload &payload);

    bool prepareOutboundStream();
    bool prepareInboundStream();

public slots:
    void connectOutbound(const QHostAddress &address, qint16 port);
    void disconnectOutbound();

    void connectInbound(const QHostAddress &address, qint16 port);
    void disconnectInbound();

    void startTelephonyEvent(int event, int volume);
    void stopTelephonyEvent();

protected:
    void timerEvent(QTimerEvent *event);

private:
    static void newPayloadType(GstElement *element, guint pt, GstPad *pad, AudioRtpStream *stream);
    static void payloadTypeChange(GstElement *element, guint pt, AudioRtpStream *stream);

    void prepareDtmfDecoder();
    void prepareDtmfToneSource();
    void prepareDtmfPacketSource();

    void inboundBusMessage(GstMessage *message);
    void outboundBusMessage(GstMessage *message);

    GstElement *m_outboundBin;
    GstElement *m_source;
    GstElement *m_dtmfToneSource;
    GstElement *m_outboundToneAdder;
    GstElement *m_encoder;
    GstElement *m_payloader;
    GstElement *m_dtmfPacketSource;
    GstElement *m_dtmfMuxer;
    GstElement *m_udpSink;
    GstBus *m_outboundBus;

    QList<QMediaRtpPayload> m_inboundPayloads;
    QMediaRtpPayload m_inboundDtmfPayload;

    QMediaRtpPayload m_outboundPayload;
    QMediaRtpPayload m_telephonyEventPayload;

    GstElement *m_inboundBin;
    GstElement *m_udpSource;
    GstElement *m_jitterBuffer;
    GstElement *m_demuxer;
    QMap<int, GstElement *> m_inboundPayloadBins;
    GstElement *m_switch;
    GstElement *m_sink;
    GstElement *m_dtmfDecoder;
    GstElement *m_dtmfSink;
    GstBus *m_inboundBus;

    int m_busTimerId;
};

class VideoRtpStream : public QMediaRtpVideoStream, public RtpStream
{
    Q_OBJECT
public:
    VideoRtpStream(Direction direction);
    ~VideoRtpStream();

    QList<QMediaRtpPayload> inboundPayloads() const;
    void setInboundPayloads(const QList<QMediaRtpPayload> &payloads);

    QMediaRtpPayload outboundPayload() const;
    void setOutboundPayload(const QMediaRtpPayload &payload);

    QList<uint> supportedVideoFormats(const QMediaRtpPayload &payload) const;

    QCameraPreviewCapture *camera() const;
    void setCamera(QCameraPreviewCapture *camera);

    QVideoSurface *videoSurface() const;

    bool prepareOutboundStream();
    bool prepareInboundStream();

    bool busMessage(GstMessage *message);

public slots:
    void connectOutbound(const QHostAddress &address, qint16 port);
    void disconnectOutbound();

    void connectInbound(const QHostAddress &address, qint16 port);
    void disconnectInbound();

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void frameReady(const QVideoFrame &frame);

private:
    static void newPayloadType(GstElement *element, guint pt, GstPad *pad, VideoRtpStream *stream);
    static void payloadTypeChange(GstElement *element, guint pt, VideoRtpStream *stream);

    void inboundBusMessage(GstMessage *message);
    void outboundBusMessage(GstMessage *message);

    GstElement *m_outboundBin;
    GstElement *m_source;
    GstElement *m_queue;
    GstElement *m_inputColorSpace;
    GstElement *m_encoder;
    GstElement *m_payloader;
    GstElement *m_udpSink;
    GstBus *m_outboundBus;

    QList<QMediaRtpPayload> m_inboundPayloads;

    QMediaRtpPayload m_outboundPayload;
    QMediaRtpPayload m_telephonyEventPayload;

    GstElement *m_inboundBin;
    GstElement *m_udpSource;
    GstElement *m_jitterBuffer;
    GstElement *m_demuxer;
    QMap<int, GstElement *> m_inboundPayloadBins;
    GstElement *m_switch;
    GstElement *m_outputColorSpace;
    GstElement *m_sink;
    GstBus *m_inboundBus;

    QCameraPreviewCapture *m_camera;
    gstreamer::VideoWidget *m_painter;

    int m_busTimerId;
};

class RtpSession : public QMediaRtpEngine
{
    Q_OBJECT
public:
    RtpSession();
    ~RtpSession();

    QList<QMediaRtpPayload> supportedInboundPayloads(QMediaRtpStream::Type type);
    QList<QMediaRtpPayload> supportedOutboundPayloads(QMediaRtpStream::Type type);

    int streamCount() const;
    QMediaRtpStream *stream(int index) const;
    QMediaRtpStream *addStream(QMediaRtpStream::Type type, QMediaRtpStream::Direction direction);
    void removeStream(QMediaRtpStream *stream);

private:
    QList<AudioRtpStream *> m_audioStreams;
    QList<VideoRtpStream *> m_videoStreams;

    QList<QMediaRtpPayload> m_inboundPayloads[2];
    QList<QMediaRtpPayload> m_outboundPayloads[2];
    QList<InboundPayloadFactory *> m_inboundFactories[2];
    QList<OutboundPayloadFactory *> m_outboundFactories[2];

    friend class RtpStream;
};

}   // ns gstreamer

#endif
