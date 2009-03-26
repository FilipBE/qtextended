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

#ifndef WIRELESSIPCONFIG_H
#define WIRELESSIPCONFIG_H

#include <custom.h>

#ifndef NO_WIRELESS_LAN

#include <QWidget>
#include <qtopianetworkinterface.h>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>


class WirelessIPPage : public QWidget
{
    Q_OBJECT
public:
    WirelessIPPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags flags = 0 );
    ~WirelessIPPage();

    QtopiaNetworkProperties properties();
    void setProperties( const QtopiaNetworkProperties& cfg );

private slots:
    void connectWdgts();
    void newNetSelected( int index );

private:
    void init();
    void initNetSelector( const QtopiaNetworkProperties& prop );
    void readConfig( );
    void saveConfig( );

private:
    QComboBox* netSelector;
    QCheckBox* autoIp;
    QGroupBox* dhcpGroup;
    QLabel* ipLabel;
    QLineEdit* ipAddress;

    QLabel* dnsLabel1, *dnsLabel2;
    QLineEdit* dnsAddress1, *dnsAddress2;

    QLabel* broadcastLabel;
    QLineEdit* broadcast;
    QLabel* gatewayLabel;
    QLineEdit* gateway;
    QLabel* subnetLabel;
    QLineEdit* subnet;
    QtopiaNetworkProperties changedSettings;
    int lastIndex;
};
#endif //NO_WIRELESS_LAN

#endif
