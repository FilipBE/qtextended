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
#include <qfile.h>

#include <qsoundqss_qws.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>



class QAudioOutputPrivate
{
public:
    QAudioOutputPrivate(const QByteArray &device)
    {
        frequency = 44100;
        channels = 2;
        bitsPerSample = 16;
        m_device = device;

        static int unique = 0;
        isopen = false;
        started = false;
        id = ((int)::getpid()) + unique++ * 100000;
        mediapipe = Qtopia::tempDir() + "mediapipe-" + QString::number(id);
        pipeFd = -1;
        if (!QFile::exists( mediapipe )) {
            if ( mknod( mediapipe.toLatin1().constData(), S_IFIFO | 0666, 0 ) )
                perror("mknod");
        }
    }

    ~QAudioOutputPrivate()
    {
        close();
    }

    bool open()
    {
        isopen = true;
        client.playRaw( id, mediapipe, frequency, channels,
                        bitsPerSample, QWSSoundClient::Streaming );
        pipeFd = ::open( mediapipe.toLatin1().constData() , O_WRONLY );
        if ( pipeFd < 0 ) {
            qWarning("error opening audio device pipe %s, sending data to /dev/null instead", mediapipe.toLatin1().constData());
            pipeFd = ::open("/dev/null", O_WRONLY);
        }

        return true;
    }

    void close()
    {
        if ( isopen ) {
            if ( pipeFd != -1 )
                ::close( pipeFd );
            client.stop( id );
            isopen = false;
            started = false;
            pending.clear();
            QFile::remove( mediapipe );
        }
    }

    void startOutput()
    {
        if ( isopen && pipeFd != -1 && pending.size() > 0 ) {
            if ( ::write( pipeFd, pending.constData(), pending.size() ) < 0 ) {
                perror( "write to audio pipe" );
                ::close( pipeFd );
                pipeFd = -1;
            }
            pending.clear();
            started = true;
        }
    }

    void write( const char *data, qint64 len )
    {
        if ( !isopen )
            return;
        if ( !started ) {
            // Queue the data until the pipe has been accepted by qss.
            int size = pending.size();
            pending.resize( size + (int)len );
            memcpy( pending.data() + size, data, (int)len );
        } else if ( pipeFd != -1 && len > 0 ) {
            if ( ::write( pipeFd, data, (int)len ) < 0 ) {
                perror( "write to audio pipe" );
                ::close( pipeFd );
                pipeFd = -1;
            }
        }
    }

    int frequency;
    int channels;
    int bitsPerSample;
    QByteArray m_device;
    bool isopen;
    bool started;
    int id;
    QString mediapipe;
    int pipeFd;
    QWSSoundClient client;
    QByteArray pending;
};

QAudioOutput::QAudioOutput( const QByteArray &device, QObject *parent )
    : QIODevice( parent )
{
    qLog(QAudioOutput) << "Device name ctor";
    d = new QAudioOutputPrivate(device);

    connect( &(d->client), SIGNAL(deviceReady(int)),
               this, SLOT(deviceReady(int)) );
    connect( &(d->client), SIGNAL(deviceError(int,QWSSoundClient::DeviceErrors)),
               this, SLOT(deviceError(int)) );
}

QAudioOutput::QAudioOutput( QObject *parent )
    : QIODevice( parent )
{
    qLog(QAudioOutput) << "No name ctor";

    d = new QAudioOutputPrivate("default");

    connect( &(d->client), SIGNAL(deviceReady(int)),
             this, SLOT(deviceReady(int)) );
    connect( &(d->client), SIGNAL(deviceError(int,QWSSoundClient::DeviceErrors)),
             this, SLOT(deviceError(int)) );
}

QAudioOutput::~QAudioOutput()
{
    delete d;
}

int QAudioOutput::frequency() const
{
    return d->frequency;
}

void QAudioOutput::setFrequency( int value )
{
    d->frequency = value;
}

int QAudioOutput::channels() const
{
    return d->channels;
}

void QAudioOutput::setChannels( int value )
{
    d->channels = value;
}

int QAudioOutput::bitsPerSample() const
{
    return d->bitsPerSample;
}

void QAudioOutput::setBitsPerSample( int value )
{
    d->bitsPerSample = value;
}

bool QAudioOutput::open( QIODevice::OpenMode mode )
{
    if ( isOpen() )
        return false;
    if ( !d->open() )
        return false;
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
    if ( id == d->id )
        d->startOutput();
}

void QAudioOutput::deviceError( int id )
{
    if ( id == d->id )
        d->close();
}

