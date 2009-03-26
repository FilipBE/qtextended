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

#include <QtopiaServiceHistoryModel>

#include <QtopiaApplication>
#include <QTest>
#include <shared/qtopiaunittest.h>



//TESTED_CLASS=QtopiaServiceHistoryModel
//TESTED_FILES=src/libraries/qtopia/qtopiaservicehistorymodel.h

/*
    The tst_QtopiaServiceHistoryModel class provides unit tests for the QtopiaServiceHistoryModel class.
*/
class tst_QtopiaServiceHistoryModel : public QObject
{
Q_OBJECT
private slots:
    void allTest();
};

QTEST_APP_MAIN( tst_QtopiaServiceHistoryModel, QtopiaApplication )
#include "tst_qtopiaservicehistorymodel.moc"

/*!
  Not much to test, all intertwined and no clearing ablity in the API,
  so just done as one test function.
*/
void tst_QtopiaServiceHistoryModel::allTest()
{
	QtopiaServiceHistoryModel model;
	QVERIFY(!model.rowCount());
	QtopiaServiceHistoryModel::insert(QtopiaServiceRequest("testA","test"),"test0","test0");
	QtopiaServiceHistoryModel::insert(QtopiaServiceRequest("testB","test"),"test1","test1");
	QtopiaServiceHistoryModel::insert(QtopiaServiceRequest("test1","test"),"test2","test2");
	QtopiaServiceHistoryModel::insert(QtopiaServiceRequest("test1","test"),"test2","test2");
	QtopiaServiceHistoryModel::insert(QtopiaServiceRequest("testD","test"),"test3","test3");
	QtopiaServiceHistoryModel::insert(QtopiaServiceRequest("testE","test"),"test4","test4");
	QTest::qWait(100);
	QCOMPARE(model.rowCount(),6);
	model.setSorting(QtopiaServiceHistoryModel::History);
	QCOMPARE(model.sorting(),QtopiaServiceHistoryModel::History);
	QCOMPARE(model.data(model.index(3,0,QModelIndex()),Qt::DisplayRole).toString(),QString("test2"));
	QCOMPARE(model.serviceRequest(model.index(3,0,QModelIndex())).service(),QString("test1"));
	QCOMPARE(model.serviceDescription(model.index(3,0,QModelIndex())).label(),QString("test2"));
	model.setSorting(QtopiaServiceHistoryModel::Recent);
	QCOMPARE(model.sorting(),QtopiaServiceHistoryModel::Recent);
	QTest::qWait(100);
	QCOMPARE(model.rowCount(),5);
	QCOMPARE(model.data(model.index(3,0,QModelIndex()),Qt::DisplayRole).toString(),QString("test1"));
	model.setSorting(QtopiaServiceHistoryModel::Frequency);
	QCOMPARE(model.sorting(),QtopiaServiceHistoryModel::Frequency);
	QTest::qWait(100);
	QCOMPARE(model.rowCount(),5);
	QCOMPARE(model.data(model.index(0,0,QModelIndex()),Qt::DisplayRole).toString(),QString("test2"));
}	
