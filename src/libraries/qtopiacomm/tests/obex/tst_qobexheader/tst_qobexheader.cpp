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

#include "../obextestsglobal.h"
#include "../headertestdata.h"
#include <QObexHeader>
#include "qobexheader_p.h"
#include "qobexauthentication_p.h"

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QDebug>

//TESTED_CLASS=QObexHeader
//TESTED_FILES=src/libraries/qtopiacomm/obex/qobexheader.cpp


static void dateTimeToString(const QDateTime &dateTime, QString &timeString)
{
    timeString = dateTime.toString(dateTime.timeSpec() == Qt::UTC?
            "yyyyMMddThhmmssZ" : "yyyyMMddThhmmss");
}


static QList<int> getHeaderIds()
{
    QList<int> l;
    l << QObexHeader::Count
            << QObexHeader::Name
            << QObexHeader::Type
            << QObexHeader::Length
            << QObexHeader::Time
            << QObexHeader::Description
            << QObexHeader::Target
            << QObexHeader::Http
            << QObexHeader::Who
            << QObexHeader::ConnectionId
            << QObexHeader::AppParameters
            << QObexHeader::AuthChallenge
            << QObexHeader::AuthResponse
            << QObexHeader::CreatorId
            << QObexHeader::WanUuid
            << QObexHeader::ObjectClass
            << QObexHeader::SessionParameters
            << QObexHeader::SessionSequenceNumber;
    return l;
}
static QList<int> headerIds = getHeaderIds();


class tst_QObexHeader : public QObject
{
    Q_OBJECT

protected:
/*
    Tests that an added header value is "visible" to common header functions
    (e.g. contains() returns true, the value is copied for operator=, etc.)

    This test assumes there is only one header present (i.e. the added header).
*/
template <typename T>
    void testAddedValue(const QObexHeader &h, int id, const T &t)
    {
        Q_ASSERT(h.contains(id));

        // value() should return the same item as the one you added
        QVariant v = h.value(id);
        QCOMPARE(v.template value<T>(), t);

        // operator=
        QObexHeader hCopy = h;
        QCOMPARE(hCopy.value(id).template value<T>(), t);

        // operator==
        QVERIFY(h == hCopy);
        QObexHeader dummy;
        QVERIFY(h != dummy);

        // keys()
        QList<int> keys = h.headerIds();
        QVERIFY(keys.contains(id));
        QCOMPARE(keys.size(), 1);

        // size()
        QCOMPARE(h.size(), 1);
    }

    /*
        Removes the header value from the header and tests contains(), etc. to
        check it's been removed properly.
    */
    void testRemoveValue(QObexHeader &h, int id)
    {
        QVERIFY(h.remove(id));
        QVERIFY(!h.contains(id));
        QVERIFY(!h.value(id).isValid());
    }

    template <typename T>
    void genericHeaderTest(QObexHeader &h, int id, const T &t)
    {
        testAddedValue(h, id, t);
        testRemoveValue(h, id);
    }


private slots:

    /*
    Test keys() function
    */
    void keys()
    {
        QObexHeader h;
        QCOMPARE(h.headerIds().size(), 0);

        // add a header
        h.setName("some name");
        QCOMPARE(h.headerIds().size(), 1);
        QVERIFY(h.headerIds().contains(QObexHeader::Name));

        // add some more headers
        h.setLength(10);
        h.setTarget(QByteArray("some target"));

        QList<int> addedHeaders;
        addedHeaders << QObexHeader::Name << QObexHeader::Length << QObexHeader::Target;

        // check keys() have all added headers
        // use QSet to compare as header ordering is arbitrary
        QCOMPARE(QSet<int>::fromList(h.headerIds()),
                 QSet<int>::fromList(addedHeaders));

        // remove everything and check the keys() are empty
        for (int i=0; i<addedHeaders.size(); i++) {
            h.remove(addedHeaders[i]);
            QVERIFY(!h.headerIds().contains(addedHeaders[i]));
        }
        QCOMPARE(h.headerIds().size(), 0);
    }

    /*
    Test size() function
    */
    void size()
    {
        QObexHeader h;
        QCOMPARE(h.size(), 0);

        // add a header
        h.setType("some type");
        QCOMPARE(h.size(), 1);

        // add some more headers
        h.setHttp(QByteArray("some bit of http"));
        h.setSessionSequenceNumber(255);
        h.setTime(QDateTime::currentDateTime());
        QCOMPARE(h.size(), 4);

        // multiple headers are not supported - this shouldn't change anything
        h.setSessionSequenceNumber(100);
        QCOMPARE(h.size(), 4);

        // remove and check again
        h.remove(QObexHeader::Type);
        QCOMPARE(h.size(), 3);
        h.remove(QObexHeader::Http);
        h.remove(QObexHeader::SessionSequenceNumber);
        QCOMPARE(h.size(), 1);
        h.remove(QObexHeader::Time);
        QCOMPARE(h.size(), 0);
    }

    /*
    Test remove() function
    */
    void remove()
    {
        QObexHeader h;
        QVERIFY(!h.remove(QObexHeader::Count));

        h.setCount(150);
        h.setCount(200);    // multiple headers not supported, should have no effect
        QCOMPARE(h.size(), 1);
        QVERIFY(h.remove(QObexHeader::Count));
        QVERIFY(!h.remove(QObexHeader::Count));
        QVERIFY(!h.contains(QObexHeader::Count));
    }

    void clear()
    {
        QObexHeader h;
        QCOMPARE(h.size(), 0);
        h.clear();
        QCOMPARE(h.size(), 0);

        h.setType("aaa");
        QCOMPARE(h.size(), 1);
        h.clear();
        QCOMPARE(h.size(), 0);

        h.setLength(5);
        h.setTarget(QByteArray("aaa"));
        QCOMPARE(h.size(), 2);
        h.clear();
        QCOMPARE(h.size(), 0);
    }

    // There's no specific contains() tests - it's pretty much used everywhere
    // in these tests.

    /*
    For each defined OBEX header, test contains(), operator=, etc.
    */
    void genericTestAllHeaders()
    {
        // The inserted values do not matter as this function just tests
        // that all the common functions (contains(), keys(), etc.)
        // recognise that a value has been inserted for a particular header.

        quint32 testNum = 10000;
        QString testString = "test string";
        QDateTime testTime = QDateTime::currentDateTime();
        QByteArray testBytes = QByteArray("ab112lkjsdf0x11");
        QUuid testUuid = QUuid("67C8770B-44F1-410A-AB9A-F9B5446F13EE");
        quint8 test1ByteNum = 255;

        for (int i=0; i<headerIds.size(); i++) {
            QObexHeader h;

            QVERIFY(!h.contains(headerIds[i]));

            switch(headerIds[i]) {
                case QObexHeader::Count:
                    h.setCount(testNum);
                    genericHeaderTest(h, headerIds[i], testNum);
                    break;
                case QObexHeader::Name:
                    h.setName(testString);
                    genericHeaderTest(h, headerIds[i], testString);
                    break;
                case QObexHeader::Type:
                    h.setType(testString);
                    genericHeaderTest(h, headerIds[i], testString);
                    break;
                case QObexHeader::Length:
                    h.setLength(testNum);
                    genericHeaderTest(h, headerIds[i], testNum);
                    break;
                case QObexHeader::Time:
                {
                    h.setTime(testTime);
                    // the generic test will call setValue() which returns the
                    // time as a string, so convert it here before testing
                    QString timeString;
                    dateTimeToString(testTime, timeString);
                    genericHeaderTest(h, headerIds[i], timeString);
                    break;
                }
                /*
                case QObexHeader::Time4Byte:
                    h.setTime4Byte(testTime);
                    // the generic test will call setValue() which returns the
                    // time as an int, so convert it here before testing
                    genericHeaderTest(h, headerIds[i], testTime.toTime_t());
                    break;
                */
                case QObexHeader::Description:
                    h.setDescription(testString);
                    genericHeaderTest(h, headerIds[i], testString);
                    break;
                case QObexHeader::Target:
                    h.setTarget(testBytes);
                    genericHeaderTest(h, headerIds[i], testBytes);
                    break;
                case QObexHeader::Http:
                    h.setHttp(testBytes);
                    genericHeaderTest(h, headerIds[i], testBytes);
                    break;
                case QObexHeader::Who:
                    h.setWho(testBytes);
                    genericHeaderTest(h, headerIds[i], testBytes);
                    break;
                case QObexHeader::ConnectionId:
                    h.setConnectionId(testNum);
                    genericHeaderTest(h, headerIds[i], testNum);
                    break;
                case QObexHeader::AppParameters:
                    h.setAppParameters(testBytes);
                    genericHeaderTest(h, headerIds[i], testBytes);
                    break;
                case QObexHeader::AuthChallenge:
                    h.setValue(QObexHeader::AuthChallenge, testBytes);
                    genericHeaderTest(h, headerIds[i], testBytes);
                    break;
                case QObexHeader::AuthResponse:
                    h.setValue(QObexHeader::AuthResponse, testBytes);
                    genericHeaderTest(h, headerIds[i], testBytes);
                    break;
                case QObexHeader::CreatorId:
                    h.setCreatorId(testNum);
                    genericHeaderTest(h, headerIds[i], testNum);
                    break;
                case QObexHeader::WanUuid:
                {
                    h.setWanUuid(testUuid);

                    // the generic test will call setValue() which returns the
                    // UUID as a byte array, so convert it here before testing
                    QByteArray bytes;
                    QDataStream dataStream(&bytes, QIODevice::WriteOnly);
                    dataStream << testUuid;
                    genericHeaderTest(h, headerIds[i], bytes);
                    break;
                }
                case QObexHeader::ObjectClass:
                    h.setObjectClass(testBytes);
                    genericHeaderTest(h, headerIds[i], testBytes);
                    break;
                case QObexHeader::SessionParameters:
                    h.setSessionParameters(testBytes);
                    genericHeaderTest(h, headerIds[i], testBytes);
                    break;
                case QObexHeader::SessionSequenceNumber:
                    h.setSessionSequenceNumber(test1ByteNum);
                    genericHeaderTest(h, headerIds[i], test1ByteNum);
                    break;
                default:
                    qLog(Autotest) << "tst_qobexheader: Untested HeaderId"
                            << QObexHeaderPrivate::headerIdToString(headerIds[i])
                            << "(" << headerIds[i] << ")";
                    QFAIL("Fail due to untested HeaderId");
            }
        }
    }


    /*
        The following functions test each of the individual setter and getter
        methods.
        e.g. setCount(), count()
    */

    void setCount_data() { HeaderTestData::setQuint32_fillData(); }
    void setCount()
    {
        QFETCH(quint32, num);

        QObexHeader h;
        h.setCount(num);
        QCOMPARE(h.count(), num);
        QVERIFY(h.contains(QObexHeader::Count));
    }

    void count()
    {
        QObexHeader h;
        QCOMPARE(h.count(), quint32(0));
        QVERIFY(!h.contains(QObexHeader::Count));
    }

    void setName_data() { HeaderTestData::setString_fillData(true); }
    void setName()
    {
        QFETCH(QString, string);

        QObexHeader h;
        h.setName(string);
        QCOMPARE(h.name(), string);
        QVERIFY(h.contains(QObexHeader::Name));
    }

    void name()
    {
        QObexHeader h;
        QVERIFY(h.name().isEmpty());
        QVERIFY(!h.contains(QObexHeader::Name));
    }

    void setType_data() { HeaderTestData::setString_fillData(false); }
    void setType()
    {
        QFETCH(QString, string);

        QObexHeader h;
        h.setType(string);
        QCOMPARE(h.type(), string);
        QVERIFY(h.contains(QObexHeader::Type));
    }

    void type()
    {
        QObexHeader h;
        QVERIFY(h.type().isEmpty());
        QVERIFY(!h.contains(QObexHeader::Type));
    }

    void setLength_data() { HeaderTestData::setQuint32_fillData(); }
    void setLength()
    {
        QFETCH(quint32, num);

        QObexHeader h;
        h.setLength(num);
        QCOMPARE(h.length(), num);
        QVERIFY(h.contains(QObexHeader::Length));
    }

    void length()
    {
        QObexHeader h;
        QCOMPARE(h.length(), quint32(0));
        QVERIFY(!h.contains(QObexHeader::Length));
    }

    void setTime_data()
    {
        QTest::addColumn<QDateTime>("time");
        QTest::addColumn<bool>("shouldAdd");

        // If an invalid QDateTime is passed to setTime(), don't add anything
        QDateTime invalid;
        QTest::newRow("invalid time") << invalid << false;

        // The milliseconds value is not set in the OBEX time string, so set this value
        // to zero in the expected QDateTime, otherwise the test will fail if
        // the milliseconds don't match up.
        QDateTime dateTime = QDateTime::currentDateTime();
        QTime t = dateTime.time();
        t.setHMS(t.hour(), t.minute(), t.second(), 0);
        dateTime.setTime(t);

        QTest::newRow("some time") << dateTime << true;

        dateTime.setTimeSpec(Qt::LocalTime);
        QTest::newRow("local time") << dateTime << true;

        dateTime.setTimeSpec(Qt::UTC);
        QTest::newRow("UTC time") << dateTime << true;
    }

    void setTime()
    {
        QFETCH(QDateTime, time);
        QFETCH(bool, shouldAdd);

        QObexHeader h;
        h.setTime(time);
        QCOMPARE(h.time(), time);
        QCOMPARE(h.contains(QObexHeader::Time), shouldAdd);
    }

    void setTime_withSetValue_data() { HeaderTestData::rawTime_fillData(); }
    void setTime_withSetValue()
    {
        QFETCH(QByteArray, rawAscii);
        QFETCH(QDateTime, qDateTimeValue);

        // should be able to set the Time header by passing a byte array
        // to setValue().
        QObexHeader h;
        h.setValue(QObexHeader::Time, rawAscii);
        QVERIFY(h.contains(QObexHeader::Time));

        QCOMPARE(h.time(), qDateTimeValue); // retrieve as QDateTime
        QCOMPARE(h.value(QObexHeader::Time).toByteArray(), rawAscii); // retrieve as ascii
    }

    void time()
    {
        QObexHeader h;
        QVERIFY(!h.time().isValid());
        QVERIFY(!h.contains(QObexHeader::Time));
    }

    /*
    void setTime4Byte_data() { HeaderTestData::setTime4Byte_fillData(); }
    void setTime4Byte()
    {
        QFETCH(QDateTime, time);
        QFETCH(QDateTime, expected);

        QObexHeader h;
        h.setTime4Byte(time);
        QCOMPARE(h.time4Byte(), expected);
    }

    void time4Byte()
    {
        QObexHeader h;
        QCOMPARE(h.time4Byte(), QDateTime());
    }

    void setTime4Byte_withSetValue_data() { HeaderTestData::rawTime4Byte_fillData(); }
    void setTime4Byte_withSetValue()
    {
        QFETCH(uint32_t, rawInt);
        QFETCH(QDateTime, qDateTimeValue);

        // should be able to set the Time4Byte header by passing an int
        // to setValue().
        QObexHeader h;
        h.setValue(QObexHeader::Time4Byte, rawInt);

        QCOMPARE(h.time4Byte(), qDateTimeValue); // retrieve as QDateTime
        QCOMPARE(h.value(QObexHeader::Time4Byte).toUInt(), rawInt); // retrieve as int
    }
    */

    void setDescription_data() { HeaderTestData::setString_fillData(true); }
    void setDescription()
    {
        QFETCH(QString, string);

        QObexHeader h;
        h.setDescription(string);
        QCOMPARE(h.description(), string);
        QVERIFY(h.contains(QObexHeader::Description));
    }

    void description()
    {
        QObexHeader h;
        QVERIFY(h.description().isEmpty());
        QVERIFY(!h.contains(QObexHeader::Description));
    }

    void setTarget_data() { HeaderTestData::setBytes_fillData(); }
    void setTarget()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setTarget(bytes);
        QCOMPARE(h.target(), bytes);
        QVERIFY(h.contains(QObexHeader::Target));
    }

    void target()
    {
        QObexHeader h;
        QVERIFY(h.target().isEmpty());
        QVERIFY(!h.contains(QObexHeader::Target));
    }

    void setHttp_data() { HeaderTestData::setBytes_fillData(); }
    void setHttp()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setHttp(bytes);
        QCOMPARE(h.http(), bytes);
        QVERIFY(h.contains(QObexHeader::Http));
    }

    void http()
    {
        QObexHeader h;
        QVERIFY(h.http().isEmpty());
        QVERIFY(!h.contains(QObexHeader::Http));
    }

    void setWho_data() { HeaderTestData::setBytes_fillData(); }
    void setWho()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setWho(bytes);
        QCOMPARE(h.who(), bytes);
        QVERIFY(h.contains(QObexHeader::Who));
    }

    void who()
    {
        QObexHeader h;
        QVERIFY(h.who().isEmpty());
        QVERIFY(!h.contains(QObexHeader::Who));
    }

    void setConnectionId_data() { HeaderTestData::setQuint32_fillData(); }
    void setConnectionId()
    {
        QFETCH(quint32, num);

        QObexHeader h;
        h.setConnectionId(num);
        QCOMPARE(h.connectionId(), num);
        QVERIFY(h.contains(QObexHeader::ConnectionId));
    }

    void connectionId()
    {
        QObexHeader h;
        QCOMPARE(h.connectionId(), quint32(0));
        QVERIFY(!h.contains(QObexHeader::ConnectionId));
    }

    void setAppParameters_data() { HeaderTestData::setBytes_fillData(); }
    void setAppParameters()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setAppParameters(bytes);
        QCOMPARE(h.appParameters(), bytes);
        QVERIFY(h.contains(QObexHeader::AppParameters));
    }

    void appParameters()
    {
        QObexHeader h;
        QVERIFY(h.appParameters().isEmpty());
        QVERIFY(!h.contains(QObexHeader::AppParameters));
    }

    void setCreatorId_data() { HeaderTestData::setQuint32_fillData(); }
    void setCreatorId()
    {
        QFETCH(quint32, num);

        QObexHeader h;
        h.setCreatorId(num);
        QCOMPARE(h.creatorId(), num);
        QVERIFY(h.contains(QObexHeader::CreatorId));
    }

    void creatorId()
    {
        QObexHeader h;
        QCOMPARE(h.creatorId(), quint32(0));
        QVERIFY(!h.contains(QObexHeader::CreatorId));
    }

    void setWanUuid_data() { HeaderTestData::setUuid_fillData(); }
    void setWanUuid()
    {
        QFETCH(QUuid, uuid);
        QFETCH(QUuid, expected);

        QObexHeader h;
        h.setWanUuid(uuid);
        QCOMPARE(h.wanUuid(), expected);
        QVERIFY(h.contains(QObexHeader::WanUuid));
    }

    void wanUuid()
    {
        QObexHeader h;
        QVERIFY(h.wanUuid().isNull());
        QVERIFY(!h.contains(QObexHeader::WanUuid));
    }

    //TODO set uuid using setValue() & retrieve it from wanUuid(), vice-versa

    void setObjectClass_data() { HeaderTestData::setBytes_fillData(); }
    void setObjectClass()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setObjectClass(bytes);
        QCOMPARE(h.objectClass(), bytes);
        QVERIFY(h.contains(QObexHeader::ObjectClass));
    }

    void objectClass()
    {
        QObexHeader h;
        QVERIFY(h.objectClass().isEmpty());
        QVERIFY(!h.contains(QObexHeader::ObjectClass));
    }

    void setSessionParameters_data() { HeaderTestData::setBytes_fillData(); }
    void setSessionParameters()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setSessionParameters(bytes);
        QCOMPARE(h.sessionParameters(), bytes);
        QVERIFY(h.contains(QObexHeader::SessionParameters));
    }

    void sessionParameters()
    {
        QObexHeader h;
        QVERIFY(h.sessionParameters().isEmpty());
        QVERIFY(!h.contains(QObexHeader::SessionParameters));
    }

    void setSessionSequenceNumber_data() { HeaderTestData::setQuint8_fillData(); }
    void setSessionSequenceNumber()
    {
        QFETCH(quint8, num);

        QObexHeader h;
        h.setSessionSequenceNumber(num);
        QCOMPARE(h.sessionSequenceNumber(), num);
        QVERIFY(h.contains(QObexHeader::SessionSequenceNumber));
    }

    void sessionSequenceNumber()
    {
        QObexHeader h;
        QCOMPARE(h.sessionSequenceNumber(), quint8(0));
        QVERIFY(!h.contains(QObexHeader::SessionSequenceNumber));
    }

    void setAuthenticationChallenge()
    {
        QObexHeader h;
        QVERIFY(!h.contains(QObexHeader::AuthChallenge));
        QCOMPARE(h.value(QObexHeader::AuthChallenge).toByteArray(), QByteArray());

        // Just check that something has been added. The tst_authentication
        // code will test that all the authentication data is correct.
        h.setAuthenticationChallenge();
        QVERIFY(h.contains(QObexHeader::AuthChallenge));
        QCOMPARE(h.value(QObexHeader::AuthChallenge).toByteArray().size(),
                 2 + QObexAuth::NonceSize);  // size for nonce tag + length + nonce value
    }


    //---------------------------------------------------------

protected:
    /*
        Generic tests for setValue() and value().
    */

    void testSetValue(int obexDataType, const QVariant &variant, bool isValidVariant)
    {
        // create a custom header for this data type
        int headerId = (0x30 | obexDataType);

        // setValue() returns whether value was of correct data type
        QObexHeader h;
        QCOMPARE(h.setValue(headerId, variant), isValidVariant);

        // the variant should be stored if it is valid for this data type
        QVariant storedVariant = h.value(headerId);
        QCOMPARE((storedVariant == variant), isValidVariant);
        QCOMPARE(storedVariant.isValid(), isValidVariant);
        QCOMPARE(h.contains(headerId), isValidVariant);
    }

    void testValue(int obexDataType)
    {
        // create a custom header for this data type
        int headerId = (0x30 | obexDataType);

        // calling value() when nothing has been added should return invalid variant
        QObexHeader h;
        QVERIFY(!h.value(headerId).isValid());
    }

private slots:
    /*
        The following tests test setValue() and value() with custom OBEX headers.
    */

    void setValue_unicode_ok_data() { HeaderTestData::setString_fillData(true); }
    void setValue_unicode_ok()
    {
        QFETCH(QString, string);
        testSetValue(QObexHeaderPrivate::HeaderUnicodeEncoding, string, true);
    }

    void setValue_unicode_fail()
    {
        // can't convert QPoint or QRect to QString, so these should fail
        testSetValue(QObexHeaderPrivate::HeaderUnicodeEncoding, QPoint(0,0), false);
        testSetValue(QObexHeaderPrivate::HeaderUnicodeEncoding, QRect(0,0,10,10), false);
    }

    void value_unicode() {
        testValue(QObexHeaderPrivate::HeaderUnicodeEncoding);
    }

    void setValue_1Byte_ok_data() { HeaderTestData::setQuint8_fillData(); }
    void setValue_1Byte_ok()
    {
        QFETCH(quint8, num);
        testSetValue(QObexHeaderPrivate::HeaderByteEncoding, num, true);
    }

    void setValue_1Byte_fail()
    {
        // can't convert QDateTime or QPoint to quint8, so these should fail
        testSetValue(QObexHeaderPrivate::HeaderByteEncoding, QDateTime::currentDateTime(), false);
        testSetValue(QObexHeaderPrivate::HeaderByteEncoding, QPoint(0,0), false);
    }

    void value_1Byte() {
        testValue(QObexHeaderPrivate::HeaderByteEncoding);
    }

    void setValue_4Byte_ok_data() { HeaderTestData::setQuint32_fillData(); }
    void setValue_4Byte_ok()
    {
        QFETCH(quint32, num);
        testSetValue(QObexHeaderPrivate::HeaderIntEncoding, num, true);
    }

    void setValue_4Byte_fail()
    {
        // can't convert QDateTime or QPoint to quint32, so these should fail
        testSetValue(QObexHeaderPrivate::HeaderIntEncoding, QDateTime::currentDateTime(), false);
        testSetValue(QObexHeaderPrivate::HeaderIntEncoding, QPoint(0,0), false);
    }

    void value_4Byte() {
        testValue(QObexHeaderPrivate::HeaderIntEncoding);
    }

    void setValue_bytes_ok_data() { HeaderTestData::setBytes_fillData(); }
    void setValue_bytes_ok()
    {
        QFETCH(QByteArray, bytes);
        testSetValue(QObexHeaderPrivate::HeaderByteSequenceEncoding, bytes, true);
    }

    void setValue_bytes_fail()
    {
        testSetValue(QObexHeaderPrivate::HeaderByteSequenceEncoding, QDateTime::currentDateTime(), false);
        testSetValue(QObexHeaderPrivate::HeaderByteSequenceEncoding, QVariant(false), false);
    }

    void value_bytes() {
        testValue(QObexHeaderPrivate::HeaderByteSequenceEncoding);
    }

};

QTEST_MAIN(tst_QObexHeader)
#include "tst_qobexheader.moc"
