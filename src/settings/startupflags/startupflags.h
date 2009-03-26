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
#ifndef STARTUPFLAGS_H
#define STARTUPFLAGS_H

#include <QMap>
#include <QDialog>

class QTreeWidget;
class QTreeWidgetItem;

class StartupFlags : public QDialog
{
    Q_OBJECT

public:
    StartupFlags(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~StartupFlags();

protected slots:
    void accept();
    void showWhatsThis();
    void flagChanged(QTreeWidgetItem *item, int column);

private:
    QMap<QString,QTreeWidgetItem*> contexts;
    QMap<QString,QTreeWidgetItem*> item;
    QTreeWidget* list;

    void loadSettings();
    QVariant currentValue(QString group);
};

#endif
