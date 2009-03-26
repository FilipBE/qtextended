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

#ifndef QMODEMGPRSNETWORKREGISTRATION_H
#define QMODEMGPRSNETWORKREGISTRATION_H

#include <qgprsnetworkregistration.h>

class QModemGprsNetworkRegistrationPrivate;
class QAtResult;
class QModemService;

class QTOPIAPHONEMODEM_EXPORT QModemGprsNetworkRegistration
        : public QGprsNetworkRegistrationServer
{
    Q_OBJECT
public:
    explicit QModemGprsNetworkRegistration( QModemService *service );
    ~QModemGprsNetworkRegistration();

private slots:
    void resetModem();
    void cgregQuery( bool ok, const QAtResult& result );
    void cgregNotify( const QString& msg );

private:
    QModemGprsNetworkRegistrationPrivate *d;
};

#endif
