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

#ifndef QMODEMADVICEOFCHARGE_H
#define QMODEMADVICEOFCHARGE_H

#include <qadviceofcharge.h>

class QModemService;
class QModemAdviceOfChargePrivate;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemAdviceOfCharge : public QAdviceOfCharge
{
    Q_OBJECT
public:
    explicit QModemAdviceOfCharge( QModemService *service );
    ~QModemAdviceOfCharge();

public slots:
    void requestCurrentCallMeter();
    void requestAccumulatedCallMeter();
    void requestAccumulatedCallMeterMaximum();
    void requestPricePerUnit();
    void resetAccumulatedCallMeter( const QString& password );
    void setAccumulatedCallMeterMaximum( int value, const QString& password );
    void setPricePerUnit
        ( const QString& currency, const QString& unitPrice,
          const QString& password );

private slots:
    void resetModem();
    void caoc( bool ok, const QAtResult& result );
    void cccm( const QString& msg );
    void cacm( bool ok, const QAtResult& result );
    void cacmSet( bool ok, const QAtResult& result );
    void camm( bool ok, const QAtResult& result );
    void cammSet( bool ok, const QAtResult& result );
    void cpuc( bool ok, const QAtResult& result );
    void cpucSet( bool ok, const QAtResult& result );

private:
    QModemAdviceOfChargePrivate *d;
};

#endif
