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

#ifndef QVPNFACTORY_H
#define QVPNFACTORY_H

#include "qvpnclient.h"

#include <QSet>

class QTOPIACOMM_EXPORT QVPNFactory {
public:
    QVPNFactory();

    QVPNClient* instance( uint vpnID,  QObject* parent = 0 );
    QVPNClient* create( QVPNClient::Type type, QObject* parent = 0 );

    static QSet<QVPNClient::Type> types();
    static QString name( uint vpnID );
    static QSet<uint> vpnIDs();
protected:
    void setServerMode( bool enable );

private:
    bool serverMode;

    friend class QtopiaVpnManager;
};

#endif
