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

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mem.h"
#include "libswscale/swscale.h"
};

#include <unistd.h>

#include <QImage>
#include <QTimer>
#include <QTime>
#include <QScreen>

#include <QMediaVideoControlServer>
#include <QDebug>

#include <private/qmediahandle_p.h>

#include "ffmpegdirectpainterwidget.h"

#include "ffmpegplaybinsession.h"

#include <QAudioOutput>

#include "qvideosurface.h"


namespace ffmpeg
{

// {{{ PlaybinSessionPrivate
class PlaybinSessionPrivate
{
public:
    bool                    haveStreamInfo;
    bool                    muted;
    quint32                 volume;
    quint32                 position;
    quint32                 pos2;
    quint32                 oldPosition;
    quint32                 length;
    int                     duration;
    QtopiaMedia::State      state;
    QMediaHandle            id;
    QString                 domain;
    QUrl                    url;
    Engine*                 engine;
    SinkWidget*             sinkWidget;
    QStringList             interfaces;
    QMediaDevice*           sink;

    QMediaVideoControlServer*   videoControlServer;

    AVFormatContext *pFormatCtx;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame         *pFrame;
    AVFrame         *pFrameRGB;

    AVCodecContext  *aCodecCtx;
    AVCodec         *aCodec;

    SwsContext      *img_convert_ctx;

    uint8_t         *buffer;
    int             numBytes;
    int             videoStream;
    int             audioStream;
    int             skipper;

    QAudioOutput*   output;
    QTimer*         timer;
    QTime*          tstamp;

    char*           input_data;
    int             input_length;

    bool            sizeset;
};
// }}}

// {{{ PlaybinSession

/*!
    \class ffmpeg::PlaybinSession
    \internal
*/

PlaybinSession::PlaybinSession
(
 Engine*        engine,
 QUuid const&   id,
 QUrl const&    url,
  QMediaDevice*          sink
):
    d(new PlaybinSessionPrivate)
{
    d->img_convert_ctx = 0;
    d->haveStreamInfo = false;
    d->muted = false;
    d->volume = 100;
    d->position = 0;
    d->oldPosition = 0;
    d->length  = -1;
    d->duration = 0;
    d->skipper = 0;
    d->state = QtopiaMedia::Stopped;
    d->engine = engine;
    d->id = QMediaHandle(id);
    d->url = url;
    d->sinkWidget = 0;
    d->videoControlServer = 0;

    d->interfaces << "Basic";
    d->sink = sink;
    d->input_data = (char *)malloc(8096*4);
    d->input_length = 0;

    d->output = new QAudioOutput(this);
    d->timer = new QTimer(this);

    d->sizeset = false;

    d->sinkWidget = new DirectPainterWidget;

    d->tstamp = new QTime(0,0,0);

    readySession();
}

void PlaybinSession::readFrame()
{
    int   err=0,i=0,skip=1;
    int   frameFinished;

    bool          v_out = false;
    QVideoFrame*  f = 0;

    AVPacket packet;

    int out_data = d->output->frequency() * d->output->channels() / 2;
    if((d->tstamp->elapsed() > 250) && ((int)d->pos2 < d->tstamp->elapsed()+500)) {
        out_data = out_data + out_data/5;
    } else if((d->tstamp->elapsed() > 250) && ((int)d->pos2 > d->tstamp->elapsed()+500)) {
        out_data = out_data - out_data/10;
    }
    err = av_read_frame(d->pFormatCtx, &packet);
    while(err>=0) {
        i++;
        // Is this a packet from the video stream?
        if(packet.stream_index==d->videoStream) {
            // Decode video frame
            avcodec_decode_video(d->pCodecCtx, d->pFrame, &frameFinished,
                    packet.data, packet.size);

            // Did we get a video frame?
            if(frameFinished) {
                if(skip < 2) {
                    bool                     match  = false;

                    uchar *pY = d->pFrame->data[0];
                    uchar* pU = d->pFrame->data[1];
                    uchar* pV = d->pFrame->data[2];

                    QVideoFrame::PixelFormat format = QVideoFrame::Format_RGB565;
                    QVideoFormatList         have;
                    QVideoFormatList         want;

                    have = d->sinkWidget->videoSurface()->supportedFormats();
                    want = d->sinkWidget->videoSurface()->preferredFormats();

                    for(int j = 0;j < want.size(); ++j) {
                        if(match) break;
                        for(int i = 0;i < have.size(); ++i) {
                            if(have[i] == want[j]) {
                                format = (QVideoFrame::PixelFormat)want[j];
                                match = true;
                                break;
                            }
                        }
                    }
                    switch(format) {
                        case QVideoFrame::Format_YUV420P:
                            f = new QVideoFrame( QVideoFrame::Format_YUV420P,
                                    QSize( d->pCodecCtx->width, d->pCodecCtx->height ),
                                    pY, pU, pV, d->pFrame->linesize[0],
                                    d->pFrame->linesize[1],
                                    d->pFrame->linesize[2]);
                            break;
                        default:
                            // Convert the image from its native format to RGB
                            if(d->img_convert_ctx == NULL) {
                                d->img_convert_ctx = sws_getContext(d->pCodecCtx->width,
                                        d->pCodecCtx->height, d->pCodecCtx->pix_fmt,
                                        d->pCodecCtx->width, d->pCodecCtx->height,
                                        PIX_FMT_RGB565, SWS_FAST_BILINEAR,
                                        NULL,NULL,NULL);
                            }
                            if(d->img_convert_ctx == NULL) {
                                qWarning("Cannot initialize the conversion context!");
                                return;
                            }
                            sws_scale(d->img_convert_ctx, &d->pFrame->data[0],
                                    d->pFrame->linesize, 0,
                                    d->pCodecCtx->height,
                                    &d->pFrameRGB->data[0],
                                    d->pFrameRGB->linesize);
                            QImage image(d->pFrameRGB->data[0],
                                    d->pCodecCtx->width,d->pCodecCtx->height,
                                    QImage::Format_RGB16);
                            if(!f) delete f;
                            f = new QVideoFrame(image);
                            break;
                    };
                    v_out = true;

                    if(skip) skip++;
                } else {
                    skip = 1;
                }
            }
        } else if(packet.stream_index==d->audioStream) {
            if((int)d->length == -1) {
                d->length =  d->pFormatCtx->duration/(AV_TIME_BASE/1000);
                emit lengthChanged(d->length);
            }

            static uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];

            int len1, data_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;

            len1 = avcodec_decode_audio2(d->aCodecCtx, (int16_t *)audio_buf,
                    &data_size,packet.data, packet.size);

            if(len1 > 0) {
                if(data_size <= 0) {
                    /* No data yet, get more frames */
                    qWarning("No data yet, get more frames");
                } else {
                    /* We have data, return it and come back for more later */
                    int spms = d->output->frequency()*d->output->channels()*2/1000;
                    d->duration=d->duration+data_size;
                    if(d->duration > spms) {
                        int inc = d->duration/spms;
                        d->position = d->position + inc;
                        d->pos2 = d->pos2 + inc;
                        d->duration = d->duration - spms*inc;
                        if(d->oldPosition/1000 != d->position/1000) {
                            d->oldPosition = d->position;
                            emit positionChanged(d->position);
                        }
                    }
                    d->output->write((const char*)audio_buf,(qint64)data_size);
                    if(v_out) {
                        v_out = false;
                        d->sinkWidget->paint(*f);
                    }
                    d->input_length=d->input_length+data_size;
                }
            }
        }
        if(d->input_length > out_data) break;
        err = av_read_frame(d->pFormatCtx, &packet);
    }
    if(err < 0) {
        stop();
    } else
        d->input_length=d->input_length-out_data;

    av_free_packet(&packet);
}

PlaybinSession::~PlaybinSession()
{
    delete d->sinkWidget;
    delete d->videoControlServer;

    stop();

    avcodec_close(d->pCodecCtx);
    avcodec_close(d->aCodecCtx);
    av_close_input_file(d->pFormatCtx);

    av_free(d->buffer);
    av_free(d->pFrameRGB);
    av_free(d->pFrame);

    delete d;
}

bool PlaybinSession::isValid() const
{
    return true;
}

void PlaybinSession::start()
{
    d->tstamp->restart();
    d->timer->start(120);

    connect(d->timer, SIGNAL(timeout()), this, SLOT(readFrame()));

    d->output->open(QIODevice::WriteOnly | QIODevice::Unbuffered);

    d->tstamp->restart();
    d->pos2 = 0;

    readFrame();
    emit playerStateChanged(d->state = QtopiaMedia::Playing);
}

void PlaybinSession::pause()
{
    emit playerStateChanged(d->state = QtopiaMedia::Paused);
    d->timer->stop();
    //d->output->close();
}

void PlaybinSession::stop()
{
    emit playerStateChanged(d->state = QtopiaMedia::Stopped);
    d->timer->stop();
    //d->output->close();
}

void PlaybinSession::suspend()
{
    pause();
}

void PlaybinSession::resume()
{
    start();
}

void PlaybinSession::seek(quint32 ms)
{
    /* TODO
    */
}

quint32 PlaybinSession::length()
{
    return d->length;
}

void PlaybinSession::setVolume(int volume)
{
    /* TODO
    */
}

int PlaybinSession::volume() const
{
    return int(d->volume * 10);
}

void PlaybinSession::setMuted(bool mute)
{
    /* TODO
    */
}

bool PlaybinSession::isMuted() const
{
    return d->muted;
}

QtopiaMedia::State PlaybinSession::playerState() const
{
    return d->state;
}

QString PlaybinSession::errorString()
{
    return QString();
}

void PlaybinSession::setDomain(QString const& domain)
{
    d->domain = domain;
}

QString PlaybinSession::domain() const
{
    return d->domain;
}

QStringList PlaybinSession::interfaces()
{
    return d->interfaces;
}

QString PlaybinSession::id() const
{
    return d->id.toString();
}

QString PlaybinSession::reportData() const
{
    return d->url.toString();
}

void PlaybinSession::getStreamsInfo()
{
    // Audio/Video
    if (!d->haveStreamInfo)
    {
        /* TODO
        */
        d->haveStreamInfo = true;
    }
}

void PlaybinSession::readySession()
{
    QString fileName = d->url.toString();

    // Open media file
    if(av_open_input_file(&d->pFormatCtx, fileName.toLocal8Bit().constData(), NULL, 0, NULL)!=0) {
        qWarning()<<"Couldn't open file "<<fileName;
        d->pFormatCtx=0;
        return;
    }

    // Retrieve stream information
    if(av_find_stream_info(d->pFormatCtx)<0) {
        qWarning("Couldn't find stream information");
        av_close_input_file(d->pFormatCtx);
        d->pFormatCtx=0;
        return;
    }
    dump_format(d->pFormatCtx, 0, fileName.toLocal8Bit().constData(), false);

    // Find the first video stream
    int i;
    d->videoStream=-1;
    d->audioStream=-1;

    for(i=0; i<(int)d->pFormatCtx->nb_streams; i++) {
        if((d->pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) &&
                (d->videoStream < 0))
            d->videoStream=i;

        if((d->pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO) &&
                (d->audioStream < 0))
            d->audioStream=i;
    }
    if(d->videoStream==-1) {
        qWarning("Didn't find a video stream");
    } else {
        d->pCodecCtx=d->pFormatCtx->streams[d->videoStream]->codec;
        // Find the decoder for the video stream
        d->pCodec=avcodec_find_decoder(d->pCodecCtx->codec_id);
        if(d->pCodec==NULL) {
            qWarning("Codec for video not found");
            return;
        }
        // Open codec
        if(avcodec_open(d->pCodecCtx, d->pCodec)<0) {
            qWarning("Could not open video codec");
            return;
        }

        // Allocate video frame
        d->pFrame=avcodec_alloc_frame();

        // Allocate an AVFrame structure
        d->pFrameRGB=avcodec_alloc_frame();
        if(d->pFrameRGB==NULL) {
            return;
        }

        // Determine required buffer size and allocate buffer
        d->numBytes=avpicture_get_size(PIX_FMT_RGB565, d->pCodecCtx->width,
                d->pCodecCtx->height);
        d->buffer=(uint8_t *)av_malloc(d->numBytes*sizeof(uint8_t));

        // Assign appropriate parts of buffer to image planes in pFrameRGB
        avpicture_fill((AVPicture *)d->pFrameRGB, d->buffer, PIX_FMT_RGB565,
                d->pCodecCtx->width, d->pCodecCtx->height);
    }

    if(d->audioStream==-1) {
        qWarning("Didn't find a audio stream");
    } else {
        d->aCodecCtx=d->pFormatCtx->streams[d->audioStream]->codec;
        d->output->setFrequency(d->aCodecCtx->sample_rate);
        d->output->setChannels(d->aCodecCtx->channels);

        // Find the decoder for the audio stream
        d->aCodec=avcodec_find_decoder(d->aCodecCtx->codec_id);
        if(d->aCodec==NULL) {
            qWarning("Codec for audio not found");
            return;
        }
        // Open codec
        if(avcodec_open(d->aCodecCtx, d->aCodec)<0) {
            qWarning("Could not open audio codec");
            return;
        }

    }

    if(d->videoStream != -1) {
        // Pre-set video element, even if no video
        d->videoControlServer = new QMediaVideoControlServer(d->id,0,this);
        //d->videoControlServer->setVideoDelegate(d->sinkWidget->videoSurface());
        d->videoControlServer->setRenderTarget(d->sinkWidget->windowId());
        d->interfaces << "Video";

        QVideoSurface *surface = d->sinkWidget->videoSurface();
        surface->setRotation(d->videoControlServer->videoRotation());
        surface->setScaleMode(d->videoControlServer->videoScaleMode());
        connect( d->videoControlServer, SIGNAL(rotationChanged(QtopiaVideo::VideoRotation)),
                surface, SLOT(setRotation(QtopiaVideo::VideoRotation)) );
        connect( d->videoControlServer, SIGNAL(scaleModeChanged(QtopiaVideo::VideoScaleMode)),
                surface, SLOT(setScaleMode(QtopiaVideo::VideoScaleMode)) );

        emit interfaceAvailable("Video");
        d->haveStreamInfo = true;
    }
}

// }}}

}   // ns ffmpeg

