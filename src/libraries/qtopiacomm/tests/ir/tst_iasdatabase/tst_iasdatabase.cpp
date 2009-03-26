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
#include <QDebug>
#include <QEventLoop>
#include <QStringList>
#include <QList>
#include <QVariant>

#include <qirnamespace.h>
#include <qiriasdatabase.h>

//TESTED_CLASS=QIrIasDatabase
//TESTED_FILES=src/libraries/qtopiacomm/ir/qiriasdatabase.cpp

//
// The test case
//
class tst_IasDatabase: public QObject
{
    Q_OBJECT
public:
    tst_IasDatabase();
    virtual ~tst_IasDatabase();


private slots:
    void init();
    void initTestCase();
    void cleanupTestCase();
    void testSetAttributes();
    void testGetAttributes();
    void testRemoveAttributes();
};

tst_IasDatabase::tst_IasDatabase() : QObject()
{

}

tst_IasDatabase::~tst_IasDatabase()
{

}

void tst_IasDatabase::initTestCase()
{
}

void tst_IasDatabase::cleanupTestCase()
{
}

void tst_IasDatabase::init()
{
}

void tst_IasDatabase::testSetAttributes()
{
    QSKIP("Test needs to be checked for correctness", SkipAll);
    QVariant strAttr = QVariant::fromValue(QString("Bleah"));
    QVariant intAttr = QVariant::fromValue(uint(54));
    QVariant octAttr = QVariant::fromValue(QByteArray("234893ksdfj"));

    bool ret;
    ret = QIrIasDatabase::setAttribute("my:foobar", "foobarint", intAttr);
    QVERIFY(ret);
    ret = QIrIasDatabase::setAttribute("my:foobar", "foobarstr", strAttr);
    QVERIFY(ret);
    ret = QIrIasDatabase::setAttribute("my:foobar", "foobarseq", octAttr);
    QVERIFY(ret);
}

void tst_IasDatabase::testGetAttributes()
{
    QSKIP("Test needs to be checked for correctness", SkipAll);
    QVariant attr;

    QVERIFY(attr.isValid() == false);

    attr = QIrIasDatabase::attribute("my:foobar", "nonexistant");
    QVERIFY(attr.isValid() == false);

    attr = QIrIasDatabase::attribute("my:foobar", "foobarstr");
    QVERIFY(attr.type() == QVariant::String);
    QVERIFY(attr.value<QString>() == "Bleah");

    attr = QIrIasDatabase::attribute("my:foobar", "foobarint");
    QVERIFY(attr.type() == QVariant::UInt);
    QVERIFY(attr.value<uint>() == 54);

    attr = QIrIasDatabase::attribute("my:foobar", "foobarseq");
    QVERIFY(attr.type() == QVariant::ByteArray);
    QVERIFY(attr.value<QByteArray>() == "234893ksdfj");
}

void tst_IasDatabase::testRemoveAttributes()
{
    QSKIP("Test needs to be checked for correctness", SkipAll);
    bool ret;

    ret = QIrIasDatabase::removeAttribute("my:foobar", "nonexistant");
    QVERIFY(ret == false);

    ret = QIrIasDatabase::removeAttribute("my:foobar", "foobarstr");
    QVERIFY(ret);

    ret = QIrIasDatabase::removeAttribute("my:foobar", "foobarint");
    QVERIFY(ret);

    ret = QIrIasDatabase::removeAttribute("my:foobar", "foobarseq");
    QVERIFY(ret);
}

QTEST_MAIN( tst_IasDatabase )

#include "tst_iasdatabase.moc"
        
