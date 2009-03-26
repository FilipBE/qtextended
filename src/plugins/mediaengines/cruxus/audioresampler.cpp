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

#include "audioresampler.h"
#include <QDebug>
#include <QList>
#include <qendian.h>

namespace cruxus
{

#include "sinctable.h"

QCache <int,AudioResampler::ResampleTable> AudioResampler::m_resampleTables;


AudioResampler::AudioResampler( int srcFrequency, int srcChannels, int dstFrequency, int duration )
{
    m_srcFrequency = srcFrequency;
    m_srcChannels = srcChannels;
    m_duration = duration;
    m_dstFrequency = dstFrequency;
    m_resampleTables.setMaxCost(100);
    m_samples = m_duration * m_dstFrequency / 1000;

    memset( m_prevSamples, 0, sizeof(m_prevSamples) );
}

static inline void checkOverflow16( qint32& s )
{
    if ( s > 32767 ) s = 32767;
    if ( s < -32767 ) s = -32767;
}

int AudioResampler::resample( const qint16* srcSamples, qint16* dstSamples, int srcSamplesCount, bool mixWithExistingData )
{
    if ( m_srcFrequency == m_dstFrequency ) {
        const int samplesCount = qMin( srcSamplesCount/m_srcChannels, m_samples );
        if ( m_srcChannels == 1 ) {
            if ( mixWithExistingData ) {
                for ( int i=0; i<samplesCount; i++ ) {
                    int ch1 = dstSamples[i*2] + srcSamples[i];
                    int ch2 = dstSamples[i*2+1] + srcSamples[i];
                    checkOverflow16(ch1);
                    checkOverflow16(ch2);
                    dstSamples[i*2] = ch1;
                    dstSamples[i*2+1] = ch2;
                }
            } else {
                for ( int i=0; i<samplesCount; i++ )
                    dstSamples[i*2] = dstSamples[i*2+1] = srcSamples[i];
            }
        } else {
            if ( mixWithExistingData ) {
                for ( int i=0; i<samplesCount*2; i+=2 ) {
                    int ch1 = dstSamples[i] + srcSamples[i];
                    int ch2 = dstSamples[i+1] + srcSamples[i+1];
                    checkOverflow16(ch1);
                    checkOverflow16(ch2);
                    dstSamples[i] = ch1;
                    dstSamples[i+1] = ch2;
                }
            } else
                memcpy( dstSamples, srcSamples, samplesCount*4 );
        }

        return samplesCount*2;
    } else {
        ResampleTable *table = resampleTable( m_srcFrequency );

        const int *fromTable = table->samples.constData();
        const int *weightTable = table->weight.constData();

        qint16 delayedSrcSamples[srcSamplesCount+16*m_srcChannels];
        memcpy( delayedSrcSamples, m_prevSamples, 16*m_srcChannels*2 );
        memcpy( delayedSrcSamples+16*m_srcChannels, srcSamples, srcSamplesCount*2 );
        memcpy( m_prevSamples, srcSamples+srcSamplesCount-16*m_srcChannels, 16*m_srcChannels*2 );

        int i = 0;
        if ( m_srcChannels == 2 ) {
            for ( i=0; i<m_samples; i++ ) {
                const int from = (*(fromTable++))*2;
                const int weight = *(weightTable++);

                if ( from >= srcSamplesCount )
                    break;

                qint32 ch1 = 0;
                qint32 ch2 = 0;

                const qint16 *src = delayedSrcSamples + from;
                const qint16 *sinc = sincTable[weight];

                /*qint16 sinc_table[16];
                const qint16 *sline1 = sincTable[weight/32];
                const qint16 *sline2 = sincTable[weight/32+1];
                int k = weight % 32;

                for ( int si=0; si<16; si++ ) {
                    sinc_table[si] = int(sline1[si])*(32-k)/32 + int(sline2[si])*k/32;
                }

                const qint16 *sinc = sinc_table;*/

                int s1 = *(sinc++);
                int i1 = int(*(src++)) * s1;
                int j1 = int(*(src++)) * s1;
                int s2 = *(sinc++);
                int i2 = int(*(src++)) * s2;
                int j2 = int(*(src++)) * s2;
                int s3 = *(sinc++);
                int i3 = int(*(src++)) * s3;
                int j3 = int(*(src++)) * s3;
                int s4 = *(sinc++);
                int i4 = int(*(src++)) * s4;
                int j4 = int(*(src++)) * s4;

                s1 = *(sinc++);
                i1 += int(*(src++)) * s1;
                j1 += int(*(src++)) * s1;
                s2 = *(sinc++);
                i2 += int(*(src++)) * s2;
                j2 += int(*(src++)) * s2;
                s3 = *(sinc++);
                i3 += int(*(src++)) * s3;
                j3 += int(*(src++)) * s3;
                s4 = *(sinc++);
                i4 += int(*(src++)) * s4;
                j4 += int(*(src++)) * s4;

                s1 = *(sinc++);
                i1 += int(*(src++)) * s1;
                j1 += int(*(src++)) * s1;
                s2 = *(sinc++);
                i2 += int(*(src++)) * s2;
                j2 += int(*(src++)) * s2;
                s3 = *(sinc++);
                i3 += int(*(src++)) * s3;
                j3 += int(*(src++)) * s3;
                s4 = *(sinc++);
                i4 += int(*(src++)) * s4;
                j4 += int(*(src++)) * s4;

                s1 = *(sinc++);
                i1 += int(*(src++)) * s1;
                j1 += int(*(src++)) * s1;
                s2 = *(sinc++);
                i2 += int(*(src++)) * s2;
                j2 += int(*(src++)) * s2;
                s3 = *(sinc++);
                i3 += int(*(src++)) * s3;
                j3 += int(*(src++)) * s3;
                s4 = *(sinc++);
                i4 += int(*(src++)) * s4;
                j4 += int(*(src++)) * s4;

                ch1 = i1 + i2 + i3 + i4;
                ch2 = j1 + j2 + j3 + j4;

                ch1 /= MAX_FILTER_VALUE;
                ch2 /= MAX_FILTER_VALUE;


                if ( mixWithExistingData ) {
                    ch1 += dstSamples[i*2];
                    ch2 += dstSamples[i*2+1];
                }

                checkOverflow16(ch1);
                checkOverflow16(ch2);

                dstSamples[i*2] = ch1;
                dstSamples[i*2+1] = ch2;
            }
        }

        if ( m_srcChannels == 1 ) {
            for ( i=0; i<m_samples; i++ ) {
                const int from = *(fromTable++);
                const int weight = *(weightTable++);

                if ( from >= srcSamplesCount )
                    break;

                qint32 ch1 = 0;

                const qint16 *src = delayedSrcSamples + from;
                const qint16 *sinc = sincTable[weight];
                int i1 = int(*(src++)) * (*(sinc++));
                int i2 = int(*(src++)) * (*(sinc++));
                int i3 = int(*(src++)) * (*(sinc++));
                int i4 = int(*(src++)) * (*(sinc++));

                i1 += int(*(src++)) * (*(sinc++));
                i2 += int(*(src++)) * (*(sinc++));
                i3 += int(*(src++)) * (*(sinc++));
                i4 += int(*(src++)) * (*(sinc++));

                i1 += int(*(src++)) * (*(sinc++));
                i2 += int(*(src++)) * (*(sinc++));
                i3 += int(*(src++)) * (*(sinc++));
                i4 += int(*(src++)) * (*(sinc++));


                i1 += int(*(src++)) * (*(sinc++));
                i2 += int(*(src++)) * (*(sinc++));
                i3 += int(*(src++)) * (*(sinc++));
                i4 += int(*(src++)) * (*(sinc++));

                ch1 = i1 + i2 + i3 + i4;

                ch1 /= MAX_FILTER_VALUE;

                qint32 ch2 = ch1;


                if ( mixWithExistingData ) {
                    ch1 += dstSamples[i*2];
                    ch2 += dstSamples[i*2+1];
                }

                checkOverflow16(ch1);
                checkOverflow16(ch2);

                dstSamples[i*2] = ch1;
                dstSamples[i*2+1] = ch2;
            }
        }

        return i*2;
    }
}

AudioResampler::ResampleTable* AudioResampler::resampleTable( int srcFrequency )
{
    ResampleTable *table = m_resampleTables[srcFrequency];

    if ( !table ) {
        table = new ResampleTable;
        table->samples.resize( m_samples );
        table->weight.resize( m_samples );

        for ( int i=0; i<m_samples; i++ ) {
            int from = i*srcFrequency/m_dstFrequency;
            int weight = ((i*srcFrequency) % m_dstFrequency)*MAX_WEIGHT/m_dstFrequency;
            //Q_ASSERT( weight >= 0 );
            //Q_ASSERT( weight <= MAX_WEIGHT );

            table->samples[i] = from;
            table->weight[i] = weight;
        }

        m_resampleTables.insert( srcFrequency, table );
    }
    return table;
}

}

