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
#include "waveform.h"

#include <qlabel.h>
#include <qpainter.h>


Waveform::Waveform(QWidget *parent, Qt::WFlags fl):
    QWidget(parent, fl),
    currentValue(0),
    numSamples(0),
    window(NULL),
    windowPosn(0),
    windowSize(100)
{
    samplesPerPixel = 8000 / (5 * windowSize);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);

    setAutoFillBackground(true);
}


void Waveform::changeSettings( int frequency, int channels )
{
    makePixmap();

    samplesPerPixel = frequency * channels / (5 * windowSize);
    if (samplesPerPixel == 0 )
        samplesPerPixel = 1;

    currentValue = 0;
    numSamples = 0;
    windowPosn = 0;

    update();
}


Waveform::~Waveform()
{
        delete[] window;
}


void Waveform::reset()
{
    makePixmap();

    currentValue = 0;
    numSamples = 0;
    windowPosn = 0;

    update();
}


void Waveform::newSamples(const short *buf, int len)
{
    // Average the incoming samples to scale them to the window.
    while ( len > 0 ) {

        currentValue += *buf++;
        --len;

        if (++numSamples >= samplesPerPixel) {

            window[windowPosn++] = (short)(currentValue / numSamples);

            if (windowPosn >= windowSize) {
                repaint();
                windowPosn = 0;
            }

            numSamples = 0;
            currentValue = 0;
        }
    }
}


void Waveform::makePixmap()
{
    delete window;

    windowSize = width();
    window = new short[windowSize];
}


void Waveform::paintEvent(QPaintEvent*)
{
    QPainter    painter(this);

    painter.setPen(Qt::green);

    if (windowPosn == 0) {

        painter.drawLine(0, height() / 2, width(), height() / 2);

    } else {

        int     middle = height() / 2;

        for (int posn = 0; posn < windowPosn; ++posn)
        {
            int mag = (window[posn] * middle) / 32768;  // SHRT_MAX + 1???

            painter.drawLine(posn, middle - mag, posn, middle + mag);
        }

        if (windowPosn < windowSize)
        {
            painter.drawLine(windowPosn, middle, windowSize, middle);
        }
    }
}

