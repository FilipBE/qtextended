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

#include <qtopiacomm/private/qbluetoothremotedeviceselector_p.h>
#include <qbluetoothremotedevice.h>

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <shared/util.h>
#include <QDebug>

//TESTED_CLASS=QBluetoothRemoteDeviceSelector
//TESTED_FILES=src/libraries/qtopiacomm/bluetooth/qbluetoothremotedeviceselector.cpp


static QList<QBluetoothAddress> btStringsToAddresses(const QStringList &addressStrings)
{
    QList<QBluetoothAddress> l;
    foreach (QString addr, addressStrings) {
        l << QBluetoothAddress(addr);
    }
    return l;
}


class tst_QBluetoothRemoteDeviceSelector : public QObject
{
    Q_OBJECT

private:
    QBluetoothRemoteDeviceSelector *m_sel;

private slots:

    void init()
    {
        m_sel = new QBluetoothRemoteDeviceSelector;
    }

    void cleanup()
    {
        delete m_sel;
    }

    void insert_data()
    {
        QTest::addColumn<QStringList>("addressesToInsert");
        QTest::addColumn< QList<QVariant> >("insertResults");

        QTest::newRow("insert one")
                << QStringList("aa:aa:aa:aa:aa:aa")
                << (QList<QVariant>() << true);

        QTest::newRow("insert three different addresses")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc")
                << (QList<QVariant>() << true << true << true);

        QTest::newRow("insert same one 3 times")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "aa:aa:aa:aa:aa:aa" << "aa:aa:aa:aa:aa:aa")
                << (QList<QVariant>() << true << false << false);
    }

    void insert()
    {
        QFETCH(QStringList, addressesToInsert);
        QFETCH(QList<QVariant>, insertResults);

        QCOMPARE(m_sel->count(), 0);

        int count = 0;
        for (int i=0; i<addressesToInsert.size(); i++) {
            QBluetoothAddress addr = QBluetoothAddress(addressesToInsert[i]);
            QCOMPARE(m_sel->insert(QBluetoothRemoteDevice(addr)), insertResults[i].toBool());
            if (insertResults[i].toBool())
                count++;
            QCOMPARE(m_sel->count(), count);
            QVERIFY(m_sel->contains(addr));
        }

        QCOMPARE(m_sel->devices().toSet(), btStringsToAddresses(addressesToInsert).toSet());
    }

    void remove_data()
    {
        QTest::addColumn<QStringList>("addressesToInsert");
        QTest::addColumn<QStringList>("addressesToRemove");
        QTest::addColumn< QList<QVariant> >("removeResults");

        QTest::newRow("no addresses, remove one")
                << QStringList()
                << QStringList("aa:aa:aa:aa:aa:aa")
                << (QList<QVariant>() << false);

        QTest::newRow("one address, remove two")
                << QStringList("aa:aa:aa:aa:aa:aa")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb")
                << (QList<QVariant>() << true << false);

        QTest::newRow("three addresses, remove first")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc")
                << QStringList("aa:aa:aa:aa:aa:aa")
                << (QList<QVariant>() << true);

        QTest::newRow("three addresses, remove second")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc")
                << QStringList("bb:bb:bb:bb:bb:bb")
                << (QList<QVariant>() << true);

        QTest::newRow("three addresses, remove last")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc")
                << QStringList("cc:cc:cc:cc:cc:cc")
                << (QList<QVariant>() << true);

        QTest::newRow("remove non-existent addresses")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc"
                                  << "dd:dd:dd:dd:dd:dd" << "ee:ee:ee:ee:ee:ee")
                << (QList<QVariant>() << true << true << true << false << false);
    }

    void remove()
    {
        QFETCH(QStringList, addressesToInsert);
        QFETCH(QStringList, addressesToRemove);
        QFETCH(QList<QVariant>, removeResults);

        int i;
        for (i=0; i<addressesToInsert.size(); i++) {
            m_sel->insert(QBluetoothRemoteDevice(QBluetoothAddress(addressesToInsert[i])));
        }

        int count = m_sel->count();
        for (int i=0; i<addressesToRemove.size(); i++) {
            QBluetoothAddress addr = QBluetoothAddress(addressesToRemove[i]);
            QCOMPARE(m_sel->remove(addr), removeResults[i].toBool());
            if (removeResults[i].toBool()) {
                count--;
                QVERIFY(!m_sel->contains(addr));
            }
            QCOMPARE(m_sel->count(), count);
        }

        QSet<QString> remainingAddresses = addressesToInsert.toSet() - addressesToRemove.toSet();
        QCOMPARE(m_sel->devices().toSet(),
                 btStringsToAddresses(QList<QString>::fromSet(remainingAddresses)).toSet());
    }

    void clear_data()
    {
        QTest::addColumn<QStringList>("addressesToInsert");

        QTest::newRow("insert one")
                << QStringList("aa:aa:aa:aa:aa:aa");
        QTest::newRow("insert two")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb");
        QTest::newRow("insert duplicates")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "bb:bb:bb:bb:bb:bb");
        QTest::newRow("insert many")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc"
                                  << "dd:dd:dd:dd:dd:dd" << "ee:ee:ee:ee:ee:ee");
    }

    void clear()
    {
        QFETCH(QStringList, addressesToInsert);

        QCOMPARE(m_sel->count(), 0);

        foreach (QString addr, addressesToInsert) {
            m_sel->insert(QBluetoothRemoteDevice(QBluetoothAddress(addr)));
            QVERIFY(m_sel->contains(QBluetoothAddress(addr)));
        }

        QCOMPARE(m_sel->count(), addressesToInsert.toSet().size());
        m_sel->clear();
        QCOMPARE(m_sel->count(), 0);

        foreach (QString addr, addressesToInsert) {
            QVERIFY(!m_sel->contains(QBluetoothAddress(addr)));
        }
    }

    void testSelectDeviceAndSelectionChanged_data()
    {
        QTest::addColumn<QStringList>("addressesToInsert");
        QTest::addColumn<QBluetoothAddress>("addressToSelect");
        QTest::addColumn<QBluetoothAddress>("selectedAddress");

        QTest::newRow("select when empty")
                << QStringList()
                << QBluetoothAddress("aa:aa:aa:aa:aa:aa") << QBluetoothAddress();

        QTest::newRow("insert one, select non-existent")
                << QStringList("aa:aa:aa:aa:aa:aa")
                << QBluetoothAddress("bb:bb:bb:bb:bb:bb") << QBluetoothAddress();

        QTest::newRow("three addresses, select first")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc")
                << QBluetoothAddress("aa:aa:aa:aa:aa:aa") << QBluetoothAddress("aa:aa:aa:aa:aa:aa");

        QTest::newRow("three addresses, select second")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc")
                << QBluetoothAddress("bb:bb:bb:bb:bb:bb") << QBluetoothAddress("bb:bb:bb:bb:bb:bb");

        QTest::newRow("three addresses, select last")
                << (QStringList() << "aa:aa:aa:aa:aa:aa" << "bb:bb:bb:bb:bb:bb" << "cc:cc:cc:cc:cc:cc")
                << QBluetoothAddress("cc:cc:cc:cc:cc:cc") << QBluetoothAddress("cc:cc:cc:cc:cc:cc");
    }

    void testSelectDeviceAndSelectionChanged()
    {
        QFETCH(QStringList, addressesToInsert);
        QFETCH(QBluetoothAddress, addressToSelect);
        QFETCH(QBluetoothAddress, selectedAddress);

        QCOMPARE(m_sel->selectedDevice(), QBluetoothAddress());
        foreach (QString addr, addressesToInsert) {
            m_sel->insert(QBluetoothRemoteDevice(QBluetoothAddress(addr)));
        }

        QCOMPARE(m_sel->selectedDevice(), QBluetoothAddress());

        QSignalSpy spy(m_sel,SIGNAL(selectionChanged()));
        m_sel->selectDevice(addressToSelect);
        if (selectedAddress != QBluetoothAddress()) {
            // expect selectDevice() to work
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();
        } else {
            // verify that signal does NOT get emitted
            for (int i = 0; i < 5000; i += 50) {
                QTest::qWait(50);
                QVERIFY( !spy.count() );
            }
        }

        QCOMPARE(m_sel->selectedDevice(), selectedAddress);
    }

    // test calling selectDeivce() multiple times
    void testSelectDeviceAndSelectionChanged_multiple()
    {
        QList<QBluetoothAddress> addresses;
        addresses << QBluetoothAddress("aa:aa:aa:aa:aa:aa")
                << QBluetoothAddress("bb:bb:bb:bb:bb:bb")
                << QBluetoothAddress("cc:cc:cc:cc:cc:cc");

        foreach (QBluetoothAddress addr, addresses) {
            m_sel->insert(QBluetoothRemoteDevice(addr));
        }

        foreach (QBluetoothAddress addr, addresses) {
            QBluetoothAddress previousAddr = m_sel->selectedDevice();

            QSignalSpy spy(m_sel,SIGNAL(selectionChanged()));
            m_sel->selectDevice(addr);
            QTRY_VERIFY( spy.count() == 1 ); spy.takeLast();

            QCOMPARE(m_sel->selectedDevice(), addr);
            QVERIFY(m_sel->selectedDevice() != previousAddr);
        }
    }

    void testSelectDeviceAndRemove()
    {
        QBluetoothAddress addr("aa:aa:aa:aa:aa:aa");

        QCOMPARE(m_sel->selectedDevice(), QBluetoothAddress());

        m_sel->insert(QBluetoothRemoteDevice(addr));
        m_sel->selectDevice(addr);
        QCOMPARE(m_sel->selectedDevice(), addr);

        m_sel->remove(addr);
        QCOMPARE(m_sel->selectedDevice(), QBluetoothAddress());
    }

    void testSelectDeviceAndClear()
    {
        QBluetoothAddress addr("aa:aa:aa:aa:aa:aa");

        QCOMPARE(m_sel->selectedDevice(), QBluetoothAddress());

        m_sel->insert(QBluetoothRemoteDevice(addr));
        m_sel->selectDevice(addr);
        QCOMPARE(m_sel->selectedDevice(), addr);

        m_sel->clear();
        QCOMPARE(m_sel->selectedDevice(), QBluetoothAddress());
    }
};

QTEST_MAIN(tst_QBluetoothRemoteDeviceSelector)
#include "tst_qbluetoothremotedeviceselector.moc"
