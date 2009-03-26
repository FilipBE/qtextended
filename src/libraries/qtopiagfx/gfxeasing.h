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

#ifndef GFXEASING_H
#define GFXEASING_H

#include <QList>
#include <QPair>
#include <QWidget>

class GfxEasingDemoWidgetPrivate;
class GfxEasingDemoWidget : public QWidget
{
Q_OBJECT
public:
    GfxEasingDemoWidget(QWidget *parent = 0);
    virtual ~GfxEasingDemoWidget();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void mousePressEvent(QMouseEvent *);

private:
    GfxEasingDemoWidgetPrivate *d;
};

class GfxEasingFunction;
class GfxEasing
{
public:
    enum Curve {
                 None,
                 InQuad, OutQuad, InOutQuad, OutInQuad,
                 InCubic, OutCubic, InOutCubic, OutInCubic,
                 InQuart, OutQuart, InOutQuart, OutInQuart,
                 InQuint, OutQuint, InOutQuint, OutInQuint,
                 InSine, OutSine, InOutSine, OutInSine,
                 InExpo, OutExpo, InOutExpo, OutInExpo,
                 InCirc, OutCirc, InOutCirc, OutInCirc,
                 InElastic, OutElastic, InOutElastic, OutInElastic,
                 InBack, OutBack, InOutBack, OutInBack,
                 InBounce, OutBounce, InOutBounce, OutInBounce
               };

    GfxEasing();
    GfxEasing(Curve);
    GfxEasing(const QString &);
    GfxEasing(const GfxEasing &);
    GfxEasing &operator=(const GfxEasing &);

    bool isLinear() const;

    qreal from() const;
    void setFrom(qreal);
    qreal to() const;
    void setTo(qreal);
    qreal length() const;
    void setLength(qreal);

    qreal valueAt(qreal t) const;
    qreal valueAt(qreal t, qreal from, qreal to, qreal length) const;

    static QStringList curves();

    typedef float (*Function)(float t, float b, float c, float d);
private:
    GfxEasingFunction *_config;
    Function _func;

    qreal _b, _c, _d;
};

#endif
