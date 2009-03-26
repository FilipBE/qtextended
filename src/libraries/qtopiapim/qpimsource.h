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

#ifndef QPIMSOURCE_H
#define QPIMSOURCE_H

#include <QUuid>
#include <QString>
#include <QSet>
#include <QUniqueId>

#include <QList>
#include <QContact>
#include <QTask>
#include <QAppointment>

#include <qtopiaipcmarshal.h>

struct QTOPIAPIM_EXPORT QPimSource
{
    QUuid context;
    QString identity;
    bool operator==(const QPimSource &other) const
    { return context == other.context && identity == other.identity; }
    bool operator!=(const QPimSource &other) const
    { return context != other.context || identity != other.identity; }
    bool operator<(const QPimSource &other) const
    {
        if (context < other.context)
            return true;
        return identity < other.identity;
    }
    bool isNull() const
    { return context.isNull() && identity.isNull(); }

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
};

Q_DECLARE_USER_METATYPE(QPimSource)

uint qHash(const QPimSource &s);

class QTOPIAPIM_EXPORT QPimContext : public QObject
{
    Q_OBJECT
public:
    virtual QIcon icon() const; // default empty
    virtual QString description() const = 0;
    virtual QString title() const = 0;
    virtual QString title(const QPimSource &) const { return title(); }

    virtual QPimSource defaultSource() const = 0;

    virtual bool editable() const;
    virtual bool editable(const QUniqueId &) const { return editable(); } // default true

    virtual void setVisibleSources(const QSet<QPimSource> &) {}
    virtual QSet<QPimSource> visibleSources() const { return sources(); }
    virtual QSet<QPimSource> sources() const = 0;
    virtual QUuid id() const = 0;

    virtual bool exists(const QUniqueId &) const = 0;
    virtual bool exists(const QUniqueId &id, const QPimSource &source) const
    {
        return !source.isNull() && (this->source(id) == source) && exists(id);
    }
    virtual QPimSource source(const QUniqueId &) const = 0;

protected:
    explicit QPimContext(QObject *parent = 0);
};

class QContact;
class QAppointment;
class QOccurrence;
class QTask;

class QTOPIAPIM_EXPORT QContactContext : public QPimContext
{
    Q_OBJECT
public:
    virtual bool updateContact(const QContact &) { return false; }
    virtual bool removeContact(const QUniqueId &) { return false; }
    virtual QUniqueId addContact(const QContact &, const QPimSource &) { return QUniqueId(); }

    virtual QList<QContact> exportContacts(const QPimSource &, bool &ok) const { ok = false; return QList<QContact>(); }
    virtual bool importContacts(const QPimSource &, const QList<QContact> &) { return false; }

    virtual QContact exportContact(const QUniqueId &, bool &ok) const { ok = false; return QContact(); }
    virtual bool importContact(const QPimSource &s, const QContact &c)
    { QList<QContact> l; l << c; return importContacts(s, l); }

protected:
    explicit QContactContext(QObject *parent = 0)
        : QPimContext(parent) {}
};

class QTOPIAPIM_EXPORT QAppointmentContext : public QPimContext
{
    Q_OBJECT
public:
    virtual bool updateAppointment(const QAppointment &) { return false; }
    virtual bool removeAppointment(const QUniqueId &) { return false; }
    virtual QUniqueId addAppointment(const QAppointment &, const QPimSource &) { return QUniqueId(); }

    virtual bool removeOccurrence(const QUniqueId &, const QDate &) { return false; }
    virtual bool restoreOccurrence(const QUniqueId &, const QDate &) { return false; }
    virtual QUniqueId replaceOccurrence(const QUniqueId &, const QOccurrence &, const QDate& = QDate()) { return QUniqueId(); }
    virtual QUniqueId replaceRemaining(const QUniqueId &, const QAppointment &, const QDate& = QDate()) { return QUniqueId(); }

    virtual QList<QAppointment> exportAppointments(const QPimSource &, bool &ok) const { ok = false; return QList<QAppointment>(); }
    virtual bool importAppointments(const QPimSource &, const QList<QAppointment> &) { return false; }

    virtual QAppointment exportAppointment(const QUniqueId &, bool &ok) const { ok = false; return QAppointment(); }
    virtual bool importAppointment(const QPimSource &s, const QAppointment &c)
    { QList<QAppointment> l; l << c; return importAppointments(s, l); }

protected:
    explicit QAppointmentContext(QObject *parent = 0)
        : QPimContext(parent) {}
};

class QTOPIAPIM_EXPORT QTaskContext : public QPimContext
{
    Q_OBJECT
public:
    virtual bool updateTask(const QTask &) { return false; }
    virtual bool removeTask(const QUniqueId &) { return false; }
    virtual QUniqueId addTask(const QTask &, const QPimSource &) { return QUniqueId(); }

    virtual QList<QTask> exportTasks(const QPimSource &, bool &ok) const { ok = false; return QList<QTask>(); }
    virtual bool importTasks(const QPimSource &, const QList<QTask> &) { return false; }

    virtual QTask exportTask(const QUniqueId &, bool &ok) const { ok = false; return QTask(); }
    virtual bool importTask(const QPimSource &s, const QTask &c)
    { QList<QTask> l; l << c; return importTasks(s, l); }
protected:
    explicit QTaskContext(QObject *parent = 0)
        : QPimContext(parent) {}
};

#endif
