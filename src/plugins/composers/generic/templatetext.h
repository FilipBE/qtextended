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
#ifndef TEMPLATETEXT_H
#define TEMPLATETEXT_H

#include <qdialog.h>
#include <qstring.h>

class QAction;
class QListWidget;
class QListWidgetItem;
class QLineEdit;

class TemplateTextDialog : public QDialog
{
    Q_OBJECT
public:
    TemplateTextDialog(QWidget *parent, const char* name);
    virtual ~TemplateTextDialog();
    QString text();
    void loadTexts();
    void saveTexts();

private slots:
    void selected();
    void slotRemove();
    void slotReset();
    void slotUpdateActions();
 private:
    QListWidgetItem *mNewTemplateText;
    QListWidget *mTemplateList;
    QAction *removeAction;
    QAction *resetAction;

    int userTemplates;
};

class NewTemplateTextDialog : public QDialog
{
    Q_OBJECT
public:
    NewTemplateTextDialog(QWidget *parent = 0);
    QString text();

private:
    QLineEdit *mEdit;
};

#endif
