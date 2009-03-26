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

#include <QTaskModel>
#include <QTask>

#include <QtopiaApplication>
#include <QtopiaSql>
#include <QDebug>
#include <QTaskModel>
#include <QAppointmentModel>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QValueSpace>

//TESTED_CLASS=QTaskModel
//TESTED_FILES=src/libraries/qtopiapim/qtaskmodel.h

/*
    The tst_QTaskModel class provides unit tests for the QTaskModel class.
*/
class tst_QTaskModel : public QObject
{
Q_OBJECT
protected slots:
    void cleanup();
    void initTestCase()
    { QValueSpace::initValuespaceManager(); }

private slots:
    void addNotify();
    void taskField();
    void checkDependentEvents();
    void addTask();
};

QTEST_APP_MAIN( tst_QTaskModel, QtopiaApplication )
#include "tst_qtaskmodel.moc"


/*?
    Test notification.
    When changes are made to one model changes should be reflected in other models both within the same process and
    in other processes.  This is an asynchronous update, however within process it should occur within two loops of
    the event loop and less than 100ms between processes.
*/
void tst_QTaskModel::addNotify()
{
    QTaskModel *source = new QTaskModel;
    QTaskModel *destination = new QTaskModel;

    /* ensure initial state */
    QVERIFY(source->count() == 0);
    QVERIFY(destination->count() == 0);

    QTask task;
    task.setDescription("Test PIM Library");
    task.setPriority(1);
    task.setDueDate(QDate(2009, 1, 1));

    task.setUid(source->addTask(task));

    QVERIFY(source->count() == 1);
    QTask stored = source->task(0);
    QVERIFY(source->task(0) == task);

    // caching and bunching of updates can mean have to go through event loop twice.
    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 1);
    QVERIFY(destination->task(0) == task);
}

void tst_QTaskModel::cleanup()
{
    QTaskModel *model = new QTaskModel;
    while(model->count())
        model->removeTask(model->task(0));
}

/* Set the basic contents of QTask C to unique strings based on S. */
/* Doesn't manipulate percent, or status (since they are partially based
   on start/end dates */
#define SET_ALL(C,S) \
    C.setPriority( 1 + (QString(S).size() % 5) );\
    C.setDescription( "description" S );\
    C.setStartedDate( QDate(1901,1,1).addDays(QString(S).size()));\
    C.setCompletedDate( QDate(1902,1,1).addDays(QString(S).size()));\
    C.setDueDate( QDate(1903,1,1).addDays(QString(S).size()));\
    C.setNotes("notes " S);


/* Verify the contents of QTask C are equal to those set by SET_ALL(). */
#define VERIFY_ALL(C,S) \
    QCOMPARE(C.priority(), QTask::Priority(1 + (QString(S).size() % 5)) );\
    QCOMPARE(C.description(), QString( "description" S ));\
    QCOMPARE(C.startedDate(),  QDate(1901,1,1).addDays(QString(S).size()));\
    QCOMPARE(C.completedDate(), QDate(1902,1,1).addDays(QString(S).size()));\
    QCOMPARE(C.dueDate(), QDate(1903,1,1).addDays(QString(S).size()));\
    QCOMPARE(C.notes(), QString("notes " S));

/*?
  Add a task to the database, retrieve it again,
  and make sure the retrieved copy is the same as
  the original task.
*/
void tst_QTaskModel::addTask()
{
    QTaskModel model;

    QTask bob;
    QList<QString> cats;
    cats << "Business";
    SET_ALL(bob, "bob");
    bob.setCustomField("custom1", "custom1value");
    bob.setCustomField("custom2", "custom2value");
    bob.setCategories(cats);
    QUniqueId id = model.addTask(bob);
    bob.setUid(id);

    QTask bobstwin = model.task(id);

    VERIFY_ALL(bobstwin, "bob");
    QCOMPARE(bob, bobstwin);
    QCOMPARE(bobstwin.customField("custom1"), QString("custom1value"));
    QCOMPARE(bobstwin.customField("custom2"), QString("custom2value"));
    QCOMPARE(bobstwin.categories(), cats);
}


/*?
  Add a task with a due date to the database,
  and make sure that a corresponding appointment has been
  created.
*/

void tst_QTaskModel::checkDependentEvents()
{
    QTaskModel model;

    QTask garbage;
    garbage.setDescription("Take out the garbage");
    garbage.setDueDate(QDate(2007,7,11));

    garbage.setUid(model.addTask(garbage));

    QAppointmentModel amodel;
    QAppointment due = amodel.appointment(garbage.dependentChildrenOfType("duedate").value(0));

    QVERIFY2(!due.uid().isNull(), "Dependent event does not exist");

    QCOMPARE(due.start().date(), garbage.dueDate());

    // This may change..
    QCOMPARE(due.description(), garbage.description());
}

/*?
  Make sure that using QTaskModel::taskField(task, field) returns the
  same as using QTaskModel::data(index(row, field))
*/
void tst_QTaskModel::taskField()
{
    QTaskModel model;

    QTask garbage;

    SET_ALL(garbage, "garbage");
    garbage.setCategories(QStringList() << "Business"); // no tr

    QUniqueId id = model.addTask(garbage);
    garbage.setUid(id);
    QModelIndex index = model.index(id);

    QList<QTaskModel::Field> fields;

    fields << QTaskModel::Description;
    fields << QTaskModel::Priority;
    fields << QTaskModel::Completed;
    fields << QTaskModel::PercentCompleted;
    fields << QTaskModel::Status;
    fields << QTaskModel::DueDate;
    fields << QTaskModel::StartedDate;
    fields << QTaskModel::CompletedDate;
    fields << QTaskModel::Notes;
    fields << QTaskModel::Identifier;
    fields << QTaskModel::Categories;

    /* We know these don't match: (taskField can't retrieve them without
       an instance of QTaskModel or QAppointmentModel */

    //fields << QTaskModel::Alarm;
    //fields << QTaskModel::RepeatRule;
    //fields << QTaskModel::RepeatEndDate;
    //fields << QTaskModel::RepeatFrequency;
    //fields << QTaskModel::RepeatWeekFlags;

    foreach(QTaskModel::Field f, fields) {
        QVariant taskField = QTaskModel::taskField(garbage, f);
        QVariant modelData = model.data(model.index(index.row(), f), Qt::DisplayRole);
        if (taskField != modelData) {
            QString msg;
            QDebug msgd(&msg);
            msgd << "field " << QTaskModel::fieldLabel(f) << "didn't match: taskField() " << taskField << "!= model.data()" << modelData;
            QByteArray ba = msg.toLatin1();
            QFAIL(ba.constData());
        }
    }
}

