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

// Local includes
#include "audioparameters_p.h"

// ============================================================================
//
// AudioParameters class
//
// ============================================================================

AudioParameters::AudioParameters()
:   mMimeType( QString() ),
    mSubFormat(),
    mFrequency( 0 ),
    mChannels( 0 )
{
}

AudioParameters::AudioParameters( const QMimeType& mimeType,
                                  const QString&   subFormat,
                                  const int        frequency,
                                  const int        channels )
:   mMimeType( mimeType ),
    mSubFormat( subFormat),
    mFrequency( frequency ),
    mChannels( channels )
{
}

const QMimeType& AudioParameters::mimeType() const
{
    return mMimeType;
}

QString AudioParameters::subFormat() const
{
    return mSubFormat;
}

int AudioParameters::frequency() const
{
    return mFrequency;
}

int AudioParameters::channels() const
{
    return mChannels;
}

void AudioParameters::setMimeType( const QMimeType& mimeType )
{
    mMimeType = mimeType;
}

void AudioParameters::setSubFormat( const QString& subFormat )
{
    mSubFormat = subFormat;
}

void AudioParameters::setFrequency( int frequency )
{
    mFrequency = frequency;
}

void AudioParameters::setChannels( int channels )
{
    mChannels = channels;
}

QDataStream& operator>>( QDataStream& stream, AudioParameters& parameters )
{
    QString mimeType;
    QString subFormat;
    int frequency;
    int channels;

    stream >> mimeType >> subFormat >> frequency >> channels;

    parameters.setMimeType( QMimeType( mimeType ) );
    parameters.setSubFormat( subFormat );
    parameters.setFrequency( frequency );
    parameters.setChannels( channels );

    return stream;
}

QDataStream& operator<<( QDataStream& stream, const AudioParameters& parameters )
{
    stream << parameters.mimeType().id()
           << parameters.subFormat()
           << parameters.frequency()
           << parameters.channels();

    return stream;
}

