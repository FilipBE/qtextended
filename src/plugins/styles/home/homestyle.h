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

#ifndef HOMESTYLE_H
#define HOMESTYLE_H

#include <QStylePlugin>
#include <private/qthumbstyle_p.h>

class HomeStylePrivate;

class HomeStyle : public QThumbStyle
{
    Q_OBJECT
public:
    HomeStyle();
    virtual ~HomeStyle(void);

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    QSize sizeFromContents ( ContentsType type, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget = 0 ) const;
    QRect subElementRect ( SubElement element, const QStyleOption * option, const QWidget * widget = 0 ) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                         const QWidget *w = 0) const;
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const;
    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget) const;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
    QPainter *p, const QWidget *widget) const;
    int styleHint(StyleHint stylehint, const QStyleOption *opt = 0,
                  const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const;

private:
    HomeStylePrivate *d;
};

class QTOPIA_PLUGIN_EXPORT HomeStylePlugin : public QStylePlugin
{
public:
    QStringList keys() const;
    QStyle *create(const QString &key);
};

#endif
