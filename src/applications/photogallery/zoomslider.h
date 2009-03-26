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
#ifndef ZOOMSLIDER_H
#define ZOOMSlIDER_H

#include <QAbstractSlider>

class ZoomSlider : public QAbstractSlider
{
    Q_OBJECT
public:
    ZoomSlider(QWidget *parent = 0);
    ~ZoomSlider();

    virtual QSize sizeHint() const;

    int opacity() const { return m_opacity; }

public slots:
    void setOpacity(int opacity) { m_opacity = opacity; update(); }

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );

private:
    int m_opacity;
};

#endif
