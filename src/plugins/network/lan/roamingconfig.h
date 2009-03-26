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

#ifndef ROAMINGCONFIG_H
#define RAOMINGCONFIG_H

#ifndef NO_WIRELESS_LAN
#include <QMultiHash>
#include <QWidget>
#include <QObject>

#include <qtopianetworkinterface.h>

#include "ui_roamingbase.h"

class QEvent;
class QListWidgetItem;
class RoamingPage : public QWidget
{
Q_OBJECT
public:
    RoamingPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~RoamingPage();

    QtopiaNetworkProperties properties();
    void setProperties( const QtopiaNetworkProperties& cfg );
    bool eventFilter( QObject* watched, QEvent* event );
private:
    void init( const QtopiaNetworkProperties& cfg );
    void readConfig();
    void saveConfig();

private slots:
    void reconnectToggled(int newState);
    void listActivated(QListWidgetItem*);

private:
    QMultiHash<QString, QVariant> props;
    Ui::RoamingBase ui;
    QListWidgetItem* currentSelection;
};

#endif //NO_WIRELESS_LAN
#endif
