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

#include "generator.h"
#include "cgen.h"

#include <QDebug>
#include <QContactModel>
#include <QAppointmentModel>
#include <QTaskModel>
#include <QtopiaIpcEnvelope>

#include <QtopiaApplication>
#include <QByteArray>

#include "qpimxml_p.h"

#include <stdlib.h>
#include <time.h>

using namespace PIMXML_NAMESPACE;

ContactGeneratorWidget::ContactGeneratorWidget(QWidget *parent, Qt::WFlags f) 
    : QWidget(parent, f)
{
    // seed the random number generator.
    srand(uint(time(0)));
    connect(qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(processRequest(QString,QByteArray)));
}

ContactGeneratorWidget::~ContactGeneratorWidget()
{
}

void ContactGeneratorWidget::processRequest(const QString &msg, const QByteArray &data)
{
    int contactsRemaining, appointmentsRemaining, tasksRemaining;

    if (msg == "generateData(int,int,int)") {
        QDataStream stream( data );

        stream >> contactsRemaining;
        stream >> appointmentsRemaining;
        stream >> tasksRemaining;

        generateData(contactsRemaining, appointmentsRemaining, tasksRemaining);
    } else if (msg == "exportRecords(QString,QString)") {
        QString type, file;
        QDataStream stream( data );

        stream >> type >> file;
        exportRecords(type, file);
    } else if (msg == "importRecords(QString,QString)") {
        QString type, file;
        QDataStream stream( data );

        stream >> type >> file;
        importRecords(type, file);
    } else if (msg == "compareRecords(QString,QString)") {
        QString type, file;
        QDataStream stream( data );

        stream >> type >> file;
        compareRecords(type, file);
    } else if (msg == "clearRecords(QString)") {
        QString type;
        QDataStream stream( data );

        stream >> type;
        clearRecords(type);
    }
}

void ContactGeneratorWidget::generateData(int contactsRemaining, int appointmentsRemaining, int tasksRemaining)
{
    if (contactsRemaining <= 0 && appointmentsRemaining <= 0 &&
            tasksRemaining <= 0)
        return;

    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    if (contactsRemaining > 0) {
        QContactModel model;

        if (model.startSyncTransaction(syncTime)) {
            ContactGenerator gen;
            for (int i = 0; i < contactsRemaining; i++) {
                QContact c = gen.nextContact();
                if (model.addContact(c).isNull()) {
                    model.abortSyncTransaction();
                    sendFail();
                    return;
                } else {
                    sendProgress(i);
                }
            }
        }
        if (!model.commitSyncTransaction()) {
            model.abortSyncTransaction();
            sendFail();
            return;
        }
    }

    if (appointmentsRemaining > 0) {
        QAppointmentModel model;

        if (model.startSyncTransaction(syncTime)) {
            AppointmentGenerator gen;
            for (int i = 0; i < appointmentsRemaining; i++) {
                if (model.addAppointment(gen.nextAppointment()).isNull()) {
                    model.abortSyncTransaction();
                    sendFail();
                    return;
                } else {
                    sendProgress(contactsRemaining + i);
                }
            }
        }
        if (!model.commitSyncTransaction()) {
            model.abortSyncTransaction();
            sendFail();
            return;
        }
    }

    if (tasksRemaining > 0) {
        QTaskModel model;

        if (model.startSyncTransaction(syncTime)) {
            TaskGenerator gen;
            for (int i = 0; i < tasksRemaining; i++) {
                if (model.addTask(gen.nextTask()).isNull()) {
                    model.abortSyncTransaction();
                    sendFail();
                    return;
                } else {
                    sendProgress(contactsRemaining + appointmentsRemaining + i);
                }
            }
        }
        if (!model.commitSyncTransaction()) {
            model.abortSyncTransaction();
            sendFail();
            return;
        }
    }
    sendCompleted();
}

void ContactGeneratorWidget::sendProgress(int count)
{
    {
        QtopiaIpcEnvelope e("QPE/Tools/PimDataGenerator", "recordsProcessed(int)");
        e << count;
    }
    qApp->processEvents();
}

void ContactGeneratorWidget::sendFail()
{
    {
        QtopiaIpcEnvelope e("QPE/Tools/PimDataGenerator", "failed()");
    }
    qApp->processEvents();
}

void ContactGeneratorWidget::sendCompleted()
{
    {
        QtopiaIpcEnvelope e("QPE/Tools/PimDataGenerator", "completed()");
    }
    qApp->processEvents();
}

static QList<QPimXmlException> pg_convertExceptions(const QAppointmentModel &model, const QList<QAppointment::Exception> origList)
{
    QList<QPimXmlException> newList;
    foreach(QAppointment::Exception e, origList)
    {
        QPimXmlException n;
        n.originalDate = e.date;
        n.replacement = !e.alternative.isNull();
        if (n.replacement)
            n.appointment = model.appointment(e.alternative);
        newList.append(n);
    }
    return newList;
}

void ContactGeneratorWidget::exportRecords(const QString &type, const QString &filename)
{
    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    if (type == "contacts") {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly))
            return;

        QPimXmlStreamWriter h(&file);
        h.writeStartDocument();
        h.writeStartContactList();

        QContactModel m;

        m.startSyncTransaction(syncTime);
        for(int i = 0; i < m.count(); i++)
            h.writeContact(m.contact(i));
        m.commitSyncTransaction();

        h.writeEndDocument();

        file.close();
    } else if (type == "tasks") {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly))
            return;

        QPimXmlStreamWriter h(&file);
        h.writeStartDocument();
        h.writeStartTaskList();

        QTaskModel m;

        m.startSyncTransaction(syncTime);
        for(int i = 0; i < m.count(); i++)
            h.writeTask(m.task(i));
        m.commitSyncTransaction();

        h.writeEndDocument();

        file.close();
    } else if (type == "calendar") {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly))
            return;

        QPimXmlStreamWriter h(&file);
        h.writeStartDocument();
        h.writeStartAppointmentList();

        QAppointmentModel m;
        m.startSyncTransaction(syncTime);
        for(int i = 0; i < m.count(); i++) {
            QAppointment c = m.appointment(i);
            if (c.isException())
                continue;

            h.writeAppointment(c, pg_convertExceptions(m, c.exceptions()));
        }
        m.commitSyncTransaction();

        h.writeEndDocument();

        file.close();
    }
}

void ContactGeneratorWidget::importRecords(const QString &type, const QString &filename)
{
    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    QString sid;

    if (type == "contacts") {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            return;

        QPimXmlStreamReader h(&file);
        h.readStartContactList();

        QContactModel m;
        QContact c;

        c = h.readContact(sid);
        m.startSyncTransaction(syncTime);
        while(!h.hasError()) {
            m.addContact(c);
            c = h.readContact(sid);
        }
        m.commitSyncTransaction();
        file.close();
    } else if (type == "tasks") {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            return;

        QPimXmlStreamReader h(&file);
        h.readStartTaskList();

        QTaskModel m;
        QTask c;
        c = h.readTask(sid);
        m.startSyncTransaction(syncTime);
        while(!h.hasError()) {
            m.addTask(c);
            c = h.readTask(sid);
        }
        m.commitSyncTransaction();
        file.close();
    } else if (type == "calendar") {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            return;

        QPimXmlStreamReader h(&file);
        h.readStartAppointmentList();

        QAppointmentModel m;
        QAppointment c;
        QList<QPimXmlException> list;
        c = h.readAppointment(sid, list);
        m.startSyncTransaction(syncTime);
        while(!h.hasError()) {
            c.setUid(m.addAppointment(c));
            foreach(QPimXmlException e, list) {
                if (e.replacement) {
                    QOccurrence o(e.originalDate, e.appointment);
                    m.replaceOccurrence(c, o);
                } else {
                    m.removeOccurrence(c, e.originalDate);
                }
            }
            c = h.readAppointment(sid, list);
        }
        m.commitSyncTransaction();
        file.close();
    } else if (type.startsWith("calendar:Qtopia-2.")) {
        qDebug() << "importing Qtopia 2.x datebook format";
        // and later vcard/vcal imports/exports
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            return;

        qDebug() << "found file";

        // the old format was simply a set of attributes in what
        // was essentially a flat table.
        // not reading replacement exceptions as such.
        QPimXmlStreamReader h(&file);
        QAppointmentModel m;
        m.startSyncTransaction(syncTime);
        if (h.readStartElement("events")) {
            qDebug() << "correct format";

            //QMap<int, QUniqueId> idMap;
            //QMap<int, QList< QPair<int, QDate> > exceptionMap;

            qDebug() << "Now on" << int(h.tokenType()) << h.name().toString();
            h.readNext();
            qDebug() << "Now on" << int(h.tokenType()) << h.name().toString();

            while(h.readStartElement("event")) {
                QAppointment a;
                QXmlStreamAttributes attr = h.attributes();

                int origid = attr.value("uid").toString().toInt();

                a.setDescription(attr.value("description").toString());
                qDebug() << "Import record" << origid << a.description();

                a.setLocation(attr.value("location").toString());
                a.setNotes(attr.value("note").toString());

                QTimeZone tz;
                if (!attr.value("timezone").isEmpty()) {
                    tz = QTimeZone(attr.value("timezone").toString().toLatin1().constData());
                    if (tz.isValid())
                        a.setTimeZone(tz);
                    else
                        tz = QTimeZone::current();
                } else {
                    tz = QTimeZone::current();
                }

                // *start and end.
                uint start_tsec = attr.value("start").toString().toUInt();
                uint end_tsec = attr.value("end").toString().toUInt();

                QDateTime start = tz.fromTime_t(start_tsec);
                QDateTime end = tz.fromTime_t(end_tsec);

                a.setAllDay(attr.value("type") == "AllDay");
                a.setStart(start);
                a.setEnd(end);

                if (!attr.value("alarm").isEmpty()) {
                    int delay = attr.value("alarm").toString().toInt();
                    if (attr.value("sound") == "silent") {
                        a.setAlarm(delay, QAppointment::Visible);
                    } else {
                        a.setAlarm(delay, QAppointment::Audible);
                    }
                }


                QList<QDate> exceptions;
                if (!attr.value("rtype").isEmpty()) {
                    QStringRef rtype = attr.value("rtype");
                    int rweek = attr.value("rweekdays").toString().toInt();
                    if (rtype == "Daily") {
                        a.setRepeatRule(QAppointment::Daily);
                    } else if (rtype == "Weekly") {
                        a.setRepeatRule( QAppointment::Weekly);
                        a.setWeekFlags(QAppointment::WeekFlags(rweek));
                    } else if (rtype == "MonthlyDay") {
                        a.setRepeatRule( QAppointment::MonthlyDay);
                    } else if (rtype == "MonthlyDate") {
                        a.setRepeatRule( QAppointment::MonthlyDate);
                    } else if (rtype == "MonthlyEndDay") {
                        a.setRepeatRule( QAppointment::MonthlyEndDay);
                    } else if (rtype == "Yearly") {
                        a.setRepeatRule( QAppointment::Yearly);
                    }

                    if (!attr.value("rfreq").isEmpty())
                        a.setFrequency( attr.value("rfreq").toString().toInt());

                    if (!attr.value("enddt").isEmpty()) {
                        uint enddate_tsec = attr.value("enddt").toString().toUInt();
                        a.setRepeatUntil(tz.fromTime_t(enddate_tsec).date());
                    }

                    // space seperated, yyyyMMdd
                    QStringList estring = attr.value("exceptions").toString().split(' ');
                    foreach(QString e, estring)
                        exceptions.append(QDate::fromString(e, "yyyyMMdd"));

                }
                h.readEndElement();
                QUniqueId id = m.addAppointment(a);
                foreach(QDate exception, exceptions) {
                    m.removeOccurrence(id, exception);
                }
                // not handling custom or categories.
            }
        }
        qDebug() << "End on" << int(h.tokenType()) << h.name().toString();
        m.commitSyncTransaction();
        file.close();
    }
}


// a big assumption is made.  That the order of the xml is the same as the contacts.
// this means if you have records that have the same sort key but different data, you may
// get false negatives from this function.
void ContactGeneratorWidget::compareRecords(const QString &type, const QString &filename, bool compareId)
{
    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    QString sid;

    if (type == "contacts") {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            return;

        QPimXmlStreamReader h(&file);
        h.readStartContactList();

        QContactModel m;
        QContact c;

        c = h.readContact(sid);
        m.startSyncTransaction(syncTime);
        int i = 0;
        while(!h.hasError() && i < m.count()) {
            QContact candidate = m.contact(i);
            if (!compareId)
                c.setUid(candidate.uid());
            if (c != candidate)  {
                qWarning() << "Contact list does not match";
                break;
            }
            c = h.readContact(sid);
            ++i;
        }
        if (i < m.count() || !h.hasError())
            qWarning() << "Contact list length does not match";
        else
            qWarning() << "i" << i << "count" << m.count() << "hasError" << h.hasError();
        m.commitSyncTransaction();
        file.close();
    } else if (type == "tasks") {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            return;

        QPimXmlStreamReader h(&file);
        h.readStartTaskList();

        QTaskModel m;
        QTask c;
        c = h.readTask(sid);
        m.startSyncTransaction(syncTime);
        int i = 0;
        while(!h.hasError() && i < m.count()) {
            QTask candidate = m.task(i);
            if (!compareId)
                c.setUid(candidate.uid());
            if (c != candidate)  {
                qWarning() << "Task list does not match";
                break;
            }
            c = h.readTask(sid);
            ++i;
        }
        if (i < m.count() || !h.hasError())
            qWarning() << "Task list length does not match";
        m.commitSyncTransaction();
        file.close();
    } else if (type == "calendar") {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            return;

        QPimXmlStreamReader h(&file);
        h.readStartAppointmentList();

        QAppointmentModel m;
        QAppointment c;
        QList<QPimXmlException> list;
        c = h.readAppointment(sid, list);
        m.startSyncTransaction(syncTime);
        int i = 0;
        // doesn't compare exceptions;
        while(!h.hasError() && i < m.count()) {
            QAppointment candidate = m.appointment(i);
            if (!candidate.isException()) {
                if (!compareId)
                    c.setUid(candidate.uid());
                if (c != candidate)  {
                    qWarning() << "Appointment list does not match";
                    break;
                }
                c = h.readAppointment(sid, list);
            }
            ++i;
        }
        if (i < m.count() || !h.hasError())
            qWarning() << "Appointment list length does not match";
        m.commitSyncTransaction();
        file.close();
    }
}


void ContactGeneratorWidget::clearRecords(const QString &type)
{
    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    // hard and fast?
    if (type == "contacts") {
        QContactModel m;
        m.startSyncTransaction(syncTime);
        while(m.count())
            m.removeContact(m.id(0));
        m.commitSyncTransaction();
    } else if (type == "tasks") {
        QTaskModel m;
        m.startSyncTransaction(syncTime);
        while(m.count())
            m.removeTask(m.id(0));
        m.commitSyncTransaction();
    } else if (type == "calendar") {
        QAppointmentModel m;
        m.startSyncTransaction(syncTime);
        while(m.count())
            m.removeAppointment(m.id(0));
        m.commitSyncTransaction();
    }
}

