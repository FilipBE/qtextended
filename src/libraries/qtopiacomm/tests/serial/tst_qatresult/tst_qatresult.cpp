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

#include <QObject>
#include <QTest>
#include <qatresult.h>
#include <qatresultparser.h>

//TESTED_CLASS=QAtResult
//TESTED_FILES=src/libraries/qtopiacomm/serial/qatresult.cpp

class tst_QAtResult : public QObject
{
    Q_OBJECT
private slots:
    void testCreate();
    void testModify();
    void testBasicResults_data();
    void testBasicResults();
    void testExtendedResults_data();
    void testExtendedResults();
    void testUserData();
    void testParserCreate();
    void testParserNext_data();
    void testParserNext();
    void testParserReset_data();
    void testParserReset();

public:
    bool userDataDeleted;
};

// Test that the object is created in the correct default state.
void tst_QAtResult::testCreate()
{
    QAtResult result;
    QCOMPARE( result.result(), QString("OK") );
    QVERIFY( result.content().isEmpty() );
    QVERIFY( result.resultCode() == QAtResult::OK );
    QVERIFY( result.ok() );
    QCOMPARE( result.verboseResult(), QString("OK") );
    QVERIFY( !result.userData() );
}

// Test that modifications stick the way they should.
void tst_QAtResult::testModify()
{
    QAtResult result;
    result.setResult( "+CME ERROR: 100" );
    QCOMPARE( result.result(), QString("+CME ERROR: 100") );
    QCOMPARE( result.verboseResult(), QString("+CME ERROR: unknown") );
    QVERIFY( result.resultCode() == QAtResult::Unknown );

    result.setContent( "+CFOO: data" );
    QCOMPARE( result.content(), QString("+CFOO: data") );
    result.append( "+CFOO: data2" );
    QCOMPARE( result.content(), QString("+CFOO: data\n+CFOO: data2") );

    result.setResultCode( QAtResult::DialStringTooLong );
    QVERIFY( result.resultCode() == QAtResult::DialStringTooLong );
    QCOMPARE( result.result(),
              QString("+CME ERROR: dial string too long") );
    QCOMPARE( result.verboseResult(),
              QString("+CME ERROR: dial string too long") );

    result.setResultCode( QAtResult::SMSOperationNotAllowed );
    QVERIFY( result.resultCode() == QAtResult::SMSOperationNotAllowed );
    QCOMPARE( result.result(),
              QString("+CMS ERROR: operation not allowed") );
    QCOMPARE( result.verboseResult(),
              QString("+CMS ERROR: operation not allowed") );

    result.setResult( "+CME ERROR: 3" );
    QVERIFY( result.resultCode() == QAtResult::OperationNotAllowed );
    QCOMPARE( result.result(), QString("+CME ERROR: 3") );
    QCOMPARE( result.verboseResult(),
              QString("+CME ERROR: operation not allowed") );

    result.setResult( "+EXT ERROR: 3" );
    QVERIFY( result.resultCode() == QAtResult::OperationNotAllowed );
    QCOMPARE( result.result(), QString("+EXT ERROR: 3") );
    QCOMPARE( result.verboseResult(),
              QString("+CME ERROR: operation not allowed") );

    result.setResult( "+CMS ERROR: 302" );
    QVERIFY( result.resultCode() == QAtResult::SMSOperationNotAllowed );
    QCOMPARE( result.result(), QString("+CMS ERROR: 302") );
    QCOMPARE( result.verboseResult(),
              QString("+CMS ERROR: operation not allowed") );

    result.setResult( "+CME ERROR: 302" );
    QVERIFY( result.resultCode() == QAtResult::SMSOperationNotAllowed );
    QCOMPARE( result.result(), QString("+CME ERROR: 302") );
    QCOMPARE( result.verboseResult(),
              QString("+CMS ERROR: operation not allowed") );

    result.setResult( "+CMS ERROR: 302" );
    QVERIFY( result.resultCode() == QAtResult::SMSOperationNotAllowed );
    QCOMPARE( result.result(), QString("+CMS ERROR: 302") );
    QCOMPARE( result.verboseResult(),
              QString("+CMS ERROR: operation not allowed") );
}

// Test that the basic result codes work.
void tst_QAtResult::testBasicResults_data()
{
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("result");
    QTest::addColumn<bool>("reverseOnly");

    QTest::newRow( "OK" )
        << (int)QAtResult::OK << "OK" << false;
    QTest::newRow( "CONNECT" )
        << (int)QAtResult::Connect << "CONNECT" << false;
    QTest::newRow( "CONNECT 9600" )
        << (int)QAtResult::Connect << "CONNECT 9600" << true;
    QTest::newRow( "ERROR" )
        << (int)QAtResult::Error << "ERROR" << false;
    QTest::newRow( "NO DIALTONE" )
        << (int)QAtResult::NoDialtone << "NO DIALTONE" << false;
    QTest::newRow( "BUSY" )
        << (int)QAtResult::Busy << "BUSY" << false;
    QTest::newRow( "NO ANSWER" )
        << (int)QAtResult::NoAnswer << "NO ANSWER" << false;
}
void tst_QAtResult::testBasicResults()
{
    QFETCH( int, code );
    QFETCH( QString, result );
    QFETCH( bool, reverseOnly );

    QAtResult res;
    if ( !reverseOnly ) {
        res.setResultCode( (QAtResult::ResultCode)code );
        QVERIFY( res.resultCode() == (QAtResult::ResultCode)code );
        QCOMPARE( res.result(), result );
        QCOMPARE( res.verboseResult(), result );
    }
    res.setResult( result );
    QVERIFY( res.resultCode() == (QAtResult::ResultCode)code );
    QCOMPARE( res.result(), result );
    QCOMPARE( res.verboseResult(), result );
}

struct QAtCodeInfo
{
    QAtResult::ResultCode   code;
    const char             *name;
};
static QAtCodeInfo const basic_codes[] = {
    {QAtResult::OK,                         "OK"},      // no tr
    {QAtResult::OK,                         "0"},       // no tr
    {QAtResult::Connect,                    "CONNECT"},     // no tr
    {QAtResult::Connect,                    "1"},       // no tr
    {QAtResult::NoCarrier,                  "NO CARRIER"},      // no tr
    {QAtResult::NoCarrier,                  "3"},       // no tr
    {QAtResult::Error,                      "ERROR"},       // no tr
    {QAtResult::Error,                      "4"},       // no tr
    {QAtResult::NoDialtone,                 "NO DIALTONE"},     // no tr
    {QAtResult::NoDialtone,                 "6"},       // no tr
    {QAtResult::Busy,                       "BUSY"},        // no tr
    {QAtResult::Busy,                       "7"},       // no tr
    {QAtResult::NoAnswer,                   "NO ANSWER"},       // no tr
    {QAtResult::NoAnswer,                   "8"}        // no tr
};
static QAtCodeInfo const ext_codes[] = {
    {QAtResult::PhoneFailure,               "phone failure"},       // no tr
    {QAtResult::NoConnectionToPhone,        "no connection to phone"},      // no tr
    {QAtResult::PhoneAdapterLinkReserved,   "phone-adaptor link reserved"},     // no tr
    {QAtResult::OperationNotAllowed,        "operation not allowed"},       // no tr
    {QAtResult::OperationNotSupported,      "operation not supported"},     // no tr
    {QAtResult::PhSimPinRequired,           "PH-SIM PIN required"},     // no tr
    {QAtResult::PhFSimPinRequired,          "PH-FSIM PIN required"},        // no tr
    {QAtResult::PhFSimPukRequired,          "PH-FSIM PUK required"},        // no tr
    {QAtResult::SimNotInserted,             "SIM not inserted"},        // no tr
    {QAtResult::SimPinRequired,             "SIM PIN required"},        // no tr
    {QAtResult::SimPukRequired,             "SIM PUK required"},        // no tr
    {QAtResult::SimFailure,                 "SIM failure"},     // no tr
    {QAtResult::SimBusy,                    "SIM busy"},        // no tr
    {QAtResult::SimWrong,                   "SIM wrong"},       // no tr
    {QAtResult::IncorrectPassword,          "incorrect password"},      // no tr
    {QAtResult::SimPin2Required,            "SIM PIN2 required"},       // no tr
    {QAtResult::SimPuk2Required,            "SIM PUK2 required"},       // no tr
    {QAtResult::MemoryFull,                 "memory full"},     // no tr
    {QAtResult::InvalidIndex,               "invalid index"},       // no tr
    {QAtResult::NotFound,                   "not found"},   // no tr
    {QAtResult::MemoryFailure,              "memory failure"},  // no tr
    {QAtResult::TextStringTooLong,          "text string too long"},    // no tr
    {QAtResult::InvalidCharsInTextString,   "invalid characters in text string"},   // no tr
    {QAtResult::DialStringTooLong,          "dial string too long"},    // no tr
    {QAtResult::InvalidCharsInDialString,   "invalid characters in dial string"},   // no tr
    {QAtResult::NoNetworkService,           "no network service"},  // no tr
    {QAtResult::NetworkTimeout,             "network timeout"}, // no tr
    {QAtResult::NetworkNotAllowed,          "network not allowed - emergency calls only"},  // no tr
    {QAtResult::NetPersPinRequired,         "network personalization PIN required"},    // no tr
    {QAtResult::NetPersPukRequired,         "network personalization PUK required"},    // no tr
    {QAtResult::NetSubsetPersPinRequired,   "network subset personalization PIN required"}, // no tr
    {QAtResult::NetSubsetPersPukRequired,   "network subset personalization PUK required"}, // no tr
    {QAtResult::ServProvPersPinRequired,    "service provider personalization PIN required"},   // no tr
    {QAtResult::ServProvPersPukRequired,    "service provider personalization PUK required"},   // no tr
    {QAtResult::CorpPersPinRequired,        "corporate personalization PIN required"},  // no tr
    {QAtResult::CorpPersPukRequired,        "corporate personalization PUK required"},  // no tr
    {QAtResult::HiddenKeyRequired,          "hidden key required"}, // no tr
    {QAtResult::Unknown,                    "unknown"}, // no tr

    {QAtResult::IllegalMS,                  "Illegal MS"},  // no tr
    {QAtResult::IllegalME,                  "Illegal ME"},  // no tr
    {QAtResult::GPRSServicesNotAllowed  ,   "GPRS services not allowed"},   // no tr
    {QAtResult::PLMNNotAllowed,             "PLMN not allowed"},    // no tr
    {QAtResult::LocationAreaNotAllowed,     "Location area not allowed"},   // no tr
    {QAtResult::RoamingNotAllowed,          "Roaming not allowed in this location area"},   // no tr
    {QAtResult::ServiceOptionNotSupported,  "service option not supported"},    // no tr
    {QAtResult::ServiceOptionNotSubscribed, "requested service option not subscribed"}, // no tr
    {QAtResult::ServiceOptionOutOfOrder,    "service option temporarily out of order"}, // no tr
    {QAtResult::UnspecifiedGPRSError,       "unspecified GPRS error"},  // no tr
    {QAtResult::PDPAuthenticationFailure,   "PDP authentication failure"},  // no tr
    {QAtResult::InvalidMobileClass,         "invalid mobile class"},    // no tr
    // no tr
    {QAtResult::VBSVGCSNotSupported,        "VBS/VGCS not supported by the network"},   // no tr
    {QAtResult::NoServiceSubscriptionOnSim, "No service subscription on SIM"},  // no tr
    {QAtResult::NoSubscriptionForGroupId,   "No subscription for group ID"},    // no tr
    {QAtResult::GroupIdNotActivatedOnSim,   "Group Id not activated on SIM"},   // no tr
    {QAtResult::NoMatchingNotification,     "No matching notification"},    // no tr
    {QAtResult::VBSVGCSCallAlreadyPresent,  "VBS/VGCS call already present"},   // no tr
    {QAtResult::Congestion,                 "Congestion"},  // no tr
    {QAtResult::NetworkFailure,             "Network failure"}, // no tr
    {QAtResult::UplinkBusy,                 "Uplink busy"}, // no tr
    {QAtResult::NoAccessRightsForSimFile,   "No access rights for SIM file"},   // no tr
    {QAtResult::NoSubscriptionForPriority,  "No subscription for priority"},    // no tr
    {QAtResult::OperationNotApplicable,     "operation not applicable or not possible"},    // no tr

    {QAtResult::MEFailure,                  "ME failure"},  // no tr
    {QAtResult::SMSServiceOfMEReserved,     "SMS service of ME reserved"},  // no tr
    {QAtResult::SMSOperationNotAllowed,     "operation not allowed"},   // no tr
    {QAtResult::SMSOperationNotSupported,   "operation not supported"}, // no tr
    {QAtResult::InvalidPDUModeParameter,    "invalid PDU mode parameter"},  // no tr
    {QAtResult::InvalidTextModeParameter,   "invalid text mode parameter"}, // no tr
    {QAtResult::USimNotInserted,            "(U)SIM not inserted"}, // no tr
    {QAtResult::USimPinRequired,            "(U)SIM PIN required"}, // no tr
    {QAtResult::PHUSimPinRequired,          "PH-(U)SIM PIN required"},  // no tr
    {QAtResult::USimFailure,                "(U)SIM failure"},  // no tr
    {QAtResult::USimBusy,                   "(U)SIM busy"}, // no tr
    {QAtResult::USimWrong,                  "(U)SIM wrong"},    // no tr
    {QAtResult::USimPukRequired,            "(U)SIM PUK required"}, // no tr
    {QAtResult::USimPin2Required,           "(U)SIM PIN2 required"},    // no tr
    {QAtResult::USimPuk2Required,           "(U)SIM PUK2 required"},    // no tr
    {QAtResult::SMSMemoryFailure,           "memory failure"},  // no tr
    {QAtResult::InvalidMemoryIndex,         "invalid memory index"},    // no tr
    {QAtResult::MemoryFull,                 "memory full"}, // no tr
    {QAtResult::SMSCAddressUnknown,         "SMSC address unknown"},    // no tr
    {QAtResult::SMSNoNetworkService,        "no network service"},  // no tr
    {QAtResult::SMSNetworkTimeout,          "network timeout"}, // no tr
    {QAtResult::NoCNMAAckExpected,          "no +CNMA acknowledgement expected"},   // no tr
    {QAtResult::UnknownError,               "unknown error"}    // no tr
};
#define num_ext_codes     ((int)( sizeof(ext_codes) / sizeof(QAtCodeInfo) ))

// Test the extended result codes.
void tst_QAtResult::testExtendedResults_data()
{
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("numeric");
    QTest::addColumn<QString>("verbose");

    int index;
    QString numeric, verbose;

    bool used[600];
    memset( used, 0, sizeof(used) );

    for ( index = 0; index < num_ext_codes; ++index ) {
        QAtResult::ResultCode code = ext_codes[index].code;
        if ( code < 300 || code > 500 ) {
            numeric = "+CME ERROR: " + QString::number( code );
            verbose = "+CME ERROR: " + QString( ext_codes[index].name );
        } else {
            numeric = "+CMS ERROR: " + QString::number( code );
            verbose = "+CMS ERROR: " + QString( ext_codes[index].name );
        }
        used[(int)code] = true;
        QTest::newRow( ext_codes[index].name )
            << (int)code << numeric << verbose;
    }

    // Test that the rest are properly recognized as CME vs CMS
    // depending upon the range they fall within.
    for ( index = 0; index < 600; ++index ) {
        if ( !used[index] ) {
            if ( index < 300 || index > 500 ) {
                numeric = "+CME ERROR: " + QString::number( index );
            } else {
                numeric = "+CMS ERROR: " + QString::number( index );
            }
            QTest::newRow( numeric.toLatin1().constData() )
                << index << numeric << numeric;
        }
    }
}
void tst_QAtResult::testExtendedResults()
{
    QFETCH( int, code );
    QFETCH( QString, numeric );
    QFETCH( QString, verbose );

    QAtResult res;
    res.setResultCode( (QAtResult::ResultCode)code );
    QVERIFY( res.resultCode() == (QAtResult::ResultCode)code );
    QCOMPARE( res.result(), verbose );
    QCOMPARE( res.verboseResult(), verbose );

    res.setResult( numeric );
    QVERIFY( res.resultCode() == (QAtResult::ResultCode)code );
    QCOMPARE( res.result(), numeric );
    QCOMPARE( res.verboseResult(), verbose );

    res.setResult( verbose );
    QVERIFY( res.resultCode() == (QAtResult::ResultCode)code );
    QCOMPARE( res.result(), verbose );
    QCOMPARE( res.verboseResult(), verbose );
}

class TestUserData : public QAtResult::UserData
{
public:
    TestUserData( tst_QAtResult *parent ) { this->parent = parent; }
    ~TestUserData() { parent->userDataDeleted = true; }

private:
    tst_QAtResult *parent;
};

// Test that user data can be attached, and is deleted when the QAtResult is.
void tst_QAtResult::testUserData()
{
    QAtResult::UserData *data = new TestUserData( this );
    QAtResult *res = new QAtResult();
    res->setUserData( data );
    QVERIFY( res->userData() == data );
    userDataDeleted = false;
    delete res;
    QVERIFY( userDataDeleted );
}

// Test creation of QAtResultParser objects.
void tst_QAtResult::testParserCreate()
{
    // Create a parser for an empty result and check that defaults are returned.
    QAtResult result1;
    QAtResultParser parser1( result1 );
    QVERIFY( !parser1.next( "+CABC:" ) );
    QVERIFY( parser1.line().isEmpty() );
    QVERIFY( parser1.readNumeric() == 0 );
    QCOMPARE( parser1.readString(), QString("") );
    QCOMPARE( parser1.readNextLine(), QString("") );

    // Create a parser for an error result and check that defaults are returned.
    QAtResult result2;
    result2.setResultCode( QAtResult::OperationNotAllowed );
    QAtResultParser parser2( result2 );
    QVERIFY( !parser2.next( "+CABC:" ) );
    QVERIFY( parser2.line().isEmpty() );
    QVERIFY( parser2.readNumeric() == 0 );
    QCOMPARE( parser2.readString(), QString("") );
    QCOMPARE( parser2.readNextLine(), QString("") );
}

// Test QAtResultParser::next().
void tst_QAtResult::testParserNext_data()
{
    QTest::addColumn<QString>("content");
    QTest::addColumn<QString>("prefix");
    QTest::addColumn<QString>("result1");
    QTest::addColumn<QString>("result2");
    QTest::addColumn<QString>("result3");

    QTest::newRow( "CCFC" )
        << "+CCFC: 0,1\n+CCFC: 0,2"
        << "+CCFC:"
        << "0,1"
        << "0,2"
        << "";

    QTest::newRow( "XCFC" )
        << "+CCFC: 0,1\n+CCFC: 0,2"
        << "+XCFC:"
        << ""
        << ""
        << "";

    QTest::newRow( "XCFC 2" )
        << "+CCFC: 0,1\n+XCFC: 0,2"
        << "+XCFC:"
        << "0,2"
        << ""
        << "";

    QTest::newRow( "Blank lines" )
        << "\n+CCFC: 0,1\n+CCFC: 0,2\n\n"
        << "+CCFC:"
        << "0,1"
        << "0,2"
        << "";

    QTest::newRow( "Mangled" )
        << "\n+CCFC: 0,1 +CCFC: 0,2\n\n"
        << "+CCFC:"
        << "0,1 +CCFC: 0,2"
        << ""
        << "";
}
void tst_QAtResult::testParserNext()
{
    QFETCH( QString, content );
    QFETCH( QString, prefix );
    QFETCH( QString, result1 );
    QFETCH( QString, result2 );
    QFETCH( QString, result3 );

    QAtResult result;
    result.setContent( content );
    QAtResultParser parser( result );
    if ( !result1.isEmpty() ) {
        QVERIFY( parser.next( prefix ) );
        QCOMPARE( parser.line(), result1 );
        if ( !result2.isEmpty() ) {
            QVERIFY( parser.next( prefix ) );
            QCOMPARE( parser.line(), result2 );
            if ( !result3.isEmpty() ) {
                QVERIFY( parser.next( prefix ) );
                QCOMPARE( parser.line(), result3 );
                QVERIFY( !parser.next( prefix ) );
            } else {
                QVERIFY( !parser.next( prefix ) );
            }
        } else {
            QVERIFY( !parser.next( prefix ) );
        }
    } else {
        QVERIFY( !parser.next( prefix ) );
    }
}

// Test that resetting will take us back to the start.
void tst_QAtResult::testParserReset_data()
{
    testParserNext_data();
}
void tst_QAtResult::testParserReset()
{
    QFETCH( QString, content );
    QFETCH( QString, prefix );
    QFETCH( QString, result1 );
    QFETCH( QString, result2 );
    QFETCH( QString, result3 );

    QAtResult result;
    result.setContent( content );
    QAtResultParser parser( result );
    if ( !result1.isEmpty() ) {
        QVERIFY( parser.next( prefix ) );
        QCOMPARE( parser.line(), result1 );
        parser.reset();
        QVERIFY( parser.next( prefix ) );
        QCOMPARE( parser.line(), result1 );
        if ( !result2.isEmpty() ) {
            QVERIFY( parser.next( prefix ) );
            QCOMPARE( parser.line(), result2 );
            parser.reset();
            QVERIFY( parser.next( prefix ) );
            QCOMPARE( parser.line(), result1 );
            QVERIFY( parser.next( prefix ) );
            QCOMPARE( parser.line(), result2 );
            if ( !result3.isEmpty() ) {
                QVERIFY( parser.next( prefix ) );
                QCOMPARE( parser.line(), result3 );
                QVERIFY( !parser.next( prefix ) );
                parser.reset();
                QVERIFY( parser.next( prefix ) );
                QCOMPARE( parser.line(), result1 );
                QVERIFY( parser.next( prefix ) );
                QCOMPARE( parser.line(), result2 );
                QVERIFY( parser.next( prefix ) );
                QCOMPARE( parser.line(), result3 );
                QVERIFY( !parser.next( prefix ) );
            } else {
                QVERIFY( !parser.next( prefix ) );
            }
        } else {
            QVERIFY( !parser.next( prefix ) );
        }
    } else {
        QVERIFY( !parser.next( prefix ) );
    }
}

QTEST_MAIN( tst_QAtResult )

#include "tst_qatresult.moc"
