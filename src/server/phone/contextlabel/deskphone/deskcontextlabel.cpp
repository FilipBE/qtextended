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

#include "deskcontextlabel.h"
#include "themecontrol.h"

#include <QThemedView>
#include <QThemeTextItem>
#include <QThemeImageItem>
#include <QtopiaIpcEnvelope>
#include <QSoftMenuBar>
#include <qtopialog.h>
#include <QtopiaServiceRequest>
#include <QDesktopWidget>

#include <QResizeEvent>
#include <QDebug>
#include <QTimer>
#include <QVBoxLayout>

const int MaxButtons = 4;

class DeskphoneContextLabelPrivate
{
public:
    DeskphoneContextLabelPrivate() : buttons(0), buttonCount(0), pressedBtn(-1),
        launcherPressedBtn(-1), activeButton(-1), loadedTheme(false),
        themeInit(false), launcherMode(true) {
    }
    struct Button {
        int key;
        QThemeItem *grpItem;
        QThemeImageItem *imgItem;
        QThemeTextItem *txtItem;
        bool changed;
    };

    int buttonForItem(QThemeItem *item) const {
        int pressed = -1;
        for( int i = 0 ; i < buttonCount ; ++i )
        {
            if( buttons[i].grpItem == item )
            {
                pressed = i;
                break;
            }
            if( buttons[i].imgItem == item )
            {
                pressed = i;
                break;
            }
            if( buttons[i].txtItem == item )
            {
                pressed = i;
                break;
            }
        }

        return pressed;
    }

    QThemedView *phoneThemedView;
    Button *buttons;
    int buttonCount;
    int pressedBtn;
    int launcherPressedBtn;
    int activeButton;
    bool loadedTheme;
    bool themeInit;
    bool launcherMode;
    QSoftMenuBarProvider *menuProvider;
};

/*!
    \class DeskphoneContextLabel
    \inpublicgroup QtUiModule
    \ingroup QtopiaServer::PhoneUI
    \brief The DeskphoneContextLabel class provides a themable dockable soft key bar for Qt Extended Home.

    An image of this context label can be found in the \l{Server Widget Classes}{server widget gallery}.
    This widget can be themed which can change its appearance significantly.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    Creates a new DeskphoneContextLabel instance with the given \a parent
    and widget \a flags.
*/
DeskphoneContextLabel::DeskphoneContextLabel(QWidget *parent, Qt::WFlags flags)
    : QAbstractContextLabel(parent, flags)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

    d = new DeskphoneContextLabelPrivate;
    d->phoneThemedView = new QThemedView();

    QVBoxLayout *vb = new QVBoxLayout();
    vb->addWidget(d->phoneThemedView);
    vb->setContentsMargins(0, 0, 0, 0);
    setLayout(vb);
    setWindowTitle("_decoration_");

    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
    pal.setBrush(QPalette::Base, QBrush(QColor(0,0,0,0)));
    setPalette(pal);

    d->menuProvider = new QSoftMenuBarProvider(this); 
    QObject::connect(d->menuProvider, SIGNAL(keyChanged(QSoftMenuBarProvider::MenuButton)), this, SLOT(keyChanged(QSoftMenuBarProvider::MenuButton)));
    d->buttonCount = d->menuProvider->keyCount();
    qLog(UI) << "DeskphoneContextLabel: generating" << d->buttonCount <<  "buttons";
    if(d->menuProvider->keyCount()) {
        d->buttons = new DeskphoneContextLabelPrivate::Button [d->buttonCount];
        for(int ii = 0; ii < d->menuProvider->keyCount(); ++ii) {
            d->buttons[ii].key = d->menuProvider->key(ii).key();
            qLog(UI) << "Add contextbutton:" << d->buttons[ii].key;
            d->buttons[ii].grpItem = 0;
            d->buttons[ii].imgItem = 0;
            d->buttons[ii].txtItem = 0;
            d->buttons[ii].changed = false;
        }
    }

    connect(d->phoneThemedView, SIGNAL(itemPressed(QThemeItem*)),
            this, SLOT(itemPressed(QThemeItem*)));
    connect(d->phoneThemedView, SIGNAL(itemReleased(QThemeItem*)),
            this, SLOT(itemReleased(QThemeItem*)));

    QtopiaInputEvents::addKeyboardFilter(this);

    ThemeControl *ctrl = qtopiaTask<ThemeControl>();
    if ( ctrl ) {
        ctrl->registerThemedView(d->phoneThemedView, "DeskContext");
        themeLoaded();
    } else  {
        qLog(Component) << "DeskphoneContextLabel: ThemeControl not available, Theme will not work properly";
    }
    setFixedSize(reservedSize());
}

/*!
    \internal
*/
DeskphoneContextLabel::~DeskphoneContextLabel()
{
    delete [] d->buttons;
    delete d->phoneThemedView;
    delete d;
}

/*!
    \reimp
*/
QSize DeskphoneContextLabel::reservedSize() const
{
     QRect rect = qApp->desktop()->screenGeometry();
#ifdef QTOPIA_HOMEUI_WIDE
    double percentage = 0.09;
    rect.setWidth(qRound(percentage * rect.width()));
#else
    double percentage = 0.20;
    rect.setHeight(qRound(percentage * rect.height()));
#endif
    d->phoneThemedView->setFixedSize(rect.size());
    return rect.size();
}

/*! \internal */
void DeskphoneContextLabel::themeLoaded()
{
    d->loadedTheme = true;
    d->themeInit = false;
    QTimer::singleShot(0, this, SLOT(initializeButtons()));
}

void DeskphoneContextLabel::initializeButtons()
{
    if (d->loadedTheme && !d->themeInit) {
        int availBtns = 0;
        int maxbuttons = d->buttonCount >= MaxButtons ? MaxButtons : d->buttonCount;
        QThemeItem *grp[MaxButtons] = { NULL, NULL, NULL, NULL };
        QThemeImageItem *img[MaxButtons] = { NULL, NULL, NULL, NULL };
        QThemeTextItem *txt[MaxButtons] = { NULL, NULL, NULL, NULL };

        if (d->buttonCount) {
            for (int i = 0; i < maxbuttons; i++) {
                d->buttons[i].grpItem = 0;
                d->buttons[i].imgItem = 0;
                d->buttons[i].txtItem = 0;
                d->buttons[i].changed = true;

                QThemeItem *gi = d->phoneThemedView->findItem("button"+QString::number(i));
                QThemeImageItem *ii = (QThemeImageItem *)
                    d->phoneThemedView->findItem("button"+QString::number(i));
                QThemeTextItem *ti = (QThemeTextItem *)
                    d->phoneThemedView->findItem("button"+QString::number(i));

                if (ii || ti) {
                    grp[availBtns] = gi;
                    img[availBtns] = ii;
                    txt[availBtns] = ti;
                    availBtns++;
                }
            }
        }

        if (d->buttonCount) {
            d->buttons[0].grpItem = grp[0];
            d->buttons[0].imgItem = img[0];
            d->buttons[0].txtItem = txt[0];
        }

        if (availBtns == d->buttonCount) {
            for (int i = 1; i < availBtns; i++) {
                d->buttons[i].grpItem = grp[i];
                d->buttons[i].imgItem = img[i];
                d->buttons[i].txtItem = txt[i];
            }
        } else if (availBtns < d->buttonCount) {
            if (availBtns == 2) {
                d->buttons[2].grpItem = grp[1];
                d->buttons[2].imgItem = img[1];
                d->buttons[2].txtItem = txt[1];
            }
        } else {
            if (d->buttonCount == 2) {
                d->buttons[1].grpItem = grp[2];
                d->buttons[1].imgItem = img[2];
                d->buttons[1].txtItem = txt[2];
            }
        }
        d->themeInit = true;

        updateLabels();
    }
}

void DeskphoneContextLabel::itemPressed(QThemeItem *item)
{
    initializeButtons();
    if (item->name() == "menu") {
        QtopiaInputEvents::processKeyEvent(0xffff, QSoftMenuBar::menuKey(), 0, true, false);
    } else if (d->launcherMode) {
        d->launcherPressedBtn = d->buttonForItem(item);
        d->pressedBtn = -1;
    } else {
        d->launcherPressedBtn = -1;
        d->pressedBtn = d->buttonForItem(item);
        if (d->pressedBtn >= 0) {
            int keycode = d->buttons[d->pressedBtn].key;
            QtopiaInputEvents::processKeyEvent(0xffff, keycode, 0, true, false);
        }
    }
}

void DeskphoneContextLabel::itemReleased(QThemeItem *item)
{
    initializeButtons();
    if (item->name() == "menu") {
        QtopiaInputEvents::processKeyEvent(0xffff, QSoftMenuBar::menuKey(), 0, false, false);
    } else if (!d->launcherMode) {
        if (d->pressedBtn >= 0) {
            int keycode = d->buttons[d->pressedBtn].key;
            QtopiaInputEvents::processKeyEvent(0xffff, keycode, 0, false, false);
            d->pressedBtn = -1;
        }
    }
}

/*!
    \internal
*/
bool DeskphoneContextLabel::filter(int unicode, int keycode, int modifiers, bool press,
                                bool autoRepeat)
{
    Q_UNUSED(unicode);
    Q_UNUSED(keycode);
    Q_UNUSED(modifiers);
    Q_UNUSED(press);
    Q_UNUSED(autoRepeat);
    return false;
}

/*! \internal */
void DeskphoneContextLabel::keyChanged(const QSoftMenuBarProvider::MenuButton &button)
{
    Q_ASSERT(button.index() < d->buttonCount);
    initializeButtons();
    d->buttons[button.index()].changed = true;
    updateLabels();
}

/*! \internal */
void DeskphoneContextLabel::updateLabels()
{
    initializeButtons();
    bool oldLauncherMode = d->launcherMode;
    /*
    if (d->menuProvider->key(0).text() == QLatin1String("Launcher")) {
        d->launcherMode = true;
        ThemeImageItem *ii = (ThemeImageItem *)d->phoneThemedView->findItem("launcher");
        if (ii)
            ii->setActive(true);
    } else {
        d->launcherMode = false;
        ThemeImageItem *ii = (ThemeImageItem *)d->phoneThemedView->findItem("launcher");
        if (ii)
            ii->setActive(false);
    }
    */
    if (d->launcherMode) {
    } else {
        for (int idx = 0; idx < d->buttonCount; idx++) {
            if (d->buttons[idx].changed || oldLauncherMode != d->launcherMode) {
                if (d->buttons[idx].grpItem) {
                    d->buttons[idx].grpItem->setActive(
                        !d->menuProvider->key(idx).text().isEmpty()
                        || !d->menuProvider->key(idx).pixmap().isNull());
                }
                if (d->buttons[idx].txtItem)
                    d->buttons[idx].txtItem->setText(d->menuProvider->key(idx).text());
                if (d->buttons[idx].imgItem)
                    d->buttons[idx].imgItem->setImage(d->menuProvider->key(idx).pixmap());
            }

            d->buttons[idx].changed = false;
        }
    }
}


QTOPIA_REPLACE_WIDGET(QAbstractContextLabel,DeskphoneContextLabel);

#include "deskcontextlabel.moc"
