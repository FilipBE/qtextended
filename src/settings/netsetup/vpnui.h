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

#ifndef VPNUI_H
#define VPNUI_H

#include <QWidget>
#include <QModelIndex>
#include <qvpnclient.h>

class QAction;
class QListView;
class VPNListModel;

class VpnUI : public QWidget {
    Q_OBJECT
public:
    VpnUI( QWidget* parent = 0, Qt::WFlags f = 0 ) ;
    ~VpnUI();

private:
    void init();

private slots:
    void newConnection();
    void editConnection();
    void removeConnection();
    void newVPNSelected( const QModelIndex& cur, const QModelIndex& prev = QModelIndex() );
    void vpnActivated( const QModelIndex& item );

private:
    QAction* propertyAction;
    QAction* removeAction;
    QListView* vpnList;
    VPNListModel* model;

    friend class NetworkUI;
};

#endif
