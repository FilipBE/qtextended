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

#ifndef TIMEPROGRESSBAR_H
#define TIMEPROGRESSBAR_H

#include <qprogressbar.h>

class TimeProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    TimeProgressBar( QWidget *parent=0 );
    ~TimeProgressBar();

    void setRecording();
    void setPlaying();

protected:
    void paintEvent( QPaintEvent *event );
    bool event( QEvent * );

private:
    int prevValue;
    QPalette origPalette;
    QPalette adjustedPalette;
    bool recording;

    void refreshPalettes();
};

#endif
