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

// Only use simple phone types (home, business, mobile, and VOIP)
// sa applications/contacts/details.h
#define SIMPLE_PHONETYPES

/*****************
 * NameGenerator *
 *****************/
NameGenerator::NameGenerator(const char *filename)
{
    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    QTextStream ts(&f);
    QString text = ts.readLine();
    while(!text.isEmpty()) {
        names.append(text);
        text = ts.readLine();
    }
    f.close();
}

QString NameGenerator::randomName() const
{
    if (names.count() == 0)
        return QString();
    int pos = rand() % names.count();
    return names[pos];
}

/*******************
 * RecordGenerator *
 *******************/
QStringList RecordGenerator::categories() const
{
    QStringList result;
    if (oneTimeIn(3))
        result.append("Personal");
    if (oneTimeIn(5))
        result.append("Business");
    return result;
}

/********************
 * ContactGenerator *
 ********************/
ContactGenerator::ContactGenerator()
    : RecordGenerator(),
    maleNames(":mnames"),
    femaleNames(":fnames"),
    surnames(":snames"),
    streetNames(":streets"),
    suburbNames(":suburbs"),
    townNames(":towns"),
    stateNames(":states"),
    emailServers(":servers")
{
}

ContactGenerator::~ContactGenerator()
{
}

QContact ContactGenerator::nextContact() const
{
    // Name
    QContact c;
    c.setFirstName(firstName());
    c.setLastName(surnames.randomName());

    // Phone Numbers
#ifdef SIMPLE_PHONETYPES
    QList<QContact::PhoneType> types;
    types << QContact::HomePhone << QContact::Mobile << QContact::BusinessPhone << QContact::VOIP;
    foreach(QContact::PhoneType type, types) {
        if (oneTimeIn(3)) {
            c.setPhoneNumber(type, phoneNumber());
        }
    }
#else
    QList<QContact::PhoneType> types = QContact::phoneTypes();
    foreach(QContact::PhoneType type, types) {
        if (oneTimeIn(8)) {
            c.setPhoneNumber(type, phoneNumber());
        }
    }
#endif
    if (c.defaultPhoneNumber().isNull())
        c.setHomePhone(phoneNumber());

    // email
    if (oneTimeIn(5)) {
        c.setDefaultEmail(emailAddress(c.firstName(), c.lastName()));
    }

    // address
    if (oneTimeIn(8)) {
        c.setAddress(QContact::Home, address());
    }
    if (oneTimeIn(8)) {
        c.setAddress(QContact::Business, address());
    }
    c.setCategories(categories());// based of Business and Personal
    return c;
}

QString ContactGenerator::firstName() const
{
    if (rand() % 2)
        return femaleNames.randomName();
    return maleNames.randomName();
}

QString ContactGenerator::phoneNumber() const
{
    char buffer[8];
    buffer[7]= '\0';
    for (int i = 0; i < 7; ++i)
        buffer[i] = (rand() % 10) + '0';

    return QString(buffer);
}

QString ContactGenerator::emailAddress(const QString &fn, const QString &ln) const
{
    switch(rand() % 4) {
        case 0:
            return fn + "@" + emailServers.randomName();
        case 1:
            return ln + "@" + emailServers.randomName();
        case 2:
            return fn + "." + ln + "@" + emailServers.randomName();
        case 3:
            break;
    }
    return fn + "." + numberRange(100, 999) + "@" + emailServers.randomName();
}

QContactAddress ContactGenerator::address() const
{
    QContactAddress a;
    a.street = numberRange(5, 200) + " " + streetNames.randomName() + "\n"
        + suburbNames.randomName();
    a.city = townNames.randomName();
    a.state = stateNames.randomName();
    a.country = "Australia";
    a.zip = numberRange(1000, 9999);

    return a;
}

/************************
 * AppointmentGenerator *
 ************************/
AppointmentGenerator::AppointmentGenerator()
    : RecordGenerator(),
    description(":desc"),
    location(":loc")
{
}

AppointmentGenerator::~AppointmentGenerator()
{
}


QAppointment AppointmentGenerator::nextAppointment() const
{
    QAppointment a;
    a.setDescription(description.randomName());
    if (!oneTimeIn(3))
        a.setLocation(description.randomName());
    a.setStart(start());
    a.setEnd(end());
    QAppointment::RepeatRule r = rule();
    if (r != QAppointment::NoRepeat) {
        a.setRepeatRule(r);
        a.setFrequency((rand() % 5) + 1);
        a.setRepeatUntil(repeatUntil());
    }
    a.setCategories(categories());
    return a;
}

QDateTime AppointmentGenerator::start() const
{
    // date range is one year around 'today';
    QDate today = QDate::currentDate();
    today = today.addDays((rand() % 365) - 183);

    int hour = (rand() % 14) + 7;
    int minutes;
    // actually less than 1 time in 4 given the chance rand() % 3 will come out 0.
    if (oneTimeIn(4)) {
        minutes = 15 * (rand() % 3);
    } else {
        minutes = 0;
    }

    int duration = 15 * (rand() % 3);
    QDateTime start(today, QTime(hour, minutes));
    lastEndTime = start.addSecs(duration*60);
    return start;
}

QDateTime AppointmentGenerator::end() const
{
    return lastEndTime;
}


QAppointment::RepeatRule AppointmentGenerator::rule() const
{
    if (oneTimeIn(4)) {
        int r = (rand() % 5)+QAppointment::Daily;
        return (QAppointment::RepeatRule)r;
    }
    return QAppointment::NoRepeat;
}

QDate AppointmentGenerator::repeatUntil() const
{
    if (oneTimeIn(4))
        return QDate();
    return lastEndTime.date().addDays(rand()%60);
}

/*****************
 * TaskGenerator *
 *****************/

TaskGenerator::TaskGenerator()
    : RecordGenerator(),
    description(":task")
{
}

TaskGenerator::~TaskGenerator()
{
}


QTask TaskGenerator::nextTask() const
{
    QTask t;
    t.setDescription(description.randomName());
    t.setPriority((rand() % 5)+1);
    t.setStatus(QTask::Status(rand()%5)); // may be over-ridden later.
    t.setDueDate(due());
    t.setCompletedDate(completed());
    t.setCategories(categories());
    if (t.status() == QTask::InProgress || t.status() == QTask::Deferred || t.status() == QTask::Waiting)
        t.setPercentCompleted((rand() %99)+1);
    return t;
}


QDate TaskGenerator::due() const
{
    if (oneTimeIn(4))
        return QDate::currentDate().addDays(rand() % 365);
    return QDate();
}


QDate TaskGenerator::completed() const
{
    if (oneTimeIn(4))
        return QDate();
    return QDate::currentDate().addDays(-(rand() % 365));
}
