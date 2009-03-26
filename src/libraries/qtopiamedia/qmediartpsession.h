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
#ifndef QMEDIARTPSESSION_H
#define QMEDIARTPSESSION_H

#include <QObject>
#include <QMap>
#include <QSharedData>
#include <QMetaType>
#include <qtopiaipcmarshal.h>
#include <media.h>

class QHostAddress;
class QWidget;
class QMediaRtpPayload;
class QMediaRtpStreamPrivate;

class QTOPIAMEDIA_EXPORT QMediaRtpStream : public QObject
{
    Q_OBJECT
public:
    enum Direction
    {
        SendOnly = 0x01,
        ReceiveOnly = 0x02,
        SendReceive = SendOnly | ReceiveOnly
    };

    enum Type
    {
        Audio,
        Video
    };

    enum State
    {
        Connecting,
        Connected,
        Disconnecting,
        Disconnected
    };

    QMediaRtpStream(Type type, Direction direction, QObject *parent = 0);
    ~QMediaRtpStream();

    Type type() const;
    Direction direction() const;

    virtual QMediaRtpPayload outboundPayload() const = 0;
    virtual void setOutboundPayload(const QMediaRtpPayload &payload) = 0;

    QHostAddress outboundAddress() const;
    qint16 outboundPort() const;
    State outboundState() const;

    QMediaRtpPayload inboundPayload() const;

    virtual QList<QMediaRtpPayload> inboundPayloads() const = 0;
    virtual void setInboundPayloads(const QList<QMediaRtpPayload> &payloads) = 0;

    QHostAddress inboundAddress() const;
    qint16 inboundPort() const;
    State inboundState() const;

    QString inboundErrorString() const;
    QString outboundErrorString() const;

public slots:
    virtual void connectOutbound(const QHostAddress &address, qint16 port) = 0;
    virtual void disconnectOutbound() = 0;

    virtual void connectInbound(const QHostAddress &address, qint16 port) = 0;
    virtual void disconnectInbound() = 0;

signals:
    void inboundPayloadChanged(const QMediaRtpPayload &payload);

    void outboundConnected();
    void outboundDisconnected();
    void outboundStateChanged(QMediaRtpStream::State state);

    void inboundConnected();
    void inboundDisconnected();
    void inboundStateChanged(QMediaRtpStream::State state);

    void inboundError(const QString &error);
    void outboundError(const QString &error);

protected:
    void setOutboundAddress(const QHostAddress &address);
    void setOutboundPort(qint16 port);
    void setOutboundState(State state);

    void setInboundAddress(const QHostAddress &address);
    void setInboundPort(qint16 port);
    void setInboundState(State state);

    void setInboundPayload(const QMediaRtpPayload &payload);

    void setInboundError(const QString &error);
    void setOutboundError(const QString &error);

private:
    QMediaRtpStreamPrivate *d;
};

class QCameraPreviewCapture;
class QVideoSurface;

class QTOPIAMEDIA_EXPORT QMediaRtpAudioStream : public QMediaRtpStream
{
    Q_OBJECT
public:
    enum DtmfTone
    {
        Dtmf_0 = 0,
        Dtmf_1 = 1,
        Dtmf_2 = 2,
        Dtmf_3 = 3,
        Dtmf_4 = 4,
        Dtmf_5 = 5,
        Dtmf_6 = 6,
        Dtmf_7 = 7,
        Dtmf_8 = 8,
        Dtmf_9 = 9,
        Dtmf_Star = 10,
        Dtmf_Pound = 11,
        Dtmf_A = 12,
        Dtmf_B = 13,
        Dtmf_C = 14,
        Dtmf_D = 15
    };

    QMediaRtpAudioStream(Direction direction, QObject *parent = 0);
    ~QMediaRtpAudioStream();

    virtual QMediaRtpPayload telephonyEventPayload() const = 0;
    virtual void setTelephonyEventPayload(const QMediaRtpPayload &payload) = 0;

public slots:
    virtual void startTelephonyEvent(int event, int volume) = 0;
    virtual void stopTelephonyEvent() = 0;
};

class QTOPIAMEDIA_EXPORT QMediaRtpVideoStream : public QMediaRtpStream
{
    Q_OBJECT
public:
    QMediaRtpVideoStream(Direction direction, QObject *parent = 0);
    ~QMediaRtpVideoStream();

    virtual QList<uint> supportedVideoFormats(const QMediaRtpPayload &payload) const = 0;

    virtual QCameraPreviewCapture *camera() const = 0;
    virtual void setCamera(QCameraPreviewCapture *capture) = 0;

    virtual QVideoSurface *videoSurface() const = 0;
};

class QMediaRtpSessionPrivate;

class QTOPIAMEDIA_EXPORT QMediaRtpSession : public QObject
{
    Q_OBJECT
public:
    QMediaRtpSession(QObject *parent = 0);
    ~QMediaRtpSession();

    QList<QMediaRtpPayload> supportedInboundPayloads(QMediaRtpStream::Type type) const;
    QList<QMediaRtpPayload> supportedOutboundPayloads(QMediaRtpStream::Type type) const;

    int streamCount() const;
    QMediaRtpStream *stream(int index) const;
    QMediaRtpStream *addStream(QMediaRtpStream::Type type, QMediaRtpStream::Direction direction);
    QMediaRtpAudioStream *addAudioStream(QMediaRtpStream::Direction direction);
    QMediaRtpVideoStream *addVideoStream(QMediaRtpStream::Direction direction);
    void removeStream(QMediaRtpStream *stream);

private:
    QMediaRtpSessionPrivate *d;
};

class QMediaRtpPayloadPrivate;

class QTOPIAMEDIA_EXPORT QMediaRtpPayload
{
public:
    QMediaRtpPayload();
    QMediaRtpPayload(int id, QMediaRtpStream::Type type);
    QMediaRtpPayload(
            int id, QMediaRtpStream::Type type, const QString &encodingName, int clockRate,
            int channels);
    QMediaRtpPayload(
            int id, QMediaRtpStream::Type type, const QString &encodingName, int clockRate,
            int channels, const QMap<QString, QString> &parameters);
    QMediaRtpPayload(const QMediaRtpPayload &other);
    QMediaRtpPayload &operator =(const QMediaRtpPayload &other);
    ~QMediaRtpPayload();

    bool operator ==(const QMediaRtpPayload &other) const;
    bool operator !=(const QMediaRtpPayload &other) const;
    bool isEquivalent(const QMediaRtpPayload &other) const;

    bool isNull() const;

    int id() const;
    QMediaRtpStream::Type type() const;
    QString encodingName() const;
    int clockRate() const;
    int channels() const;

    QList< QString > parametersKeys() const;
    QString parameter(const QString &key) const;
    QMap< QString, QString > parameters() const;

    static QList<QMediaRtpPayload> commonPayloads(
            const QList<QMediaRtpPayload> &localPayloads, const QList<QMediaRtpPayload> &remotePayloads);

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QExplicitlySharedDataPointer<QMediaRtpPayloadPrivate> d;
};

Q_DECLARE_USER_METATYPE(QMediaRtpPayload);
Q_DECLARE_USER_METATYPE_ENUM(QMediaRtpStream::Type);

QDebug QTOPIAMEDIA_EXPORT operator <<(QDebug debug, const QMediaRtpPayload &filter);

#endif
