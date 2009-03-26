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

#ifndef QADVICEOFCHARGE_H
#define QADVICEOFCHARGE_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QAdviceOfCharge : public QCommInterface
{
    Q_OBJECT
public:
    explicit QAdviceOfCharge( const QString& service = QString(),
                              QObject *parent = 0,
                              QCommInterface::Mode mode = Client );
    ~QAdviceOfCharge();

public slots:
    virtual void requestCurrentCallMeter();
    virtual void requestAccumulatedCallMeter();
    virtual void requestAccumulatedCallMeterMaximum();
    virtual void requestPricePerUnit();
    virtual void resetAccumulatedCallMeter( const QString& password );
    virtual void setAccumulatedCallMeterMaximum
        ( int value, const QString& password );
    virtual void setPricePerUnit
        ( const QString& currency, const QString& unitPrice,
          const QString& password );

signals:
    void currentCallMeter( int value, bool explicitRequest );
    void accumulatedCallMeter( int value );
    void accumulatedCallMeterMaximum( int value );
    void callMeterMaximumWarning();
    void pricePerUnit( const QString& currency, const QString& unitPrice );
    void resetAccumulatedCallMeterResult( QTelephony::Result result );
    void setAccumulatedCallMeterMaximumResult( QTelephony::Result result );
    void setPricePerUnitResult( QTelephony::Result result );
};

#endif
