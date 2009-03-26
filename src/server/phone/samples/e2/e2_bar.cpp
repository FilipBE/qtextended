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

#include "e2_bar.h"
#include <QPainter>
#include <QMouseEvent>
#include <QHBoxLayout>
#include "e2_colors.h"
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

E2Button::E2Button(QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags),
  m_enabled(true),
  m_fillBrush(":image/samples/e2_button"),
  m_fillBrushPressed(":image/samples/e2_button_pressed"),
  m_menu(0),
  m_menuRemainDepressed(false),
  m_isPressed(false),
  m_menuUp(false)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0, 0, 0, 0));
    setPalette(pal);
    setFixedHeight(m_fillBrush.height() + 2);
}

E2Button::~E2Button()
{
    if(m_menu)
        delete m_menu;
}

void E2Button::setMenu(E2Menu *menu, bool remainDepressed)
{
    if(m_menu) {
        QObject::disconnect(m_menu, SIGNAL(closing()),
                            this, SLOT(menuClosing()));
    }

    m_menu = menu;

    if(m_menu) {
        QObject::connect(m_menu, SIGNAL(closing()),
                         this, SLOT(menuClosing()));
    }

    m_menuUp = false;
    m_menuRemainDepressed = remainDepressed;
}

void E2Button::menuClosing()
{
    m_menuUp = false;
    update();
}

void E2Button::setText(const QString &text)
{
    m_text = text;
    update();
}

void E2Button::setPixmap(const QPixmap &pix)
{
    m_pixmap = pix;
    update();
}

void E2Button::setEnabled(bool enabled)
{
    m_enabled = enabled;
    update();
}

void E2Button::popupMenu()
{
    if(m_menuUp || !m_menu)
        return;

    m_menuUp = true;
    m_menu->show();
    m_menu->raise();

    int x = 0;
    int y = 0;
    int w = m_menu->width();
    int h = m_menu->height();
    if(w < width()) w = width();

    QPoint me = mapToGlobal(QPoint(0,0));
    x = me.x();
    y = me.y() - 1 - h;

    m_menu->setGeometry(x, y, w, h);
}

void E2Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(Qt::black);
    p.drawLine(1, 0, width() - 2, 0);
    p.drawLine(0, 1, 0, height() - 2);
    p.drawLine(1, height() - 1, width() - 2, height() - 1);
    p.drawLine(width() - 1, 1, width() - 1, height() - 2);

    p.setPen(Qt::NoPen);
    bool pressedLook = m_isPressed || (m_menuUp && m_menuRemainDepressed) || !m_enabled;

    p.drawTiledPixmap(1, 1, width() - 2, height() - 2,
                      (pressedLook?m_fillBrushPressed:m_fillBrush));

    if(!m_pixmap.isNull() && m_text.isEmpty()) {
        p.drawPixmap((width() - m_pixmap.width()) / 2,
                     (height() - m_pixmap.height()) / 2,
                     m_pixmap);
    } else if(!m_text.isEmpty()) {
        if(m_enabled)
            p.setPen(Qt::black);
        else
            p.setPen(Qt::gray);

        p.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, m_text);

        if(!m_pixmap.isNull()) {
            // Draw along right edge
            p.drawPixmap(width() - m_pixmap.width() - 5,
                         (height() - m_pixmap.height()) / 2,
                         m_pixmap);
        }
    }
}

void E2Button::mousePressEvent(QMouseEvent *)
{
    m_isPressed = true;

    if(m_menu && m_menuRemainDepressed && m_enabled) {
        popupMenu();
        m_isPressed = false;
    }

    update();
}

void E2Button::mouseReleaseEvent(QMouseEvent *e)
{
    m_isPressed = false;
    if(rect().contains(e->pos())) {
        if(m_menu && m_enabled)
            popupMenu();

        if(m_enabled)
            emit clicked();
    }
    update();
}

E2Bar::E2Bar(QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags),
  m_layWidth(0),
  m_fillBrush(":image/samples/e2_bar"),
  m_buttonsStale(false)
{
    setFixedHeight(m_fillBrush.height());
}

void E2Bar::addButton(E2Button *but, int width)
{
    m_buttons.append(qMakePair(but, width));
    m_buttonsStale = true;
    but->setParent(this);
    relayout();
}

void E2Bar::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.drawTiledPixmap(rect(), m_fillBrush);
}

void E2Bar::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    relayout();
}

void E2Bar::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    relayout();
}

void E2Bar::relayout()
{
    if(isHidden() || (!m_buttonsStale && m_layWidth == width()) || m_buttons.isEmpty())
        return;

    // Determine "fill" width
    int fillButtons = 0;
    int filledWidth = 0;
    for(int ii = 0; ii < m_buttons.count(); ++ii) {
        if(0 == m_buttons.at(ii).second) {
            ++fillButtons;
        } else {
            filledWidth += m_buttons.at(ii).second;
        }
    }

    // Determine fill size
    int fillSize = (0 == fillButtons)?0:((width() - 2 /* 1 pixel either side */ - filledWidth - (m_buttons.count() - 1) * 2) / fillButtons);

    // Layout
    int uptoX = 1;
    for(int ii = 0; ii < m_buttons.count(); ++ii) {
        int butWidth = m_buttons.at(ii).second?m_buttons.at(ii).second:fillSize;
        m_buttons.at(ii).first->setGeometry(uptoX, 1, butWidth, height() - 2);
        uptoX += butWidth + 2;
    }

    m_buttonsStale = false;
    m_layWidth = width();
}

// declare E2MenuPrivate
class E2MenuPrivate : public QWidget
{
Q_OBJECT
public:
    E2MenuPrivate(QWidget *parent);

    void addItem(const QString &);
    void addSeparator();

    virtual QSize sizeHint() const;
signals:
    void itemClicked(int);
    void closeMe();

protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    virtual void keyPressEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *);

    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

public:
    int m_selected;
    QStringList m_items;
    int m_separators;
};

// define E2Menu
E2Menu::E2Menu(QWidget *parent, Qt::WFlags flags)
: E2PopupFrame(parent, flags), d(0)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);
    d = new E2MenuPrivate(this);
    layout->addWidget(d);
    QObject::connect(d, SIGNAL(itemClicked(int)),
                     this, SIGNAL(itemClicked(int)));
    QObject::connect(d, SIGNAL(closeMe()), this, SLOT(hide()));
    QObject::connect(d, SIGNAL(closeMe()), this, SIGNAL(closing()));
    setFocusProxy(d);
}

void E2Menu::replaceItem(int item, const QString &name)
{
    Q_ASSERT(item < d->m_items.count());
    d->m_items[item] = name;
    d->update();
}

void E2Menu::addItem(const QString &name)
{
    d->addItem(name);
}

void E2Menu::addSeparator()
{
    d->addSeparator();
}

void E2MenuPrivate::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
        case Qt::Key_Down:
            m_selected = (m_selected + 1) % (m_items.count() - m_separators);
            break;

        case Qt::Key_Up:
            m_selected = (m_selected - 1);
            if(m_selected < 0) m_selected =  (m_items.count() - m_separators) - 1;
            break;

        case Qt::Key_Select:
        case Qt::Key_Return:
            if(m_selected >= 0)
                emit itemClicked(m_selected);
            emit closeMe();
            break;

        default:
            QWidget::keyPressEvent(e);
            return;
    }

    update();
    e->accept();
}

// define E2MenuPrivate
E2MenuPrivate::E2MenuPrivate(QWidget *parent)
: QWidget(parent), m_selected(-1), m_separators(0)
{
    setMouseTracking(true);
}

void E2MenuPrivate::addItem(const QString &name)
{
    if(!name.isEmpty()) {
        m_items.append(name);
        update();
    }
}

void E2MenuPrivate::addSeparator()
{
    m_items.append(QString());
    ++m_separators;
}

QSize E2MenuPrivate::sizeHint() const
{
    QFontMetrics fm(font());
    int h = m_separators * 2 + (m_items.count() - m_separators) * fm.height();

    return QSize(100, h);
}

void E2MenuPrivate::paintEvent(QPaintEvent *)
{
    if(m_items.isEmpty())
        return;

    QPainter p(this);

    int sizePerItem = (m_items.count() == m_separators)?0:((height() - (m_separators * 2)) / (m_items.count() - m_separators));

    int uptoY = 0;
    int seenItems = 0;
    for(int ii = 0; ii < m_items.count(); ++ii) {
        if(m_items.at(ii).isEmpty()) {
            p.setPen(GREY);
            p.drawLine(1, uptoY, width() - 1, uptoY);
            p.setPen(REALLY_LIGHT_GREY);
            p.drawLine(1, uptoY + 1, width() - 1, uptoY + 1);
            uptoY += 2;
        } else {
            QRect textrect(20, uptoY, width() - 20, sizePerItem);
            if(seenItems == m_selected) {
                p.fillRect(1, uptoY, width() - 2, sizePerItem, REALLY_LIGHT_GREY);
                p.setPen(Qt::black);
                p.drawText(textrect, Qt::AlignVCenter,
                           m_items.at(ii));
            } else {
                p.setPen(Qt::white);
                p.drawText(textrect, Qt::AlignVCenter,
                           m_items.at(ii));
            }
            uptoY += sizePerItem;
            ++seenItems;
        }
    }
}

void E2MenuPrivate::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
    if(-1 == m_selected) {
        emit closeMe();
    }
}

void E2MenuPrivate::mouseReleaseEvent(QMouseEvent *e)
{
    if(-1 != m_selected && rect().contains(e->pos())) {
        emit itemClicked(m_selected);
        emit closeMe();
    }
}

void E2MenuPrivate::showEvent(QShowEvent *e)
{
    m_selected = -1;
    grabMouse();
    QWidget::showEvent(e);
}

void E2MenuPrivate::hideEvent(QHideEvent *e)
{
    releaseMouse();
    QWidget::hideEvent(e);
}

void E2MenuPrivate::mouseMoveEvent(QMouseEvent *e)
{
    // Need to find the item we're over
    int sizePerItem = (m_items.count() == m_separators)?0:((height() - (m_separators * 2)) / (m_items.count() - m_separators));

    int uptoY = 0;
    int seenItems = 0;
    for(int ii = 0; ii < m_items.count(); ++ii) {
        if(m_items.at(ii).isEmpty()) {
            uptoY += 2;
        } else {
            QRect r(0, uptoY, width(), sizePerItem);
            if(r.contains(e->pos())) {
                if(seenItems != m_selected) {
                    m_selected = seenItems;
                    update();
                }
                return;
            }
            ++seenItems;
            uptoY += sizePerItem;
        }
    }

    if(-1 != m_selected) {
        m_selected = -1;
        update();
    }
}

#include "e2_bar.moc"
