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

#ifndef ATINDICATORS_H
#define ATINDICATORS_H

#include <qobject.h>
#include "atcallmanager.h"

class AtIndicatorsPrivate;

class AtIndicators : public QObject
{
    Q_OBJECT
public:
    AtIndicators( QObject *parent );
    ~AtIndicators();

    int addIndicator( const QString& name, int maxValue, int initValue = 0 );
    QStringList indicators() const;
    int indicator( const QString& name ) const;
    int numIndicators() const;
    QString name( int ind ) const;

    int value( int ind ) const;
    int value( const QString& name ) const;
    int maxValue( int ind ) const;
    void setValue( int ind, int value );
    void setValue( const QString& name, int value );

signals:
    void indicatorChanged( int ind, int value );

private:
    AtIndicatorsPrivate *d;
};

class AtPhoneIndicatorsPrivate;

class AtPhoneIndicators : public AtIndicators
{
    Q_OBJECT
public:
    AtPhoneIndicators( QObject *parent );
    ~AtPhoneIndicators();

    int signalQuality() const;
    int batteryCharge() const;

public slots:
    void setOnCall( bool value );
    void setCallSetup( AtCallManager::CallSetup callSetup );
    void setCallHold( AtCallManager::CallHoldState callHold );

signals:
    void signalQualityChanged( int value );
    void batteryChargeChanged( int value );

private slots:
    void updateRegistrationState();
    void updateSignalQuality();
    void updateBatteryCharge();
#ifdef QTOPIA_CELL
    void unreadCountChanged();
    void updateSmsMemoryFull();
#endif

private:
    AtPhoneIndicatorsPrivate *d;
};

#endif
