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
#ifndef DURATIONSLIDER_H
#define DURATIONSLIDER_H

#include <QAbstractSlider>
#include <QTimeLine>

class DurationSlider : public QAbstractSlider
{
    Q_OBJECT
public:
    DurationSlider(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void setTransparency(int transparency);
    void fadeOutFinished();

private:
    QPoint m_pressedPosition;
    QTimeLine m_fadeOut;
    int m_transparency;
};

#endif
