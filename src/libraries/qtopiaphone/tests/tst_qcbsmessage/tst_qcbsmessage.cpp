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

#include <QCBSMessage>
#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QtopiaApplication>


//TESTED_CLASS=QCBSMessage
//TESTED_FILES=src/libraries/qtopiaphone/qcbsmessage.h

/*
    The tst_QCBSMessage class provides unit tests for the QCBSMessage class.

    This test is incomplete.  QCBSMessage::fromPdu and QCBSMessage::toPdu are not
    yet tested, but should be.
*/
class tst_QCBSMessage : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void setGet();
    void setGet_data();
};

QTEST_APP_MAIN( tst_QCBSMessage, QtopiaApplication )
#include "tst_qcbsmessage.moc"

/*?
    Warns of incomplete test.
*/
void tst_QCBSMessage::initTestCase()
{
    qWarning("toPdu and fromPdu not yet tested!");
}

/*?
    Data for the setGet() test function.
*/
void tst_QCBSMessage::setGet_data()
{
    QTest::addColumn<uint>("channel");
    QTest::addColumn<uint>("language");
    QTest::addColumn<uint>("numPages");
    QTest::addColumn<uint>("page");
    QTest::addColumn<uint>("scope");
    QTest::addColumn<uint>("updateNumber");
    QTest::addColumn<QString>("text");
    
    // Typical test case
    QTest::newRow("simple")
        << (uint)50
        << (uint)QCBSMessage::English
        << (uint)10
        << (uint)5
        << (uint)QCBSMessage::LocationAreaWide
        << (uint)3
        << "Hi there";

    // Very large numbers
    QTest::newRow("extreme")
        << (uint)50000
        << (uint)QCBSMessage::German
        << (uint)10000000
        << (uint)500000
        << (uint)QCBSMessage::CellWide
        << (uint)1234567890
        << "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // Zero numbers
    QTest::newRow("zero")
        << (uint)0
        << (uint)QCBSMessage::German
        << (uint)0
        << (uint)0
        << (uint)QCBSMessage::CellWide
        << (uint)0
        << QString();

    // Current page larger than number of pages.
    // Currently QCBSMessage accepts this with no complaint.
    QTest::newRow("page too large")
        << (uint)50
        << (uint)QCBSMessage::Norwegian
        << (uint)10
        << (uint)50
        << (uint)QCBSMessage::PLMNWide
        << (uint)0
        << "Brisbane";

    // Test case for every supported language.
    for (int i = QCBSMessage::German; i <= QCBSMessage::Turkish; ++i) {
        QTest::newRow(QString("language %1").arg(i).toLatin1())
            << (uint)50
            << (uint)i
            << (uint)50
            << (uint)10
            << (uint)QCBSMessage::PLMNWide
            << (uint)0
            << "Brisbane";
    }

    // Test case for every supported scope.
    for (int i = QCBSMessage::CellWide; i <= QCBSMessage::CellWide2; ++i) {
        QTest::newRow(QString("scope %1").arg(i).toLatin1())
            << (uint)50
            << (uint)QCBSMessage::English
            << (uint)50
            << (uint)10
            << (uint)i
            << (uint)0
            << "Oslo";
    }
}

/*?
    Test function for setting and getting QCBSMessage attributes.
    This test:
        * Constructs a QCBSMessage with the default constructor.
        * Sets all attributes to given test values.
        * Retrieves the attributes and tests they have been set
          to the correct value.
        * Constructs a copy of the first QCBSMessage with the copy
          constructor.
        * Retrieves the attributes of the copy and tests they have
          been set to the correct value.
*/
void tst_QCBSMessage::setGet()
{
    QFETCH(uint, channel);
    QFETCH(uint, language);
    QFETCH(uint, numPages);
    QFETCH(uint, page);
    QFETCH(uint, scope);
    QFETCH(uint, updateNumber);
    QFETCH(QString, text);

    QCBSMessage message;
    message.setChannel(channel);
    message.setLanguage((QCBSMessage::Language)language);
    message.setNumPages(numPages);
    message.setPage(page);
    message.setScope((QCBSMessage::GeographicalScope)scope);
    message.setUpdateNumber(updateNumber);
    message.setText(text);

    QCOMPARE(channel, message.channel());
    QCOMPARE(language, (uint)message.language());
    QCOMPARE(numPages, message.numPages());
    QCOMPARE(page, message.page());
    QCOMPARE(scope, (uint)message.scope());
    QCOMPARE(updateNumber, message.updateNumber());
    QCOMPARE(text, message.text());

    QCBSMessage copy(message);
    QCOMPARE(copy, message);
    QCOMPARE(channel, copy.channel());
    QCOMPARE(language, (uint)copy.language());
    QCOMPARE(numPages, copy.numPages());
    QCOMPARE(page, copy.page());
    QCOMPARE(scope, (uint)copy.scope());
    QCOMPARE(updateNumber, copy.updateNumber());
    QCOMPARE(text, copy.text());
}
