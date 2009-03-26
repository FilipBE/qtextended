/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef Q3MAINWINDOW_P_H
#define Q3MAINWINDOW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qwidget_p.h>

QT_BEGIN_NAMESPACE

class Q3MainWindowLayout;

class Q3MainWindowPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(Q3MainWindow)
public:
    Q3MainWindowPrivate()
        :  mb(0), sb(0), ttg(0), mc(0), tll(0), mwl(0), ubp(false), utl(false),
           justify(false), movable(true), opaque(false), dockMenu(true)
    {
        docks.insert(Qt::DockTop, true);
        docks.insert(Qt::DockBottom, true);
        docks.insert(Qt::DockLeft, true);
        docks.insert(Qt::DockRight, true);
        docks.insert(Qt::DockMinimized, false);
        docks.insert(Qt::DockTornOff, true);
    }

    ~Q3MainWindowPrivate()
    {
    }

#ifndef QT_NO_MENUBAR
    mutable QMenuBar * mb;
#else
    mutable QWidget * mb;
#endif
    QStatusBar * sb;
    QToolTipGroup * ttg;

    QWidget * mc;

    QBoxLayout * tll;
    Q3MainWindowLayout * mwl;

    uint ubp :1;
    uint utl :1;
    uint justify :1;
    uint movable :1;
    uint opaque :1;
    uint dockMenu :1;

    Q3DockArea *topDock, *bottomDock, *leftDock, *rightDock;

    QList<Q3DockWindow *> dockWindows;
    QMap<Qt::Dock, bool> docks;
    QStringList disabledDocks;
    QHideDock *hideDock;

    QPointer<Q3PopupMenu> rmbMenu, tbMenu, dwMenu;
    QMap<Q3DockWindow*, bool> appropriate;
    mutable QMap<Q3PopupMenu*, Q3MainWindow::DockWindows> dockWindowModes;
};

QT_END_NAMESPACE

#endif // Q3MAINWINDOW_P_H
