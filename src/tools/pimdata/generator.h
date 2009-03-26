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

#ifndef GENERATOR_H
#define GENERATOR_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QContact>
#include <QAppointment>
#include <QTask>
#include <QDateTime>

class NameGenerator
{
public:
    NameGenerator(const char *file);

    QString randomName() const;
private:
    QVector<QString> names;
};

class RecordGenerator
{
public:
    RecordGenerator() {}
    virtual ~RecordGenerator() {}

    QStringList categories() const;
    bool oneTimeIn(int odds) const { return (rand() % odds) == 0; }
    QString numberRange(int start, int end) const { return QString::number((rand() % end-start) + start); }

    // should have function for setting reference date.
};

class ContactGenerator : public RecordGenerator
{
public:
    ContactGenerator();
    virtual ~ContactGenerator();

    QContact nextContact() const;

    QString firstName() const;
    QString phoneNumber() const;
    QString emailAddress(const QString &fn, const QString &ln) const;
    QContactAddress address() const;

private:
    NameGenerator maleNames;
    NameGenerator femaleNames;
    NameGenerator surnames;

    NameGenerator streetNames;
    NameGenerator suburbNames;
    NameGenerator townNames;
    NameGenerator stateNames;

    NameGenerator emailServers;
};

class AppointmentGenerator : public RecordGenerator
{
public:
    AppointmentGenerator();
    virtual ~AppointmentGenerator();

    QAppointment nextAppointment() const;

    QDateTime start() const;
    QDateTime end() const;

    QAppointment::RepeatRule rule() const;
    QDate repeatUntil() const;

private:
    NameGenerator description;
    NameGenerator location;

    mutable QDateTime lastEndTime;
};

class TaskGenerator : public RecordGenerator
{
public:
    TaskGenerator();
    virtual ~TaskGenerator();

    QTask nextTask() const;

    QDate due() const;

    QDate completed() const;

private:
    NameGenerator description;
};

#endif
