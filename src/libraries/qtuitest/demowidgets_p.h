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

#ifndef DEMOWIDGETS_P_H
#define DEMOWIDGETS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QWidget>
#include <QBasicTimer>

class MouseClickDemoWidget : public QWidget
{
    Q_OBJECT
public:
    MouseClickDemoWidget();

    void start(const QPoint&,int);
    void stop();

protected:
    void paintEvent(QPaintEvent*);
    void timerEvent(QTimerEvent*);

private:
    QVector<QPoint> offsets;
    QRect bound;
    QPixmap pm;
    int tickCount;
    int pressTick;
    QBasicTimer pressTimer;
};

class MousePressDemoWidget : public QWidget
{
    Q_OBJECT
public:
    MousePressDemoWidget();

    void start(const QPoint&);
    void stop();

protected:
    void paintEvent(QPaintEvent*);
    void timerEvent(QTimerEvent*);

private:
    static QPixmap& pm();
    QBasicTimer pressTimer;
};

#endif

