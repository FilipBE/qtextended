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

#include "e1_popup.h"
#include <QPainter>
#include <QListWidgetItem>
#include <QPainter>
#include <QListWidget>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QDebug>
#include <QPalette>
#include <QStyle>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>

E1Popup::E1Popup()
: QWidget(0, Qt::FramelessWindowHint), m_selected(-1)
{
}

void E1Popup::addItem(const QString &text)
{
    PopupItem item;
    item.text = text;
    item.size = sizeHint(item);
    m_items.append(item);
}

void E1Popup::addItem(const QPixmap &pix)
{
    PopupItem item;
    item.pix = pix;
    item.size = sizeHint(item);
    m_items.append(item);
}

QSize E1Popup::sizeHint() const
{
    int x = 0;
    int y = 0;

    for(int ii = 0; ii < m_items.count(); ++ii) {
        QSize s = sizeHint(m_items.at(ii));
        if(s.width() > x)
            x = s.width();
        y += s.height();
    }

    x += 4;
    y += 5;

    return QSize(x, y);
}

QSize E1Popup::sizeHint(const PopupItem &item) const
{
    if(!item.text.isEmpty()) {
        QFontMetrics metrics(font());
        QRect br = metrics.boundingRect(item.text);
        return br.size() + QSize(20, 6);
    }

    if(!item.pix.isNull())
        return item.pix.size() + QSize(4, 6);

    return QSize(0, 0);
}

void E1Popup::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setPen(palette().color(QPalette::Highlight));
    p.drawLine(0, 0, 0, height());
    p.drawLine(0, 3, width(), 3);
    p.drawLine(1, 3, 1, height());
    p.drawLine(0, height() - 1, width(), height() - 1);
    p.drawLine(0, height() - 2, width(), height() - 2);
    p.drawLine(width() - 1, 0, width() - 1, height());
    p.drawLine(width() - 2, 3, width() - 2, height());

    p.setPen(Qt::black);
    p.drawLine(1, 0, width() - 2, 0);
    p.drawLine(1, 2, width() - 2, 2);

    p.setPen(palette().color(QPalette::Dark));
    p.drawLine(1, 1, 4, 1);
    p.drawLine(width() - 5, 1, width() - 2, 1);

    int x = 2;
    int w = width() - 4;
    int currentY = 3;

    for(int ii = 0; ii < m_items.count(); ++ii) {
        QSize s = sizeHint(m_items.at(ii));
        QRect r(x, currentY, w, s.height());
        currentY += s.height();
        paint(&p, r, m_items.at(ii), ii == m_selected);
    }
}

void E1Popup::paint(QPainter *p, const QRect &rect,
                     const PopupItem &item, bool selected)
{
    if(selected)
        p->fillRect(rect, palette().color(QPalette::Highlight));

    if(!item.text.isEmpty()) {
        if(selected)
            p->setPen(palette().color(QPalette::HighlightedText));
        else
            p->setPen(palette().color(QPalette::Text));

        p->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, item.text);
        return;
    }

    if(!item.pix.isNull()) {
        p->drawPixmap(rect.x() + (rect.width() - item.pix.width()) / 2,
                      rect.y() + (rect.height() - item.pix.height()) / 2,
                      item.pix);
        return;
    }
}

void E1Popup::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
    if(-1 == m_selected)
        doClose();
}

void E1Popup::mouseMoveEvent(QMouseEvent *e)
{
    int item = findItem(e->pos());

    if(item != m_selected) {
        m_selected = item;
        update();
    }
}

void E1Popup::mouseReleaseEvent(QMouseEvent *)
{
    if(-1 != m_selected) {
        emit selected(m_selected);
        update();
        QTimer::singleShot(100, this, SLOT(doClose()));
    }
}

void E1Popup::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);
}

void E1Popup::showEvent(QShowEvent *e)
{
    grabMouse();
    QWidget::showEvent(e);
}

int E1Popup::findItem(const QPoint &p)
{
    int currentY = 3;
    for(int ii = 0; ii < m_items.count(); ++ii) {
        if(p.y() > currentY && p.y() <= currentY + m_items.at(ii).size.height()
           && p.x() >= 0 && p.x() <= width())
            return ii;

        currentY += m_items.at(ii).size.height();
    }
    return -1;
}

void E1Popup::popup(const QPoint &pos)
{
    QSize s = sizeHint();
    QPoint showPoint(pos.x() - s.width(), pos.y() - s.height());
    move(showPoint);
    show();
    raise();
}

void E1Popup::doClose()
{
    m_selected = -1;
    hide();
    emit closed();
}

