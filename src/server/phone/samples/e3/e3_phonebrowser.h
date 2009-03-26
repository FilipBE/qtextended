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

#ifndef E3_PHONEBROWSER_H
#define E3_PHONEBROWSER_H

#include "qabstractbrowserscreen.h"
#include <QContentSet>

class QAction;
class E3BrowserScreenStack;
class E3BrowserScreen : public QAbstractBrowserScreen
{
Q_OBJECT
public:
    E3BrowserScreen(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;
    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void listMode();
    void gridMode();

private:
    QAction *listAction;
    QAction *gridAction;
    E3BrowserScreenStack *m_stack;
};

#endif
