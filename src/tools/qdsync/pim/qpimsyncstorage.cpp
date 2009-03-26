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
#include <QDateTime>
#include <QString>
#include <QByteArray>

#include <QContactModel>
#include <QTaskModel>
#include <QAppointmentModel>
#include <QCategoryManager>
#include <QSqlQuery>
#include <QSqlError>
#include <QtopiaSql>
#include <qtopialog.h>

#include "qpimsyncstorage.h"

QPimSyncStorageFactory::QPimSyncStorageFactory( QObject *parent )
    : Qtopia4SyncPluginFactory( parent )
{
}
QPimSyncStorageFactory::~QPimSyncStorageFactory()
{
}

QStringList QPimSyncStorageFactory::keys()
{
    return QStringList() << "calendar" << "contacts" << "tasks";
}

Qtopia4SyncPlugin *QPimSyncStorageFactory::plugin( const QString &key )
{
    if (key == "calendar") {
        return new QAppointmentSyncStorage();
    } else if (key == "contacts") {
        return new QContactSyncStorage();
    } else if (key == "tasks") {
        return new QTaskSyncStorage();
    }
    return 0;
}

QTOPIA_EXPORT_PLUGIN(QPimSyncStorageFactory)

QPimSyncStorage::QPimSyncStorage(const QString &dataset, QObject *parent)
    : Qtopia4SyncPlugin(parent), mDataset( dataset )
{
}

void QPimSyncStorage::setModel(QPimModel *model)
{
    m = model;
    // set editable sources only for model
    QSet<QPimSource> sources = m->availableSources();
    QMutableSetIterator<QPimSource> it(sources);
    while(it.hasNext()) {
        QPimSource s = it.next();
        if (!m->context(s)->editable()) 
            it.remove();
    }
    m->setVisibleSources(sources);
}

QPimSyncStorage::~QPimSyncStorage(){}

void QPimSyncStorage::beginTransaction(const QDateTime &time)
{
    mUnmappedCategories.clear();
    m->startSyncTransaction(time);
}

void QPimSyncStorage::abortTransaction()
{
    m->abortSyncTransaction();
}

void QPimSyncStorage::commitTransaction()
{
    m->commitSyncTransaction();
}

// appointments
QAppointmentSyncStorage::QAppointmentSyncStorage()
    : QPimSyncStorage("calendar", 0), model(new QAppointmentModel(this))
{
    setModel(model);
}

QAppointmentSyncStorage::~QAppointmentSyncStorage()
{
}

void QAppointmentSyncStorage::commitTransaction()
{
    QPimSyncStorage::commitTransaction();
    QCategoryManager c("Calendar");
    foreach(const QString &category, unmappedCategories()) {
        QString id = c.add(category);
        if (id != category) {
            QSqlQuery q(QtopiaSql::instance()->systemDatabase());
            if (!q.prepare("UPDATE appointmentcategories SET categoryid = :i WHERE categoryid = :c"))
                qWarning() << "Failed to prepare category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();

            q.bindValue(":c", category);
            q.bindValue(":i", id);
            if (!q.exec())
                qWarning() << "Failed to execute category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();
        }
    }
}

void QAppointmentSyncStorage::createServerRecord(const QByteArray &record)
{
    qLog(Synchronization) << "QAppointmentSyncStorage::createServerRecord" << record;
    QAppointment appointment;
    // exceptions need to be applied after the appointment is added.
    QList<QPimXmlException> exceptions;

    QString serverId;
    QPimXmlStreamReader h(record);
    appointment = h.readAppointment(serverId, exceptions, model);
    if (h.hasError())
        return;

    mergeUnmappedCategories(h.missedLabels());

    QUniqueId clientId = model->addAppointment(appointment);
    appointment.setUid(clientId);

    emit mappedId(serverId, clientId.toString());

    foreach(const QPimXmlException &e, exceptions) {
        if (e.replacement) {
            QOccurrence o(e.originalDate, e.appointment);
            model->replaceOccurrence(appointment, o);
        } else {
            model->removeOccurrence(appointment, e.originalDate);
        }
    }
}

void QAppointmentSyncStorage::replaceServerRecord(const QByteArray &record)
{
    qLog(Synchronization) << "QAppointmentSyncStorage::replaceServerRecord" << record;
    QAppointment appointment;
    // exceptions need to be applied after the appointment is added.
    QList<QPimXmlException> exceptions;

    QString serverId;
    QPimXmlStreamReader h(record);
    appointment = h.readAppointment(serverId, exceptions, model);
    if (h.hasError())
        return;

    mergeUnmappedCategories(h.missedLabels());

    // update all exceptions as well.
    model->updateAppointment(appointment);
    // refresh to get exception info from db.
    appointment = model->appointment(appointment.uid());

    // The desktop sends down all exceptions so remove whatever is here now!
    QList<QAppointment::Exception> origList = appointment.exceptions();
    foreach(const QAppointment::Exception &oe, origList) {
        model->restoreOccurrence(appointment.uid(), oe.date);
    }

    // Set the exceptions
    foreach(const QPimXmlException &e, exceptions) {
        if (e.replacement) {
            QOccurrence o(e.originalDate, e.appointment);
            model->replaceOccurrence(appointment, o);
        } else {
            model->removeOccurrence(appointment, e.originalDate);
        }
    }
}

void QAppointmentSyncStorage::removeServerRecord(const QString &localId)
{
    qLog(Synchronization) << "QAppointmentSyncStorage::removeServerRecord" << localId;
    QUniqueId id(localId);
    model->removeAppointment(id);
}

QList<QPimXmlException> QAppointmentSyncStorage::convertExceptions(const QList<QAppointment::Exception> origList) const
{
    QList<QPimXmlException> newList;
    foreach(const QAppointment::Exception &e, origList)
    {
        QPimXmlException n;
        n.originalDate = e.date;
        n.replacement = !e.alternative.isNull();
        if (n.replacement)
            n.appointment = model->appointment(e.alternative);
        newList.append(n);
    }
    return newList;
}

void QAppointmentSyncStorage::fetchChangesSince(const QDateTime &since)
{
    // if since null, slow sync, send all added.
    // else two way sync, use added, removed and modified.

    QList<QUniqueId> changes = model->added(since);
    QMap<QUniqueId, int> exceptions;

    // created
    foreach(const QUniqueId &id, changes) {
        QAppointment a = model->appointment(id);
        // virtual events are not sent
        if (!a.parentDependency().isNull())
            continue;
        if (a.isException()) {
            if (!exceptions.contains(a.exceptionParent()))
                exceptions.insert(a.exceptionParent(), 0);
            continue;
        } else if (a.hasExceptions()) {
            exceptions.insert(a.uid(), 1);
        }

        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeAppointment(a, convertExceptions(a.exceptions()));
        emit createClientRecord(record);
    }
    if (since.isNull()) {
        // slow sync, modified and removed make no sense, create for
        // exceptions would have been caught anyway;
        emit clientChangesCompleted();
        return;
    }

    changes = model->removed(since);

    // removed. (removing can lead to modified)
    foreach(const QUniqueId &id, changes) {
        QAppointment a = model->appointment(id);
        // virtual events are not sent
        if (!a.parentDependency().isNull())
            continue;
        if (a.isException()) {
            if (!exceptions.contains(a.exceptionParent()))
                exceptions.insert(a.exceptionParent(), 0);
            continue;
        } else if (a.hasExceptions()) {
            exceptions.insert(a.uid(), 3);
        }
        emit removeClientRecord(id.toString());
    }

    // modified
    changes = model->modified(since);
    foreach(const QUniqueId &id, changes) {
        QAppointment a = model->appointment(id);
        // virtual events are not sent
        if (!a.parentDependency().isNull())
            continue;
        if (a.isException()) {
            if (!exceptions.contains(a.exceptionParent()))
                exceptions.insert(a.exceptionParent(), 0);
            continue;
        } else if (a.hasExceptions()) {
            exceptions.insert(a.uid(), 2);
        }
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeAppointment(a, convertExceptions(a.exceptions()));
        emit replaceClientRecord(record);
    }
    // now for the exceptions where the parent not already changed
    QMapIterator<QUniqueId, int> it(exceptions);
    while(it.hasNext()) {
        it.next();
        if (it.value() == 0) {
            // changed exception, parent not already 1(added) 2(modified) or 3(removed);
            QAppointment a = model->appointment(it.key());
            QByteArray record;
            QPimXmlStreamWriter h(&record);
            h.writeAppointment(a, convertExceptions(a.exceptions()));
            emit replaceClientRecord(record);

        }

    }

    emit clientChangesCompleted();
}

QContactSyncStorage::QContactSyncStorage()
    : QPimSyncStorage("contacts", 0), model(new QContactModel(this))
{
    setModel(model);
}

QContactSyncStorage::~QContactSyncStorage()
{
}

void QContactSyncStorage::commitTransaction()
{
    qLog(Synchronization) << "task sync transaction.";
    QPimSyncStorage::commitTransaction();
    QCategoryManager c("Address Book");
    foreach(const QString &category, unmappedCategories()) {
        QString id = c.add(category);
        qLog(Synchronization) << "add category" << category << "with id" << id;
        if (id != category) {
            QSqlQuery q(QtopiaSql::instance()->systemDatabase());
            if (!q.prepare("UPDATE contactcategories SET categoryid = :i WHERE categoryid = :c"))
                qWarning() << "Failed to prepare category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();

            q.bindValue(":c", category);
            q.bindValue(":i", id);
            if (!q.exec())
                qWarning() << "Failed to execute category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();
        }
    }
}

void QContactSyncStorage::createServerRecord(const QByteArray &record)
{
    qLog(Synchronization) << "QContactSyncStorage::createServerRecord" << record;
    QPimXmlStreamReader h(record);
    QString serverId;
    QContact c = h.readContact(serverId, model);
    if (!h.hasError()) {
        c.setUid(model->addContact(c));
        mergeUnmappedCategories(h.missedLabels());
        //qDebug() << "QContactSyncStorage::createServerRecord" << "mappedId" << serverId << c.uid().toString();
        emit mappedId(serverId, c.uid().toString());
    } else {
        qLog(Synchronization) << "failed to parse:" << int(h.error()) << h.errorString();
    }
}

void QContactSyncStorage::replaceServerRecord(const QByteArray &record)
{
    qLog(Synchronization) << "QContactSyncStorage::replaceServerRecord" << record;
    QPimXmlStreamReader h(record);
    QString serverId;
    QContact c = h.readContact(serverId, model);
    if (!h.hasError()) {
        model->updateContact(c);
        mergeUnmappedCategories(h.missedLabels());
    } else {
        qLog(Synchronization) << "failed to parse:" << int(h.error()) << h.errorString();
    }
}

void QContactSyncStorage::removeServerRecord(const QString &localId)
{
    qLog(Synchronization) << "QContactSyncStorage::removeServerRecord" << localId;
    model->removeContact(QUniqueId(localId));
}

void QContactSyncStorage::fetchChangesSince(const QDateTime &since)
{
    QList<QUniqueId> changes = model->added(since);
    qLog(Synchronization) << "added" << changes.count();

    foreach(const QUniqueId &id, changes) {
        QContact c = model->contact(id);
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeContact(c);
        emit createClientRecord(record);
    }
    if (since.isNull()) {
        // slow sync, modified and removed make no sense
        emit clientChangesCompleted();
        return;
    }

    changes = model->removed(since);

    qLog(Synchronization) << "removed" << changes.count();

    foreach(const QUniqueId &id, changes) {
        emit removeClientRecord(id.toString());
    }

    changes = model->modified(since);

    qLog(Synchronization) << "modified" << changes.count();

    foreach(const QUniqueId &id, changes) {
        QContact c = model->contact(id);
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeContact(c);
        emit replaceClientRecord(record);
    }

    emit clientChangesCompleted();
}

QTaskSyncStorage::QTaskSyncStorage()
    : QPimSyncStorage("tasks", 0), model(new QTaskModel(this))
{
    setModel(model);
}

QTaskSyncStorage::~QTaskSyncStorage()
{
}

void QTaskSyncStorage::commitTransaction()
{
    QPimSyncStorage::commitTransaction();
    QCategoryManager c("Todo List");
    foreach(const QString &category, unmappedCategories()) {
        QString id = c.add(category);
        if (id != category) {
            QSqlQuery q(QtopiaSql::instance()->systemDatabase());
            if (!q.prepare("UPDATE taskcategories SET categoryid = :i WHERE categoryid = :c"))
                qWarning() << "Failed to prepare category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();

            q.bindValue(":c", category);
            q.bindValue(":i", id);
            if (!q.exec())
                qWarning() << "Failed to execute category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();
        }
    }
}

void QTaskSyncStorage::createServerRecord(const QByteArray &record)
{
    qLog(Synchronization) << "QTaskSyncStorage::createServerRecord" << record;
    QPimXmlStreamReader h(record);
    QString serverId;
    QTask t = h.readTask(serverId, model);
    if (!h.hasError()) {
        t.setUid(model->addTask(t));
        mergeUnmappedCategories(h.missedLabels());
        emit mappedId(serverId, t.uid().toString());
    } else {
        qLog(Synchronization) << "failed to parse:" << int(h.error()) << h.errorString();
    }
}

void QTaskSyncStorage::replaceServerRecord(const QByteArray &record)
{
    qLog(Synchronization) << "QTaskSyncStorage::replaceServerRecord" << record;
    QString serverId;
    QPimXmlStreamReader h(record);
    QTask t = h.readTask(serverId, model);
    if (!h.hasError()) {
        model->updateTask(t);
        mergeUnmappedCategories(h.missedLabels());
    } else {
        qLog(Synchronization) << "failed to parse:" << int(h.error()) << h.errorString();
    }
}

void QTaskSyncStorage::removeServerRecord(const QString &localId)
{
    qLog(Synchronization) << "QTaskSyncStorage::removeServerRecord" << localId;
    model->removeTask(QUniqueId(localId));
}

void QTaskSyncStorage::fetchChangesSince(const QDateTime &since)
{
    QList<QUniqueId> changes = model->added(since);

    foreach(const QUniqueId &id, changes) {
        QTask t = model->task(id);
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeTask(t);
        emit createClientRecord(record);
    }
    if (since.isNull()) {
        // slow sync, modified and removed make no sense
        emit clientChangesCompleted();
        return;
    }

    changes = model->removed(since);
    foreach(const QUniqueId &id, changes) {
        emit removeClientRecord(id.toString());
    }

    changes = model->modified(since);
    foreach(const QUniqueId &id, changes) {
        QTask t = model->task(id);
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeTask(t);
        emit replaceClientRecord(record);
    }

    emit clientChangesCompleted();
}

