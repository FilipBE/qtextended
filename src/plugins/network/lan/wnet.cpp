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

#include "wnet.h"
#include <QDebug>

/*!
  \internal
  \class WirelessNetwork
  \inpublicgroup QtConnectivityModule
  \brief This class is a small container class for wireless LAN parameter.
  */

WirelessNetwork::WirelessNetwork()
{
    dataMap.clear();
    custom.clear();
}

WirelessNetwork::~WirelessNetwork()
{
}

void WirelessNetwork::setData( ParameterType t, const QVariant& data )
{
    dataMap.insert( (int)t, data );
}

QVariant WirelessNetwork::data( ParameterType t ) const
{
    return dataMap.value( (int)t );
}

void WirelessNetwork::addCustomData( const QVariant& data )
{
    custom.append( data );
}

QList<QVariant> WirelessNetwork::customData() const
{
    return custom;
}

bool WirelessNetwork::isValid() const
{
    return ( dataMap.size() > 0 );
}

/*
void WirelessNetwork::dump() const
{
    QList<int> keys = dataMap.keys();
    foreach( int s, keys )
    {
        qLog(Network) << s << dataMap[s];
    }
    foreach( QVariant a, custom )
        qLog(Network) << a;
}*/
