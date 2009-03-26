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

#ifndef VENDOR_NEO_P_H
#define VENDOR_NEO_P_H

#include <qmodemservice.h>
#include <qmodemcall.h>
#include <qmodemcallprovider.h>
#include <qmodemsimtoolkit.h>
#include <qmodemphonebook.h>
#include <qmodempinmanager.h>
#include <qmodempreferrednetworkoperators.h>
#include <qbandselection.h>
#include <qvibrateaccessory.h>
#include <qmodemnetworkregistration.h>
#include <qmodemsiminfo.h>
#include <qmodemcallvolume.h>

#include <alsa/asoundlib.h>

class NeoCallProvider : public QModemCallProvider
{
    Q_OBJECT
public:
    NeoCallProvider( QModemService *service );
    ~NeoCallProvider();

protected:
    QModemCallProvider::AtdBehavior atdBehavior() const;
    void abortDial( uint modemIdentifier, QPhoneCall::Scope scope );
    QModemService *modemService;

    QString dialVoiceCommand(const QDialOptions& options) const;

private slots:
    void cpiNotification( const QString& msg );
    void cnapNotification( const QString& msg );
};

class NeoSimToolkit : public QModemSimToolkit
{
    Q_OBJECT
public:
    NeoSimToolkit( QModemService *service );
    ~NeoSimToolkit();

public slots:
    void initialize();
    void begin();
    void sendResponse( const QSimTerminalResponse& resp );
    void sendEnvelope( const QSimEnvelope& env );

private slots:
    void sataNotification( const QString& msg );
    void satnNotification( const QString& msg );

private:
    QSimCommand lastCommand;
    QByteArray lastCommandBytes;
    QSimCommand mainMenu;
    QByteArray mainMenuBytes;
    bool lastResponseWasExit;
};

class NeoPhoneBook : public QModemPhoneBook
{
    Q_OBJECT
public:
    NeoPhoneBook( QModemService *service );
    ~NeoPhoneBook();

protected:
    bool hasModemPhoneBookCache() const;
    bool hasEmptyPhoneBookIndex() const;

private slots:
    void cstatNotification( const QString& msg );

private:
    bool m_phoneBookIsReady;
    bool m_smsIsReady;    
    QModemService *service;
};

class NeoPinManager : public QModemPinManager
{
    Q_OBJECT
public:
    NeoPinManager( QModemService *service );
    ~NeoPinManager();

protected:
    bool emptyPinIsReady() const;
};

class NeoBandSelection : public QBandSelection
{
    Q_OBJECT
public:
    NeoBandSelection( QModemService *service );
    ~NeoBandSelection();

public slots:
    void requestBand();
    void requestBands();
    void setBand( QBandSelection::BandMode mode, const QString& value );

private slots:
    void bandQuery( bool ok, const QAtResult& result );
    void bandList( bool ok, const QAtResult& result );
    void bandSet( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

 class NeoModemNetworkRegistration : public QModemNetworkRegistration
 {
         Q_OBJECT
             public:
     explicit NeoModemNetworkRegistration( QModemService *service );

 protected:
      virtual QString setCurrentOperatorCommand
          ( QTelephony::OperatorMode mode, const QString& id,
            const QString& technology );
 };

class NeoModemService : public QModemService
{
    Q_OBJECT
public:
    NeoModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent = 0 );
    ~NeoModemService();

    void initialize();

private:
    NeoModemNetworkRegistration *neoNetRego;

private slots:
    void csq( const QString& msg );
    void firstCsqQuery();

    void ctzu( const QString& msg );
    void configureDone( bool ok );
    void reset();
    void suspend();
    void wake();
    void sendSuspendDone();
    void mcsqOff();
    void mcsqOn();
    void sendRego();
};

class  NeoVibrateAccessory : public QVibrateAccessoryProvider
{
    Q_OBJECT
public:
     NeoVibrateAccessory( QModemService *service );
    ~NeoVibrateAccessory();

public slots:
    void setVibrateNow( const bool value );
    void setVibrateOnRing( const bool value );
};

class NeoCallVolume : public QModemCallVolume
{
    Q_OBJECT
public:
    explicit NeoCallVolume( NeoModemService *service);
    ~NeoCallVolume();

public slots:
    void setSpeakerVolume( int volume );
    void setMicrophoneVolume( int volume );
    void setSpeakerVolumeRange(int,int);
    void setMicVolumeRange(int,int);

protected:
    bool hasDelayedInit() const;

private:
    NeoModemService *service;
    bool regoSent;
};

class NeoSimInfoPrivate;

class NeoSimInfo : public QSimInfo
{
    Q_OBJECT
public:
    NeoSimInfo(NeoModemService *service );
    ~NeoSimInfo();

protected slots:
    void simInserted();
    void simRemoved();

private slots:
    void requestIdentity();
    void cimi( bool ok, const QAtResult& result );
    void serviceItemPosted( const QString& item );

private:
    NeoSimInfoPrivate *d;

    static QString extractIdentity( const QString& content );
};

class NeoPreferredNetworkOperators : public QModemPreferredNetworkOperators
{
    Q_OBJECT
public:
    explicit NeoPreferredNetworkOperators( QModemService *service );
    ~NeoPreferredNetworkOperators();
};

 
#endif
