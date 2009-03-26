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
#include "titlewindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QTimerEvent>

TitleWindow::TitleWindow(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags | Qt::Tool | Qt::FramelessWindowHint)
    , m_state(Hidden)
    , m_timeout(10000)
    , m_hideTimerId(-1)
{
    m_fade.setFrameRange(0, 100);
    m_fade.setDuration(1000);

    connect(&m_fade, SIGNAL(frameChanged(int)), this, SLOT(setOpacity(int)));
    connect(&m_fade, SIGNAL(finished()), this, SLOT(fadeFinished()));

    if (parent)
        parent->installEventFilter(this);
}

bool TitleWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == parent()) {
        switch (event->type()) {
        case QEvent::Hide:
        case QEvent::WindowDeactivate:
            hide();
        default:
            break;
        }
    }

    return QWidget::eventFilter(object, event);
}

void TitleWindow::fadeIn()
{
    killHideTimer();

    switch (m_state) {
    case Hidden:
        setOpacity(0);

        m_fade.setDirection(QTimeLine::Forward);
        m_fade.start();

        m_state = FadingIn;

        show();

        break;
    case FadingOut:
        m_fade.setDirection(QTimeLine::Forward);

        m_state = FadingIn;

        break;
    default:
        break;
    }
}

void TitleWindow::fadeOut()
{
    killHideTimer();

    switch (m_state) {
    case Visible:
        m_fade.setDirection(QTimeLine::Backward);
        m_fade.start();

        m_state = FadingOut;

        break;
    case FadingIn:
        m_fade.setDirection(QTimeLine::Backward);

        m_state = FadingOut;

        break;
    default:
        break;
    }
}


void TitleWindow::resetHideTimer()
{
    switch (m_state) {
    case FadingOut:
        killHideTimer();
        m_fade.setDirection(QTimeLine::Forward);
        m_state = FadingIn;
        break;
    case Visible:
        if (m_hideTimerId != -1)
            killTimer(m_hideTimerId);
        m_hideTimerId = startTimer(m_timeout);
        break;
    default:
        break;
    }

}

void TitleWindow::showEvent(QShowEvent *event)
{
    if (m_hideTimerId != -1)
        killTimer(m_hideTimerId);

    m_hideTimerId = startTimer(m_timeout);

    switch (m_state) {
    case Hidden:
        setWindowOpacity(100);

        m_state = Visible;

        break;
    case FadingOut:
        fadeIn();
        break;
    case FadingIn:
    case Visible:
        break;
    }

    QRect geometry = QApplication::desktop()->availableGeometry(this);
    geometry.setHeight(minimumSizeHint().height());
    setGeometry(geometry);

    QWidget::showEvent(event);

    emit visibilityChanged(true);
}

void TitleWindow::hideEvent(QHideEvent *event)
{
    killHideTimer();

    if (m_fade.state() == QTimeLine::Running) {
        m_fade.stop();

        fadeFinished();
    }

    m_state = Hidden;

    QWidget::hideEvent(event);

    emit visibilityChanged(false);
}

void TitleWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_hideTimerId) {
        fadeOut();
    } else {
        QWidget::timerEvent(event);
    }
}

void TitleWindow::setOpacity(int opacity)
{
    setWindowOpacity(qreal(opacity) / 100.0);

    opacityChanged(opacity);
}

void TitleWindow::fadeFinished()
{
    switch (m_state) {
    case FadingIn:
        m_state = Visible;
        break;
    case FadingOut:
        hide();
        break;
    default:
        break;
    }
}

void TitleWindow::killHideTimer()
{
    if (m_hideTimerId != -1) {
        killTimer(m_hideTimerId);

        m_hideTimerId = -1;
    }
}
