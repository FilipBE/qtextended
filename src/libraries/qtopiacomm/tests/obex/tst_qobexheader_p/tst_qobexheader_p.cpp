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
#include "openobexheaderprocessor.h"
#include <QObexHeader>
#include "qobexheader_p.h"

#include <qtopialog.h>

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <shared/util.h>
#include <QDebug>

#include <netinet/in.h>
#include <openobex/obex.h>

//TESTED_CLASS=QObexHeader
//TESTED_FILES=src/libraries/qtopiacomm/obex/qobexheader_p.h

class tst_qobexheader_p : public QObject
{
    Q_OBJECT

private:
    OpenObexHeaderProcessor *m_helper;

    QObexHeader m_result;
    bool m_readResult;

public slots:

    void receivedHeaders(obex_t *handle, obex_object_t *obj)
    {
        // Call readOpenObexHeaders() to execute the test
        // This must be called here in a callback like this because the
        // obex_object_t will be invalid once a request has finished.
        m_readResult = QObexHeaderPrivate::readOpenObexHeaders(m_result, handle, obj);
    }

protected:
    void runTest(QList<HeaderValue *> *results = 0)
    {
        // Wait for header processor to finish the test request - i.e. wait
        // until it emits done().
        // Use SIGNAL_EXPECT & SIGNAL_WAIT instead of QEventLoop just in case
        // the signal is never emitted.
        // Don't need a QEventLoop because SIGNAL_WAIT() will drive the event
        // loop.

        QSignalSpy spy(m_helper,SIGNAL(done()));
        m_helper->run(results);
        QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
    }

    void hexdump(const QByteArray &str)
    {
        fprintf(stdout, "\n======\n");
        for (int i = 0; i < str.length(); i++) {
            if ((i % 10) == 0) {
                fprintf(stdout, "\n");
            }
            unsigned int c = static_cast<unsigned char>(str[i]);
            fprintf(stdout, "%2x ", c);
        }
        fprintf(stdout, "\n======\n");
    }

    void hexdump(const QString &str)
    {
        fprintf(stdout, "--->\n");
        for (int i = 0; i < str.length(); i++) {
            if ((i % 10) == 0) {
                fprintf(stdout, "\n");
            }
            unsigned char buf[2];
            QChar c = str[i];
            memcpy(buf, &c, 2);
            fprintf(stdout, "%2x %2x ", static_cast<unsigned int>(buf[0]), static_cast<unsigned int>(buf[1]));
        }
        fprintf(stdout, "<---\n");
    }

private slots:

    void initTestCase()
    {
        m_helper = new OpenObexHeaderProcessor;
        connect(m_helper, SIGNAL(receivedHeaders(obex_t*,obex_object_t*)),
                SLOT(receivedHeaders(obex_t*,obex_object_t*)));
    }

    void cleanupTestCase()
    {
        delete m_helper;
    }

    void init()
    {
        m_result = QObexHeader();
        m_helper->startNewRequest();
    }

    /*
    Tests for readOpenObexHeaders() follow.

    The main objective when testing readOpenObexHeaders() is to make sure that
    headers are read correctly from an OpenOBEX object into a QObexHeader.

    This is achieved by writing a header value directly to an OpenOBEX object
    for a client request, sending the request to an OBEX server, then calling
    readOpenObexHeader() on the OpenOBEX request object that is received by
    the OBEX server to place the headers into a QObexHeader object, and then
    checking the header value in the QObexHeader object.

    This essentially tests that we interpreting header data correctly when
    reading it into a QObexHeader, especially when the data is not valid for
    a particular header -- for example:
        - if a "Count" value is not received then QObexHeader::count() should
          return zero
        - QString values for "Name" and "Description" should be correctly
          converted to the unicode format defined by the OBEX specs
        - if a "Time" value is received but the time string does not conform
          to the ISO 8601 format, then QObexHeader::time() should return an
          invalid time object.
    */

    void readOpenObexHeaders_count_data() {
        HeaderTestData::rawUint32_fillData();
    }
    void readOpenObexHeaders_count()
    {
        QFETCH(uint32_t, num);

        m_helper->add4ByteHeader(OBEX_HDR_COUNT, num);
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Count), true);
        QCOMPARE(m_result.count(), num);
    }

    void readOpenObexHeaders_count_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Count), false);
        QCOMPARE(m_result.count(), quint32(0));
    }

    void readOpenObexHeaders_name_data() { HeaderTestData::setString_fillData(true); }
    void readOpenObexHeaders_name()
    {
        /*
        Write a unicode string to an obex_object_t* as raw unicode data, and check
        that the string is processed correctly back into a QString when it is
        received.
        */

        QFETCH(QString, string);
        QByteArray bytes;
        QObexHeaderPrivate::unicodeBytesFromString(bytes, string);

        m_helper->addBytesHeader(OBEX_HDR_NAME,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Name), true);
        QCOMPARE(m_result.name().size(), string.size());
        QCOMPARE(m_result.name(), string);

        // the value should never be null (null strings get sent as empty strings)
        QVERIFY(!m_result.name().isNull());
    }

    void readOpenObexHeaders_name_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Name), false);
        QVERIFY(m_result.name().isEmpty());
    }

    void readOpenObexHeaders_type_data() {
        // don't need separate tests for inserting raw strings, just use qobexheader tests
        HeaderTestData::setString_fillData(false);
    }
    void readOpenObexHeaders_type()
    {
        // TODO test the palm calendar header is added.

        QFETCH(QString, string);

        // the type value should be null terminated (as stated in specs)
        QByteArray bytes = string.toAscii();
        if (bytes.size() > 0 && bytes[bytes.size()] != '\0')
            bytes.append('\0');

        m_helper->addBytesHeader(OBEX_HDR_TYPE,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Type), true);
        QCOMPARE(m_result.type(), string);

        // the value should never be null (null strings get sent as empty strings)
        QVERIFY(!m_result.type().isNull());

        // the resulting string should not be null-terminated (the null
        // terminator should have been taken out by readOpenObexHeader())
        if (string.size() > 0) {
            QVERIFY(m_result.type()[string.size()-1] != '\0');
        }
    }

    void readOpenObexHeaders_type_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Type), false);
        QVERIFY(m_result.type().isEmpty());
    }

    void readOpenObexHeaders_length_data() {
        HeaderTestData::rawUint32_fillData();
    }
    void readOpenObexHeaders_length()
    {
        QFETCH(uint32_t, num);

        m_helper->add4ByteHeader(OBEX_HDR_LENGTH, num);
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Length), true);
        QCOMPARE(m_result.length(), num);
    }

    void readOpenObexHeaders_length_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Length), false);
        QCOMPARE(m_result.length(), quint32(0));
    }

    void readOpenObexHeaders_time_data() {
        HeaderTestData::rawTime_fillData();
    }
    void readOpenObexHeaders_time()
    {
        QFETCH(QByteArray, rawAscii);
        QFETCH(QDateTime, qDateTimeValue);
        QFETCH(bool, valid);

        m_helper->addBytesHeader(
                OBEX_HDR_TIME,
                reinterpret_cast<const uint8_t*>(rawAscii.constData()),
                rawAscii.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Time), true);
        QCOMPARE(m_result.time(), qDateTimeValue);
        QCOMPARE(qDateTimeValue.isValid(), valid);
    }

    void readOpenObexHeaders_time_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Time), false);
        QVERIFY(!m_result.time().isValid());
    }

    /*
    void readOpenObexHeaders_time4Byte_data() {
        HeaderTestData::rawTime4Byte_fillData();
    }
    void readOpenObexHeaders_time4Byte()
    {
        QFETCH(uint32_t, rawInt);
        QFETCH(QDateTime, qDateTimeValue);

        m_helper->add4ByteHeader(OBEX_HDR_TIME2, rawInt);
        runTest();

        // the QDateTime value should always be valid, since it's just created
        // from the number using setTime_t(), so it really can't ever be
        // invalid (unlike the date strings)
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Time4Byte), true);
        QCOMPARE(m_result.time4Byte(), qDateTimeValue);
        QVERIFY(qDateTimeValue.isValid());
    }

    void readOpenObexHeaders_time4Byte_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Time4Byte), false);
        QVERIFY(!m_result.time().isValid());
    }
    */

    void readOpenObexHeaders_description_data() { HeaderTestData::setString_fillData(true); }
    void readOpenObexHeaders_description()
    {
        /*
        Write a unicode string to an obex_object_t* as raw unicode data, and check
        that the string is processed correctly back into a QString when it is
        received.
        */

        QFETCH(QString, string);
        QByteArray bytes;
        QObexHeaderPrivate::unicodeBytesFromString(bytes, string);

        m_helper->addBytesHeader(OBEX_HDR_DESCRIPTION,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Description), true);
        QCOMPARE(m_result.description(), string);

        // the value should never be null (null strings get sent as empty strings)
        QVERIFY(!m_result.description().isNull());
    }

    void readOpenObexHeaders_description_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Description), false);
        QVERIFY(m_result.description().isEmpty());
    }

    void readOpenObexHeaders_target_data() {
        // don't need separate tests for inserting raw bytes, just use qobexheader tests
        HeaderTestData::setBytes_fillData();
    }
    void readOpenObexHeaders_target()
    {
        QFETCH(QByteArray, bytes);

        m_helper->addBytesHeader(OBEX_HDR_TARGET,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Target), true);
        QCOMPARE(m_result.target(), bytes);

        // the value should never be null (null bytearrays get sent as empty bytearrays)
        QVERIFY(!m_result.target().isNull());
    }

    void readOpenObexHeaders_target_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Target), false);
        QVERIFY(m_result.target().isEmpty());
    }

    void readOpenObexHeaders_http_data() {
        // don't need separate tests for inserting raw bytes, just use qobexheader tests
        HeaderTestData::setBytes_fillData();
    }
    void readOpenObexHeaders_http()
    {
        QFETCH(QByteArray, bytes);

        m_helper->addBytesHeader(OBEX_HDR_HTTP,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Http), true);
        QCOMPARE(m_result.http(), bytes);

        // the value should never be null (null bytearrays get sent as empty bytearrays)
        QVERIFY(!m_result.http().isNull());
    }

    void readOpenObexHeaders_http_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Http), false);
        QVERIFY(m_result.http().isEmpty());
    }

    void readOpenObexHeaders_who_data() {
        // don't need separate tests for inserting raw bytes, just use qobexheader tests
        HeaderTestData::setBytes_fillData();
    }
    void readOpenObexHeaders_who()
    {
        QFETCH(QByteArray, bytes);

        m_helper->addBytesHeader(OBEX_HDR_WHO,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Who), true);
        QCOMPARE(m_result.who(), bytes);

        // the value should never be null (null bytearrays get sent as empty bytearrays)
        QVERIFY(!m_result.who().isNull());
    }

    void readOpenObexHeaders_who_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::Who), false);
        QVERIFY(m_result.who().isEmpty());
    }

    void readOpenObexHeaders_connectionId_data() {
        HeaderTestData::rawUint32_fillData();
    }
    void readOpenObexHeaders_connectionId()
    {
        QFETCH(uint32_t, num);

        m_helper->add4ByteHeader(OBEX_HDR_CONNECTION, num);
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::ConnectionId), true);
        QCOMPARE(m_result.connectionId(), num);
    }

    void readOpenObexHeaders_connectionId_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::ConnectionId), false);
        QCOMPARE(m_result.connectionId(), quint32(0));
    }

    void readOpenObexHeaders_appParameters_data() {
        // don't need separate tests for inserting raw bytes, just use qobexheader tests
        HeaderTestData::setBytes_fillData();
    }
    void readOpenObexHeaders_appParameters()
    {
        QFETCH(QByteArray, bytes);

        m_helper->addBytesHeader(OBEX_HDR_APPARAM,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::AppParameters), true);
        QCOMPARE(m_result.appParameters(), bytes);

        // the value should never be null (null bytearrays get sent as empty bytearrays)
        QVERIFY(!m_result.appParameters().isNull());
    }

    void readOpenObexHeaders_appParameters_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::AppParameters), false);
        QVERIFY(m_result.appParameters().isEmpty());
    }

    void readOpenObexHeaders_creatorId_data() {
        HeaderTestData::rawUint32_fillData();
    }
    void readOpenObexHeaders_creatorId()
    {
        QFETCH(uint32_t, num);

        m_helper->add4ByteHeader(OBEX_HDR_CREATOR, num);
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::CreatorId), true);
        QCOMPARE(m_result.creatorId(), num);
    }

    void readOpenObexHeaders_creatorId_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::CreatorId), false);
        QCOMPARE(m_result.creatorId(), quint32(0));
    }

    void readOpenObexHeaders_wanUuid_data()
    {
        QTest::addColumn<QByteArray>("rawUuid");
        QTest::addColumn<QUuid>("quuidValue");
        QTest::addColumn<bool>("isNull");

        // an empty UUID byte stream should be converted to a null QUuid, and
        // vice-versa
        QTest::newRow("empty raw uuid")
                << QByteArray("")
                << QUuid()
                << true;

        QTest::newRow("some uuid")
            << QByteArray("\x12\x34\x56\x78\x12\x34\x56\x78\x12\x34\x56\x78\x12\x34\x56\x78",
                          16)
            << QUuid("12345678-1234-5678-1234-567812345678")
            << false;

        // Don't know if there's any point in having a "bad UUID" (e.g. "123-34")
        // because it still gets turned into a valid object by QUuid on construction.
    }
    void readOpenObexHeaders_wanUuid()
    {
        QFETCH(QByteArray, rawUuid);
        QFETCH(QUuid, quuidValue);
        QFETCH(bool, isNull);

        m_helper->addBytesHeader(OBEX_HDR_WANUUID,
                                 reinterpret_cast<const uint8_t*>(rawUuid.constData()),
                                 rawUuid.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::WanUuid), true);
        QCOMPARE(m_result.wanUuid(), quuidValue);
        QCOMPARE(m_result.wanUuid().isNull(), isNull);
    }

    void readOpenObexHeaders_wanUuid_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::WanUuid), false);
        QVERIFY(m_result.wanUuid().isNull());
    }


    void readOpenObexHeaders_objectClass_data() {
        // don't need separate tests for inserting raw bytes, just use qobexheader tests
        HeaderTestData::setBytes_fillData();
    }
    void readOpenObexHeaders_objectClass()
    {
        QFETCH(QByteArray, bytes);

        m_helper->addBytesHeader(OBEX_HDR_OBJECTCLASS,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::ObjectClass), true);
        QCOMPARE(m_result.objectClass(), bytes);

        // the value should never be null (null bytearrays get sent as empty bytearrays)
        QVERIFY(!m_result.objectClass().isNull());
    }

    void readOpenObexHeaders_objectClass_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::ObjectClass), false);
        QVERIFY(m_result.objectClass().isEmpty());
    }

    void readOpenObexHeaders_sessionParameters_data() {
        // don't need separate tests for inserting raw bytes, just use qobexheader tests
        HeaderTestData::setBytes_fillData();
    }
    void readOpenObexHeaders_sessionParameters()
    {
        QFETCH(QByteArray, bytes);

        m_helper->addBytesHeader(OBEX_HDR_SESSIONPARAM,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::SessionParameters), true);
        QCOMPARE(m_result.sessionParameters(), bytes);

        // the value should never be null (null bytearrays get sent as empty bytearrays)
        QVERIFY(!m_result.sessionParameters().isNull());
    }

    void readOpenObexHeaders_sessionParameters_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::SessionParameters), false);
        QVERIFY(m_result.sessionParameters().isEmpty());
    }

    void readOpenObexHeaders_sessionSequenceNumber_data() {
        HeaderTestData::rawUint8_fillData();
    }
    void readOpenObexHeaders_sessionSequenceNumber()
    {
        QFETCH(uint8_t, num);

        m_helper->add1ByteHeader(OBEX_HDR_SESSIONSEQ, num);
        runTest();

        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::SessionSequenceNumber), true);

        QCOMPARE(m_result.sessionSequenceNumber(), num);
    }

    void readOpenObexHeaders_sessionSequenceNumber_none()
    {
        runTest();
        QVERIFY(m_readResult);
        QCOMPARE(m_result.contains(QObexHeader::SessionSequenceNumber), false);
        QCOMPARE(m_result.sessionSequenceNumber(), uint8_t(0));
    }

    /*
    Tests for reading custom headers follow.
    */

    void readOpenObexHeaders_setValue_unicode_data() { HeaderTestData::setString_fillData(true); }
    void readOpenObexHeaders_setValue_unicode()
    {
        QFETCH(QString, string);
        int customHeaderId = (0x30 | QObexHeaderPrivate::HeaderUnicodeEncoding);
        QByteArray bytes;
        QObexHeaderPrivate::unicodeBytesFromString(bytes, string);
        m_helper->addBytesHeader(customHeaderId,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QVERIFY(m_result.contains(customHeaderId));
        QCOMPARE(m_result.value(customHeaderId).toString().size(), string.size());
        QCOMPARE(m_result.value(customHeaderId).toString(), string);
    }

    void readOpenObexHeaders_setValue_byteStream_data() { HeaderTestData::setBytes_fillData(); }
    void readOpenObexHeaders_setValue_byteStream()
    {
        QFETCH(QByteArray, bytes);
        int customHeaderId = (0x30 | QObexHeaderPrivate::HeaderByteSequenceEncoding);
        m_helper->addBytesHeader(customHeaderId,
                                 reinterpret_cast<const uint8_t*>(bytes.constData()),
                                 bytes.size());
        runTest();

        QVERIFY(m_readResult);
        QVERIFY(m_result.contains(customHeaderId));
        QCOMPARE(m_result.value(customHeaderId).toByteArray(), bytes);
    }

    void readOpenObexHeaders_setValue_byte_data() { HeaderTestData::rawUint8_fillData(); }
    void readOpenObexHeaders_setValue_byte()
    {
        QFETCH(quint8, num);
        int customHeaderId = (0x30 | QObexHeaderPrivate::HeaderByteEncoding);
        m_helper->add1ByteHeader(customHeaderId, num);
        runTest();

        QVERIFY(m_readResult);
        QVERIFY(m_result.contains(customHeaderId));
        QCOMPARE(m_result.value(customHeaderId).value<quint8>(), num);
    }

    void readOpenObexHeaders_setValue_uint_data() { HeaderTestData::rawUint32_fillData(); }
    void readOpenObexHeaders_setValue_uint()
    {
        QFETCH(quint32, num);
        int customHeaderId = (0x30 | QObexHeaderPrivate::HeaderIntEncoding);
        m_helper->add4ByteHeader(customHeaderId, num);
        runTest();

        QVERIFY(m_readResult);
        QVERIFY(m_result.contains(customHeaderId));
        QCOMPARE(m_result.value(customHeaderId).value<quint32>(), num);
    }


    //---------------------------------------------------------

    /*
    Tests for writeOpenObexHeaders() follow.

    The main objective when testing writeOpenObexHeaders() is to test that
    headers values that are added to a QObexHeader are written correctly to
    the OpenOBEX object (before it is sent over the transport connection)

    Since we cannot look at an OpenOBEX object before it is sent (the OpenOBEX
    library does not offer this capability), the test is executed by writing
    a test value to a QObexHeader object, calling writeOpenObexHeaders() to
    put the value into the OpenOBEX object of a client request, then sending
    the request to an OBEX server, and finally checking the headers that are
    received by the OBEX server. The tests use the more low-level OpenOBEX
    headers when checking the received headers to ensure the values have not
    been converted or reinterpreted in any way (e.g. when parsing into a
    QObexHeader).

    This essentially tests that various values are written correctly to
    low-level OpenOBEX objects. For example, if no "Name" value has been set
    in the QObexHeader, then this header should not be present in the received
    OpenOBEX object; or, if a null QByteArray has been set for the "Target"
    value, then an empty byte stream should be set for the "Target" in the
    OpenOBEX object, rather than sending nothing at all.
    */

    /*
    If haven't written anything to QObexHeader, should get nothing on other side.
    */
    void writeOpenObexHeaders_none()
    {
        QObexHeader h;
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QVERIFY(results.isEmpty());
    }

    void writeOpenObexHeaders_count_data() { HeaderTestData::setQuint32_fillData(); }
    void writeOpenObexHeaders_count()
    {
        QFETCH(quint32, num);

        QObexHeader h;
        h.setCount(num);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Count));
        QCOMPARE(header->m_bq4, num);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_name_data() { HeaderTestData::setString_fillData(true); }
    void writeOpenObexHeaders_name()
    {
        QFETCH(QString, string);

        QObexHeader h;
        h.setName(string);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Name));

        QByteArray bytes;
        QObexHeaderPrivate::unicodeBytesFromString(bytes, string);

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(resultBytes, bytes);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_type_data() { HeaderTestData::setString_fillData(false); }
    void writeOpenObexHeaders_type()
    {
        QFETCH(QString, string);

        QObexHeader h;
        h.setType(string);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Type));

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(QString::fromAscii(resultBytes), string);

        // writeOpenObexHeaders() should have added a null terminator to the
        // string (OBEX specs define that the type value should be null-terminated)
        QString s = QString::fromAscii(resultBytes, header->size()); // don't crop at null
        if (s.size() > 0)
            QVERIFY(s[s.size()-1] == '\0');

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_length_data() { HeaderTestData::setQuint32_fillData(); }
    void writeOpenObexHeaders_length()
    {
        QFETCH(quint32, num);

        QObexHeader h;
        h.setLength(num);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Length));
        QCOMPARE(header->m_bq4, num);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_time_data() { HeaderTestData::rawTime_fillData(); }
    void writeOpenObexHeaders_time()
    {
        QFETCH(QByteArray, rawAscii);
        QFETCH(QDateTime, qDateTimeValue);

        QObexHeader h;
        h.setTime(qDateTimeValue);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        if (qDateTimeValue.isValid()) {

            QCOMPARE(results.size(), 1);
            HeaderValue *header = results[0];
            QCOMPARE(header->id(), int(QObexHeader::Time));

            uint8_t *buf;
            header->readBuf(&buf);
            QByteArray bytes(reinterpret_cast<const char*>(buf), header->size());

            QCOMPARE(bytes, rawAscii);

        } else {
            // setTime() should not have added the header if the datetime value
            // was invalid
            QCOMPARE(results.size(), 0);
        }

        HeaderValue::clearList(results);
    }

    /*
    void writeOpenObexHeaders_time4Byte_data() { HeaderTestData::rawTime4Byte_fillData(); }
    void writeOpenObexHeaders_time4Byte()
    {
        QFETCH(uint32_t, rawInt);
        QFETCH(QDateTime, qDateTimeValue);

        QObexHeader h;
        h.setTime4Byte(qDateTimeValue);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Time4Byte));
        QCOMPARE(header->m_bq4, rawInt);

        HeaderValue::clearList(results);
    }
    */

    void writeOpenObexHeaders_description_data() { HeaderTestData::setString_fillData(true); }
    void writeOpenObexHeaders_description()
    {
        QFETCH(QString, string);

        QObexHeader h;
        h.setDescription(string);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Description));

        QByteArray bytes;
        QObexHeaderPrivate::unicodeBytesFromString(bytes, string);

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(resultBytes, bytes);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_target_data() { HeaderTestData::setBytes_fillData(); }
    void writeOpenObexHeaders_target()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setTarget(bytes);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Target));

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(bytes, resultBytes);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_http_data() { HeaderTestData::setBytes_fillData(); }
    void writeOpenObexHeaders_http()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setHttp(bytes);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Http));

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(bytes, resultBytes);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_who_data() { HeaderTestData::setBytes_fillData(); }
    void writeOpenObexHeaders_who()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setWho(bytes);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::Who));

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(bytes, resultBytes);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_connectionId_data() { HeaderTestData::setQuint32_fillData(); }
    void writeOpenObexHeaders_connectionId()
    {
        QFETCH(quint32, num);

        QObexHeader h;
        h.setConnectionId(num);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::ConnectionId));
        QCOMPARE(header->m_bq4, num);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_appParameters_data() { HeaderTestData::setBytes_fillData(); }
    void writeOpenObexHeaders_appParameters()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setAppParameters(bytes);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::AppParameters));

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(bytes, resultBytes);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_creatorId_data() { HeaderTestData::setQuint32_fillData(); }
    void writeOpenObexHeaders_creatorId()
    {
        QFETCH(quint32, num);

        QObexHeader h;
        h.setCreatorId(num);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::CreatorId));
        QCOMPARE(header->m_bq4, num);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_objectClass_data() { HeaderTestData::setBytes_fillData(); }
    void writeOpenObexHeaders_objectClass()
    {
        QFETCH(QByteArray, bytes);

        QObexHeader h;
        h.setObjectClass(bytes);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::ObjectClass));

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(bytes, resultBytes);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_wanUuid_data()
    {
        QTest::addColumn<QUuid>("quuidValue");
        QTest::addColumn<QByteArray>("rawUuid");

        // if you set a null QUuid as the wan uuid, it should send the equivalent
        // of the "0000..." uuid (rather than sending nothing at all).
        QTest::newRow("null uuid")
            << QUuid()
            << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
                          16);

        QTest::newRow("some uuid")
            << QUuid("12345678-1234-5678-1234-567812345678")
            << QByteArray("\x12\x34\x56\x78\x12\x34\x56\x78\x12\x34\x56\x78\x12\x34\x56\x78",
                          16);
    }

    void writeOpenObexHeaders_wanUuid()
    {
        QFETCH(QByteArray, rawUuid);
        QFETCH(QUuid, quuidValue);

        QObexHeader h;
        h.setWanUuid(quuidValue);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::WanUuid));

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(resultBytes, rawUuid);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_sessionSequenceNumber_data() { HeaderTestData::setQuint8_fillData(); }
    void writeOpenObexHeaders_sessionSequenceNumber()
    {
        QFETCH(uint8_t, num);

        QObexHeader h;
        h.setSessionSequenceNumber(num);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), int(QObexHeader::SessionSequenceNumber));
        QCOMPARE(header->m_bq1, num);

        HeaderValue::clearList(results);
    }

    /*
    Tests for writing custom headers follow.
    */

    void writeOpenObexHeaders_setValue_unicode_data() { HeaderTestData::setString_fillData(true); }
    void writeOpenObexHeaders_setValue_unicode()
    {
        QFETCH(QString, string);
        int customHeaderId = (0x30 | QObexHeaderPrivate::HeaderUnicodeEncoding);
        QObexHeader h;
        h.setValue(customHeaderId, string);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        // check custom header was received
        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), customHeaderId);

        // check raw unicode value is correct
        QByteArray bytes;
        QObexHeaderPrivate::unicodeBytesFromString(bytes, string);
        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(resultBytes.size(), bytes.size());
        QCOMPARE(resultBytes, bytes);

        HeaderValue::clearList(results);

        // TODO add test where there's bad data e.g. not null terminated unicode
    }

    void writeOpenObexHeaders_setValue_byteStream_data() { HeaderTestData::setBytes_fillData(); }
    void writeOpenObexHeaders_setValue_byteStream()
    {
        QFETCH(QByteArray, bytes);
        int customHeaderId = (0x30 | QObexHeaderPrivate::HeaderByteSequenceEncoding);
        QObexHeader h;
        h.setValue(customHeaderId, bytes);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), customHeaderId);

        uint8_t *buf;
        header->readBuf(&buf);
        QByteArray resultBytes(reinterpret_cast<const char*>(buf), header->size());
        QCOMPARE(resultBytes, bytes);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_setValue_byte_data() { HeaderTestData::rawUint8_fillData(); }
    void writeOpenObexHeaders_setValue_byte()
    {
        // Must use qVariantFromValue() when creating variant or it will do
        // some weird cast and add a zero instead of the proper quint8 value.
        QFETCH(uint8_t, num);
        int customHeaderId = (0x30 | QObexHeaderPrivate::HeaderByteEncoding);
        QObexHeader h;
        h.setValue(customHeaderId, qVariantFromValue<uint8_t>(num));
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), customHeaderId);

        QCOMPARE(header->m_bq1, num);

        HeaderValue::clearList(results);
    }

    void writeOpenObexHeaders_setValue_uint_data() { HeaderTestData::rawUint32_fillData(); }
    void writeOpenObexHeaders_setValue_uint()
    {
        QFETCH(uint32_t, num);
        int customHeaderId = (0x30 | QObexHeaderPrivate::HeaderIntEncoding);
        QObexHeader h;
        h.setValue(customHeaderId, num);
        QObexHeaderPrivate::writeOpenObexHeaders(m_helper->clientHandle(), m_helper->currentRequest(), true, h);

        QList<HeaderValue *> results;
        runTest(&results);

        QCOMPARE(results.size(), 1);
        HeaderValue *header = results[0];
        QCOMPARE(header->id(), customHeaderId);
        QCOMPARE(header->m_bq4, num);

        HeaderValue::clearList(results);
    }


    //----------------------------------------------------------------
    /*
    Tests for other functions in qobexheader_p.h follow.
    */

protected:
    static QByteArray toUnicodeBytes(const ushort *uc, int ucSize)
    {
        QByteArray bytes;
        bytes.resize(ucSize);
        memcpy(bytes.data(), uc, bytes.size());
        return bytes;
    }


private slots:

    /*
    unicodeBytesFromString() should convert a QString into a QByteArray of unicode bytes
    that conform to the OBEX unicode data requirements (UTF-16, network order,
    null terminated).
    */
    void unicodeBytesFromStringTest_data()
    {
        QTest::addColumn<QString>("string");
        QTest::addColumn<QByteArray>("unicodeBytes");

        // test empty string = empty bytes
        QTest::newRow("unicode size 0") << QString("") << QByteArray("");

        // test 1 byte QString = 4 byte OBEX unicode
        QString oneByte_qstring(QChar(0x0055));
        const ushort oneByte_raw[] = { htons(0x0055), 0x0000, 0x0000 };
        QTest::newRow("unicode size 1")
                << oneByte_qstring
                << toUnicodeBytes(oneByte_raw, 1*2 + 2);  // size*2 + 2 nulls

        // test 4 byte QString = 10 byte OBEX unicode
        const QChar fourByte_qchars[4] = { 0x006e, 0x0055, 0x006e, 0x03a3 };
        QString fourByte_qstring(fourByte_qchars, 4);

        const ushort fourByte_raw[] = { htons(0x006e), htons(0x0055), htons(0x006e), htons(0x03a3), 0x0000, 0x0000 };

        QTest::newRow("unicode size 4")
                << fourByte_qstring
                << toUnicodeBytes(fourByte_raw, 4*2 + 2);  // size*2 + 2 nulls


        // What happens if the original qstring ends with a null already? I guess
        // then the resulting bytes should have 3 nulls on the end...? QStrings
        // shouldn't normally have nulls in them unless the user explicitly put
        // one in, so it probably shouldn't be cropped off.
        const QChar strangeString_qchars[2] = { 0x03a3, 0x0000 };
        QString strangeString_qstring(strangeString_qchars, 2);
        const ushort strangeString_raw[] = { htons(0x03a3), 0x0000, 0x0000, 0x0000 };
        QTest::newRow("string with null terminator")
                << strangeString_qstring
                << toUnicodeBytes(strangeString_raw, 2*2 + 2);  // size*2 + 2 nulls
    }

    void unicodeBytesFromStringTest()
    {
        QFETCH(QString, string);
        QFETCH(QByteArray, unicodeBytes);

        QByteArray bytes;
        QObexHeaderPrivate::unicodeBytesFromString(bytes, string);
        QCOMPARE(bytes.size(), unicodeBytes.size());
        QCOMPARE(bytes, unicodeBytes);

        if (bytes.size() > 0) {
            // confirm the last 2 bytes are null terminators
            QCOMPARE( static_cast<ushort>(bytes.data()[bytes.size()-1]),
                    static_cast<ushort>(0x0000) );
            QCOMPARE( static_cast<ushort>(bytes.data()[bytes.size()-2]),
                    static_cast<ushort>(0x0000) );
        }
    }

    /*
    stringFromRawUnicode() should convert a QByteArray of unicode bytes from an OBEX
    header value (should be UTF-16, network order, null terminated) into an
    ordinary QString (in host order, not null terminated).
    */
    void stringFromRawUnicodeTest_data()
    {
        QTest::addColumn<QByteArray>("unicodeBytes");
        QTest::addColumn<QString>("string");

        // test empty unicode = empty qstring
        QTest::newRow("unicode size 0") << QByteArray("") << QString("");

        // test 1 byte raw unicode = empty qstring (byte size < 2 is invalid)
        QTest::newRow("unicode size 0") << QByteArray("a") << QString("");

        // test 2 byte raw unicode of nulls = empty qstring
        const ushort nullsOnly_raw[] = { 0x0000, 0x0000 };
        QTest::newRow("unicode two null chars")
                << toUnicodeBytes(nullsOnly_raw, 2)
                << QString();

        // test bad data - no null terminators - still get good qstring
        const ushort noNulls_raw[] = { htons(0x03a3), htons(0x0055) };
        const QChar noNulls_qchars[] = { 0x03a3, 0x0055 };
        QTest::newRow("bad data - no null chars")
                << toUnicodeBytes(noNulls_raw, 2*2)
                << QString(noNulls_qchars, 2);

        // test bad data - only 1 null terminator - still get good qstring
        const ushort oneNullOnly_raw[] = { htons(0x03a3), 0x0000 };
        const QChar oneNullOnly_qchars[] = { 0x03a3 };
        QTest::newRow("bad data - only 1 null char")
                << toUnicodeBytes(oneNullOnly_raw, 1*2 + 1)
                << QString(oneNullOnly_qchars, 1);

        // test unicode of 1 char + 2 nulls
        const ushort oneChar_raw[] = { htons(0x0055), 0x0000, 0x0000 };
        QTest::newRow("unicode size 1")
                << toUnicodeBytes(oneChar_raw, 1*2 + 2)   // size*2 + 2 nulls
                << QString(QChar(0x0055));

        // test unicode of 3 chars + 2 nulls
        const ushort threeChars_raw[] = { htons(0x0055), htons(0x03a3), htons(0x03a3), 0x0000, 0x0000 };
        const QChar threeChars_qchars[] = { 0x0055, 0x03a3, 0x03a3 };
        QTest::newRow("unicode size 3")
                << toUnicodeBytes(threeChars_raw, 3*2 + 2)    // size*2 + 2 nulls
                << QString(threeChars_qchars, 3);
    }

    void stringFromRawUnicodeTest()
    {
        QFETCH(QString, string);
        QFETCH(QByteArray, unicodeBytes);

        uchar data[unicodeBytes.size()];
        memcpy(&data, unicodeBytes.constData(), unicodeBytes.size());

        QString s;
        QObexHeaderPrivate::stringFromUnicodeBytes(s, data, unicodeBytes.size());
        QCOMPARE(s.size(), string.size());
        QCOMPARE(s, string);

        if (s.size() > 0) {
            // confirm there is no null terminator in the last 2 bytes
            QVERIFY( s[s.size()-1] != QChar(0x0000) );
            if (s.size() > 1)
                QVERIFY( s[s.size()-2] != QChar(0x0000) );
        }
    }


    //==================================================================

    /*
    void check_unicode_processing()
    {
        const QChar unicode[] = {
            0x005A, 0x007F, 0x00A4, 0x0060,
            0x1009, 0x0020, 0x0020};
        int uc_size = sizeof(unicode) / sizeof(QChar);
        QString s = QString(unicode, uc_size);
        int size = (s.size() + 1) * 2;

        //QByteArray sourceBytes;
        //sourceBytes.resize(size);
        //memcpy(sourceBytes.data(), reinterpret_cast<const char*>(s.utf16()), size);

        QByteArray sourceBytes;
        stringToRawUnicode(s, sourceBytes);

        m_helper->addBytesHeader(OBEX_HDR_NAME,
                                 reinterpret_cast<const uint8_t*>(sourceBytes.constData()),
                                 sourceBytes.size());

        QList<HeaderValue *> results;
        runTest(&results);

        // verify that the received bytes are same as the ones we sent
        HeaderValue *h = results[0];
        QByteArray recvdBytes;
        recvdBytes.resize(h->size());
        h->readBuf(reinterpret_cast<uint8_t*>(recvdBytes.data()));
        QCOMPARE(sourceBytes, recvdBytes);

        hexdump(sourceBytes);


        //QString foo = "test";
        QString foo = QString(unicode, uc_size);
        fprintf(stdout, "Hi mom1\n");
        hexdump(foo);

        QByteArray result;
        stringToRawUnicode(foo, result);

        fprintf(stdout, "Hi mom2\n");
        hexdump(result);

        fprintf(stdout, "result.size(): %d\n", result.size());

        QString recvdStr;
        rawUnicodeToString(reinterpret_cast<const uchar*>(result.constData()),
                result.size(), recvdStr);

        hexdump(recvdStr);

        QCOMPARE(recvdStr, s);
    }
    */
};


QTEST_MAIN(tst_qobexheader_p)
#include "tst_qobexheader_p.moc"
