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

#ifndef QSOFTMENUBAR_H
#define QSOFTMENUBAR_H

#include <qtopiaglobal.h>

#include <QWidget>
#include <qstring.h>
#include <QList>

class QWidget;
class QMenu;

class QTOPIA_EXPORT QSoftMenuBar
{
public:
    enum FocusState {
        EditFocus=1, NavigationFocus=2, AnyFocus=3
    };

    enum StandardLabel {
        NoLabel, Options, Ok, Edit, Select, View, Cancel, Back, BackSpace, Next, Previous, EndEdit, RevertEdit, Deselect, Finish
    };

    enum LabelType {
        IconLabel, TextLabel
    };

    static void setLabel(QWidget *, int key, const QString &, const QString &, FocusState state=AnyFocus);
    static void setLabel(QWidget *, int key, StandardLabel, FocusState state=AnyFocus);
    static void clearLabel(QWidget *, int key, FocusState state=AnyFocus);

    static const QList<int> &keys();

    static int menuKey();
    static QMenu *menuFor(QWidget *w, FocusState state=AnyFocus);
    static bool hasMenu(QWidget *w, FocusState state=AnyFocus);
    static void addMenuTo(QWidget *w, QMenu *menu, FocusState state=AnyFocus);
    static void removeMenuFrom(QWidget *w, QMenu *menu, FocusState state=AnyFocus);
    static QWidgetList widgetsFor(const QMenu *menu, FocusState state=AnyFocus);
    static void setHelpEnabled(QWidget *w, bool enable);
    static void setCancelEnabled(QWidget *w, bool enable);
    static void setInputMethodEnabled(QWidget *widget, bool enable);
    static QMenu *createEditMenu();
    static QMenu *activeMenu();

private:
    QSoftMenuBar();
};

#endif
