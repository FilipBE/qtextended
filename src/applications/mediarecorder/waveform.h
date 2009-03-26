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
#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <qwidget.h>
#include <qpixmap.h>




class Waveform : public QWidget
{
public:
    Waveform(QWidget *parent = 0, Qt::WFlags fl = 0);
    ~Waveform();

    void changeSettings(int frequency, int channels);
    void reset();
    void newSamples(const short *buf, int len);

protected:

    void paintEvent(QPaintEvent *event);

private:
    void makePixmap();
    void draw();

    int samplesPerPixel;
    int currentValue;
    int numSamples;
    short *window;
    int windowPosn;
    int windowSize;
};


#endif

