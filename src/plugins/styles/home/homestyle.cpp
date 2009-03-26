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

#include "homestyle.h"

#include <qtopianamespace.h>
#include <QApplication>
#include <QStyleOption>
#include <QPainter>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QPalette>
#include <QLabel>
#include <QLayout>
#include <QFormLayout>
#include <QDesktopWidget>
#include <gfx.h>
#include <gfxpainter.h>
#include <QValueSpaceItem>
#include <QtopiaIpcEnvelope>
#include <QDebug>
#include <private/qtopiainputdialog_p.h>
#include <QSet>
#include <QAbstractItemView>
#include <private/homewidgets_p.h>

const int PushButtonMargin = 12;
static const QColor callsColor(244,152,23);

class CallMonitor : public QObject
{
    Q_OBJECT
public:
    static CallMonitor *instance() {
        static CallMonitor *inst = 0;
        if (!inst)
            inst = new CallMonitor();
        return inst;
    }

    bool callsActive() const {
        return active;
    }

    QString duration() const {
        return vsiDuration->value().toString();
    }

signals:
    void callsActiveChanged(bool);
    void callDurationChanged(const QString &duration);

private slots:
    void callStatusChanged() {
        bool newActive = !vsiActive->value().toString().isEmpty();
        if (newActive != active) {
            active = newActive;
            emit callsActiveChanged(active);
        }
    }

    void durationChanged() {
        emit callDurationChanged(vsiDuration->value().toString());
    }

private:
    CallMonitor() : QObject(), active(false) {
        vsiActive = new QValueSpaceItem("/Communications/Calls/Primary/Identifier", this);
        connect(vsiActive, SIGNAL(contentsChanged()), this, SLOT(callStatusChanged()));
        vsiDuration = new QValueSpaceItem("/Communications/Calls/Primary/Duration", this);
        connect(vsiDuration, SIGNAL(contentsChanged()), this, SLOT(durationChanged()));
        active = !vsiActive->value().toString().isEmpty();
    }
    QValueSpaceItem *vsiActive;
    QValueSpaceItem *vsiDuration;
    bool active;
};

class CallButton : public QToolButton
{
    Q_OBJECT
public:
    CallButton(QWidget *parent=0) : QToolButton(parent) {
        setText(tr("Call"));
        setFocusPolicy(Qt::NoFocus);
        QPalette pal = palette();
        pal.setBrush(QPalette::Button, callsColor);
        setPalette(pal);
        connect(CallMonitor::instance(), SIGNAL(callDurationChanged(QString)),
                this, SLOT(updateDuration(QString)));
        connect(CallMonitor::instance(), SIGNAL(callsActiveChanged(bool)),
                this, SLOT(setVisible(bool)));
        setVisible(CallMonitor::instance()->callsActive());
        connect(this, SIGNAL(clicked()), this, SLOT(showCalls()));
        updateDuration(CallMonitor::instance()->duration());
    }

private slots:
    void updateDuration(const QString &duration) {
        if (!duration.isEmpty()) {
            QString label(tr("Call %1", "call duration"));
            setText(label.arg(duration));
        } else {
            setText(tr("Call"));
        }
    }

    void showCalls() {
        QtopiaIpcEnvelope e("QPE/System", "showCallScreen()");
    }
};

class TitleWidget : public QWidget
{
    Q_OBJECT
public:
    TitleWidget(QWidget *parent);

protected:
    bool eventFilter(QObject* watched, QEvent* event);
private:
    QToolButton *cancelBtn;
    QToolButton *acceptBtn;
    QLabel *caption;
};

TitleWidget::TitleWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);
    hb->setSpacing(0);
    QDialog *dialog = qobject_cast<QDialog*>(parent);
    if (dialog && dialog->isModal()) {
        // Cancel
        cancelBtn = new QToolButton(this);
        QPalette pal = cancelBtn->palette();
        pal.setBrush(QPalette::Button, QtopiaHome::standardColor(QtopiaHome::Red));
        cancelBtn->setPalette(pal);
        cancelBtn->setText(tr("Cancel"));
        cancelBtn->setFocusPolicy(Qt::NoFocus);
        connect(cancelBtn, SIGNAL(clicked()), parent, SLOT(reject()));
        hb->addWidget(cancelBtn);
    }

    // Caption
    caption = new QLabel(parent->windowTitle(), this);
    caption->setAlignment(Qt::AlignCenter);
    QPalette pal = caption->palette();
    pal.setBrush(QPalette::Window, pal.brush(QPalette::Button));
    caption->setPalette(pal);
    caption->setAutoFillBackground(true);
    hb->addWidget(caption);

    // Return to call
    CallButton *callPB = new CallButton(this);
    hb->addWidget(callPB);

    if (dialog && dialog->isModal()) {
        // accept
        acceptBtn = new QToolButton(this);
        pal = acceptBtn->palette();
        pal.setBrush(QPalette::Button, QtopiaHome::standardColor(QtopiaHome::Green));
        acceptBtn->setPalette(pal);
        acceptBtn->setText(tr("Done"));
        acceptBtn->setFocusPolicy(Qt::NoFocus);
        connect(acceptBtn, SIGNAL(clicked()), parent, SLOT(accept()));
        hb->addWidget(acceptBtn);
    } else {
        // Back
        QToolButton *backPB = new QToolButton(this);
        backPB->setText(tr("Back"));
        backPB->setFocusPolicy(Qt::NoFocus);
        connect(backPB, SIGNAL(clicked()), parent, SLOT(accept()));
        hb->addWidget(backPB);
    }
    parent->installEventFilter(this);
}

bool TitleWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (!watched->isWidgetType())
        return false;

    QWidget *widget = qobject_cast<QWidget*>(watched);

    switch(event->type()) {
        case QEvent::WindowTitleChange: {
            caption->setText(widget->windowTitle());
            break;
        }
        break;
    default:
        break;
    }
    return false;
}

//===========================================================================

class HomeStylePrivate : public QObject
{
    Q_OBJECT
public:
    HomeStylePrivate(QObject *parent=0);

    void setupTabWidget(QTabWidget *tabWidget);
    void addPopupDialog(QDialog *dlg);

    QSet<QDialog*> popupDialogs;

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    bool paintPopupDialogBackground(QWidget *mb);

private slots:
    void objectDestroyed(QObject *o);

private:
    QWidget *addTitleWidget(QWidget *widget);
};

HomeStylePrivate::HomeStylePrivate(QObject *parent)
    : QObject(parent)
{
}

void HomeStylePrivate::setupTabWidget(QTabWidget *tabWidget)
{
    QDialog *dialog = qobject_cast<QDialog*>(tabWidget->window());
    if (!dialog)
        return;
    CallMonitor::instance()->callsActive();
    if (dialog->isModal()) {
        // Cancel
        QToolButton *cancelPB = new QToolButton(tabWidget);
        cancelPB->setText(tr("Cancel"));
        cancelPB->setFocusPolicy(Qt::NoFocus);
        QPalette pal = cancelPB->palette();
        pal.setBrush(QPalette::Button, QtopiaHome::standardColor(QtopiaHome::Red));
        cancelPB->setPalette(pal);
        tabWidget->setCornerWidget(cancelPB, Qt::TopLeftCorner);
        connect(cancelPB, SIGNAL(clicked()), dialog, SLOT(reject()));
    }

    QWidget *container = new QWidget(tabWidget);
    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->setMargin(0);
    hbl->setSpacing(0);
    // Return to call
    CallButton *callPB = new CallButton(container);
    hbl->addWidget(callPB);
    // Done
    QToolButton *savePB = new QToolButton(container);
    savePB->setFocusPolicy(Qt::NoFocus);
    if (dialog->isModal()) {
        savePB->setText(tr("Done"));
        QPalette pal = savePB->palette();
        pal.setBrush(QPalette::Button, QtopiaHome::standardColor(QtopiaHome::Green));
        savePB->setPalette(pal);
    } else {
        savePB->setText(tr("Back"));
    }
    hbl->addWidget(savePB);
    container->setLayout(hbl);
    tabWidget->setCornerWidget(container, Qt::TopRightCorner);
    connect(savePB, SIGNAL(clicked()), dialog, SLOT(accept()));
}

void HomeStylePrivate::addPopupDialog(QDialog *dlg)
{
    if (popupDialogs.contains(dlg))
        return;

    dlg->setAttribute(Qt::WA_OpaquePaintEvent, false);
    dlg->setAutoFillBackground(true);
    QPalette pal = dlg->palette();
    pal.setBrush(QPalette::Window, Qt::NoBrush);
    dlg->setPalette(pal);
    dlg->setWindowFlags(Qt::Dialog|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
    if (dlg->layout())
        dlg->layout()->setMargin(40);
    popupDialogs.insert(dlg);
    connect(dlg, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
}

bool HomeStylePrivate::paintPopupDialogBackground(QWidget *mb)
{
    if (mb->layout()) {
        QPainter p(mb);
        p.fillRect(mb->rect(), QColor(64, 64, 64, 192));
        p.setPen(QPen(QColor(64,64,64), 2));
        p.setBrush(QColor(128, 128, 128, 224));
        p.setRenderHint(QPainter::Antialiasing);
        QRect rect = mb->childrenRect().adjusted(-10, -10, 10, 10);
        p.drawRoundedRect(rect, 6, 6);
        return true;
    }

    return false;
}

bool HomeStylePrivate::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return false;

    bool ret = false;
    QWidget *widget = (QWidget*)o;
    switch(e->type()) {
    case QEvent::ParentChange:
        if (!widget->isWindow())
            widget->removeEventFilter(this);
        break;
    case QEvent::Show:
    case QEvent::Resize: {
        QDialog *dlg = qobject_cast<QDialog*>(widget);
        if (dlg && !popupDialogs.contains(dlg)) {
            if (QWidget *tw = addTitleWidget(widget))
                tw->resize(widget->width(), tw->sizeHint().height());
        }
        break;
    }
    case QEvent::Paint: {
        QDialog *dlg = qobject_cast<QDialog*>(widget);
        if (dlg && popupDialogs.contains(dlg))
            ret = paintPopupDialogBackground(widget);
        break;
    }
    case QEvent::LayoutRequest: {
        // Setup margins around popup dialog
        QDialog *dlg = qobject_cast<QDialog*>(widget);
        if (dlg && popupDialogs.contains(dlg)) {
            if (widget->layout())
                widget->layout()->setMargin(40);
        }
        break;
    }
    case QEvent::DynamicPropertyChange:
        if (widget->property("QHWindowStyle").toString() == QLatin1String("PopupDialog"))
            addPopupDialog(qobject_cast<QDialog*>(widget));
        break;
    default:
        break;
    }

    return ret;
}

void HomeStylePrivate::objectDestroyed(QObject *o)
{
    popupDialogs.remove((QDialog*)o);
}

QWidget *HomeStylePrivate::addTitleWidget(QWidget *widget)
{
    if ((widget->windowFlags() & Qt::Tool) == Qt::Tool
        || (widget->windowFlags() & Qt::Popup) == Qt::Popup) {
        return 0;
    }
    QList<QTabWidget*> tabWidgets = widget->findChildren<QTabWidget*>();
    if (tabWidgets.count())
        return 0;
    QList<TitleWidget*> titleWidgets = widget->findChildren<TitleWidget *>();
    if (titleWidgets.count())
        return titleWidgets[0];

    QLayout *tl = widget->layout();
    TitleWidget *w = new TitleWidget(widget);
    w->setGeometry(QRect(QPoint(0, 0), w->sizeHint()));
    w->show();
    if (tl) {
        int left, top, right, bottom;
        tl->getContentsMargins(&left, &top, &right, &bottom);
        tl->setContentsMargins(left, top + w->sizeHint().height(), right, bottom);
        tl->activate();
    } else {
        int left, top, right, bottom;
        widget->getContentsMargins(&left, &top, &right, &bottom);
        widget->setContentsMargins(left, top + w->sizeHint().height(), right, bottom);
    }
    return w;
}

//===========================================================================

HomeStyle::HomeStyle()
    : QThumbStyle()
{
    d = new HomeStylePrivate(this);
}

HomeStyle::~HomeStyle()
{}

void HomeStyle::polish(QWidget *widget)
{
    QThumbStyle::polish(widget);
    if (qobject_cast<QTextEdit *>(widget) || qobject_cast<QLineEdit *>(widget) || qobject_cast<QComboBox *>(widget)
        || qobject_cast<QAbstractSpinBox *>(widget)) {
        if (!qobject_cast<QtopiaInputDialog*>(widget->window()))
            widget->setFocusPolicy(Qt::NoFocus);
    }
    if (qobject_cast<QLineEdit*>(widget) || qobject_cast<QComboBox*>(widget)) {
        QPalette pal = widget->palette();
        QColor temptext = pal.color(QPalette::Normal, QPalette::Text);
        pal.setColor(QPalette::Normal, QPalette::Text, pal.color(QPalette::Normal, QPalette::WindowText));
        widget->setPalette(pal);
    }
    if (QTabWidget *tabWidget = qobject_cast<QTabWidget*>(widget)) {
        if ((tabWidget->window()->windowFlags() & Qt::Dialog) == Qt::Dialog)
            d->setupTabWidget(tabWidget);
    }
    if (QTabBar *tabBar = qobject_cast<QTabBar*>(widget)) {
        tabBar->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    }

    if (widget->isWindow() && widget->windowType() == Qt::Dialog) {
        widget->installEventFilter(d);
        if (widget->property("QHWindowStyle").toString() == QLatin1String("PopupDialog"))
            d->addPopupDialog(qobject_cast<QDialog*>(widget));
    }

    QMessageBox *mb = qobject_cast<QMessageBox*>(widget);
    if (mb || widget->inherits("PhoneMessageBox"))
        d->addPopupDialog(qobject_cast<QDialog*>(widget));

    //undo alignment set in QThumbStyle
    QAbstractSpinBox *sb = qobject_cast<QAbstractSpinBox*>(widget);
    if (sb && !qobject_cast<QtopiaInputDialog*>(widget->window())) {
        sb->setAlignment(Qt::AlignLeft);
    }

    //undo background changes made in QPhoneStyle
    QTextEdit *te = qobject_cast<QTextEdit*>(widget);
    if (te)
        te->viewport()->setBackgroundRole(QPalette::Base);
    QAbstractItemView *aiv = qobject_cast<QAbstractItemView*>(widget);
    if (aiv)
        aiv->viewport()->setBackgroundRole(QPalette::Base);
}

void HomeStyle::unpolish(QWidget *widget)
{
    QThumbStyle::unpolish(widget);
}

int HomeStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_TabBarTabVSpace:
        ret = 12;
        break;
    case PM_TabBarTabHSpace:
    case PM_TabBarTabOverlap:
        ret = 0;
//        ret = QWindowsStyle::pixelMetric(metric, option, widget);
        break;
    case PM_TabBarTabShiftVertical:
        ret = 0;
        break;
    case PM_ButtonIconSize:
    case PM_ToolBarIconSize:
    case PM_SmallIconSize:
        ret = 20;
        break;
    case PM_ListViewIconSize:
        ret = 30;
        break;
    case PM_SliderControlThickness:
        {
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
                QRect rct(slider->rect);
                rct.adjust(2,2,-2,-2);
                ret = slider->orientation == Qt::Vertical ? rct.width() : rct.height();
            } else
                ret = 0;
        }
        break;
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        ret = 10;
        break;
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        ret = 1;
        break;
    default:
        ret = QThumbStyle::pixelMetric(metric, option, widget);
    }

    return ret;
}

void HomeStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget) const
{
    switch(ce) {
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            QStyleOptionTabV2 tabV2(*tab);
            QRect r = tabV2.rect;
            bool verticalTabs = tabV2.shape == QTabBar::RoundedEast
                                || tabV2.shape == QTabBar::RoundedWest
                                || tabV2.shape == QTabBar::TriangularEast
                                || tabV2.shape == QTabBar::TriangularWest;
            bool bottomTabs = tabV2.shape == QTabBar::RoundedSouth
                                || tabV2.shape == QTabBar::TriangularSouth;
            bool selected = tabV2.state & State_Selected;
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            bool lastTab = (tab->direction == Qt::LeftToRight &&
                            tab->position == QStyleOptionTab::End) ||
                           (tab->direction == Qt::RightToLeft &&
                            tab->position == QStyleOptionTab::Beginning);
            int overlap = (onlyOne || lastTab) ? 0 : pixelMetric(PM_TabBarTabOverlap, opt, widget);
            int verticalOffset = selected ? 0 : pixelMetric(PM_TabBarTabShiftVertical, opt, widget);

            if (verticalTabs || bottomTabs) {   //use Windows style for "non-standard" tabs
                QWindowsStyle::drawControl(ce, opt, p, widget);
                break;
            }

            r.adjust(0,verticalOffset,overlap,0);

            p->setRenderHint(QPainter::Antialiasing);

            QColor bg = opt->palette.color(selected ? QPalette::Highlight : QPalette::Button);
            QLinearGradient bgg(r.x(), r.y(), r.x(), r.bottom());
            bgg.setColorAt(0.0, bg.lighter(160));
            bgg.setColorAt(1.0f, bg);
            p->setBrush(bgg);
            //TODO: cache gradient

            p->setPen(Qt::NoPen);
            p->drawRect(r);

            p->setPen(QPen(bg));
            p->drawLine(r.left(), r.bottom(), r.left(), r.top());
            p->drawLine(r.left(), r.top(), r.right(), r.top());
            p->drawLine(r.right(), r.top(), r.right(), r.bottom());
        }
        break;
    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            QStyleOptionTabV2 tabV2(*tab);
            QRect tr = tabV2.rect;
            bool verticalTabs = tabV2.shape == QTabBar::RoundedEast
                                || tabV2.shape == QTabBar::RoundedWest
                                || tabV2.shape == QTabBar::TriangularEast
                                || tabV2.shape == QTabBar::TriangularWest;
            if (verticalTabs || tab->text.isEmpty()) {
                QWindowsStyle::drawControl(ce, opt, p, widget);
            } else {
                // Only show text if it is present.
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                drawItemText(p, tr, alignment, tab->palette, tab->state & State_Enabled, tab->text, QPalette::WindowText);
            }
        }
        break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QRect textRect = button->rect;
            uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::AlignHCenter;
            if (!styleHint(SH_UnderlineShortcut, button, widget))
                tf |= Qt::TextHideMnemonic;

            if (!button->icon.isNull()) {
                QRect iconRect;
                QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                if (mode == QIcon::Normal && button->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (button->state & State_On)
                    state = QIcon::On;

                QPixmap pixmap = button->icon.pixmap(button->iconSize, mode, state);
                int labelHeight = pixmap.height();
                int iconSpacing = 4;//### 4 is currently hardcoded in QPushButton::sizeHint()
                iconRect = QRect(textRect.x(),
                                 textRect.y() + (textRect.height() - labelHeight) / 2,
                                 pixmap.width(), pixmap.height());
                iconRect = visualRect(button->direction, textRect, iconRect);
                if (button->direction == Qt::RightToLeft)
                    textRect.setRight(iconRect.left() - iconSpacing);
                else
                    textRect.setLeft(iconRect.left() + iconRect.width() + iconSpacing);
                p->drawPixmap(iconRect, pixmap);
            }
            if (button->state & (State_On | State_Sunken))
                textRect.translate(pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                             pixelMetric(PM_ButtonShiftVertical, opt, widget));

            if (button->features & QStyleOptionButton::HasMenu) {
                int indicatorSize = pixelMetric(PM_MenuButtonIndicator, button, widget);
                if (button->direction == Qt::LeftToRight)
                    textRect = textRect.adjusted(0, 0, -indicatorSize, 0);
                else
                    textRect = textRect.adjusted(indicatorSize, 0, 0, 0);
            }
            drawItemText(p, textRect, tf, button->palette, (button->state & State_Enabled),
                         button->text, QPalette::Text);
        }
        break;

    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            QRect r(opt->rect);
            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
                p->setPen(menuitem->palette.color(QPalette::ButtonText));
                p->drawLine(r.topLeft(), r.topRight());
            } else {
                QStyleOptionMenuItem myMenuItem(*menuitem);
                QLinearGradient grad(r.x(), r.y(), r.x(), r.bottom());
                grad.setColorAt(0.0, myMenuItem.palette.color(QPalette::Button).lighter());
                grad.setColorAt(1.0, myMenuItem.palette.color(QPalette::Button));
                myMenuItem.palette.setBrush(QPalette::Button, grad);
                QThumbStyle::drawControl(ce, &myMenuItem, p, widget);
            }
        }
        break;
    default:
        QThumbStyle::drawControl(ce, opt, p, widget);
        break;
    }
}

void HomeStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
    QPainter *p, const QWidget *widget) const
{
    switch (cc) {
    case CC_Slider: {
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            p->setRenderHint(QPainter::Antialiasing);
            QRect rct (slider->rect);
            double normalized
                = ((double)slider->sliderValue - slider->minimum) / ((double)slider->maximum - slider->minimum);
            bool vertical
                = (slider->orientation == Qt::Vertical);
            if (vertical) {
                normalized = 1 - normalized;
                rct.adjust(2,2,-2,-2);
                int radius = rct.width() >> 1;
                QPoint valueCenter (
                        (int)qMin (qMax ((double)rct.x() + radius, (double)0), (double)rct.right() - radius),
                        (int)qMin (qMax ((double)rct.y() + radius, (double)(rct.y()+normalized*rct.height())), (double)rct.bottom() - radius)
                        );
                QPainterPath path;
                path.addRoundRect(rct,vertical?50:10,vertical?10:50);
                QLinearGradient grad (rct.x(),rct.y(),vertical?rct.right():rct.x(),vertical?rct.y():rct.bottom());
                grad.setColorAt(0,QColor(151,161,186));
                grad.setColorAt(0.5,QColor(100,116,139));
                grad.setColorAt(1,QColor(151,161,186));
                QBrush gradBrush (grad);
                p->setBrush(gradBrush);
                p->fillPath(path,gradBrush);
                p->setPen(QPen(QColor(153,153,153)));
                p->drawPath(path);
                QRadialGradient grad_circle(valueCenter,radius);
                grad_circle.setColorAt(0,QColor(255,255,255));
                grad_circle.setColorAt(1,QColor(232,232,232));
                QPainterPath path_sel;
                QBrush gradCircBrush (grad_circle);
                p->setBrush(gradCircBrush );
                path_sel.addEllipse(QRect(QPoint(valueCenter.x()-radius, valueCenter.y()-radius), QSize(radius*2+1,radius*2+1)));
                p->fillPath(path_sel,gradCircBrush );
                p->setPen(QPen(QColor(66,76,86,0xcc)));
                p->drawPath(path_sel);
            } else {
                int rad = (rct.height()*3)>>3;
                p->setPen(QPen(QColor(47,53,61)));
                p->setBrush(QBrush(QColor(110,125,146,0xcc)));
                rct.setHeight(rad<<1);
                p->fillRect(rct,p->brush());
                p->drawRect(rct);
                p->setPen(QPen(QColor(150,162,177)));
                rct.adjust(1,1,-1,-1);
                p->drawRect(rct);
                QPainterPath path;
                QPoint valueCenter (
                        (int)qMin (qMax ((double)rct.x() + (normalized*rct.width()), (double)rad), (double)rct.right() - rad),
                        rad*2 - (rad>>1)
                        );
                QRect smallEll (valueCenter,QSize(1,1));
                QRect bigEll (valueCenter,QSize(1,1));
                smallEll.adjust(0-(rad>>1),0-(rad>>1),rad>>1,rad>>1);
                bigEll.adjust(0-rad,0-rad,rad,rad);
                path.addEllipse(smallEll);
                path.addEllipse(bigEll);
                QRadialGradient grad_circle(valueCenter,rad);
                grad_circle.setColorAt(0,QColor(255,255,255));
                grad_circle.setColorAt(1,QColor(232,232,232));
                QBrush gradCircBrush (grad_circle);
                p->setBrush(gradCircBrush );
                p->fillPath(path,gradCircBrush );
                p->setPen(QPen(QColor(66,76,86,0xcc)));
                p->drawPath(path);
            }
        }
    }
    break;
    default:
        QThumbStyle::drawComplexControl(cc,opt,p,widget);
        break;
    }
 }

void HomeStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch(pe) {
    case PE_FrameButtonBevel:
    case PE_FrameButtonTool:
    case PE_PanelButtonTool:
    case PE_PanelButtonBevel: {
        QRectF r = opt->rect;
        r.adjust(0.5,0.5,-0.5,-0.5);

        bool enabled = opt->state & State_Enabled;
        bool focus = opt->state & State_HasFocus;
        bool pressed = opt->state & (State_On | State_Sunken);

        p->setRenderHint(QPainter::Antialiasing);

        QColor bg = opt->palette.color(!enabled ? QPalette::Window :
                (focus && !Qtopia::mousePreferred()) ? QPalette::Highlight : QPalette::Button);
        p->setPen(QPen(bg.darker(150), 1.5));
        if (pressed)
            bg = bg.darker(180);
        QLinearGradient bgg(r.x(), r.y(), r.x(), r.bottom());
        bgg.setColorAt(0.0f, pressed ? bg.darker(160) : bg);
        bgg.setColorAt(1.0f, pressed ? bg : bg.darker(160));
        p->setBrush(bgg);
        //TODO: cache gradient

        if (pe == PE_FrameButtonTool || pe == PE_PanelButtonTool)
            p->drawRect(r);
        else
            drawRoundRect(p, r, 30, 30);
        break; }
    case PE_PanelButtonCommand: {
            QRectF r = opt->rect;
            r.adjust(1.0,1.0,-1.0,-1.0);

            bool enabled = opt->state & State_Enabled;
            bool focus = opt->state & State_HasFocus;
            bool pressed = opt->state & (State_On | State_Sunken);

            QColor bg = opt->palette.color(!enabled ? QPalette::Button :
                    (focus && !Qtopia::mousePreferred()) ? QPalette::Highlight : QPalette::Base);
            p->setPen(QPen(bg, 2.0));
            if (pressed)
                bg = bg.darker(180);
            QLinearGradient bgg(r.x(), r.y(), r.x(), r.bottom());
            bgg.setColorAt(0.0, pressed ? bg.darker(160) : bg);
            bgg.setColorAt(1.0f, pressed ? bg : bg.darker(160));
            p->setBrush(bgg);
            p->setRenderHint(QPainter::Antialiasing);
            drawRoundRect(p, r, 30, 30);
        }
        break;
    case PE_FrameGroupBox: {
        p->setBrush(Qt::NoBrush);
        p->setPen(opt->palette.mid().color());
        QRect r = opt->rect;
        r.adjust(0,0,-1,-1);
        drawRoundRect(p, r, 8, 8);
        break; }
    case PE_FrameTabWidget:
        QThumbStyle::drawPrimitive(pe, opt, p, w);
        break;
    case PE_FrameMenu:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            QRectF rect = frame->rect;
            rect.adjust(0.5, 0.5, -0.5, -0.5);
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(QPen(opt->palette.color(QPalette::ButtonText).darker(130), 2));
            p->setBrush(Qt::NoBrush);
            drawRoundRect(p, rect, 16, 16);
        }
        break;
    default:
        QThumbStyle::drawPrimitive(pe, opt, p, w);
        break;
    }
}

QSize HomeStyle::sizeFromContents (ContentsType type, const QStyleOption * opt,
                                       const QSize & csz, const QWidget *widget) const
{
    QSize sz(csz);
    switch (type) {
    case CT_ToolButton:
        sz = QThumbStyle::sizeFromContents(type, opt, csz, widget);
        sz = sz.expandedTo(QSize(0,30));
        break;
    case CT_MenuItem: {
        QStyleOptionMenuItem *mopt = (QStyleOptionMenuItem*)opt;
        sz = QThumbStyle::sizeFromContents(type, opt, csz, widget);
        if (mopt->menuItemType == QStyleOptionMenuItem::Separator) {
            sz.setHeight(1);
        } else {
            if (mopt->menuItemType == QStyleOptionMenuItem::Normal
                    || mopt->menuItemType == QStyleOptionMenuItem::DefaultItem
                    || mopt->menuItemType == QStyleOptionMenuItem::SubMenu)
                sz = sz.expandedTo(QSize(0,26));
        }
        break; }
    case CT_PushButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            int w = csz.width(),
                h = csz.height(),
                bm = pixelMetric(PM_ButtonMargin, btn, widget),
            fw = pixelMetric(PM_DefaultFrameWidth, btn, widget) * 2;
            w += bm + fw;
            h += bm + fw;
            if (btn->features & QStyleOptionButton::AutoDefaultButton){
                int dbw = pixelMetric(PM_ButtonDefaultIndicator, btn, widget) * 2;
                w += dbw;
                h += dbw;
            }
            if (!btn->icon.isNull() && btn->text.isEmpty())
                w -= 4;
            w += 2 * PushButtonMargin;
            sz = QSize(w, h).expandedTo(QSize(0,30));;
        }
        break;
    case CT_TabBarTab:
        if (const QStyleOptionTabV2 *tab = qstyleoption_cast<const QStyleOptionTabV2 *>(opt)) {
            int w = 0;
            int h = 0;
            if ( (tab->direction == Qt::LeftToRight &&
                    tab->position == QStyleOptionTab::End) ||
                  (tab->direction == Qt::RightToLeft &&
                    tab->position == QStyleOptionTab::Beginning) ) {
                w += pixelMetric(PM_TabBarTabOverlap, opt, widget);
            }
            if (!tab->text.isEmpty() && !tab->icon.isNull()) {
                // We don't show an icon if text is present.
                w -= tab->iconSize.width();
            }
            w += 8;
            sz += QSize(w,h);
        }
        break;
    default:
        sz = QThumbStyle::sizeFromContents(type, opt, csz, widget);
        break;
    }

    return sz;
}

QRect HomeStyle::subElementRect(SubElement element, const QStyleOption *opt, const QWidget *w) const
{
    QRect r;
    switch (element) {
    case SE_TabWidgetTabBar:
        if (const QStyleOptionTabWidgetFrame *twf
                = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            r.setSize(twf->tabBarSize);
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                r.setWidth(twf->rect.width() - twf->leftCornerWidgetSize.width()
                        - twf->rightCornerWidgetSize.width());
                r.moveTopLeft(QPoint(twf->leftCornerWidgetSize.width(), 0));
                r = visualRect(twf->direction, twf->rect, r);
                break;
            default:
                r = QWindowsStyle::subElementRect(element, opt, w);
                break;
            }
        }
        break;
    case SE_TabWidgetTabPane:
    case SE_TabWidgetTabContents:
        r = QWindowsStyle::subElementRect(SE_TabWidgetTabPane, opt, w);
        break;
    case SE_PushButtonContents:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            int dx1, dx2;
            dx1 = pixelMetric(PM_DefaultFrameWidth, btn, w);
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                dx1 += pixelMetric(PM_ButtonDefaultIndicator, btn, w);
            dx2 = dx1 * 2;
            r.setRect(opt->rect.x() + dx1 + PushButtonMargin,
                    opt->rect.y() + dx1,
                    opt->rect.width() - dx2 - 2 * PushButtonMargin,
                    opt->rect.height() - dx2);
            r = visualRect(opt->direction, opt->rect, r);
        }
        break;
    default:
        r = QThumbStyle::subElementRect(element, opt, w);
    }
    return r;
}

/*!
    \reimp
*/
QRect HomeStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                SubControl sc, const QWidget *w) const
{
    if (w && qobject_cast<QtopiaInputDialog*>(w->window()))
        return QThumbStyle::subControlRect(cc, opt, sc, w);

    QRect ret;

    switch (cc) {
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            int fw = spinbox->frame ? pixelMetric(PM_SpinBoxFrameWidth, spinbox, w) : 0;
            switch (sc) {
            case SC_SpinBoxUp:
                ret = QRect(0, 0, 0, 0);
                break;
            case SC_SpinBoxDown:
                ret = QRect(0, 0, 0, 0);
                break;
            case SC_SpinBoxEditField:
                ret = QRect(fw, fw, spinbox->rect.width() - 2*fw, spinbox->rect.height() - 2*fw);
                break;
            case SC_SpinBoxFrame:
                ret = spinbox->rect;
            default:
                break;
            }
            ret = visualRect(spinbox->direction, spinbox->rect, ret);
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            int x = 0,
                y = 0,
                wi = cb->rect.width(),
                he = cb->rect.height();
            int margin = cb->frame ? 2 : 0;
            switch (sc) {
            case SC_ComboBoxFrame:
                ret = cb->rect;
                break;
            case SC_ComboBoxArrow:
                ret.setRect(0, 0, 0, 0);
                break;
            case SC_ComboBoxEditField:
                ret.setRect(x + margin, y + margin, wi - 2 * margin, he - 2 * margin);
                break;
            case SC_ComboBoxListBoxPopup:
                ret = cb->rect;
                break;
            default:
                break;
            }
            ret = visualRect(cb->direction, cb->rect, ret);
        }
        break;
    default:
        ret = QThumbStyle::subControlRect(cc, opt, sc, w);
    }

    return ret;
}

int HomeStyle::styleHint(StyleHint stylehint, const QStyleOption *option,
                           const QWidget *widget, QStyleHintReturn* returnData) const
{
    int ret = 0;
    switch (stylehint) {
    case SH_TabBar_Alignment:
        ret = Qt::AlignJustify;
        break;
    case SH_ExtendedFocusHighlight:
        ret = 0;
        break;
    case SH_PopupShadows:
        ret = 0;
        break;
    case SH_FormLayoutFormAlignment:
        return Qt::AlignLeft | Qt::AlignTop;
    case SH_FormLayoutLabelAlignment:
        return Qt::AlignLeft;
    default:
        ret = QThumbStyle::styleHint(stylehint, option, widget, returnData);
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

QStringList HomeStylePlugin::keys() const
{
    return QStringList() << "HomeStyle";
}

 QStyle *HomeStylePlugin::create(const QString &key)
{
    if (key.toLower() == "homestyle")
        return new HomeStyle;
    return 0;
}

QTOPIA_EXPORT_QT_PLUGIN(HomeStylePlugin)

#include "homestyle.moc"
