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

#ifndef PHONESERVERDUMMYMODEM_H
#define PHONESERVERDUMMYMODEM_H

#include <qtelephonyservice.h>
#include <qphonecallprovider.h>
#include <qphonebook.h>
#include <qnetworkregistration.h>
#include <qsiminfo.h>
#include <qphonerffunctionality.h>
#include <qpinmanager.h>

class QPhoneCallDummy : public QPhoneCallImpl
{
    Q_OBJECT
public:
    QPhoneCallDummy( QPhoneCallProvider *provider, const QString& identifier,
                     const QString& callType );
    ~QPhoneCallDummy();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void join( bool detachSubscriber );
    void tone( const QString& tones );
    void transfer( const QString& number );

private slots:
    void dialTimeout();
};

class QPhoneCallProviderDummy : public QPhoneCallProvider
{
    Q_OBJECT
public:
    QPhoneCallProviderDummy( const QString& service, QObject *parent );
    ~QPhoneCallProviderDummy();

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );
};


class QPhoneLineDummy;


class QPhoneBookDummy : public QPhoneBook
{
    Q_OBJECT
public:
    QPhoneBookDummy( const QString& service, QObject *parent );
    ~QPhoneBookDummy();

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

class QNetworkRegistrationDummy : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    QNetworkRegistrationDummy( const QString& service, QObject *parent );
    ~QNetworkRegistrationDummy();

public slots:
    void setCurrentOperator( QTelephony::OperatorMode mode,
                             const QString& id, const QString& technology );
    void requestAvailableOperators();

private slots:
    void searching();
    void home();
    void initDone();
};

class QSimInfoDummy : public QSimInfo
{
    Q_OBJECT
public:
    QSimInfoDummy( const QString& service, QObject *parent );
    ~QSimInfoDummy();
};

class QPhoneRfFunctionalityDummy : public QPhoneRfFunctionality
{
    Q_OBJECT
public:
    QPhoneRfFunctionalityDummy( const QString& service, QObject *parent );
    ~QPhoneRfFunctionalityDummy();

public slots:
    void forceLevelRequest();
    void setLevel( QPhoneRfFunctionality::Level level );
};

class QPinManagerDummy : public QPinManager
{
    Q_OBJECT
public:
    QPinManagerDummy( const QString& service, QObject *parent );
    ~QPinManagerDummy();

public slots:
    void querySimPinStatus();
    void enterPin( const QString& type, const QString& pin );
    void enterPuk( const QString& type, const QString& puk,
                   const QString& newPin );
    void cancelPin( const QString& type );
    void changePin( const QString& type, const QString& oldPin,
                    const QString& newPin );
};

class QTelephonyServiceDummy : public QTelephonyService
{
    Q_OBJECT
public:
    QTelephonyServiceDummy( const QString& service, QObject *parent = 0 );
    ~QTelephonyServiceDummy();

    void initialize();
};

#endif
