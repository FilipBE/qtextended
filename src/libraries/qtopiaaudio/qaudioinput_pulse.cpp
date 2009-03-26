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

#include "qaudioinput.h"

#include <QTimer>
#include <QObject>

#include <qtopianamespace.h>
#include <qtopialog.h>

#include <pulse/simple.h>
#include <pulse/error.h>

class QAudioInputPrivate
{
public:
    QAudioInputPrivate(const QByteArray &device)
    {
        err = 0;
        handle = 0;

        params.format = PA_SAMPLE_S16LE;
        params.rate = 44100;
        params.channels = 1;
        m_device = device;

        timer = 0;
        bytes = 0;
    }

    ~QAudioInputPrivate()
    {
        close();
    }

    int bytes;

    QByteArray m_device;

    pa_sample_spec  params;
    pa_simple       *handle;
    pa_buffer_attr  attr;
    int             err;

    QTimer          *timer;

    bool open( QObject *input );
    void close();
    qint64 bytesAvailable() const;
    qint64 read( char *data, qint64 maxlen );
};

bool QAudioInputPrivate::open( QObject *input )
{
    bytes = params.rate * params.channels / 10;

    if(timer)
        delete timer;
    timer = new QTimer(input);

    memset(&attr,0,sizeof(attr));
    attr.tlength = pa_bytes_per_second(&params)/6;
    attr.maxlength = (attr.tlength*3)/2;
    attr.minreq = attr.tlength/50;
    attr.prebuf = (attr.tlength - attr.minreq)/4;
    attr.fragsize = attr.tlength/50;

    if(!(handle = pa_simple_new(NULL, m_device.constData(),
                    PA_STREAM_RECORD, NULL,
                    "record", &params, NULL, &attr,
                    &err))) {
        qLog(QAudioInput) << "QAudioInput failed to open, your pulseaudio daemon is not configured correctly";
        return false;
    }
    QObject::connect(timer, SIGNAL(timeout()), input, SIGNAL(readyRead()));
    timer->start(100);
    return true;
}

void QAudioInputPrivate::close()
{
    //pa_simple_free(handle);
    handle = 0;
}

qint64 QAudioInputPrivate::bytesAvailable() const
{
    if ( !handle )
        return 0;

    return bytes;
}

qint64 QAudioInputPrivate::read( char *data, qint64 maxlen )
{
    qint64 rc=-1;

    if(handle != NULL) {
        rc = pa_simple_read(handle, data, maxlen, &err);
        if (rc < 0) {
            qLog(QAudioInput) << "QAudioInput::read err, can't read from pulseaudio daemon";
            return -1;
        }
    }
    return maxlen;
}

QAudioInput::QAudioInput( const QByteArray &device, QObject *parent) : QIODevice(parent)
{
    d = new QAudioInputPrivate(device);
}

QAudioInput::QAudioInput( QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioInputPrivate("default");
}

QAudioInput::~QAudioInput()
{
    delete d;
}

int QAudioInput::frequency() const
{
    return d->params.rate;
}

void QAudioInput::setFrequency( int value )
{
    d->params.rate = (uint32_t)value;
}

int QAudioInput::channels() const
{
    return d->params.channels;
}

void QAudioInput::setChannels( int value )
{
    d->params.channels = (uint8_t)value;
}

int QAudioInput::samplesPerBlock() const
{
    if(d->params.format == PA_SAMPLE_S16LE)
        return 16;
    if(d->params.format == PA_SAMPLE_U8)
        return 8;

    return 16;
}

void QAudioInput::setSamplesPerBlock( int value )
{
    switch(value) {
        case 16:
            d->params.format = PA_SAMPLE_S16LE;
            break;
        case 8:
            d->params.format = PA_SAMPLE_U8;
    }
}

bool QAudioInput::open( QIODevice::OpenMode mode )
{
    if ( isOpen() )
        return false;
    if ( !d->open( this ) )
        return false;
    setOpenMode( mode | QIODevice::Unbuffered );
    return true;
}

void QAudioInput::close()
{
    d->close();
    setOpenMode( NotOpen );
}

bool QAudioInput::isSequential() const
{
    return true;
}

qint64 QAudioInput::bytesAvailable() const
{
    return d->bytesAvailable();
}

bool QAudioInput::event( QEvent *ev )
{
    // Handle the async event from the Alsa library.
    if ( ev->type() == QEvent::User ) {
        emit readyRead();
        return true;
    } else {
        return QObject::event( ev );
    }
}

qint64 QAudioInput::readData( char *data, qint64 maxlen )
{
    return d->read( data, maxlen );
}

qint64 QAudioInput::writeData( const char *, qint64 len )
{
    // Cannot write to audio input devices.
    return len;
}

