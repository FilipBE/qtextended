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

#include "tst_qsimtoolkit.h"
#include "simapp.h"
#include "testphonecall.h"
#include <qvaluespace.h>
#include <QEventLoop>
#include <QKeyEvent>
#include <QTimer>
#include <QtopiaApplication>
#include <shared/qtopiaunittest.h>

//TESTED_CLASS=
//TESTED_FILES=

tst_QSimToolkit::tst_QSimToolkit()
{
    sawSignal = false;
    innerLoop = 0;
    simapp = 0;
    expectedView = 0;
    currentView = 0;
    nomenu = false;
    fs = 0;
}

// Test that when a QSimCommand is created, all values are in their default states.
void tst_QSimToolkit::testCommandCreate()
{
    QSimCommand cmd;
    QVERIFY( cmd.commandNumber() == 1 );
    QVERIFY( cmd.type() == QSimCommand::NoCommand );
    QVERIFY( cmd.sourceDevice() == QSimCommand::SIM );
    QVERIFY( cmd.destinationDevice() == QSimCommand::ME );
    QVERIFY( !cmd.hasHelp() );
    QVERIFY( cmd.text().isEmpty() );
    QVERIFY( !cmd.suppressUserFeedback() );
    QVERIFY( cmd.otherText().isEmpty() );
    QVERIFY( !cmd.highPriority() );
    QVERIFY( cmd.clearAfterDelay() );
    QVERIFY( !cmd.immediateResponse() );
    QVERIFY( !cmd.ucs2Input() );
    QVERIFY( !cmd.packedInput() );
    QVERIFY( cmd.wantDigits() );
    QVERIFY( !cmd.wantYesNo() );
    QVERIFY( cmd.minimumLength() == 0 );
    QVERIFY( cmd.maximumLength() == 255 );
    QVERIFY( cmd.echo() );
    QVERIFY( cmd.disposition() == QSimCommand::IfNoOtherCalls );
    QVERIFY( !cmd.withRedial() );
    QVERIFY( cmd.number().isEmpty() );
    QVERIFY( cmd.subAddress().isEmpty() );
    QVERIFY( cmd.callClass() == QSimCommand::Voice );
    QVERIFY( cmd.tone() == QSimCommand::ToneNone );
    QVERIFY( cmd.toneTime() == 0 );
    QVERIFY( cmd.duration() == 0 );
    QVERIFY( !cmd.softKeysPreferred() );
    QVERIFY( cmd.menuPresentation() == QSimCommand::AnyPresentation );
    QVERIFY( cmd.title().isEmpty() );
    QVERIFY( cmd.defaultItem() == 0 );
    QVERIFY( cmd.menuItems().isEmpty() );
    QVERIFY( cmd.refreshType() == QSimCommand::InitAndFullFileChange );
    QVERIFY( cmd.events() == QSimCommand::NoEvent );
    QVERIFY( cmd.browserLaunchMode() == QSimCommand::IfNotAlreadyLaunched );
    QVERIFY( cmd.url().isEmpty() );
    QVERIFY( cmd.iconId() == 0 );
    QVERIFY( !cmd.iconSelfExplanatory() );
    QVERIFY( cmd.otherIconId() == 0 );
    QVERIFY( !cmd.otherIconSelfExplanatory() );
    QVERIFY( !cmd.smsPacking() );
    QVERIFY( cmd.extensionData().isEmpty() );
}

void tst_QSimToolkit::initTestCase()
{
    QValueSpace::initValuespaceManager();
    server = new tst_QSimToolkitServer( "test", this );
    files = new tst_QSimToolkitFiles( "test", this );
    phoneCallProvider = new TestPhoneCallProvider( "modem", this );
    QValueSpaceObject::sync();

    client = new QSimToolkit( "test", this );
    connect( client, SIGNAL(command(QSimCommand)), this, SLOT(commandReceived(QSimCommand)) );
    connect( client, SIGNAL(controlEvent(QSimControlEvent)),
             this, SLOT(controlEventReceived(QSimControlEvent)) );
    QVERIFY( client->available() );
}

void tst_QSimToolkit::init()
{
    if (!simapp && QByteArray(QTest::currentTestFunction()).startsWith("testUI"))
        initSimApp();
}

void tst_QSimToolkit::initSimApp()
{
    if (simapp) return;

    // Install a dummy test menu, so that simapp's QSimToolkit::begin() call will work.
    server->startUsingTestMenu( QSimCommand() );

    // Create the main window widget and display it.
    simapp = new SimApp(0);
    QtopiaApplication::instance()->showMainWidget( simapp );

    // Wait for the initial dummy test menu to be displayed.
    waitForView( SimMenu::staticMetaObject );
}

void tst_QSimToolkit::cleanupSimApp()
{
    delete simapp;
    simapp = 0;
}

// Wait for "simapp" to display a specific view.
bool tst_QSimToolkit::waitForView( const QMetaObject& meta, int timeout, bool mayHappenBefore )
{
    if (!simapp) {
        qCritical("waitForView called with null simapp");
        return false;
    }
    if ( mayHappenBefore && simapp->currentView() && simapp->currentView()->metaObject() == &meta ) {
        // Nested event loop already displayed the view.
        currentView = simapp->currentView();
        sawSignal = true;
        return true;
    }
    QObject::connect( simapp, SIGNAL(viewChanged(SimCommandView*)),
                      this, SLOT(viewSeen(SimCommandView*)) );
    expectedView = &meta;
    currentView = 0;
    sawSignal = false;
    innerLoop = new QEventLoop( this );
    QTimer::singleShot( timeout, innerLoop, SLOT(quit()) );
    innerLoop->exec();
    delete innerLoop;
    innerLoop = 0;
    QObject::disconnect( simapp, SIGNAL(viewChanged(SimCommandView*)),
                         this, SLOT(viewSeen(SimCommandView*)) );
    bool saw = sawSignal;
    msleep(0);      // Flush event queue and stablise simapp.
    return saw;
}

void tst_QSimToolkit::keyClick( int key, const QString& text )
{
    QKeyEvent *press = new QKeyEvent( QEvent::KeyPress, key, Qt::NoModifier, text );
    QKeyEvent *release = new QKeyEvent( QEvent::KeyRelease, key, Qt::NoModifier, text );
    QWidget *w = QWidget::keyboardGrabber();
    if (!w) {
        w = QApplication::activePopupWidget();
        if (w && w->focusWidget())
            w = w->focusWidget();
    }
    if (!w) w = QApplication::focusWidget();
    if (!w) w = QApplication::activeWindow();
    if (!w) w = simapp;

    QApplication::postEvent( w, press );
    QApplication::postEvent( w, release );
}

void tst_QSimToolkit::msleep( int msecs )
{
    // Wait for a signal that won't happen during the timeout.
    QFutureSignal::wait( this, SIGNAL(nothingSeen()), msecs );
}

void tst_QSimToolkit::select()
{
    if (!nomenu) {
        // The test is running inside a menu, so just press the OK button.
        keyClick( Qt::Key_Select );
    } else {
        // The test should be run outside a menu, so hide the main menu,
        // and then emit the pending command.
        simapp->hideApp();
        server->emitPendingCommand();
    }
}

void tst_QSimToolkit::viewSeen( SimCommandView *view )
{
    if ( view->metaObject() == expectedView ) {
        sawSignal = true;
        if ( innerLoop )
            innerLoop->quit();
        currentView = view;
    }
}

void tst_QSimToolkit::commandReceived( const QSimCommand& cmd )
{
    deliveredCommand = cmd;
    emit commandSeen();
}

void tst_QSimToolkit::controlEventReceived( const QSimControlEvent& ev )
{
    deliveredEvent = ev;
    emit eventSeen();
}

tst_QSimToolkitServer::tst_QSimToolkitServer( const QString& service, QObject *parent )
    : QSimToolkit( service, parent, QCommInterface::Server )
{
    usingTestMenu = false;
    clear();
}

tst_QSimToolkitServer::~tst_QSimToolkitServer()
{
}

void tst_QSimToolkitServer::clear()
{
    respCount = 0;
    envCount = 0;
    resp = QByteArray();
    env = QByteArray();
}

void tst_QSimToolkitServer::emitCommand( const QSimCommand& cmd )
{
    emitCommandAndRespond( cmd );
}

void tst_QSimToolkitServer::emitControlEvent( const QSimControlEvent& ev )
{
    emit controlEvent( ev );
}

void tst_QSimToolkitServer::begin()
{
    emit beginSeen();
    if ( testMenu.type() == QSimCommand::SetupMenu )
        emitCommandAndRespond( testMenu );
    else
        emit beginFailed();
}

void tst_QSimToolkitServer::end()
{
    emit endSeen();
}

void tst_QSimToolkitServer::sendResponse( const QSimTerminalResponse& resp )
{
    // Bail out if this is the ack for the test menu.
    if ( usingTestMenu && resp.command().type() == QSimCommand::SetupMenu )
        return;

    ++respCount;
    this->resp = resp.toPdu();
    emit responseSeen();
}

void tst_QSimToolkitServer::sendEnvelope( const QSimEnvelope& env )
{
    if ( usingTestMenu ) {
        if ( env.type() == QSimEnvelope::MenuSelection && env.menuItem() == 1 &&
             !env.requestHelp() ) {
            // Send the pending command, which is the actual command under test.
            emitCommandAndRespond( pendingCommand );
            return;
        }
    }
    ++envCount;
    this->env = env.toPdu();
    emit envelopeSeen();
}

void tst_QSimToolkitServer::startUsingTestMenu( const QSimCommand& cmd )
{
    clear();

    // Save the pending command, which is to be sent once the user selects
    // the "Run Test" option from the test menu.
    pendingCommand = cmd;

    QSimMenuItem item;
    QList<QSimMenuItem> items;

    testMenu.setType( QSimCommand::SetupMenu );
    if ( cmd.type() == QSimCommand::NoCommand )
        testMenu.setTitle( "Dummy" );
    else
        testMenu.setTitle( "Test" );

    item.setIdentifier( 1 );
    item.setLabel( "Run Test" );
    items += item;

    testMenu.setMenuItems( items );

    usingTestMenu = true;

    // Force "simapp" back to the test menu.
    emitCommandAndRespond( testMenu );
}


void tst_QSimToolkitServer::stopUsingTestMenu()
{
    usingTestMenu = false;
}

tst_QSimToolkitFiles::tst_QSimToolkitFiles( const QString& service, QObject *parent )
    : QSimFiles( service, parent, Server )
{
    static unsigned char const EFimg[] = {
        0x01, 0x2E, 0x28, 0x11, 0x4F, 0x01, 0x00, 0x00, 0x00, 0xE8,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x01, 0x08, 0x08, 0x21, 0x4F, 0x02, 0x00, 0x00, 0x00, 0x1F,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x01, 0x18, 0x10, 0x11, 0x4F, 0x03, 0x00, 0x00, 0x00, 0x32,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x01, 0x08, 0x08, 0x11, 0x4F, 0x04, 0x00, 0x00, 0x00, 0x0A,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x01, 0x05, 0x05, 0x11, 0x4F, 0x05, 0x00, 0x00, 0x00, 0x08,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };
    fileStore.insert( "7F105F504F20", QByteArray( (char *)EFimg, sizeof(EFimg) ) );
    recordSizes.insert( "7F105F504F20", 20 );

    static unsigned char const EFimg1[] = {
        0x2E, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0xFF, 0x80, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00,
        0x00, 0x77, 0xFE, 0x00, 0x00, 0x00, 0x01, 0xBF, 0xF8, 0x00,
        0x00, 0x00, 0x06, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x1A, 0x03,
        0x80, 0x00, 0x00, 0x00, 0x6B, 0xF6, 0xBC, 0x00, 0x00, 0x01,
        0xAF, 0xD8, 0x38, 0x00, 0x00, 0x06, 0xBF, 0x60, 0x20, 0x00,
        0x00, 0x1A, 0xFD, 0x80, 0x40, 0x00, 0x00, 0x6B, 0xF6, 0x00,
        0x80, 0x00, 0x01, 0xA0, 0x1F, 0x02, 0x00, 0x00, 0x06, 0xFF,
        0xE4, 0x04, 0x00, 0x00, 0x1B, 0xFF, 0x90, 0x10, 0x00, 0x00,
        0x6D, 0xEE, 0x40, 0x40, 0x00, 0x01, 0xBF, 0xF9, 0x01, 0x00,
        0x00, 0x6F, 0xFF, 0xE4, 0x04, 0x00, 0x00, 0x1B, 0xFF, 0x90,
        0x10, 0x00, 0x00, 0x6F, 0xFE, 0x40, 0x40, 0x00, 0x01, 0xBF,
        0xF9, 0x01, 0x00, 0x00, 0x06, 0xFF, 0xE6, 0x04, 0x00, 0x00,
        0x1B, 0xFF, 0x88, 0x10, 0x00, 0x00, 0x6F, 0xFE, 0x20, 0x40,
        0x00, 0x01, 0xBF, 0xF8, 0x66, 0x00, 0x00, 0x06, 0xFF, 0xE0,
        0xF0, 0x00, 0x00, 0x1B, 0xFF, 0x80, 0x80, 0x00, 0x00, 0x7F,
        0xFE, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x00, 0x00,
        0x1F, 0xFF, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x1C, 0x21, 0x08, 0x44, 0xEE, 0x00, 0x48, 0xC4,
        0x31, 0x92, 0x20, 0x01, 0x25, 0x11, 0x45, 0x50, 0x80, 0x07,
        0x14, 0x45, 0x15, 0x43, 0x80, 0x12, 0x71, 0x1C, 0x4D, 0x08,
        0x00, 0x4A, 0x24, 0x89, 0x32, 0x20, 0x01, 0xC8, 0x9E, 0x24,
        0x4E, 0xE0
    };
    fileStore.insert( "7F105F504F01", QByteArray( (char *)EFimg1, sizeof(EFimg1) ) );
    recordSizes.insert( "7F105F504F01", 1 );

    static unsigned char const EFimg2[] = {
        0x08, 0x08, 0x02, 0x03, 0x00, 0x16, 0xAA, 0xAA, 0x80, 0x02,
        0x85, 0x42, 0x81, 0x42, 0x81, 0x42, 0x81, 0x52, 0x80, 0x02,
        0xAA, 0xAA, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
        0xFF
    };
    fileStore.insert( "7F105F504F02", QByteArray( (char *)EFimg2, sizeof(EFimg2) ) );
    recordSizes.insert( "7F105F504F02", 1 );

    static unsigned char const EFimg3[] = {
        0x18, 0x10, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x01, 0x80, 0x00,
        0x01, 0x80, 0x00, 0x01, 0x8F, 0x3C, 0xF1, 0x89, 0x20, 0x81,
        0x89, 0x20, 0x81, 0x89, 0x20, 0xF1, 0x89, 0x20, 0x11, 0x89,
        0x20, 0x11, 0x89, 0x20, 0x11, 0x8F, 0x3C, 0xF1, 0x80, 0x00,
        0x01, 0x80, 0x00, 0x01, 0x80, 0x00, 0x01, 0xFF, 0xFF, 0xFF
    };
    fileStore.insert( "7F105F504F03", QByteArray( (char *)EFimg3, sizeof(EFimg3) ) );
    recordSizes.insert( "7F105F504F03", 1 );

    static unsigned char const EFimg4[] = {
        0x08, 0x08, 0xFF, 0x03, 0xA5, 0x99, 0x99, 0xA5, 0xC3, 0xFF
    };
    fileStore.insert( "7F105F504F04", QByteArray( (char *)EFimg4, sizeof(EFimg4) ) );
    recordSizes.insert( "7F105F504F04", 1 );

    static unsigned char const EFimg5[] = {
        0x05, 0x05, 0xFE, 0xEB, 0xBF, 0xFF, 0xFF, 0xFF
    };
    fileStore.insert( "7F105F504F05", QByteArray( (char *)EFimg5, sizeof(EFimg5) ) );
    recordSizes.insert( "7F105F504F05", 1 );
}

tst_QSimToolkitFiles::~tst_QSimToolkitFiles()
{
}

void tst_QSimToolkitFiles::requestFileInfo( const QString& reqid, const QString& fileid )
{
    if ( !fileStore.contains( fileid ) ) {
        emit error( reqid, QTelephony::SimFileNotFound );
    } else {
        int size = fileStore[fileid].size();
        int recordSize = recordSizes[fileid];
        if ( recordSize == 1 )
            emit fileInfo( reqid, size, recordSize, QTelephony::SimFileTransparent );
        else
            emit fileInfo( reqid, size, recordSize, QTelephony::SimFileLinearFixed );
    }
}

void tst_QSimToolkitFiles::readBinary
        ( const QString& reqid, const QString& fileid, int pos, int len )
{
    if ( !fileStore.contains( fileid ) ) {
        emit error( reqid, QTelephony::SimFileNotFound );
    } else {
        emit readDone( reqid, fileStore[fileid].mid( pos, len ), pos );
    }
}

void tst_QSimToolkitFiles::writeBinary
        ( const QString& reqid, const QString& , int , const QByteArray& )
{
    // We don't support binary file writing.
    emit error( reqid, QTelephony::SimFileInvalidWrite );
}

void tst_QSimToolkitFiles::readRecord
        ( const QString& reqid, const QString& fileid, int recno, int recordSize )
{
    if ( !fileStore.contains( fileid ) ) {
        emit error( reqid, QTelephony::SimFileNotFound );
    } else {
        emit readDone( reqid, fileStore[fileid].mid( recno * recordSize, recordSize ), recno );
    }
}

void tst_QSimToolkitFiles::writeRecord
        ( const QString& reqid, const QString& , int , const QByteArray& )
{
    // We only support binary files.
    emit error( reqid, QTelephony::SimFileInvalidWrite );
}

QTEST_APP_MAIN( tst_QSimToolkit, QtopiaApplication )
