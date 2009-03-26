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

#ifndef QTGRADIENTEDITOR_H
#define QTGRADIENTEDITOR_H

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QtGradientEditor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QGradient gradient READ gradient WRITE setGradient)
    Q_PROPERTY(bool backgroundCheckered READ isBackgroundCheckered WRITE setBackgroundCheckered)
    Q_PROPERTY(bool detailsVisible READ detailsVisible WRITE setDetailsVisible)
    Q_PROPERTY(bool detailsButtonVisible READ isDetailsButtonVisible WRITE setDetailsButtonVisible)
public:
    QtGradientEditor(QWidget *parent = 0);
    ~QtGradientEditor();

    void setGradient(const QGradient &gradient);
    QGradient gradient() const;

    bool isBackgroundCheckered() const;
    void setBackgroundCheckered(bool checkered);

    bool detailsVisible() const;
    void setDetailsVisible(bool visible);

    bool isDetailsButtonVisible() const;
    void setDetailsButtonVisible(bool visible);

    QColor::Spec spec() const;
    void setSpec(QColor::Spec spec);

signals:

    void gradientChanged(const QGradient &gradient);
    void aboutToShowDetails(bool details, int extenstionWidthHint);

private:
    class QtGradientEditorPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGradientEditor)
    Q_DISABLE_COPY(QtGradientEditor)
    Q_PRIVATE_SLOT(d_func(), void slotGradientStopsChanged(const QGradientStops &stops))
    Q_PRIVATE_SLOT(d_func(), void slotTypeChanged(int type))
    Q_PRIVATE_SLOT(d_func(), void slotSpreadChanged(int type))
    Q_PRIVATE_SLOT(d_func(), void slotStartLinearXChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotStartLinearYChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotEndLinearXChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotEndLinearYChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotCentralRadialXChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotCentralRadialYChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotFocalRadialXChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotFocalRadialYChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotRadiusRadialChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotCentralConicalXChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotCentralConicalYChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotAngleConicalChanged(double value))
    Q_PRIVATE_SLOT(d_func(), void slotDetailsChanged(bool details))
    Q_PRIVATE_SLOT(d_func(), void startLinearChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void endLinearChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void centralRadialChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void focalRadialChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void radiusRadialChanged(qreal))
    Q_PRIVATE_SLOT(d_func(), void centralConicalChanged(const QPointF &))
    Q_PRIVATE_SLOT(d_func(), void angleConicalChanged(qreal))
};

QT_END_NAMESPACE

#endif
