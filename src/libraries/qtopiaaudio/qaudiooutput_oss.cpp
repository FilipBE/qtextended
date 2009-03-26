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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include <QFile>
#include <qtopialog.h>
#include <qtopianamespace.h>

#include "qaudiooutput.h"


/*!
    \class QAudioOutput
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule

    \brief The QAudioOutput class provides an interface for sending live audio samples to the default audio output device.

    The QAudioOutput class provides an interface for sending live audio
    samples to the default output device.  It is intended for use by
    media streaming applications that can produce a stream of 8-bit unsigned
    or 16-bit signed audio samples.  The sound output will be mixed with
    audio data from other applications.

    The usual sequence for opening the default audio output device is as follows:

    \code
    QAudioOutput *audio = new QAudioOutput();
    audio->setFrequency( 11025 );
    audio->setChannels( 1 );
    audio->setBitsPerSample( 8 );
    audio->open( QIODevice::WriteOnly );
    \endcode

    \sa QAudioInput
    \ingroup multimedia
*/

// Conversion handler for an audio output format.  Format handlers
// are responsible for converting from the supplied sample format
// to the device's supported encoding.
class AudioOutputFormatHandler
{
public:
    virtual ~AudioOutputFormatHandler() { }

    virtual unsigned int writeSize( unsigned int length ) = 0;

    virtual int convert( short *output, const short *input, int length ) = 0;
};

class MToS16AudioOutputFormatHandler : public AudioOutputFormatHandler
{
public:
    unsigned int writeSize( unsigned int length ) { return length * 2; }

    int convert( short *output, const short *input, int length );
};

int MToS16AudioOutputFormatHandler::convert( short *output, const short *input, int length )
{
    int samples = length / 2;

    for (int i = 0; i < samples; i++) {
        output[2*i + 0] = input[i];
        output[2*i + 1] = input[i];
    }
    return 2*length;
}

class QAudioOutputPrivate
{
public:
    QAudioOutputPrivate(const QByteArray &device)
    {
        frequency = 44100;
        channels = 2;
        bitsPerSample = 16;
        m_device = device;
        fd = -1;
        handler = NULL;
    }

    ~QAudioOutputPrivate()
    {
        close();
    }

    bool open()
    {
        // Open the device.
        if ( ( fd = ::open( "/dev/dsp", O_WRONLY ) ) < 0 ) {
            if ( ( fd = ::open( "/dev/dsp1", O_WRONLY ) ) < 0 ) {
	        qWarning("error opening audio devices /dev/dsp and /dev/dsp1, sending data to /dev/null instead" );
	        fd = ::open( "/dev/null", O_WRONLY );
            }
        }
        fcntl( fd, F_SETFD, 1 );

        // Set the requested audio parameters.
        int format, freq, chans;
        if ( bitsPerSample == 8 )
            format = AFMT_U8;
        else
            format = AFMT_S16_NE;
        freq = frequency;
        chans = channels;
        if ( ::ioctl( fd, SNDCTL_DSP_SETFMT, &format ) < 0 )
            perror( "SNDCTL_DSP_SETFMT" );
        if ( ::ioctl( fd, SNDCTL_DSP_SPEED, &freq ) < 0 )
            perror( "SNDCTL_DSP_SPEED" );
        if ( ::ioctl( fd, SNDCTL_DSP_CHANNELS, &chans ) < 0 )
            perror( "SNDCTL_DSP_CHANNELS" );

        if ( channels == 1 && chans == 2 ) {
            handler = new MToS16AudioOutputFormatHandler;
        }

        return true;
    }

    void close()
    {
        if ( handler ) {
            delete handler;
            handler = NULL;
        }
        if ( fd != -1 ) {
            ::ioctl( fd, SNDCTL_DSP_RESET, 0 );
            ::close( fd );
            fd = -1;
        }
    }

    void startOutput()
    {
    }

    void write( const char *data, qint64 len )
    {
        if ( fd != -1 ) {
            int convertedLen;
            char *convertedData;

            if ( handler ) {
                convertedLen = handler->writeSize( len );
                convertedData = new char[convertedLen];

                handler->convert( (short *)convertedData, (const short *)data, len );
            } else {
                convertedLen = len;
                convertedData = (char *)data;
            }

            while ( ::write( fd, convertedData, convertedLen ) < 0 ) {
                if ( errno == EINTR || errno == EWOULDBLOCK )
                    continue;
                perror( "write to audio device" );
                ::close( fd );
                fd = -1;
                break;
            }

            if ( handler ) {
                delete[] convertedData;
            }
        }
    }

    int frequency;
    int channels;
    int bitsPerSample;
    QByteArray m_device;
    int fd;
    AudioOutputFormatHandler *handler;
};

/*!
    Construct a new audio output stream and attach it to \a parent.
    The default parameters are 44100 Hz Stereo, with 16-bit samples.

    The device parameter is implementation-specific, and might
    not be honored by all implementations.  It is usually an Alsa
    device name such as \c{plughw:0,0}.  The string \c{default}
    can be passed for \a device if the client application wishes
    to use the default device and is not concerned with what
    that default device may be called.
 */
QAudioOutput::QAudioOutput( const QByteArray &device, QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioOutputPrivate(device);
}

/*!
    Construct a new audio output stream and attach it to \a parent.
    The default parameters are 44100 Hz Stereo, with 16-bit samples.
*/
QAudioOutput::QAudioOutput( QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioOutputPrivate("default");
}

/*!
    Destroy this audio output stream.
*/
QAudioOutput::~QAudioOutput()
{
    delete d;
}

/*!
    Returns the current frequency of audio samples.  The default value is 44100.

    \sa setFrequency()
*/
int QAudioOutput::frequency() const
{
    return d->frequency;
}

/*!
    Sets the frequency of audio samples to \a value.  Should be called
    before open().

    \sa frequency()
*/
void QAudioOutput::setFrequency( int value )
{
    d->frequency = value;
}

/*!
    Returns the number of playback channels.  The default value is 2.

    \sa setChannels()
*/
int QAudioOutput::channels() const
{
    return d->channels;
}

/*!
    Sets the number of playback channels to \a value.  Should be called
    before open().

    \sa channels()
*/
void QAudioOutput::setChannels( int value )
{
    d->channels = value;
}

/*!
    Returns the number of bits per sample (8 or 16).  If the value is 16, the samples
    must be signed and in host byte order.  If bitsPerSample() is 8, the samples must
    be unsigned.

    \sa setBitsPerSample()
*/
int QAudioOutput::bitsPerSample() const
{
    return d->bitsPerSample;
}

/*!
    Sets the number of bits per sample to \a value (8 or 16).  Should be
    called before open().  If the value is 16, the samples must be signed
    and in host byte order.  If bitsPerSample() is 8, the samples must be unsigned.

    \sa bitsPerSample()
*/
void QAudioOutput::setBitsPerSample( int value )
{
    d->bitsPerSample = value;
}

/*!
    Opens this audio output stream in \a mode.  Returns true if the
    audio output stream could be opened; false otherwise.
*/
bool QAudioOutput::open( QIODevice::OpenMode mode )
{
    if ( isOpen() )
        return false;
    if ( !d->open() )
        return false;
    setOpenMode( mode | QIODevice::Unbuffered );
    return true;
}

/*!
    Closes this audio output stream.
*/
void QAudioOutput::close()
{
    d->close();
    setOpenMode( NotOpen );
}

/*!
    Determines if this QIODevice is sequential.  Always returns true.
*/
bool QAudioOutput::isSequential() const
{
    return true;
}

/*!
    Reads up to \a maxlen bytes into \a data.  Not used for audio
    output devices.
*/
qint64 QAudioOutput::readData( char *, qint64 )
{
    // Cannot read from audio output devices.
    return 0;
}

/*!
    Writes \a len bytes from \a data to the audio output stream.

    If bitsPerSample() is 16, the samples must be signed and in host byte order.
    If bitsPerSample() is 8, the samples must be unsigned.
*/
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
