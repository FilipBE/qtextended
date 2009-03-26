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

#include <QSpeedDial>

#include <QtopiaApplication>
#include <shared/qtopiaunittest.h>



//TESTED_CLASS=QSpeedDial
//TESTED_FILES=src/libraries/qtopia/qspeeddial.h

/*
    The tst_QSpeedDial class provides unit tests for the QSpeedDial class.
*/
class tst_QSpeedDial : public QObject
{
Q_OBJECT
private slots:
    void init();
    void initTestCase();

    void coreTest();
    void possibleTest();
};

QTEST_APP_MAIN( tst_QSpeedDial, QtopiaApplication )
#include "tst_qspeeddial.moc"

/*!
    initTestCase() function called once before all tests.
    brings the table into existence, and checks it's empty.
*/
void tst_QSpeedDial::initTestCase()
{
        //foreach(QString str, QSpeedDial::assignedInputs())
        //        qDebug() << str << "Is assigned to" << QSpeedDial::find(str)->label();
        //It starts with the 1: Call VoiceMail which is default.
        QSpeedDial::remove("1");
        QCOMPARE(QSpeedDial::assignedInputs(),QList<QString>());
}

/*!
    init() function called before each testcase.
    empties the table
*/
void tst_QSpeedDial::init()
{
        QList<QString> in = QSpeedDial::assignedInputs();
        foreach(QString str,in)
                QSpeedDial::remove(str);

        QCOMPARE(QSpeedDial::assignedInputs(),QList<QString>());
}

/*!
  Tests the QSpeedDial core access functions,
  set, remove and find.
*/
void tst_QSpeedDial::coreTest()
{
        const int numDescs=5;
        QtopiaServiceDescription test[numDescs];
        for(int i = 0; i<numDescs; i++){
                test[i] = QtopiaServiceDescription(QtopiaServiceRequest(QString("test%1").arg(i),
                                QString("test%1").arg(i)),QString("test%1").arg(i),
                                QString("test%1").arg(i));
        }

        QtopiaServiceDescription *found;
        for(int i = 1; i<100; i++){
                found = QSpeedDial::find(QString::number(i));
                QVERIFY(!found);
        }
        for(int i = 1; i<100; i++){
                QSpeedDial::set(QString::number(i),test[i%numDescs]);
                found = QSpeedDial::find(QString::number(i));
                QVERIFY(found);
                QCOMPARE(*found, test[i%numDescs]);
        }
        //Can only have one speed dial per description
        for(int i = 1; i<=numDescs; i++){
                found = QSpeedDial::find(QString::number(i));
                QVERIFY(!found);
        }
        for(int i = 100-numDescs; i<100; i++){
                found = QSpeedDial::find(QString::number(i));
                QVERIFY2(found, qPrintable(QString(
                    "Couldn't find %1 in speed dial!  assignedInputs returns %2"
                    ).arg(QString::number(i)).arg(QStringList(QSpeedDial::assignedInputs()).join(","))
                ));
                QCOMPARE(*found, test[i%numDescs]);
        }

        for(int i = 1; i<100; i++){
            if(i!=(100 - numDescs)){
                QSpeedDial::remove(QString::number(i));
                found = QSpeedDial::find(QString::number(i));
                QVERIFY(!found);
            }
        }
        found=QSpeedDial::find(QString::number(100 - numDescs));
        QVERIFY(found);
        QCOMPARE(*found,test[0]);
        QSpeedDial::remove(QString::number(100 - numDescs));
        found=QSpeedDial::find("1");
        QVERIFY(!found);
}

/*!
  Tests that the possible inputs are possible inputs.
  Also tests that assigned inputs are possible.
  Doesn't test that these are the only possible inputs,
  behaviour with other inputs is unspecified (probably
  will fail in some manner).
*/
void tst_QSpeedDial::possibleTest()
{
        QtopiaServiceDescription test(QtopiaServiceRequest("test","test"),"test","test");
        QtopiaServiceDescription *found;
        //test possible are possible
        QList<QString> possible = QSpeedDial::possibleInputs();
        foreach(QString str,possible){
                QSpeedDial::set(str,test);
                found = QSpeedDial::find(str);
                QVERIFY(found);
                QCOMPARE(*found,test);
        }

        //test assigned are possible
        QList<QString> assigned = QSpeedDial::assignedInputs();
        foreach(QString str,assigned)
                QVERIFY(possible.contains(str));
}

