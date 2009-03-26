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

#include <QAppointmentModel>
#include <QAppointment>

#include <QtopiaApplication>
#include <QValueSpace>
#include <QDebug>
#include <QDateTime>
#include <QTest>
#include <shared/qtopiaunittest.h>



//TESTED_CLASS=QAppointmentModel
//TESTED_FILES=src/libraries/qtopiapim/qappointmentmodel.h

/*
    The tst_QAppointmentModel class provides unit tests for the QAppointmentModel class.
*/
class tst_QAppointmentModel : public QObject
{
Q_OBJECT
protected slots:
    void cleanup();
    void initTestCase()
    { QValueSpace::initValuespaceManager(); }
private slots:
    void appointmentField();
    void addAppointment();
    void addNotify();
    void updateNotify();
    void replaceOccurrence();
    void replaceOccurrenceNotify();
    void removeOccurrence();
    void removeOccurrenceNotify();
};

QTEST_APP_MAIN( tst_QAppointmentModel, QtopiaApplication )
#include "tst_qappointmentmodel.moc"

/*?
    Test QAppointmentModel change notification.
    When changes are made to one model changes should be reflected in other models both within the same process and
    in other processes.  This is an asynchronous update, however within process it should occur within two loops of
    the event loop and less than 100ms between processes.
*/
void tst_QAppointmentModel::addNotify()
{
    QAppointmentModel *source = new QAppointmentModel;
    QAppointmentModel *destination = new QAppointmentModel;

    /* ensure initial state */
    QVERIFY(source->count() == 0);
    QVERIFY(destination->count() == 0);

    QAppointment appointment;
    appointment.setDescription("Run Unit Tests");
    appointment.setStart(QDateTime(QDate(2008, 12, 31), QTime(23, 59, 59)));
    appointment.setEnd(QDateTime(QDate(2009, 1, 1), QTime(0, 0, 0)));

    appointment.setUid(source->addAppointment(appointment));

    QVERIFY(source->count() == 1);
    QAppointment stored = source->appointment(0);
    QVERIFY(source->appointment(0) == appointment);

    // caching and bunching of updates can mean have to go through event loop twice.
    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 1);
    QVERIFY(destination->appointment(0) == appointment);
}

/*?
    Test QAppointmentModel change notification.
    When changes are made to one model changes should be reflected in other models both within the same process and
    in other processes.  This is an asynchronous update, however within process it should occur within two loops of
    the event loop and less than 100ms between processes.
*/
void tst_QAppointmentModel::updateNotify()
{
    QAppointmentModel *source = new QAppointmentModel;
    QAppointmentModel *destination = new QAppointmentModel;

    /* ensure initial state */
    QVERIFY(source->count() == 0);
    QVERIFY(destination->count() == 0);

    QAppointment appointment;
    appointment.setDescription("Run Unit Tests");
    appointment.setStart(QDateTime(QDate(2008, 12, 31), QTime(23, 59, 59)));
    appointment.setEnd(QDateTime(QDate(2009, 1, 1), QTime(0, 0, 0)));

    appointment.setUid(source->addAppointment(appointment));

    QVERIFY(source->count() == 1);
    QAppointment stored = source->appointment(0);
    QVERIFY(source->appointment(0) == appointment);

    // caching and bunching of updates can mean have to go through event loop twice.
    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 1);
    QVERIFY(destination->appointment(0) == appointment);


    // Now do an update and check notification
    appointment.setStart(QDateTime(QDate(2008,12,31), QTime(23,58,59)));
    source->updateAppointment(appointment);

    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 1);
    QVERIFY(destination->appointment(0) == appointment);

}

/* Set the basic contents of QAppointment C to unique strings based on S. */
/* Doesn't set repeats or alarms or timezones */
#define SET_ALL(C,S) \
    C.setDescription( "description " S );\
    C.setLocation( "location " S );\
    C.setStart( QDateTime(QDate(1900,1,1).addDays(QString(S).size()), QTime(1,2,0).addSecs(60 * QString(S).size())));\
    C.setEnd( QDateTime(QDate(1900,1,2).addDays(QString(S).size()), QTime(1,2,0).addSecs(60 * QString(S).size())));\
    C.setNotes("notes " S);


/* Verify the contents of QAppointment C are equal to those set by SET_ALL(). */
#define VERIFY_ALL(C,S) \
    QCOMPARE(C.description(), QString( "description " S ));\
    QCOMPARE(C.location(), QString("location " S ));\
    QCOMPARE(C.start(), QDateTime(QDate(1900,1,1).addDays(QString(S).size()), QTime(1,2,0).addSecs(60 * QString(S).size())));\
    QCOMPARE(C.end(), QDateTime(QDate(1900,1,2).addDays(QString(S).size()), QTime(1,2,0).addSecs(60 * QString(S).size())));\
    QCOMPARE(C.notes(), QString("notes " S));

/*?
  Add an event to the database, retrieve it again,
  and make sure the retrieved copy is the same as
  the original event.
*/
void tst_QAppointmentModel::addAppointment()
{
    QAppointmentModel model;

    QAppointment bob;
    QList<QString> cats;
    cats << "Business";
    SET_ALL(bob, "bob");
    bob.setCustomField("custom1", "custom1value");
    bob.setCustomField("custom2", "custom2value");
    bob.setCategories(cats);
    QUniqueId id = model.addAppointment(bob);
    bob.setUid(id);

    QAppointment bobstwin = model.appointment(id);

    VERIFY_ALL(bobstwin, "bob");
    QCOMPARE(bob, bobstwin);
    QCOMPARE(bobstwin.customField("custom1"), QString("custom1value"));
    QCOMPARE(bobstwin.customField("custom2"), QString("custom2value"));
    QCOMPARE(bobstwin.categories(), cats);
}

/*?
  Add an event to the database, add an exception,
  and make sure the exception is handled correctly.
*/
void tst_QAppointmentModel::replaceOccurrence()
{
    QAppointmentModel model;

    QAppointment bob;
    QList<QString> cats;
    cats << "Business";
    SET_ALL(bob, "bob");
    bob.setCustomField("custom1", "custom1value");
    bob.setCustomField("custom2", "custom2value");
    bob.setCategories(cats);
    bob.setRepeatRule(QAppointment::Daily);
    bob.setEnd(bob.start().addSecs(600));
    QUniqueId id = model.addAppointment(bob);
    bob.setUid(id);

    QAppointment exception = bob;
    QOccurrence victim = bob.nextOccurrence(bob.start().date().addDays(1));
    exception.setRepeatRule(QAppointment::NoRepeat);
    exception.setStart(victim.start().addDays(1));
    exception.setDescription("Exception");

    QUniqueId exceptionId = model.replaceOccurrence(bob, exception.firstOccurrence(), victim.start().date());
    QVERIFY(!exceptionId.isNull());

    // Fetch bob again
    bob = model.appointment(bob.uid());
    QVERIFY(bob.hasExceptions());
    QVERIFY(bob.exceptions().count() == 1);

    // Fetch exception again
    exception = model.appointment(bob.exceptions()[0].alternative);
    QVERIFY(exception.isException());
    QVERIFY(exception.exceptionDate() == victim.start().date());
    QVERIFY(exception.description() == "Exception");
    QVERIFY(exception.exceptionParent() == bob.uid());

    // Check occurrences
    QOccurrence start = bob.firstOccurrence();
    QOccurrence next1 = bob.nextOccurrence(start.end().date().addDays(1)); // (would have been victim, should be the day after)
    QOccurrence next2 = bob.nextOccurrence(next1.end().date().addDays(1));
    QOccurrence next3 = bob.nextOccurrence(next2.end().date().addDays(1));

    QVERIFY(start.start().daysTo(next1.start()) == 2); // should skip a day
    QVERIFY(next1.start().daysTo(next2.start()) == 1);
    QVERIFY(next2.start().daysTo(next3.start()) == 1);

    QVERIFY(next1.start() == exception.start());
}

/*?
    Test QAppointmentModel change notification (for replaceOccurrence)
*/
void tst_QAppointmentModel::replaceOccurrenceNotify()
{
    QAppointmentModel *source = new QAppointmentModel;
    QAppointmentModel *destination = new QAppointmentModel;

    /* ensure initial state */
    QVERIFY(source->count() == 0);
    QVERIFY(destination->count() == 0);

    QAppointment appointment;
    appointment.setDescription("Run Unit Tests");
    appointment.setStart(QDateTime(QDate(2008, 12, 31), QTime(23, 59, 59)));
    appointment.setEnd(QDateTime(QDate(2009, 1, 1), QTime(0, 0, 0)));
    appointment.setRepeatRule(QAppointment::Daily);

    appointment.setUid(source->addAppointment(appointment));

    QVERIFY(source->count() == 1);
    QAppointment stored = source->appointment(0);
    QVERIFY(source->appointment(0) == appointment);

    // caching and bunching of updates can mean have to go through event loop twice.
    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 1);
    QVERIFY(destination->appointment(0) == appointment);

    // Now do the replaceOccurrence
    QAppointment exception = appointment;
    QOccurrence victim = appointment.nextOccurrence(appointment.start().date().addDays(1));
    exception.setRepeatRule(QAppointment::NoRepeat);
    exception.setStart(victim.start().addDays(1));
    exception.setDescription("Exception");

    QUniqueId exceptionId = source->replaceOccurrence(appointment, exception.firstOccurrence(), victim.start().date());
    QVERIFY(!exceptionId.isNull());

    // now another set of updates
    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 2);
    QAppointment dest = destination->appointment(appointment.uid());
    QVERIFY(dest == appointment);
    QVERIFY(dest.hasExceptions());

    exception = destination->appointment(dest.exceptions()[0].alternative);
    QVERIFY(exception.isException());
    QVERIFY(exception.exceptionDate() == victim.start().date());
    QVERIFY(exception.description() == "Exception");
    QVERIFY(exception.exceptionParent() == appointment.uid());
}

/*?
  Add an event to the database, remove an occurrence
  and make sure the exception is handled correctly.
*/
void tst_QAppointmentModel::removeOccurrence()
{
    QAppointmentModel model;

    QAppointment bob;
    QList<QString> cats;
    cats << "Business";
    SET_ALL(bob, "bob");
    bob.setCustomField("custom1", "custom1value");
    bob.setCustomField("custom2", "custom2value");
    bob.setCategories(cats);
    bob.setRepeatRule(QAppointment::Daily);
    bob.setEnd(bob.start().addSecs(600));
    QUniqueId id = model.addAppointment(bob);
    bob.setUid(id);

    QOccurrence victim = bob.nextOccurrence(bob.end().date().addDays(1));
    model.removeOccurrence(bob, victim.start().date());

    // Fetch bob again
    bob = model.appointment(bob.uid());
    QVERIFY(bob.hasExceptions());
    QVERIFY(bob.exceptions().count() == 1);

    // Check occurrences
    QOccurrence start = bob.firstOccurrence();
    QOccurrence next1 = bob.nextOccurrence(start.end().date().addDays(1)); // (would have been victim, should be the day after)
    QOccurrence next2 = bob.nextOccurrence(next1.end().date().addDays(1));
    QOccurrence next3 = bob.nextOccurrence(next2.end().date().addDays(1));

    QVERIFY(start.start().daysTo(next1.start()) == 2); // should skip a day
    QVERIFY(next1.start().daysTo(next2.start()) == 1);
    QVERIFY(next2.start().daysTo(next3.start()) == 1);
}

/*?
    Test QAppointmentModel change notification (for removeOccurrence)
*/
void tst_QAppointmentModel::removeOccurrenceNotify()
{
    QAppointmentModel *source = new QAppointmentModel;
    QAppointmentModel *destination = new QAppointmentModel;

    /* ensure initial state */
    QVERIFY(source->count() == 0);
    QVERIFY(destination->count() == 0);

    QAppointment appointment;
    appointment.setDescription("Run Unit Tests");
    appointment.setStart(QDateTime(QDate(2008, 12, 31), QTime(23, 59, 59)));
    appointment.setEnd(QDateTime(QDate(2009, 1, 1), QTime(0, 0, 0)));
    appointment.setRepeatRule(QAppointment::Daily);

    appointment.setUid(source->addAppointment(appointment));

    QVERIFY(source->count() == 1);
    QAppointment stored = source->appointment(0);
    QVERIFY(source->appointment(0) == appointment);

    // caching and bunching of updates can mean have to go through event loop twice.
    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 1);
    QVERIFY(destination->appointment(0) == appointment);

    // Now do the removeOccurrence
    QOccurrence victim = appointment.nextOccurrence(appointment.end().date().addDays(1));
    source->removeOccurrence(appointment, victim.start().date());

    // now another set of updates
    qApp->processEvents();
    qApp->processEvents();

    // and now the update...
    QVERIFY(destination->count() == 1);
    QAppointment dest = destination->appointment(0);
    QVERIFY(dest == appointment);
    QVERIFY(dest.hasExceptions());

    // Check occurrences
    QOccurrence start = dest.firstOccurrence();
    QOccurrence next1 = dest.nextOccurrence(start.end().date().addDays(1)); // (would have been victim, should be the day after)
    QOccurrence next2 = dest.nextOccurrence(next1.end().date().addDays(1));
    QOccurrence next3 = dest.nextOccurrence(next2.end().date().addDays(1));

    QVERIFY(start.start().daysTo(next1.start()) == 2); // should skip a day
    QVERIFY(next1.start().daysTo(next2.start()) == 1);
    QVERIFY(next2.start().daysTo(next3.start()) == 1);
}

/*?
  Make sure that using QAppointmentModel::appointmentField(task, field) returns the
  same as using QAppointmentModel::data(index(row, field))
*/
void tst_QAppointmentModel::appointmentField()
{
    QAppointmentModel model;

    QList<QAppointmentModel::Field> fields;

    QAppointment garbage;
    QAppointment bday;
    QAppointment exam;
    QAppointment lecture;

    SET_ALL(garbage, "recycling");
    SET_ALL(bday,    "birthday");
    SET_ALL(lecture, "lecture");
    SET_ALL(exam,    "exam");

    lecture.setCategories(QStringList() << "Business"); // no tr

    bday.setRepeatRule(QAppointment::Yearly);

    garbage.setRepeatRule(QAppointment::Weekly);

    lecture.setRepeatRule(QAppointment::Weekly);
    lecture.setWeekFlags(QAppointment::OccurTuesday | QAppointment::OccurThursday);
    lecture.setFrequency(2);
    lecture.setRepeatUntil(lecture.start().date().addDays(140));
    lecture.setAlarm(60, QAppointment::Audible);

    exam.setAlarm(60*24*3, QAppointment::Visible);

    model.addAppointment(garbage);
    model.addAppointment(bday);
    model.addAppointment(exam);
    model.addAppointment(lecture);

    fields << QAppointmentModel::Description;
    fields << QAppointmentModel::Location;
    fields << QAppointmentModel::Start;
    fields << QAppointmentModel::End;
    fields << QAppointmentModel::AllDay;
    fields << QAppointmentModel::TimeZone;
    fields << QAppointmentModel::Notes;
    fields << QAppointmentModel::Alarm;
    fields << QAppointmentModel::RepeatRule;
    fields << QAppointmentModel::RepeatFrequency;
    fields << QAppointmentModel::RepeatEndDate;
    fields << QAppointmentModel::RepeatWeekFlags;
    fields << QAppointmentModel::Identifier;
    fields << QAppointmentModel::Categories;

    for(int i=0; i < model.rowCount(); i++) {
        QAppointment a = model.appointment(i);
        foreach(QAppointmentModel::Field f, fields) {
            QVariant appointmentField = QAppointmentModel::appointmentField(a, f);
            QVariant modelData = model.data(model.index(i, f), Qt::DisplayRole);

            if (appointmentField != modelData) {
                QString msg;
                QDebug msgd(&msg);
                msgd << "row" << i << " field " << QAppointmentModel::fieldLabel(f) << "didn't match: appointmentField() " << appointmentField << "!= model.data()" << modelData;
                QByteArray ba = msg.toLatin1();
                QFAIL(ba.constData());
            }
        }
    }
}

/*?
    Cleanup after each test function.
    Removes all appointments from a QAppointmentModel.
*/
void tst_QAppointmentModel::cleanup()
{
    QAppointmentModel *model = new QAppointmentModel;
    while(model->count())
        model->removeAppointment(model->appointment(0));
}

