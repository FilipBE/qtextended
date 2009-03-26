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
#include <shared/qtopiaunittest.h>
#include <QtopiaApplication>
#include <qtopiacollective/private/sippresencereader_p.h>
#include <QCollectivePresenceInfo>

//TESTED_CLASS=SipPresenceReader
//TESTED_FILES=src/libraries/qtopiacollective/sippresencereader_p.h,src/libraries/qtopiacollective/sippresencereader.cpp

/*
    This class is a unit test for the SipPresenceReader class.
*/
class tst_SipPresenceReader : public QObject
{
    Q_OBJECT

private slots:
    void parsePidf();
    void parsePidf_data();

    void parseXpidf();
    void parseXpidf_data();
};

void tst_SipPresenceReader::parsePidf_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("uri");
    QTest::addColumn<QString>("status");

    QTest::newRow("pidf01") << QtopiaUnitTest::baseDataPath() + "/pidf01.txt"
            << "someone@example.com" << "Online";
    QTest::newRow("pidf02") << QtopiaUnitTest::baseDataPath() + "/pidf02.txt"
            << "someone@example.com" << "Online";
    QTest::newRow("pidf03") << QtopiaUnitTest::baseDataPath() + "/pidf03.txt"
            << "someone@example.com" << "Online";
    QTest::newRow("pidf04") << QtopiaUnitTest::baseDataPath() + "/pidf04.txt"
            << "someone@example.com" << "Online";
}

/*?
    Ensure pidf parsing is done correctly.
*/
void tst_SipPresenceReader::parsePidf()
{
    QFETCH(QString, filename);
    QFETCH(QString, uri);
    QFETCH(QString, status);
    QFile file(filename);

    QVERIFY(file.open(QIODevice::ReadOnly));

    QByteArray data = file.readAll();
    file.close();

    SipPresenceReader reader("application/pidf+xml", data);
    QCollectivePresenceInfo info(reader.info());

    QVERIFY(info.uri() == uri);
    QVERIFY(info.presence() == status);

    qDebug() << info;
}

void tst_SipPresenceReader::parseXpidf_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("uri");
    QTest::addColumn<QString>("status");

    QTest::newRow("xpidf01") << QtopiaUnitTest::baseDataPath() + "/xpidf01.txt"
            << "3035551200@255.255.255.79" << "On the Phone";
    QTest::newRow("xpidf02") << QtopiaUnitTest::baseDataPath() + "/xpidf02.txt"
            << "3035551200@255.255.255.79" << "Online";
    QTest::newRow("xpidf03") << QtopiaUnitTest::baseDataPath() + "/xpidf03.txt"
            << "user@example.com" << "Online";
    QTest::newRow("xpidf04") << QtopiaUnitTest::baseDataPath() + "/xpidf04.txt"
            << "user@example.com" << "Online";
    QTest::newRow("xpidf05") << QtopiaUnitTest::baseDataPath() + "/xpidf05.txt"
            << "user@example.com" << "Online";
    QTest::newRow("xpidf06") << QtopiaUnitTest::baseDataPath() + "/xpidf06.txt"
            << "user@example.com" << "Online";
}

void tst_SipPresenceReader::parseXpidf()
{
    QFETCH(QString, filename);
    QFETCH(QString, uri);
    QFETCH(QString, status);
    QFile file(filename);

    QVERIFY(file.open(QIODevice::ReadOnly));

    QByteArray data = file.readAll();
    file.close();

    SipPresenceReader reader("application/xpidf+xml", data);
    QCollectivePresenceInfo info(reader.info());

    QVERIFY(info.uri() == uri);
    QVERIFY(info.presence() == status);

    qDebug() << info;
}

QTEST_APP_MAIN( tst_SipPresenceReader, QtopiaApplication )
#include "tst_sippresencereader.moc"
