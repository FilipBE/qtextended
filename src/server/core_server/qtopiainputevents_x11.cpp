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

#include "qtopiainputevents.h"

#ifdef Q_WS_X11

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>

/* Special keys used by Qtopia, mapped into the X11 private keypad range */
#define QTOPIAXK_Select		0x11000601
#define QTOPIAXK_Yes		0x11000602
#define QTOPIAXK_No		0x11000603

#define QTOPIAXK_Cancel		0x11000604
#define QTOPIAXK_Printer	0x11000605
#define QTOPIAXK_Execute	0x11000606
#define QTOPIAXK_Sleep		0x11000607
#define QTOPIAXK_Play		0x11000608
#define QTOPIAXK_Zoom		0x11000609

#define QTOPIAXK_Context1	0x1100060A
#define QTOPIAXK_Context2	0x1100060B
#define QTOPIAXK_Context3	0x1100060C
#define QTOPIAXK_Context4	0x1100060D
#define QTOPIAXK_Call		0x1100060E
#define QTOPIAXK_Hangup		0x1100060F
#define QTOPIAXK_Flip		0x11000610

static int prevModifiers = 0;
static bool hasTestExtension = false;
static bool initialized = false;
static KeySym shiftKeycode = 0;
static KeySym modeSwitchKeycode = 0;

void QtopiaInputEvents::sendKeyEvent
    (int /*unicode*/, int keycode, Qt::KeyboardModifiers modifiers,
     bool isPress, bool /*autoRepeat*/)
{
    Display *dpy = QX11Info::display();
    if (!dpy)
        return;

    // Initialize the XTEST extension if necessary.
    if (!initialized) {
        int event_base, error_base, major, minor;
        initialized = true;
        if (!XTestQueryExtension
                (dpy, &event_base, &error_base, &major, &minor)) {
            qWarning("X server does not support the XTEST extension; "
                     "cannot emulate key events");
            return;
        }
        hasTestExtension = true;
        shiftKeycode = XKeysymToKeycode(dpy, XK_Shift_L);
        if (shiftKeycode == NoSymbol)
            shiftKeycode = XKeysymToKeycode(dpy, XK_Shift_R);
        modeSwitchKeycode = XKeysymToKeycode(dpy, XK_Mode_switch);
    } else if (!hasTestExtension) {
        return;
    }

    // Convert the Qt key code into an X keysym.
    KeySym keysym = NoSymbol;
    switch (keycode) {
        case Qt::Key_Escape: keysym = XK_Escape; break;
        case Qt::Key_Tab: keysym = XK_Tab; break;
        case Qt::Key_Backtab: keysym = XK_ISO_Left_Tab; break;
        case Qt::Key_Backspace: keysym = XK_BackSpace; break;
        case Qt::Key_Return: keysym = XK_Return; break;
        case Qt::Key_Enter: keysym = XK_KP_Enter; break;
        case Qt::Key_Insert: keysym = XK_KP_Insert; break;
        case Qt::Key_Delete: keysym = XK_KP_Delete; break;
        case Qt::Key_Pause: keysym = XK_Pause; break;
        case Qt::Key_Print: keysym = XK_Print; break;
        case Qt::Key_SysReq: keysym = 0x1005FF60; break;
        case Qt::Key_Clear: keysym = XK_KP_Begin; break;
        case Qt::Key_Home: keysym = XK_KP_Home; break;
        case Qt::Key_End: keysym = XK_KP_End; break;
        case Qt::Key_Left: keysym = XK_KP_Left; break;
        case Qt::Key_Up: keysym = XK_KP_Up; break;
        case Qt::Key_Right: keysym = XK_KP_Right; break;
        case Qt::Key_Down: keysym = XK_KP_Down; break;
        case Qt::Key_PageUp: keysym = XK_KP_Prior; break;
        case Qt::Key_PageDown: keysym = XK_KP_Next; break;
        case Qt::Key_Shift: keysym = XK_Shift_L; break;
        case Qt::Key_Control: keysym = XK_Control_L; break;
        case Qt::Key_Meta: keysym = XK_Meta_L; break;
        case Qt::Key_Alt: keysym = XK_Alt_L; break;
        case Qt::Key_CapsLock: keysym = XK_Caps_Lock; break;
        case Qt::Key_NumLock: keysym = XK_Num_Lock; break;
        case Qt::Key_ScrollLock: keysym = XK_Scroll_Lock; break;
        case Qt::Key_F1: keysym = XK_F1; break;
        case Qt::Key_F2: keysym = XK_F2; break;
        case Qt::Key_F3: keysym = XK_F3; break;
        case Qt::Key_F4: keysym = XK_F4; break;
        case Qt::Key_F5: keysym = XK_F5; break;
        case Qt::Key_F6: keysym = XK_F6; break;
        case Qt::Key_F7: keysym = XK_F7; break;
        case Qt::Key_F8: keysym = XK_F8; break;
        case Qt::Key_F9: keysym = XK_F9; break;
        case Qt::Key_F10: keysym = XK_F10; break;
        case Qt::Key_F11: keysym = XK_F11; break;
        case Qt::Key_F12: keysym = XK_F12; break;
        case Qt::Key_F13: keysym = XK_F13; break;
        case Qt::Key_F14: keysym = XK_F14; break;
        case Qt::Key_F15: keysym = XK_F15; break;
        case Qt::Key_F16: keysym = XK_F16; break;
        case Qt::Key_F17: keysym = XK_F17; break;
        case Qt::Key_F18: keysym = XK_F18; break;
        case Qt::Key_F19: keysym = XK_F19; break;
        case Qt::Key_F20: keysym = XK_F20; break;
        case Qt::Key_F21: keysym = XK_F21; break;
        case Qt::Key_F22: keysym = XK_F22; break;
        case Qt::Key_F23: keysym = XK_F23; break;
        case Qt::Key_F24: keysym = XK_F24; break;
        case Qt::Key_F25: keysym = XK_F25; break;
        case Qt::Key_F26: keysym = XK_F26; break;
        case Qt::Key_F27: keysym = XK_F27; break;
        case Qt::Key_F28: keysym = XK_F28; break;
        case Qt::Key_F29: keysym = XK_F29; break;
        case Qt::Key_F30: keysym = XK_F30; break;
        case Qt::Key_F31: keysym = XK_F31; break;
        case Qt::Key_F32: keysym = XK_F32; break;
        case Qt::Key_F33: keysym = XK_F33; break;
        case Qt::Key_F34: keysym = XK_F34; break;
        case Qt::Key_F35: keysym = XK_F35; break;
        case Qt::Key_Super_L: keysym = XK_Super_L; break;
        case Qt::Key_Super_R: keysym = XK_Super_R; break;
        case Qt::Key_Menu: keysym = XK_Menu; break;
        case Qt::Key_Hyper_L: keysym = XK_Hyper_L; break;
        case Qt::Key_Hyper_R: keysym = XK_Hyper_R; break;
        case Qt::Key_Help: keysym = XK_Help; break;
        case Qt::Key_Direction_L: keysym = NoSymbol; break; // ???
        case Qt::Key_Direction_R: keysym = NoSymbol; break; // ???
        case Qt::Key_Space: keysym = XK_space; break;
        case Qt::Key_Exclam: keysym = XK_exclam; break;
        case Qt::Key_QuoteDbl: keysym = XK_quotedbl; break;
        case Qt::Key_NumberSign: keysym = XK_numbersign; break;
        case Qt::Key_Dollar: keysym = XK_dollar; break;
        case Qt::Key_Percent: keysym = XK_percent; break;
        case Qt::Key_Ampersand: keysym = XK_ampersand; break;
        case Qt::Key_Apostrophe: keysym = XK_apostrophe; break;
        case Qt::Key_ParenLeft: keysym = XK_parenleft; break;
        case Qt::Key_ParenRight: keysym = XK_parenright; break;
        case Qt::Key_Asterisk: keysym = XK_asterisk; break;
        case Qt::Key_Plus: keysym = XK_plus; break;
        case Qt::Key_Comma: keysym = XK_comma; break;
        case Qt::Key_Minus: keysym = XK_minus; break;
        case Qt::Key_Period: keysym = XK_period; break;
        case Qt::Key_Slash: keysym = XK_slash; break;
        case Qt::Key_0: keysym = XK_0; break;
        case Qt::Key_1: keysym = XK_1; break;
        case Qt::Key_2: keysym = XK_2; break;
        case Qt::Key_3: keysym = XK_3; break;
        case Qt::Key_4: keysym = XK_4; break;
        case Qt::Key_5: keysym = XK_5; break;
        case Qt::Key_6: keysym = XK_6; break;
        case Qt::Key_7: keysym = XK_7; break;
        case Qt::Key_8: keysym = XK_8; break;
        case Qt::Key_9: keysym = XK_9; break;
        case Qt::Key_Colon: keysym = XK_colon; break;
        case Qt::Key_Semicolon: keysym = XK_semicolon; break;
        case Qt::Key_Less: keysym = XK_less; break;
        case Qt::Key_Equal: keysym = XK_equal; break;
        case Qt::Key_Greater: keysym = XK_greater; break;
        case Qt::Key_Question: keysym = XK_question; break;
        case Qt::Key_At: keysym = XK_at; break;
        case Qt::Key_A: keysym = XK_a; break; // Must be lower case keysyms
        case Qt::Key_B: keysym = XK_b; break; // for correct shift handling.
        case Qt::Key_C: keysym = XK_c; break;
        case Qt::Key_D: keysym = XK_d; break;
        case Qt::Key_E: keysym = XK_e; break;
        case Qt::Key_F: keysym = XK_f; break;
        case Qt::Key_G: keysym = XK_g; break;
        case Qt::Key_H: keysym = XK_h; break;
        case Qt::Key_I: keysym = XK_i; break;
        case Qt::Key_J: keysym = XK_j; break;
        case Qt::Key_K: keysym = XK_k; break;
        case Qt::Key_L: keysym = XK_l; break;
        case Qt::Key_M: keysym = XK_m; break;
        case Qt::Key_N: keysym = XK_n; break;
        case Qt::Key_O: keysym = XK_o; break;
        case Qt::Key_P: keysym = XK_p; break;
        case Qt::Key_Q: keysym = XK_q; break;
        case Qt::Key_R: keysym = XK_r; break;
        case Qt::Key_S: keysym = XK_s; break;
        case Qt::Key_T: keysym = XK_t; break;
        case Qt::Key_U: keysym = XK_u; break;
        case Qt::Key_V: keysym = XK_v; break;
        case Qt::Key_W: keysym = XK_w; break;
        case Qt::Key_X: keysym = XK_x; break;
        case Qt::Key_Y: keysym = XK_y; break;
        case Qt::Key_Z: keysym = XK_z; break;
        case Qt::Key_BracketLeft: keysym = XK_bracketleft; break;
        case Qt::Key_Backslash: keysym = XK_backslash; break;
        case Qt::Key_BracketRight: keysym = XK_bracketright; break;
        case Qt::Key_AsciiCircum: keysym = XK_asciicircum; break;
        case Qt::Key_Underscore: keysym = XK_underscore; break;
        case Qt::Key_QuoteLeft: keysym = XK_quoteleft; break;
        case Qt::Key_BraceLeft: keysym = XK_braceleft; break;
        case Qt::Key_Bar: keysym = XK_bar; break;
        case Qt::Key_BraceRight: keysym = XK_braceright; break;
        case Qt::Key_AsciiTilde: keysym = XK_asciitilde; break;

        case Qt::Key_AltGr: keysym = XK_ISO_Level3_Shift; break;
        case Qt::Key_Mode_switch: keysym = XK_Mode_switch; break;

        case Qt::Key_Back: keysym = XF86XK_Back; break;
        case Qt::Key_Forward: keysym = XF86XK_Forward; break;
        case Qt::Key_Stop: keysym = XF86XK_Stop; break;
        case Qt::Key_Refresh: keysym = XF86XK_Refresh; break;

        case Qt::Key_VolumeDown: keysym = XF86XK_AudioLowerVolume; break;
        case Qt::Key_VolumeMute: keysym = XF86XK_AudioMute; break;
        case Qt::Key_VolumeUp: keysym = XF86XK_AudioRaiseVolume; break;

        case Qt::Key_MediaPlay: keysym = XF86XK_AudioPlay; break;
        case Qt::Key_MediaStop: keysym = XF86XK_AudioStop; break;
        case Qt::Key_MediaPrevious: keysym = XF86XK_AudioPrev; break;
        case Qt::Key_MediaNext: keysym = XF86XK_AudioNext; break;
        case Qt::Key_MediaRecord: keysym = XF86XK_AudioRecord; break;

        case Qt::Key_HomePage: keysym = XF86XK_HomePage; break;
        case Qt::Key_Favorites: keysym = XF86XK_Favorites; break;
        case Qt::Key_Search: keysym = XF86XK_Search; break;
        case Qt::Key_Standby: keysym = XF86XK_Standby; break;
        case Qt::Key_OpenUrl: keysym = XF86XK_OpenURL; break;

        case Qt::Key_LaunchMail: keysym = XF86XK_Mail; break;
        case Qt::Key_LaunchMedia: keysym = XF86XK_AudioMedia; break;
        case Qt::Key_Launch0: keysym = XF86XK_Launch0; break;
        case Qt::Key_Launch1: keysym = XF86XK_Launch1; break;
        case Qt::Key_Launch2: keysym = XF86XK_Launch2; break;
        case Qt::Key_Launch3: keysym = XF86XK_Launch3; break;
        case Qt::Key_Launch4: keysym = XF86XK_Launch4; break;
        case Qt::Key_Launch5: keysym = XF86XK_Launch5; break;
        case Qt::Key_Launch6: keysym = XF86XK_Launch6; break;
        case Qt::Key_Launch7: keysym = XF86XK_Launch7; break;
        case Qt::Key_Launch8: keysym = XF86XK_Launch8; break;
        case Qt::Key_Launch9: keysym = XF86XK_Launch9; break;
        case Qt::Key_LaunchA: keysym = XF86XK_LaunchA; break;
        case Qt::Key_LaunchB: keysym = XF86XK_LaunchB; break;
        case Qt::Key_LaunchC: keysym = XF86XK_LaunchC; break;
        case Qt::Key_LaunchD: keysym = XF86XK_LaunchD; break;
        case Qt::Key_LaunchE: keysym = XF86XK_LaunchE; break;
        case Qt::Key_LaunchF: keysym = XF86XK_LaunchF; break;

        case Qt::Key_MediaLast: keysym = NoSymbol; break;   // ???

        case Qt::Key_Select: keysym = QTOPIAXK_Select; break;
        case Qt::Key_Yes: keysym = QTOPIAXK_Yes; break;
        case Qt::Key_No: keysym = QTOPIAXK_No; break;

        case Qt::Key_Cancel: keysym = QTOPIAXK_Cancel; break;
        case Qt::Key_Printer: keysym = QTOPIAXK_Printer; break;
        case Qt::Key_Execute: keysym = QTOPIAXK_Execute; break;
        case Qt::Key_Sleep: keysym = QTOPIAXK_Sleep; break;
        case Qt::Key_Play: keysym = QTOPIAXK_Play; break;
        case Qt::Key_Zoom: keysym = QTOPIAXK_Zoom; break;

        case Qt::Key_Context1: keysym = QTOPIAXK_Context1; break;
        case Qt::Key_Context2: keysym = QTOPIAXK_Context2; break;
        case Qt::Key_Context3: keysym = QTOPIAXK_Context3; break;
        case Qt::Key_Context4: keysym = QTOPIAXK_Context4; break;
        case Qt::Key_Call: keysym = QTOPIAXK_Call; break;
        case Qt::Key_Hangup: keysym = QTOPIAXK_Hangup; break;
        case Qt::Key_Flip: keysym = QTOPIAXK_Flip; break;

        case Qt::Key_unknown: keysym = NoSymbol; break;
    }
    if (keysym == NoSymbol) {
        qWarning("Could not map Qt keycode 0x%x to an X keysym", (int)keycode);
        return;
    }

    // Convert the X keysym into an X keycode.
    KeyCode xkeycode = XKeysymToKeycode(dpy, keysym);
    if (xkeycode == NoSymbol) {
        qWarning("X keysym 0x%x could not be mapped to an X keycode",
                 (int)keysym);
        return;
    }

    // Determine if we need to fake shift keys as well.
    int index = 0;
    while (index < 4 && XKeycodeToKeysym(dpy, xkeycode, index) != keysym)
        ++index;
    int extraModifiers = 0;
    if ((index & 1) != 0)
        extraModifiers |= ShiftMask;
    if ((index & 2) != 0)
        extraModifiers |= Mod2Mask;
    if ((prevModifiers & LockMask) != 0) {
        // If Caps Lock is set, then flip the shift state for alphabetic keys.
        if (keycode >= Qt::Key_A && keycode <= Qt::Key_Z)
            extraModifiers ^= ShiftMask;
    }
    if (keycode >= Qt::Key_A && keycode <= Qt::Key_Z &&
        (modifiers & Qt::ShiftModifier) != 0) {
        // Alphabetic shift modifier supplied in the parameters.
        extraModifiers ^= ShiftMask;
    }

    // Adjust modifier keys for the required shift states.
    unsigned long delay = 0;
    if (extraModifiers != 0) {
        if ((extraModifiers & ShiftMask) != 0) {
            if ((prevModifiers & ShiftMask) == 0)
                XTestFakeKeyEvent(dpy, shiftKeycode, true, delay++);
        } else {
            if ((prevModifiers & ShiftMask) != 0)
                XTestFakeKeyEvent(dpy, shiftKeycode, false, delay++);
        }
        if ((extraModifiers & Mod2Mask) != 0) {
            if ((prevModifiers & Mod2Mask) == 0)
                XTestFakeKeyEvent(dpy, modeSwitchKeycode, true, delay++);
        } else {
            if ((prevModifiers & Mod2Mask) != 0)
                XTestFakeKeyEvent(dpy, modeSwitchKeycode, false, delay++);
        }
    }

    // Fake the actual key.
    XTestFakeKeyEvent(dpy, xkeycode, (Bool)isPress, delay++);

    // Adjust the modifiers back.
    if (extraModifiers != 0) {
        if ((extraModifiers & ShiftMask) != 0) {
            if ((prevModifiers & ShiftMask) == 0)
                XTestFakeKeyEvent(dpy, shiftKeycode, false, delay++);
        } else {
            if ((prevModifiers & ShiftMask) != 0)
                XTestFakeKeyEvent(dpy, shiftKeycode, true, delay++);
        }
        if ((extraModifiers & Mod2Mask) != 0) {
            if ((prevModifiers & Mod2Mask) == 0)
                XTestFakeKeyEvent(dpy, modeSwitchKeycode, false, delay++);
        } else {
            if ((prevModifiers & Mod2Mask) != 0)
                XTestFakeKeyEvent(dpy, modeSwitchKeycode, true, delay++);
        }
    }

    // Flush the key events.
    XFlush(dpy);

    // Update the modifiers if this was a shift key.
    if (isPress) {
        if (keycode == Qt::Key_Shift)
            prevModifiers |= ShiftMask;
        if (keycode == Qt::Key_CapsLock)
            prevModifiers |= LockMask;
        if (keycode == Qt::Key_Mode_switch)
            prevModifiers |= Mod2Mask;
    } else {
        if (keycode == Qt::Key_Shift)
            prevModifiers &= ~ShiftMask;
        if (keycode == Qt::Key_CapsLock)
            prevModifiers &= ~LockMask;
        if (keycode == Qt::Key_Mode_switch)
            prevModifiers &= ~Mod2Mask;
    }
}

void QtopiaInputEvents::processKeyEvent
    (int unicode, int keycode, Qt::KeyboardModifiers modifiers,
    bool isPress, bool autoRepeat)
{
    // processKeyEvent() is the same as sendKeyEvent() for X11.
    // Physical keyboards and input methods will typically be
    // handled via XIM, rather than direct key faking.
    sendKeyEvent(unicode, keycode, modifiers, isPress, autoRepeat);
}

void QtopiaInputEvents::addKeyboardFilter(QtopiaKeyboardFilter *f)
{
    Q_UNUSED(f);
}

void QtopiaInputEvents::removeKeyboardFilter()
{
}

void QtopiaInputEvents::suspendMouse()
{
}

void QtopiaInputEvents::resumeMouse()
{
}

void QtopiaInputEvents::openMouse()
{
}

void QtopiaInputEvents::openKeyboard()
{
}

#endif // Q_WS_X11
