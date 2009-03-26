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

#ifndef WNET_H
#define WNET_H

#include <QVariant>
#include <QHash>

class WirelessNetwork
{
public:
    WirelessNetwork();
    ~WirelessNetwork();

    enum ParameterType {
        Protocol = 0,   //SIOCGIWNAME
        AP,             //SIOCGIWAP
        ESSID,          //SIOCGIWESSID
        Mode,           //SIOCGIWMODE
        NWID,           //SIOCGIWNWID
        BitRate,        //SIOCGIWRATE
        Frequency,      //SIOCGIWFREQ
        Channel,        //based on SIOCGIWFREQ
        Encoding,       //SIOCGIWENCODE
        Security,       //based on SIOCGIENCODE
        Quality,        //IWEVQUAL
        Signal,         //IWEVQUAL
        Noise,          //IWEVQUAL
        Custom,         //IWEVCUSTOM
    };

    //void dump() const;
    bool isValid() const;
    void setData( ParameterType t, const QVariant& data );
    QVariant data( ParameterType t ) const;

    void addCustomData( const QVariant& data );
    QList<QVariant> customData() const;

private:
    QHash<int,QVariant> dataMap;
    QList<QVariant> custom;
};
#endif
