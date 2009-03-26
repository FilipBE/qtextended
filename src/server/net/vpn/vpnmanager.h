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

#ifndef VPNMANAGER_H
#define VPNMANAGER_H

#include <qtopiaipcadaptor.h>
#include <QHash>

class QVPNClient;
class QVPNFactory;

class QtopiaVpnManager : public QtopiaIpcAdaptor {
    Q_OBJECT
public:
    explicit QtopiaVpnManager( QObject* parent = 0 );
    ~QtopiaVpnManager();

public slots:
    void connectVPN( uint vpnID );
    void disconnectVPN( uint vpnID );
    void deleteVPN( uint vpnID ) ;

private:
    QVPNFactory* vpnFactory;
    QHash<uint,QVPNClient*> idToVPN;
};

#endif
