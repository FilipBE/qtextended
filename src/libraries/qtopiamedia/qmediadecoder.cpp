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

#include "qmediadecoder.h"


/*!
    \class QMediaDecoder
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaDecoder class is an abstraction for decoders written for
    the Media Engine.

    \ingroup multimedia
*/


/*!
    Destroy the QMediaDecoder object.
*/

QMediaDecoder::~QMediaDecoder()
{
}

/*!
    \fn void QMediaDecoder::start();

    Start decoding data from the inputPipe

    \sa QMediaDevice
*/

/*!
    \fn void QMediaDecoder::stop();

    Stop decoding data to the outputPipe

    \sa QMediaDevice
*/

/*!
    \fn void QMediaDecoder::pause();

    Pause decoding data.
*/

/*!
    \fn quint32 QMediaDecoder::length();

    Return the length of the decoded data, to date if total not available.
*/

/*!
    \fn bool QMediaDecoder::seek(qint64 ms);

    Seek to the absolute position \a ms from the beginning of the file.
*/

/*!
    \fn void QMediaDecoder::setVolume(int volume);

    Adjust the \a volume of the data, the decoder should not attempt to adjust
    hardware volume, only amplification of the data produced by the decoder.
    The \a volume will be normalized by the system to a value between 0 and
    100, the decoder should adjust internally as necessary.
*/

/*!
    \fn int QMediaDecoder::volume();

    Return the current volume level.
*/

/*!
    \fn void QMediaDecoder::setMuted(bool mute);

    Set the \a mute state of the data, the decoder should continue to process
    audio data but not produce output, if the volume is muted.
*/

/*!
    \fn bool QMediaDecoder::isMuted();

    Return the mute state of the decoder.
*/

/*!
    \fn void QMediaDecoder::playerStateChanged(QtopiaMedia::State state);

    Signals the current \a state of the QMediaDecoder

    \sa QtopiaMedia::State
*/

/*!
    \fn void QMediaDecoder::positionChanged(quint32 ms);

    Signals a change in the current position \a ms of the media in
    milliseconds.
*/

/*!
    \fn void QMediaDecoder::lengthChanged(quint32 ms);

    Signals the length information of the media has changed to \a ms
    milliseconds.
*/

/*!
    \fn void QMediaDecoder::volumeChanged(int volume);

    Signals the \a volume of the media has changed.
*/

/*!
    \fn void QMediaDecoder::volumeMuted(bool muted);

    Signals the mute status of the volume has been set to \a muted.
*/

