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

#include "e2_frames.h"
#include "e2_colors.h"
#include <QPainter>
#include "e2_bar.h"
#include <QModelIndex>
#include <QVariant>
#include <QDesktopWidget>
#include <QApplication>

E2PopupFrame::E2PopupFrame(QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags | Qt::FramelessWindowHint)
{
    setContentsMargins(3, 3, 3, 2);
}

void E2PopupFrame::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), GREEN_DARK);

    p.setPen(REALLY_LIGHT_GREY);
    p.drawLine(0, 0, width() - 2, 0);
    p.drawLine(0, 0, 0, height() - 2);
    p.drawLine(width() - 3, 3, width() - 3, height() - 3);

    p.setPen(GREY);
    p.drawLine(1, 1, width() - 2, 1);
    p.drawLine(1, 1, 1, height() - 2);
    p.drawLine(1, height() - 2, width() - 2, height() - 2);
    p.drawLine(width() - 2, 2, width() - 2, height() - 2);

    p.setPen(REALLY_DARK_GREY);
    p.drawLine(2, 2, width() - 3, 2);
    p.drawLine(2, 2, 2, height() - 3);
    p.drawLine(0, height() - 1, width() - 1, height() - 1);
    p.drawLine(width() - 1, 0, width() - 1, height() - 1);
}


E2TitleFrame::E2TitleFrame(TitleType title, QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags | Qt::FramelessWindowHint), m_titleType(title),
  m_titleFill(":image/samples/e2_titlefill")
{
    m_bar = new E2Bar(this);

    if(NoTitle == m_titleType) {
        setContentsMargins(2, 4, 2, 36);
    } else {
        setContentsMargins(2, 4 + m_titleFill.height(), 2, 36);
    }
}

void E2TitleFrame::setTitleText(const QString &title)
{
    m_title = title;
    update();
}

void E2TitleFrame::resizeEvent(QResizeEvent *)
{
    doLayout();
}

E2Bar *E2TitleFrame::bar() const
{
    return m_bar;
}

void E2TitleFrame::doLayout()
{
    m_bar->setGeometry(1, height() - 34, width() - 2, 33);
}

void E2TitleFrame::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    // Draw border
    p.setPen(Qt::black);
    p.drawLine(0, 0, width(), 0);
    p.drawLine(0, 3, width(), 3);
    p.drawLine(0, 0, 0, height());
    p.drawLine(width() - 1, 0, width() - 1, height());
    p.drawLine(0, height() - 1, width(), height() - 1);
    p.drawLine(2, height() - 35, width() - 3, height() - 35);

    p.setPen(LIGHT_GREEN_BAR);
    p.drawLine(1, 1, width() - 2, 1);
    p.setPen(DARK_GREEN_BAR);
    p.drawLine(1, 2, width() - 2, 2);

    p.setPen(REALLY_LIGHT_GREY);
    p.drawLine(1, 4, width() - 2, 4);
    p.drawLine(1, 4, 1, height() - 35);
    p.drawLine(width() - 2, 4, width() - 2, height() - 35);

    if(GradientTitle == m_titleType)
        p.drawTiledPixmap(2, 5, width() - 4, m_titleFill.height(), m_titleFill);

    p.setPen(Qt::black);
    p.drawText(QRect(10, 5, width() - 10 - 3, m_titleFill.height()), Qt::AlignLeft | Qt::AlignVCenter, m_title);
}

E2ListDelegate::E2ListDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

void E2ListDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem & option,
                             const QModelIndex & index ) const
{
    painter->save();

    QString text = index.data(Qt::DisplayRole).toString();

    if(option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, GREEN_HIGHLIGHT);

    painter->setPen(Qt::black);
    QRect r = option.rect;
    r.setX(r.x() + 15);
    r.setWidth(r.width() - 15);
    painter->drawText(r, Qt::AlignLeft | Qt::AlignVCenter, text);
    painter->setPen(Qt::DotLine);
    painter->drawLine(option.rect.x(),
               option.rect.y() + option.rect.height() - 1,
               option.rect.x() + option.rect.width(),
               option.rect.y() + option.rect.height() - 1);

    painter->restore();
}

E2ListWidget::E2ListWidget(QWidget *parent)
: QListWidget(parent)
{
    setItemDelegate(new E2ListDelegate(this));
}

void e2Center(QWidget *wid)
{
    QRect geom = QApplication::desktop()->availableGeometry();

    wid->move(geom.x() + (geom.width() - wid->width()) / 2,
              geom.y() + (geom.height() - wid->height()) / 2);
}

