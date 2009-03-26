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

//#include "graph.h"
#include "networkinfo.h"

#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QtNetwork>
#include <QScrollArea>
#include <QFrame>
#include <QGroupBox>

#include <QtopiaNetwork>
#include <QNetworkState>
#include <QNetworkDevice>
#include <QNetworkAddressEntry>
#include <QHostAddress>

#include <QtopiaNetworkInterface>

#include <QWapAccount>

#include <stdio.h>
#include <errno.h>


NetworkInfoView::NetworkInfoView( QWidget *parent )
    : QWidget( parent) , area(0)
{
    QTimer::singleShot(10, this, SLOT(init()));
}

void NetworkInfoView::init()
{
    QLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( 0 );
    layout->setMargin( 0 );
    area = new QScrollArea;
    layout->addWidget( area );

    area->setFocusPolicy( Qt::TabFocus );
    area->setFrameShape( QFrame::NoFrame );

    updateNetInfo();
}

void NetworkInfoView::timerEvent(QTimerEvent*)
{
    if (isVisible()) {
        emit updated();
    }
}

QSize NetworkInfoView::sizeHint() const
{
    QSize s = area ? area->sizeHint() : QSize();
    return QSize( s.width()+8, s.height() );
}

void NetworkInfoView::updateNetInfo()
{
    QWidget *vb = new QWidget;
    QVBoxLayout *vLayout = new QVBoxLayout(vb);
    vLayout->setSpacing( 6 );
    vLayout->setMargin(0);

    QNetworkState *netState;
    netState = new QNetworkState(this);
    if( !netState->gateway().isEmpty()) {
        QString addy = QNetworkDevice( netState->gateway()).address().addressEntries()[0].ip().toString();
        QLabel *gatewayLabel;
        gatewayLabel = new QLabel(tr("Default Gateway: ") + addy);
        vLayout->addWidget( gatewayLabel );
    }

    QString wapName;
    QString wapAccount = netState->defaultWapAccount();
    if ( !wapAccount.isEmpty() ) {
        QWapAccount acc( netState->defaultWapAccount() );
        wapName = acc.name();
        QLabel *wapLabel;
        wapLabel = new QLabel(tr("WAP: ") + wapName);
        vLayout->addWidget( wapLabel );
    }


    QList<QString> knownNetworkDevices = QNetworkState::availableNetworkDevices( QtopiaNetwork::Any );

    if (knownNetworkDevices.isEmpty()) {
        QLabel *nothingLabel;
        nothingLabel = new QLabel(tr("No interfaces found..."));
        nothingLabel->setWordWrap(true);
        vLayout->addWidget( nothingLabel );
    }

    QNetworkDevice *networkDevice;
    for(int i = 0; i < knownNetworkDevices.count(); ++i) {
        networkDevice = new QNetworkDevice( knownNetworkDevices.at(i));
        NetworkDeviceInfo* netinfo = new NetworkDeviceInfo( networkDevice);
        // Force the minimum size (or it gets squashed)
        netinfo->setMinimumSize( netinfo->sizeHint() );
        netinfo->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
        vLayout->addWidget(netinfo);
    }


    vLayout->addStretch( 1 );
    if ( area->widget() )
        delete area->takeWidget();
    area->setWidget( vb );
    area->setWidgetResizable( true );
    vb->show();
}

NetworkDeviceInfo::NetworkDeviceInfo( const QNetworkDevice* netDevice, QWidget *parent )
    : QFrame( parent )
{

    this->setFrameShape(QFrame::StyledPanel);

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setSpacing( 7 );

    QLabel*name;
    name = new QLabel( netDevice->name());
    vb->addWidget( name);

    QLabel* iname;
    iname = new QLabel(tr("Interface: ") + netDevice->interfaceName());
    vb->addWidget( iname );

    QNetworkInterface interface;
    interface = netDevice->address();

    QString interfaceStatus;
    switch (netDevice->state()) {
    case QtopiaNetworkInterface::Unknown:
        interfaceStatus = tr("Unknown");
        break;
    case QtopiaNetworkInterface::Down:
        interfaceStatus = tr("Down");
        break;
    case QtopiaNetworkInterface::Up:
        interfaceStatus = tr("Up");
        break;
    case QtopiaNetworkInterface::Pending:
        interfaceStatus = tr("Pending");
        break;
    case QtopiaNetworkInterface::Demand:
        interfaceStatus = tr("On Demand");
        break;
    case QtopiaNetworkInterface::Unavailable:
        interfaceStatus = tr("Unavailable");
        break;
    };

    QLabel* istatus;
    istatus = new QLabel(tr("Status: ") + interfaceStatus);
    vb->addWidget( istatus );

    QLabel* mac;
    mac = new QLabel(tr("MAC: ") + interface.hardwareAddress());
    vb->addWidget( mac );

    QList<QNetworkAddressEntry> addressEntries = interface.addressEntries();
    QLabel* ipLabel;
    QLabel* broadcastLabel;
    QLabel* netmaskLabel;


    for(int j = 0; j < addressEntries.size(); ++j) {
        ipLabel = new QLabel(tr("IP: ") +  addressEntries.at(j).ip().toString());
        broadcastLabel = new QLabel(tr("Broadcast: ") + addressEntries.at(j).broadcast().toString());
        netmaskLabel = new QLabel(tr("Netmask: ") + addressEntries.at(j).netmask().toString());

        vb->addWidget( ipLabel );
        vb->addWidget( broadcastLabel );
        vb->addWidget( netmaskLabel );
    }
}

NetworkDeviceInfo::~NetworkDeviceInfo()
{
}

