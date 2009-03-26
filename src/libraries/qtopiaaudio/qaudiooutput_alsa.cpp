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

#include <alsa/asoundlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "custom.h"

#ifndef ALSA_OUTPUT_NAME
#define ALSA_OUTPUT_NAME "default"
#endif


class QAudioOutputPrivate
{
public:
    QAudioOutputPrivate(const QByteArray &device)
    {
        frequency = 44100;
        channels = 2;
        bitsPerSample = 16;
        m_device = device;
        handle = 0;
        access = SND_PCM_ACCESS_RW_INTERLEAVED;
        format = SND_PCM_FORMAT_S16;
        period_time=0;
        buffer_time=100000;
    }
    ~QAudioOutputPrivate()
    {
        close();
    }

    int frequency;
    int channels;
    int bitsPerSample;
    QByteArray m_device;
    snd_pcm_t *handle;
    snd_pcm_access_t  access;
    snd_pcm_format_t  format;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t period_size;
    unsigned int buffer_time;
    unsigned int period_time;
    int periods;

    int xrun_recovery(int err)
    {
        int  count = 0;
        bool reset = false;

        if(err == -EPIPE) {
            qLog(QAudioOutput)<<"under-run occured!!!";
            err = snd_pcm_prepare(handle);
            if(err < 0)
                reset = true;

        } else if((err == -ESTRPIPE)||(err == -EIO)) {
            qLog(QAudioOutput)<<"suspend or I/O err";
            while((err = snd_pcm_resume(handle)) == -EAGAIN){
                usleep(100);
                count++;
                if(count > 5) {
                    reset = true;
                    break;
                }
            }
            if(err < 0) {
                err = snd_pcm_prepare(handle);
                if(err < 0)
                    reset = true;
            }
        }
        if(reset) {
            qLog(QAudioOutput)<<"full reset, close and re-open";
            close();
            open();
            snd_pcm_prepare(handle);
            return 0;
        }
        return err;
    }

    bool open()
    {
        // Open the Alsa playback device.
        int err=-1,count=0;
        unsigned int        freakuency = frequency;

        while((count < 5) && (err < 0)) {
            err = snd_pcm_open
                  ( &handle,  ALSA_OUTPUT_NAME, SND_PCM_STREAM_PLAYBACK, 0 );

            if(err < 0) {
                count++;
                qWarning()<<"QAudioOutput::open() err="<<err<<", count="<<count;
            }
        }
        if (( err < 0)||(handle == 0)) {
            qWarning( "QAudioOuput: snd_pcm_open: error %d", err );
            return false;
        }
        snd_pcm_nonblock( handle, 0 );

        // Set the desired HW parameters.
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_hw_params_alloca( &hwparams );

        err = snd_pcm_hw_params_any( handle, hwparams );
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_any: err %d", err);
            return false;
        }

        err = snd_pcm_hw_params_set_access( handle, hwparams, access );
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_set_access: err %d", err);
            return false;
        }

        err = snd_pcm_hw_params_set_format( handle, hwparams,
             ( bitsPerSample == 16 ? SND_PCM_FORMAT_S16
                                   : SND_PCM_FORMAT_U8 ) );
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_set_format: err %d", err);
            return false;
        }

        err = snd_pcm_hw_params_set_channels
            ( handle, hwparams, (unsigned int)channels );
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_set_channels: err %d", err);
            return false;
        }

        err = snd_pcm_hw_params_set_rate_near
            ( handle, hwparams, &freakuency, 0 );
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_set_rate_near: err %d", err);
            return false;
        }

#ifndef ALSA_BUFFER_SIZE
        err = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, 0);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_set_buffer_time_near: err %d",err);
        }
        err = snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_get_buffer_size: err %d",err);
        }
#else
        buffer_size = ALSA_BUFFER_SIZE;
        err = snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &buffer_size);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_set_buffer_time_near: err %d",err);
        }
#endif

#ifndef ALSA_PERIOD_SIZE
        period_time = buffer_time/4;
        err = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, 0);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_set_period_time_near: err %d",err);
        }
#else
        period_size = ALSA_PERIOD_SIZE;
        err = snd_pcm_hw_params_set_period_size_near(handle, hwparams, &period_size, 0);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params_set_period_size_near: err %d",err);
        }
#endif

        err = snd_pcm_hw_params(handle, hwparams);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_hw_params: err %d", err);
            return false;
        }

        int                  dir;
        unsigned int         vval, vval2;
        snd_pcm_access_t     val;
        snd_pcm_format_t     fval;
        snd_pcm_subformat_t  sval;


        qLog(QAudioOutput) << "PCM handle name =" << snd_pcm_name(handle);
        qLog(QAudioOutput) << "PCM state =" << snd_pcm_state_name(snd_pcm_state(handle));
        snd_pcm_hw_params_get_access(hwparams,&val);
        vval = (unsigned int)val;
        if ( (int)vval != (int)access ) {
            qWarning( "QAudioInput: access type not set, want %s got %s",
                    snd_pcm_access_name((snd_pcm_access_t)access),
                    snd_pcm_access_name((snd_pcm_access_t)vval) );
            access = (snd_pcm_access_t)vval;
        }
        qLog(QAudioOutput) << "access type =" << snd_pcm_access_name((snd_pcm_access_t)vval);
        snd_pcm_hw_params_get_format(hwparams, &fval);
        vval = (unsigned int)fval;
        if ( (int)vval != (int)format ) {
            qWarning( "QAudioInput: format type not set, want %s got %s",
                    snd_pcm_format_name((snd_pcm_format_t)format),
                    snd_pcm_format_name((snd_pcm_format_t)vval) );
            format = (snd_pcm_format_t)vval;
        }
        qLog(QAudioOutput) << QString("format = '%1' (%2)").arg(snd_pcm_format_name((snd_pcm_format_t)vval))
            .arg(snd_pcm_format_description((snd_pcm_format_t)vval))
            .toLatin1().constData();
        snd_pcm_hw_params_get_subformat(hwparams,&sval);
        vval = (unsigned int)sval;
        qLog(QAudioOutput) << QString("subformat = '%1' (%2)").arg(snd_pcm_subformat_name((snd_pcm_subformat_t)vval))
            .arg(snd_pcm_subformat_description((snd_pcm_subformat_t)vval))
            .toLatin1().constData();
        snd_pcm_hw_params_get_channels(hwparams, &vval);
        if ( (int)vval != (int)channels ) {
            qWarning( "QAudioInput: channels type not set, want %d got %d",channels,vval);
            channels = vval;
        }
        qLog(QAudioOutput) << "channels =" << vval;
        snd_pcm_hw_params_get_rate(hwparams, &vval, &dir);
        if ( (int)vval != (int)frequency ) {
            qWarning( "QAudioInput: frequency type not set, want %d got %d",frequency,vval);
            frequency = vval;
        }
        qLog(QAudioOutput) << "rate =" << vval << "bps";
        snd_pcm_hw_params_get_period_time(hwparams,&period_time, &dir);
        qLog(QAudioOutput) << "period time =" << period_time << "us";
        snd_pcm_hw_params_get_period_size(hwparams,&period_size, &dir);
        qLog(QAudioOutput) << "period size =" << (int)period_size << "frames";
        qLog(QAudioOutput) << "buffer time =" << (int)buffer_time << "us";
        qLog(QAudioOutput) << "buffer size =" << (int)buffer_size << "frames";
        snd_pcm_hw_params_get_periods(hwparams, &vval, &dir);
        periods = vval;
        qLog(QAudioOutput) << "periods per buffer =" << vval << "frames";
        snd_pcm_hw_params_get_rate_numden(hwparams, &vval, &vval2);
        qLog(QAudioOutput) << QString("exact rate = %1/%2 bps").arg(vval).arg(vval2).toLatin1().constData();
        vval = snd_pcm_hw_params_get_sbits(hwparams);
        qLog(QAudioOutput) << "significant bits =" << vval;
        vval = snd_pcm_hw_params_is_batch(hwparams);
        qLog(QAudioOutput) << "is batch =" << vval;
        vval = snd_pcm_hw_params_is_block_transfer(hwparams);
        qLog(QAudioOutput) << "is block transfer =" << vval;
        vval = snd_pcm_hw_params_is_double(hwparams);
        qLog(QAudioOutput) << "is double =" << vval;
        vval = snd_pcm_hw_params_is_half_duplex(hwparams);
        qLog(QAudioOutput) << "is half duplex =" << vval;
        vval = snd_pcm_hw_params_is_joint_duplex(hwparams);
        qLog(QAudioOutput) << "is joint duplex =" << vval;
        vval = snd_pcm_hw_params_can_overrange(hwparams);
        qLog(QAudioOutput) << "can overrange =" << vval;
        vval = snd_pcm_hw_params_can_mmap_sample_resolution(hwparams);
        qLog(QAudioOutput) << "can mmap =" << vval;
        vval = snd_pcm_hw_params_can_pause(hwparams);
        qLog(QAudioOutput) << "can pause =" << vval;
        vval = snd_pcm_hw_params_can_resume(hwparams);
        qLog(QAudioOutput) << "can resume =" << vval;
        vval = snd_pcm_hw_params_can_sync_start(hwparams);
        qLog(QAudioOutput) << "can sync start =" << vval;

        // Set the desired SW parameters.
        snd_pcm_sw_params_t *swparams;
        snd_pcm_sw_params_alloca(&swparams);
        err = snd_pcm_sw_params_current(handle, swparams);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_sw_params_current: err %d",err);
        }
        err = snd_pcm_sw_params_set_start_threshold(handle,swparams,buffer_size);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_sw_params_set_start_threshold: err %d",err);
        }
        err = snd_pcm_sw_params_set_stop_threshold(handle,swparams,buffer_size);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_sw_params_set_stop_threshold: err %d",err);
        }
        err = snd_pcm_sw_params_set_avail_min(handle, swparams,period_size);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_sw_params_set_avail_min: err %d",err);
        }
        err = snd_pcm_sw_params(handle, swparams);
        if ( err < 0 ) {
            qWarning( "QAudioOutput: snd_pcm_sw_params: err %d",err);
        }

        // Prepare for audio output.
        snd_pcm_prepare( handle );

        return true;
    }

    void close()
    {
        if ( handle ) {
            snd_pcm_drop( handle );
            snd_pcm_close( handle );
            handle = 0;
        }
    }

    void startOutput()
    {
    }

    void write( const char *data, qint64 len )
    {
        if ( !handle )
            return;

        int count=0;

        qLog(QAudioOutput)<<"frames to write out = "<<
            snd_pcm_bytes_to_frames( handle, (int)len )<<" ("<<len<<") bytes";

        while ( len > 0 ) {

            int err=0;
            int frames = snd_pcm_bytes_to_frames( handle, (int)len );

#ifdef ALSA_USE_AVAILABLE
            if(frames < (int)period_size)
                return;

            int available = snd_pcm_avail_update(handle);
            qLog(QAudioOutput) <<"available space = "<<available;
            if(available == 0) {
                while(available < frames) {
                    snd_pcm_wait(handle,period_size/1000);
                    usleep(period_size*10);
                    available = snd_pcm_avail_update(handle);
                    qLog(QAudioOutput) <<"->available space = "<<available;
                    count++;
                    if((count > 5)||(available < 0))
                        return;
                }
            }
#endif

            err = snd_pcm_writei( handle, data, frames );

            // Handle errors
            if ( err >= 0 ) {
                if(err == 0) count++;
                int bytes = snd_pcm_frames_to_bytes( handle, err );
                qLog(QAudioOutput) << QString("write out = %1").arg(bytes).toLatin1().constData();
                data += bytes;
                len -= bytes;
            } else {
                count++;
                qLog(QAudioOutput) <<"err = "<<err;
                err = xrun_recovery(err);
            }
            if(count > 5) {
                qLog(QAudioOutput) <<"failing to write, close() and re-open() to try and recover!";
                close();
                open();
                snd_pcm_prepare(handle);
                break;
            }
        }
    }

};

QAudioOutput::QAudioOutput( const QByteArray &device, QObject *parent )
    : QIODevice( parent )
{
    Q_UNUSED(device)

    d = new QAudioOutputPrivate("default");
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
    Q_UNUSED(id)
}

void QAudioOutput::deviceError( int id )
{
    Q_UNUSED(id)
}
