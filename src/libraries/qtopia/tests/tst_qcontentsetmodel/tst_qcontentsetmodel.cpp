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


#include <shared/qtopiaunittest.h>
#include <QContentSetModel>
#include <QContentSet>
#include <QContent>
#include <QSignalSpy>
#include <QtopiaApplication>
#include <shared/util.h>
#include <QTest>
#include <QtopiaSql>

//TESTED_CLASS=QContentSetModel
//TESTED_FILES=src/libraries/qtopia/qcontentsetmodel.cpp

class tst_QContentSetModel: public QObject
{
    Q_OBJECT
private slots:
    void updateInProgress();

protected slots:
    void initTestCase();

public slots:
    void onUpdateStarted();
    void onUpdateFinished();

private:
    bool m_updateInProgress_started;
    bool m_updateInProgress_finished;
};

QTEST_APP_MAIN( tst_QContentSetModel, QtopiaApplication )

void tst_QContentSetModel::initTestCase()
{
    QtopiaSql::instance()->openDatabase();
}

/*
    Tests that updateInProgress() is correctly set in three cases:
        1.  Before updateStarted() is emitted.
        2.  Immediately after updateStarted() is emitted and before
            updateFinished() is emitted.
        3.  Immediately after updateFinished() is emitted.
*/
void tst_QContentSetModel::updateInProgress()
{
    QContentSet contentAsync(QContentSet::Asynchronous);
    QContentSetModel model(&contentAsync);

    QObject::connect(&model, SIGNAL(updateStarted()),
                     this,   SLOT(onUpdateStarted()),
                     Qt::DirectConnection);
    QObject::connect(&model, SIGNAL(updateFinished()),
                     this,   SLOT(onUpdateFinished()),
                     Qt::DirectConnection);

    // Set these to the opposite values we expect.
    m_updateInProgress_started  = false;
    m_updateInProgress_finished = true;

    // Verify the original value.
    QVERIFY( !model.updateInProgress() );

    // Start update and wait until it finishes.
    contentAsync.setCriteria(QContent::Application);
    QSignalSpy spy(&model, SIGNAL(updateFinished()));
    QTRY_VERIFY(spy.count());

    // We are done...
    QVERIFY( !model.updateInProgress() );

    // Now check the two values set immediately after the signals were emitted.
    QVERIFY( m_updateInProgress_started );
    QVERIFY( !m_updateInProgress_finished );
}

void tst_QContentSetModel::onUpdateStarted()
{
    QContentSetModel* model = qobject_cast<QContentSetModel*>(sender());
    Q_ASSERT(model);
    m_updateInProgress_started = model->updateInProgress();
}

void tst_QContentSetModel::onUpdateFinished()
{
    QContentSetModel* model = qobject_cast<QContentSetModel*>(sender());
    Q_ASSERT(model);
    m_updateInProgress_finished = model->updateInProgress();
}

#include "tst_qcontentsetmodel.moc"

