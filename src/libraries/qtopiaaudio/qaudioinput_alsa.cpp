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
#include <qsocketnotifier.h>
#include <qevent.h>
#include <qapplication.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qfile.h>

#include "custom.h"

// Add override to your device profiles custom.h to use an alternative pcm
#ifndef INPUT_AUDIO
#define INPUT_AUDIO "default"
#endif

#define ALSA_PCM_NEW_HW_PARAMS_API  // Needed for older Alsa versions.
#include <alsa/asoundlib.h>


class QAudioInputPrivate
{
public:
    QAudioInputPrivate(const QByteArray &device)
    {
        frequency = 44100;
        channels = 2;
        samplesPerBlock = -1;
        notifier = 0;
        m_device = device;
        handle = 0;
        ahandler = 0;
        access = SND_PCM_ACCESS_RW_INTERLEAVED;
        format = SND_PCM_FORMAT_S16;
        period_time=0;
        buffer_time=500000;
    }

    ~QAudioInputPrivate()
    {
        close();
    }
    int frequency;
    int channels;
    int samplesPerBlock;

    QSocketNotifier *notifier;
    QByteArray m_device;
    snd_pcm_t *handle;
    snd_async_handler_t *ahandler;
    snd_pcm_access_t  access;
    snd_pcm_format_t  format;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t period_size;
    unsigned int buffer_time;
    unsigned int period_time;
    int sample_size;

    bool open( QObject *input );
    void close();
    qint64 bytesAvailable() const;
    qint64 read( char *data, qint64 maxlen );
};

bool QAudioInputPrivate::open( QObject *input )
{
    // Open the Alsa capture device.
    bool    rc = true;
    int     err;

    unsigned int        freakuency = frequency;

    if ((err = snd_pcm_open(&handle,
                             m_device.constData(), //"plughw:0,0"
                             SND_PCM_STREAM_CAPTURE,
                                 0/*SND_PCM_ASYNC*/)) < 0) {

        qWarning( "QAudioInput: snd_pcm_open: error %d", err);

        rc = false;
    }
    else {
        snd_pcm_hw_params_t *hwparams;

        // We want non-blocking mode.
        snd_pcm_nonblock(handle, 1);

        // Set the desired parameters.
        snd_pcm_hw_params_alloca(&hwparams);

        err = snd_pcm_hw_params_any(handle, hwparams);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_hw_params_any: err %d", err);
        }

        err = snd_pcm_hw_params_set_access(handle, hwparams,
                                           access);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_hw_params_set_access: err %d",err);
        }

        err = snd_pcm_hw_params_set_format(handle, hwparams,format);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_hw_params_set_format: err %d",err);
        }

        err = snd_pcm_hw_params_set_channels(handle,hwparams,(unsigned int)channels);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_hw_params_set_channels: err %d",err);
        }

        err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &freakuency, 0);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_hw_params_set_rate_near: err %d",err);
        }
        if(freakuency > 1.05 * frequency || freakuency < 0.95 * frequency) {
            qWarning("QAudioInput: warning, sample rate %i not supported by the hardware, using %u", frequency, freakuency);
        }

        if ( samplesPerBlock != -1 ) {
            // Set buffer and period sizes based on the supplied block size.
            sample_size = (snd_pcm_uframes_t)( samplesPerBlock * channels / 8 );
            err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &freakuency, 0);
            if ( err < 0 ) {
                qWarning( "QAudioInput: snd_pcm_hw_params_set_rate_near: err %d",err);
            }
            if(freakuency > 1.05 * frequency || freakuency < 0.95 * frequency) {
                qWarning( "QAudioInput: warning, sample rate %i not supported by the hardware, using %u", frequency, freakuency);
            }

            err = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, 0);
            if ( err < 0 ) {
                qWarning( "QAudioInput: snd_pcm_hw_params_set_buffer_time_near: err %d",err);
            }
            period_time = 1000000 * 256 / frequency;
            err = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, 0);
            if ( err < 0 ) {
                qWarning( "QAudioInput: snd_pcm_hw_params_set_period_time_near: err %d",err);
            }
        } else {
            // Use the largest buffer and period sizes we can.
            err = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, 0);
            if ( err < 0 ) {
                qWarning( "QAudioInput: snd_pcm_hw_params_set_buffer_time_near: err %d",err);
            }
            period_time = 1000000 * 256 / frequency;
            err = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, 0);
            if ( err < 0 ) {
                qWarning( "QAudioInput: snd_pcm_hw_params_set_period_time_near: err %d",err);
            }
       }

        err = snd_pcm_hw_params(handle, hwparams);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_hw_params: err %d",err);
        }

        int                  dir;
        unsigned int         vval, vval2;
        snd_pcm_access_t     aval;
        snd_pcm_format_t     fval;
        snd_pcm_subformat_t  sval;

        qLog(QAudioInput) << "PCM handle name = " << snd_pcm_name(handle);
        qLog(QAudioInput) << "PCM state = " << snd_pcm_state_name(snd_pcm_state(handle));

        snd_pcm_hw_params_get_access(hwparams,&aval);
        vval = (unsigned int)aval;
        if ( (int)vval != (int)access ) {
            qLog(QAudioInput) << QString("access type not set, want %1 got %2")
                       .arg(snd_pcm_access_name((snd_pcm_access_t)access))
                       .arg(snd_pcm_access_name((snd_pcm_access_t)vval));
            access = (snd_pcm_access_t)vval;
        }
        qLog(QAudioInput) << "access type = " << snd_pcm_access_name((snd_pcm_access_t)vval);

        snd_pcm_hw_params_get_format(hwparams, &fval);
        vval = (unsigned int)fval;
        if ( (int)vval != (int)format ) {
            qLog(QAudioInput) << QString("format type not set, want %1 got %2")
                       .arg(snd_pcm_format_name((snd_pcm_format_t)format))
                       .arg(snd_pcm_format_name((snd_pcm_format_t)vval));
            format = (snd_pcm_format_t)vval;
        }
        qLog(QAudioInput) << QString("format = '%1' (%2)")
            .arg(snd_pcm_format_name((snd_pcm_format_t)vval))
            .arg(snd_pcm_format_description((snd_pcm_format_t)vval))
            .toLatin1().constData();

        snd_pcm_hw_params_get_subformat(hwparams,&sval);
        vval = (unsigned int)sval;
        qLog(QAudioInput) << QString("subformat = '%1' (%2)")
            .arg(snd_pcm_subformat_name((snd_pcm_subformat_t)vval))
            .arg(snd_pcm_subformat_description((snd_pcm_subformat_t)vval))
            .toLatin1().constData();

        snd_pcm_hw_params_get_channels(hwparams, &vval);
        if ( (int)vval != (int)channels ) {
            qLog(QAudioInput) << QString("channels type not set, want %1 got %2").arg(channels).arg(vval);
            channels = vval;
        }
        qLog(QAudioInput) << "channels = " << vval;

        snd_pcm_hw_params_get_rate(hwparams, &vval, &dir);
        if ( (int)vval != (int)frequency ) {
            qLog(QAudioInput) << QString("frequency type not set, want %1 got %2").arg(frequency).arg(vval);
            frequency = vval;
        }
        qLog(QAudioInput) << "rate =" <<  vval << " bps";

        snd_pcm_hw_params_get_period_time(hwparams,&period_time, &dir);
        qLog(QAudioInput) << "period time =" << period_time << " us";
        snd_pcm_hw_params_get_period_size(hwparams,&period_size, &dir);
        qLog(QAudioInput) << "period size =" << (int)period_size;
        snd_pcm_hw_params_get_buffer_time(hwparams,&buffer_time, &dir);
        qLog(QAudioInput) << "buffer time =" << buffer_time;
        snd_pcm_hw_params_get_buffer_size(hwparams,(snd_pcm_uframes_t *) &buffer_size);
        qLog(QAudioInput) << "buffer size =" << (int)buffer_size;
        snd_pcm_hw_params_get_periods(hwparams, &vval, &dir);
        qLog(QAudioInput) << "periods per buffer =" << vval;
        snd_pcm_hw_params_get_rate_numden(hwparams, &vval, &vval2);
        qLog(QAudioInput) << QString("exact rate = %1/%2 bps").arg(vval).arg(vval2).toLatin1().constData();
        vval = snd_pcm_hw_params_get_sbits(hwparams);
        qLog(QAudioInput) << "significant bits =" << vval;
        snd_pcm_hw_params_get_tick_time(hwparams,&vval, &dir);
        qLog(QAudioInput) << "tick time =" << vval;
        vval = snd_pcm_hw_params_is_batch(hwparams);
        qLog(QAudioInput) << "is batch =" << vval;
        vval = snd_pcm_hw_params_is_block_transfer(hwparams);
        qLog(QAudioInput) << "is block transfer =" << vval;
        vval = snd_pcm_hw_params_is_double(hwparams);
        qLog(QAudioInput) << "is double =" << vval;
        vval = snd_pcm_hw_params_is_half_duplex(hwparams);
        qLog(QAudioInput) << "is half duplex =" << vval;
        vval = snd_pcm_hw_params_is_joint_duplex(hwparams);
        qLog(QAudioInput) << "is joint duplex =" << vval;
        vval = snd_pcm_hw_params_can_overrange(hwparams);
        qLog(QAudioInput) << "can overrange =" << vval;
        vval = snd_pcm_hw_params_can_mmap_sample_resolution(hwparams);
        qLog(QAudioInput) << "can mmap =" << vval;
        vval = snd_pcm_hw_params_can_pause(hwparams);
        qLog(QAudioInput) << "can pause =" << vval;
        vval = snd_pcm_hw_params_can_resume(hwparams);
        qLog(QAudioInput) << "can resume =" << vval;
        vval = snd_pcm_hw_params_can_sync_start(hwparams);
        qLog(QAudioInput) << "can sync start =" << vval;

        snd_pcm_sw_params_t *swparams;
        snd_pcm_sw_params_alloca(&swparams);
        err = snd_pcm_sw_params_current(handle, swparams);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_sw_params_current: err %d",err);
        }
        err = snd_pcm_sw_params_set_start_threshold(handle,swparams,period_size);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_sw_params_set_start_threshold: err %d",err);
        }
        err = snd_pcm_sw_params_set_avail_min(handle, swparams,period_size);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_sw_params_set_avail_min: err %d",err);
        }
        err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_sw_params_set_xfer_align: err %d",err);
        }
        err = snd_pcm_sw_params(handle, swparams);
        if ( err < 0 ) {
            qWarning( "QAudioInput: snd_pcm_sw_params: err %d",err);
        }

        snd_pcm_prepare(handle);
        snd_pcm_start(handle);

        int     count = snd_pcm_poll_descriptors_count(handle);
        pollfd  *pfds = new pollfd[count];

        snd_pcm_poll_descriptors(handle, pfds, count);

        for (int i = 0; i < count; ++i)
        {
            if ((pfds[i].events & POLLIN) != 0) {
                notifier = new QSocketNotifier(pfds[i].fd, QSocketNotifier::Read);
                QObject::connect(notifier,
                                 SIGNAL(activated(int)),
                                 input,
                                 SIGNAL(readyRead()));

                break;
            }
        }

        if (notifier == NULL) {
            rc = false;
        }

        delete pfds;
    }

    return rc;
}

void QAudioInputPrivate::close()
{
    delete notifier;
    notifier = NULL;

    if (handle != NULL) {
        snd_pcm_close(handle);
        handle = NULL;
        ahandler = 0;
    }
    qLog(QAudioInput) << "void QAudioInputPrivate::close()";
}

qint64 QAudioInputPrivate::bytesAvailable() const
{
    if ( !handle )
        return 0;

    snd_pcm_sframes_t frames;
    if ( snd_pcm_delay( handle, &frames ) < 0 )
        return 0;

    return snd_pcm_frames_to_bytes( handle, frames );
}

qint64 QAudioInputPrivate::read( char *data, qint64 maxlen )
{
    int     rc = -1, count=0;

    if (handle != NULL)
    {
        while(count < 5) {
            int frames = snd_pcm_bytes_to_frames(handle, (int)maxlen);
            int readFrames = snd_pcm_readi(handle, data, frames);

            if (readFrames >= 0) {
                rc = snd_pcm_frames_to_bytes(handle, readFrames);
                qLog(QAudioInput) << QString("read in bytes = %1 (frames=%2)").arg(rc).arg(readFrames).toLatin1().constData();
                break;
            }
            else if ((readFrames == -EAGAIN) || (readFrames == -EINTR)) {
                qLog(QAudioInput) << "ALSA: EAGAIN || EINTR error";
                rc = 0;
                break;
            }
            else {
                if(readFrames == -EPIPE) {
                    qLog(QAudioInput) << "ALSA: underrun!!!";
                    rc = snd_pcm_prepare(handle);
                } else if(readFrames == -ESTRPIPE) {
                    qLog(QAudioInput) << "ALSA: suspend recovery!!!!";
                    rc = snd_pcm_prepare(handle);
                }
                if(rc != 0) break;
            }
            count++;
        }
    }
    return rc;
}

QAudioInput::QAudioInput( const QByteArray &device, QObject *parent) : QIODevice(parent)
{
    d = new QAudioInputPrivate(device);
}

QAudioInput::QAudioInput( QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioInputPrivate(INPUT_AUDIO);
}

QAudioInput::~QAudioInput()
{
    delete d;
}

int QAudioInput::frequency() const
{
    return d->frequency;
}

void QAudioInput::setFrequency( int value )
{
    d->frequency = value;
}

int QAudioInput::channels() const
{
    return d->channels;
}

void QAudioInput::setChannels( int value )
{
    d->channels = value;
}

int QAudioInput::samplesPerBlock() const
{
    return d->samplesPerBlock;
}

void QAudioInput::setSamplesPerBlock( int value )
{
    d->samplesPerBlock = value;
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

