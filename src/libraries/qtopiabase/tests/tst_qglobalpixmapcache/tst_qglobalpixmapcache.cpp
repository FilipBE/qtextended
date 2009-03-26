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

#include <QGlobalPixmapCache>
#include <QObject>
#include <QTest>
#include <QPainter>
#include <qtopianamespace.h>
#include <QtopiaApplication>
#include <QString>

#include <shared/qtopiaunittest.h>

//TESTED_CLASS=QGlobalPixmapCache
//TESTED_FILES=src/libraries/qtopiabase/qglobalpixmapcache.h

/*
    The tst_QGlobalPixmapCache class provides unit tests for the QGlobalPixmapCache class.
*/
class tst_QGlobalPixmapCache : public QObject
{
    Q_OBJECT
public:

private slots:
    void initTestCase();

    void insert_remove_bug();

    void insert_find();
    void insert_find_data();

    void insert_remove();
    void insert_remove_data();

/* These tests are disabled until the bug related to insert_remove_bug is
   fixed - until then, these cause the test to hang. */
private:
    void notFillCache();
    void fillCache();
};

QTEST_APP_MAIN( tst_QGlobalPixmapCache, QtopiaApplication )
#include "tst_qglobalpixmapcache.moc"

/*?
    Initialisation called before the first test.
*/
void tst_QGlobalPixmapCache::initTestCase()
{
}

/*?
    Test function specifically for testing Hooligan Task 143439.
    This test:
        * Checks that a pixmap does not exist for a specific key.
        * Adds a valid, non-null pixmap for that specific key.
        * Checks that the pixmap can be retrieved with the key and
          it matches the added pixmap.
        * Removes the pixmap with the key.
        * Checks that there is no longer a pixmap in the cache
          for the key.
*/
void tst_QGlobalPixmapCache::insert_remove_bug()
{
    // Test for a specific bug: pixmap not being removed
    QString key = "some_key_not_appearing_elsewhere";

    // Ensure key is not currently being used
    {
        QPixmap check;
        QVERIFY( !QGlobalPixmapCache::find(key, check) );
        QVERIFY( check.isNull() );
    }

    // Add pixmap
    {
        QPixmap pm(Qtopia::qtopiaDir() + QString( "/pics/callbutton.png"));
        QVERIFY( QGlobalPixmapCache::insert(key, pm) );
        // Ensure it can be found and is correct
        {
            QPixmap check;
            QVERIFY( QGlobalPixmapCache::find(key, check) );
            QCOMPARE( check, pm );
        } 
    }


    // Note that every QPixmap created in this function is now out of scope,
    // so QGlobalPixmapCache::remove has no excuse not to remove pixmap.
    QGlobalPixmapCache::remove(key);


    // Force "flush" the cache by inserting a lot of small images
    QPixmap p(1, 1);
    for(int ii = 0; ii < 150; ++ii) 
        QGlobalPixmapCache::insert(key + QString::number(ii), p);

    {
        QPixmap check;

        QVERIFY( !QGlobalPixmapCache::find(key, check) );
        QVERIFY( check.isNull() );
    }

    for(int ii = 0; ii < 150; ++ii) 
        QGlobalPixmapCache::remove(key + QString::number(ii));
}

/*?
    Data for the insert_find() test function.
    Note that the order of test cases matters, as some tests are to
    do with overwriting previous data.
*/
void tst_QGlobalPixmapCache::insert_find_data()
{
    QTest::addColumn<QString>("key");    // key in cache
    QTest::addColumn<QPixmap>("pixmap"); // pixmap to add to cache
    QTest::addColumn<bool>("overwrite"); // true if key should already
                                         // be used in cache
    QTest::addColumn<bool>("remove");    // true if pixmap should be removed
                                         // after test

    // Test adding with an empty key
    QTest::newRow("empty key")
        << ""
        << QPixmap(Qtopia::qtopiaDir() + QString( "/pics/qtopia.png"))
        << false
        << true;

    // Test a typical case, adding with a regular key
    QTest::newRow("simple1")
        << "some_picture"
        << QPixmap(Qtopia::qtopiaDir() + QString( "/pics/callbutton.png"))
        << false
        << true;

    // Test adding the same key,pixmap pair again, and this time leave it
    // in the cache.
    QTest::newRow("simple2")
        << "some_picture"
        << QPixmap(Qtopia::qtopiaDir() + QString( "/pics/callbutton.png"))
        << false
        << false;

    // Test overwriting an existing cache entry
    QTest::newRow("overwrite")
        << "some_picture"
        << QPixmap(Qtopia::qtopiaDir() + QString( "/pics/qtlogo.png"))
        << true
        << true;

    // Test empty pixmap
    QTest::newRow("empty pixmap")
        << "empty_picture"
        << QPixmap()
        << false
        << true;
}

/*?
    Test function for inserting and retrieving pixmaps from cache.
    This function:
        * Checks that a pixmap either is or is not in cache, depending on
          what the test data says should be the case.
        * Inserts a pixmap into the cache using a specified key.  This
          overwrites a pixmap if one already exists for that key.
        * Checks that the pixmap can be retrieved from the cache using the
          key, and that it matches the original pixmap.
        * Removes the pixmap from the cache if the test data says to do so.
*/
void tst_QGlobalPixmapCache::insert_find()
{
    QFETCH( QString, key );
    QFETCH( QPixmap, pixmap );
    QFETCH( bool, overwrite );
    QFETCH( bool, remove );

    QEXPECT_FAIL("simple2", "Task 143439", Abort);

    // Verify key is or is not already used in cache, depending on what we
    // expect.
    {
        QPixmap check;
        QVERIFY( overwrite && QGlobalPixmapCache::find( key, check )
                 || !overwrite && !QGlobalPixmapCache::find( key, check ) );
    }

    // Ensure insert works correctly
    {
        QPixmap add = pixmap.copy();
        QVERIFY( QGlobalPixmapCache::insert(key, add) );
    }

    // Ensure pixmap can be retrieved correctly and is the same as the
    // added pixmap
    {
        QPixmap check;
        QVERIFY( QGlobalPixmapCache::find( key, check ) );
        QCOMPARE( check, pixmap );
    }

    if (remove) {
        QGlobalPixmapCache::remove(key);
    }
}

/*?
    Data for insert_remove() test function.
    Uses same data as insert_find().
*/
void tst_QGlobalPixmapCache::insert_remove_data()
{
    insert_find_data();
}

/*?
    Test function for inserting and removing pixmaps from cache.
    This function:
        * Checks that a pixmap either is or is not in cache, depending on
          what the test data says should be the case.
        * Inserts a pixmap into the cache many times using a specified key.
          This overwrites the pixmap every time after the initial addition,
          so at the end, only one pixmap should be in the cache for the
          given key.
        * Checks that the pixmap can be retrieved from the cache using the
          key, and that it matches the original pixmap.
        * Removes the pixmap from the cache exactly once.
        * Ensures that the pixmap has been completely removed from the cache.
*/
void tst_QGlobalPixmapCache::insert_remove()
{
    QFETCH( QString, key );
    QFETCH( QPixmap, pixmap );
    QFETCH( bool, overwrite );

    QEXPECT_FAIL("empty key", "Task 143439", Abort);
    QEXPECT_FAIL("simple1", "Task 143439", Abort);
    QEXPECT_FAIL("simple2", "Task 143439", Abort);
    QEXPECT_FAIL("empty pixmap", "Task 143439", Abort);

    // Verify key is or is not already used in cache, depending on what we
    // expect.
    {
        QPixmap check;
        QVERIFY( overwrite && QGlobalPixmapCache::find( key, check )
                 || !overwrite && !QGlobalPixmapCache::find( key, check ) );
    }

    // Add the pixmap many times...
    for ( int i = 0; i < 10; ++i ) {
        QPixmap add = pixmap.copy();
        QVERIFY( QGlobalPixmapCache::insert(key, add) );
    }

    // Ensure it is in place correctly
    {
        QPixmap check;
        QVERIFY( QGlobalPixmapCache::find( key, check ) );
        QCOMPARE( check, pixmap );
    }

    // Remove the pixmap once
    QGlobalPixmapCache::remove( key );

    QEXPECT_FAIL("overwrite", "Task 143439", Abort);

    // Ensure it is really gone
    {
        QPixmap check;
        QVERIFY( !QGlobalPixmapCache::find( key, check ) );
        QCOMPARE( check, QPixmap() );
    }
}

/*?
    Test function to ensure the pixmap cache functions correctly even after
    being filled.
    This test is currently disabled pending fix of task 143439.
    This function:
        * Adds many copies of a pixmap to the cache using different keys until
          the cache refuses to add any more.
        * Removes all pixmaps which were added to the cache.
        * Adds three more pixmaps to the cache and verifies they have been
          correctly added.
*/
void tst_QGlobalPixmapCache::fillCache()
{
   // Keep adding pixmaps till we fill the cache
    bool pixmapAdded = true;
    int pixmapsAdded = 0;
    QList<QPixmap> pms;
    QPixmap pixmap(Qtopia::qtopiaDir() + QString( "/pics/qtopia.png"));
    while ( pixmapAdded ) {
        QString key = "fill_cache_";
        key += QString::number( pixmapsAdded );
        QPixmap addMe( pixmap );
        pixmapAdded = QGlobalPixmapCache::insert( key, addMe );
        if ( pixmapAdded )
            pms.append( addMe );
        else
            break;

        if ( pixmapsAdded > 1000000 )
            break;

        ++pixmapsAdded;
    }

    // Clear the cache
    pms.clear();
    for ( int i=0; i<pixmapsAdded; ++i ) {
        QString key = "fill_cache_";
        key += QString::number( i );
        QGlobalPixmapCache::remove( key );
    }

    // Now check that we can add another pixmap
    QPixmap redPm1( pixmap );
    QVERIFY( QGlobalPixmapCache::insert( "myred", redPm1 ) );
    QPixmap redPm2;
    QVERIFY( QGlobalPixmapCache::find( "myred", redPm2 ) );
    QCOMPARE( redPm2, redPm1 );

    QPixmap yellowPm1( pixmap );
    QVERIFY( QGlobalPixmapCache::insert( "myyellow", yellowPm1 ) );
    QPixmap yellowPm2;
    QVERIFY( QGlobalPixmapCache::find( "myyellow", yellowPm2 ) );
    QCOMPARE( yellowPm2, yellowPm1 );

    QPixmap greenPm1( pixmap );
    QVERIFY( QGlobalPixmapCache::insert( "mygreen", greenPm1 ) );
    QPixmap greenPm2;
    QVERIFY( QGlobalPixmapCache::find( "mygreen", greenPm2 ) );
    QCOMPARE( greenPm2, greenPm1 );

    // Cleanup
    QGlobalPixmapCache::remove( "myred" );
    QGlobalPixmapCache::remove( "myyellow" );
    QGlobalPixmapCache::remove( "mygreen" );
}

/*?
    Test function to ensure the pixmap cache does not get filled when
    overwriting the same pixmap many times.
    This test is currently disabled pending fix of task 143439.
    This function:
        * Adds a pixmap to the cache using the same key an
          extremely large number of times.
        * Verifies that the pixmap was correctly added.
*/
void tst_QGlobalPixmapCache::notFillCache()
{
    // Keep adding pixmaps with the _same_ key, and hence cache
    // should not be filled.
    bool pixmapAdded;
    bool didNotFill = false;
    int pixmapsAdded = 0;
    QPixmap pixmap(Qtopia::qtopiaDir() + QString( "/pics/qtopia.png"));
    do {
        QString key = "should_not_fill_cache";
        pixmapAdded = QGlobalPixmapCache::insert( key, pixmap );
        //QGlobalPixmapCache::remove( key );

        // Break out of the loop when we've added a ridiculous number of pixmaps
        if ( pixmapsAdded > 100000 ) {
            didNotFill = true;
            break;
        }

        ++pixmapsAdded;
    } while ( pixmapAdded );

    QVERIFY( didNotFill );
}

