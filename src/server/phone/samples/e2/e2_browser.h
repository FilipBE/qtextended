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

#ifndef E2_BROWSER_H
#define E2_BROWSER_H

#include "qabstractbrowserscreen.h"
#include <QStringList>
class E2Menu;
class E2Button;
class E2BrowserStack;
class E2BrowserScreen : public QAbstractBrowserScreen
{
Q_OBJECT
public:
    E2BrowserScreen(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;
    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

private slots:
    void categoryChanged(int);
    void viewChanged(const QString &);
    void menuClicked(int);

private:
    void toggleViewMode();
    enum { ListView, IconView } m_mode;
    E2Button *m_catMenu;
    QStringList m_categories;
    QStringList m_categoryNames;
    E2BrowserStack *m_stack;
    E2Menu *m_menu;
};

#endif
