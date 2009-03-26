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



#ifndef EDITACCOUNT_H
#define EDITACCOUNT_H

#include "ui_editaccountbasephone.h"

#include <qlistwidget.h>
#include <qtextedit.h>
#include <qtimer.h>

class QMailAccount;
class QTabWidget;
class AccountConfiguration;

class EditAccount : public QDialog, public Ui::EditAccountBase
{
    Q_OBJECT

public:
    EditAccount( QWidget* parent = 0, const char* name = 0,Qt::WFlags fl = 0 );
    virtual ~EditAccount(){};
    void setAccount(QMailAccount *in, AccountConfiguration* config, bool defaultServer);
    bool isDefaultAccount();
    bool eventFilter( QObject *, QEvent * );

protected slots:
    void accept();
    void deleteAccount();
    void emailModified();
    void typeChanged(int);
    void sigPressed();
//  void configureFolders();
    void authChanged(int index);
    void createTabbedView();
    void currentTabChanged(int index);
    void intervalCheckChanged(int enabled);
    void pushCheckChanged(int enabled);

private:
    QMailAccount *account;
    AccountConfiguration *config;
    bool emailTyped;
    QString sig;
    QLineEdit* accountNameInput;
};

class SigEntry : public QDialog
{
    Q_OBJECT
public:
    SigEntry(QWidget *parent, const char* name, Qt::WFlags fl = 0 );
    void setEntry(QString sig) { input->insertPlainText(sig); };
    QString entry() { return input->toPlainText(); };

private:
    QTextEdit *input;
};


#endif
