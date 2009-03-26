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
#ifndef TITLEWINDOW_H
#define TITLEWINDOW_H

#include <QTimeLine>
#include <QWidget>

class TitleWindow : public QWidget
{
    Q_OBJECT
public:
    enum State
    {
        Hidden,
        FadingIn,
        Visible,
        FadingOut
    };

    TitleWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    State state() const { return m_state; }

    int hideTimeout() const { return m_timeout; }
    void setHideTimeout(int milliseconds) { m_timeout = milliseconds; }

    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void fadeIn();
    void fadeOut();

    void resetHideTimer();

signals:
    void opacityChanged(int opacity);
    void visibilityChanged(bool visible);

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void timerEvent(QTimerEvent *event);

private slots:
    void setOpacity(int opacity);
    void fadeFinished();

private:
    void killHideTimer();

    QTimeLine m_fade;
    State m_state;
    int m_timeout;
    int m_hideTimerId;
};

#endif
