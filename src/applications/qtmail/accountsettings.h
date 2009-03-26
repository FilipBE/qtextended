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

#ifndef ACCOUNTSETTINGS_H
#define ACCOUNTSETTINGS_H

#include <qdialog.h>
#include <qmap.h>
#include <QMailAccountId>
#include <QMailRetrievalAction>
#include <QModelIndex>

class EmailClient;
class StatusDisplay;
class QMailAccount;
class QMenu;
class QAction;
class QSmoothList;
class QMailAccountListModel;

class AccountSettings : public QDialog
{
    Q_OBJECT
public:
    AccountSettings(EmailClient *parent, const char *name=0, bool modal=true, const QMailAccountId& defaultId = QMailAccountId());

    QMailAccountId defaultAccountId() const;

signals:
    void deleteAccount(const QMailAccountId &id);

public slots:
    void addAccount();

protected:
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);

private slots:
    void removeAccount();
    void accountSelected(QModelIndex index);
    void updateActions();
    void displayProgress(uint, uint);
    void activityChanged(QMailServiceAction::Activity activity);
    void retrieveFolders();

private:
    void editAccount(QMailAccount *account);

private:
    QMap<int,int> listToAccountIdx;
    QMailAccountListModel *accountModel;
    QSmoothList *accountView;
    QMenu *context;
    QAction *addAccountAction;
    QAction *removeAccountAction;
    StatusDisplay *statusDisplay;
    QMailAccountId defaultId;
    QPoint cPos;
    bool preExisting;
    QMailRetrievalAction *retrievalAction;
};

#endif
