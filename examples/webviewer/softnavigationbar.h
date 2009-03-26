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

#ifndef SOFTNAVIGATIONBAR_H
#define SOFTNAVIGATIONBAR_H

#include <QObject>

class QAction;

class SoftNavigationBar : public QObject
{
    Q_OBJECT

public:
    SoftNavigationBar(QWidget* parent = 0);

    void setBack(QAction*);
    void setForward(QAction*);

private slots:
    void actionChanged();

protected:
    bool eventFilter(QObject *, QEvent *);

private:
    QWidget *mainWindow;
    QAction *backAction;
    QAction *forwardAction;
};

#endif
