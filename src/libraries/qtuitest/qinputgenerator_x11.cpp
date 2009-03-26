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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qinputgenerator_p.h"

#include <QtCore>
#include <QtGui>
#include <QX11Info>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

#include "qtuitestnamespace.h"

#define QINPUTGENERATOR_DEBUG() if (1); else qDebug() << "QInputGenerator:"

QMap<int,int> qt_key_to_keysym_make()
{
    QMap<int,int> m;

#define QT_K(Qt,X) m.insert(Qt,X)
    QT_K( Qt::Key_Escape,     XK_Escape );
    QT_K( Qt::Key_Tab,        XK_Tab );
    QT_K( Qt::Key_Backtab,    XK_ISO_Left_Tab );
    QT_K( Qt::Key_Backspace,  XK_BackSpace );
    QT_K( Qt::Key_Return,     XK_Return );
    QT_K( Qt::Key_Enter,      XK_KP_Enter );
    QT_K( Qt::Key_Insert,     XK_Insert );
    QT_K( Qt::Key_Delete,     XK_Delete );
    QT_K( Qt::Key_Pause,      XK_Pause );
    QT_K( Qt::Key_Print,      XK_Print );
    QT_K( Qt::Key_SysReq,     XK_Sys_Req );
    QT_K( Qt::Key_Home,       XK_Home );
    QT_K( Qt::Key_End,        XK_End );
    QT_K( Qt::Key_Left,       XK_Left );
    QT_K( Qt::Key_Up,         XK_Up );
    QT_K( Qt::Key_Right,      XK_Right );
    QT_K( Qt::Key_Down,       XK_Down );
    QT_K( Qt::Key_CapsLock,   XK_Caps_Lock );
    QT_K( Qt::Key_NumLock,    XK_Num_Lock );
    QT_K( Qt::Key_ScrollLock, XK_Scroll_Lock );
    QT_K( Qt::Key_F1,         XK_F1 );
    QT_K( Qt::Key_F2,         XK_F2 );
    QT_K( Qt::Key_F3,         XK_F3 );
    QT_K( Qt::Key_F4,         XK_F4 );
    QT_K( Qt::Key_F5,         XK_F5 );
    QT_K( Qt::Key_F6,         XK_F6 );
    QT_K( Qt::Key_F7,         XK_F7 );
    QT_K( Qt::Key_F8,         XK_F8 );
    QT_K( Qt::Key_F9,         XK_F9 );
    QT_K( Qt::Key_F10,        XK_F10 );
    QT_K( Qt::Key_F11,        XK_F11 );
    QT_K( Qt::Key_F12,        XK_F12 );
    QT_K( Qt::Key_F13,        XK_F13 );
    QT_K( Qt::Key_F14,        XK_F14 );
    QT_K( Qt::Key_F15,        XK_F15 );
    QT_K( Qt::Key_F16,        XK_F16 );
    QT_K( Qt::Key_F17,        XK_F17 );
    QT_K( Qt::Key_F18,        XK_F18 );
    QT_K( Qt::Key_F19,        XK_F19 );
    QT_K( Qt::Key_F20,        XK_F20 );
    QT_K( Qt::Key_F21,        XK_F21 );
    QT_K( Qt::Key_F22,        XK_F22 );
    QT_K( Qt::Key_F23,        XK_F23 );
    QT_K( Qt::Key_F24,        XK_F24 );
    QT_K( Qt::Key_F25,        XK_F25 );
    QT_K( Qt::Key_F26,        XK_F26 );
    QT_K( Qt::Key_F27,        XK_F27 );
    QT_K( Qt::Key_F28,        XK_F28 );
    QT_K( Qt::Key_F29,        XK_F29 );
    QT_K( Qt::Key_F30,        XK_F30 );
    QT_K( Qt::Key_F31,        XK_F31 );
    QT_K( Qt::Key_F32,        XK_F32 );
    QT_K( Qt::Key_F33,        XK_F33 );
    QT_K( Qt::Key_F34,        XK_F34 );
    QT_K( Qt::Key_F35,        XK_F35 );
    QT_K( Qt::Key_Super_L,    XK_Super_L );
    QT_K( Qt::Key_Super_R,    XK_Super_R );
    QT_K( Qt::Key_Menu,       XK_Menu );
    QT_K( Qt::Key_Hyper_L,    XK_Hyper_L );
    QT_K( Qt::Key_Hyper_R,    XK_Hyper_R );
    QT_K( Qt::Key_Help,       XK_Help );
    QT_K( '/',                XK_KP_Divide );
    QT_K( '*',                XK_KP_Multiply );
    QT_K( '-',                XK_KP_Subtract );
    QT_K( '+',                XK_KP_Add );
    QT_K( Qt::Key_Return,     XK_KP_Enter );

    // Modifiers
    QT_K( Qt::ShiftModifier,  XK_Shift_L );
    QT_K( Qt::ControlModifier,XK_Control_L );
    QT_K( Qt::AltModifier,    XK_Alt_L );
    QT_K( Qt::MetaModifier,   XK_Meta_L );
    // FIXME support Qt::KeypadModifier

#undef QT_K

    return m;
}

QMap<int,uint> qt_button_to_x_button_make()
{
    QMap<int,uint> m;

    m.insert(Qt::LeftButton,  1);
    m.insert(Qt::MidButton,   2);
    m.insert(Qt::RightButton, 3);

    return m;
}

QMap<int,int> qt_modifier_to_x_modmask_make()
{
    QMap<int,int> m;

    m.insert( Qt::ShiftModifier,   ShiftMask );
    m.insert( Qt::ControlModifier, ControlMask );
    m.insert( Qt::AltModifier,     Mod1Mask );
    m.insert( Qt::MetaModifier,    Mod4Mask );

    return m;
}

struct QInputGeneratorPrivate
{
    QInputGeneratorPrivate();
    QInputGenerator* q;

    void keyEvent(Qt::Key, bool);
    void mouseEvent(QPoint const&, Qt::MouseButtons);

    Qt::KeyboardModifiers currentModifiers() const;
    void ensureModifiers(Qt::KeyboardModifiers);

    QMap<int,int> const keymap;

    QPoint              currentPos;
    Qt::MouseButtons    currentButtons;
};

QInputGeneratorPrivate::QInputGeneratorPrivate()
    :   keymap(qt_key_to_keysym_make()),
        currentPos(),
        currentButtons(0)
{}

QInputGenerator::QInputGenerator(QObject* parent)
    : QObject(parent),
      d(new QInputGeneratorPrivate)
{
    d->q = this;
    QINPUTGENERATOR_DEBUG() << "constructor";
}

QInputGenerator::~QInputGenerator()
{
    QINPUTGENERATOR_DEBUG() << "destructor";

    /*
        Restore all keyboard modifiers to off.
        If we don't do this, the current modifiers stay activated for the current X server
        even when this application is closed.
        Note that there is no guarantee this code actually gets run.
    */
    d->ensureModifiers(0);

    d->q = 0;
    delete d;
    d = 0;
}

static Qt::KeyboardModifier const qt_AllModifiers[] =
    { Qt::ShiftModifier, Qt::ControlModifier, Qt::AltModifier, Qt::MetaModifier };

/*
    Returns the Qt keyboard modifiers which are currently pressed.
*/
Qt::KeyboardModifiers QInputGeneratorPrivate::currentModifiers() const
{
    Window root  = 0;
    Window child = 0;
    int root_x   = 0;
    int root_y   = 0;
    int win_x    = 0;
    int win_y    = 0;
    uint buttons = 0;
    Display* dpy = QX11Info::display();

    // Grab all of the pointer info, though all we really care about is the modifiers.
    bool ok = false;
    for (int i = 0; i < ScreenCount(dpy) && !ok; ++i) {
        if (XQueryPointer(dpy, QX11Info::appRootWindow(i), &root, &child, &root_x, &root_y,
                          &win_x, &win_y, &buttons))
            ok = true;
    }

    if (!ok) {
        qWarning() <<
            "QInputGenerator: could not determine current state of keyboard modifiers. "
            "Simulated key events may be incorrect.";
        return 0;
    }

    // Convert to Qt::KeyboardModifiers.
    static const QMap<int,int> modmap = qt_modifier_to_x_modmask_make();

    Qt::KeyboardModifiers ret(0);
    for (unsigned i = 0; i < sizeof(qt_AllModifiers)/sizeof(Qt::KeyboardModifier); ++i) {
        Qt::KeyboardModifier thisMod = qt_AllModifiers[i];
        int mask = modmap.value(thisMod);
        if (buttons & mask) {
            QINPUTGENERATOR_DEBUG() << "mod enabled:" << (void*)(int)thisMod << (void*)mask;
            ret |= thisMod;
        }
    }

    QINPUTGENERATOR_DEBUG() << "current modifiers:" << (void*)buttons << (void*)(int)ret;

    return ret;
}

void QInputGeneratorPrivate::ensureModifiers(Qt::KeyboardModifiers desiredMod)
{
    Qt::KeyboardModifiers currentMod = currentModifiers();

    // For each modifier..
    for (unsigned i = 0; i < sizeof(qt_AllModifiers)/sizeof(Qt::KeyboardModifier); ++i) {
        Qt::KeyboardModifier thisMod = qt_AllModifiers[i];
        // If the modifier is currently disabled but we want it enabled, or vice-versa...
        if ((desiredMod & thisMod) && !(currentMod & thisMod)) {
            QINPUTGENERATOR_DEBUG() << "Enabling modifier" << (void*)thisMod << "by press";
            // press to enable
            keyEvent(static_cast<Qt::Key>(thisMod), true);
        } else if (!(desiredMod & thisMod) && (currentMod & thisMod)) {
            QINPUTGENERATOR_DEBUG() << "Disabling modifier" << (void*)thisMod << "by release";
            // release to disable
            keyEvent(static_cast<Qt::Key>(thisMod), false);
        }
    }

    if (currentMod != desiredMod) {
        currentMod = desiredMod;
        for (int i = 0;
                i < 1000 && QApplication::keyboardModifiers() != desiredMod;
                i += 50, QtUiTest::wait(50))
        {}

        if (QApplication::keyboardModifiers() != desiredMod)
            qWarning() << "QtUitest: couldn't set all modifiers to desired state! "
                "Current state:" << (void*)(int)QApplication::keyboardModifiers() <<
                "Desired:"       << (void*)(int)desiredMod;
    }

}

void QInputGeneratorPrivate::keyEvent(Qt::Key key, bool is_press)
{
    KeySym sym = 0;

    do {
        if (key < 0x1000) {
            sym = QChar(key).toLower().unicode();
            break;
        }

        QMap<int,int>::const_iterator found = keymap.find(key);
        if (found != keymap.end()) {
            sym = *found;
            break;
        }

        qWarning() << "QInputGenerator: don't know how to translate Qt key"
                   << (void*)key << "into X11 keysym";
        return;

    } while(0);

    QINPUTGENERATOR_DEBUG() << (is_press ? "press" : "release") << XKeysymToString(sym);

    XTestFakeKeyEvent(
        QX11Info::display(),
        XKeysymToKeycode(QX11Info::display(), sym),
        is_press,
        0
    );
}

void QInputGenerator::keyPress(Qt::Key key, Qt::KeyboardModifiers mod, bool autoRepeat)
{
    Q_UNUSED(autoRepeat);
    d->ensureModifiers(mod);
    d->keyEvent(key, true);
}

void QInputGenerator::keyRelease(Qt::Key key, Qt::KeyboardModifiers mod)
{
    d->ensureModifiers(mod);
    d->keyEvent(key, false);
}

void QInputGenerator::keyClick(Qt::Key key, Qt::KeyboardModifiers mod)
{
    keyPress(key,mod);
    keyRelease(key,mod);
}

void QInputGeneratorPrivate::mouseEvent(QPoint const& local, Qt::MouseButtons state)
{
    static QMap<int,uint> const buttonmap = qt_button_to_x_button_make();

    QPoint pos(q->mapFromActiveWindow(local));

    if (currentPos != pos) {
        currentPos = pos;

        XTestFakeMotionEvent(
            QX11Info::display(),
            QX11Info::appScreen(),
            pos.x(),
            pos.y(),
            0
        );

        for (int i = 0;
                i < 1000 && QCursor::pos() != pos;
                i += 50, QtUiTest::wait(50))
        {}

        if (QCursor::pos() != pos)
            qWarning() << "QtUitest: couldn't move cursor to desired point! "
                "Current position:" << QCursor::pos() <<
                "Desired:"          << pos;

    }

    typedef QPair<uint,bool> ButtonEvent;
    QList<ButtonEvent> buttonEvents;

    foreach (int button, buttonmap.keys()) {
        bool desired = button & state;
        bool current = button & currentButtons;

        // Do we need to press?
        if (desired && !current) {
            buttonEvents << qMakePair(buttonmap[button], true);
        }

        // Do we need to release?
        if (!desired && current) {
            buttonEvents << qMakePair(buttonmap[button], false);
        }
    }

    foreach (ButtonEvent const& event, buttonEvents) {
        QINPUTGENERATOR_DEBUG() << "Button event at" << pos << ":" << event.first << event.second;
        XTestFakeButtonEvent(
            QX11Info::display(),
            event.first,
            event.second,
            0
        );
    }

    currentButtons = state;
}

void QInputGenerator::mousePress(QPoint const& pos, Qt::MouseButtons state)
{
    QINPUTGENERATOR_DEBUG() << "Mouse press" << pos << (void*)(int)state;
    d->mouseEvent(pos, state);
}

void QInputGenerator::mouseRelease(QPoint const& pos, Qt::MouseButtons state)
{
    QINPUTGENERATOR_DEBUG() << "Mouse release" << pos << (void*)(int)(d->currentButtons & ~state);
    d->mouseEvent(pos, d->currentButtons & ~state);
}

void QInputGenerator::mouseClick(QPoint const& pos, Qt::MouseButtons state)
{
    mousePress  (pos,state);
    mouseRelease(pos,state);
}

