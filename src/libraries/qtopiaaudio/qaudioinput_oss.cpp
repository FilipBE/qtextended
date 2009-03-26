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
#include <qtopialog.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#if defined(QTOPIA_HAVE_ALSA)
    #define ALSA_PCM_NEW_HW_PARAMS_API  // Needed for older Alsa versions.
    #include <alsa/asoundlib.h>
    #define USE_ALSA
#elif defined(QTOPIA_HAVE_OSS)
    #include <sys/soundcard.h>
    #define USE_OSS
    #define AUDIO_RECORD_SOURCE SOUND_MIXER_MIC
#else
    #define USE_NO_AUDIO
#endif

/*!
    \class QAudioInput
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule

    \brief The QAudioInput class provides an interface to read raw audio data from a microphone input.

    Client applications that need direct access to the microphone should use QAudioInput
    to capture raw audio samples for further processing.  This class abstracts the
    details of the underlying operating system, so that clients do not need to be aware
    of whether the operating system uses OSS or Alsa.

    The usual sequence for opening the microphone is as follows:

    \code
    QAudioInput *audio = new QAudioInput();
    audio->setFrequency( 11025 );
    audio->setChannels( 1 );
    if ( audio->open( QIODevice::ReadOnly ) )
        audio->read( buffer, sizeof( buffer ) );
    \endcode

    You must call read() at least once just after opening because some
    devices will not start recording until the first read is performed.

    The audio data will be normalized to signed 16-bit samples in host
    byte order, regardless of what the underlying device supports.

    Use the readyRead() signal to be notified of new input.

    \sa QAudioOutput
    \ingroup multimedia
*/

#ifdef USE_OSS

// Conversion handler for an audio input format.  Format handlers
// are responsible for converting from the device's encoding to
// normalized 16-bit host byte order samples.
class AudioFormatHandler
{
public:
    virtual ~AudioFormatHandler() { }

    // Get the read size for a specific number of raw sound samples.
    virtual unsigned int readSize( unsigned int length ) = 0;

    // Convert a buffer of audio data into raw 16-bit sound samples.
    // "length" is the number of bytes read from the device.  Returns
    // the number of raw sound samples.
    virtual int convert( short *buffer, int length ) = 0;

    // Create the appropriate conversion handler for a device format.
    static AudioFormatHandler *create( int format );

};


// Convert signed 16-bit samples to normalized raw samples.  This doesn't
// need to do anything, as the incoming data is already normalized.
class S16AudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length * 2; }
    int convert( short *, int length )           { return length / 2; }

};


// Convert signed 16-bit samples to normalized raw samples by byte-swapping.
class S16SwapAudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length * 2; }
    int convert( short *buffer, int length );

};


int S16SwapAudioFormatHandler::convert( short *buffer, int length )
{
    int result = length / 2;
    while ( length >= 2 ) {
        *buffer = (short)((*buffer << 8) | ((*buffer >> 8) & 0xFF));
        ++buffer;
        length -= 2;
    }
    return result;
}


// Convert unsigned 16-bit samples to normalized raw samples.
class U16AudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length * 2; }
    int convert( short *buffer, int length );

};


int U16AudioFormatHandler::convert( short *buffer, int length )
{
    int result = length / 2;
    while ( length >= 2 ) {
        *buffer += (short)0x8000;
        ++buffer;
        length -= 2;
    }
    return result;
}


// Convert unsigned 16-bit samples to normalized raw samples and byte-swap.
class U16SwapAudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length * 2; }
    int convert( short *buffer, int length );

};


int U16SwapAudioFormatHandler::convert( short *buffer, int length )
{
    int result = length / 2;
    while ( length >= 2 ) {
        *buffer = (short)(((*buffer << 8) | ((*buffer >> 8) & 0xFF)) + 0x8000);
        ++buffer;
        length -= 2;
    }
    return result;
}


// Convert unsigned 8-bit samples to normalized raw samples.
class U8AudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length; }
    int convert( short *buffer, int length );

};


int U8AudioFormatHandler::convert( short *buffer, int length )
{
    int result = length;
    unsigned char *buf = (((unsigned char *)buffer) + length);
    buffer += length;
    while ( length > 0 ) {
        *(--buffer) = (short)((((int)(*(--buf))) - 128) << 8);
        --length;
    }
    return result;
}


// Convert signed 8-bit samples to normalized raw samples.
class S8AudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length; }
    int convert( short *buffer, int length );

};


int S8AudioFormatHandler::convert( short *buffer, int length )
{
    int result = length;
    unsigned char *buf = (((unsigned char *)buffer) + length);
    buffer += length;
    while ( length > 0 ) {
        *(--buffer) = (short)(((int)(*(--buf))) << 8);
        --length;
    }
    return result;
}


AudioFormatHandler *AudioFormatHandler::create( int format )
{
    // Determine if the host is little-endian or big-endian.
    union
    {
        short v1;
        char  v2[2];
    } un;
    un.v1 = 0x0102;
    bool littleEndian = ( un.v2[0] == 0x02 );

    // Construct an appropriate handler from the incoming format.
    switch ( format ) {

        case AFMT_U8:
            return new U8AudioFormatHandler;

        case AFMT_S16_LE:
            if ( littleEndian )
                return new S16AudioFormatHandler;
            else
                return new S16SwapAudioFormatHandler;

        case AFMT_S16_BE:
            if ( littleEndian )
                return new S16SwapAudioFormatHandler;
            else
                return new S16AudioFormatHandler;

        case AFMT_S8:
            return new S8AudioFormatHandler;

        case AFMT_U16_LE:
            if ( littleEndian )
                return new U16AudioFormatHandler;
            else
                return new U16SwapAudioFormatHandler;

        case AFMT_U16_BE:
            if ( littleEndian )
                return new U16SwapAudioFormatHandler;
            else
                return new U16AudioFormatHandler;

        default:
            qWarning( "unknown audio input format - assuming signed 16-bit" );
            return new S16AudioFormatHandler;
    }
}


// Perform channel doubling to convert mono samples into stereo.
class MToSAudioFormatHandler : public AudioFormatHandler
{
public:

    MToSAudioFormatHandler( AudioFormatHandler *_linked ) { linked = _linked; }
    ~MToSAudioFormatHandler() { delete linked; }

    unsigned int readSize( unsigned int length )
        { return linked->readSize( length ); }

    int convert( short *buffer, int length );

private:

    AudioFormatHandler *linked;

};


int MToSAudioFormatHandler::convert( short *buffer, int length )
{
    // Convert the raw samples into their normalized 16-bit form.
    int samples = linked->convert( buffer, length );

    // Perform doubling on the the samples.
    int posn = samples * 2;
    while ( posn > 0 ) {
        posn -= 2;
        buffer[posn] = buffer[posn + 1] = buffer[posn / 2];
    }
    return samples * 2;
}


// Perform channel averaging to convert stereo samples into mono.
class SToMAudioFormatHandler : public AudioFormatHandler
{
public:

    SToMAudioFormatHandler( AudioFormatHandler *_linked ) { linked = _linked; }
    ~SToMAudioFormatHandler() { delete linked; }

    unsigned int readSize( unsigned int length )
        { return linked->readSize( length ); }

    int convert( short *buffer, int length );

private:

    AudioFormatHandler *linked;

};


int SToMAudioFormatHandler::convert( short *buffer, int length )
{
    // Convert the raw samples into their normalized 16-bit form.
    int samples = linked->convert( buffer, length );

    // Perform averaging on the the samples.
    int posn = 0;
    int limit = samples / 2;
    while ( posn < limit ) {
        buffer[posn] = (short)(((int)(buffer[posn * 2])) +
                               ((int)(buffer[posn * 2 + 1])) / 2);
        ++posn;
    }
    return limit;
}


// Resample an audio stream to a different frequency.
class ResampleAudioFormatHandler : public AudioFormatHandler
{
public:

    ResampleAudioFormatHandler( AudioFormatHandler *_linked,
                                int _from, int _to, int _channels,
                                int bufferSize );
    ~ResampleAudioFormatHandler() { delete linked; delete[] temp; }

    unsigned int readSize( unsigned int length )
        { return linked->readSize( length ); }

    int convert( short *buffer, int length );

private:

    AudioFormatHandler *linked;
    int from, to, channels;
    long samplesDue;
    long rollingLeft;
    long rollingRight;
    int numInRolling;
    short *temp;

};


ResampleAudioFormatHandler::ResampleAudioFormatHandler
        ( AudioFormatHandler *_linked, int _from, int _to,
          int _channels, int bufferSize )
{
    linked = _linked;
    from = _from;
    to = _to;
    channels = _channels;
    samplesDue = 0;
    rollingLeft = 0;
    rollingRight = 0;
    numInRolling = 0;
    temp = new short [bufferSize];
}

int ResampleAudioFormatHandler::convert( short *buffer, int length )
{
    // Convert the raw samples into their normalized 16-bit form.
    int samples = linked->convert( buffer, length );
    if ( !samples )
        return 0;

    // Resample the data.  We should probably do some kind of curve
    // fit algorithm, but that can be *very* expensive CPU-wise.
    memcpy( temp, buffer, samples * sizeof(short) );
    int inposn = 0;
    int outposn = 0;
    short left, right;
    long due = samplesDue;
    long rollLeft = rollingLeft;
    long rollRight = rollingRight;
    int num = numInRolling;
    if ( from < to ) {
        // Replicate samples to convert to a higher sample rate.
        if ( channels == 1 ) {
            while ( inposn < samples ) {
                due += to;
                left = temp[inposn++];
                while ( due >= from ) {
                    buffer[outposn++] = left;
                    due -= from;
                }
            }
        } else {
            while ( inposn < samples ) {
                due += to;
                left = temp[inposn++];
                right = temp[inposn++];
                while ( due >= from ) {
                    buffer[outposn++] = left;
                    buffer[outposn++] = right;
                    due -= from;
                }
            }
        }
    } else {
        // Average samples to convert to a lower sample rate.
        // This may lose a small number (from / to) of samples
        // off the end of the stream.
        if ( channels == 1 ) {
            while ( inposn < samples ) {
                left = temp[inposn++];
                rollLeft += (long)left;
                due += to;
                ++num;
                if ( due >= from ) {
                    buffer[outposn++] = (short)(rollLeft / num);
                    rollLeft = 0;
                    num = 0;
                    due -= from;
                }
            }
        } else {
            while ( inposn < samples ) {
                left = temp[inposn++];
                right = temp[inposn++];
                rollLeft += (long)left;
                rollLeft += (long)right;
                due += to;
                ++num;
                if ( due >= from ) {
                    buffer[outposn++] = (short)(rollLeft / num);
                    buffer[outposn++] = (short)(rollRight / num);
                    rollLeft = 0;
                    rollRight = 0;
                    num = 0;
                    due -= from;
                }
            }
        }
    }
    samples = outposn;
    samplesDue = due;
    rollingLeft = rollLeft;
    rollingRight = rollRight;
    numInRolling = num;

    // Done
    return samples;
}

#endif // USE_OSS

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
#ifdef USE_ALSA
        handle = 0;
        ahandler = 0;
#endif
#ifdef USE_OSS
        fd = -1;
        handler = 0;
#endif
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
#ifdef USE_ALSA
    snd_pcm_t *handle;
    snd_async_handler_t *ahandler;
#endif
#ifdef USE_OSS
    int fd;
    int devChannels;
    int devFrequency;
    int devBufferSize;
    AudioFormatHandler *handler;
#endif

    bool open( QObject *input );
    void close();
    qint64 bytesAvailable() const;
    qint64 read( char *data, qint64 maxlen );
};

#ifdef USE_ALSA

bool QAudioInputPrivate::open( QObject *input )
{
    // Open the Alsa capture device.
    bool    rc = true;
    int     err;

    if ((err = snd_pcm_open(&handle,
                             m_device.constData(), //"plughw:0,0"
                             SND_PCM_STREAM_CAPTURE,
                                 0/*SND_PCM_ASYNC*/)) < 0) {

        qWarning( "QAudioInput: snd_pcm_open: error %d", err);

        rc = false;
    }
    else {

        unsigned int        freakuency = frequency;
        unsigned int        buffer_time;
        unsigned int        period_time;
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_uframes_t   val;

        // We want non-blocking mode.
        snd_pcm_nonblock(handle, 1);

        // Set the desired parameters.
        snd_pcm_hw_params_alloca(&hwparams);
        snd_pcm_hw_params_any(handle, hwparams);

        snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16);
        snd_pcm_hw_params_set_channels(handle, hwparams, (unsigned int)channels);
        snd_pcm_hw_params_set_rate_near(handle, hwparams, &freakuency, 0);

        if ( samplesPerBlock != -1 ) {
            // Set buffer and period sizes based on the supplied block size.
            val = (snd_pcm_uframes_t)( samplesPerBlock / channels );
            snd_pcm_hw_params_set_period_size_near(handle, hwparams, &val, 0);
            val = (snd_pcm_uframes_t)( samplesPerBlock / channels * 2 );
            snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &val);
        } else {
            // Use the largest buffer and period sizes we can.
            snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0);
            period_time = buffer_time / 4;
            snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, 0);
            snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, 0);
        }

        snd_pcm_hw_params(handle, hwparams);

        if (qtopiaLogEnabled("QAudioInput")) {
            int                  dir;
            unsigned int         vval, vval2;
            snd_pcm_uframes_t    size;
            snd_pcm_uframes_t    frames;

            qLog(QAudioInput) << "PCM handle name =" << snd_pcm_name(handle);
            qLog(QAudioInput) << "PCM state =" << snd_pcm_state_name(snd_pcm_state(handle));
            snd_pcm_hw_params_get_access(hwparams,(snd_pcm_access_t *) &vval);
            qLog(QAudioInput) << "access type =" << snd_pcm_access_name((snd_pcm_access_t)vval);
            snd_pcm_hw_params_get_format(hwparams, (snd_pcm_format_t *) &vval);
            qLog(QAudioInput) << QString("format = '%1' (%2)").arg(snd_pcm_format_name((snd_pcm_format_t)vval))
                                                              .arg(snd_pcm_format_description((snd_pcm_format_t)vval))
                                                              .toLatin1().constData();
            snd_pcm_hw_params_get_subformat(hwparams,(snd_pcm_subformat_t *)&vval);
            qLog(QAudioInput) << QString("subformat = '%1' (%2)").arg(snd_pcm_subformat_name((snd_pcm_subformat_t)vval))
                                                                 .arg(snd_pcm_subformat_description((snd_pcm_subformat_t)vval))
                                                                 .toLatin1().constData();
            snd_pcm_hw_params_get_channels(hwparams, &vval);
            qLog(QAudioInput) << "channels =" << vval;
            snd_pcm_hw_params_get_rate(hwparams, &vval, &dir);
            qLog(QAudioInput) << "rate =" << vval << "bps";
            snd_pcm_hw_params_get_period_time(hwparams,&vval, &dir);
            qLog(QAudioInput) << "period time =" << vval << "us";
            snd_pcm_hw_params_get_period_size(hwparams,&frames, &dir);
            qLog(QAudioInput) << "period size =" << (int)frames << "frames";
            snd_pcm_hw_params_get_buffer_time(hwparams,&vval, &dir);
            qLog(QAudioInput) << "buffer time =" << vval << "us";
            snd_pcm_hw_params_get_buffer_size(hwparams,(snd_pcm_uframes_t *) &vval);
            qLog(QAudioInput) << "buffer size =" << vval << "frames";
            snd_pcm_hw_params_get_periods(hwparams, &vval, &dir);
            qLog(QAudioInput) << "periods per buffer =" << vval << "frames";
            snd_pcm_hw_params_get_rate_numden(hwparams, &vval, &vval2);
            qLog(QAudioInput) << QString("exact rate = %1/%2 bps").arg(vval).arg(vval2).toLatin1().constData();
            val = snd_pcm_hw_params_get_sbits(hwparams);
            qLog(QAudioInput) << "significant bits =" << vval;
            snd_pcm_hw_params_get_tick_time(hwparams,&vval, &dir);
            qLog(QAudioInput) << "tick time =" << vval << "us";
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
        }

        // Prepare for audio input.
        snd_pcm_prepare(handle);
        snd_pcm_start(handle);

        // listen for read events
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
    int     rc = -1;

    if (handle != NULL)
    {
    top:
        int frames = snd_pcm_bytes_to_frames(handle, (int)maxlen);
        int readFrames = snd_pcm_readi(handle, data, frames);

        if (readFrames >= 0) {
            rc = snd_pcm_frames_to_bytes(handle, readFrames);
        }
        else if (readFrames == -EPIPE) {
            snd_pcm_prepare(handle);
            // ALL ALSA examples don't do this next line
            //snd_pcm_start(handle);
            rc = 0;
            goto top;
        }
        else if (readFrames == -EAGAIN) {
            rc = 0;
        }
        else {
            close();
        }
    }

    return rc;
}

#endif // USE_ALSA

#ifdef USE_OSS

// Sound formats, for use by AudioFormatHandler::create().
#ifndef AFMT_U8
#define AFMT_U8                 0x00000008
#define AFMT_S16_LE             0x00000010
#define AFMT_S16_BE             0x00000020
#define AFMT_S8                 0x00000040
#define AFMT_U16_LE             0x00000080
#define AFMT_U16_BE             0x00000100
#endif

// The list of audio formats that we understand, in the order
// in which we probe for them.  We prefer formats that require
// very little modification by "AudioFormatHandler" instances.
static int const formatList[] = {
    AFMT_S16_NE,        // Try 16-bit native host order first.
    AFMT_S16_LE,
    AFMT_S16_BE,
    AFMT_U16_LE,
    AFMT_U16_BE,
    AFMT_U8,
    AFMT_S8
};
const int formatListSize = sizeof( formatList ) / sizeof( int );

bool QAudioInputPrivate::open( QObject *input )
{
    // Attempt to open the DSP audio device.  Try both /dev/dsp and
    // /dev/dsp1, because different machines have the audio input
    // stream on different device nodes.
    if ( ( fd = ::open( "/dev/dsp", O_RDONLY | O_NONBLOCK ) ) < 0 ) {
        if ( ( fd = ::open( "/dev/dsp1", O_RDONLY | O_NONBLOCK ) ) < 0 ) {
            qWarning( "error opening audio device /dev/dsp for input: %s", strerror( errno ) );
            return false;
        }
    }

    // Stop the file descriptor from being inherited across an exec().
#ifdef F_SETFD
    fcntl( fd, F_SETFD, 1 );
#endif

    // Search for a supported audio format.
    int formats = 0;
    if ( ioctl( fd, SNDCTL_DSP_GETFMTS, &formats ) == -1 ) {
        // If we can't get the supported formats, then we probably can't record.
        ::close( fd );
        fd = -1;
        return false;
    }
    int posn;
    int format = AFMT_S16_LE;
    for ( posn = 0; posn < formatListSize; ++posn ) {
        if ( (formats & formatList[posn]) != 0 ) {
            format = formatList[posn];
            break;
        }
    }

    // Configure the device with the recommended values,
    // and read back what the device is really capable of.
    int value = format;
    ioctl( fd, SNDCTL_DSP_SETFMT, &value );
    format = value;
    value = channels;
    ioctl( fd, SNDCTL_DSP_CHANNELS, &value );
    devChannels = value;
#ifdef QT_QWS_SL5XXX
    // The Sharp Zaurus audio input device has a bug that causes it
    // to halve the specified frequency when set to stereo, so we
    // have to double the value before setting.
    if ( channels == 2 ) {
        value = frequency * 2;
        ioctl( fd, SNDCTL_DSP_SPEED, &value );
        devFrequency = frequency;
    } else {
        value = frequency;
        ioctl( fd, SNDCTL_DSP_SPEED, &value );
        devFrequency = value;
    }
#else
    value = frequency;
    ioctl( fd, SNDCTL_DSP_SPEED, &value );
    devFrequency = value;
#endif

    // Resample the input signal if it differs by more than 50 Hz
    // from the requested frequency rate.
    int diff = (int)(devFrequency - frequency);
    bool resample = (diff < -50 || diff > 50);
    if ( !resample )
      frequency = devFrequency;

    // Create the format handler to normalize the input samples
    // to 16-bit, host byte order.
    handler = AudioFormatHandler::create( format );

    // Get the recommended buffer size from the device, in 16-bit units.
    devBufferSize = 16;
    ioctl( fd, SNDCTL_DSP_GETBLKSIZE, &devBufferSize );
    if ( devBufferSize < 16 )
        devBufferSize = 16;
    if ( format != AFMT_U8 && format != AFMT_S8 )
        devBufferSize /= 2;

    // Get the inferred buffer size for the requested frequency and channels.
    int bufferSize;
    if ( resample || channels != devChannels ) {
        int devSamples = (devBufferSize / devChannels);
        int actualSamples = (devSamples * frequency / devFrequency) + 1;
        bufferSize = actualSamples * channels;
        if ( bufferSize < devBufferSize )
            bufferSize = devBufferSize;
    } else {
        bufferSize = devBufferSize;
    }

    // Add extra audio format handlers to do channel conversion and resampling.
    if ( channels == 2 && devChannels == 1 )
        handler = new MToSAudioFormatHandler( handler );
    else if ( channels == 1 && devChannels == 2 )
        handler = new SToMAudioFormatHandler( handler );
    if ( resample && frequency != devFrequency ) {
        handler = new ResampleAudioFormatHandler
            ( handler, devFrequency, frequency, channels, bufferSize );
    }

    // Register a socket notifier to be appraised of data arrival.
    notifier = new QSocketNotifier( fd, QSocketNotifier::Read );
    QObject::connect( notifier, SIGNAL(activated(int)),
                      input, SIGNAL(readyRead()) );
    return true;
}

void QAudioInputPrivate::close()
{
    if ( notifier ) {
        delete notifier;
        notifier = 0;
    }
    if ( handler ) {
        delete handler;
        handler = 0;
    }
    if ( fd != -1) {
        ::close( fd );
        fd = -1;
    }
}

qint64 QAudioInputPrivate::bytesAvailable() const
{
    if ( fd != -1 ) {
        // See comments in Qt4's qsocketlayer_unix.cpp for the reason
        // why this ioctl call sequence is a little bizarre in structure.
        size_t nbytes = 0;
        qint64 available = 0;
        if (::ioctl( fd, FIONREAD, (char *) &nbytes ) >= 0)
            available = (qint64) *((int *) &nbytes);
        return available;
    } else {
        return 0;
    }
}

qint64 QAudioInputPrivate::read( char *data, qint64 maxlen )
{
    // Bail out if the device is not currently open.
    if ( fd == -1 ) {
        return 0;
    }

    // Read bytes from the device using its own encoding.
    // Calculate best buffer size in samples, and then convert to bytes.
    int length;
    if ( ( maxlen / 2 ) > devBufferSize ) {
        length = devBufferSize;
    } else {
        length = (int)( maxlen / 2 );
    }
    unsigned int len = handler->readSize( length );
    int result = ::read( fd, data, len );
    if ( result <= 0 ) {
        return 0;
    }

    // Convert the device's encoding into normalized sample values.
    return handler->convert( (short *)data, result ) * 2;
}

#endif // USE_OSS

#ifdef USE_NO_AUDIO

// Stub everything out if we don't have audio support at all.

bool QAudioInputPrivate::open( QObject * )
{
    return false;
}

void QAudioInputPrivate::close()
{
}

qint64 QAudioInputPrivate::bytesAvailable() const
{
    return 0;
}

qint64 QAudioInputPrivate::read( char *, qint64 )
{
    return 0;
}

#endif // USE_NO_AUDIO

/*!
    Construct a new audio input stream and attach it to \a parent.
    The device that is opened is specified by \a device.
    The default parameters are 44100 Hz Stereo, with 16-bit samples.

    The device parameter is implementation-specific, and might
    not be honored by all implementations.  It is usually an Alsa
    device name such as \c{plughw:0,0}.  The string \c{default}
    can be passed for \a device if the client application wishes
    to use the default device and is not concerned with what
    that default device may be called.
*/
QAudioInput::QAudioInput( const QByteArray &device, QObject *parent) : QIODevice(parent)
{
    d = new QAudioInputPrivate(device);
}

/*!
    Construct a new audio input stream and attach it to \a parent.
    The default parameters are 44100 Hz Stereo, with 16-bit samples.
*/
QAudioInput::QAudioInput( QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioInputPrivate("default");
}

/*!
    Destroy this audio input stream.
*/
QAudioInput::~QAudioInput()
{
    delete d;
}

/*!
    Returns the current frequency of audio samples.  The default value is 44100.

    \sa setFrequency()
*/
int QAudioInput::frequency() const
{
    return d->frequency;
}

/*!
    Sets the frequency of audio samples to \a value.  Should be called
    before open().

    \sa frequency()
*/
void QAudioInput::setFrequency( int value )
{
    d->frequency = value;
}

/*!
    Returns the number of playback channels.  The default value is 2.

    \sa setChannels()
*/
int QAudioInput::channels() const
{
    return d->channels;
}

/*!
    Sets the number of playback channels to \a value.  Should be called
    before open().

    \sa channels()
*/
void QAudioInput::setChannels( int value )
{
    d->channels = value;
}

/*!
    Returns the number of samples per block that the audio input stream
    will attempt to deliver upon each read.  The default value is -1,
    which indicates that the hardware should dictate the block size.

    \sa setSamplesPerBlock()
*/
int QAudioInput::samplesPerBlock() const
{
    return d->samplesPerBlock;
}

/*!
    Sets the number of samples per block that the audio input stream
    will attempt to deliver upon each read to \a value.  The value -1 indicates
    that the hardware should dictate the block size.  Should be called
    before open().

    \sa samplesPerBlock()
*/
void QAudioInput::setSamplesPerBlock( int value )
{
    d->samplesPerBlock = value;
}

/*!
    Opens this audio input stream in \a mode.  Returns true if the audio
    input stream could be opened; false otherwise.
*/
bool QAudioInput::open( QIODevice::OpenMode mode )
{
    if ( isOpen() )
        return false;
    if ( !d->open( this ) )
        return false;
    setOpenMode( mode | QIODevice::Unbuffered );
    return true;
}

/*!
    Closes this audio input stream.
*/
void QAudioInput::close()
{
    d->close();
    setOpenMode( NotOpen );
}

/*!
    Determines if this QIODevice is sequential.  Always returns true.
*/
bool QAudioInput::isSequential() const
{
    return true;
}

/*!
    Returns the number of bytes that are available for reading.

    \sa read()
*/
qint64 QAudioInput::bytesAvailable() const
{
    return d->bytesAvailable();
}

/*!
    \internal
*/
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

/*!
    Reads a block of raw audio samples from the input device into \a data.
    The samples are guaranteed to be signed, 16 bits in size, and in host
    byte order.  The \a maxlen value is the maximum buffer size in bytes,
    which is twice the number of logical samples.

    Returns the number of bytes read, or zero if none are currently
    available.  This function will not block.

    If the device is mono, then the number of raw samples is the same
    as the number of logical samples.  If the device is stereo, then
    the number of raw samples is twice the number of logical samples.

    This function may reduce the length to a logical device buffer size.
    Use bytesAvailable() to determine if there are still bytes left
    to be read.
*/
qint64 QAudioInput::readData( char *data, qint64 maxlen )
{
    return d->read( data, maxlen );
}

/*!
    Writes \a len bytes from \a data to this QIODevice.  Not used
    for audio input devices.
*/
qint64 QAudioInput::writeData( const char *, qint64 len )
{
    // Cannot write to audio input devices.
    return len;
}
