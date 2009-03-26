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

#ifndef VENDOR_GREENPHONE_P_H
#define VENDOR_GREENPHONE_P_H

#include <qmodemsimtoolkit.h>
#include <qmodemservice.h>
#include <qmodemcall.h>
#include <qmodemcallprovider.h>
#include <qmodemphonebook.h>
#include <qmodemsiminfo.h>
#include <qmodemsimfiles.h>
#include <qmodemservicenumbers.h>
#include <qmodemcellbroadcast.h>
#include <qmodemconfiguration.h>
#include <qmodemcallvolume.h>
#include <qbandselection.h>
#include <qvibrateaccessory.h>

class GreenphoneModemService;

class GreenphoneCall : public QModemCall
{
    Q_OBJECT
public:
    GreenphoneCall( QModemCallProvider *provider, const QString& identifier,
                    const QString& callType );
    ~GreenphoneCall();

    void tone( const QString& tones );
};

class GreenphoneCallProvider : public QModemCallProvider
{
    Q_OBJECT
public:
    GreenphoneCallProvider( GreenphoneModemService *service );
    ~GreenphoneCallProvider();

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );
    QModemCallProvider::AtdBehavior atdBehavior() const;
    void abortDial( uint modemIdentifier, QPhoneCall::Scope scope );
    QString setBusyCommand() const;

protected slots:
    void resetModem();

private slots:
    void allCommandsReady();
    void simChanged();
    void mcamNotification( const QString& msg );
    void callsChanged();

private:
    GreenphoneModemService *service;
    uint callingId;
    uint alertingId;
    uint abortingId;
    bool sawMrdy3;
    bool needClipSet;
    bool prevActiveCall;

    void doAbortDial();
};

class GreenphoneSimToolkit : public QModemSimToolkit
{
    Q_OBJECT
public:
    GreenphoneSimToolkit( GreenphoneModemService *service );
    ~GreenphoneSimToolkit();

protected:
    void groupInitialized( QAbstractIpcInterfaceGroup *group );

public slots:
    void initialize();
    void begin();
    void end();
    void sendResponse( const QSimTerminalResponse& resp );
    void sendEnvelope( const QSimEnvelope& env );

private slots:
    void simInserted();
    void simRemoved();
    void fetchMenu();
    void fetchMenuIfNecessary();
    void mtsmenu( const QString& msg );
    void mstkev();
    void menuItem( const QString& msg );
    void menuDone();
    void mtmenu( bool ok );
    void mtdisp( const QString& msg );
    void mtdispData( const QString& msg );
    void mtkey( const QString& msg );
    void mtkeyData( const QString& msg );
    void mtgin( const QString& msg );
    void mtginData( const QString& msg );
    void mtginDefaultInput( const QString& msg );
    void mtkeygin( const QString& data, const QString& defaultInput );
    void mttone( const QString& msg );
    void mtrsh( const QString& msg );
    void mtrshEFList( const QString& msg );
    void mtsms( const QString& msg );
    void mtss( const QString& msg );
    void mtussd( const QString& msg );
    void mtdtmf( const QString& msg );
    void mtcall( const QString& msg );
    void mtitxt( const QString& msg );
    void mtitxtData( const QString& msg );
    void mtlang();
    void mtlangnt( const QString& msg );
    void mtlbr( const QString& msg );
    void mtlbrText( const QString& msg );
    void mtlbrAlpha( const QString& msg );
    void mtrunat( const QString& msg );
    void mtchevt( const QString& msg );
    void mtstkcc( const QString& msg );

private:
    uint registeredMenuItems;
    uint expectedMenuItems;
    QSimCommand::Type menuType;
    QList<QSimMenuItem> items;
    QSimCommand lastCommand;
    QSimCommand mainMenu;
    int responseType;
    bool unicodeStrings;
    bool hasHelp;
    bool fetchingMainMenu;
    QString firstLine;
    QString secondLine;

    QString decodeString( const QString& value );
    void parseIconInfo( QSimCommand& cmd, const QString& msg, uint& posn );
    void clearLastCommand();
    void clearMainMenu();
    QAtChat *atchat() const { return service()->primaryAtChat(); }
    void emitCommand( int responseType, const QSimCommand& cmd );
    void emitCommandNoResponse( const QSimCommand& cmd );
};

class GreenphonePhoneBook : public QModemPhoneBook
{
    Q_OBJECT
public:
    GreenphonePhoneBook( GreenphoneModemService *service );
    ~GreenphonePhoneBook();

protected:
    bool hasModemPhoneBookCache() const;
};

class GreenphoneSimInfo : public QModemSimInfo
{
    Q_OBJECT
public:
    GreenphoneSimInfo( GreenphoneModemService *service );
    ~GreenphoneSimInfo();
};

class GreenphoneSimFiles : public QModemSimFiles
{
    Q_OBJECT
public:
    GreenphoneSimFiles( GreenphoneModemService *service );
    ~GreenphoneSimFiles();

protected:
    bool useCSIM() const;
};

class GreenphoneBandSelection : public QBandSelection
{
    Q_OBJECT
public:
    GreenphoneBandSelection( GreenphoneModemService *service );
    ~GreenphoneBandSelection();

public slots:
    void requestBand();
    void requestBands();
    void setBand( QBandSelection::BandMode mode, const QString& value );

private slots:
    void mbsel( bool ok, const QAtResult& result );
    void mbselSet( bool ok, const QAtResult& result );

private:
    GreenphoneModemService *service;
};

class GreenphoneVibrateAccessory : public QVibrateAccessoryProvider
{
    Q_OBJECT
public:
    GreenphoneVibrateAccessory( QModemService *service );
    ~GreenphoneVibrateAccessory();

public slots:
    void setVibrateNow( const bool value );
};

class GreenphoneServiceNumbers : public QModemServiceNumbers
{
    Q_OBJECT
public:
    GreenphoneServiceNumbers( QModemService *service );
    ~GreenphoneServiceNumbers();

public slots:
    void requestServiceNumber( QServiceNumbers::NumberId id );
    void setServiceNumber
            ( QServiceNumbers::NumberId id, const QString& number );

private slots:
    void vmQueryDone( bool ok, const QAtResult& result );
    void vmModifyDone( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

class GreenphoneCellBroadcast : public QModemCellBroadcast
{
    Q_OBJECT
public:
    GreenphoneCellBroadcast( QModemService *service );
    ~GreenphoneCellBroadcast();

    bool isRegistered() const;
    void suspend();
    void resume();

protected:
    bool removeBeforeChange() const;
    bool removeOneByOne() const;

private:
    QModemService *service;
};

class GreenphoneConfiguration : public QModemConfiguration
{
    Q_OBJECT
public:
    explicit GreenphoneConfiguration( GreenphoneModemService *service );
    ~GreenphoneConfiguration();

public slots:
    void request( const QString& name );

private slots:
    void mvers( bool ok, const QAtResult& result );

private:
    GreenphoneModemService *service;
};

class GreenphoneCallVolume : public QModemCallVolume
{
    Q_OBJECT
public:
    explicit GreenphoneCallVolume( GreenphoneModemService *service );
    ~GreenphoneCallVolume();

protected:
    bool hasDelayedInit() const;

private:
    GreenphoneModemService *service;
};

class GreenphoneModemService : public QModemService
{
    Q_OBJECT
public:
    GreenphoneModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent = 0 );
    ~GreenphoneModemService();

    void initialize();

private slots:
    void mrdyNotification( const QString& msg );
    void mcsqNotification( const QString& msg );
    void mtz2Notification( const QString& msg );
    void reset();
    void setOutput(int output);
    void setEchoCancellation(int val);
    void setNoiseSuppression(int val);
    void sendSuspendDone();
    void mcsqOff();
    void mcsqOn();
    void cmsError322();
    void cmgfDone( bool ok );
    void cpmsDone( bool ok );
    void sendCMGF();
    void sendCPMS();

protected slots:
    void needSms();
    void suspend();
    void wake();

signals:
    void moduleIsReady();               // +MRDY: 1
    void emergencyCallsPossible();      // +MRDY: 2
    void allCommandsReady();            // +MRDY: 3
    void simInserted();                 // +MRDY: 4
    void simRemoved();                  // +MRDY: 5
    void noNetworkService();            // +MRDY: 6
    void emergencyCallsOnly();          // +MRDY: 7

private:
    int smsRetryCount;
    GreenphoneCellBroadcast *cb;
};

#endif
