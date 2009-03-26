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
#include <qdebug.h>
#include "packageversion.h"

//TESTED_CLASS=Version
//TESTED_FILES=src/settings/packagemanager/version.cpp

class tst_Version: public QObject
{
    Q_OBJECT
private slots:

    void rangeVsMix();
    void rangeVsMix_data();

    void singleVsMix();
    void singleVsMix_data();

    void mixVsMix();
    void mixVsMix_data();

    void version();
    void version_data();

};

void tst_Version::rangeVsMix()
{
    QFETCH(QString, Versions );
    QFETCH(bool, IsCompatible );

    QString qtopiaVersion = "4.2.2-4.2.6";

    QCOMPARE( VersionUtil::checkVersionLists(qtopiaVersion, Versions ), IsCompatible );
}

void tst_Version::rangeVsMix_data()
{

    QTest::addColumn<QString>("Versions");
    QTest::addColumn<bool>("IsCompatible");

    QTest::newRow( "Range exact match" )                    << "4.2.2-4.2.6"    << true;
    QTest::newRow( "Range min exact match, max within" )    << "4.2.2-4.2.4"    << true;
    QTest::newRow( "Range min exact match, max outside" )   << "4.2.2-4.2.7"    << true;
    QTest::newRow( "Range min within, max exact match" )    << "4.2.3-4.2.6"    << true;
    QTest::newRow( "Range min outside, max exact match" )   << "4.2.1-4.2.6"    << true;
    QTest::newRow( "Range min within, max within " )        << "4.2.3-4.2.4"    << true;
    QTest::newRow( "Range min without, max without " )      << "4.2.1-4.2.7"    << true;
    QTest::newRow( "Single match min" )                     << "4.2.2"          << true;
    QTest::newRow( "Single match max" )                     << "4.2.6"          << true;
    QTest::newRow( "Single match" )                         << "4.2.5"          << true;
    QTest::newRow( "Matching single + NM ranges" )          << "4.2.4,3-4,4.8-4.9"      << true;
    QTest::newRow( "Matching single + NM singles" )         << "4.2.5, 5.2.1, 6.4.1"    << true;
    QTest::newRow( "NM ranges + matching single" )          << "5.2-6,3.4-3.5.5,4.2.6"  << true;
    QTest::newRow( "NM singles + matching single" )         << "4.3.9,4.3.6,4.2.3"      << true;
    QTest::newRow( "M single btwn NM ranges/singles " )     << "4.3-4.4.2,4.2.4,4.3.8" << true;

    QTest::newRow( "Range just below" )                     << "2.7.1-4.2.1"    << false;
    QTest::newRow( "Range just above" )                     << "4.2.7-5.2.0"    << false;
    QTest::newRow( "Range below" )                          << "2.8.1-3.4.8"    << false;
    QTest::newRow( "Range above" )                          << "5.4.1-6.2.3"    << false;
    QTest::newRow( "Invalid Range" )                        << "5-3"            << false;
    QTest::newRow( "NM single just above" )                 << "4.2.7"          << false;
    QTest::newRow( "NM single just belo" )                  << "4.2.1"          << false;
    QTest::newRow( "NM single (1)" )                        << "4"              << false;
    QTest::newRow( "NM single (2)" )                        << "4.2"            << false;
    QTest::newRow( "NM single (3)" )                        << "4.3.9"          << false;
    QTest::newRow( "Multiple NM single" )                   << "2.1,5.9,4.7"    << false;
    QTest::newRow( "Mix NM ranges and NM Single" )          << "4.4.4-4.4.5,5,2-3.2" << false;

}

void tst_Version::singleVsMix()
{
    QFETCH(QString, Versions );
    QFETCH(bool, IsCompatible );

    QString qtopiaVersion = "4.2.2";

    QCOMPARE( VersionUtil::checkVersionLists(qtopiaVersion, Versions ), IsCompatible );
}

void tst_Version::singleVsMix_data()
{

    QTest::addColumn<QString>("Versions");
    QTest::addColumn<bool>("IsCompatible");

    QTest::newRow( "Exact match" )                          << "4.2.2"          << true;
    QTest::newRow( "Range min exact" )                      << "4.2.2-4.2.3"    << true;
    QTest::newRow( "Range max exact" )                      << "4.1.4-4.2.2"    << true;
    QTest::newRow( "M Range" )                       << "4.1.2-4.4.2"    << true;
    QTest::newRow( "M Range: (1,3)" )                << "4-5.2.2"        << true;
    QTest::newRow( "M Range: (3,1)" )                << "4.1.0-6"        << true;
    QTest::newRow( "M Range: (2,3)" )                << "4.2-4.2.3"      << true;
    QTest::newRow( "M Range: (3,2)" )                << "4.2.1-4.4"      << true;
    QTest::newRow( "M Range: (2,2)" )                << "2.7-7.2"        << true;
    QTest::newRow( "M Range: (1,1)" )                << "1-5"            << true;
    QTest::newRow( "M Range: (2,1)" )                << "4.2-5"          << true;
    QTest::newRow( "M Range: (1,2)" )                << "4-5.1"          << true;
    QTest::newRow( "NM Range + exact match" )               << "2.4.7-4.1.7,4.2.2"  << true;
    QTest::newRow( "Exact match + NM range" )               << "4.2.2, 2.4.8-3.7"   << true;
    QTest::newRow( "Exact match between NM ranges" )        << "2-4,4.2.2,4.3-4.6"  << true;
    QTest::newRow( "Matching range + NM vers" )             << "4-5,2.4.7,5.8.2"    << true;
    QTest::newRow( "NM vers + matching range" )             << "4.1.8,5.2,4.1-4.3"  << true;
    QTest::newRow( "Matching range between NM vers" )       << "4.2.1,4.2-4.3,4.4"  << true;


    QTest::newRow( "One version above" )                    << "4.2.3"      << false;
    QTest::newRow( "One version below" )                    << "4.2.1"      << false;
    QTest::newRow( "Multiple non-matching range" )          << "1.1-2.4,3-4,4.2.3-4.4" << false;
    QTest::newRow( "Multiple non-matching versions" )       << "2.7,3.4.8,4.2,5" << false;
    QTest::newRow( "NM range, min one ver above" )          << "4.2.3-5"    << false;
    QTest::newRow( "NM range, max one ver under" )          << "2-4.2.1"    << false;
    QTest::newRow( "NM version" )                           << "2.4.7"      << false;
    QTest::newRow( "NM range: (1,3)" )                      << "1-4.2.1"    << false;
    QTest::newRow( "NM range: (3,1)" )                      << "1.1.1-4"    << false;
    QTest::newRow( "NM range: (2,3)" )                      << "2.2-4.2.1"  << false;
    QTest::newRow( "NM range: (3,2)" )                      << "2.2.4-4.1"  << false;
    QTest::newRow( "NM range: (2,2)" )                      << "4.3-4.4"    << false;
    QTest::newRow( "NM range: (1,1)" )                      << "3-4"        << false;
    QTest::newRow( "NM range: (2,1)" )                      << "3.2-4"      << false;
    QTest::newRow( "NM range: (1,2)" )                      << "5-6.2"      << false;
    QTest::newRow( "Mix NM range and NM vers" )             << "2-4,4.3.7,6-7"  << false;
}


void tst_Version::mixVsMix()
{
    QFETCH(QString, Versions );
    QFETCH(bool, IsCompatible );

    QString qtopiaVersion = "4.2-4.3,4.4";

    QCOMPARE( VersionUtil::checkVersionLists(qtopiaVersion, Versions ), IsCompatible );
}

void tst_Version::mixVsMix_data()
{

    QTest::addColumn<QString>("Versions");
    QTest::addColumn<bool>("IsCompatible");

    QTest::newRow( "S M QR" )                           << "4.2.9"          << true;
    QTest::newRow( "S M QS" )                           << "4.4.0"          << true;
    QTest::newRow( "R M QR" )                           << "4.2.9-4.3.1"    << true;
    QTest::newRow( "R M QS" )                           << "4.3.5-4.4.5"    << true;
    QTest::newRow( "R M QS and QR" )                    << "4.1-4.5"        << true;
    QTest::newRow( "R M QR and S M QS" )                << "4.2-4.3,4.4"    << true;
    QTest::newRow( "R NM Q + S M QS" )                  << "1-2,4.4"        << true;
    QTest::newRow( "S M QS + R NM Q" )                  << "4.4, 1.2-3"     << true;
    QTest::newRow( "S NM Q + R M QR" )                  << "4.4.1,4.2-4.2.1"  << true;
    QTest::newRow( "R M QR + S NM Q" )                  << "4.2.10-4.2.11,5"    << true;
    QTest::newRow( "R M QR between NM S & R" )          << "4.1,4-4.2.8,1.1-2"  << true;

    QTest::newRow( "NM R between QR and QS" )           << "4.3.5-4.3.9"    << false;
    QTest::newRow( "NM S between QR and QS" )           << "4.3.1"          << false;
    QTest::newRow( "Various NM S and R" )                << "4,4.1-4.1.5,8" << false;
}

void tst_Version::version()
{

    QFETCH(QString, Versions );
    QFETCH(bool, IsCompatible );
    Version version( Versions );
    Version baseVersion( "4.2.3" );

    QCOMPARE( baseVersion < version , IsCompatible );
    QCOMPARE( baseVersion > version, IsCompatible );
    QCOMPARE( baseVersion == version, IsCompatible );
    QCOMPARE( baseVersion <= version, IsCompatible );
    QCOMPARE( baseVersion >= version, IsCompatible );
}
void tst_Version::version_data()
{
    QTest::addColumn<QString>("Versions");
    QTest::addColumn<bool>("IsCompatible");
    QTest::newRow( "Garbage" )              << "abcd"           << false;
    QTest::newRow( "Prefixed" )             << "Version4.2.3"     << false;
    QTest::newRow( "Garbage mixed in pos intern" ) << "4.ab2.hh3"  << false;
    QTest::newRow( "Garbage mixed in pos extern" ) << "gg4.ab2.hh3ij"  << false;
    QTest::newRow( "Garbage suffix" )       << "4.2.3sdfdfsf" << false;
}


QTEST_MAIN(tst_Version)
#include "tst_version.moc"
