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

#ifndef TST_QSIMTOOLKIT_H
#define TST_QSIMTOOLKIT_H

#include <QObject>
#include <QTest>
#include <QDebug>
#ifndef SYSTEMTEST
#include <qsimtoolkit.h>
#include <qsimfiles.h>
#else
#include <QSystemTest>
#include <qsimcommand.h>
#include <qsimterminalresponse.h>
#include <qsimenvelope.h>
#include <qsimcontrolevent.h>
#endif
#include "simapp.h"
#include "qfuturesignal.h"

class tst_QSimToolkitServer;
class tst_QSimToolkitFiles;
class TestPhoneCallProvider;

class QSimToolkitData
{
public:
    static void populateDataDisplayText();
    static void populateDataGetInkey();
    static void populateDataGetInput();
    static void populateDataMoreTime();
    static void populateDataPlayTone();
    static void populateDataPollInterval();
    static void populateDataRefresh();
    static void populateDataSetupMenu();
    static void populateDataSelectItem();
    static void populateDataSendSMS();
    static void populateDataSendSS();
    static void populateDataSendUSSD();
    static void populateDataSetupCall();
    static void populateDataPollingOff();
    static void populateDataProvideLocalInformation();
    static void populateDataSetupEventList();
    static void populateDataPerformCardAPDU();
    static void populateDataPowerOffCard();
    static void populateDataPowerOnCard();
    static void populateDataGetReaderStatus();
    static void populateDataTimerManagement();
    static void populateDataSetupIdleModeText();
    static void populateDataRunATCommand();
    static void populateDataSendDTMF();
    static void populateDataLanguageNotification();
    static void populateDataLaunchBrowser();
    static void populateDataOpenChannel();
    static void populateDataCloseChannel();
    static void populateDataReceiveData();
    static void populateDataSendData();
    static void populateDataGetChannelStatus();
    static void populateDataSmsPPDownload();
    static void populateDataSmsCBDownload();
    static void populateDataEventDownload();
    static void populateDataCallControlBySim();
    static void populateDataMoSmsControl();
};

#define InputQualifierNone              0x00
#define InputQualifierWantDigits        0x01
#define InputQualifierUcs2Input         0x02
#define InputQualifierWantYesNo         0x04
#define InputQualifierEcho              0x04
#define InputQualifierPackedInput       0x08
#define InputQualifierHasHelp           0x80

#define Open_OnDemand           0x00
#define Open_Immediate          0x01
#define Open_AutomaticReconnect 0x02

#define Send_Store          0
#define Send_Immediately    1

#define Timer_Start             0
#define Timer_Deactivate        1
#define Timer_GetCurrentValue   2

#ifndef SYSTEMTEST

class tst_QSimToolkit : public QObject
{
    Q_OBJECT
public:
    tst_QSimToolkit();

private slots:
    void initTestCase();
    void init();
    void testCommandCreate();

    // Test encoding and decoding of the individual SIM toolkit commands.
    void testEncodeDisplayText_data();
    void testEncodeDisplayText();
    void testEncodeGetInkey_data();
    void testEncodeGetInkey();
    void testEncodeGetInput_data();
    void testEncodeGetInput();
    void testEncodeMoreTime_data();
    void testEncodeMoreTime();
    void testEncodePlayTone_data();
    void testEncodePlayTone();
    void testEncodePollInterval_data();
    void testEncodePollInterval();
    void testEncodeRefresh_data();
    void testEncodeRefresh();
    void testEncodeSetupMenu_data();
    void testEncodeSetupMenu();
    void testEncodeSelectItem_data();
    void testEncodeSelectItem();
    void testEncodeSendSMS_data();
    void testEncodeSendSMS();
    void testEncodeSendSS_data();
    void testEncodeSendSS();
    void testEncodeSendUSSD_data();
    void testEncodeSendUSSD();
    void testEncodeSetupCall_data();
    void testEncodeSetupCall();
    void testEncodePollingOff_data();
    void testEncodePollingOff();
    void testEncodeProvideLocalInformation_data();
    void testEncodeProvideLocalInformation();
    void testEncodeSetupEventList_data();
    void testEncodeSetupEventList();
    void testEncodePerformCardAPDU_data();
    void testEncodePerformCardAPDU();
    void testEncodePowerOffCard_data();
    void testEncodePowerOffCard();
    void testEncodePowerOnCard_data();
    void testEncodePowerOnCard();
    void testEncodeGetReaderStatus_data();
    void testEncodeGetReaderStatus();
    void testEncodeTimerManagement_data();
    void testEncodeTimerManagement();
    void testEncodeSetupIdleModeText_data();
    void testEncodeSetupIdleModeText();
    void testEncodeRunATCommand_data();
    void testEncodeRunATCommand();
    void testEncodeSendDTMF_data();
    void testEncodeSendDTMF();
    void testEncodeLanguageNotification_data();
    void testEncodeLanguageNotification();
    void testEncodeLaunchBrowser_data();
    void testEncodeLaunchBrowser();
    void testEncodeOpenChannel_data();
    void testEncodeOpenChannel();
    void testEncodeCloseChannel_data();
    void testEncodeCloseChannel();
    void testEncodeReceiveData_data();
    void testEncodeReceiveData();
    void testEncodeSendData_data();
    void testEncodeSendData();
    void testEncodeGetChannelStatus_data();
    void testEncodeGetChannelStatus();
    void testEncodeSmsPPDownload_data();
    void testEncodeSmsPPDownload();
    void testEncodeSmsCBDownload_data();
    void testEncodeSmsCBDownload();
    void testEncodeEventDownload_data();
    void testEncodeEventDownload();
    void testEncodeCallControlBySim_data();
    void testEncodeCallControlBySim();
    void testEncodeMoSmsControl_data();
    void testEncodeMoSmsControl();

    // Test end to end delivery of SIM toolkit commands and responses via QSimToolkit.
    void testDeliverDisplayText_data();
    void testDeliverDisplayText();
    void testDeliverGetInkey_data();
    void testDeliverGetInkey();
    void testDeliverGetInputText_data();
    void testDeliverGetInputText();
    void testDeliverMoreTime_data();
    void testDeliverMoreTime();
    void testDeliverPlayTone_data();
    void testDeliverPlayTone();
    void testDeliverPollInterval_data();
    void testDeliverPollInterval();
    void testDeliverRefresh_data();
    void testDeliverRefresh();
    void testDeliverSetupMenu_data();
    void testDeliverSetupMenu();
    void testDeliverSelectItem_data();
    void testDeliverSelectItem();
    void testDeliverSendSMS_data();
    void testDeliverSendSMS();
    void testDeliverSendSS_data();
    void testDeliverSendSS();
    void testDeliverSendUSSD_data();
    void testDeliverSendUSSD();
    void testDeliverSetupCall_data();
    void testDeliverSetupCall();
    void testDeliverPollingOff_data();
    void testDeliverPollingOff();
    void testDeliverProvideLocalInformation_data();
    void testDeliverProvideLocalInformation();
    void testDeliverSetupEventList_data();
    void testDeliverSetupEventList();
    void testDeliverPerformCardAPDU_data();
    void testDeliverPerformCardAPDU();
    void testDeliverPowerOffCard_data();
    void testDeliverPowerOffCard();
    void testDeliverPowerOnCard_data();
    void testDeliverPowerOnCard();
    void testDeliverGetReaderStatus_data();
    void testDeliverGetReaderStatus();
    void testDeliverTimerManagement_data();
    void testDeliverTimerManagement();
    void testDeliverSetupIdleModeText_data();
    void testDeliverSetupIdleModeText();
    void testDeliverRunATCommand_data();
    void testDeliverRunATCommand();
    void testDeliverSendDTMF_data();
    void testDeliverSendDTMF();
    void testDeliverLanguageNotification_data();
    void testDeliverLanguageNotification();
    void testDeliverLaunchBrowser_data();
    void testDeliverLaunchBrowser();
    void testDeliverOpenChannel_data();
    void testDeliverOpenChannel();
    void testDeliverCloseChannel_data();
    void testDeliverCloseChannel();
    void testDeliverReceiveData_data();
    void testDeliverReceiveData();
    void testDeliverSendData_data();
    void testDeliverSendData();
    void testDeliverGetChannelStatus_data();
    void testDeliverGetChannelStatus();
    void testDeliverSmsPPDownload_data();
    void testDeliverSmsPPDownload();
    void testDeliverSmsCBDownload_data();
    void testDeliverSmsCBDownload();
    void testDeliverEventDownload_data();
    void testDeliverEventDownload();
    void testDeliverCallControlBySim_data();
    void testDeliverCallControlBySim();
    void testDeliverMoSmsControl_data();
    void testDeliverMoSmsControl();

    // Initialize "simapp" for the user interface tests.
    void initSimApp();

    // Test the user interface elements within "simapp".
    void testUIDisplayText_data();
    void testUIDisplayText();
    void testUISetupMenu_data();
    void testUISetupMenu();
    void testUIGetInkey_data();
    void testUIGetInkey();
    void testUIGetInput_data();
    void testUIGetInput();
    void testUIPlayTone_data();
    void testUIPlayTone();
    void testUISelectItem_data();
    void testUISelectItem();
    void testUISendSMS_data();
    void testUISendSMS();
    void testUISendSS_data();
    void testUISendSS();
    void testUISendUSSD_data();
    void testUISendUSSD();
    void testUISetupCall_data();
    void testUISetupCall();
    void testUIOpenChannel_data();
    void testUIOpenChannel();
    void testUISetupIdleModeText_data();
    void testUISetupIdleModeText();

    // Test the user interface elements within "simapp", with
    // the UI elements displayed outside of the menu structure.
    void testUINoMenuDisplayText_data() { testUIDisplayText_data(); }
    void testUINoMenuDisplayText() { nomenu = true; testUIDisplayText(); }
    void testUINoMenuGetInkey_data() { testUIGetInkey_data(); }
    void testUINoMenuGetInkey() { nomenu = true; testUIGetInkey(); }
    void testUINoMenuGetInput_data() { testUIGetInput_data(); }
    void testUINoMenuGetInput() { nomenu = true; testUIGetInput(); }
    void testUINoMenuPlayTone_data() { testUIPlayTone_data(); }
    void testUINoMenuPlayTone() { nomenu = true; testUIPlayTone(); }
    void testUINoMenuSelectItem_data() { testUISelectItem_data(); }
    void testUINoMenuSelectItem() { nomenu = true; testUISelectItem(); }
    void testUINoMenuSendSMS_data() { testUISendSMS_data(); }
    void testUINoMenuSendSMS() { nomenu = true; testUISendSMS(); }
    void testUINoMenuSendSS_data() { testUISendSS_data(); }
    void testUINoMenuSendSS() { nomenu = true; testUISendSS(); }
    void testUINoMenuSendUSSD_data() { testUISendUSSD_data(); }
    void testUINoMenuSendUSSD() { nomenu = true; testUISendUSSD(); }
    void testUINoMenuSetupCall_data() { testUISetupCall_data(); }
    void testUINoMenuSetupCall() { nomenu = true; testUISetupCall(); }
    void testUINoMenuOpenChannel_data() { testUIOpenChannel_data(); }
    void testUINoMenuOpenChannel() { nomenu = true; testUIOpenChannel(); }
    void testUINoMenuSetupIdleModeText_data() { testUISetupIdleModeText_data(); }
    void testUINoMenuSetupIdleModeText() { nomenu = true; testUISetupIdleModeText(); }

    // Shut down "simapp" as the user interface tests are over.
    void cleanupSimApp();

private:
    bool checkMenuItems( const QList<QSimMenuItem>& menuItems, const QString& expected,
                         bool hasHelp );
    QList<QSimMenuItem> parseMenuItems( const QString& expected, bool hasHelp );
    bool waitForView( const QMetaObject& meta, int timeout = 1000, bool mayHappenBefore = false );

    void keyClick( int key, const QString& text = QString() );
    void msleep( int msecs );
    void select();

    QSimToolkit *client;
    tst_QSimToolkitServer *server;
    tst_QSimToolkitFiles *files;
    TestPhoneCallProvider *phoneCallProvider;
    bool sawSignal;
    QEventLoop *innerLoop;
    QSimCommand deliveredCommand;
    QSimControlEvent deliveredEvent;
    SimApp *simapp;
    const QMetaObject *expectedView;
    SimCommandView *currentView;
    bool nomenu;
    QFutureSignal *fs;

public slots:
    void viewSeen( SimCommandView *view );
    void commandReceived( const QSimCommand& cmd );
    void controlEventReceived( const QSimControlEvent& ev );

signals:
    void commandSeen();
    void eventSeen();
    void nothingSeen();
};

class tst_QSimToolkitServer : public QSimToolkit
{
    Q_OBJECT
public:
    tst_QSimToolkitServer( const QString& service, QObject *parent );
    ~tst_QSimToolkitServer();

    void clear();
    void emitCommand( const QSimCommand& cmd );
    void emitControlEvent( const QSimControlEvent& ev );
    QByteArray lastResponse() const { return resp; }
    QByteArray lastEnvelope() const { return env; }
    int responseCount() const { return respCount; }
    int envelopeCount() const { return envCount; }

    void startUsingTestMenu( const QSimCommand& cmd );
    void stopUsingTestMenu();

    void emitPendingCommand() { emitCommand( pendingCommand ); }

public slots:
    void begin();
    void end();
    void sendResponse( const QSimTerminalResponse& resp );
    void sendEnvelope( const QSimEnvelope& env );

signals:
    void responseSeen();
    void envelopeSeen();
    void beginSeen();
    void endSeen();

private:
    int respCount;
    int envCount;
    QByteArray resp;
    QByteArray env;
    QSimCommand testMenu;
    bool usingTestMenu;
    QSimCommand pendingCommand;
};

class tst_QSimToolkitFiles : public QSimFiles
{
    Q_OBJECT
public:
    tst_QSimToolkitFiles( const QString& service, QObject *parent );
    ~tst_QSimToolkitFiles();

public slots:
    void requestFileInfo( const QString& reqid, const QString& fileid );
    void readBinary( const QString& reqid, const QString& fileid,
                     int pos, int len );
    void writeBinary( const QString& reqid, const QString& fileid,
                      int pos, const QByteArray& data );
    void readRecord( const QString& reqid, const QString& fileid,
                     int recno, int recordSize );
    void writeRecord( const QString& reqid, const QString& fileid,
                      int recno, const QByteArray& data );

private:
    QMap<QString, QByteArray> fileStore;
    QMap<QString, int> recordSizes;
};

#endif

#endif
