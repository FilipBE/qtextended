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

// For contact birthday/anniversary feature etc.

#include "qpimdependencylist_p.h"
#include "qpimsqlio_p.h"
#include "qtopialog.h"

/*******
  QPimDependencyList
  ******/

/*!
  \class QPimDependencyList
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QPimDependencyList class manages the dependencies between QPimRecords.

  This allows the relationships between certain types of records to be maintained.
  For example, a QContact may have a QAppointment that represents the birthday or
  anniversary of that contact.  In this case, the QAppointment depends on the QContact.
  When the QContact is removed, the QAppointments will be removed as well.

  Currently there are three types of dependencies defined:
  \list
  \o \c "birthday" - the parent record is a QContact, and the dependent
    record is the QAppointment representing that contact's birthday
  \o \c "anniversary" - the parent record is a QContact, and the dependent
    record is the QAppointment representing that contact's anniversary
  \o \c "duedate" - the parent record is a QTask, and the dependent
    record is the QAppointment representing that task's due date.
  \endlist

  \sa {Pim Library}
  */

class QPimDependencyListData
{
    public:
        QPimDependencyListData() :
            parentQuery("SELECT srcrecid FROM pimdependencies WHERE destrecid = :destrecid"),
            childrenQuery("SELECT destrecid, deptype FROM pimdependencies WHERE srcrecid = :srcrecid"),
            typedChildrenQuery("SELECT destrecid FROM pimdependencies WHERE srcrecid = :srcrecid AND deptype = :deptype"),
            insertQuery("INSERT INTO pimdependencies (srcrecid, destrecid, deptype) VALUES(:srcrecid, :destrecid, :deptype)"),
            updateQuery("UPDATE pimdependencies SET destrecid=:destrecid WHERE srcrecid=:srcrecid AND deptype = :deptype"),
            removeQuery("DELETE FROM pimdependencies WHERE srcrecid = :srcrecid AND destrecid = :destrecid AND deptype = :deptype"),
            typeQuery("SELECT deptype FROM pimdependencies WHERE destrecid = :destrecid")
    {
    }

    QPreparedSqlQuery parentQuery;
    QPreparedSqlQuery childrenQuery;
    QPreparedSqlQuery typedChildrenQuery;
    QPreparedSqlQuery insertQuery;
    QPreparedSqlQuery updateQuery;
    QPreparedSqlQuery removeQuery;
    QPreparedSqlQuery typeQuery;
};

Q_GLOBAL_STATIC(QPimDependencyListData, depData);


/*!
  If the record identified by \a recid has a parent dependency,
  returns the identifier for that parent.  Otherwise returns
  a null identifier.

  \sa parentDependencyType()
*/
QUniqueId QPimDependencyList::parentRecord(const QUniqueId& recid)
{
    QPimDependencyListData *d = depData();

    if (!d->parentQuery.prepare()) {
        qWarning("Failed to prepare parent dependency query: %s", d->parentQuery.lastError().text().toLocal8Bit().constData());
        return QUniqueId();
    }

    d->parentQuery.bindValue(":destrecid", recid.toUInt());

    if (!d->parentQuery.exec()) {
        qWarning("Failed to execute find parent dependency query: %s", d->parentQuery.lastError().text().toLocal8Bit().constData());
    } else if (d->parentQuery.next()) {
        QUniqueId ret = QUniqueId::fromUInt(d->parentQuery.value(0).toUInt());
        d->parentQuery.reset();
        return ret;
    }
    return QUniqueId();
}

/*!
  If the record identified by \a recid has a parent dependency,
  returns the type of the dependency.  Otherwise returns
  a null string.

  \sa parentRecord()
*/
QString QPimDependencyList::parentDependencyType(const QUniqueId& recid)
{
    QPimDependencyListData *d = depData();

    if (!d->typeQuery.prepare()) {
        qWarning("Failed to prepare parent type dependency query: %s", d->typeQuery.lastError().text().toLocal8Bit().constData());
        return QString();
    }

    d->typeQuery.bindValue(":destrecid", recid.toUInt());

    if (!d->typeQuery.exec()) {
        qWarning("Failed to execute find parent type dependency query: %s", d->typeQuery.lastError().text().toLocal8Bit().constData());
    } else if (d->typeQuery.next()) {
        QString ret = d->typeQuery.value(0).toString();
        d->typeQuery.reset();
        return ret;
    }
    return QString();
}

/*!
  Returns the dependency type and identifier of any records that
  are a dependant of the record with the given \a recid

  \sa typedChildrenRecords()
*/
QMap<QString, QUniqueId> QPimDependencyList::childrenRecords(const QUniqueId& recid)
{
    QMap<QString, QUniqueId> deps;
    QPimDependencyListData *d = depData();

    if (d->childrenQuery.prepare()) {
        d->childrenQuery.bindValue(":srcrecid", recid.toUInt());

        if (d->childrenQuery.exec()) {
            while(d->childrenQuery.next()) {
                deps.insert(d->childrenQuery.value(1).toString(), QUniqueId::fromUInt(d->childrenQuery.value(0).toUInt()));
            }
            d->childrenQuery.reset();
        } else {
            qWarning("Failed to execute find child dependency query: %s", d->childrenQuery.lastError().text().toLocal8Bit().constData());
        }
    } else {
        qWarning("Failed to prepare child dependency query: %s", d->childrenQuery.lastError().text().toLocal8Bit().constData());
    }
    return deps;
}

/*!
  Returns the identifiers of any records that are a dependant
  of the record with the given \a recid, and have the given
  \a deptype of dependency.

  \sa childrenRecords()
*/
QList<QUniqueId> QPimDependencyList::typedChildrenRecords(const QUniqueId& recid, const QString& deptype)
{
    QList<QUniqueId> deps;
    QPimDependencyListData *d = depData();

    if (d->typedChildrenQuery.prepare()) {
        d->typedChildrenQuery.bindValue(":srcrecid", recid.toUInt());
        d->typedChildrenQuery.bindValue(":deptype", deptype);

        if (d->typedChildrenQuery.exec()) {
            while(d->typedChildrenQuery.next()) {
                deps.append(QUniqueId::fromUInt(d->typedChildrenQuery.value(0).toUInt()));
            }
            d->typedChildrenQuery.reset();
        } else {
            qWarning("Failed to execute find typed child dependency query: %s", d->typedChildrenQuery.lastError().text().toLocal8Bit().constData());
        }
    } else {
        qWarning("Failed to prepare typed child dependency query: %s", d->typedChildrenQuery.lastError().text().toLocal8Bit().constData());
    }
    return deps;
}

/*!
  Records the dependency of the record given by \a destid on the parent record \a srcid, with
  the given dependency type \a deptype.

  Returns true on success, false otherwise.
*/
bool QPimDependencyList::addDependency(const QUniqueId& srcid, const QUniqueId& destid, const QString& deptype)
{
    bool ret = true;
    QPimDependencyListData *d = depData();

    QPreparedSqlQuery& q = d->insertQuery;
    if (q.prepare()) {
        q.bindValue(":srcrecid", srcid.toUInt());
        q.bindValue(":destrecid", destid.toUInt());
        q.bindValue(":deptype", deptype);

        if (!q.exec()) {
            qWarning("Failed to execute insert dependency query: %s", q.lastError().text().toLocal8Bit().constData());
            ret = false;
        }
    } else {
        qWarning("Failed to prepare insert dependency query: %s", q.lastError().text().toLocal8Bit().constData());
        ret = false;
    }

    return ret;
}

/*!
  Updates the target of the dependency from the given \a srcid and type \a deptype to
  the new destination \a destid.

  Returns true on success, false otherwise.
  */
bool QPimDependencyList::updateDependency(const QUniqueId& srcid, const QUniqueId& destid, const QString& deptype)
{
    bool ret = true;
    QPimDependencyListData *d = depData();

    QPreparedSqlQuery& q = d->updateQuery;
    if (q.prepare()) {
        q.bindValue(":srcrecid", srcid.toUInt());
        q.bindValue(":destrecid", destid.toUInt());
        q.bindValue(":deptype", deptype);

        if (!q.exec()) {
            qWarning("Failed to execute update dependency query: %s", q.lastError().text().toLocal8Bit().constData());
            ret = false;
        }
    } else {
        qWarning("Failed to prepare update dependency query: %s", q.lastError().text().toLocal8Bit().constData());
        ret = false;
    }

    return ret;
}

/*!
  Removes the dependency from the record \a destid to the record \a srcid, with the
  specified dependency type \a deptype.

  Returns true on success, false otherwise.
*/
bool QPimDependencyList::removeDependency(const QUniqueId& srcid, const QUniqueId& destid, const QString& deptype)
{
    bool ret = true;
    QPimDependencyListData *d = depData();

    QPreparedSqlQuery& q = d->removeQuery;
    if (q.prepare()) {
        q.bindValue(":srcrecid", srcid.toUInt());
        q.bindValue(":destrecid", destid.toUInt());
        q.bindValue(":deptype", deptype);

        if (!q.exec()) {
            qWarning("Failed to execute remove dependency query: %s", q.lastError().text().toLocal8Bit().constData());
            ret = false;
        }
    } else {
        qWarning("Failed to prepare remove dependency query: %s", q.lastError().text().toLocal8Bit().constData());
        ret = false;
    }

    return ret;
}

