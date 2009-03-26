/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#ifndef QTWINDOWLISTMENU_H
#define QTWINDOWLISTMENU_H

#include <QMap>
#include <QMenu>
#include <QActionGroup>

QT_BEGIN_NAMESPACE

class QMenuBar;
class QWidget;
class QString;
class QWorkspace;
class QAction;

class QtWindowListMenu : public QMenu
{
    Q_OBJECT
public:
	QtWindowListMenu(QWorkspace *workspace, QWidget *parent = 0, const char *name = 0);
	QAction *addTo(const QString &text, QMenuBar *menubar, int idx = -1);
	void removeWindow(QWidget *w, bool windowDestroyed = false);

	virtual bool eventFilter(QObject *obj, QEvent *e);

    void setWindowIcon(QWidget *widget, const QIcon &icon);
    void setDefaultIcon(const QIcon &icon);

    void setCloseIcon(const QIcon &icon);
    void setCloseAllIcon(const QIcon &icon);
    void setCascadeIcon(const QIcon &icon);
    void setTileIcon(const QIcon &icon);

public slots:
	void addWindow(QWidget *w);
    void addWindow(QWidget *w, const QIcon &icon);
	virtual void setEnabled(bool b);
	void windowDestroyed(QObject *obj);

private slots:
    void setSenderChecked(bool checked);

private:
	QMenuBar *m_menubar;
	QAction *m_my_action;
    QAction *m_close_current_action;
    QAction *m_close_all_action;
    QAction *m_cascade_action;
    QAction *m_tile_action;

    QIcon m_default_icon;

	/* A list of window/QAction* pairs. If the QAction-pointer is 0, we are keeping
	   track of the window, but it's hidden so it's not in the menu. */
	typedef QMap<QWidget *, QAction *> WindowList;
	WindowList m_window_list;
	QWorkspace *m_workspace;
    QActionGroup groupWindows;

	bool isEmpty();
    void setChecked(bool checked, QAction *a);
};

QT_END_NAMESPACE

#endif
