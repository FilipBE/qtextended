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

#ifndef EXAMPLEMOUSEHANDLER_H
#define EXAMPLEMOUSEHANDLER_H

#define TS_SAMPLES   5

#include <QtGui/QWSCalibratedMouseHandler>

class QSocketNotifier;
class ExampleMouseHandler : public QObject, public QWSCalibratedMouseHandler {
    Q_OBJECT
public:
    ExampleMouseHandler(const QString &device = QString("/dev/input/event1"));
    ~ExampleMouseHandler();

    void suspend();
    void resume();

private:
    int  nX, nY;
    int  sx[TS_SAMPLES+3], sy[TS_SAMPLES+3];
    int  index_x1, index_x2, index_y1, index_y2, min_x, min_y;
    int  mouseIdx;
    static const int mouseBufSize = 2048;
    uchar mouseBuf[mouseBufSize];
    QPoint oldmouse;

    QSocketNotifier *m_notify;
    int  mouseFd;

private Q_SLOTS:
    void readMouseData();
};

#endif
