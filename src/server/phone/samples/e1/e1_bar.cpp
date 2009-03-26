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

#include "e1_bar.h"

#include <QWidget>
#include <QPainter>
#include <QLinearGradient>
#include <QPalette>
#include <QColor>
#include <QMouseEvent>
#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include "e1_popup.h"

// define E1Bar
E1Bar::E1Bar(QWidget *parent, Qt::WFlags wflags)
: QWidget(parent, wflags), m_clicked(-1), m_border(BarBorder)
{
}

E1Bar::~E1Bar()
{
    qDeleteAll(m_items);
}

void E1Bar::addItem(E1BarItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(!item->m_bar);
    m_items.append(item);
    item->m_bar = this;
    update();
}

void E1Bar::addSeparator()
{
    m_items.append(0);
    update();
}

void E1Bar::setBorder(Border b)
{
    if(m_border != b) {
        m_border = b;
        update();
    }
}

E1Bar::Border E1Bar::border() const
{
    return m_border;
}

int E1Bar::leadin() const
{
    if(BarBorder == border())
        return 2;
    else
        return 1;
}

int E1Bar::vertin() const
{
    return 1;
}

int E1Bar::leadout() const
{
    if(BarBorder == border())
        return 3;
    else
        return 1;
}

int E1Bar::vertout() const
{
    if(BarBorder == border())
        return 2;
    else
        return 1;
}

QColor E1Bar::darken(const QColor &color, int percent) const
{
    QColor rv( (color.red() * percent) / 100,
               (color.green() * percent) / 100,
               (color.blue() * percent) / 100);

    return rv;
}

QColor E1Bar::lighten(const QColor &color, int percent) const
{
    QColor rv( color.red() + ((255 - color.red()) * percent) / 100,
              color.green() + ((255 - color.green()) * percent) / 100,
              color.blue() + ((255 - color.blue()) * percent) / 100);

    return rv;
}

void E1Bar::drawBorder(QPainter *painter)
{
    if(BarBorder == border()) {
        painter->setPen(Qt::black);
        painter->drawLine(0, 1, 0, 3);
        painter->drawPoint(0, height() - 1);
        painter->drawPoint(1, 1);

        painter->setPen(highlight());
        painter->drawLine(0, 3, 0, height() - 2);
        painter->setPen(lighten(highlight(), 10));
        painter->drawLine(1, 2, 1, height());

        painter->setPen(lighten(highlight(), 25));
        painter->drawLine(2, height() - 2, width(), height() - 2);
        painter->setPen(lighten(highlight(), 45));
        painter->drawLine(2, height() - 1, width(), height() - 1);

        painter->setPen(QColor(0, 0, 0));
        painter->drawLine(0, 0, width(), 0);

        painter->setPen(Qt::black);
        painter->drawPoint(width() - 1, height() - 1);
        painter->drawPoint(width() - 1, 0);

        painter->setPen(highlight());
        painter->drawLine(width() - 1, 1, width() - 1, height() - 2);
        painter->setPen(lighten(highlight(), 10));
        painter->drawLine(width() - 2, 1, width() - 2, height());
    } else {
        // Black border
        painter->setPen(Qt::black);
        painter->drawLine(1, 0, width() - 2, 0);
        painter->drawLine(1, height() - 1, width() - 2, height() - 1);

        painter->drawLine(0, 1, 0, height() - 2);
        painter->drawLine(width() - 1, 1, width() - 1, height() - 2);
    }
}

void E1Bar::drawSeparator(QPainter *paint, int x)
{
    paint->setPen(palette().color(QPalette::Light));
    paint->drawLine(x, vertin(), x, height() - vertout());
    paint->setPen(palette().color(QPalette::Dark));
    paint->drawLine(x + 1, vertin(), x + 1, height() - vertout());
}

QColor E1Bar::pressed() const
{
    return darken(highlight(), 70);
}

QColor E1Bar::unpressed() const
{
    return lighten(highlight(), 50);
}

QColor E1Bar::highlight() const
{
    QColor h = palette().color(QPalette::Highlight);
    return darken(h, 90);
}

QLinearGradient E1Bar::gradient() const
{
    QColor highlight = this->highlight();

    QColor topstart = lighten(highlight, 50);
    QColor topend = lighten(highlight, 10);
    QColor bottomend = lighten(highlight, 50);
    QColor light = lighten(highlight, 80);

    QLinearGradient grad(0, 0, 0, height());

    grad.setColorAt(0, topstart);
    grad.setColorAt(0.35, topend);
    grad.setColorAt(0.351, highlight);
    grad.setColorAt(0.9, bottomend);
    grad.setColorAt(1, light);

    return grad;
}

QLinearGradient E1Bar::pressedGradient() const
{
    QColor highlight = this->highlight();
    QColor end = lighten(highlight, 30);

    QLinearGradient grad(0, 0, 0, height());
    grad.setColorAt(0, highlight);
    grad.setColorAt(1, end);

    return grad;
}

void E1Bar::updateRects()
{
    // Determine free space
    int consumed = 0;
    int unspecified = 0;

    bool wasSeparator = false;
    foreach(E1BarItem *item, m_items) {
        if(item) {
            if(item->flags() & E1BarItem::Enabled) {
                QSize size = item->sizeHint();
                if(-1 == size.width() ||
                   item->flags() & E1BarItem::Expanding) {
                    ++unspecified;
                }

                if(-1 != size.width())
                    consumed += size.width();

                wasSeparator = false;
            }
        } else {
            if(!wasSeparator) {
                wasSeparator = true;
                consumed += 2;
            }
        }
    }

    int unspecifiedWidth = 0;
    if(unspecified)
        unspecifiedWidth = (width() - consumed - leadin() - leadout()) /
            unspecified;

    int currentX = leadin();
    wasSeparator = false;
    foreach(E1BarItem *item, m_items) {
        if(item) {
            if(item->flags() & E1BarItem::Enabled) {
                QSize size = item->sizeHint();
                QRect r(currentX, vertin(),
                        (-1 == size.width())?unspecifiedWidth:size.width(),
                        height() - vertout() - 1);

                if(item->flags() & E1BarItem::Expanding)
                    r.setWidth(r.width() + unspecifiedWidth);

                item->m_rect = r;
                currentX += r.width();
                wasSeparator = false;
            } else {
                item->m_rect = QRect();
            }
        } else {
            if(!wasSeparator) {
                wasSeparator = true;
                currentX += 2;
            }
        }
    }
}

void E1Bar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    updateRects();

    // Render items
    int currentX = 0;

    currentX += leadin();

    bool wasSeparator = false;
    for(int ii = 0; ii < m_items.count(); ++ii) {
        E1BarItem *item = m_items.at(ii);

        if(!item) {
            if(!wasSeparator) {
                drawSeparator(&painter, currentX);
                currentX += 2;
                wasSeparator = true;
            }
        } else if(!(item->flags() & E1BarItem::Enabled)) {
        } else if(ii == m_clicked ||
                  item->flags() & E1BarItem::Clicked) {
            QLinearGradient grad = pressedGradient();
            painter.setPen(Qt::NoPen);
            painter.setBrush(grad);
            painter.drawRect(item->rect());
            item->paint(&painter, item->rect(), pressed());
            currentX += item->rect().width();
            wasSeparator = false;
        } else {
            QLinearGradient grad = gradient();
            painter.setPen(Qt::NoPen);
            painter.setBrush(grad);
            painter.drawRect(item->rect());
            item->paint(&painter, item->rect(), unpressed());
            currentX += item->rect().width();
            wasSeparator = false;
        }
    }

    QLinearGradient grad = gradient();
    painter.setPen(Qt::NoPen);
    painter.setBrush(grad);
    painter.drawRect(currentX, vertin(), width() - leadin() - leadout() - currentX - 1, height() - vertout() - 1);

    drawBorder(&painter);
}

void E1Bar::mousePressEvent(QMouseEvent *e)
{
    updateRects();

    // Find the clicked item
    for(int ii = 0; ii < m_items.count(); ++ii) {
        E1BarItem *item = m_items.at(ii);
        if(item && item->rect().contains(e->pos())) {
            if(item->flags() & E1BarItem::Clickable) {
                m_clicked = ii;
                update();
            }
            return;
        }
    }
}

void E1Bar::mouseReleaseEvent(QMouseEvent *)
{
    if(-1 != m_clicked) {
        int wasClicked = m_clicked;
        m_clicked = -1;
        update();
        m_items.at(wasClicked)->clicked();
    }
}

// define E1BarItem
E1BarItem::E1BarItem(Flags f)
: m_flags(f), m_bar(0)
{
}

E1BarItem::~E1BarItem()
{
}

void E1BarItem::paint(QPainter *, const QRect &, const QColor &)
{
}

void E1BarItem::clicked()
{
}

QSize E1BarItem::sizeHint() const
{
    return m_sizeHint;
}

void E1BarItem::setSizeHint(QSize size)
{
    m_sizeHint = size;
    if(m_bar)
        m_bar->update();
}

E1BarItem::Flags E1BarItem::flags() const
{
    return m_flags;
}

QRect E1BarItem::rect() const
{
    return m_rect;
}

E1Bar * E1BarItem::bar() const
{
    return m_bar;
}

void E1BarItem::setFlag(Flags flag)
{
    m_flags = (Flags)(m_flags | flag);
    if(m_bar)
        m_bar->update();
}

void E1BarItem::clearFlag(Flags flag)
{
    m_flags = (Flags)(m_flags & ~flag);
    if(m_bar)
        m_bar->update();
}

// define E1Button
E1Button::E1Button()
: E1BarItem((Flags)(Clickable | Enabled)), m_stretch(true), m_margin(0),
  m_tint(true), m_tintText(true)
{
}

void E1Button::setMargin(int margin)
{
    Q_ASSERT(margin >= 0);
    m_margin = margin;
    if(bar())
        bar()->update();
}

QSize E1Button::sizeHint() const
{
    if(!bar())
        return QSize();

    if(m_text.isEmpty()) {
        if(m_mask.height() == 0)
            return QSize();

        if(m_stretch) {
            int height = bar()->height();

            return QSize(2 * m_margin + (m_mask.width() * height) / m_mask.height(),
                         height);
        } else {
            return QSize(2 * m_margin + m_mask.width(), m_mask.height());
        }

    } else {
        return bar()->fontMetrics().size(0, m_text);
    }
}

void E1Button::setPixmap(const QPixmap &pix, bool stretch, bool tint)
{
    m_stretch = stretch;
    m_tint = tint;
    if(m_tint)
        m_mask = pix.createMaskFromColor(QColor(0,0,0,255));
    else
        m_mask = pix;

    if(bar())
        bar()->update();
}

void E1Button::setText(const QString &text, bool tint)
{
    m_text = text;
    m_tintText = tint;
    if(bar())
        bar()->update();
}

void E1Button::paint(QPainter *painter, const QRect &rect, const QColor &color)
{
    if(m_text.isEmpty()) {
        // Scale mask
        QPixmap pix;
        if(m_tint) {
            QRegion reg(m_mask);
            pix = QPixmap(m_mask.rect().width(), m_mask.rect().height());
            pix.fill(QColor(0,0,0,0));

            reg = QRegion(pix.rect()).subtract(reg);
            {
                QPainter p(&pix);
                p.setClipRegion(reg);
                p.fillRect(pix.rect(), color);
            }
        } else {
            pix = m_mask;
        }

        if(m_stretch)
            pix = pix.scaledToHeight(rect.height());

        painter->drawPixmap(rect.x() + (rect.width() - pix.width()) / 2,
                            rect.y() + (rect.height() - pix.height()) / 2, pix);
    } else {
        // Draw the text
        if(m_tintText)
            painter->setPen(color);
        else
            painter->setPen(Qt::black);

        // Determine if we need to shrink the font
        QFont f = bar()->font();

        QFontMetrics fm(f);
        while(fm.width(m_text) > rect.width() && f.pointSize() > 2) {
            f.setPointSize(f.pointSize() - 1);
            fm = QFontMetrics(f);
        }

        painter->save();
        painter->setFont(f);
        painter->drawText(rect, m_text, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
        painter->restore();
    }
}

// define E1CloseButton
E1CloseButton::E1CloseButton()
{
    setPixmap(QPixmap(":image/samples/e1_close"));
}

void E1CloseButton::clicked()
{
    if((flags() & Enabled) && bar())
        bar()->window()->close();

    E1Button::clicked();
}

// define E1Menu
E1Menu::E1Menu()
: m_menu(0)
{
}

void E1Menu::setMenu(E1Popup *menu)
{
    // XXX memory cleanup
    m_menu = menu;
}

E1Popup *E1Menu::menu()
{
    if(!m_menu)
        m_menu = new E1Popup();

    return m_menu;
}

void E1Menu::showMenu()
{
    if(!m_menu)
        return;

    QPoint pp = bar()->mapToGlobal(m_lastDraw.topRight());
    pp.setX(pp.x() + 4);
    m_menu->popup(pp);
}

void E1Menu::activateMenu()
{
    showMenu();
}

void E1Menu::paint(QPainter *p, const QRect &rect, const QColor &c)
{
    m_lastDraw = rect;
    E1Button::paint(p, rect, c);
}

void E1Menu::clicked()
{
    if(m_menu)
        activateMenu();

    E1Button::clicked();
}

// define E1Spacer
E1Spacer::E1Spacer(int space)
: m_space(space)
{
}

QSize E1Spacer::sizeHint() const
{
    return QSize(m_space, bar()?bar()->height():-1);
}

