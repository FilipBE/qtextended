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

#ifndef QMODEMINDICATORS_H
#define QMODEMINDICATORS_H

#include <qmodemservice.h>

class QModemIndicatorsPrivate;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemIndicators : public QObject
{
    Q_OBJECT
public:
    explicit QModemIndicators( QModemService *service );
    ~QModemIndicators();

    enum ChargeState
    {
        PoweredByBattery,
        NotPoweredByBattery,
        NoBattery,
        PowerFault
    };

    enum SmsMemoryFullStatus
    {
        SmsMemoryOK,
        SmsMemoryFull,
        SmsMemoryMessageRejected
    };

public slots:
    void setSignalQuality( int value, int maxValue );
    void setBatteryCharge( int value, int maxValue,
                           QModemIndicators::ChargeState state );
    void setSmsMemoryFull( QModemIndicators::SmsMemoryFullStatus value );
    void setNetworkTimeZone( int zone, int dst );
    void setNetworkTime( uint time, int zone, int dst );

private slots:
    void resetModem();
    void csq( bool ok, const QAtResult& result );
    void csqn( const QString& msg );
    void cbc( bool ok, const QAtResult& result );
    void cbcn( const QString& msg );
    void ciev( const QString& msg );
    void cind( bool ok, const QAtResult& result );
    void ctzv( const QString& msg );
    void pollSignalQuality();
    void pollBatteryCharge();

private:
    QModemIndicatorsPrivate *d;

    void setSignalQualityInternal( int value, int maxValue );
    void setBatteryChargeInternal( int value, int maxValue,
                                   QModemIndicators::ChargeState state );
};

#endif
