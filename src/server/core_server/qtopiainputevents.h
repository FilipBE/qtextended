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

#ifndef QTOPIAINPUTEVENTS_H
#define QTOPIAINPUTEVENTS_H

#include <qobject.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#ifdef Q_WS_QWS
typedef QWSServer::KeyboardFilter QtopiaKeyboardFilter;
#else
class QtopiaKeyboardFilter
{
public:
    virtual ~QtopiaKeyboardFilter() {}
    virtual bool filter(int unicode, int keycode, int modifiers,
                        bool isPress, bool autoRepeat)=0;
};
#endif

class QtopiaInputEvents
{
private:
    QtopiaInputEvents() {}
    ~QtopiaInputEvents() {}

public:
    static void sendKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                             bool isPress, bool autoRepeat);
    static void processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                bool isPress, bool autoRepeat);

    static void addKeyboardFilter(QtopiaKeyboardFilter *f);
    static void removeKeyboardFilter();

    static void suspendMouse();
    static void resumeMouse();

    static void openMouse();
    static void openKeyboard();
};

#endif
