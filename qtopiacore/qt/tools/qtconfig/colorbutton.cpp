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

#include "colorbutton.h"

#include <QApplication>
#include <QtEvents>
#include <QColorDialog>
#include <QPainter>
#include <QMimeData>
#include <QStyle>
#include <QStyleOption>

QT_BEGIN_NAMESPACE

ColorButton::ColorButton(QWidget *parent)
    : QAbstractButton(parent), mousepressed(false)
{
    setAcceptDrops(true);
    col = Qt::black;
    connect(this, SIGNAL(clicked()), SLOT(changeColor()));
}


ColorButton::ColorButton(const QColor &c, QWidget *parent)
    : QAbstractButton(parent)
{
    setAcceptDrops(true);
    col = c;
    connect(this, SIGNAL(clicked()), SLOT(changeColor()));
}


void ColorButton::setColor(const QColor &c)
{
    col = c;
    update();
}


void ColorButton::changeColor()
{
    QColor c = QColorDialog::getColor(col, qApp->activeWindow());

    if (c.isValid()) {
        setColor(c);
        emit colorChanged(color());
    }
}


QSize ColorButton::sizeHint() const
{
    return QSize(40, 25);
}


QSize ColorButton::minimumSizeHint() const
{
    return QSize(40, 25);
}


void ColorButton::drawButton(QPainter *p)
{
    QStyleOptionButton buttonOptions;
    buttonOptions.init(this);
    buttonOptions.features = QStyleOptionButton::None;
    buttonOptions.rect = rect();
    buttonOptions.palette = palette();
    buttonOptions.state = (isDown() ? QStyle::State_Sunken : QStyle::State_Raised);
    style()->drawPrimitive(QStyle::PE_PanelButtonBevel, &buttonOptions, p, this);

    p->save();
    drawButtonLabel(p);
    p->restore();

    QStyleOptionFocusRect frectOptions;
    frectOptions.init(this);
    frectOptions.rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &buttonOptions, this);
    if (hasFocus())
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &frectOptions, p, this);
}


void ColorButton::drawButtonLabel(QPainter *p)
{
    QPalette::ColorGroup cg =
        (isEnabled() ? (hasFocus() ? QPalette::Active : QPalette::Inactive) : QPalette::Disabled);

    p->setPen(palette().color(cg, QPalette::ButtonText));
    p->setBrush(col);
    p->drawRect(width() / 4, height() / 4, width() / 2 - 1, height() / 2 - 1);
}


void ColorButton::dragEnterEvent(QDragEnterEvent *e)
{
    if (!e->mimeData()->hasColor()) {
        e->ignore();
        return;
    }
}


void ColorButton::dragMoveEvent(QDragMoveEvent *e)
{
    if (!e->mimeData()->hasColor()) {
        e->ignore();
        return;
    }

    e->accept();
}


void ColorButton::dropEvent(QDropEvent *e)
{
    if (!e->mimeData()->hasColor()) {
        e->ignore();
        return;
    }

    QColor c = qvariant_cast<QColor>(e->mimeData()->colorData());
    setColor(c);
    emit colorChanged(color());
}


void ColorButton::mousePressEvent(QMouseEvent *e)
{
    presspos = e->pos();
    mousepressed = true;
    QAbstractButton::mousePressEvent(e);
}


void ColorButton::mouseReleaseEvent(QMouseEvent *e)
{
    mousepressed = false;
    QAbstractButton::mouseReleaseEvent(e);
}


void ColorButton::mouseMoveEvent(QMouseEvent *e)
{
    if (! mousepressed)
        return;

    if ((presspos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        mousepressed = false;
        setDown(false);

        QDrag *drag = new QDrag(this);
        QMimeData *data = new QMimeData;
        data->setColorData(color());
        drag->setMimeData(data);
        drag->start(Qt::CopyAction);
    }
}

void ColorButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawButton(&p);
}

QT_END_NAMESPACE
