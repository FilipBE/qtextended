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

#ifndef VENDOR_WAVECOM_P_H
#define VENDOR_WAVECOM_P_H

#include <qmodemservice.h>
#include <qmodemcall.h>
#include <qmodemcallprovider.h>
#include <qmodemsimtoolkit.h>
#include <qmodemphonebook.h>
#include <qmodemsiminfo.h>
#include <qmodemrffunctionality.h>
#include <qmodemconfiguration.h>
#include <QTimer>

class QAtResultParser;
class WavecomModemService;

class WavecomCallProvider : public QModemCallProvider
{
    Q_OBJECT
public:
    WavecomCallProvider( WavecomModemService *service );
    ~WavecomCallProvider();

protected:
    QModemCallProvider::AtdBehavior atdBehavior() const;
    void abortDial( uint modemIdentifier, QPhoneCall::Scope scope );

private slots:
    void callReleased( uint index );
    void callingPartyAlerting();
};

class WavecomSimToolkit : public QModemSimToolkit
{
    Q_OBJECT
public:
    WavecomSimToolkit( QModemService *service );
    ~WavecomSimToolkit();

public slots:
    void initialize();
    void begin();
    void end();
    void sendResponse( const QSimTerminalResponse& resp );
    void sendEnvelope( const QSimEnvelope& env );

private slots:
    void stinNotification( const QString& msg );
    void configureDone( bool ok );
    void activateDone( bool ok );
    void cfunQueryDone( bool ok, const QAtResult& result );
    void getCommandDone( bool ok, const QAtResult& result );
    void getCommandTimeout();

private:
    uint inputFormat;
    QSimCommand lastCommand;
    int pendingCommand;
    bool supportsStk;
    bool seenBegin;
    bool awaitingCommand;
    QTimer *getTimer;
    uint getTimeout;

    void readMenu( QAtResultParser& cmd, QSimCommand& scmd );
    QAtChat *atchat() const { return service()->primaryAtChat(); }
};

class WavecomPhoneBook : public QModemPhoneBook
{
    Q_OBJECT
public:
    WavecomPhoneBook( WavecomModemService *service );
    ~WavecomPhoneBook();

protected:
    bool hasModemPhoneBookCache() const;
};

class WavecomSimInfo : public QModemSimInfo
{
    Q_OBJECT
public:
    WavecomSimInfo( WavecomModemService *service );
    ~WavecomSimInfo();
};

class WavecomRfFunctionality : public QModemRfFunctionality
{
    Q_OBJECT
public:
    WavecomRfFunctionality( WavecomModemService *service );
    ~WavecomRfFunctionality();

public slots:
    void forceLevelRequest();
    void setLevel( QPhoneRfFunctionality::Level level );

private slots:
    void cfun( bool ok, const QAtResult& result );
    void cfunSet( bool ok, const QAtResult& result );

private:
    WavecomModemService *service;
    bool planeMode;
};

class WavecomConfiguration : public QModemConfiguration
{
    Q_OBJECT
public:
    explicit WavecomConfiguration( WavecomModemService *service );
    ~WavecomConfiguration();

public slots:
    void request( const QString& name );

private slots:
    void whwv( bool ok, const QAtResult& result );
    void wssv( bool ok, const QAtResult& result );
    void wdop( bool ok, const QAtResult& result );

private:
    WavecomModemService *service;
    QString info;
};

class WavecomModemService : public QModemService
{
    Q_OBJECT
public:
    WavecomModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent = 0 );
    ~WavecomModemService();

    void initialize();

private slots:
    void windNotification( const QString& msg );

signals:
    void simRemoved();                  // +WIND: 0
    void simInserted();                 // +WIND: 1
    void callingPartyAlerting();        // +WIND: 2
    void basicCommandsReady();          // +WIND: 3
    void allCommandsReady();            // +WIND: 4
    void callCreated( uint index );     // +WIND: 5,<index>
    void callReleased( uint index );    // +WIND: 6,<index>
    void emergencyCallsPossible();      // +WIND: 7
    void networkLost();                 // +WIND: 8
    void audioOn();                     // +WIND: 9
    void phoneBooksReady();             // +WIND: 10
};

#endif
