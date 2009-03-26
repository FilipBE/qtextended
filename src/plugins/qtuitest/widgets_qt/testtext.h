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

#ifndef TESTTEXT_H
#define TESTTEXT_H

#include <qtuitestnamespace.h>
#include "testwidgetslog.h"

namespace TestText {

    inline
    QByteArray textChangedSignal(QObject* q)
    {
        QByteArray textChangedSignal("textChanged(QString)");
        if (-1 == q->metaObject()->indexOfSignal(textChangedSignal)) {
            textChangedSignal = "textChanged()";
        }
        textChangedSignal.prepend(SIGNAL());
        return textChangedSignal;
    }

    // Erase the text in the widget q by using key clicks
    template <typename T> inline
    bool eraseTextByKeys(T* q)
    {
        using namespace QtUiTest;

        if (!q->hasFocus()) {
            setErrorString("Cannot erase text from unfocused widget.");
            return false;
        }

        Widget* w = qtuitest_cast<Widget*>(q);
        bool hadEditFocus = w->hasEditFocus();
        if (!w->setEditFocus(true)) {
            setErrorString("Couldn't erase text from widget: couldn't give edit focus to widget.");
            return false;
        }

        const int MAX = q->text().length()+1;

        int pos = q->cursorPosition();
        int oldPos = -1;
        int i = 0;

        TestWidgetsLog() << "pos" << pos << "MAX" << MAX;

        do {
            keyClick(q, SIGNAL(cursorPositionChanged(int,int)), Qt::Key_Right);
            oldPos = pos;
            pos = q->cursorPosition();
        } while (oldPos != pos && (++i < MAX));

        if (i >= MAX) {
            setErrorString("Could not move cursor to rightmost position in widget.");
            return false;
        }

        QByteArray textChanged = textChangedSignal(q);
        do {
            if (!keyClick(q, textChanged, Qt::Key_Backspace))
                return false;
        } while (q->cursorPosition() != 0 && (++i < MAX));

        if (i >= MAX || !q->text().isEmpty()) {
            setErrorString("Could not erase all text by pressing backspace.");
            return false;
        }

        if (!w->setEditFocus(hadEditFocus)) {
            setErrorString("Couldn't return widget to original edit focus state.");
            return false;
        }

        return true;
    }

    template <typename T> inline
    bool eraseTextByMouse(T*)
    {
        QtUiTest::setErrorString("Erasing text by mouse is not implemented.");
        return false;
    }

    bool enterText(QObject* q, QString const& item, QString const& expectedText, bool commit = true);
    bool enterTextByProxy(QtUiTest::InputWidget* proxy, QObject* q, QString const& item, QString const& expectedText, bool commit = true);
};

#endif

