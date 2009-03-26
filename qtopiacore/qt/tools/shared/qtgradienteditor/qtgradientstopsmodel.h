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

#ifndef QTGRADIENTSTOPSMODEL_H
#define QTGRADIENTSTOPSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class QColor;

class QtGradientStopsModel;

class QtGradientStop
{
public:
    qreal position() const;
    QColor color() const;
    QtGradientStopsModel *gradientModel() const;

private:
    void setColor(const QColor &color);
    void setPosition(qreal position);
    friend class QtGradientStopsModel;
    QtGradientStop(QtGradientStopsModel *model = 0);
    ~QtGradientStop();
    class QtGradientStopPrivate *d_ptr;
};

class QtGradientStopsModel : public QObject
{
    Q_OBJECT
public:
    typedef QMap<qreal, QtGradientStop *> PositionStopMap;

    QtGradientStopsModel(QObject *parent = 0);
    ~QtGradientStopsModel();

    PositionStopMap stops() const;
    QtGradientStop *at(qreal pos) const;
    QColor color(qreal pos) const; // calculated between points
    QList<QtGradientStop *> selectedStops() const;
    QtGradientStop *currentStop() const;
    bool isSelected(QtGradientStop *stop) const;
    QtGradientStop *firstSelected() const;
    QtGradientStop *lastSelected() const;
    QtGradientStopsModel *clone() const;

    QtGradientStop *addStop(qreal pos, const QColor &color);
    void removeStop(QtGradientStop *stop);
    void moveStop(QtGradientStop *stop, qreal newPos);
    void swapStops(QtGradientStop *stop1, QtGradientStop *stop2);
    void changeStop(QtGradientStop *stop, const QColor &newColor);
    void selectStop(QtGradientStop *stop, bool select);
    void setCurrentStop(QtGradientStop *stop);

    void moveStops(double newPosition); // moves current stop to newPos and all selected stops are moved accordingly
    void clear();
    void clearSelection();
    void flipAll();
    void selectAll();
    void deleteStops();

signals:
    void stopAdded(QtGradientStop *stop);
    void stopRemoved(QtGradientStop *stop);
    void stopMoved(QtGradientStop *stop, qreal newPos);
    void stopsSwapped(QtGradientStop *stop1, QtGradientStop *stop2);
    void stopChanged(QtGradientStop *stop, const QColor &newColor);
    void stopSelected(QtGradientStop *stop, bool selected);
    void currentStopChanged(QtGradientStop *stop);

private:
    class QtGradientStopsModelPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGradientStopsModel)
    Q_DISABLE_COPY(QtGradientStopsModel)
};

QT_END_NAMESPACE

#endif
