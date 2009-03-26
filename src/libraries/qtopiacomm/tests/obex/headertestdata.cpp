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

//QTEST_SKIP_TEST_DOC

#include "headertestdata.h"

#include <QObject>
#include <QUuid>
#include <QDebug>
#include <QByteArray>
#include <QMetaType>
#include <QTest>

Q_DECLARE_METATYPE(QUuid)


/*
static QByteArray toString(const char *header, const char *detail)
{
    return QByteArray(header).append(" / ").append(detail);
}

static QByteArray toString(const char *header, int detail)
{
    QString desc = QString("%1 / %2").arg(header).arg(detail);
    return desc.toAscii();
}
*/

static QByteArray toString(int detail)
{
    QString desc = QString("%1").arg(detail);
    return desc.toAscii();
}

QString HeaderTestData::unicodeString()
{
    const QChar unicode[] = {
        0x005A, 0x007F, 0x00A4, 0x0060,
        0x1009, 0x0020, 0x0020};
    int size = sizeof(unicode) / sizeof(QChar);
    return QString(unicode, size);
}

void HeaderTestData::setQuint32_fillData()
{
    QTest::addColumn<quint32>("num");

    quint32 nums[] = { 0, 1, 2, 10000 };
    for (quint32 i=0; i<4; i++) {
        QTest::newRow(toString(nums[i]).constData()) << nums[i];
    }
}

void HeaderTestData::rawUint32_fillData()
{
    QTest::addColumn<uint32_t>("num");

    uint32_t nums[] = { 0, 1, 2, 10000 };
    for (uint32_t i=0; i<4; i++) {
        QTest::newRow(toString(nums[i]).constData()) << nums[i];
    }
}

void HeaderTestData::setQuint8_fillData()
{
    QTest::addColumn<quint8>("num");

    quint8 nums[] = { 0, 1, 2, 255 };
    for (quint32 i=0; i<4; i++) {
        QTest::newRow(toString(nums[i]).constData()) << nums[i];
    }
}

void HeaderTestData::rawUint8_fillData()
{
    QTest::addColumn<uint8_t>("num");

    uint8_t nums[] = { 0, 1, 2, 255 };
    for (uint8_t i=0; i<4; i++) {
        QTest::newRow(toString(nums[i]).constData()) << nums[i];
    }
}

void HeaderTestData::setString_fillData(bool canBeUnicode)
{
    QTest::addColumn<QString>("string");

    QTest::newRow("null str") << QString();
    QTest::newRow("empty str") << QString("");
    QTest::newRow("ascii str") << "ascii string";

    if (canBeUnicode) {
        QTest::newRow("unicode str") << unicodeString();
    }
}

void HeaderTestData::rawTime_fillData()
{
    QTest::addColumn<QByteArray>("rawAscii");
    QTest::addColumn<QDateTime>("qDateTimeValue");
    QTest::addColumn<bool>("valid");

    // If the raw time is invalid, readOpenObexHeader() should interpret it as
    // an invalid QDateTime, and add an invalid QDateTime to represent it,
    // instead of adding nothing at all
    QTest::newRow("invalid time - empty string")
            << QByteArray("") << QDateTime() << false;
    QTest::newRow("invalid time - bad format")
            << QByteArray("12345") << QDateTime() << false;

    // how to format the time string according to specs 2.2.5
    QString formatLocalTime = "yyyyMMddThhmmss";
    QString formatUtcTime = formatLocalTime + "Z";

    QDateTime dateTime = QDateTime::currentDateTime();

    // The milliseconds value is not sent in the OBEX field, so set this value
    // to zero in the expected QDateTime, otherwise the test will fail if
    // the milliseconds don't match up.
    QTime t = dateTime.time();
    t.setHMS(t.hour(), t.minute(), t.second(), 0);
    dateTime.setTime(t);

    dateTime.setTimeSpec(Qt::LocalTime);
    QTest::newRow("local time")
            << dateTime.toString(formatLocalTime).toAscii() << dateTime << true;

    dateTime.setTimeSpec(Qt::UTC);
    QTest::newRow("UTC time")
            << dateTime.toString(formatUtcTime).toAscii() << dateTime << true;
}

/*
void HeaderTestData::setTime4Byte_fillData()
{
    QTest::addColumn<QDateTime>("time");
    QTest::addColumn<QDateTime>("expected");

    QDateTime invalid;
    QTest::newRow("invalid time") << invalid << QDateTime::fromTime_t(0);

    QDateTime dateTime = QDateTime::currentDateTime();
    QTime t = dateTime.time();
    t.setHMS(t.hour(), t.minute(), t.second(), 0);
    dateTime.setTime(t);

    QTest::newRow("some time") << dateTime << dateTime;
}

void HeaderTestData::rawTime4Byte_fillData()
{
    QTest::addColumn<uint32_t>("rawInt");
    QTest::addColumn<QDateTime>("qDateTimeValue");

    QDateTime dateTime = QDateTime::currentDateTime();
    uint32_t times[] = { 0, 54321, dateTime.toTime_t() };

    QDateTime t;
    for (uint32_t i=0; i<3; i++) {
        t.setTime_t(times[i]);
        QTest::newRow(toString(static_cast<int>(times[i]))) << times[i] << t;
    }
}
*/

void HeaderTestData::setBytes_fillData()
{
    QTest::addColumn<QByteArray>("bytes");

    QTest::newRow("null bytearray") << QByteArray();
    QTest::newRow("empty bytearray") << QByteArray("");

    QByteArray bytes("123asdf as w12wefwf31231230x110x99");
    QTest::newRow("some bytearray") << bytes;
}

void HeaderTestData::setUuid_fillData()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<QUuid>("expected");

    QTest::newRow("null uuid") << QUuid() << QUuid();

    // 67C8770B-44F1-410A-AB9A-F9B5446F13EE
    QUuid uuid(0x67c8770b, 0x44f1, 0x410a, 0xab, 0x9a, 0xf9, 0xb5,
               0x44, 0x6f, 0x13, 0xee);
    QTest::newRow("some uuid") << uuid << uuid;
}

