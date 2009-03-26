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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "demowidgets_p.h"

#include <QSettings>
#include <QPainter>
#include <QTimerEvent>

static const int npressticks=9;

MouseClickDemoWidget::MouseClickDemoWidget()
: QWidget(0, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint),
    pressTick(0)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    // This allows the widget to be on top of server widgets (see windowmanagement.cpp)
    setWindowTitle("_allow_on_top_");

    QSettings cfg(QLatin1String("Trolltech"),QLatin1String("presstick"));
    cfg.beginGroup(QLatin1String("PressTick"));
    pm = QPixmap(QLatin1String(":image/")+cfg.value(QLatin1String("Image")).toString());
    pm = pm.scaledToHeight(pm.height()*2);
    cfg.endGroup();
    offsets.resize(npressticks);
    tickCount = 0;
    for (int i = 0; i < npressticks; i++) {
        cfg.beginGroup(QLatin1String("Tick")+QString::number(i));
        if (cfg.contains("Dx") || cfg.contains("Dy")) {
            int dx = cfg.value(QLatin1String("Dx")).toInt()*2;
            int dy = cfg.value(QLatin1String("Dy")).toInt()*2;
            offsets[i] = QPoint(dx, dy);
            bound |= QRect(offsets[i], pm.size());
            tickCount++;
        }
        cfg.endGroup();
    }
}

void MouseClickDemoWidget::start(const QPoint &pos, int timeout)
{
    pressTick = npressticks;
    pressTimer.start(timeout/pressTick, this); // #### pref.
    setGeometry(pos.x()+bound.x(), pos.y()+bound.y(), bound.width(), bound.height());
    raise();
}

void MouseClickDemoWidget::stop()
{
    pressTimer.stop();
    hide();
}

void MouseClickDemoWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (pressTick < tickCount) {
        int dx = offsets[pressTick].x();
        int dy = offsets[pressTick].y();
        QPainter p(this);
        if (pressTick == tickCount-1) {
            p.setCompositionMode(QPainter::CompositionMode_Clear);
            p.eraseRect(rect());
            p.setCompositionMode(QPainter::CompositionMode_Source);
        }
        p.drawPixmap(bound.width()/2+dx, bound.height()/2+dy,pm);
    }
}

void MouseClickDemoWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == pressTimer.timerId()) {
        if (pressTick) {
            pressTick--;
            if (pressTick < tickCount) {
                show();
                raise();
                repaint();
            }
        } else {
            // Right pressed
            pressTimer.stop();
            hide();
        }
    }
}

MousePressDemoWidget::MousePressDemoWidget()
: QWidget(0, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    // This allows the widget to be on top of server widgets (see windowmanagement.cpp)
    setWindowTitle("_allow_on_top_");

    QSettings cfg(QLatin1String("Trolltech"),QLatin1String("presstick"));
    cfg.beginGroup(QLatin1String("PressTick"));
    if (pm().isNull()) {
        pm() = QPixmap(QLatin1String(":image/")+cfg.value(QLatin1String("Image")).toString());
        pm() = pm().scaledToHeight(pm().height()*2);
    }
}

void MousePressDemoWidget::start(const QPoint &pos)
{
    pressTimer.start(2000, this);
    setGeometry(pos.x()+pm().rect().x(), pos.y()+pm().rect().y(), pm().rect().width(), pm().rect().height());
    setEnabled(false);
    show();
    raise();
    update();
}

void MousePressDemoWidget::stop()
{
    pressTimer.stop();
    hide();
}

void MousePressDemoWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.drawPixmap(pm().rect(), pm());
}

void MousePressDemoWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == pressTimer.timerId()) {
        stop();
    }
}

QPixmap &MousePressDemoWidget::pm()
{
    static QPixmap pixmap;
    return pixmap;
}

