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

#include "ipconfig.h"
#include <ipvalidator.h>

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QDesktopWidget>
#include <QFormLayout>

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>


/*!
  \class IPPage
    \inpublicgroup QtBaseModule
  \brief The IPPage class provides the user interface for IP configurations.


  \internal

  The IPPage widget is exclusively used by the Qt Extended network plug-ins. It 
  allows the user to edit the following IP details:

  \list
    \o IP address
    \o 1st DNS server
    \o 2nd DNS server
    \o broadcast address
    \o subnet mask
    \o gateway address
  \endlist
  
  This is not a public class.
*/

IPPage::IPPage( const QtopiaNetworkProperties& prop, QWidget* parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    init();
    readConfig( prop );

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this , true );

    setObjectName("tcpip");
}

IPPage::~IPPage()
{
}

void IPPage::init()
{
    QVBoxLayout* vLayout = new QVBoxLayout( this );
    vLayout->setSpacing( 4 );
    vLayout->setMargin( 5 );

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

void IPPage::readConfig( const QtopiaNetworkProperties& prop )
{
    if (prop.value("Properties/DHCP").toString() != "n")
        autoIp->setCheckState( Qt::Checked);
    else {
        autoIp->setCheckState( Qt::Unchecked );
        ipAddress->setText( prop.value("Properties/IPADDR").toString() );
        dnsAddress1->setText( prop.value("Properties/DNS_1").toString() );
        dnsAddress2->setText( prop.value("Properties/DNS_2").toString() );
        broadcast->setText( prop.value("Properties/BROADCAST").toString() );
        gateway->setText( prop.value("Properties/GATEWAY").toString() );
        subnet->setText( prop.value("Properties/SUBNET").toString() );
    }

    connectWdgts();
}

QtopiaNetworkProperties IPPage::properties()
{
    QtopiaNetworkProperties props;
    if (autoIp->checkState() == Qt::Checked ) {
        props.insert("Properties/DHCP",  "y");
        props.insert("Properties/IPADDR", QString());
        props.insert("Properties/DNS_1", QString());
        props.insert("Properties/DNS_2", QString());
        props.insert("Properties/BROADCAST", QString());
        props.insert("Properties/GATEWAY", QString());
        props.insert("Properties/SUBNET", QString());
    } else {
        props.insert("Properties/DHCP", "n");
        props.insert("Properties/IPADDR", ipAddress->text());
        props.insert("Properties/DNS_1", dnsAddress1->text());
        props.insert("Properties/DNS_2", dnsAddress2->text());
        props.insert("Properties/BROADCAST", broadcast->text());
        props.insert("Properties/GATEWAY", gateway->text());
        props.insert("Properties/SUBNET", subnet->text());
    }

    return props;
}

void IPPage::connectWdgts()
{
    if (autoIp->checkState() == Qt::Unchecked) {
        dhcpGroup->setEnabled( true );
    } else {
        dhcpGroup->setEnabled( false );
    }
}

