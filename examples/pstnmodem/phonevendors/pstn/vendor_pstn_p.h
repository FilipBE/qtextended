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

#ifndef VENDOR_PSTN_P_H
#define VENDOR_PSTN_P_H

#include <qmodemservice.h>
#include <qmodemcall.h>
#include <qmodemcallprovider.h>
#include <qsiminfo.h>
#include <qpinmanager.h>
#include <qphonerffunctionality.h>
#include <qnetworkregistration.h>
#include <qmodemservicenumbers.h>
#include <qphonebook.h>
#include <qtelephonyconfiguration.h>

class PstnModemService;

class PstnCallProvider : public QModemCallProvider
{
    Q_OBJECT
public:
    PstnCallProvider( PstnModemService *service );
    ~PstnCallProvider();

protected:
    QModemCallProvider::AtdBehavior atdBehavior() const;
    void abortDial( uint modemIdentifier, QPhoneCall::Scope scope );
    QString releaseCallCommand( uint modemIdentifier ) const;
    QString releaseActiveCallsCommand() const;
    QString releaseHeldCallsCommand() const;
    QString putOnHoldCommand() const;
    QString setBusyCommand() const;
    QString activateCallCommand
	( uint modemIdentifier, bool otherActiveCalls ) const;
    QString activateHeldCallsCommand() const;
    QString joinCallsCommand( bool detachSubscriber ) const;
    QString deflectCallCommand( const QString& number ) const;

protected slots:
    void resetModem();

private:
    PstnModemService *service;
};

class PstnSimInfo : public QSimInfo
{
    Q_OBJECT
public:
    PstnSimInfo( PstnModemService *service );
    ~PstnSimInfo();
};

class PstnPinManager : public QPinManager
{
    Q_OBJECT
public:
    PstnPinManager( PstnModemService *service );
    ~PstnPinManager();

public slots:
    void querySimPinStatus();
    void enterPin( const QString& type, const QString& pin );
    void enterPuk( const QString& type, const QString& puk,
                   const QString& newPin );
    void cancelPin( const QString& type );
    void changePin( const QString& type, const QString& oldPin,
                    const QString& newPin );

private:
    PstnModemService *service;
    bool readySent;
};

class PstnRfFunctionality : public QPhoneRfFunctionality
{
    Q_OBJECT
public:
    PstnRfFunctionality( const QString& service, QObject *parent );
    ~PstnRfFunctionality();

public slots:
    void forceLevelRequest();
    void setLevel( QPhoneRfFunctionality::Level level );
};

class PstnNetworkRegistration : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    PstnNetworkRegistration( const QString& service, QObject *parent );
    ~PstnNetworkRegistration();

public slots:
    void setCurrentOperator( QTelephony::OperatorMode mode,
                             const QString& id, const QString& technology );
    void requestAvailableOperators();

private slots:
    void initDone();
};

class PstnPhoneBook : public QPhoneBook
{
    Q_OBJECT
public:
    PstnPhoneBook( const QString& service, QObject *parent );
    ~PstnPhoneBook();

public slots:
    void getEntries( const QString& store );
    void add( const QPhoneBookEntry& entry, const QString& store, bool flush );
    void remove( uint index, const QString& store, bool flush );
    void update( const QPhoneBookEntry& entry, const QString& store, bool flush );
    void flush( const QString& store );
    void setPassword( const QString& store, const QString& passwd );
    void clearPassword( const QString& store );
    void requestLimits( const QString& store );
    void requestFixedDialingState();
    void setFixedDialingState( bool enabled, const QString& pin2 );

private:
    QList<QPhoneBookEntry> ents;
    bool fixedDialingEnabled;
};

class PstnServiceNumbers : public QModemServiceNumbers
{
    Q_OBJECT
public:
    explicit PstnServiceNumbers( QModemService *service );
    ~PstnServiceNumbers();

public slots:
    void requestServiceNumber( QServiceNumbers::NumberId id );
    void setServiceNumber
            ( QServiceNumbers::NumberId id, const QString& number );
};

class PstnConfiguration : public QTelephonyConfiguration
{
    Q_OBJECT
public:
    explicit PstnConfiguration( PstnModemService *service );
    ~PstnConfiguration();

public slots:
    void update( const QString& name, const QString& value );
    void request( const QString& name );

private slots:
    void gmi( bool ok, const QAtResult& result );
    void gmm( bool ok, const QAtResult& result );
    void gmr( bool ok, const QAtResult& result );
    void gsn( bool ok, const QAtResult& result );
    void extra( bool ok, const QAtResult& result );

private:
    PstnModemService *service;
};

class PstnModemService : public QModemService
{
    Q_OBJECT
public:
    PstnModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent = 0 );
    ~PstnModemService();

    void initialize();

    enum ModemType
    {
	V253,
	Rockwell
    };

    ModemType modemType() const { return type; }

private slots:
    void reset();
    void mfr( bool ok );

private:
    ModemType type;
};

#endif
