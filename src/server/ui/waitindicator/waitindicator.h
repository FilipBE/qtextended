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

#ifndef WAITINDICATOR_H
#define WAITINDICATOR_H

#include <QWidget>
#include <QPixmap>
#include <QTimeLine>
#include <QBasicTimer>

class QValueSpaceItem;

class WaitIndicator : public QWidget
{
    Q_OBJECT
public:
    WaitIndicator();

protected:
    void paintEvent(QPaintEvent *e);
    void timerEvent(QTimerEvent *e);

private slots:
    void frameChanged(int frame);
    void finished();
    void busyChanged();

private:
    QValueSpaceItem *vsi;
    QPixmap waitIcon;
    QTimeLine timeline;
    QBasicTimer timer;
    int angle;
};

#endif
