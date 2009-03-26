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

#include "qthumbstyle_p.h"
#include <QApplication>
#include <QAbstractScrollArea>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QAbstractItemView>
#include <QBasicTimer>
#include <QDebug>
#include <QTextEdit>
#include <QMessageBox>

static int spinArrowWidth = 16;             // spinbox arrow width

class QThumbStylePrivate : public QObject
{
    Q_OBJECT
public:
    QThumbStylePrivate();
    ~QThumbStylePrivate();

    bool handleMousePress(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e);
    bool handleMouseMove(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e);
    bool handleMouseRelease(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e);

    bool eventFilter(QObject *o, QEvent *e);

    QSize editableStrut;

protected:
    void timerEvent(QTimerEvent *e);

private:
    QPoint mousePos;
    Qt::MouseButtons buttons;
    QPointer<QAbstractScrollArea> scrollArea;
    QPointer<QWidget> target;
    bool filterPress;
    bool pressed;
    int moveThreshold;
    QBasicTimer ptimer;
};

QThumbStylePrivate::QThumbStylePrivate()
{
    scrollArea = 0;
    target = 0;
    filterPress = false;
    pressed = false;
    moveThreshold = QApplication::desktop()->screenGeometry().width()/40;
    qApp->installEventFilter(this);
}

QThumbStylePrivate::~QThumbStylePrivate()
{
    qApp->removeEventFilter(this);
}

bool QThumbStylePrivate::handleMousePress(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e)
{
    if (!e->spontaneous())
        return false;
    if (e->button() == Qt::LeftButton && !qobject_cast<QSlider*>(t)) {
        target = t;
        QWidget *cw = t;
        while (cw) {
            if (cw->focusPolicy() != Qt::NoFocus)
                break;
            cw = cw->parentWidget();
        }
        if (!cw) {
            // If we click outside a focusable widget, clear focus
            // to hide the input method.
            if (QApplication::focusWidget() && w)
                QApplication::focusWidget()->clearFocus();
        }

        if (w) {
            scrollArea = w;
            mousePos = e->globalPos();
            buttons = e->buttons();
            filterPress = false;
//            e->accept();
            ptimer.start(250, this);
            pressed = true;
            return true;
        }
    } else {
        target = 0;
        scrollArea = 0;
        filterPress = false;
    }

    return false;
}

bool QThumbStylePrivate::handleMouseMove(QAbstractScrollArea *w, QWidget* /*t*/, QMouseEvent *e)
{
    if (!e->spontaneous())
        return false;
    if (scrollArea && scrollArea == w) {
        QPoint diff = mousePos - e->globalPos();
        if (!filterPress
            && (qAbs(diff.y()) > moveThreshold 
            || qAbs(diff.x()) > moveThreshold)) {
            filterPress = true;
            if (!ptimer.isActive()) {
                // bogus move to get e.g. QPushButtons unpressed.
                QPoint bogusPos(-1,-1);
                QApplication::postEvent(target,
                        new QMouseEvent(QEvent::MouseMove,
                                        target->mapFromGlobal(bogusPos),
                                        bogusPos, Qt::LeftButton, buttons,
                                        QApplication::keyboardModifiers()));
            }
            ptimer.stop();
            diff = QPoint(0,0); // avoid jump
            if (QAbstractItemView *iv = qobject_cast<QAbstractItemView *>(w))
                iv->clearSelection();
        }
        if (filterPress) {
            QScrollBar *sb = scrollArea->verticalScrollBar();
            if (diff.y() && sb->isVisible() && sb->isEnabled() && sb->height()) {
                int moveY = diff.y() * sb->pageStep() / sb->height();
                sb->setValue(sb->value() + moveY);
            }

            sb = scrollArea->horizontalScrollBar();
            if (diff.x() && sb->isVisible() && sb->isEnabled() && sb->width()) {
                int moveX = diff.x() * sb->pageStep() / sb->width();
                sb->setValue(sb->value() + moveX);
            }

            mousePos = e->globalPos();
            return true;
        }
        if (ptimer.isActive())
            return true;
    }

    return false;
}

bool QThumbStylePrivate::handleMouseRelease(QAbstractScrollArea *sa, QWidget * /*t*/, QMouseEvent *e)
{
    if (!e->spontaneous())
        return false;
    if (e->button() == Qt::LeftButton) {
        pressed = false;
        if (target) {
            if (sa != scrollArea) {
                target = 0;
                scrollArea = 0;
                filterPress = false;
                return false;
            }
            scrollArea = 0;
            if (filterPress) {
                // Don't send any release
                target = 0;
                ptimer.stop();
                e->accept();
                filterPress = false;
                return true;
            } else {
                QWidget *fw = target;
                while (fw) {
                    if (fw->isEnabled() && fw->focusPolicy() != Qt::NoFocus) {
                        fw->setFocus(Qt::MouseFocusReason);
                        break;
                    }
                    if (fw->isWindow())
                        break;
                    fw = fw->parentWidget();
                }
            }
            if (ptimer.isActive()) {
                ptimer.start(0, this);
                return true;
            }
        }
    }

    return false;
}

bool QThumbStylePrivate::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QObject::eventFilter(o, e);

    QWidget *widget = (QWidget*)o;

    if (e->type() == QEvent::MouseButtonPress
            || e->type() == QEvent::MouseMove
            || e->type() == QEvent::MouseButtonRelease) {
        QAbstractScrollArea *sa = 0;
        while (widget->parentWidget() && !widget->isWindow()) {
            if (widget->objectName() == QLatin1String("qt_scrollarea_viewport")) {
                sa = qobject_cast<QAbstractScrollArea*>(widget->parentWidget());
            }
            widget = widget->parentWidget();
        }
        switch (e->type()) {
            case QEvent::MouseButtonPress:
                if (handleMousePress(sa, (QWidget*)o, (QMouseEvent*)e))
                    return true;
                break;
            case QEvent::MouseMove:
                if (handleMouseMove(sa, (QWidget*)o, (QMouseEvent*)e))
                    return true;
                break;
            case QEvent::MouseButtonRelease:
                if (handleMouseRelease(sa, (QWidget*)o, (QMouseEvent*)e))
                    return true;
                break;
            default:
                break;
        }
    }

    return QObject::eventFilter(o, e);
}

void QThumbStylePrivate::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == ptimer.timerId()) {
        ptimer.stop();
        QApplication::postEvent(target, new QMouseEvent(QEvent::MouseButtonPress, target->mapFromGlobal(mousePos), mousePos, Qt::LeftButton, buttons, QApplication::keyboardModifiers()));
        if (!pressed)
            QApplication::postEvent(target, new QMouseEvent(QEvent::MouseButtonRelease, target->mapFromGlobal(mousePos), mousePos, Qt::LeftButton, buttons, QApplication::keyboardModifiers()));
    }
}


//===========================================================================

QThumbStyle::QThumbStyle() : QPhoneStyle()
{
    d = new QThumbStylePrivate;

    int dpi = QApplication::desktop()->screen()->logicalDpiY();

    // 30 and 20 pixels on a 100dpi screen
    int hstrutSize = qRound(30.0 * dpi / 100.0);
    int vstrutSize = qRound(20.0 * dpi / 100.0);
    d->editableStrut = QSize(hstrutSize, vstrutSize);

    spinArrowWidth = qRound(18.0 * dpi / 100.0);
}

QThumbStyle::~QThumbStyle()
{
    delete d;
}

void QThumbStyle::polish(QWidget *widget)
{
    QAbstractItemView *aiv = qobject_cast<QAbstractItemView*>(widget);
    if (aiv) {
        aiv->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        aiv->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    }
    if (qobject_cast<QTabWidget*>(widget))
        widget->setFocusPolicy(Qt::NoFocus);

    if (widget->focusPolicy() & Qt::ClickFocus) {
        // We'll handle click to focus ourselves.
        widget->setFocusPolicy(Qt::TabFocus);
    }

    QAbstractSpinBox *sb = qobject_cast<QAbstractSpinBox*>(widget);
    if (sb) {
        sb->setAlignment(Qt::AlignHCenter);
    }

    QPhoneStyle::polish(widget);

    QTextEdit *te = qobject_cast<QTextEdit*>(widget);
    if (te && te->document()) {
        // We'd like links to be about 0.7cm high (~20pt)
        QString sheet("a { color: palette(link); font-size:20pt; }; a:visited { color: palette(link-visited); font-size:20pt; };");
        te->document()->setDefaultStyleSheet(sheet);
    }
}

/*!
    \reimp
*/
QRect QThumbStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                SubControl sc, const QWidget *w) const
{
    QRect ret;

    switch (cc) {
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QSize bs;
            int fw = spinbox->frame ? pixelMetric(PM_SpinBoxFrameWidth, spinbox, w) : 0;
            bs.setHeight(spinbox->rect.height());
            bs.setWidth(spinArrowWidth);
            bs = bs.expandedTo(QApplication::globalStrut());
            //int y = fw;
            int x, lx, rx;
            x = spinbox->rect.width() - bs.width();
            lx = fw;
            rx = x - bs.width() - fw * 2;
            switch (sc) {
            case SC_SpinBoxUp:
                ret = QRect(x, 0, bs.width(), bs.height());
                break;
            case SC_SpinBoxDown:
                ret = QRect(lx, 0, bs.width(), bs.height());
                break;
            case SC_SpinBoxEditField:
                ret = QRect(bs.width(), fw, rx, spinbox->rect.height() - 2*fw);
                break;
            case SC_SpinBoxFrame:
                ret = spinbox->rect;
            default:
                break;
            }
            ret = visualRect(spinbox->direction, spinbox->rect, ret);
        }
        break;
    default:
        ret = QPhoneStyle::subControlRect(cc, opt, sc, w);
    }

    return ret;
}

QSize QThumbStyle::sizeFromContents(ContentsType type, const QStyleOption* opt,
                                const QSize &csz, const QWidget *widget ) const
{
    QSize sz(csz);
    switch (type) {
    case CT_LineEdit:
    case CT_ComboBox:
    case CT_TabBarTab:
    case CT_CheckBox:
    case CT_ToolButton:
    case CT_PushButton:
        sz = QPhoneStyle::sizeFromContents(type, opt, csz, widget);
        sz = sz.expandedTo(d->editableStrut);
        break;
    case CT_MenuItem: {
        QStyleOptionMenuItem *mopt = (QStyleOptionMenuItem*)opt;
        sz = QPhoneStyle::sizeFromContents(type, opt, csz, widget);
        if (mopt->menuItemType == QStyleOptionMenuItem::Normal
            || mopt->menuItemType == QStyleOptionMenuItem::DefaultItem
            || mopt->menuItemType == QStyleOptionMenuItem::SubMenu)
            sz = sz.expandedTo(d->editableStrut);
        break; }
    case CT_SpinBox:
        sz.setWidth(sz.width() + spinArrowWidth / 2);   //add a little padding
        break;
    default:
        sz = QPhoneStyle::sizeFromContents(type, opt, csz, widget);
        break;
    }

    return sz;
}

int QThumbStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_TabBarIconSize:
        ret = QtopiaStyle::pixelMetric(PM_TabBarIconSize, option, widget);  //bigger than normal QPhoneStyle tabicons
        break;

    case PM_ButtonIconSize:
    case PM_ToolBarIconSize:
    case PM_SmallIconSize: {
            static int size = 0;
            if (!size) {
                // We would like a 20x20 icon at 100dpi
                size = (20 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
            }
            ret = size;
        }
        break;

    case PM_LargeIconSize:
    case PM_MessageBoxIconSize:
    case PM_IconViewIconSize: {
            static int size = 0;
            if (!size) {
                // We would like a 32x32 icon at 100dpi
                size = (32 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
            }
            ret = size;
        }
        break;

    case PM_ListViewIconSize: {
            static int size = 0;
            if (!size) {
                // We would like a 22x20 icon at 100dpi
                size = (20 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
            }
            ret = size;
        }
        break;
    case PM_MenuScrollerHeight:
        ret = d->editableStrut.height();
        break;
    case PM_TabBarScrollButtonWidth:
        ret = d->editableStrut.width();
        break;
    default:
        ret = QPhoneStyle::pixelMetric(metric, option, widget);
    }

    return ret;
}


#include "qthumbstyle_p.moc"
