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

#ifndef QSLIDER_H
#define QSLIDER_H

#include <QtGui/qabstractslider.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_SLIDER

class QSliderPrivate;
class QStyleOptionSlider;
class Q_GUI_EXPORT QSlider : public QAbstractSlider
{
    Q_OBJECT

    Q_ENUMS(TickPosition)
    Q_PROPERTY(TickPosition tickPosition READ tickPosition WRITE setTickPosition)
    Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval)

public:
    enum TickPosition {
        NoTicks = 0,
        TicksAbove = 1,
        TicksLeft = TicksAbove,
        TicksBelow = 2,
        TicksRight = TicksBelow,
        TicksBothSides = 3

#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        ,NoMarks = NoTicks,
        Above = TicksAbove,
        Left = TicksAbove,
        Below = TicksBelow,
        Right = TicksRight,
        Both = TicksBothSides
#endif
    };

    explicit QSlider(QWidget *parent = 0);
    explicit QSlider(Qt::Orientation orientation, QWidget *parent = 0);

    ~QSlider();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setTickPosition(TickPosition position);
    TickPosition tickPosition() const;

    void setTickInterval(int ti);
    int tickInterval() const;

    bool event(QEvent *event);

protected:
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void initStyleOption(QStyleOptionSlider *option) const;

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QSlider(QWidget *parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QSlider(Qt::Orientation, QWidget *parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QSlider(int minValue, int maxValue, int pageStep, int value,
                                  Qt::Orientation orientation,
                                  QWidget *parent = 0, const char *name = 0);
    inline QT3_SUPPORT void setTickmarks(TickPosition position) { setTickPosition(position); }
    inline QT3_SUPPORT TickPosition tickmarks() const { return tickPosition(); }
public Q_SLOTS:
    inline QT_MOC_COMPAT void addStep() { triggerAction(SliderSingleStepAdd); };
    inline QT_MOC_COMPAT void subtractStep() { triggerAction(SliderSingleStepSub); };
#endif

private:
    friend Q_GUI_EXPORT QStyleOptionSlider qt_qsliderStyleOption(QSlider *slider);

    Q_DISABLE_COPY(QSlider)
    Q_DECLARE_PRIVATE(QSlider)
};

#endif // QT_NO_SLIDER

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSLIDER_H
