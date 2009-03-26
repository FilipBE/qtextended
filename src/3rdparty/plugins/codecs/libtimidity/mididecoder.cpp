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
#include <QMediaDevice>

#include <qtopialog.h>

#include <timidity.h>

#include "mididecoder.h"

#define MIDI_BUFFER_SIZE    1024 * 8
#define MAX_AMPLIFICATION   200         // From libtimidity


// {{{ MidiDecoderPrivate
class MidiDecoderPrivate
{
public:
    bool                initialized;
    bool                muted;
    int                 volume;
    quint32             length;
    quint32             position;
    QMediaDevice*       inputDevice;
    MidSong*            song;
    MidIStream*         midiStream;
    MidSongOptions      options;
    QtopiaMedia::State  state;
    QMediaDevice::Info  outputInfo;
};
// }}}


// {{{ MidiDecoder
MidiDecoder::MidiDecoder():
    d(new MidiDecoderPrivate)
{
    // init
    d->initialized = false;
    d->muted = false;
    d->volume = MAX_AMPLIFICATION / 8;
    d->length = 0;
    d->position = 0;
    d->state = QtopiaMedia::Stopped;
    d->song = 0;

    d->outputInfo.type = QMediaDevice::Info::PCM;
    d->outputInfo.frequency = 44100;
    d->outputInfo.bitsPerSample = 16;
    d->outputInfo.channels = 2;

    d->outputInfo.volume = 50;
}

MidiDecoder::~MidiDecoder()
{
    if (d->song != 0)
        mid_song_free(d->song);

    delete d;
}

QMediaDevice::Info const& MidiDecoder::dataType() const
{
    return d->outputInfo;
}

bool MidiDecoder::connectToInput(QMediaDevice* input)
{
    if (input->dataType().type != QMediaDevice::Info::Raw)
        return false;

    d->inputDevice = input;

    return true;
}

void MidiDecoder::disconnectFromInput(QMediaDevice* input)
{
    Q_UNUSED(input);

    d->inputDevice = 0;
}

void MidiDecoder::start()
{
    if (!d->initialized)
    {
        QIODevice::open(QIODevice::ReadWrite | QIODevice::Unbuffered);

        d->options.rate        = d->outputInfo.frequency;
        d->options.format      = MID_AUDIO_S16LSB;  // 16
        d->options.channels    = d->outputInfo.channels;
        d->options.buffer_size = MIDI_BUFFER_SIZE / (16 * 2 / 8);

        d->midiStream = mid_istream_open_callbacks(midiReadCallback,
                                                  midiCloseCallback,
                                                  this);

        d->song = mid_song_load(d->midiStream, &d->options);

        if (d->song != 0)
        {
            d->state = QtopiaMedia::Playing;

            d->length = mid_song_get_total_time(d->song);
            emit lengthChanged(d->length);

            mid_song_set_volume(d->song, d->muted ? 0 : d->volume * 2);

            mid_song_start(d->song);

            d->initialized = true;
        }
        else {
            qLog(Media) << "MidiDecoder::start(); Failed to load MIDI file";
            d->state = QtopiaMedia::Error;
        }
    }
    else
        d->state = QtopiaMedia::Playing;

    if (d->initialized) {
        emit readyRead();
        emit playerStateChanged(d->state);
    }
}

void MidiDecoder::stop()
{
    emit playerStateChanged(d->state = QtopiaMedia::Stopped);
    seek(0);
}

void MidiDecoder::pause()
{
    emit playerStateChanged(d->state = QtopiaMedia::Paused);
}

quint64 MidiDecoder::length()
{
    return d->length;
}

bool MidiDecoder::seek(qint64 ms)
{
    if (d->initialized)
        mid_song_seek(d->song, ms);

    return true;
}

void MidiDecoder::setVolume(int volume)
{
    d->volume = volume;
    if (d->initialized && !d->muted)
        mid_song_set_volume(d->song, d->volume * 2);

    emit volumeChanged(d->volume);
}

int MidiDecoder::volume()
{
    return d->volume;
}

void MidiDecoder::setMuted(bool mute)
{
    d->muted = mute;

    if (d->initialized)
        mid_song_set_volume(d->song, mute ? 0 : d->volume * 2);

    emit volumeMuted(d->muted);
}

bool MidiDecoder::isMuted()
{
    return d->muted;
}

//private:
qint64 MidiDecoder::readData(char *data, qint64 maxlen)
{
    if (d->state != QtopiaMedia::Playing)
        return 0;

    qint64      rc = 0;

    if (maxlen > 0) {
        quint32 position = (mid_song_get_time(d->song) / 1000) * 1000;
        if (d->position != position) {
            d->position = position;
            emit positionChanged(d->position);
        }

        if ((rc = (qint64) mid_song_read_wave(d->song, data, maxlen)) == 0) {
            seek(0);
            mid_song_start(d->song);
            emit playerStateChanged(d->state = QtopiaMedia::Stopped);
        }
    }

    return rc;
}

qint64 MidiDecoder::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

size_t MidiDecoder::readCallback(void* dst, size_t itemSize, size_t numItems)
{
    return d->inputDevice->read(reinterpret_cast<char*>(dst), itemSize * numItems) / itemSize;
}

int MidiDecoder::closeCallback()
{
    return 0;
}

// static
size_t MidiDecoder::midiReadCallback(void* ctx, void* ptr, size_t size, size_t nmemb)
{
    return reinterpret_cast<MidiDecoder*>(ctx)->readCallback(ptr, size, nmemb);
}

int MidiDecoder::midiCloseCallback(void* ctx)
{
    return reinterpret_cast<MidiDecoder*>(ctx)->closeCallback();
}
// }}}

