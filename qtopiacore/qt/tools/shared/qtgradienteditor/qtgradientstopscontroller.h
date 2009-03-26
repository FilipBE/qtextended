/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef QTGRADIENTSTOPSCONTROLLER_H
#define QTGRADIENTSTOPSCONTROLLER_H

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

namespace Ui {
    class QtGradientEditor;
}

class QtGradientStopsController : public QObject
{
    Q_OBJECT
public:
    QtGradientStopsController(QObject *parent = 0);
    ~QtGradientStopsController();

    void setUi(Ui::QtGradientEditor *editor);

    void setGradientStops(const QGradientStops &stops);
    QGradientStops gradientStops() const;

    QColor::Spec spec() const;
    void setSpec(QColor::Spec spec);

signals:

    void gradientStopsChanged(const QGradientStops &stops);

private:
    class QtGradientStopsControllerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGradientStopsController)
    Q_DISABLE_COPY(QtGradientStopsController)
    Q_PRIVATE_SLOT(d_func(), void slotHsvClicked())
    Q_PRIVATE_SLOT(d_func(), void slotRgbClicked())
    Q_PRIVATE_SLOT(d_func(), void slotCurrentStopChanged(QtGradientStop *stop))
    Q_PRIVATE_SLOT(d_func(), void slotStopMoved(QtGradientStop *stop, qreal newPos))
    Q_PRIVATE_SLOT(d_func(), void slotStopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2))
    Q_PRIVATE_SLOT(d_func(), void slotStopChanged(QtGradientStop *stop, const QColor &newColor))
    Q_PRIVATE_SLOT(d_func(), void slotStopSelected(QtGradientStop *stop, bool selected))
    Q_PRIVATE_SLOT(d_func(), void slotStopAdded(QtGradientStop *stop))
    Q_PRIVATE_SLOT(d_func(), void slotStopRemoved(QtGradientStop *stop))
    Q_PRIVATE_SLOT(d_func(), void slotUpdatePositionSpinBox())
    Q_PRIVATE_SLOT(d_func(), void slotChangeColor(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeHue(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeSaturation(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeValue(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeAlpha(const QColor &color))
    Q_PRIVATE_SLOT(d_func(), void slotChangeHue(int))
    Q_PRIVATE_SLOT(d_func(), void slotChangeSaturation(int))
    Q_PRIVATE_SLOT(d_func(), void slotChangeValue(int))
    Q_PRIVATE_SLOT(d_func(), void slotChangeAlpha(int))
    //Q_PRIVATE_SLOT(d_func(), void slotChangePosition(double newPos))
    Q_PRIVATE_SLOT(d_func(), void slotChangePosition(double value))
    Q_PRIVATE_SLOT(d_func(), void slotChangeZoom(int value))
    Q_PRIVATE_SLOT(d_func(), void slotZoomIn())
    Q_PRIVATE_SLOT(d_func(), void slotZoomOut())
    Q_PRIVATE_SLOT(d_func(), void slotZoomAll())
    Q_PRIVATE_SLOT(d_func(), void slotZoomChanged(double))
};

QT_END_NAMESPACE

#endif
