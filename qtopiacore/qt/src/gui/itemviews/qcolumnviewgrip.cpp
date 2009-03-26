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

#ifndef QT_NO_QCOLUMNVIEW

#include "qcolumnviewgrip_p.h"
#include <qstyleoption.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qevent.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*
    \internal
    class QColumnViewGrip

    QColumnViewGrip is created to go inside QAbstractScrollArea's corner.
    When the mouse it moved it will resize the scroll area and emit's a signal.
 */

/*
    \internal
    \fn void QColumnViewGrip::gripMoved()
    Signal that is emitted when the grip moves the parent widget.
 */

/*!
    Creates a new QColumnViewGrip with the given \a parent to view a model.
    Use setModel() to set the model.
*/
QColumnViewGrip::QColumnViewGrip(QWidget *parent)
:  QWidget(*new QColumnViewGripPrivate, parent, 0)
{
#ifndef QT_NO_CURSOR
    setCursor(Qt::SplitHCursor);
#endif
}

/*!
  \internal
*/
QColumnViewGrip::QColumnViewGrip(QColumnViewGripPrivate & dd, QWidget *parent, Qt::WFlags f)
:  QWidget(dd, parent, f)
{
}

/*!
  Destroys the view.
*/
QColumnViewGrip::~QColumnViewGrip()
{
}

/*!
    Attempt to resize the parent object by \a offset
    returns the amount of offset that it was actually able to resized
*/
int QColumnViewGrip::moveGrip(int offset)
{
    QWidget *parentWidget = (QWidget*)parent();

    // first resize the parent
    int oldWidth = parentWidget->width();
    int newWidth = oldWidth;
    if (isRightToLeft())
       newWidth -= offset;
    else
       newWidth += offset;
    newWidth = qMax(parentWidget->minimumWidth(), newWidth);
    parentWidget->resize(newWidth, parentWidget->height());

    // Then have the view move the widget
    int realOffset = parentWidget->width() - oldWidth;
    int oldX = parentWidget->x();
    if (realOffset != 0)
        emit gripMoved(realOffset);
    if (isRightToLeft())
        realOffset = -1 * (oldX - parentWidget->x());
    return realOffset;
}

/*!
    \reimp
*/
void QColumnViewGrip::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawControl(QStyle::CE_ColumnViewGrip, &opt, &painter, this);
    event->accept();
}

/*!
    \reimp
    Resize the parent window to the sizeHint
*/
void QColumnViewGrip::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    QWidget *parentWidget = (QWidget*)parent();
    int offset = parentWidget->sizeHint().width() - parentWidget->width();
    if (isRightToLeft())
        offset *= -1;
    moveGrip(offset);
    event->accept();
}

/*!
    \reimp
    Begin watching for mouse movements
*/
void QColumnViewGrip::mousePressEvent(QMouseEvent *event)
{
    Q_D(QColumnViewGrip);
    d->originalXLocation = event->globalX();
    event->accept();
}

/*!
    \reimp
    Calculate the movement of the grip and moveGrip() and emit gripMoved
*/
void QColumnViewGrip::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QColumnViewGrip);
    int offset = event->globalX() - d->originalXLocation;
    d->originalXLocation = moveGrip(offset) + d->originalXLocation;
    event->accept();
}

/*!
    \reimp
    Stop watching for mouse movements
*/
void QColumnViewGrip::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QColumnViewGrip);
    d->originalXLocation = -1;
    event->accept();
}

/*
 * private object implementation
 */
QColumnViewGripPrivate::QColumnViewGripPrivate()
:  QWidgetPrivate(),
originalXLocation(-1)
{
}

QT_END_NAMESPACE

#endif // QT_NO_QCOLUMNVIEW
