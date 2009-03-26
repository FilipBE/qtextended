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

#ifndef QTOPIASTYLE_H
#define QTOPIASTYLE_H

#include <qwindowsstyle.h>
#include <qtopiaglobal.h>

class QtopiaStylePrivate;
class QLabel;

class QTOPIA_EXPORT QtopiaStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QtopiaStyle();
    virtual ~QtopiaStyle();

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;

    virtual int styleHint(StyleHint stylehint, const QStyleOption *opt = 0,
                      const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const;
    virtual QPixmap standardPixmap(StandardPixmap standardPixmap,
                    const QStyleOption *option=0, const QWidget *widget=0) const;

    static void drawRoundRect(QPainter *p, const QRect &r, int xRnd, int yRnd);
    static void drawRoundRect(QPainter *p, const QRectF &r, int xRnd, int yRnd);

    static QLabel *buddyForWidget(const QWidget *widget);

protected:
    QtopiaStylePrivate *d;
};

#endif
