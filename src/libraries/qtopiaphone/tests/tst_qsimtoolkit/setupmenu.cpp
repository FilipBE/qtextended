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
// Test encoding and decoding of SET UP MENU commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.8 - SET UP MENU.
void tst_QSimToolkit::testEncodeSetupMenu_data()
{
    QSimToolkitData::populateDataSetupMenu();
}
void tst_QSimToolkit::testEncodeSetupMenu()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, envelope );
    QFETCH( int, resptype );
    QFETCH( QString, title );
    QFETCH( QString, items );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( bool, hasHelp );
    QFETCH( bool, softKeysPreferred );
    QFETCH( int, selectedItem );
    QFETCH( bool, requestHelp );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::SetupMenu );
    QVERIFY( decoded.destinationDevice() == QSimCommand::ME );
    QCOMPARE( decoded.title(), title );
    QCOMPARE( (int)decoded.iconId(), iconId );
    QCOMPARE( decoded.iconSelfExplanatory(), iconSelfExplanatory );
    QCOMPARE( decoded.hasHelp(), hasHelp );
    QCOMPARE( decoded.softKeysPreferred(), softKeysPreferred );
    QVERIFY( checkMenuItems( decoded.menuItems(), items, hasHelp ) );

    // Check that the original command PDU can be reconstructed correctly.
    QByteArray encoded = decoded.toPdu( (QSimCommand::ToPduOptions)options );
    QCOMPARE( encoded, data );

    // Check that the terminal response PDU can be parsed correctly.
    QSimTerminalResponse decodedResp = QSimTerminalResponse::fromPdu(resp);
    QVERIFY( data.contains( decodedResp.commandPdu() ) );
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

    // Bail out if no envelope in the test data.
    if ( envelope.isEmpty() )
        return;

    // Check that the envelope PDU can be parsed correctly.
    QSimEnvelope decodedEnv = QSimEnvelope::fromPdu(envelope);
    QVERIFY( decodedEnv.type() == QSimEnvelope::MenuSelection );
    QVERIFY( decodedEnv.sourceDevice() == QSimCommand::Keypad );
    QCOMPARE( (int)decodedEnv.menuItem(), selectedItem );
    QCOMPARE( decodedEnv.requestHelp(), requestHelp );

    // Check that the original envelope PDU can be reconstructed correctly.
    QCOMPARE( decodedEnv.toPdu(), envelope );
}

// Test that MORE TIME commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverSetupMenu_data()
{
    QSimToolkitData::populateDataSetupMenu();
}
void tst_QSimToolkit::testDeliverSetupMenu()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, envelope );
    QFETCH( int, resptype );
    QFETCH( QString, title );
    QFETCH( QString, items );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( bool, hasHelp );
    QFETCH( bool, softKeysPreferred );
    QFETCH( int, selectedItem );
    QFETCH( bool, requestHelp );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SetupMenu );
    cmd.setTitle( title );
    cmd.setMenuItems( parseMenuItems( items, hasHelp ) );
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
    QVERIFY( deliveredCommand.hasHelp() == cmd.hasHelp() );
    QVERIFY( deliveredCommand.softKeysPreferred() == cmd.softKeysPreferred() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The terminal response should have been sent immediately to ack reception of the menu.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    if ( resptype != 0x0004 ) {
        QCOMPARE( server->lastResponse(), resp );
    } else {
        // We cannot test the "icon not displayed" case because the qtopiaphone
        // library will always respond with "command performed successfully".
        // Presumably the Qtopia user interface can always display icons.
        QByteArray resp2 = resp;
        resp2[resp2.size() - 1] = 0x00;
        QCOMPARE( server->lastResponse(), resp2 );
    }

    // Bail out if no envelope in the test data.
    if ( envelope.isEmpty() )
        return;

    // Compose and send the envelope.
    QSimEnvelope env;
    env.setType( QSimEnvelope::MenuSelection );
    env.setSourceDevice( QSimCommand::Keypad );
    env.setMenuItem( selectedItem );
    env.setRequestHelp( requestHelp );
    client->sendEnvelope( env );

    // Wait for the envelope to be received.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

    // Check that the envelope is what we expected to get and that we didn't
    // get any further terminal responses.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 1 );
    QCOMPARE( server->lastEnvelope(), envelope );
}

// Test the user interface in "simapp" for SET UP MENU.
void tst_QSimToolkit::testUISetupMenu_data()
{
    QSimToolkitData::populateDataSetupMenu();
}
void tst_QSimToolkit::testUISetupMenu()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( QByteArray, envelope );
    QFETCH( int, resptype );
    QFETCH( QString, title );
    QFETCH( QString, items );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( bool, hasHelp );
    QFETCH( bool, softKeysPreferred );
    QFETCH( int, selectedItem );
    QFETCH( bool, requestHelp );
    QFETCH( int, options );

    Q_UNUSED(options);

    // Skip the test if it expects "Icon not displayed", because we always
    // attempt to display icons in "simapp".
    if ( resptype == 0x0004 )
        QSKIP( "", SkipSingle );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Create the command to be tested.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SetupMenu );
    cmd.setTitle( title );
    cmd.setMenuItems( parseMenuItems( items, hasHelp ) );
    cmd.setHasHelp( hasHelp );
    cmd.setSoftKeysPreferred( softKeysPreferred );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );

    // Stop using the test menu and replace it with our menu.
    server->stopUsingTestMenu();
    msleep(50);        // Wait for system to stablise.
    server->clear();
    server->emitCommand( cmd );

    // Wait for the menu view to display.
    if ( softKeysPreferred )
        QVERIFY( waitForView( SoftKeysMenu::staticMetaObject ) );
    else
        QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // Wait for icons to load.  Should be fast because they are in-process.
    if ( iconId != 0 )
        msleep(1000);

    // We should have a single response at this point, ack'ing the SET UP MENU.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QCOMPARE( server->lastResponse(), resp );

    // If we don't have an envelope for item selection, then we are done.
    if ( envelope.isEmpty() )
        return;

    // Find the menu item to be selected and issue that many down arrows to select it.
    if ( softKeysPreferred ) {
        if ( cmd.menuItems()[0].identifier() == (uint)selectedItem )
            keyClick( Qt::Key_Context1 );
        else
            keyClick( Qt::Key_Select );
    } else {
        int index;
        for ( index = 0; index < cmd.menuItems().size(); ++index ) {
            if ( cmd.menuItems()[index].identifier() == (uint)selectedItem )
                break;
            keyClick( Qt::Key_Down );
            msleep(50);
        }

        // Press the OK button, or select "Help" via the context button.
        if ( !requestHelp ) {
            keyClick( Qt::Key_Select );
        } else {
            keyClick( Qt::Key_Context1 );
        }
    }

    // Wait for the envelope to be delivered.
    QVERIFY( QFutureSignal::wait( server, SIGNAL(envelopeSeen()), 100 ) );

    // Check that the envelope is the same as what we expected.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 1 );
    QCOMPARE( server->lastEnvelope(), envelope );
}

// Get an argument value from a menu item expected value specification.
static QString getArg
    ( const QString& exp, const QString& name, const QString& defValue = QString() )
{
    int index = exp.indexOf( name + "=" );
    if ( index < 0 )
        return defValue;
    index += name.length() + 1;
    if ( name == "label" )
        return exp.mid( index );
    int index2 = exp.indexOf( QChar(' '), index );
    if ( index2 < 0 )
        index2 = exp.length();
    return exp.mid( index, index2 - index );
}

// Check the contents of a menu against an expected item list.
// Also used by the SELECT ITEM test cases in selectitem.cpp.
bool tst_QSimToolkit::checkMenuItems
            ( const QList<QSimMenuItem>& menuItems, const QString& expected, bool hasHelp )
{
    QStringList itemList;
    if ( !expected.isEmpty() )
        itemList = expected.split( QChar('\n') );
    if ( menuItems.size() != itemList.size() ) {
        qDebug() << "Wrong number of menu items";
        return false;
    }
    bool ok = true;
    for ( int index = 0; index < menuItems.size(); ++index ) {
        QSimMenuItem item = menuItems[index];
        QString exp = itemList[index];
        if ( item.hasHelp() != hasHelp ) {
            qDebug() << "Item" << index << "hasHelp flag wrong";
            ok = false;
        }
        if ( item.label() != getArg( exp, "label" ) ) {
            qDebug() << "Item" << index << "label wrong, actual:" << item.label()
                     << ", expected:" << getArg( exp, "label" );
            ok = false;
        }
        uint id = getArg( exp, "id" ).toUInt( 0, 16 );
        if ( item.identifier() != id ) {
            qDebug() << "Item" << index << "identifier wrong, actual:" << item.identifier()
                     << ", expected:" << id;
            ok = false;
        }
        id = getArg( exp, "icon", "0" ).toUInt();
        if ( item.iconId() != id ) {
            qDebug() << "Item" << index << "icon identifier wrong, actual:" << item.iconId()
                     << ", expected:" << id;
            ok = false;
        }
        id = getArg( exp, "next", "0" ).toUInt( 0, 16 );
        if ( item.nextAction() != id ) {
            qDebug() << "Item" << index << "next action wrong, actual:" << item.nextAction()
                     << ", expected:" << id;
            ok = false;
        }
        bool flag = ( getArg( exp, "selfexpl", "false" ) == "true" );
        if ( item.iconSelfExplanatory() != flag ) {
            qDebug() << "Item" << index << "icon self-explanatory flag wrong, actual:"
                     << item.iconSelfExplanatory() << ", expected:" << flag;
            ok = false;
        }
    }
    return ok;
}

// Parse an expected menu item list into an actual list of menu items.
QList<QSimMenuItem> tst_QSimToolkit::parseMenuItems( const QString& expected, bool hasHelp )
{
    QList<QSimMenuItem> items;
    QStringList itemList;
    if ( !expected.isEmpty() )
        itemList = expected.split( QChar('\n') );
    foreach ( QString exp, itemList ) {
        QSimMenuItem item;
        item.setHasHelp( hasHelp );
        item.setLabel( getArg( exp, "label" ) );
        item.setIdentifier( getArg( exp, "id" ).toUInt( 0, 16 ) );
        item.setIconId( getArg( exp, "icon", "0" ).toUInt() );
        item.setIconSelfExplanatory( getArg( exp, "selfexpl", "false" ) == "true" );
        item.setNextAction( getArg( exp, "next", "0" ).toUInt( 0, 16 ) );
        items += item;
    }
    return items;
}

#endif // !SYSTEMTEST

// Populate data-driven tests for SET UP MENU from the GCF test cases
// in GSM 51.010, section 27.22.4.8.
void QSimToolkitData::populateDataSetupMenu()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<QByteArray>("envelope");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("title");
    QTest::addColumn<QString>("items");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<bool>("hasHelp");
    QTest::addColumn<bool>("softKeysPreferred");
    QTest::addColumn<int>("selectedItem");
    QTest::addColumn<bool>("requestHelp");
    QTest::addColumn<int>("options");

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x3B, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07,
         0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03, 0x49, 0x74,
         0x65, 0x6D, 0x20, 0x33, 0x8F, 0x07, 0x04, 0x49, 0x74, 0x65, 0x6D, 0x20,
         0x34};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_1_1_1[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x02};
    QTest::newRow( "SET UP MENU 1.1.1 - GCF 27.22.4.8.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << QByteArray( (char *)enve_1_1_1, sizeof(enve_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Menu" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2\n"
                    "id=03 label=Item 3\n"
                    "id=04 label=Item 4" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_2[] =
        {0xD0, 0x23, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x04, 0x11, 0x4F, 0x6E, 0x65, 0x8F, 0x04, 0x12, 0x54, 0x77,
         0x6F};
    static unsigned char const resp_1_1_2[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_1_1_2[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x12};
    QTest::newRow( "SET UP MENU 1.1.2 - GCF 27.22.4.8.1" )
        << QByteArray( (char *)data_1_1_2, sizeof(data_1_1_2) )
        << QByteArray( (char *)resp_1_1_2, sizeof(resp_1_1_2) )
        << QByteArray( (char *)enve_1_1_2, sizeof(enve_1_1_2) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Menu" )
        << QString( "id=11 label=One\n"
                    "id=12 label=Two" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0x12         // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_1_3[] =
        {0xD0, 0x0D, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x00, 0x8F, 0x00};
    static unsigned char const resp_1_1_3[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    QTest::newRow( "SET UP MENU 1.1.3 - GCF 27.22.4.8.1" )
        << QByteArray( (char *)data_1_1_3, sizeof(data_1_1_3) )
        << QByteArray( (char *)resp_1_1_3, sizeof(resp_1_1_3) )
        << QByteArray()
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << -1           // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x81, 0xFC, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82,
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
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_1_2_1[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x3D};
    QTest::newRow( "SET UP MENU 1.2.1 - GCF 27.22.4.8.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << QByteArray( (char *)enve_1_2_1, sizeof(enve_1_2_1) )
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
        << 0x3D         // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_2[] =
        {0xD0, 0x81, 0xF3, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x85, 0x0A, 0x4C, 0x61, 0x72, 0x67, 0x65, 0x4D, 0x65, 0x6E, 0x75, 0x32,
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
    static unsigned char const resp_1_2_2[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_1_2_2[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0xFB};
    QTest::newRow( "SET UP MENU 1.2.2 - GCF 27.22.4.8.1" )
        << QByteArray( (char *)data_1_2_2, sizeof(data_1_2_2) )
        << QByteArray( (char *)resp_1_2_2, sizeof(resp_1_2_2) )
        << QByteArray( (char *)enve_1_2_2, sizeof(enve_1_2_2) )
        << 0x0000       // Command performed successfully
        << QString( "LargeMenu2" )
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
        << 0xFB         // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_2_3[] =
        {0xD0, 0x81, 0xFC, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82,
         0x85, 0x81, 0xEC, 0x54, 0x68, 0x65, 0x20, 0x53, 0x49, 0x4D, 0x20, 0x73,
         0x68, 0x61, 0x6C, 0x6C, 0x20, 0x73, 0x75, 0x70, 0x70, 0x6C, 0x79, 0x20,
         0x61, 0x20, 0x73, 0x65, 0x74, 0x20, 0x6F, 0x66, 0x20, 0x6D, 0x65, 0x6E,
         0x75, 0x20, 0x69, 0x74, 0x65, 0x6D, 0x73, 0x2C, 0x20, 0x77, 0x68, 0x69,
         0x63, 0x68, 0x20, 0x73, 0x68, 0x61, 0x6C, 0x6C, 0x20, 0x62, 0x65, 0x20,
         0x69, 0x6E, 0x74, 0x65, 0x67, 0x72, 0x61, 0x74, 0x65, 0x64, 0x20, 0x77,
         0x69, 0x74, 0x68, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6D, 0x65, 0x6E, 0x75,
         0x20, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x20, 0x28, 0x6F, 0x72, 0x20,
         0x6F, 0x74, 0x68, 0x65, 0x72, 0x20, 0x4D, 0x4D, 0x49, 0x20, 0x66, 0x61,
         0x63, 0x69, 0x6C, 0x69, 0x74, 0x79, 0x29, 0x20, 0x69, 0x6E, 0x20, 0x6F,
         0x72, 0x64, 0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x67, 0x69, 0x76, 0x65,
         0x20, 0x74, 0x68, 0x65, 0x20, 0x75, 0x73, 0x65, 0x72, 0x20, 0x74, 0x68,
         0x65, 0x20, 0x6F, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x75, 0x6E, 0x69, 0x74,
         0x79, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x68, 0x6F, 0x6F, 0x73, 0x65, 0x20,
         0x6F, 0x6E, 0x65, 0x20, 0x6F, 0x66, 0x20, 0x74, 0x68, 0x65, 0x73, 0x65,
         0x20, 0x6D, 0x65, 0x6E, 0x75, 0x20, 0x69, 0x74, 0x65, 0x6D, 0x73, 0x20,
         0x61, 0x74, 0x20, 0x68, 0x69, 0x73, 0x20, 0x6F, 0x77, 0x6E, 0x20, 0x64,
         0x69, 0x73, 0x63, 0x72, 0x65, 0x74, 0x69, 0x6F, 0x6E, 0x2E, 0x20, 0x45,
         0x61, 0x63, 0x68, 0x20, 0x69, 0x74, 0x65, 0x6D, 0x20, 0x63, 0x6F, 0x6D,
         0x70, 0x72, 0x69, 0x73, 0x65, 0x73, 0x20, 0x61, 0x20, 0x73, 0x68, 0x8F,
         0x02, 0x01, 0x59};
    static unsigned char const resp_1_2_3[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_1_2_3[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x01};
    QTest::newRow( "SET UP MENU 1.2.3 - GCF 27.22.4.8.1" )
        << QByteArray( (char *)data_1_2_3, sizeof(data_1_2_3) )
        << QByteArray( (char *)resp_1_2_3, sizeof(resp_1_2_3) )
        << QByteArray( (char *)enve_1_2_3, sizeof(enve_1_2_3) )
        << 0x0000       // Command performed successfully
        << QString( "The SIM shall supply a set of menu items, which shall be "
                    "integrated with the menu system (or other MMI facility) in "
                    "order to give the user the opportunity to choose one of these "
                    "menu items at his own discretion. Each item comprises a sh" )
        << QString( "id=01 label=Y" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 0x01         // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_2_1_1[] =
        {0xD0, 0x3B, 0x81, 0x03, 0x01, 0x25, 0x80, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07,
         0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03, 0x49, 0x74,
         0x65, 0x6D, 0x20, 0x33, 0x8F, 0x07, 0x04, 0x49, 0x74, 0x65, 0x6D, 0x20,
         0x34};
    static unsigned char const resp_2_1_1[] =
        {0x81, 0x03, 0x01, 0x25, 0x80, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_2_1_1[] =
        {0xD3, 0x09, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x02, 0x15, 0x00};
    QTest::newRow( "SET UP MENU 2.1.1 - GCF 27.22.4.8.2" )
        << QByteArray( (char *)data_2_1_1, sizeof(data_2_1_1) )
        << QByteArray( (char *)resp_2_1_1, sizeof(resp_2_1_1) )
        << QByteArray( (char *)enve_2_1_1, sizeof(enve_2_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Menu" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2\n"
                    "id=03 label=Item 3\n"
                    "id=04 label=Item 4" )
        << 0 << false   // Icon details
        << true         // Help flag
        << false        // Soft keys preferred flag
        << 2            // Selected item
        << true         // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_3_1_1[] =
        {0xD0, 0x41, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07,
         0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03, 0x49, 0x74,
         0x65, 0x6D, 0x20, 0x33, 0x8F, 0x07, 0x04, 0x49, 0x74, 0x65, 0x6D, 0x20,
         0x34, 0x18, 0x04, 0x13, 0x10, 0x15, 0x26};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_3_1_1[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x02};
    QTest::newRow( "SET UP MENU 3.1.1 - GCF 27.22.4.8.3" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << QByteArray( (char *)enve_3_1_1, sizeof(enve_3_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Menu" )
        << QString( "id=01 next=13 label=Item 1\n"
                    "id=02 next=10 label=Item 2\n"
                    "id=03 next=15 label=Item 3\n"
                    "id=04 next=26 label=Item 4" )
        << 0 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_1_1a[] =
        {0xD0, 0x3C, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07,
         0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03, 0x49, 0x74,
         0x65, 0x6D, 0x20, 0x33, 0x9E, 0x02, 0x01, 0x01, 0x9F, 0x04, 0x01, 0x05,
         0x05, 0x05};
    static unsigned char const resp_4_1_1a[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_4_1_1a[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x02};
    QTest::newRow( "SET UP MENU 4.1.1A - GCF 27.22.4.8.4" )
        << QByteArray( (char *)data_4_1_1a, sizeof(data_4_1_1a) )
        << QByteArray( (char *)resp_4_1_1a, sizeof(resp_4_1_1a) )
        << QByteArray( (char *)enve_4_1_1a, sizeof(enve_4_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Menu" )
        << QString( "id=01 icon=5 selfexpl=false label=Item 1\n"
                    "id=02 icon=5 selfexpl=false label=Item 2\n"
                    "id=03 icon=5 selfexpl=false label=Item 3" )
        << 1 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_1_1b[] =
        {0xD0, 0x3C, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07,
         0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03, 0x49, 0x74,
         0x65, 0x6D, 0x20, 0x33, 0x9E, 0x02, 0x01, 0x01, 0x9F, 0x04, 0x01, 0x05,
         0x05, 0x05};
    static unsigned char const resp_4_1_1b[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    static unsigned char const enve_4_1_1b[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x02};
    QTest::newRow( "SET UP MENU 4.1.1B - GCF 27.22.4.8.4" )
        << QByteArray( (char *)data_4_1_1b, sizeof(data_4_1_1b) )
        << QByteArray( (char *)resp_4_1_1b, sizeof(resp_4_1_1b) )
        << QByteArray( (char *)enve_4_1_1b, sizeof(enve_4_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Toolkit Menu" )
        << QString( "id=01 icon=5 selfexpl=false label=Item 1\n"
                    "id=02 icon=5 selfexpl=false label=Item 2\n"
                    "id=03 icon=5 selfexpl=false label=Item 3" )
        << 1 << false   // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_2_1a[] =
        {0xD0, 0x3C, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07,
         0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03, 0x49, 0x74,
         0x65, 0x6D, 0x20, 0x33, 0x9E, 0x02, 0x00, 0x01, 0x9F, 0x04, 0x00, 0x05,
         0x05, 0x05};
    static unsigned char const resp_4_2_1a[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_4_2_1a[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x02};
    QTest::newRow( "SET UP MENU 4.2.1A - GCF 27.22.4.8.4" )
        << QByteArray( (char *)data_4_2_1a, sizeof(data_4_2_1a) )
        << QByteArray( (char *)resp_4_2_1a, sizeof(resp_4_2_1a) )
        << QByteArray( (char *)enve_4_2_1a, sizeof(enve_4_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Menu" )
        << QString( "id=01 icon=5 selfexpl=true label=Item 1\n"
                    "id=02 icon=5 selfexpl=true label=Item 2\n"
                    "id=03 icon=5 selfexpl=true label=Item 3" )
        << 1 << true    // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_4_2_1b[] =
        {0xD0, 0x3C, 0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07,
         0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32, 0x8F, 0x07, 0x03, 0x49, 0x74,
         0x65, 0x6D, 0x20, 0x33, 0x9E, 0x02, 0x00, 0x01, 0x9F, 0x04, 0x00, 0x05,
         0x05, 0x05};
    static unsigned char const resp_4_2_1b[] =
        {0x81, 0x03, 0x01, 0x25, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04};
    static unsigned char const enve_4_2_1b[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x02};
    QTest::newRow( "SET UP MENU 4.2.1B - GCF 27.22.4.8.4" )
        << QByteArray( (char *)data_4_2_1b, sizeof(data_4_2_1b) )
        << QByteArray( (char *)resp_4_2_1b, sizeof(resp_4_2_1b) )
        << QByteArray( (char *)enve_4_2_1b, sizeof(enve_4_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Toolkit Menu" )
        << QString( "id=01 icon=5 selfexpl=true label=Item 1\n"
                    "id=02 icon=5 selfexpl=true label=Item 2\n"
                    "id=03 icon=5 selfexpl=true label=Item 3" )
        << 1 << true    // Icon details
        << false        // Help flag
        << false        // Soft keys preferred flag
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_5_1_1[] =
        {0xD0, 0x29, 0x81, 0x03, 0x01, 0x25, 0x01, 0x82, 0x02, 0x81, 0x82, 0x85,
         0x0C, 0x54, 0x6F, 0x6F, 0x6C, 0x6B, 0x69, 0x74, 0x20, 0x4D, 0x65, 0x6E,
         0x75, 0x8F, 0x07, 0x01, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x31, 0x8F, 0x07,
         0x02, 0x49, 0x74, 0x65, 0x6D, 0x20, 0x32};
    static unsigned char const resp_5_1_1[] =
        {0x81, 0x03, 0x01, 0x25, 0x01, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00};
    static unsigned char const enve_5_1_1[] =
        {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81, 0x90, 0x01, 0x02};
    QTest::newRow( "SET UP MENU 5.1.1 - GCF 27.22.4.8.5" )
        << QByteArray( (char *)data_5_1_1, sizeof(data_5_1_1) )
        << QByteArray( (char *)resp_5_1_1, sizeof(resp_5_1_1) )
        << QByteArray( (char *)enve_5_1_1, sizeof(enve_5_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Toolkit Menu" )
        << QString( "id=01 label=Item 1\n"
                    "id=02 label=Item 2" )
        << 0 << false   // Icon details
        << false        // Help flag
        << true         // Soft keys preferred flag
        << 2            // Selected item
        << false        // Request help
        << (int)( QSimCommand::NoPduOptions );
}
