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

#include <QTimer>
#include <QFileInfo>

#include <qendian.h>
#include <qtopianamespace.h>
#include <qtopialog.h>

#include "wavdecoder.h"

#ifdef WAVGSM_SUPPORTED
extern "C" {
#include "gsm.h"
};

#define WAV_DECODER_BUFFER 4096
#endif

// {{{ Wave Info
namespace
{
static const char* riffId = "RIFF";
static const char* waveId = "WAVE";
static const char* fmtId = "fmt ";

struct chunk
{
    char        id[4];
    quint32     size;
};

struct RIFFHeader
{
    chunk       descriptor;
    char        type[4];
};

struct WAVEHeader
{
    chunk       descriptor;
    quint16     audioFormat;        // PCM = 1
    quint16     numChannels;
    quint32     sampleRate;
    quint32     byteRate;
    quint16     blockAlign;
    quint16     bitsPerSample;
    quint32     xFreq1;
    chunk       fact;
    quint32     xfact;
    chunk       data;
};

struct DATAHeader
{
    chunk       descriptor;
    quint8      data[];
};

struct CombinedHeader
{
    RIFFHeader  riff;
    WAVEHeader  wave;
    DATAHeader  data;
};
}
// }}}

// {{{ WavDecoderPrivate
class WavDecoderPrivate
{
public:
    bool                initialized;
    bool                muted;
    int                 volume;
    quint32             length;
    quint32             position;
    quint32             rawDataRead;
    QMediaDevice*       inputDevice;
    QtopiaMedia::State  state;
    CombinedHeader      header;
    QMediaDevice::Info  outputInfo;

#ifdef WAVGSM_SUPPORTED
    char             *input_data, *input_pos;
    int              input_length;
    char             *output_data, *output_pos;
    int              output_length;

    struct gsm_state *gsmhandle;
    gsm_signal       gsmsamples[320]; //signed short 16bit
#endif
};
// }}}

/*!
    \class WavDecoder
    \brief The WavDecoder class is used to read and PCM data from a Standard
    WAVE format data source.
*/

// {{{ WavDecoder

/*!
    Construct a WavDecoder.
*/

WavDecoder::WavDecoder():
    d(new WavDecoderPrivate)
{
    // init
    d->initialized = false;
    d->muted = false;
    d->volume = 100;
    d->length = 0;
    d->position = 0;
    d->rawDataRead = 0;
    d->state = QtopiaMedia::Stopped;

    d->outputInfo.type = QMediaDevice::Info::PCM;
}

/*!
    Destroy the WavDecoder object.
*/

WavDecoder::~WavDecoder()
{
#ifdef WAVGSM_SUPPORTED
    if(d->header.wave.audioFormat == 49)
        gsm_destroy( d->gsmhandle );
#endif
    delete d;
}

/*!
    Return information about the Wave data as well as the current volume at which the Wave
    data should be played.

    \sa QMediaDecoder
*/

QMediaDevice::Info const& WavDecoder::dataType() const
{
    return d->outputInfo;
}

/*!
    Connect to \a input as a source of data for this decoder, return true if the source
    is compatible.
*/

bool WavDecoder::connectToInput(QMediaDevice* input)
{
    if (input->dataType().type != QMediaDevice::Info::Raw)
        return false;

    d->inputDevice = input;

    return true;
}

/*!
    Disconnect from the \a input source of data.
*/

void WavDecoder::disconnectFromInput(QMediaDevice* input)
{
    Q_UNUSED(input);

    d->inputDevice = 0;
}

/*!
    Starting actively decoding the Wave data from the input QMediaDevice.
*/

void WavDecoder::start()
{
    if (!d->initialized)
    {
        if (QIODevice::open(QIODevice::ReadWrite | QIODevice::Unbuffered))
        {
            if (d->inputDevice->read((char*)&d->header, sizeof(CombinedHeader)) == sizeof(CombinedHeader))
            {
                if (memcmp(&d->header.riff.descriptor.id, riffId, 4) == 0 &&
                    memcmp(&d->header.riff.type, waveId, 4) == 0 &&
                    memcmp(&d->header.wave.descriptor.id, fmtId, 4) == 0)
                {
                    if (d->header.wave.audioFormat == 1)
                    {
                        d->outputInfo.type = QMediaDevice::Info::PCM;
                        d->outputInfo.frequency = qFromLittleEndian<quint32>(d->header.wave.sampleRate);
                        d->outputInfo.bitsPerSample = qFromLittleEndian<quint16>(d->header.wave.bitsPerSample);
                        d->outputInfo.channels = qFromLittleEndian<quint16>(d->header.wave.numChannels);
                        d->outputInfo.volume = d->muted ? 0 : d->volume;

                        d->length = quint32((double(d->header.riff.descriptor.size) /
                                        d->outputInfo.frequency /
                                        d->outputInfo.channels /
                                        (d->outputInfo.bitsPerSample / 8)) * 1000);

                        qLog(Media) << "WavDecoder::start(); Info" <<
                                    d->outputInfo.frequency <<
                                    d->outputInfo.bitsPerSample <<
                                    d->outputInfo.channels <<
                                    "length:" << d->length;

                        emit lengthChanged(d->length);

                        d->initialized = true;
#ifdef WAVGSM_SUPPORTED
                    } else if(d->header.wave.audioFormat == 49) {
                        d->inputDevice->seek(60);
                        d->input_data = (char *)malloc(WAV_DECODER_BUFFER);
                        d->input_pos = d->input_data;
                        d->input_length = 0;
                        d->output_data = (char *)malloc(WAV_DECODER_BUFFER*6);
                        d->output_pos = d->output_data;
                        d->output_length = 0;
                        d->gsmhandle = gsm_create();
                        int value = 1;
                        gsm_option( d->gsmhandle, GSM_OPT_WAV49, &value );

                        d->outputInfo.type = QMediaDevice::Info::PCM;
                        d->outputInfo.frequency = qFromLittleEndian<quint32>(d->header.wave.sampleRate);
                        d->outputInfo.bitsPerSample = 16;
                        d->outputInfo.channels = qFromLittleEndian<quint16>(d->header.wave.numChannels);
                        d->outputInfo.volume = d->volume;
                        d->length = d->inputDevice->dataType().dataSize*1000/d->header.wave.sampleRate*64/13;
                        qLog(Media) << "WavDecoder::start(); Info" <<
                                    d->outputInfo.frequency <<
                                    d->outputInfo.bitsPerSample <<
                                    d->outputInfo.channels <<
                                    "length:" << d->length;

                        emit lengthChanged(d->length);
                        d->initialized = true;
#endif
                    } else {
                        qWarning("WAV file is in %d audio format, not supported!",d->header.wave.audioFormat);
                    }
                }
            }
        }
    }

    if (d->initialized)
    {
        if (d->state == QtopiaMedia::Stopped)
            seek(0);

        d->state = QtopiaMedia::Playing;

        emit readyRead();
        emit playerStateChanged(d->state);
    }
}

/*!
    Stop decoding data from the input QMediaDevice.
*/

void WavDecoder::stop()
{
    emit playerStateChanged(d->state = QtopiaMedia::Stopped);
    seek(0);
}

/*!
    Pause decoding data from the input QMediaDevice.
*/

void WavDecoder::pause()
{
    emit playerStateChanged(d->state = QtopiaMedia::Paused);
}

/*!
    Return the length, in milliseconds of the Wave data.
*/

quint64 WavDecoder::length()
{
    return d->length;
}

/*!
    Seek to \a ms milliseconds from the beginning of that data. Returning
    true if the seek was able to be performed.
*/

bool WavDecoder::seek(qint64 ms)
{
    ms /= 1000;

#ifdef WAVGSM_SUPPORTED
    if(d->header.wave.audioFormat == 49) {
        // no seek support for wav49 format
        if((int)ms == 0) {
            d->input_length=0;
            d->output_length=0;
            d->rawDataRead = 0;
            d->inputDevice->seek(60);
            d->position = 0;
            emit positionChanged(0);
            return true;
        } else {
            d->rawDataRead = 60 + (ms*d->header.wave.sampleRate/64*13)/65*65;
            d->inputDevice->seek(d->rawDataRead);
            d->input_length=0;
            d->output_length=0;
            emit positionChanged(d->position = ms * 1000);
            return true;
        }
    }
#endif

    int     rawPos = sizeof(CombinedHeader) +
                         (ms * d->outputInfo.frequency *
                         d->outputInfo.channels *
                         (d->outputInfo.bitsPerSample / 8));

    if (d->inputDevice->seek(rawPos))
    {
        d->rawDataRead = rawPos;
        emit positionChanged(d->position = ms * 1000);

        return true;
    }

    return false;
}

/*!
    Set the volume of the Wave data to \a volume.
*/

void WavDecoder::setVolume(int volume)
{
    d->volume = qMin(qMax(volume, 0), 100);

    if (!d->muted)
        d->outputInfo.volume = d->volume;

    emit volumeChanged(d->volume);
}

/*!
    Return the current volume of the wave data.
*/

int WavDecoder::volume()
{
    return d->volume;
}

/*!
    Set the mute status to \a mute. if true the volume will be set
    to 0.
*/

void WavDecoder::setMuted(bool mute)
{
    d->outputInfo.volume = mute ? 0 : d->volume;

    emit volumeMuted(d->muted = mute);
}

/*!
    Return the mute status.
*/

bool WavDecoder::isMuted()
{
    return d->muted;
}

//private:
/*!
    \internal
*/

qint64 WavDecoder::readData(char *data, qint64 maxlen)
{
    qint64      rc = 0;
    if (maxlen > 0)
    {
        if(d->header.wave.audioFormat == 1) {
            if (d->state == QtopiaMedia::Playing)
            {
                quint32 position = quint32((double(d->rawDataRead) /
                            (double(d->outputInfo.frequency) *
                             d->outputInfo.channels *
                             (d->outputInfo.bitsPerSample / 8))) * 1000);

                if (d->position != position)
                {
                    d->position = position;
                    emit positionChanged(d->position);
                }

                rc = d->inputDevice->read(data, maxlen);

                if (rc == 0)
                    emit playerStateChanged(d->state = QtopiaMedia::Stopped);
                else
                    d->rawDataRead += rc;
            }
        }
#ifdef WAVGSM_SUPPORTED
        else if(d->header.wave.audioFormat == 49) {
            if (d->state == QtopiaMedia::Playing)
            {
                quint32 position = d->rawDataRead*1000/d->header.wave.sampleRate*64/13;
                if((int)position > 0)
                    if (d->position != position)
                    {
                        d->position = position;
                        emit positionChanged(d->position);
                    }

                if(d->output_length > (int)maxlen) {
                    memcpy(data,d->output_data,(int)maxlen);
                    memmove(d->output_data,d->output_data+(int)maxlen,
                            d->output_length-(int)maxlen);
                    d->output_length -= (int)maxlen;
                    return maxlen;
                }

                //First top up input buffer with data
                rc = d->inputDevice->read(d->input_data+d->input_length,
                        WAV_DECODER_BUFFER-d->input_length);
                d->input_length +=rc;
                d->input_pos=d->input_data;
                rc=0;

                // decode and fill output buffer with data
                d->output_pos=d->output_data+d->output_length;
                int pos = 0;
                while (((pos + 65) <= d->input_length) && (d->output_length + 640 < WAV_DECODER_BUFFER*5)) {
                    // decode first half of 65 byte GSM frame
                    gsm_decode( d->gsmhandle, (gsm_byte*)(d->input_pos), d->gsmsamples );
                    // decode second half of 65 byte GSM frame
                    gsm_decode( d->gsmhandle, (gsm_byte*)(d->input_pos+33),d->gsmsamples + 160 );
                    pos += 65;
                    d->input_pos += 65;
                    d->rawDataRead += 65;

                    for(int i=0;i<320;i++) {
                        unsigned char *c = (unsigned char *)&d->gsmsamples[i];
                        *d->output_pos++ = *c;
                        *d->output_pos++ = *(c+1);
                        d->output_length+=2;
                        if(d->outputInfo.channels==2) {
                            *d->output_pos++ = *c;
                            *d->output_pos++ = *(c+1);
                            d->output_length+=2;
                        }
                    }
                }

                // Move the unprocessed input data to the start point
                memmove(d->input_data, d->input_data + pos, d->input_length - pos);
                d->input_length -= pos;

                // Copy output data out
                if(d->output_length > (int)maxlen) {
                    memcpy(data,d->output_data,(int)maxlen);
                    memmove(d->output_data,d->output_data+(int)maxlen,
                            d->output_length-(int)maxlen);
                    d->output_length -= (int)maxlen;
                    rc = (int)maxlen;
                } else {
                    rc = d->output_length;
                    memcpy(data,d->output_data,d->output_length);
                    d->output_length = 0;
                }
                if (rc == 0)
                    emit playerStateChanged(d->state = QtopiaMedia::Stopped);
            }
        }
#endif
    }
    return rc;
}

    /*!
    \internal
*/

qint64 WavDecoder::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}
// }}}


