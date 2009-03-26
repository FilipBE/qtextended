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

#ifndef SYSTEMTEST

// Test encoding and decoding of EVENT DOWNLOAD envelopes based on the
// Test encoding and decoding of SELECT ITEM commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.9 - SELECT ITEM.
void tst_QSimToolkit::testEncodeSelectItem_data()
{
    QSimToolkitData::populateDataSelectItem();
}
void tst_QSimToolkit::testEncodeSelectItem()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, title );
    QFETCH( QString, items );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( bool, hasHelp );
    QFETCH( bool, softKeysPreferred );
    QFETCH( int, defaultItem );
    QFETCH( int, menuPresentation );
    QFETCH( int, selectedItem );
    QFETCH( bool, requestHelp );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::SelectItem );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.title(), title );
    QCOMPARE( (int)decoded.iconId(), iconId );
    QCOMPARE( decoded.iconSelfExplanatory(), iconSelfExplanatory );
    QCOMPARE( decoded.hasHelp(), hasHelp );
    QCOMPARE( decoded.softKeysPreferred(), softKeysPreferred );
    QCOMPARE( (int)decoded.defaultItem(), defaultItem );
    QCOMPARE( (int)decoded.menuPresentation(), menuPresentation );
    QVERIFY( checkMenuItems( decoded.menuItems(), items, hasHelp ) );

    // Check that the original command PDU can be reconstructed correctly.
    QByteArray encoded = decoded.toPdu( (QSimCommand::ToPduOptions)options );
    QCOMPARE( encoded, data );

    // Check that the terminal response PDU can be parsed correctly.
    QSimTerminalResponse decodedResp = QSimTerminalResponse::fromPdu(resp);
    QVERIFY( data.contains( decodedResp.commandPdu() ) );
    QCOMPARE( (int)decodedResp.menuItem(), selectedItem );
    if ( requestHelp )
        QVERIFY( decodedResp.result() == QSimTerminalResponse::HelpInformationRequested );
    else
        QVERIFY( decodedResp.result() != QSimTerminalResponse::HelpInformationRequested );
    if ( resptype < 0x0100 ) {
        QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)resptype );
        QVERIFY( decodedResp.causeData().isEmpty() );
        QVERIFY( decodedResp.cause() == QSimTerminalResponse::NoSpecificCause );
    } else {
        QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)(resptype >> 8) );
        QVERIFY( decodedResp.causeData().size() == 1 );
        QVERIFY( decodedResp.cause() == (QSimTerminalResponse::Cause)(resptype & 0xFF) );
    }

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );
}

// Test that SELECT ITEM commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverSelectItem_data()
{
    QSimToolkitData::populateDataSelectItem();
}
void tst_QSimToolkit::testDeliverSelectItem()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, title );
    QFETCH( QString, items );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( bool, hasHelp );
    QFETCH( bool, softKeysPreferred );
    QFETCH( int, defaultItem );
    QFETCH( int, menuPresentation );
    QFETCH( int, selectedItem );
    QFETCH( bool, requestHelp );
    QFETCH( int, options );

    Q_UNUSED(requestHelp);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SelectItem );
    cmd.setTitle( title );
    cmd.setMenuItems( parseMenuItems( items, hasHelp ) );
    cmd.setDefaultItem( defaultItem );
    cmd.setMenuPresentation( (QSimCommand::MenuPresentation)menuPresentation );
    cmd.setHasHelp( hasHelp );
    cmd.setSoftKeysPreferred( softKeysPreferred );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.title() == cmd.title() );
    QVERIFY( checkMenuItems( deliveredCommand.menuItems(), items, hasHelp ) );
    QVERIFY( deliveredCommand.defaultItem() == cmd.defaultItem() );
    QVERIFY( deliveredCommand.menuPresentation() == cmd.menuPresentation() );
    QVERIFY( deliveredCommand.hasHelp() == cmd.hasHelp() );
    QVERIFY( deliveredCommand.softKeysPreferred() == cmd.softKeysPreferred() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // There should be no responses or envelopes in the reverse direction yet.
    QCOMPARE( server->responseCount(), 0 );
    QCOMPARE( server->envelopeCount(), 0 );

    // Compose and send the response.
    QSimTerminalResponse response;
    response.setCommand( deliveredCommand );
    response.setMenuItem( selectedItem );
    if ( resptype < 0x0100 ) {
        response.setResult( (QSimTerminalResponse::Result)resptype );
    } else {
        response.setResult( (QSimTerminalResponse::Result)(resptype >> 8) );
        response.setCause( (QSimTerminalResponse::Cause)(resptype & 0xFF) );
    }
    client->sendResponse( response );

    // Wait for the response to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );

    // Check that the response is what we expected to get.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );
}

// Test the user interface in "simapp" for SELECT ITEM.
void tst_QSimToolkit::testUISelectItem_data()
{
    QSimToolkitData::populateDataSelectItem();
}
void tst_QSimToolkit::testUISelectItem()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, title );
    QFETCH( QString, items );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( bool, hasHelp );
    QFETCH( bool, softKeysPreferred );
    QFETCH( int, defaultItem );
    QFETCH( int, menuPresentation );
    QFETCH( int, selectedItem );
    QFETCH( bool, requestHelp );
    QFETCH( int, options );

    bool usingExpect = false;

    Q_UNUSED(options);

    // Skip the test if it expects "Icon not displayed", because we always
    // attempt to display icons in "simapp".  Also skip backward moves and
    // session terminates that expect a selected item id, which we don't support
    // (GCF allows us to choose whether to support item ids or not).
    if ( resptype == 0x0004 )
        QSKIP( "", SkipSingle );
    if ( ( resptype == 0x0011 || resptype == 0x0010 ) && selectedItem != 0 )
        QSKIP( "", SkipSingle );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Create the command to be tested.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SelectItem );
    cmd.setTitle( title );
    cmd.setMenuItems( parseMenuItems( items, hasHelp ) );
    cmd.setDefaultItem( defaultItem );
    cmd.setMenuPresentation( (QSimCommand::MenuPresentation)menuPresentation );
    cmd.setHasHelp( hasHelp );
    cmd.setSoftKeysPreferred( softKeysPreferred );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );

    // Set up the server with the command, ready to be selected
    // from the "Run Test" menu item on the test menu.
    server->startUsingTestMenu( cmd );
    QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // Clear the server state just before we request the actual command under test.
    server->clear();

    // Select the first menu item.
    select();

    // Wait for the menu to display.
    if ( softKeysPreferred )
        QVERIFY( waitForView( SoftKeysMenu::staticMetaObject ) );
    else
        QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // We should have no responses at this point.
    QCOMPARE( server->responseCount(), 0 );
    QCOMPARE( server->envelopeCount(), 0 );

    // Determine the kind of response that is required.
    if ( resptype == 0x0011 ) {
        // Backward move requested.
        keyClick( Qt::Key_Back );
    } else if ( resptype == 0x0010 ) {
        // SIM session terminated.
        usingExpect = true;
        QFutureSignal fs( server, SIGNAL(responseSeen()) );
        simapp->terminateSession();
        QVERIFY( fs.wait(100) );
    } else if ( resptype == 0x0012 ) {
        // No response from user.
        qDebug() << "Waiting 5 seconds for no-response indication";
        qobject_cast<SimMenu *>( currentView )->setNoResponseTimeout( 5000 );
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 5100 ) );
        QCOMPARE( server->responseCount(), 1 );
        QCOMPARE( server->envelopeCount(), 0 );
        QCOMPARE( server->lastResponse(), resp );
        return;
    } else if ( softKeysPreferred ) {
        if ( cmd.menuItems()[0].identifier() == (uint)selectedItem )
            keyClick( Qt::Key_Context1 );
        else
            keyClick( Qt::Key_Select );
    } else {
        // Find the menu item to be selected and issue up or down arrows to select it.
        int posn = 0;
        int defaultPosn = 0;
        int index;
        for ( index = 0; index < cmd.menuItems().size(); ++index ) {
            if ( cmd.menuItems()[index].identifier() == (uint)selectedItem )
                posn = index;
            if ( defaultItem && cmd.menuItems()[index].identifier() == (uint)defaultItem )
                defaultPosn = index;
        }
        if ( posn < defaultPosn ) {
            // Item is before the default.
            for ( index = posn; index > defaultPosn; ++index ) {
                keyClick( Qt::Key_Up );
                msleep(50);
            }
        } else {
            // Item is after the default.
            for ( index = defaultPosn; index < posn; ++index ) {
                keyClick( Qt::Key_Down );
                msleep(50);
            }
        }

        // Press the OK button, or select "Help" via the context button.
        if ( !requestHelp ) {
            keyClick( Qt::Key_Select );
        } else {
            keyClick( Qt::Key_Context1 );
        }
    }

    // Wait for the response to be delivered.
    if ( !usingExpect )
        QVERIFY( QFutureSignal::wait( server, SIGNAL(responseSeen()), 100 ) );

    // Check that the response is the same as what we expected.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for SELECT ITEM from the GCF test cases
// in GSM 51.010, section 27.22.4.9.
void QSimToolkitData::populateDataSelectItem()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("items");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<bool>("hasHelp");
    QTest::addColumn<bool>("softKeysPreferred");
    QTest::addColumn<int>("defaultItem");
    QTest::addColumn<int>("menuPresentation");
    QTest::addColumn<int>("selectedItem");
    QTest::addColumn<bool>("requestHelp");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x3D, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33, 0x8F, 0x07, 0x04, 0x49, 0x74, 0x65,
         0x6D, 0x20, 0x34};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x02};
    QTest::newRow( "SELECT ITEM 1.1.1 - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Select" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2\n"
                    "id=03 label=Item 3\n"
                    "id=04 label=Item 4" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x81, 0xFC, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x85, 0x0A, 0x4C, 0x61, 0x72, 0x67, 0x65, 0x4D, 0x65, 0x6E, 0x75, 0x31,
         0x8F, 0x05, 0x50, 0x5A, 0x65, 0x72, 0x6F, 0x8F, 0x04, 0x4F, 0x4F, 0x6E,
         0x65, 0x8F, 0x04, 0x4E, 0x54, 0x77, 0x6F, 0x8F, 0x06, 0x4D, 0x54, 0x68,
         0x72, 0x65, 0x65, 0x8F, 0x05, 0x4C, 0x46, 0x6F, 0x75, 0x72, 0x8F, 0x05,
         0x4B, 0x46, 0x69, 0x76, 0x65, 0x8F, 0x04, 0x4A, 0x53, 0x69, 0x78, 0x8F,
         0x06, 0x49, 0x53, 0x65, 0x76, 0x65, 0x6E, 0x8F, 0x06, 0x48, 0x45, 0x69,
         0x67, 0x68, 0x74, 0x8F, 0x05, 0x47, 0x4E, 0x69, 0x6E, 0x65, 0x8F, 0x06,
         0x46, 0x41, 0x6C, 0x70, 0x68, 0x61, 0x8F, 0x06, 0x45, 0x42, 0x72, 0x61,
         0x76, 0x6F, 0x8F, 0x08, 0x44, 0x43, 0x68, 0x61, 0x72, 0x6C, 0x69, 0x65,
         0x8F, 0x06, 0x43, 0x44, 0x65, 0x6C, 0x74, 0x61, 0x8F, 0x05, 0x42, 0x45,
         0x63, 0x68, 0x6F, 0x8F, 0x09, 0x41, 0x46, 0x6F, 0x78, 0x2D, 0x74, 0x72,
         0x6F, 0x74, 0x8F, 0x06, 0x40, 0x42, 0x6C, 0x61, 0x63, 0x6B, 0x8F, 0x06,
         0x3F, 0x42, 0x72, 0x6F, 0x77, 0x6E, 0x8F, 0x04, 0x3E, 0x52, 0x65, 0x64,
         0x8F, 0x07, 0x3D, 0x4F, 0x72, 0x61, 0x6E, 0x67, 0x65, 0x8F, 0x07, 0x3C,
         0x59, 0x65, 0x6C, 0x6C, 0x6F, 0x77, 0x8F, 0x06, 0x3B, 0x47, 0x72, 0x65,
         0x65, 0x6E, 0x8F, 0x05, 0x3A, 0x42, 0x6C, 0x75, 0x65, 0x8F, 0x07, 0x39,
         0x56, 0x69, 0x6F, 0x6C, 0x65, 0x74, 0x8F, 0x05, 0x38, 0x47, 0x72, 0x65,
         0x79, 0x8F, 0x06, 0x37, 0x57, 0x68, 0x69, 0x74, 0x65, 0x8F, 0x06, 0x36,
         0x6D, 0x69, 0x6C, 0x6C, 0x69, 0x8F, 0x06, 0x35, 0x6D, 0x69, 0x63, 0x72,
         0x6F, 0x8F, 0x05, 0x34, 0x6E, 0x61, 0x6E, 0x6F, 0x8F, 0x05, 0x33, 0x70,
         0x69, 0x63, 0x6F};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x3D};
    QTest::newRow( "SELECT ITEM 1.2.1 - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "LargeMenu1" )
        << QString( "id=50 label=Zero\n"
                    "id=4F label=One\n"
                    "id=4E label=Two\n"
                    "id=4D label=Three\n"
                    "id=4C label=Four\n"
                    "id=4B label=Five\n"
                    "id=4A label=Six\n"
                    "id=49 label=Seven\n"
                    "id=48 label=Eight\n"
                    "id=47 label=Nine\n"
                    "id=46 label=Alpha\n"
                    "id=45 label=Bravo\n"
                    "id=44 label=Charlie\n"
                    "id=43 label=Delta\n"
                    "id=42 label=Echo\n"
                    "id=41 label=Fox-trot\n"
                    "id=40 label=Black\n"
                    "id=3F label=Brown\n"
                    "id=3E label=Red\n"
                    "id=3D label=Orange\n"
                    "id=3C label=Yellow\n"
                    "id=3B label=Green\n"
                    "id=3A label=Blue\n"
                    "id=39 label=Violet\n"
                    "id=38 label=Grey\n"
                    "id=37 label=White\n"
                    "id=36 label=milli\n"
                    "id=35 label=micro\n"
                    "id=34 label=nano\n"
                    "id=33 label=pico"
                  )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 0x3D         // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x81, 0xFB, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x85, 0x0A, 0x4C, 0x61, 0x72, 0x67, 0x65, 0x4D, 0x65, 0x6E, 0x75, 0x32,
         0x8F, 0x1E, 0xFF, 0x43, 0x61, 0x6C, 0x6C, 0x20, 0x46, 0x6F, 0x72, 0x77,
         0x61, 0x72, 0x64, 0x69, 0x6E, 0x67, 0x20, 0x55, 0x6E, 0x63, 0x6F, 0x6E,
         0x64, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x8F, 0x1D, 0xFE, 0x43,
         0x61, 0x6C, 0x6C, 0x20, 0x46, 0x6F, 0x72, 0x77, 0x61, 0x72, 0x64, 0x69,
         0x6E, 0x67, 0x20, 0x4F, 0x6E, 0x20, 0x55, 0x73, 0x65, 0x72, 0x20, 0x42,
         0x75, 0x73, 0x79, 0x8F, 0x1C, 0xFD, 0x43, 0x61, 0x6C, 0x6C, 0x20, 0x46,
         0x6F, 0x72, 0x77, 0x61, 0x72, 0x64, 0x69, 0x6E, 0x67, 0x20, 0x4F, 0x6E,
         0x20, 0x4E, 0x6F, 0x20, 0x52, 0x65, 0x70, 0x6C, 0x79, 0x8F, 0x26, 0xFC,
         0x43, 0x61, 0x6C, 0x6C, 0x20, 0x46, 0x6F, 0x72, 0x77, 0x61, 0x72, 0x64,
         0x69, 0x6E, 0x67, 0x20, 0x4F, 0x6E, 0x20, 0x55, 0x73, 0x65, 0x72, 0x20,
         0x4E, 0x6F, 0x74, 0x20, 0x52, 0x65, 0x61, 0x63, 0x68, 0x61, 0x62, 0x6C,
         0x65, 0x8F, 0x1E, 0xFB, 0x42, 0x61, 0x72, 0x72, 0x69, 0x6E, 0x67, 0x20,
         0x4F, 0x66, 0x20, 0x41, 0x6C, 0x6C, 0x20, 0x4F, 0x75, 0x74, 0x67, 0x6F,
         0x69, 0x6E, 0x67, 0x20, 0x43, 0x61, 0x6C, 0x6C, 0x73, 0x8F, 0x2C, 0xFA,
         0x42, 0x61, 0x72, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x4F, 0x66, 0x20, 0x41,
         0x6C, 0x6C, 0x20, 0x4F, 0x75, 0x74, 0x67, 0x6F, 0x69, 0x6E, 0x67, 0x20,
         0x49, 0x6E, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x61,
         0x6C, 0x20, 0x43, 0x61, 0x6C, 0x6C, 0x73, 0x8F, 0x11, 0xF9, 0x43, 0x4C,
         0x49, 0x20, 0x50, 0x72, 0x65, 0x73, 0x65, 0x6E, 0x74, 0x61, 0x74, 0x69,
         0x6F, 0x6E};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0xFB};
    QTest::newRow( "SELECT ITEM 1.3.1 - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "LargeMenu2" )
        << QString( "id=FF label=Call Forwarding Unconditional\n"
                    "id=FE label=Call Forwarding On User Busy\n"
                    "id=FD label=Call Forwarding On No Reply\n"
                    "id=FC label=Call Forwarding On User Not Reachable\n"
                    "id=FB label=Barring Of All Outgoing Calls\n"
                    "id=FA label=Barring Of All Outgoing International Calls\n"
                    "id=F9 label=CLI Presentation"
                  )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 0xFB         // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1a[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0B, 0x53, 0x65, 0x6C, 0x65, 0x63, 0x74, 0x20, 0x49, 0x74, 0x65, 0x6D,
         0x8F, 0x04, 0x11, 0x4F, 0x6E, 0x65, 0x8F, 0x04, 0x12, 0x54, 0x77, 0x6F};
    static unsigned char const resp_1_4_1a[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x11};
    QTest::newRow( "SELECT ITEM 1.4.1A - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_4_1a, sizeof(data_1_4_1a) )
        << QByteArray( (char *)resp_1_4_1a, sizeof(resp_1_4_1a) )
        << 0x0011       // Backward move
        << QString( "Select Item" )
        << QString( "id=11 label=One\n"
                    "id=12 label=Two" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 0            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_1b[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0B, 0x53, 0x65, 0x6C, 0x65, 0x63, 0x74, 0x20, 0x49, 0x74, 0x65, 0x6D,
         0x8F, 0x04, 0x11, 0x4F, 0x6E, 0x65, 0x8F, 0x04, 0x12, 0x54, 0x77, 0x6F};
    static unsigned char const resp_1_4_1b[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x11,
         0x90, 0x01, 0x02};
    QTest::newRow( "SELECT ITEM 1.4.1B - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_4_1b, sizeof(data_1_4_1b) )
        << QByteArray( (char *)resp_1_4_1b, sizeof(resp_1_4_1b) )
        << 0x0011       // Backward move
        << QString( "Select Item" )
        << QString( "id=11 label=One\n"
                    "id=12 label=Two" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_2a[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0B, 0x53, 0x65, 0x6C, 0x65, 0x63, 0x74, 0x20, 0x49, 0x74, 0x65, 0x6D,
         0x8F, 0x04, 0x11, 0x4F, 0x6E, 0x65, 0x8F, 0x04, 0x12, 0x54, 0x77, 0x6F};
    static unsigned char const resp_1_4_2a[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x10};
    QTest::newRow( "SELECT ITEM 1.4.2A - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_4_2a, sizeof(data_1_4_2a) )
        << QByteArray( (char *)resp_1_4_2a, sizeof(resp_1_4_2a) )
        << 0x0010       // Proactive SIM session terminated
        << QString( "Select Item" )
        << QString( "id=11 label=One\n"
                    "id=12 label=Two" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 0            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_4_2b[] =
        {0xD0, 0x22, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0B, 0x53, 0x65, 0x6C, 0x65, 0x63, 0x74, 0x20, 0x49, 0x74, 0x65, 0x6D,
         0x8F, 0x04, 0x11, 0x4F, 0x6E, 0x65, 0x8F, 0x04, 0x12, 0x54, 0x77, 0x6F};
    static unsigned char const resp_1_4_2b[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x10,
         0x90, 0x01, 0x02};
    QTest::newRow( "SELECT ITEM 1.4.2B - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_4_2b, sizeof(data_1_4_2b) )
        << QByteArray( (char *)resp_1_4_2b, sizeof(resp_1_4_2b) )
        << 0x0010       // Proactive SIM session terminated
        << QString( "Select Item" )
        << QString( "id=11 label=One\n"
                    "id=12 label=Two" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x81, 0xFD, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x85, 0x81, 0xED, 0x54, 0x68, 0x65, 0x20, 0x53, 0x49, 0x4D, 0x20, 0x73,
         0x68, 0x61, 0x6C, 0x6C, 0x20, 0x73, 0x75, 0x70, 0x70, 0x6C, 0x79, 0x20,
         0x61, 0x20, 0x73, 0x65, 0x74, 0x20, 0x6F, 0x66, 0x20, 0x69, 0x74, 0x65,
         0x6D, 0x73, 0x20, 0x66, 0x72, 0x6F, 0x6D, 0x20, 0x77, 0x68, 0x69, 0x63,
         0x68, 0x20, 0x74, 0x68, 0x65, 0x20, 0x75, 0x73, 0x65, 0x72, 0x20, 0x6D,
         0x61, 0x79, 0x20, 0x63, 0x68, 0x6F, 0x6F, 0x73, 0x65, 0x20, 0x6F, 0x6E,
         0x65, 0x2E, 0x20, 0x45, 0x61, 0x63, 0x68, 0x20, 0x69, 0x74, 0x65, 0x6D,
         0x20, 0x63, 0x6F, 0x6D, 0x70, 0x72, 0x69, 0x73, 0x65, 0x73, 0x20, 0x61,
         0x20, 0x73, 0x68, 0x6F, 0x72, 0x74, 0x20, 0x69, 0x64, 0x65, 0x6E, 0x74,
         0x69, 0x66, 0x69, 0x65, 0x72, 0x20, 0x28, 0x75, 0x73, 0x65, 0x64, 0x20,
         0x74, 0x6F, 0x20, 0x69, 0x6E, 0x64, 0x69, 0x63, 0x61, 0x74, 0x65, 0x20,
         0x74, 0x68, 0x65, 0x20, 0x73, 0x65, 0x6C, 0x65, 0x63, 0x74, 0x69, 0x6F,
         0x6E, 0x29, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x61, 0x20, 0x74, 0x65, 0x78,
         0x74, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x2E, 0x20, 0x4F, 0x70,
         0x74, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x6C, 0x79, 0x20, 0x74, 0x68, 0x65,
         0x20, 0x53, 0x49, 0x4D, 0x20, 0x6D, 0x61, 0x79, 0x20, 0x69, 0x6E, 0x63,
         0x6C, 0x75, 0x64, 0x65, 0x20, 0x61, 0x6E, 0x20, 0x61, 0x6C, 0x70, 0x68,
         0x61, 0x20, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x65, 0x72,
         0x2E, 0x20, 0x54, 0x68, 0x65, 0x20, 0x61, 0x6C, 0x70, 0x68, 0x61, 0x20,
         0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x65, 0x72, 0x20, 0x69,
         0x8F, 0x02, 0x01, 0x59};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 1.5.1 - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << 0x0000       // Command performed successfully
        << QString( "The SIM shall supply a set of items from which the user may choose "
                    "one. Each item comprises a short identifier (used to indicate the "
                    "selection) and a text string. Optionally the SIM may include an alpha "
                    "identifier. The alpha identifier i" )
        << QString( "id=01 label=Y" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 1            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_6_1[] =
        {0xD0, 0x81, 0xF3, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x85, 0x0A, 0x30, 0x4C, 0x61, 0x72, 0x67, 0x65, 0x4D, 0x65, 0x6E, 0x75,
         0x8F, 0x1D, 0xFF, 0x31, 0x20, 0x43, 0x61, 0x6C, 0x6C, 0x20, 0x46, 0x6F,
         0x72, 0x77, 0x61, 0x72, 0x64, 0x20, 0x55, 0x6E, 0x63, 0x6F, 0x6E, 0x64,
         0x69, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x8F, 0x1C, 0xFE, 0x32, 0x20,
         0x43, 0x61, 0x6C, 0x6C, 0x20, 0x46, 0x6F, 0x72, 0x77, 0x61, 0x72, 0x64,
         0x20, 0x4F, 0x6E, 0x20, 0x55, 0x73, 0x65, 0x72, 0x20, 0x42, 0x75, 0x73,
         0x79, 0x8F, 0x1B, 0xFD, 0x33, 0x20, 0x43, 0x61, 0x6C, 0x6C, 0x20, 0x46,
         0x6F, 0x72, 0x77, 0x61, 0x72, 0x64, 0x20, 0x4F, 0x6E, 0x20, 0x4E, 0x6F,
         0x20, 0x52, 0x65, 0x70, 0x6C, 0x79, 0x8F, 0x25, 0xFC, 0x34, 0x20, 0x43,
         0x61, 0x6C, 0x6C, 0x20, 0x46, 0x6F, 0x72, 0x77, 0x61, 0x72, 0x64, 0x20,
         0x4F, 0x6E, 0x20, 0x55, 0x73, 0x65, 0x72, 0x20, 0x4E, 0x6F, 0x74, 0x20,
         0x52, 0x65, 0x61, 0x63, 0x68, 0x61, 0x62, 0x6C, 0x65, 0x8F, 0x20, 0xFB,
         0x35, 0x20, 0x42, 0x61, 0x72, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x4F, 0x66,
         0x20, 0x41, 0x6C, 0x6C, 0x20, 0x4F, 0x75, 0x74, 0x67, 0x6F, 0x69, 0x6E,
         0x67, 0x20, 0x43, 0x61, 0x6C, 0x6C, 0x73, 0x8F, 0x24, 0xFA, 0x36, 0x20,
         0x42, 0x61, 0x72, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x4F, 0x66, 0x20, 0x41,
         0x6C, 0x6C, 0x20, 0x4F, 0x75, 0x74, 0x67, 0x6F, 0x69, 0x6E, 0x67, 0x20,
         0x49, 0x6E, 0x74, 0x20, 0x43, 0x61, 0x6C, 0x6C, 0x73, 0x8F, 0x13, 0xF9,
         0x37, 0x20, 0x43, 0x4C, 0x49, 0x20, 0x50, 0x72, 0x65, 0x73, 0x65, 0x6E,
         0x74, 0x61, 0x74, 0x69, 0x6F, 0x6E};
    static unsigned char const resp_1_6_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0xFB};
    QTest::newRow( "SELECT ITEM 1.6.1 - GCF 27.22.4.9.1" )
        << QByteArray( (char *)data_1_6_1, sizeof(data_1_6_1) )
        << QByteArray( (char *)resp_1_6_1, sizeof(resp_1_6_1) )
        << 0x0000       // Command performed successfully
        << QString( "0LargeMenu" )
        << QString( "id=FF label=1 Call Forward Unconditional\n"
                    "id=FE label=2 Call Forward On User Busy\n"
                    "id=FD label=3 Call Forward On No Reply\n"
                    "id=FC label=4 Call Forward On User Not Reachable\n"
                    "id=FB label=5 Barring Of All Outgoing Calls\n"
                    "id=FA label=6 Barring Of All Outgoing Int Calls\n"
                    "id=F9 label=7 CLI Presentation"
                  )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 0xFB         // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x39, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33, 0x18, 0x03, 0x13, 0x10, 0x26};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x02};
    QTest::newRow( "SELECT ITEM 2.1.1 - GCF 27.22.4.9.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Select" )
        << QString( "id=01 next=13 label=Item 1\n"
                    "id=02 next=10 label=Item 2\n"
                    "id=03 next=26 label=Item 3" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_1_1[] =
        {0xD0, 0x37, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33, 0x90, 0x01, 0x02};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x03};
    QTest::newRow( "SELECT ITEM 3.1.1 - GCF 27.22.4.9.3" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Select" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2\n"
                    "id=03 label=Item 3" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 2            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 3            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_1_1[] =
        {0xD0, 0x34, 0x81, 0x03, 0x01, 0x24, 0x80, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33};
    static unsigned char const resp_4_1_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x13,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 4.1.1 - GCF 27.22.4.9.4" )
        << QByteArray( (char *)data_4_1_1, sizeof(data_4_1_1) )
        << QByteArray( (char *)resp_4_1_1, sizeof(resp_4_1_1) )
        << 0x0013       // Help information requested
        << QString( "Toolkit Select" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2\n"
                    "id=03 label=Item 3" )
        << 0 << false   // Icon details
        << true         // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 1            // Selected item
        << true         // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_1_1a[] =
        {0xD0, 0x3E, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33, 0x9E, 0x02, 0x01, 0x01, 0x9F, 0x04,
         0x01, 0x05, 0x05, 0x05};
    static unsigned char const resp_5_1_1a[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 5.1.1A - GCF 27.22.4.9.5" )
        << QByteArray( (char *)data_5_1_1a, sizeof(data_5_1_1a) )
        << QByteArray( (char *)resp_5_1_1a, sizeof(resp_5_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Select" )
        << QString( "id=01 icon=5 selfexpl=false label=Item 1\n"
                    "id=02 icon=5 selfexpl=false label=Item 2\n"
                    "id=03 icon=5 selfexpl=false label=Item 3" )
        << 1 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 1            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_1_1b[] =
        {0xD0, 0x3E, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33, 0x9E, 0x02, 0x01, 0x01, 0x9F, 0x04,
         0x01, 0x05, 0x05, 0x05};
    static unsigned char const resp_5_1_1b[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 5.1.1B - GCF 27.22.4.9.5" )
        << QByteArray( (char *)data_5_1_1b, sizeof(data_5_1_1b) )
        << QByteArray( (char *)resp_5_1_1b, sizeof(resp_5_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Toolkit Select" )
        << QString( "id=01 icon=5 selfexpl=false label=Item 1\n"
                    "id=02 icon=5 selfexpl=false label=Item 2\n"
                    "id=03 icon=5 selfexpl=false label=Item 3" )
        << 1 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 1            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_2_1a[] =
        {0xD0, 0x3E, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33, 0x9E, 0x02, 0x00, 0x01, 0x9F, 0x04,
         0x00, 0x05, 0x05, 0x05};
    static unsigned char const resp_5_2_1a[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 5.2.1A - GCF 27.22.4.9.5" )
        << QByteArray( (char *)data_5_2_1a, sizeof(data_5_2_1a) )
        << QByteArray( (char *)resp_5_2_1a, sizeof(resp_5_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Select" )
        << QString( "id=01 icon=5 selfexpl=true label=Item 1\n"
                    "id=02 icon=5 selfexpl=true label=Item 2\n"
                    "id=03 icon=5 selfexpl=true label=Item 3" )
        << 1 << true    // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 1            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_2_1b[] =
        {0xD0, 0x3E, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33, 0x9E, 0x02, 0x00, 0x01, 0x9F, 0x04,
         0x00, 0x05, 0x05, 0x05};
    static unsigned char const resp_5_2_1b[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 5.2.1B - GCF 27.22.4.9.5" )
        << QByteArray( (char *)data_5_2_1b, sizeof(data_5_2_1b) )
        << QByteArray( (char *)resp_5_2_1b, sizeof(resp_5_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Toolkit Select" )
        << QString( "id=01 icon=5 selfexpl=true label=Item 1\n"
                    "id=02 icon=5 selfexpl=true label=Item 2\n"
                    "id=03 icon=5 selfexpl=true label=Item 3" )
        << 1 << true    // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 1            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_1_1[] =
        {0xD0, 0x34, 0x81, 0x03, 0x01, 0x24, 0x03, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33};
    static unsigned char const resp_6_1_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x03, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 6.1.1 - GCF 27.22.4.9.6" )
        << QByteArray( (char *)data_6_1_1, sizeof(data_6_1_1) )
        << QByteArray( (char *)resp_6_1_1, sizeof(resp_6_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Select" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2\n"
                    "id=03 label=Item 3" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::NavigationOptionsPresentation )
        << 1            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_6_2_1[] =
        {0xD0, 0x34, 0x81, 0x03, 0x01, 0x24, 0x01, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03,
         0x49, 0x74, 0x65, 0x6D, 0x20, 0x33};
    static unsigned char const resp_6_2_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 6.2.1 - GCF 27.22.4.9.6" )
        << QByteArray( (char *)data_6_2_1, sizeof(data_6_2_1) )
        << QByteArray( (char *)resp_6_2_1, sizeof(resp_6_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Select" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2\n"
                    "id=03 label=Item 3" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::DataValuesPresentation )
        << 1            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_7_1_1[] =
        {0xD0, 0x2B, 0x81, 0x03, 0x01, 0x24, 0x04, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0E, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x53, 0x65, 0x6C,
         0x65, 0x63, 0x74, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31,
         0x8F, 0x07, 0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32};
    static unsigned char const resp_7_1_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x04, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x90, 0x01, 0x01};
    QTest::newRow( "SELECT ITEM 7.1.1 - GCF 27.22.4.9.7" )
        << QByteArray( (char *)data_7_1_1, sizeof(data_7_1_1) )
        << QByteArray( (char *)resp_7_1_1, sizeof(resp_7_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Select" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2" )
        << 0 << false   // Icon details
        << false        // Help flag
        << true         // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 1            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_8_1_1[] =
        {0xD0, 0x30, 0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0A, 0x3C, 0x54, 0x49, 0x4D, 0x45, 0x2D, 0x4F, 0x55, 0x54, 0x3E, 0x8F,
         0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07, 0x02, 0x49,
         0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03, 0x49, 0x74, 0x65, 0x6D,
         0x20, 0x33};
    static unsigned char const resp_8_1_1[] =
        {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x12};
    QTest::newRow( "SELECT ITEM 8.1.1 - GCF 27.22.4.9.8" )
        << QByteArray( (char *)data_8_1_1, sizeof(data_8_1_1) )
        << QByteArray( (char *)resp_8_1_1, sizeof(resp_8_1_1) )
        << 0x0012       // No response from user
        << QString( "<TIME-OUT>" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2\n"
                    "id=03 label=Item 3" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0            // Default item
        << (int)( QSimCommand::AnyPresentation )
        << 0            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );
}
