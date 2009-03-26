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
#ifndef DOCKEDKEYBOARD_H
#define DOCKEDKEYBOARD_H

#include <qwindowsystem_qws.h>
#include "../keyboard/keyboard.h"

/*
    DockedKeyboard is an input method for Qtopia.  DockedKeyboard docks a popup widget depicting keys on the bottom of the screen and reduces the available screen for other widgets by this space.  The popup widget (KeyboardFrame), interprets mouse events and converts them into key events.

*/

class DockedKeyboard : public Keyboard
{
    Q_OBJECT
public:
    DockedKeyboard( QWidget* parent=0, Qt::WFlags f=0 );
    virtual ~DockedKeyboard();

    QWidget* frame();
    void resetState();
    void queryResponse ( int , const QVariant & );
};

#endif
