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

#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include "ui_searchviewbasephone.h"

#include <QDialog>
#include <QMailMessageKey>

class QScrollArea;
class QAction;

class SearchView : public QDialog, public Ui::SearchViewBase
{
    Q_OBJECT

public:
    SearchView(QWidget* parent = 0);
    ~SearchView();

    QMailMessageKey searchKey() const;
    QString bodyText() const;

    void reset();

private:
    void init();

private slots:
    void updateActions();
    void editRecipients();

private:
    QDate dateBefore, dateAfter;
    QScrollArea *sv;
    QAction *pickAddressAction;
};

#endif
