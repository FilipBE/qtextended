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
#include "wavrecord.h"

#ifdef WAVGSM_SUPPORTED
extern "C" {
#include "gsm.h"
};
#endif

//#define WAVGSM_SUPPORTED 1


const int WAV_PCM_HEADER_LEN = 44;
const int WAV_GSM_HEADER_LEN = 60;
const int WAV_MAX_HEADER_LEN = WAV_GSM_HEADER_LEN;


WavRecorderPlugin::WavRecorderPlugin()
{
    // Reset the recording state.
    device = 0;
    start = 0;
    channels = 0;
    frequency = 0;
    writtenHeader = false;
    totalBytes = 0;
    totalSamples = 0;
    headerLen = 0;
    gsmPosn = 0;
    gsmHandle = 0;
    encodeAsGsm = false;

    // Determine if we need to byte-swap samples before writing
    // them to the output device (wav is little-endian).
    union
    {
        short v1;
        char  v2[2];
    } un;
    un.v1 = 0x0102;
    byteSwap = (un.v2[0] == 0x01);
}


int WavRecorderPlugin::pluginNumFormats() const
{
#ifdef WAVGSM_SUPPORTED
    return 2;
#else
    return 1;
#endif
}


QString WavRecorderPlugin::pluginFormatName( int format ) const
{
    Q_UNUSED(format);
#ifdef WAVGSM_SUPPORTED
    if (format == 1)
        return "GSM WAV"; // No tr
    else
#endif
        return "PCM WAV"; // No tr
}


QString WavRecorderPlugin::pluginFormatTag( int format ) const
{
    Q_UNUSED(format);
#ifdef WAVGSM_SUPPORTED
    if (format == 1)
        return "gsm";
    else
#endif
        return "pcm";
}


bool WavRecorderPlugin::begin( QIODevice *_device, const QString& formatTag )
{
    // Bail out if we are already recording.
    if ( device )
        return false;

    // Bail out if the new device is invalid.
    if ( !_device )
        return false;

    // Record the device and its current position so that we
    // can seek back to the start to back-patch the header
    // when recording is finished.
    device = _device;
    start = _device->pos();

#ifdef WAVGSM_SUPPORTED
    // Set up for GSM encoding if necessary.
    encodeAsGsm = (formatTag == "gsm");
    if ( encodeAsGsm ) {
        gsmPosn = 0;
        gsmHandle = (void *)(gsm_create());
        if ( gsmHandle ) {
            int value = 1;
            gsm_option( (gsm)gsmHandle, GSM_OPT_WAV49, &value );
        }
    }
#endif
    return true;
}


bool WavRecorderPlugin::end()
{
    bool result;

    // Bail out if we were not recording.
    if ( !device )
        return false;

#ifdef WAVGSM_SUPPORTED
    // Flush the last GSM block if necessary.
    if ( encodeAsGsm && gsmPosn > 0 ) {
        memset( gsmBuffer + gsmPosn, 0, sizeof(short) * (320 - gsmPosn) );
        gsmFlush( true );
        gsm_destroy( (gsm)gsmHandle );
    }
#endif

    // Back-patch the header with the total file length.
    if ( writtenHeader ) {
        if ( !device->seek( start ) ) {
            result = false;
        } else if ( !writeHeader() ) {
            result = false;
        } else {
            result = device->seek( (int)(start + headerLen + totalBytes) );
        }
    } else {
        result = writeHeader();
    }

    // Reset the recording state for the next file.
    device = 0;
    start = 0;
    channels = 0;
    frequency = 0;
    writtenHeader = false;
    totalBytes = 0;
    totalSamples = 0;
    headerLen = 0;
    gsmPosn = 0;
    gsmHandle = 0;
    return result;
}


bool WavRecorderPlugin::setAudioChannels( int _channels )
{
    if ( device && !writtenHeader &&
         ( _channels == 1 || _channels == 2 ) ) {
        channels = _channels;
        return true;
    } else {
        return false;
    }
}


bool WavRecorderPlugin::setAudioFrequency( int _frequency )
{
    if ( device && !writtenHeader && _frequency > 0 ) {
        frequency = _frequency;
        return true;
    } else {
        return false;
    }
}


bool WavRecorderPlugin::writeAudioSamples( const short *samples, long numSamples )
{
    uint len;

    // Bail out if we are not currently recording.
    if ( !device || device->isSequential() )
        return false;

    // Write the header if necessary.
    if ( !writtenHeader ) {
        if(!writeHeader())
            return false;
        writtenHeader = true;
    }

    // Write the audio data to the output device.
    if ( !encodeAsGsm ) {

        // Encode PCM data.
        len = (uint)(numSamples * 2);
        if ( !byteSwap ) {

            // Write little-endian data directly to the device.
            if ( device->write( (const char *)samples, len ) != (int)len )
                return false;

        } else {

            // Byte-swap big-endian data before writing to the device.
            if ( writeByteSwapped( (const char *)samples, len ) != (int)len )
                return false;

        }
        totalBytes += (long)len;

    } else if ( channels == 1 ) {

        // Encode mono GSM data.
        while ( numSamples > 0 ) {
            gsmBuffer[gsmPosn++] = *samples++;
            if ( gsmPosn >= 320 ) {
                gsmFlush( false );
                gsmPosn = 0;
            }
            --numSamples;
        }

    } else {

        // Encode stereo GSM data after down-converting to mono.
        while ( numSamples >= 2 ) {
            gsmBuffer[gsmPosn++] =
                (short)((((int)(samples[0])) + ((int)(samples[1]))) / 2);
            if ( gsmPosn >= 320 ) {
                gsmFlush( false );
                gsmPosn = 0;
            }
            samples += 2;
            numSamples -= 2;
        }

    }

    return true;
}


long WavRecorderPlugin::estimateAudioBps( int frequency, int channels, const QString& formatTag )
{
    // GSM is always mono, so ignore channels when estimating its size.
    if ( formatTag == "gsm" )
        return (long)(((frequency + 319) / 320) * 65);
    else
        return (long)(frequency * channels * 2);
}


int WavRecorderPlugin::writeByteSwapped( const char *data, uint len )
{
    char buf[256];
    uint writelen, templen, posn;

    writelen = 0;
    while ( len > 0) {

        // Determine how many bytes that we can process this time.
        if (len > sizeof(buf) )
            templen = sizeof(buf);
        else
        templen = len;

        // Byte-swap the input data segument.
        for ( posn = 0; posn < templen; posn += 2 ) {
            buf[posn] = data[posn + 1];
            buf[posn + 1] = data[posn];
        }

        // Write the byte-swapped data segment to the output device.
        if ( device->write( buf, templen ) != (int)templen )
            break;

        // Advance to the next buffer segment.
        data += templen;
        len -= templen;
        writelen += templen;
    }

    return writelen;
}


// Write a little-endian short value to a buffer.
static inline void write_short(char *buf, int value)
{
    buf[0] = (char)(value);
    buf[1] = (char)(value >> 8);
}


// Write a little-endian long value to a buffer.
static inline void write_long(char *buf, long value)
{
    buf[0] = (char)(value);
    buf[1] = (char)(value >> 8);
    buf[2] = (char)(value >> 16);
    buf[3] = (char)(value >> 24);
}


bool WavRecorderPlugin::writeHeader()
{
    char header[WAV_MAX_HEADER_LEN];

    // Construct the complete wav header, up to the start of the data.
    if ( !encodeAsGsm ) {

        strncpy( header, "RIFF", 4 );
        write_long( header + 4, totalBytes + (WAV_PCM_HEADER_LEN - 8) );
        strncpy( header + 8, "WAVE", 4 );
        strncpy( header + 12, "fmt ", 4 );
        write_long( header + 16, 16 );                       // size of "fmt"
        write_short( header + 20, 1 );                       // WAVE_FORMAT_PCM
        write_short( header + 22, channels );                // nChannels
        write_long( header + 24, frequency );                // nSamplesPerSec
        write_long( header + 28, frequency * channels * 2 ); // nAvgBytesPerSec
        write_short( header + 32, channels * 2 );            // nBlockAlign
        write_short( header + 34, 16 );                      // nBitsPerSample
        strncpy( header + 36, "data", 4 ); // No tr
        write_long( header + 40, totalBytes );
        headerLen = WAV_PCM_HEADER_LEN;

    } else {

        strncpy( header, "RIFF", 4 );
        write_long( header + 4, totalBytes + (WAV_GSM_HEADER_LEN - 8) );
        strncpy( header + 8, "WAVE", 4 );
        strncpy( header + 12, "fmt ", 4 );
        write_long( header + 16, 20 );                       // size of "fmt"
        write_short( header + 20, 0x31 );                    // WAVE_FORMAT_GSM
        write_short( header + 22, 1 );                       // nChannels
        write_long( header + 24, frequency );                // nSamplesPerSec
        write_long( header + 28, frequency * 65 / 320 );     // nAvgBytesPerSec
        write_short( header + 32, 65 );                      // nBlockAlign
        write_short( header + 34, 0 );                       // nBitsPerSample
        write_short( header + 36, 2 );                       // wExtSize
        write_short( header + 38, 320 );                     // wSampsPerBlock
        strncpy( header + 40, "fact", 4 ); // No tr
        write_long( header + 44, 4 );                        // size of "fact"
        write_long( header + 48, totalSamples );             // dwSamplesWritten
        strncpy( header + 52, "data", 4 ); // No tr
        write_long( header + 56, totalBytes );
        headerLen = WAV_GSM_HEADER_LEN;

    }

    // Write the header to the output device.
    return ( device->write( header, (uint)headerLen ) == headerLen );
}


void WavRecorderPlugin::gsmFlush( bool last )
{
#ifdef WAVGSM_SUPPORTED
    gsm_byte frame[65];

    // Bail out if GSM initialization failed.
    if ( !gsmHandle )
        return;

    // Note: the encoding side of GSM encodes an even frame of 32 bytes
    // and an odd frame of 33 bytes.  But the decoding side decodes an
    // even frame of 33 bytes and an odd frame of 32 bytes.  While this
    // may seem completely illogical, if it isn't done this way then
    // it doesn't work at all!  So, if you think the code below is buggy
    // because it doesn't match decoding, then think again!

    // Encode the 32-byte even part of the frame.
    gsm_encode( (gsm)gsmHandle, gsmBuffer, frame );

    // Encode the 33-byte odd part of the frame.
    gsm_encode( (gsm)gsmHandle, gsmBuffer + 160, frame + 32 );

    // Write the frame to the device.
    device->write( (char *)frame, (uint)65 );

    // Update the byte and sample totals.
    totalBytes += 65;
    totalSamples += 320;

    // Pad the final block to an even byte boundary.
    if ( last && (totalBytes % 2) != 0 ) {
        frame[0] = 0;
        device->write( (char *)frame, (uint)1 );
        ++totalBytes;
    }
#endif
}

