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

#include "wirelessipconfig.h"

#ifndef NO_WIRELESS_LAN

#include <QSoftMenuBar>
#include <QVBoxLayout>
#include <QFormLayout>
#include <ipvalidator.h>
#include <QtopiaApplication>


WirelessIPPage::WirelessIPPage( const QtopiaNetworkProperties& cfg, QWidget* parent, Qt::WFlags flags )
    : QWidget(parent, flags)
{
    init();
    initNetSelector( cfg );
 
    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
 
    setObjectName("tcpip");
}

WirelessIPPage::~WirelessIPPage()
{
}

void WirelessIPPage::init()
{
    QVBoxLayout* vLayout = new QVBoxLayout( this );
    vLayout->setSpacing(4);
    vLayout->setMargin(5);
 
    netSelector = new QComboBox( this );
    vLayout->addWidget(netSelector);

    QFrame* line = new QFrame( this );
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vLayout->addWidget(line);

    autoIp = new QCheckBox( tr("Autom. IP (DHCP)"), this );
    vLayout->addWidget( autoIp );

    dhcpGroup = new QGroupBox( this );
    QFormLayout* formLayout = new QFormLayout( dhcpGroup );

    IPValidator* val = new IPValidator( this );

    ipLabel = new QLabel( tr("IP Address:"), dhcpGroup );
    ipAddress = new QLineEdit( dhcpGroup );
    ipAddress->setValidator( val );
    ipLabel->setBuddy( ipAddress );
    formLayout->addRow( ipLabel, ipAddress );

    dnsLabel1 = new QLabel( tr("First DNS:"), dhcpGroup );
    dnsAddress1 = new QLineEdit( dhcpGroup );
    dnsAddress1->setValidator( val );
    dnsLabel1->setBuddy( dnsAddress1 );
    formLayout->addRow( dnsLabel1, dnsAddress1 );

    dnsLabel2 = new QLabel( tr("Second DNS:"), dhcpGroup );
    dnsAddress2 = new QLineEdit( dhcpGroup );
    dnsAddress2->setValidator( val );
    dnsLabel2->setBuddy( dnsAddress2 );
    formLayout->addRow( dnsLabel2, dnsAddress2 );

    broadcastLabel = new QLabel( tr("Broadcast:"), dhcpGroup );
    broadcast = new QLineEdit( dhcpGroup );
    broadcast->setValidator( val );
    broadcastLabel->setBuddy( broadcast );
    formLayout->addRow( broadcastLabel, broadcast );

    gatewayLabel = new QLabel( tr("Gateway:"), dhcpGroup );
    gateway = new QLineEdit( dhcpGroup );
    gateway->setValidator( val );
    gatewayLabel->setBuddy( gateway );
    formLayout->addRow( gatewayLabel, gateway );

    subnetLabel = new QLabel( tr("Subnet mask:"), dhcpGroup );
    subnet = new QLineEdit( dhcpGroup );
    subnet->setValidator( val );
    subnetLabel->setBuddy( subnet );
    formLayout->addRow( subnetLabel, subnet );

    vLayout->addWidget(dhcpGroup);

    QtopiaApplication::setInputMethodHint(ipAddress, "netmask");
    QtopiaApplication::setInputMethodHint(dnsAddress1, "netmask");
    QtopiaApplication::setInputMethodHint(dnsAddress2, "netmask");
    QtopiaApplication::setInputMethodHint(broadcast, "netmask");
    QtopiaApplication::setInputMethodHint(gateway, "netmask");
    QtopiaApplication::setInputMethodHint(subnet, "netmask");

    connect( autoIp, SIGNAL(stateChanged(int)), this, SLOT(connectWdgts()));
}

void WirelessIPPage::initNetSelector( const QtopiaNetworkProperties& prop )
{
    changedSettings.clear();
    netSelector->clear();
    const QList<QString> keys = prop.keys();
    const int numKnownNetworks = prop.value( QLatin1String("WirelessNetworks/size"), 0).toInt();

    dhcpGroup->setEnabled(numKnownNetworks>0);
    netSelector->setEnabled(numKnownNetworks>0);
    autoIp->setEnabled(numKnownNetworks>0);
    disconnect( netSelector, 0, this, 0 );
    if (numKnownNetworks==0) {
        setEnabled(false);
        netSelector->addItem(tr("<No WLAN defined>"));
        readConfig();
        return;
    } else {
        setEnabled(true);
    }


    disconnect( netSelector, 0, this, 0 );
    foreach( QString key, keys ) {
        if ( !key.startsWith( "WirelessNetworks" ) )
            continue;
        int idx = key.mid(17 /*strlen("WirelessNetworks/")*/, key.indexOf(QChar('/'), 17)-17).toInt();
        if ( idx <= numKnownNetworks ) {
            //copy all values into changedSettings where we keep track of changes
            changedSettings.insert( key, prop.value( key ) );
            if ( key.endsWith( "ESSID" ) ) {
                QString text = prop.value( key ).toString();
                if ( text.isEmpty() )
                    text = tr("Unnamed network");
                netSelector->addItem( text );
            }
        }
    }
    lastIndex = 0;
    netSelector->setCurrentIndex( 0 );
    readConfig();
    connect( netSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(newNetSelected(int)) );
}

void WirelessIPPage::newNetSelected( int index )
{
    if ( index < 0 || index >= netSelector->count() )
        return;
    saveConfig( );
    //lastIndex contains the index of the previously unselected entry
    //after calling saveConfig we have to update lastIndex
    lastIndex = index;
    readConfig( );
}


/*!
    \internal

    Loads details of new networks into the UI elements.
*/
void WirelessIPPage::readConfig( )
{
    const int num = netSelector->currentIndex() + 1;
    const QString s = QString("WirelessNetworks/%1/").arg(num);

    if (changedSettings.value(s+"DHCP").toString() != "n")
        autoIp->setCheckState( Qt::Checked);
    else {
        autoIp->setCheckState( Qt::Unchecked );
    }
    ipAddress->setText( changedSettings.value(s+"IPADDR").toString() );
    dnsAddress1->setText( changedSettings.value(s+"DNS_1").toString() );
    dnsAddress2->setText( changedSettings.value(s+"DNS_2").toString() );
    broadcast->setText( changedSettings.value(s+"BROADCAST").toString() );
    gateway->setText( changedSettings.value(s+"GATEWAY").toString() );
    subnet->setText( changedSettings.value(s+"SUBNET").toString() );

    connectWdgts();
}

QtopiaNetworkProperties WirelessIPPage::properties()
{
    if ( !isEnabled() ) {
        QtopiaNetworkProperties p;
        p.insert("WirelessNetworks/size",0);
        return p;
    }

    saveConfig();
    return changedSettings;
}


/*!
    \internal

    Saves the selected network options.
*/
void WirelessIPPage::saveConfig()
{
    if ( lastIndex < 0 || lastIndex >= netSelector->count() )
        return;

    const QString s = QString("WirelessNetworks/%1/").arg(lastIndex+1);

    if ( autoIp->checkState() == Qt::Unchecked )
        changedSettings.insert(s+"DHCP", "n");
    else
        changedSettings.insert(s+"DHCP", "y");

    changedSettings.insert(s+"IPADDR", ipAddress->text());
    changedSettings.insert(s+"DNS_1", dnsAddress1->text());
    changedSettings.insert(s+"DNS_2", dnsAddress2->text());
    changedSettings.insert(s+"BROADCAST", broadcast->text());
    changedSettings.insert(s+"GATEWAY", gateway->text());
    changedSettings.insert(s+"SUBNET", subnet->text());
}

void WirelessIPPage::setProperties( const QtopiaNetworkProperties& cfg )
{
    initNetSelector( cfg );
}

void WirelessIPPage::connectWdgts()
{
    if (autoIp->checkState() == Qt::Unchecked) {
        dhcpGroup->setEnabled( true );
    } else {
        dhcpGroup->setEnabled( false );
    }
}

#endif
