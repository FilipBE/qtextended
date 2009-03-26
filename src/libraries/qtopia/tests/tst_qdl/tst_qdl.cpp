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

#include <QDL>
#include <QDLClient>
#include <QDLEditClient>
#include <QDLBrowserClient>
#include <QDSData>
#include <QtopiaApplication>

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QWidget>
#include <QTextEdit>
#include <QTextBrowser>
#include <QUniqueId>

static const int            NUM_QDL_WIDGETS             =   3;
static const char* const    TSTQDL_LEVEL0_WIDGET_NAME   =   "QDL Client A";
static const char* const    TSTQDL_LEVEL1_WIDGET_NAME   =   "QDL Client B";
static const char* const    TSTQDL_LEVEL2_WIDGET_NAME   =   "QDL Client C";

static const QString        TSTQDL_LEVEL0_LNK_SERVICE
                                =    QString( "ApplicationA" );
static const QByteArray     TSTQDL_LEVEL0_LNK_DATA
                                =    QByteArray( "ApplicationA_Data" );
static const QString        TSTQDL_LEVEL0_LNK_DESC
                                =    QString( "ApplicationA Description" );
static const QString        TSTQDL_LEVEL0_LNK_ICON
                                =    QString( "ApplicationA_Icon" );

static const QString        TSTQDL_LEVEL1_LNK_SERVICE
                                =    QString( "ApplicationB" );
static const QByteArray     TSTQDL_LEVEL1_LNK_DATA
                                =    QByteArray( "ApplicationB_Data" );
static const QString        TSTQDL_LEVEL1_LNK_DESC
                                =    QString( "ApplicationB Description" );
static const QString        TSTQDL_LEVEL1_LNK_ICON
                                =    QString( "ApplicationB_Icon" );

static const QString        TSTQDL_LEVEL2_LNK_SERVICE
                                =    QString( "ApplicationC" );
static const QByteArray     TSTQDL_LEVEL2_LNK_DATA
                                =    QByteArray( "ApplicationC_Data" );
static const QString        TSTQDL_LEVEL2_LNK_DESC
                                =    QString( "ApplicationC Description" );
static const QString        TSTQDL_LEVEL2_LNK_ICON
                                =    QString( "ApplicationC_Icon" );



//TESTED_CLASS=QDL,QDLLink,QDLClient,QDLEditClient
//TESTED_FILES=src/libraries/qtopia/qdl.cpp,src/libraries/qtopia/qdllink.cpp,src/libraries/qtopia/qdlclient.cpp,src/libraries/qtopia/qdleditclient.cpp

/*
    The tst_QDL class provides unit tests for the QDL class and
    closely related classes.
*/
class tst_QDL : public QObject
{
    Q_OBJECT
public:
    tst_QDL();

protected slots:
    void init();
    void cleanup();

private slots:

    // QDL class test cases
    void qdl_clients();
    void qdl_loadAndSaveLinks();

    // QDLLink class test cases
    void qdllink_construction();
    void qdllink_constructionDataObject();
    void qdllink_parameterAdjustment();
    void qdllink_breaklink();
    void qdllink_toQDSData();

    // QDLClient test cases
    void qdlclient_hint();
    void qdlclient_addLink();
    void qdlclient_setLink();
    void qdlclient_removeLink();
    void qdlclient_linkAnchorText();
    void qdlclient_loadAndSaveLinks();
    void qdlclient_link();
    void qdlclient_clearLinks();
    void qdlclient_breakLink();

    // QDLEditClient test cases
    void qdleditclient_hint();
    void qdleditclient_addLink();
    void qdleditclient_setLink();
    void qdleditclient_removeLink();
    void qdleditclient_verifyLinks();

private:
    QWidget*    mParent;
    QTextEdit*  mTextEditA;
    QTextEdit*  mTextEditB;
};

QTEST_APP_MAIN( tst_QDL, QtopiaApplication )
#include "tst_qdl.moc"

tst_QDL::tst_QDL()
:   mParent( 0 ), mTextEditA( 0 ), mTextEditB( 0 )
{
}

/*?
    Initialisation before each test function.
    Constructs a hierarchy of widgets with associated QDL clients
    and links.
*/
void tst_QDL::init()
{
    // Create a QDL client hierarchy with multiple levels
    mParent = new QWidget();
    QDLClient* clientLevel0
        = new QDLClient( mParent, TSTQDL_LEVEL0_WIDGET_NAME );

    mTextEditA = new QTextEdit( mParent );
    QDLEditClient* clientLevel1
        = new QDLEditClient( mTextEditA, TSTQDL_LEVEL1_WIDGET_NAME );

    mTextEditB = new QTextEdit( mTextEditA );
    QDLEditClient* clientLevel2
        = new QDLEditClient( mTextEditB, TSTQDL_LEVEL2_WIDGET_NAME );

    // Add some links to the QDL clients, the parameters can really be anything for these 
    // test cases
    QDSData data0 = QDLLink( TSTQDL_LEVEL0_LNK_SERVICE,
                             TSTQDL_LEVEL0_LNK_DATA,
                             TSTQDL_LEVEL0_LNK_DESC,
                             TSTQDL_LEVEL0_LNK_ICON ).toQDSData();
    clientLevel0->addLink( data0 );

    QDSData data1 = QDLLink( TSTQDL_LEVEL1_LNK_SERVICE,
                             TSTQDL_LEVEL1_LNK_DATA,
                             TSTQDL_LEVEL1_LNK_DESC,
                             TSTQDL_LEVEL1_LNK_ICON ).toQDSData();
    clientLevel1->addLink( data1 );

    QDSData data2 = QDLLink( TSTQDL_LEVEL2_LNK_SERVICE,
                             TSTQDL_LEVEL2_LNK_DATA,
                             TSTQDL_LEVEL2_LNK_DESC,
                             TSTQDL_LEVEL2_LNK_ICON ).toQDSData();
    clientLevel2->addLink( data2 );
}

/*?
    Cleanup after each test function.
    Destroys all QDL clients and the widget hierarchy created in init().
*/
void tst_QDL::cleanup()
{
    QList<QDLClient* > list = QDL::clients( mParent );
    foreach( QDLClient* client, list )
        client->clear();

    if (mParent) delete mParent;
    mParent = 0;
    mTextEditA = 0;
    mTextEditB = 0;
}

/*?
    Test copy constructor and assignment operator of QDLLink.
*/
void tst_QDL::qdllink_construction()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Get a link from one of the clients
    QDLLink origLink = list[2]->link( 1 );
    QVERIFY( !origLink.isNull() );

    // Create another link using copy construction
    QDLLink copyLink( origLink );
    QCOMPARE( origLink.service(), copyLink.service() );
    QCOMPARE( origLink.data(), copyLink.data() );
    QCOMPARE( origLink.description(), copyLink.description() );
    QCOMPARE( origLink.icon(), copyLink.icon() );

    // Create another link using assignment
    QDLLink assignLink = origLink;
    QCOMPARE( origLink.service(), assignLink.service() );
    QCOMPARE( origLink.data(), assignLink.data() );
    QCOMPARE( origLink.description(), assignLink.description() );
    QCOMPARE( origLink.icon(), assignLink.icon() );
}

/*?
    Test storing and retrieving a QDLLink through a QDSData object.
*/
void tst_QDL::qdllink_constructionDataObject()
{
    // Key used to recreate stored link
    QUniqueId key;

    // Create a QDLLink data object
    {
        QDSData linkData = QDLLink( TSTQDL_LEVEL2_LNK_SERVICE,
                                    TSTQDL_LEVEL2_LNK_DATA,
                                    TSTQDL_LEVEL2_LNK_DESC,
                                    TSTQDL_LEVEL2_LNK_ICON ).toQDSData();
        key = linkData.store();
        QVERIFY( key != QUniqueId() );

        // Create a QDLLink from the data object
        QDLLink link( linkData );

        // Check that they are the same
        QVERIFY( link.service() == TSTQDL_LEVEL2_LNK_SERVICE );
        QVERIFY( link.data() == TSTQDL_LEVEL2_LNK_DATA );
        QVERIFY( link.description() == TSTQDL_LEVEL2_LNK_DESC );
        QVERIFY( link.icon() == TSTQDL_LEVEL2_LNK_ICON );
        QVERIFY( !link.isBroken() );
    }

    // Now create one from a stored version of the data object
    QDSData linkData( key );

    // Create a QDLLink from the data object
    QDLLink link( linkData );

    // Check that they are the same
    QVERIFY( link.service() == TSTQDL_LEVEL2_LNK_SERVICE );
    QVERIFY( link.data() == TSTQDL_LEVEL2_LNK_DATA );
    QVERIFY( link.description() == TSTQDL_LEVEL2_LNK_DESC );
    QVERIFY( link.icon() == TSTQDL_LEVEL2_LNK_ICON );
    QVERIFY( !link.isBroken() );

    // Clear the link data object
    linkData.remove();
}

/*?
    Basic set-get tests on QDLLink.
*/
void tst_QDL::qdllink_parameterAdjustment()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Get a link from one of the clients
    QDLLink origLink = list[1]->link( 1 );
    QVERIFY( !origLink.isNull() );
    QDLLink copyLink( origLink );

    // Adjust the parameters
    QString str = "A new string";
    copyLink.setService( str );
    copyLink.setDescription( str );
    copyLink.setIcon( str );
    QByteArray data = "Some new data";
    copyLink.setData( data );

    // Test that the link parameters have changed
    QCOMPARE( copyLink.service(), str );
    QCOMPARE( copyLink.data(), data );
    QCOMPARE( copyLink.description(), str );
    QCOMPARE( copyLink.icon(), str );

    // Test that the orignal link parameters have not changed
    QCOMPARE( origLink.service(), TSTQDL_LEVEL1_LNK_SERVICE );
    QCOMPARE( origLink.data(), TSTQDL_LEVEL1_LNK_DATA );
    QCOMPARE( origLink.description(), TSTQDL_LEVEL1_LNK_DESC );
    QCOMPARE( origLink.icon(), TSTQDL_LEVEL1_LNK_ICON );
}

/*?
    Break and unbreak a QDLLink and ensure it behaves correctly.
*/
void tst_QDL::qdllink_breaklink()
{
    // Key used to recreate stored link
    QUniqueId key;

    // Create a QDLLink data object
    {
        QDSData linkData = QDLLink( TSTQDL_LEVEL2_LNK_SERVICE,
                                    TSTQDL_LEVEL2_LNK_DATA,
                                    TSTQDL_LEVEL2_LNK_DESC,
                                    TSTQDL_LEVEL2_LNK_ICON ).toQDSData();
        key = linkData.store();
        QVERIFY( key != QUniqueId() );

        // Create a QDLLink from the data object
        QDLLink link( linkData );

        // Check that they are the same
        QVERIFY( link.service() == TSTQDL_LEVEL2_LNK_SERVICE );
        QVERIFY( link.data() == TSTQDL_LEVEL2_LNK_DATA );
        QVERIFY( link.description() == TSTQDL_LEVEL2_LNK_DESC );
        QVERIFY( link.icon() == TSTQDL_LEVEL2_LNK_ICON );
        QVERIFY( !link.isBroken() );
    }

    // Now break the link
    {
        QDSData linkData( key );
        QDLLink link( linkData );
        link.setBroken( true );

        // Check that the link was broken
        QVERIFY( link.service() == TSTQDL_LEVEL2_LNK_SERVICE );
        QVERIFY( link.data() == TSTQDL_LEVEL2_LNK_DATA );
        QVERIFY( link.description() == TSTQDL_LEVEL2_LNK_DESC );
        QVERIFY( link.icon() == TSTQDL_LEVEL2_LNK_ICON );
        QVERIFY( link.isBroken() );

        // now modify the link data in the QDSDataStore
        linkData.modify( link.toQDSData().data() );
    }

    // Now check that a reconstructed link is broken
    {
        QDSData linkData( key );
        QDLLink link( linkData );

        // Check that the link was broken
        QVERIFY( link.service() == TSTQDL_LEVEL2_LNK_SERVICE );
        QVERIFY( link.data() == TSTQDL_LEVEL2_LNK_DATA );
        QVERIFY( link.description() == TSTQDL_LEVEL2_LNK_DESC );
        QVERIFY( link.icon() == TSTQDL_LEVEL2_LNK_ICON );
        QVERIFY( link.isBroken() );

        // now modify the link data in the QDSDataStore
        linkData.modify( link.toQDSData().data() );
    }

    // Now unbreak the link
    {
        QDSData linkData( key );
        QDLLink link( linkData );
        link.setBroken( false );

        // Check that the link was broken
        QVERIFY( link.service() == TSTQDL_LEVEL2_LNK_SERVICE );
        QVERIFY( link.data() == TSTQDL_LEVEL2_LNK_DATA );
        QVERIFY( link.description() == TSTQDL_LEVEL2_LNK_DESC );
        QVERIFY( link.icon() == TSTQDL_LEVEL2_LNK_ICON );
        QVERIFY( !link.isBroken() );

        // now modify the link data in the QDSDataStore
        linkData.modify( link.toQDSData().data() );
    }

    // Now check that a reconstructed link is not broken
    {
        QDSData linkData( key );
        QDLLink link( linkData );

        // Check that the link was broken
        QVERIFY( link.service() == TSTQDL_LEVEL2_LNK_SERVICE );
        QVERIFY( link.data() == TSTQDL_LEVEL2_LNK_DATA );
        QVERIFY( link.description() == TSTQDL_LEVEL2_LNK_DESC );
        QVERIFY( link.icon() == TSTQDL_LEVEL2_LNK_ICON );
        QVERIFY( !link.isBroken() );

        // now modify the link data in the QDSDataStore
        linkData.modify( link.toQDSData().data() );
    }

    // Clear the link data object
    QDSData( key ).remove();
}

/*?
    Ensure converting a QDLLink into a QDSData object results in the
    same data as streaming to a QByteArray.
*/
void tst_QDL::qdllink_toQDSData()
{
    // Create the link
    QDLLink link( TSTQDL_LEVEL2_LNK_SERVICE,
                  TSTQDL_LEVEL2_LNK_DATA,
                  TSTQDL_LEVEL2_LNK_DESC,
                  TSTQDL_LEVEL2_LNK_ICON );

    // Convert the link to a data object
    QDSData linkData = link.toQDSData();

    // Check the data in the data object is the same streaming the link into 
    // a bytearray
    QByteArray array;
    {
        QDataStream ds( &array, QIODevice::WriteOnly );
        ds << link;
    }
    QVERIFY( array == linkData.data() );
}

/*?
    Ensure QDL::clients() returns correct client list.
*/
void tst_QDL::qdl_clients()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Check that the correct widgets were obtained. Note that this implementation
    // implies an order which isn't necessary.
    QVERIFY( list[0]->objectName() == TSTQDL_LEVEL0_WIDGET_NAME );
    QVERIFY( list[1]->objectName() == TSTQDL_LEVEL1_WIDGET_NAME );
    QVERIFY( list[2]->objectName() == TSTQDL_LEVEL2_WIDGET_NAME );
}

/*?
    Ensure links can be correctly saved and loaded.
*/
void tst_QDL::qdl_loadAndSaveLinks()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Save the original links
    QString origLinks;
    QDL::saveLinks( origLinks, list );

    // Load them back into the clients
    QDL::loadLinks( origLinks, list );

    // Check that the saved links equals the original links
    QString savedLinks;
    QDL::saveLinks( savedLinks, list );
    QVERIFY( origLinks == savedLinks);
}

/*?
    Set-get tests on QDLClient hint.
*/
void tst_QDL::qdlclient_hint()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Check that the client's hint is originally empty
    QString hint = list[0]->hint();
    QVERIFY( hint.isNull() );

    // Set the client's hint and make sure it's been set
    hint = "A new hint";
    QString str = "Some other text";
    list[0]->setHint(hint);
    QVERIFY( list[0]->hint() == hint );
}

/*?
    Verify that links can be added to a QDLClient.
*/
void tst_QDL::qdlclient_addLink()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Add 100 more links to the third client
    const int linksToAdd = 100;
    for ( int i = 0; i < linksToAdd; ++i ) {
        QDSData data = QDLLink( TSTQDL_LEVEL2_LNK_SERVICE,
                                TSTQDL_LEVEL2_LNK_DATA,
                                TSTQDL_LEVEL2_LNK_DESC,
                                TSTQDL_LEVEL2_LNK_ICON ).toQDSData();
        list[2]->addLink( data );
    }

    // Check that the client has 101 links
    QVERIFY( list[2]->linkIds().count() == linksToAdd + 1 );
}

/*?
    Set-get tests on links through a QDLClient.
*/
void tst_QDL::qdlclient_setLink()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Swap the 70th link in the third client around
    const unsigned int swap = 1;
    QDLLink link = list[2]->link( swap );
    QDLLink copyLink(link);

    QString str = "Some other text";
    QByteArray data = "Some new data";
    link.setService( str );
    link.setData( data );
    link.setDescription( str );
    link.setIcon( str );

    list[2]->setLink( swap, link );

    // Check that the link has been set correctly
    QDLLink check = list[2]->link( swap );
    QVERIFY( check.service() == str );

    // Check what happens if you exceed the number of links
    list[2]->setLink( list[2]->linkIds().count() + 1, link );
    QDLLink empty = list[2]->link( list[2]->linkIds().count() + 1 );
    QVERIFY( empty.isNull() );

    // Swap the original link back in
    list[2]->setLink( swap, copyLink );
    check = list[2]->link( swap );
    QVERIFY( check.service() == TSTQDL_LEVEL2_LNK_SERVICE );
    QVERIFY( check.data() == TSTQDL_LEVEL2_LNK_DATA );
    QVERIFY( check.description() == TSTQDL_LEVEL2_LNK_DESC );
    QVERIFY( check.icon() == TSTQDL_LEVEL2_LNK_ICON );
}

/*?
    Test that QDL clients can have links removed from them.
*/
void tst_QDL::qdlclient_removeLink()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Try removing a link which doesn't exist, and check that nothing has changed
    int initialCount = list[0]->linkIds().count();
    list[0]->removeLink( initialCount + 1 );
    QVERIFY( list[0]->linkIds().count() == initialCount );

    // Now remove every link in the client. ACTUALLY doing this isn't correct because
    // currently the QDL client stores the links in a map, so if we remove a link in the
    // middle beforehand there will be a gap in the lids.
    for ( int i = 0; i < initialCount; ++i) {
        list[0]->removeLink( initialCount - i );
    }

    // Check that no links remain in the client
    QVERIFY( list[0]->linkIds().count() == 0 );
}

/*?
    Compare QDLClient::linkAnchorText() with expected values.
*/
void tst_QDL::qdlclient_linkAnchorText()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Create the anchor text without icon
    QString anchor = list[0]->linkAnchorText( 1, true );
    QString expectedAnchor = "<a href=\"QDL://QDL Client A:1\">";
    expectedAnchor += TSTQDL_LEVEL0_LNK_DESC;
    expectedAnchor += "</a>&nbsp;";
    QVERIFY( anchor == expectedAnchor );

    // Create the anchor text
    anchor = list[0]->linkAnchorText( 1 );
    expectedAnchor = "<a href=\"QDL://QDL Client A:1\">";
    expectedAnchor += TSTQDL_LEVEL0_LNK_DESC;
    expectedAnchor += "<img width=\"12\" height=\"12\"";
    expectedAnchor += " src=\"";
    expectedAnchor += TSTQDL_LEVEL0_LNK_ICON;
    expectedAnchor += "\"></a>&nbsp;";
    QVERIFY( anchor == expectedAnchor );
}

/*?
    Ensure links can be correctly saved and loaded via QDLClient.
*/
void tst_QDL::qdlclient_loadAndSaveLinks()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Save the original links
    QByteArray origLinks;
    {
        QDataStream ds( &origLinks, QIODevice::WriteOnly );
        list[0]->saveLinks( ds );
    }

    // Load them back into the clients
    {
        QDataStream ds( &origLinks, QIODevice::ReadOnly );
        list[0]->loadLinks( ds );
    }

    // Check that the saved links equals the original links
    QByteArray savedLinks;
    {
        QDataStream ds( &savedLinks, QIODevice::WriteOnly );
        list[0]->saveLinks( ds );
    }
    QVERIFY( origLinks == savedLinks);
}

/*?
    Set-get tests on links.
*/
void tst_QDL::qdlclient_link()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Get a link that should exist and check it's parameters
    QList<int> ids = list[1]->linkIds();
    QDLLink l = list[1]->link( ids[ 0 ] );
    QVERIFY( !l.isNull() );
    QVERIFY( l.service() == TSTQDL_LEVEL1_LNK_SERVICE );
    QVERIFY( l.data() == TSTQDL_LEVEL1_LNK_DATA );
    QVERIFY( l.description() == TSTQDL_LEVEL1_LNK_DESC );
    QVERIFY( l.icon() == TSTQDL_LEVEL1_LNK_ICON );
    QVERIFY( !l.isBroken() );

    // Get a link that shouldn't exist
    QDLLink l2 = list[1]->link( 0 );
    QVERIFY( l2.isNull() );
    QVERIFY( l2.service() == QString() );
    QVERIFY( l2.data() == QByteArray() );
    QVERIFY( l2.description() == QString() );
    QVERIFY( l2.icon() == QString() );
    QVERIFY( l2.isBroken() );
}

/*?
    Clear all links on a client and test that no links exist.
*/
void tst_QDL::qdlclient_clearLinks()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Clear the second widget
    list[1]->clear();

    // Check that the second widget now has no widgets
    QVERIFY( list[1]->linkIds().count() == 0 );
}

/*?
    Break links and ensure they are reported as broken.
*/
void tst_QDL::qdlclient_breakLink()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );

    // Add 1 more link to the second client
    QDSData data = QDLLink( TSTQDL_LEVEL2_LNK_SERVICE,
                            TSTQDL_LEVEL2_LNK_DATA,
                            TSTQDL_LEVEL2_LNK_DESC,
                            TSTQDL_LEVEL2_LNK_ICON ).toQDSData();
    list[1]->addLink( data );

    // Check that the client has 101 links
    QList<int> ids = list[1]->linkIds();
    QVERIFY( ids.count() == 2 );

    // Now break the first link and check that it was broken, and not the
    // second link
    list[1]->breakLink( ids[0], true );
    QVERIFY( list[1]->link( ids[0] ).isBroken() );
    QVERIFY( !list[1]->link( ids[1] ).isBroken() );

    // Now unbreak the first link and check that it was fixed
    list[1]->breakLink( ids[0], false );
    QVERIFY( !list[1]->link( ids[0] ).isBroken() );
    QVERIFY( !list[1]->link( ids[1] ).isBroken() );
}

/*?
    Set-get tests on client hint.
*/
void tst_QDL::qdleditclient_hint()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );
    QDLEditClient* widgetClient = qobject_cast<QDLEditClient*>( list[1] );

    // Check that the client's hint is originally empty (will be because
    //  we've selected no text in the widget.
    QString hint = widgetClient->hint();
    QVERIFY( hint.isNull() );

    // Set the client's hint and make sure it's been set
    hint = "A new hint";
    QString str = "Some other text";
    widgetClient->setHint(hint);
    QVERIFY( widgetClient->hint() == hint );
}

/*?
    Add many links to a client and ensure they appear in the text of
    the underlying widget.
*/
void tst_QDL::qdleditclient_addLink()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );
    QDLEditClient* widgetClient = qobject_cast<QDLEditClient*>( list[1] );

    // Get the text before
    QString textBefore = mTextEditA->toHtml();
    QVERIFY( !textBefore.isEmpty() );

    // Add 100 more links to the third client
    const int linksToAdd = 100;
    for ( int i=0; i<linksToAdd; ++i ) {
        QDSData data = QDLLink( TSTQDL_LEVEL2_LNK_SERVICE,
                                TSTQDL_LEVEL2_LNK_DATA,
                                TSTQDL_LEVEL2_LNK_DESC,
                                TSTQDL_LEVEL2_LNK_ICON ).toQDSData();
        widgetClient->addLink( data );
    }

    // Check that the client has 101 links
    QList<int> ids = widgetClient->linkIds();
    QVERIFY( ids.count() == linksToAdd + 1 );

    // Get the text after and check that each link is included in the text
    QString textAfter = mTextEditA->toHtml();
    QVERIFY( !textAfter.isEmpty() );
    for ( QList<int>::ConstIterator cit=ids.begin(); cit!=ids.end(); ++cit ) {
        QString pattern = QString(":") + QString::number( *cit ) + QString("\"");
        int p = textAfter.indexOf( pattern );
        QVERIFY( p != -1 );
    }
}

/*?
    Set some links through a client and ensure they appear in the text of
    the underlying widget.
*/
void tst_QDL::qdleditclient_setLink()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );
    QDLEditClient* widgetClient = qobject_cast<QDLEditClient*>( list[1] );

    // Get the text before
    QString textBefore = mTextEditA->toHtml();
    QVERIFY( !textBefore.isEmpty() );

    // Swap the first link in the third client around
    const unsigned int swap = 1;
    QDLLink link = widgetClient->link( swap );
    QDLLink copyLink(link);

    QString str = "Some other text";
    QByteArray data = "Some new data";
    link.setService( str );
    link.setData( data );
    link.setDescription( str );
    link.setIcon( str );

    widgetClient->setLink( swap, link );

    // Check that the link has been set correctly
    QDLLink check = widgetClient->link( swap );
    QVERIFY( check.service() == str );
    QVERIFY( check.data() == data );
    QVERIFY( check.description() == str );
    QVERIFY( check.icon() == str );

    // Check what happens if you exceed the number of links
    widgetClient->setLink( widgetClient->linkIds().count() + 1, link );
    QDLLink empty = widgetClient->link( widgetClient->linkIds().count() + 1 );
    QVERIFY( empty.isNull() );

    // Get the text after and check that it contains the new description
    QString textAfter = mTextEditA->toHtml();
    QVERIFY( !textAfter.isEmpty() );
    QVERIFY( textAfter.contains( str ) );

    // Swap the original link back in
    widgetClient->setLink( swap, copyLink );
    check = widgetClient->link( swap );
    QVERIFY( check.service() == TSTQDL_LEVEL1_LNK_SERVICE );
    QVERIFY( check.data() == TSTQDL_LEVEL1_LNK_DATA );
    QVERIFY( check.description() == TSTQDL_LEVEL1_LNK_DESC );
    QVERIFY( check.icon() == TSTQDL_LEVEL1_LNK_ICON );
}

/*?
    Remove some links through a client and ensure they no longer appear in the text of
    the underlying widget.
*/
void tst_QDL::qdleditclient_removeLink()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );
    QDLEditClient* widgetClient
        = qobject_cast<QDLEditClient*>( list[1] );

    // Get the text before
    QString textBefore = mTextEditA->toHtml();
    QVERIFY( !textBefore.isEmpty() );

    // Try removing a link which doesn't exist, and check that 
    // nothing has changed
    int initialCount = widgetClient->linkIds().count();
    widgetClient->removeLink( initialCount + 1 );
    QVERIFY( widgetClient->linkIds().count() == initialCount );

    QList<int> ids = widgetClient->linkIds();
    for ( QList<int>::ConstIterator cit=ids.begin(); cit!=ids.end(); ++cit ) {
        widgetClient->removeLink( *cit );
    }

    // Check that no links remain in the client
    QVERIFY( widgetClient->linkIds().count() == 0 );

    // Get the text after and check that it's basically empty (might contain a
    // few html tags
    QString textAfter = mTextEditA->toHtml();
    QVERIFY( !textAfter.contains( "QDL:" ) );
}

/*?
    Ensure the verifyLinks() method works.
*/
void tst_QDL::qdleditclient_verifyLinks()
{
    // Get the client list for the root widget
    QList<QDLClient* > list = QDL::clients( mParent );

    // Check that we obtained all the widgets
    QVERIFY( list.count() == NUM_QDL_WIDGETS );
    QDLEditClient* widgetClient
        = qobject_cast<QDLEditClient*>( list[1] );

    // Get the text before
    QString textBefore = mTextEditA->toHtml();
    QVERIFY( !textBefore.isEmpty() );
    QVERIFY( !textBefore.contains( "Broken" ) );

    // Add a false link
    QString falseLinkHref = "QDL://";
    falseLinkHref += TSTQDL_LEVEL1_WIDGET_NAME;
    falseLinkHref += ":1034";
    QString falseLink = "<a href=\"";
    falseLink += falseLinkHref;
    falseLink += "\">FalseLink</a>";
    textBefore += falseLink;
    mTextEditA->setText( textBefore );

    // Check that it exists in the widget text
    textBefore = mTextEditA->toHtml();
    QVERIFY( !textBefore.isEmpty() );
    QVERIFY( !textBefore.contains( "Broken" ) );
    QVERIFY( textBefore.contains( falseLinkHref ) );

    // Add another link to the client and break it
    // Add 1 more link to the second client
    QDSData data = QDLLink( TSTQDL_LEVEL2_LNK_SERVICE,
                            TSTQDL_LEVEL2_LNK_DATA,
                            TSTQDL_LEVEL2_LNK_DESC,
                            TSTQDL_LEVEL2_LNK_ICON ).toQDSData();

    widgetClient->addLink( data );

    // Check that the client has 2 links
    QList<int> ids = list[1]->linkIds();
    QVERIFY( ids.count() == 2 );

    // Now break the first link and check that it was broken, and not the
    // second link
    list[1]->breakLink( ids[0], true );
    QVERIFY( list[1]->link( ids[0] ).isBroken() );
    QVERIFY( !list[1]->link( ids[1] ).isBroken() );

    // Now verify the links
    widgetClient->verifyLinks();

    // Get the text after
    QString textAfter = mTextEditA->toHtml();

    QVERIFY( !textAfter.isEmpty() );
    QVERIFY( textAfter.contains( "Broken" ) );

    /* Since FalseLink was not added via QDLLink, verifying it is expensive, and is only
     * done if QDL is compiled with QDL_STRICT_LINK_CHECKS defined. */
#ifdef QDL_STRICT_LINK_CHECKS
    QVERIFY( textAfter.contains( "FalseLink (Broken Link)" ) );
#endif
}

