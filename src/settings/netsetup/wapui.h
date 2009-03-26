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
#ifndef WAPUI_H
#define WAPUI_H

class QAction;
class QLabel;
class QListWidget;

#include <QWidget>

class WapUI : public QWidget
{
   Q_OBJECT
public:
    WapUI( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~WapUI();

    enum {
        ConfigRole = Qt::UserRole,
        DefaultRole = Qt::UserRole+1} WapRoles;

private slots:
    void addWap();
    void removeWap();
    void doWap();
    void updateActions();
    void selectDefaultWAP();

private:
    void loadConfigs();
    void init();
    QStringList availableWapConfigs();
    void updateNetStates();

private:

    QListWidget* wapList;
    QLabel* dfltAccount;
    QAction* wap_add;
    QAction* wap_remove;
    QAction* wap_props;

    friend class NetworkUI;
};

#endif
