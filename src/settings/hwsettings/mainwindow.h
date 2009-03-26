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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QList>
#include <profile.h>

class QIMPenProfile;
class CharSetDlg;
class QDialog;
class GeneralPref;
class QTreeWidget;
class QTreeWidgetItem;

class QIMPenProfileEdit : public QDialog
{
    Q_OBJECT
public:
    QIMPenProfileEdit(QWidget *parent, Qt::WFlags f = 0);
    ~QIMPenProfileEdit();

private slots:
    void editItem(QTreeWidgetItem *);

private:
    bool loadProfiles();
    bool saveProfiles();

    QList<QIMPenProfile *> profileList;

    QTreeWidget *lv;

    CharSetDlg *cdiag;
    QDialog *gdiag;
    GeneralPref *gpb;
};

#endif
