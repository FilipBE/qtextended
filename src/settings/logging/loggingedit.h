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
#ifndef LOGGINGEDIT_H
#define LOGGINGEDIT_H

#include <QDialog>
#include <QMap>
#include <QString>

class QTreeWidget;
class QTreeWidgetItem;

class LoggingEdit : public QDialog
{
    Q_OBJECT

public:
    LoggingEdit(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~LoggingEdit();

protected slots:
    void accept();

private:
    QMap<QString,QTreeWidgetItem*> item;
    QTreeWidget* list;
};

// This implementation is a generic mechanism for adding a What's This?
// menu item to the softmenubar menu.
//
// This is a prototype for a mechanism to be put into a Qtopia library,
// potentially Qtopia Core or Qt proper.
//
// See also LoggingEdit::showWhatsThis below.
//
class QWhatThisInMenu : public QObject {
    Q_OBJECT
public:
    QWhatThisInMenu(QWidget* parent);

protected slots:
    void findWhatsThis();
    void showWhatsThis();

private:
    bool sendWhatsThisEvent(bool query);
    QAction *action;
};


#endif
