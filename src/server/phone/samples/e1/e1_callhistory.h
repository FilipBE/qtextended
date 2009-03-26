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

#ifndef E1_CALLHISTORY_H
#define E1_CALLHISTORY_H

#include "e1_dialog.h"

#include <QListWidget>

class E1CallHistory : public E1Dialog // creates an E1Dialog and sets a selector in it
{
    Q_OBJECT
public:
    enum Type
    {
        Dialed,
        Answered,
        Missed
    };
    E1CallHistory( QWidget* parent );
private slots:
    void typeSelected(const E1CallHistory::Type& t);
};

class E1CallHistorySelector : public QListWidget // the selector
{
    Q_OBJECT
public:
    E1CallHistorySelector( QWidget* parent );

private slots:
    void itemSelected(QListWidgetItem*);

signals:
    void selected(const E1CallHistory::Type&);
};

class E1CallHistoryList : public QListWidget
{
    Q_OBJECT
public:
    E1CallHistoryList( QWidget* parent, const E1CallHistory::Type& t );

signals:
    void closeMe();

private slots:
    void clicked();

private:
    QStringList numbers;
};

#endif
