/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QSTYLEPAINTER_H
#define QSTYLEPAINTER_H

#include <QtGui/qpainter.h>
#include <QtGui/qstyle.h>
#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QStylePainter : public QPainter
{
public:
    inline QStylePainter() : QPainter(), widget(0), wstyle(0) {}
    inline explicit QStylePainter(QWidget *w) { begin(w, w); }
    inline QStylePainter(QPaintDevice *pd, QWidget *w) { begin(pd, w); }
    inline bool begin(QWidget *w) { return begin(w, w); }
    inline bool begin(QPaintDevice *pd, QWidget *w) {
        Q_ASSERT_X(w, "QStylePainter::QStylePainter", "Widget must be non-zero");
        widget = w;
        wstyle = w->style();
        return QPainter::begin(pd);
    };
    inline void drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &opt);
    inline void drawControl(QStyle::ControlElement ce, const QStyleOption &opt);
    inline void drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &opt);
    inline void drawItemText(const QRect &r, int flags, const QPalette &pal, bool enabled,
                             const QString &text, QPalette::ColorRole textRole = QPalette::NoRole);
    inline void drawItemPixmap(const QRect &r, int flags, const QPixmap &pixmap);
    inline QStyle *style() const { return wstyle; }

private:
    QWidget *widget;
    QStyle *wstyle;
    Q_DISABLE_COPY(QStylePainter)
};

void QStylePainter::drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &opt)
{
    wstyle->drawPrimitive(pe, &opt, this, widget);
}

void QStylePainter::drawControl(QStyle::ControlElement ce, const QStyleOption &opt)
{
    wstyle->drawControl(ce, &opt, this, widget);
}

void QStylePainter::drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &opt)
{
    wstyle->drawComplexControl(cc, &opt, this, widget);
}

void QStylePainter::drawItemText(const QRect &r, int flags, const QPalette &pal, bool enabled,
                                 const QString &text, QPalette::ColorRole textRole)
{
    wstyle->drawItemText(this, r, flags, pal, enabled, text, textRole);
}

void QStylePainter::drawItemPixmap(const QRect &r, int flags, const QPixmap &pixmap)
{
    wstyle->drawItemPixmap(this, r, flags, pixmap);
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSTYLEPAINTER_H
