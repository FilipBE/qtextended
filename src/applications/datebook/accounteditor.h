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

#ifndef ACCOUNTEDITOR_H
#define ACCOUNTEDITOR_H

#include <QWidget>
#ifndef QT_NO_OPENSSL
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QAction;
class QTimer;
class QAppointmentModel;

class AccountEditor : public QWidget
{
    Q_OBJECT
public:
    AccountEditor(QWidget *parent = 0);
    ~AccountEditor();

    void setModel(QAppointmentModel *model);

    static bool editableAccounts(const QAppointmentModel *);

private slots:
    void addAccount();
    void removeCurrentAccount();
    void editCurrentAccount();

    void syncAllAccounts();
    void syncCurrentAccount();

    void updateActions();

    void updateProgress();

    void updateAccountName(const QString& account);

    void populate();

    void hideProgressBar();
    void processSyncStatus(const QString &, int);

    void currentAccountChanged(QListWidgetItem *);

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    QAppointmentModel *mModel;

    QListWidget *mChoices;
    QProgressBar *mProgress; // if a sync is activated, activate this.  more than one?  or is a busy indicator more appropriate?

    QAction *actionAdd;
    QAction *actionEdit;
    QAction *actionRemove;
    QAction *actionSync;
    QAction *actionSyncAll;

    QTimer *progressHideTimer;
};
#endif //QT_NO_OPENSSL

#endif
