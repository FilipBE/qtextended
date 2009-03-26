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

#include "qaudiooutput.h"
#include <qtopianamespace.h>
#include <qtopialog.h>

#include <pulse/simple.h>
#include <pulse/error.h>

class QAudioOutputPrivate
{
public:
    QAudioOutputPrivate(const QByteArray &device)
    {
        m_device = device;

        err = 0;
        handle = 0;

        params.format = PA_SAMPLE_S16LE;
        params.rate = 44100;
        params.channels = 2;
    }
    ~QAudioOutputPrivate()
    {
        close();
    }

    QByteArray m_device;

    pa_sample_spec  params;
    pa_simple*      handle;
    pa_buffer_attr  attr;
    int             err;

    bool            delay;
    bool            connected;
    bool            writing;
    int             count;

    bool open()
    {
        count     = 0;

        memset(&attr,0,sizeof(attr));
        attr.tlength = pa_bytes_per_second(&params)/6;
        attr.maxlength = (attr.tlength*3)/2;
        attr.minreq = attr.tlength/50;
        attr.prebuf = (attr.tlength - attr.minreq)/4;
        attr.fragsize = attr.tlength/50;

        if(!(handle = pa_simple_new(NULL, m_device.constData(),
                        PA_STREAM_PLAYBACK, NULL,
                        "playback", &params, NULL, &attr,
                        &err))) {
            qLog(QAudioOutput) << "QAudioOutput failed to open, your pulseaudio daemon is not configured correctly";
            return false;
        }
        connected = true;
        delay     = false;
        writing   = false;

        return true;
    }

    void close()
    {
        qLog(QAudioOutput)<<"QAudioOutput close()";
        pa_simple_flush(handle, &err);
        pa_simple_free(handle);
    }

    void startOutput()
    {
    }

    void write( const char *data, qint64 len )
    {
        if(!connected) {
            count++;
            if(count > 100) open();
            return;
        } else if(!delay) {
            delay = true;
            return;
        } else
            writing = true;

        if (pa_simple_write(handle, data, (size_t)len, &err) < 0) {
            qLog(QAudioOutput) << "QAudioOutput::write err, can't write to pulseaudio daemon";
            close();
            connected = false;
        }
    }
};

QAudioOutput::QAudioOutput( const QByteArray &device, QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioOutputPrivate(device);
}

QAudioOutput::QAudioOutput( QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioOutputPrivate("default");
}

QAudioOutput::~QAudioOutput()
{
    delete d;
}

int QAudioOutput::frequency() const
{
    return d->params.rate;
}

void QAudioOutput::setFrequency( int value )
{
    d->params.rate = (uint32_t)value;
}

int QAudioOutput::channels() const
{
    return d->params.channels;
}

void QAudioOutput::setChannels( int value )
{
    d->params.channels = (uint8_t)value;
}

int QAudioOutput::bitsPerSample() const
{
    if(d->params.format == PA_SAMPLE_S16LE)
        return 16;
    if(d->params.format == PA_SAMPLE_U8)
        return 8;

    return 16;
}

void QAudioOutput::setBitsPerSample( int value )
{
    switch(value) {
        case 16:
            d->params.format = PA_SAMPLE_S16LE;
            break;
        case 8:
            d->params.format = PA_SAMPLE_U8;
    }
}

bool QAudioOutput::open( QIODevice::OpenMode mode )
{
    if ( isOpen() )
        return false;
    if ( !d->open() )
        return false;
    d->writing = true;
    setOpenMode( mode | QIODevice::Unbuffered );
    return true;
}

void QAudioOutput::close()
{
    d->close();
    setOpenMode( NotOpen );
}

bool QAudioOutput::isSequential() const
{
    return true;
}

qint64 QAudioOutput::readData( char *, qint64 )
{
    // Cannot read from audio output devices.
    return 0;
}

qint64 QAudioOutput::writeData( const char *data, qint64 len )
{
    if ( !isOpen() )
        return len;
    d->write( data, len );
    return len;
}

void QAudioOutput::deviceReady( int id )
{
    Q_UNUSED(id)
}

void QAudioOutput::deviceError( int id )
{
    Q_UNUSED(id)
}
