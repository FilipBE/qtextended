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

#ifndef AUDIORESAMPLER_H
#define AUDIORESAMPLER_H

#include <QByteArray>
#include <QVector>
#include <QCache>

namespace cruxus
{

class AudioResampler
{
public:
    AudioResampler( int srcFrequency, int srcChannels, int dstFrequency, int duration );

    int resample( const qint16* srcSamples, qint16* dstBuffer, int srcSamplesCount, bool mixWithExistingData );

private:
    int m_srcFrequency;
    int m_srcChannels;
    int m_dstFrequency;
    int m_duration;
    int m_samples;//per channel

    qint16 m_prevSamples[16*2];

    struct ResampleTable {
        QVector<int> samples;
        QVector<int> weight;
    };

    ResampleTable* resampleTable( int frequency );
    static QCache <int,ResampleTable> m_resampleTables;
};

};

#endif

