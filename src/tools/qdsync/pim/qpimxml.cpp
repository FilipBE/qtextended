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
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMap>
#include <QBuffer>
#include <QDebug>

#include <QTaskModel>
#include <QContactModel>
#include <QAppointmentModel>

#include <QCategoryManager>

#include <qtopialog.h>

#include "qpimxml_p.h"

namespace PIMXML_NAMESPACE {

// have a static set of pre-declared QLatin1Strings.  re-constructiong QStrings for each element will be expensive.
// in order of appearance in schema definitions, duplicates obviously excluded.
static const QLatin1String t_Contacts("Contacts");
static const QLatin1String t_Tasks("Tasks");
static const QLatin1String t_Appointments("Appointments");

static const QLatin1String t_HomePhone("HomePhone");
static const QLatin1String t_BusinessPhone("BusinessPhone");
static const QLatin1String t_HomeMobile("HomeMobile");
static const QLatin1String t_HomeFax("HomeFax");
static const QLatin1String t_BusinessMobile("BusinessMobile");
static const QLatin1String t_BusinessFax("BusinessFax");
static const QLatin1String t_BusinessPager("BusinessPager");
static const QLatin1String t_Contact("Contact");
static const QLatin1String t_Identifier("Identifier");
static const QLatin1String t_localIdentifier("localIdentifier");
static const QLatin1String t_NameTitle("NameTitle");
static const QLatin1String t_FirstName("FirstName");
static const QLatin1String t_pronunciation("pronunciation");
static const QLatin1String t_MiddleName("MiddleName");
static const QLatin1String t_LastName("LastName");
static const QLatin1String t_Suffix("Suffix");
static const QLatin1String t_Company("Company");
static const QLatin1String t_BusinessWebpage("BusinessWebpage");
static const QLatin1String t_JobTitle("JobTitle");
static const QLatin1String t_Department("Department");
static const QLatin1String t_Office("Office");
static const QLatin1String t_Profession("Profession");
static const QLatin1String t_Assistant("Assistant");
static const QLatin1String t_Manager("Manager");
static const QLatin1String t_HomeWebpage("HomeWebpage");
static const QLatin1String t_Spouse("Spouse");
static const QLatin1String t_Nickname("Nickname");
static const QLatin1String t_Children("Children");
static const QLatin1String t_Birthday("Birthday");
static const QLatin1String t_Anniversary("Anniversary");
static const QLatin1String t_Portrait("Portrait");
static const QLatin1String t_Notes("Notes");
static const QLatin1String t_Gender("Gender");
static const QLatin1String t_UnspecifiedGender("UnspecifiedGender");
static const QLatin1String t_Male("Male");
static const QLatin1String t_Female("Female");
static const QLatin1String t_Addresses("Addresses");
static const QLatin1String t_Address("Address");
static const QLatin1String t_Street("Street");
static const QLatin1String t_City("City");
static const QLatin1String t_State("State");
static const QLatin1String t_Zip("Zip");
static const QLatin1String t_Country("Country");
static const QLatin1String t_type("type");
static const QLatin1String t_Home("Home");
static const QLatin1String t_Business("Business");
static const QLatin1String t_PhoneNumbers("PhoneNumbers");
static const QLatin1String t_Number("Number");
static const QLatin1String t_default("default");
static const QLatin1String t_EmailAddresses("EmailAddresses");
static const QLatin1String t_Email("Email");
static const QLatin1String t_maxItems("maxItems");
static const QLatin1String t_Categories("Categories");
static const QLatin1String t_Category("Category");
static const QLatin1String t_CustomFields("CustomFields");
static const QLatin1String t_Field("Field");
static const QLatin1String t_Key("Key");
static const QLatin1String t_Value("Value");
static const QLatin1String t_NotStarted("NotStarted");
static const QLatin1String t_InProgress("InProgress");
static const QLatin1String t_Completed("Completed");
static const QLatin1String t_Waiting("Waiting");
static const QLatin1String t_Deferred("Deferred");
static const QLatin1String t_VeryHigh("VeryHigh");
static const QLatin1String t_High("High");
static const QLatin1String t_Normal("Normal");
static const QLatin1String t_Low("Low");
static const QLatin1String t_VeryLow("VeryLow");
static const QLatin1String t_Task("Task");
static const QLatin1String t_Description("Description");
static const QLatin1String t_Priority("Priority");
static const QLatin1String t_Status("Status");
static const QLatin1String t_DueDate("DueDate");
static const QLatin1String t_StartedDate("StartedDate");
static const QLatin1String t_CompletedDate("CompletedDate");
static const QLatin1String t_PercentCompleted("PercentCompleted");
static const QLatin1String t_Daily("Daily");
static const QLatin1String t_Weekly("Weekly");
static const QLatin1String t_MonthlyDate("MonthlyDate");
static const QLatin1String t_MonthlyDay("MonthlyDay");
static const QLatin1String t_MonthlyEndDay("MonthlyEndDay");
static const QLatin1String t_Yearly("Yearly");
static const QLatin1String t_Visible("Visible");
static const QLatin1String t_Audible("Audible");
static const QLatin1String t_Monday("Monday");
static const QLatin1String t_Tuesday("Tuesday");
static const QLatin1String t_Wednesday("Wednesday");
static const QLatin1String t_Thursday("Thursday");
static const QLatin1String t_Friday("Friday");
static const QLatin1String t_Saturday("Saturday");
static const QLatin1String t_Sunday("Sunday");
static const QLatin1String t_Appointment("Appointment");
static const QLatin1String t_Location("Location");
static const QLatin1String t_TimeZone("TimeZone");
static const QLatin1String t_When("When");
static const QLatin1String t_Start("Start");
static const QLatin1String t_End("End");
static const QLatin1String t_StartDate("StartDate");
static const QLatin1String t_EndDate("EndDate");
static const QLatin1String t_Alarm("Alarm");
static const QLatin1String t_Type("Type");
static const QLatin1String t_Delay("Delay");
static const QLatin1String t_Repeat("Repeat");
static const QLatin1String t_Frequency("Frequency");
static const QLatin1String t_Until("Until");
static const QLatin1String t_Nearest("Nearest");
static const QLatin1String t_WeekMask("WeekMask");
static const QLatin1String t_Exception("Exception");
static const QLatin1String t_OriginalDate("OriginalDate");

static const QLatin1String t_DateFormat("yyyy-MM-dd");
static const QLatin1String t_DateTimeFormat("yyyy-MM-ddThh:mm:ss");
static const QLatin1String t_DateTimeFormatUTC("yyyy-MM-ddThh:mm:ssZ");



QPimXmlStreamReader::QPimXmlStreamReader()
    : QXmlStreamReader(), readPast(false), categoryManager(0)
{}

QPimXmlStreamReader::QPimXmlStreamReader(QIODevice *device)
    : QXmlStreamReader(device), readPast(false), categoryManager(0)
{}

QPimXmlStreamReader::QPimXmlStreamReader(const char *text)
    : QXmlStreamReader(text), readPast(false), categoryManager(0)
{}

QPimXmlStreamReader::QPimXmlStreamReader(const QString &text)
    : QXmlStreamReader(text), readPast(false), categoryManager(0)
{}

QPimXmlStreamReader::QPimXmlStreamReader(const QByteArray &array)
    : QXmlStreamReader(array), readPast(false), categoryManager(0)
{}

QPimXmlStreamReader::~QPimXmlStreamReader()
{
    if (categoryManager)
        delete categoryManager;
}

QPimXmlStreamWriter::QPimXmlStreamWriter()
    : QXmlStreamWriter(), categoryManager(0)
{
    setAutoFormatting(true);
}

QPimXmlStreamWriter::QPimXmlStreamWriter(QByteArray *data)
    : QXmlStreamWriter(data), categoryManager(0)
{
    setAutoFormatting(true);
}

QPimXmlStreamWriter::QPimXmlStreamWriter(QIODevice *data)
    : QXmlStreamWriter(data), categoryManager(0)
{
    setAutoFormatting(true);
}

QPimXmlStreamWriter::QPimXmlStreamWriter(QString *data)
    : QXmlStreamWriter(data), categoryManager(0)
{
    setAutoFormatting(true);
}

QPimXmlStreamWriter::~QPimXmlStreamWriter()
{
    if (categoryManager)
        delete categoryManager;
}

void QPimXmlStreamWriter::writeTextElement(const QString &qualifiedName, const QString &value)
{
    QXmlStreamWriter::writeTextElement(qualifiedName, value);
}

void QPimXmlStreamWriter::writeAttribute(const QString &qualifiedName, const QString &value)
{
    QXmlStreamWriter::writeAttribute(qualifiedName, value);
}

void QPimXmlStreamWriter::writeDateElement(const QString &qualifiedName, const QDate &value)
{
    if (value.isValid())
        QXmlStreamWriter::writeTextElement(qualifiedName, value.toString(t_DateFormat));
    else
        QXmlStreamWriter::writeTextElement(qualifiedName, QString());

}

void QPimXmlStreamWriter::writeDateTimeElement(const QString &qualifiedName, const QDateTime &value, bool utc)
{
    if (value.isValid())
        QXmlStreamWriter::writeTextElement(qualifiedName, value.toString(utc?t_DateTimeFormatUTC:t_DateTimeFormat));
    else
        QXmlStreamWriter::writeTextElement(qualifiedName, QString());
}

void QPimXmlStreamWriter::writeBooleanElement(const QString &qualifiedName, bool value)
{
    static QLatin1String ttext("true");
    static QLatin1String ftext("false");
    QXmlStreamWriter::writeTextElement(qualifiedName, value ? ttext : ftext);
}

void QPimXmlStreamWriter::writeCategoryElements(const QStringList &categories)
{
    writeStartElement(t_Categories);
    foreach(const QString &c, categories)
        writeTextElement(t_Category, categoryLabel(c));
    writeEndElement();
}

void QPimXmlStreamWriter::writeCustomFieldElements(const QMap<QString, QString> &fields)
{
    writeStartElement(t_CustomFields);
    QMapIterator<QString, QString> it(fields);
    while(it.hasNext()) {
        it.next();
        writeStartElement(t_Field);
        writeTextElement(t_Key, it.key());
        writeTextElement(t_Value, it.value());
        writeEndElement();
    }
    writeEndElement();
}

/*
   If the current or next starting element has the given \a name moves to that
   element and returns true.
*/
bool QPimXmlStreamReader::readStartElement(const QString &qualifiedName)
{
    qLog(Synchronization) << "::readStartElement(" << qualifiedName << ")" << lineNumber();
    if (readPast)
        return false;

    while(!isEndElement() && !isStartElement() && !atEnd())
        readNext();
    if (isEndElement()) {
        qLog(Synchronization) << "::readStartElement() - premature end element.  Return false for all until readEndElement called." << lineNumber();
        readPast = true;
        return false;
    }
    if (!atEnd() && this->qualifiedName() == qualifiedName) {
        qLog(Synchronization) << "::readStartElement() - correct start element found" << lineNumber();
        a = attributes();
        return true;
    }
    qLog(Synchronization) << "::readStartElement() - incorrect start element found" << lineNumber();
    return false;
}

/*
  Reads to the end element for the current depth, then steps past it, (readNext)
*/
void QPimXmlStreamReader::readEndElement()
{
    qLog(Synchronization) << "::readEndElement()" << lineNumber();
    if (readPast) {
        qLog(Synchronization) << "::readEndElement() - already at premature end element.  Clear flag and return." << lineNumber();
        readNext();
        readPast = false;
        return;
    }
    uint unknownDepth = 0;
    bool first = true;
    while (!atEnd()) {
        TokenType t = tokenType();
        // Don't assume there's going to be whitespace before an element we're interested in
        if (!first || (t != StartElement && t != EndElement))
            readNext();
        first = false;
        if (isStartElement()) {
            unknownDepth++;
        } else if (isEndElement()) {
            if (unknownDepth) {
                unknownDepth--;
            } else {
                readNext();
                break;
            }
        }
    }
    qLog(Synchronization) << "::readEndElement() - foundElement:" << isEndElement() << lineNumber();
}

// read until element, enter, attributes?
QString QPimXmlStreamReader::readTextElement(const QString &qualifiedName)
{
    QString t;
    if (readStartElement(qualifiedName)) {
        t = readElementText();
        Q_ASSERT(tokenType() == EndElement);
        readNext(); // Skip over the EndElement
    }
    return t;
}

QDate QPimXmlStreamReader::readDateElement(const QString &qualifiedName)
{
    QString text = readTextElement(qualifiedName);
    if (text.isEmpty())
        return QDate();
    else
        return QDate::fromString(text, t_DateFormat);
}

QDateTime QPimXmlStreamReader::readDateTimeElement(const QString &qualifiedName, bool utc)
{
    QString text = readTextElement(qualifiedName);
    if (text.isEmpty())
        return QDateTime();
    else
        return QDateTime::fromString(text, utc?t_DateTimeFormatUTC:t_DateTimeFormat);
}

QUniqueId QPimXmlStreamReader::readIdentifierElement(QString &serverId)
{
    QString text = readTextElement(t_Identifier);
    QString mapAttr = readAttribute(t_localIdentifier);
    static QLatin1String ttext("false");
    static QLatin1String tnum("0");
    if (mapAttr == ttext || mapAttr == tnum) {
        serverId = text;
        return QUniqueId();
    }
    return QUniqueId(text);
}

bool QPimXmlStreamReader::readBooleanElement(const QString &qualifiedName)
{
    static QLatin1String ttext("true");
    static QLatin1String tnum("1");
    QString text = readTextElement(qualifiedName);
    return text == ttext || text == tnum;
}

// read until element, enter, attributes?
QString QPimXmlStreamReader::readAttribute(const QString &qualifiedName)
{
    return a.value(qualifiedName).toString();
}

void QPimXmlStreamWriter::setCategoryScope(const QString &scope)
{
    if (categoryManager)
        delete categoryManager;
    categoryManager = new QCategoryManager(scope);
}

void QPimXmlStreamReader::setCategoryScope(const QString &scope)
{
    if (categoryManager)
        delete categoryManager;
    categoryManager = new QCategoryManager(scope);
    mMissedLabels.clear();
}

QString QPimXmlStreamWriter::categoryLabel(const QString &categoryId)
{
    return categoryManager->label(categoryId);
}

QString QPimXmlStreamReader::categoryId(const QString &categoryLabel)
{
    QString id = categoryManager->idForLabel(categoryLabel);
    if (!id.isEmpty())
        return id; // has an id
    if (categoryManager->exists(categoryLabel))
        return categoryLabel; // is an id
#if 0
    // adding goes to a server which gets locked out and fails since we
    // are currently in a transaction already, sqlite doesn't nest, doesn't
    // lock per-table.
    return categoryManager->add(categoryLabel, QString(), true);
#else
    mMissedLabels.insert(categoryLabel); // neither, add after commit PIM record changes.
    return categoryLabel;
#endif
}

int QPimXmlStreamReader::maxItems()
{
    QStringRef text = a.value(t_maxItems);
    if (!text.isEmpty()) {
        bool ok;
        int result = text.toString().toInt(&ok);
        if (ok)
            return result;
    }
    return -1;
}

void QPimXmlStreamReader::readCategoryElements(QPimRecord &record)
{
    if (readStartElement(t_Categories)) {
        bool partial;
        int max = maxItems();
        QStringList orig = record.categories();
        if (orig.count() > max)
            partial = true;
        else
            partial = false;

        readNext();
        QStringList result;
        QString category = readTextElement(t_Category);
        while (!category.isNull()) {
            result.append(categoryId(category));
            category = readTextElement(t_Category);
        }
        readEndElement();
        if (partial) {
            // non-destructive.
            foreach(const QString &cat, orig) {
                if (!result.contains(cat))
                    result.append(cat);
            }
        }
        record.setCategories(result);
    }
}

void QPimXmlStreamReader::readCustomFieldElements(QPimRecord &record)
{
    if (readStartElement(t_CustomFields)) {
        bool partial;
        QMap<QString, QString> orig = record.customFields();
        int max = maxItems();
        if (orig.count() > max)
            partial = true;
        else
            partial = false;

        readNext();
        QMap<QString, QString> result;
        while(readStartElement(t_Field)) {
            readNext();
            QString key = readTextElement(t_Key);
            QString value = readTextElement(t_Value);
            result.insert(key, value);
            readEndElement();
        }
        readEndElement();
        if (partial) {
            QMapIterator<QString, QString> it(orig);
            while(it.hasNext()) {
                it.next();
                // non-destructive.
                if (!result.contains(it.key()))
                    result.insert(it.key(), it.value());
            }
        }
        record.setCustomFields(result);
    }
}

QContact QPimXmlStreamReader::readContact(QString &serverId, const QContactModel *model)
{
    setCategoryScope("Address Book");
    QContact contact;
    if (!readStartElement(t_Contact)) {
        raiseError("Invalid tag");
        return contact;
    }
    readNext();

    QUniqueId id = readIdentifierElement(serverId);
    if (model && !id.isNull() && model->exists(id)) {
        contact = model->contact(id);
    } else {
        contact.setUid(id);
    }

    // Takes advantage of strict ordering of known nodes.
    // Unknown nodes will be skipped automatically.
    if (readStartElement(t_NameTitle))
        contact.setNameTitle(readTextElement(t_NameTitle));
    if (readStartElement(t_FirstName)) {
        contact.setFirstName(readTextElement(t_FirstName));
        // should check...
        contact.setFirstNamePronunciation(readAttribute(t_pronunciation));
    }
    if (readStartElement(t_MiddleName))
        contact.setMiddleName(readTextElement(t_MiddleName));
    if (readStartElement(t_LastName)) {
        contact.setLastName(readTextElement(t_LastName));
        contact.setLastNamePronunciation(readAttribute(t_pronunciation));
    }
    if (readStartElement(t_Suffix))
        contact.setSuffix(readTextElement(t_Suffix));
    if (readStartElement(t_Company)) {
        contact.setCompany(readTextElement(t_Company));
        contact.setCompanyPronunciation(readAttribute(t_pronunciation));
    }
    if (readStartElement(t_BusinessWebpage))
        contact.setBusinessWebpage(readTextElement(t_BusinessWebpage));
    if (readStartElement(t_JobTitle))
        contact.setJobTitle(readTextElement(t_JobTitle));
    if (readStartElement(t_Department))
        contact.setDepartment(readTextElement(t_Department));
    if (readStartElement(t_Office))
        contact.setOffice(readTextElement(t_Office));
    if (readStartElement(t_Profession))
        contact.setProfession(readTextElement(t_Profession));
    if (readStartElement(t_Assistant))
        contact.setAssistant(readTextElement(t_Assistant));
    if (readStartElement(t_Manager))
        contact.setManager(readTextElement(t_Manager));
    if (readStartElement(t_HomeWebpage))
        contact.setHomeWebpage(readTextElement(t_HomeWebpage));
    if (readStartElement(t_Spouse))
        contact.setSpouse(readTextElement(t_Spouse));
    if (readStartElement(t_Nickname))
        contact.setNickname(readTextElement(t_Nickname));
    if (readStartElement(t_Children))
        contact.setChildren(readTextElement(t_Children));
    if (readStartElement(t_Birthday))
        contact.setBirthday(readDateElement(t_Birthday));
    if (readStartElement(t_Anniversary))
        contact.setAnniversary(readDateElement(t_Anniversary));

    if (readStartElement(t_Portrait)) {
        QString portraitText = readTextElement(t_Portrait);
        if (!portraitText.isNull()) {
            QPixmap pixmap;
            QByteArray bytes = QByteArray::fromBase64(portraitText.toLatin1());
            if (pixmap.loadFromData(bytes))
                contact.changePortrait(pixmap);
            else
                contact.changePortrait(QPixmap());
        } else
            contact.changePortrait(QPixmap());
    }

    if (readStartElement(t_Notes))
        contact.setNotes(readTextElement(t_Notes));

    if (readStartElement(t_Gender)) {
        QString genderString = readTextElement(t_Gender);
        if (genderString == t_Male)
            contact.setGender(QContact::Male);
        else if (genderString == t_Female)
            contact.setGender(QContact::Female);
        else
            contact.setGender(QContact::UnspecifiedGender);
    }

    if (readStartElement(t_Addresses)) {
        if (contact.addresses().count() <= maxItems())
            contact.clearAddresses();
        readNext();
        while(readStartElement(t_Address)) {
            readNext();
            QString addressType = readAttribute(t_type);
            QContactAddress address;
            address.street = readTextElement(t_Street);
            address.city = readTextElement(t_City);
            address.state = readTextElement(t_State);
            address.zip = readTextElement(t_Zip);
            address.country = readTextElement(t_Country);
            contact.setAddress(addressType == t_Business ? QContact::Business : QContact::Home, address);
            readEndElement();
        }
        readEndElement();
    }

    if (readStartElement(t_PhoneNumbers)) {
        QString defaultNumber = readAttribute(t_default);
        if (contact.phoneNumbers().count() <= maxItems())
            contact.clearPhoneNumbers();
        readNext();
        QString number = readTextElement(t_Number);
        while (!number.isNull()) {
            QString phType = readAttribute(t_type);
            if (phType == t_BusinessPhone)
                contact.setBusinessPhone(number);
            else if (phType == t_HomeMobile)
                contact.setHomeMobile(number);
            else if (phType == t_HomeFax)
                contact.setHomeFax(number);
            else if (phType == t_BusinessMobile)
                contact.setBusinessMobile(number);
            else if (phType == t_BusinessFax)
                contact.setBusinessFax(number);
            else if (phType == t_BusinessPager)
                contact.setBusinessPager(number);
            else // HomePhone
                contact.setHomePhone(number);

            number = readTextElement(t_Number);
        }
        if (!defaultNumber.isEmpty())
            contact.setDefaultPhoneNumber(defaultNumber);
        readEndElement();
    }


    if (readStartElement(t_EmailAddresses)) {
        QString defaultEmail = readAttribute(t_default);
        QStringList emailList;
        if (contact.emailList().count() > maxItems())
            emailList = contact.emailList();
        readNext();
        QString email = readTextElement(t_Email);
        while(!email.isNull()) {
            if (!emailList.contains(email))
                emailList.append(email);
            email = readTextElement(t_Email);
        }
        contact.setEmailList(emailList);
        contact.setDefaultEmail(defaultEmail);
        readEndElement();
    }

    readCategoryElements(contact);
    readCustomFieldElements(contact);

    readEndElement(); // exit contact.

    return contact;
}

QTask QPimXmlStreamReader::readTask(QString &serverId, const QTaskModel *model)
{
    setCategoryScope("Todo List");
    QTask task;
    if (!readStartElement(t_Task)) {
        raiseError("Invalid tag");
        return task;
    }
    readNext();

    QUniqueId id = readIdentifierElement(serverId);
    if (model && !id.isNull() && model->exists(id)) {
        task = model->task(id);
    } else {
        task.setUid(id);
    }

    // Takes advantage of strict ordering of known nodes.
    // Unknown nodes will be skipped automatically.
    if (readStartElement(t_Description))
        task.setDescription(readTextElement(t_Description));

    if (readStartElement(t_Priority)) {
        QString priorityText = readTextElement(t_Priority);
        if (priorityText == t_VeryHigh)
            task.setPriority(QTask::VeryHigh);
        else if (priorityText == t_High)
            task.setPriority(QTask::High);
        else if (priorityText == t_Normal)
            task.setPriority(QTask::Normal);
        else if (priorityText == t_Low)
            task.setPriority(QTask::Low);
        else if (priorityText == t_VeryLow)
            task.setPriority(QTask::VeryLow);
    }

    if (readStartElement(t_Status)) {
        QString statusText = readTextElement(t_Status);
        if (statusText == t_NotStarted)
            task.setStatus(QTask::NotStarted);
        else if (statusText == t_InProgress)
            task.setStatus(QTask::InProgress);
        else if (statusText == t_Completed)
            task.setStatus(QTask::Completed);
        else if (statusText == t_Waiting)
            task.setStatus(QTask::Waiting);
        else if (statusText == t_Deferred)
            task.setStatus(QTask::Deferred);
    }

    if (readStartElement(t_DueDate))
        task.setDueDate(readDateElement(t_DueDate));
    if (readStartElement(t_StartedDate))
        task.setStartedDate(readDateElement(t_StartedDate));
    if (readStartElement(t_CompletedDate))
        task.setCompletedDate(readDateElement(t_CompletedDate));

    if (readStartElement(t_PercentCompleted)) {
        QString percentText = readTextElement(t_PercentCompleted);
        if (!percentText.isEmpty())
            task.setPercentCompleted(percentText.toInt());
        else
            task.setPercentCompleted(0);
    }
    if (readStartElement(t_Notes))
        task.setNotes(readTextElement(t_Notes));

    readCategoryElements(task);
    readCustomFieldElements(task);

    readEndElement(); // exit task.

    return task;
}

void QPimXmlStreamReader::readBaseAppointmentFields(QAppointment &appointment)
{
    appointment.setDescription(readTextElement(t_Description));
    appointment.setLocation(readTextElement(t_Location));
    QString tzText = readTextElement(t_TimeZone);
    bool utc = false;
    if (!tzText.isEmpty()) {
        appointment.setTimeZone(QTimeZone(tzText.toLatin1()));
        utc = true;
    }
    if (readStartElement(t_When)) {
        readNext();
        // will either be 'Start' or 'StartDate'
        QDateTime startTime = readDateTimeElement(t_Start, utc);
        if (startTime.isValid()) {
            appointment.setStart(startTime);
            appointment.setEnd(readDateTimeElement(t_End, utc));
        } else {
            appointment.setAllDay(true);
            appointment.setStart(QDateTime(readDateElement(t_StartDate), QTime(0,0)));
            appointment.setEnd(QDateTime(readDateElement(t_EndDate), QTime(23,59)));
        }
        readEndElement();
    }
    if (readStartElement(t_Alarm)) {
        readNext();
        if ( tokenType() == QXmlStreamReader::EndElement ) {
            appointment.clearAlarm();
        } else {
            // must have an type and delay if an alarm specified.. no defaults.
            QAppointment::AlarmFlags f;
            QString type = readTextElement(t_Type);
            if ( type == t_Visible )
                f = QAppointment::Visible;
            else
                f = QAppointment::Audible;
            int delay = readTextElement(t_Delay).toInt();
            if ( type.isEmpty() )
                appointment.clearAlarm();
            else
                appointment.setAlarm(delay, f);
        }
        readEndElement();
    }
}

QAppointment QPimXmlStreamReader::readAppointment(QString &serverId, QList<QPimXmlException> &list, const QAppointmentModel *model)
{
    setCategoryScope("Calendar");
    QAppointment appointment;
    if (!readStartElement(t_Appointment)) {
        raiseError("Invalid tag");
        return appointment;
    }
    readNext();

    QUniqueId id = readIdentifierElement(serverId);
    if (model && !id.isNull() && model->exists(id)) {
        appointment = model->appointment(id);
    } else {
        appointment.setUid(id);
    }

    // Takes advantage of strict ordering of known nodes.
    // Unknown nodes will be skipped automatically.
    readBaseAppointmentFields(appointment);

    if (readStartElement(t_Repeat)) {
        readNext();
        QString rtype = readTextElement(t_Type);

        if (rtype == t_Daily)
            appointment.setRepeatRule(QAppointment::Daily);
        else if (rtype == t_Weekly)
            appointment.setRepeatRule(QAppointment::Weekly);
        else if (rtype == t_MonthlyDate)
            appointment.setRepeatRule(QAppointment::MonthlyDate);
        else if (rtype == t_MonthlyDay)
            appointment.setRepeatRule(QAppointment::MonthlyDay);
        else if (rtype == t_MonthlyEndDay)
            appointment.setRepeatRule(QAppointment::MonthlyEndDay);
        else if (rtype == t_Yearly)
            appointment.setRepeatRule(QAppointment::Yearly);

        appointment.setFrequency(readTextElement(t_Frequency).toInt());
        QDate dt = readDateElement(t_Until);
        if ( dt.isValid() )
            appointment.setRepeatUntil( dt );
        else
            appointment.setRepeatForever();
        if (readStartElement(t_Nearest))
            appointment.setShowOnNearest(readBooleanElement(t_Nearest));
        QAppointment::WeekFlags f = 0;
        QStringList wlist = readTextElement(t_WeekMask).split(QLatin1String(" "));
        foreach(const QString &day, wlist) {
            if (day == t_Monday)
                f |= QAppointment::OccurMonday;
            else if (day == t_Tuesday)
                f |= QAppointment::OccurTuesday;
            else if (day == t_Wednesday)
                f |= QAppointment::OccurWednesday;
            else if (day == t_Thursday)
                f |= QAppointment::OccurThursday;
            else if (day == t_Friday)
                f |= QAppointment::OccurFriday;
            else if (day == t_Saturday)
                f |= QAppointment::OccurSaturday;
            else if (day == t_Sunday)
                f |= QAppointment::OccurSunday;
        }
        appointment.setWeekFlags(f);
        while (readStartElement(t_Exception)) {
            readNext();
            QPimXmlException e;
            e.originalDate = readDateElement(t_OriginalDate);
            if (readStartElement(t_Appointment)) {
                readNext();
                e.replacement = true;

                QUniqueId id = readIdentifierElement(e.serverId);
                readBaseAppointmentFields(e.appointment);
                e.appointment.setNotes(readTextElement(t_Notes));

                readCategoryElements(e.appointment);
                readCustomFieldElements(e.appointment);

                readEndElement();
            } else {
                e.replacement = false;
            }
            list.append(e);
            readEndElement();
        }
        readEndElement();
    }
    if (readStartElement(t_Notes))
        appointment.setNotes(readTextElement(t_Notes));
    readCategoryElements(appointment);
    readCustomFieldElements(appointment);

    readEndElement(); // exit appointment.

    return appointment;
}

void QPimXmlStreamWriter::writeContact(const QContact &contact)
{
    setCategoryScope("Address Book");
    writeStartElement(t_Contact);

    writeTextElement(t_Identifier, contact.uid().toString());
    writeTextElement(t_NameTitle, contact.nameTitle());
    writeStartElement(t_FirstName);
    writeAttribute(t_pronunciation, contact.firstNamePronunciation());
    writeCharacters(contact.firstName());
    writeEndElement();
    writeTextElement(t_MiddleName, contact.middleName());
    writeStartElement(t_LastName);
    writeAttribute(t_pronunciation, contact.lastNamePronunciation());
    writeCharacters(contact.lastName());
    writeEndElement();
    writeTextElement(t_Suffix, contact.suffix());
    writeStartElement(t_Company);
    writeAttribute(t_pronunciation, contact.companyPronunciation());
    writeCharacters(contact.company());
    writeEndElement();
    writeTextElement(t_BusinessWebpage, contact.businessWebpage());
    writeTextElement(t_JobTitle, contact.jobTitle());
    writeTextElement(t_Department, contact.department());
    writeTextElement(t_Office, contact.office());
    writeTextElement(t_Profession, contact.profession());
    writeTextElement(t_Assistant, contact.assistant());
    writeTextElement(t_Manager, contact.manager());
    writeTextElement(t_HomeWebpage, contact.homeWebpage());
    writeTextElement(t_Spouse, contact.spouse());
    writeTextElement(t_Nickname, contact.nickname());
    writeTextElement(t_Children, contact.children());

    writeDateElement(t_Birthday, contact.birthday());
    writeDateElement(t_Anniversary, contact.anniversary());

    if (!contact.portraitFile().isNull()) {
        QPixmap portrait = contact.portrait();
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        portrait.save(&buffer, "PNG");
        writeTextElement(t_Portrait, bytes.toBase64().constData());
    } else
        writeTextElement(t_Portrait, QString());

    writeTextElement(t_Notes, contact.notes());
    switch (contact.gender()) {
        case QContact::UnspecifiedGender:
            writeTextElement(t_Gender, QString());
            break;
        case QContact::Male:
            writeTextElement(t_Gender, t_Male);
            break;
        case QContact::Female:
            writeTextElement(t_Gender, t_Female);
            break;
    }
    // addresses

    writeStartElement(t_Addresses);
    QMap<QContact::Location, QContactAddress> aList = contact.addresses();
    QMapIterator<QContact::Location, QContactAddress> ait(aList);
    while (ait.hasNext()) {
        ait.next();
        QContactAddress a = ait.value();
        writeStartElement(t_Address);
        writeAttribute(t_type, ait.key() == QContact::Business ? t_Business : t_Home);
        writeTextElement(t_Street, a.street);
        writeTextElement(t_City, a.city);
        writeTextElement(t_State, a.state);
        writeTextElement(t_Zip, a.zip);
        writeTextElement(t_Country, a.country);
        writeEndElement();
    }
    writeEndElement(); // end Addresses

    // phone numbers
    QMap<QContact::PhoneType, QString> phList = contact.phoneNumbers();
    writeStartElement(t_PhoneNumbers);
    if (phList.count()) {
        writeAttribute(t_default, contact.defaultPhoneNumber());

        QMapIterator<QContact::PhoneType, QString> phit(phList);
        while (phit.hasNext()) {
            phit.next();
            QString ph = phit.value();
            writeStartElement(t_Number);
            switch(phit.key()) {
                default:
                case QContact::HomePhone:
                    writeAttribute(t_type, t_HomePhone);
                    break;
                case QContact::BusinessPhone:
                    writeAttribute(t_type, t_BusinessPhone);
                    break;
                case QContact::HomeMobile:
                    writeAttribute(t_type, t_HomeMobile);
                    break;
                case QContact::HomeFax:
                    writeAttribute(t_type, t_HomeFax);
                    break;
                case QContact::BusinessMobile:
                    writeAttribute(t_type, t_BusinessMobile);
                    break;
                case QContact::BusinessFax:
                    writeAttribute(t_type, t_BusinessFax);
                    break;
                case QContact::BusinessPager:
                    writeAttribute(t_type, t_BusinessPager);
                    break;
            }
            writeCharacters(ph);
            writeEndElement();
        }
    }
    writeEndElement(); // end PhoneNumbers

    writeStartElement(t_EmailAddresses);
    QStringList eList = contact.emailList();
    if (eList.count()) {
        writeAttribute(t_default, contact.defaultEmail());
        foreach(const QString &e, eList) {
            writeTextElement(t_Email, e);
        }
    }
    writeEndElement(); // end EmailAddresses;

    writeCategoryElements(contact.categories());
    writeCustomFieldElements(contact.customFields());

    writeEndElement();
}

void QPimXmlStreamWriter::writeTask(const QTask &task)
{
    setCategoryScope("Todo List");
    writeStartElement(t_Task);

    writeTextElement(t_Identifier, task.uid().toString());
    writeTextElement(t_Description, task.description());

    switch(task.priority()) {
        case QTask::VeryHigh:
            writeTextElement(t_Priority, t_VeryHigh);
            break;
        case QTask::High:
            writeTextElement(t_Priority, t_High);
            break;
        case QTask::Normal:
            writeTextElement(t_Priority, t_Normal);
            break;
        case QTask::Low:
            writeTextElement(t_Priority, t_Low);
            break;
        case QTask::VeryLow:
            writeTextElement(t_Priority, t_VeryLow);
            break;
    }

    switch(task.status()) {
        case QTask::NotStarted:
            writeTextElement(t_Status, t_NotStarted);
            break;
        case QTask::InProgress:
            writeTextElement(t_Status, t_InProgress);
            break;
        case QTask::Completed:
            writeTextElement(t_Status, t_Completed);
            break;
        case QTask::Waiting:
            writeTextElement(t_Status, t_Waiting);
            break;
        case QTask::Deferred:
            writeTextElement(t_Status, t_Deferred);
            break;
    }

    writeDateElement(t_DueDate, task.dueDate());
    writeDateElement(t_StartedDate, task.startedDate());
    writeDateElement(t_CompletedDate, task.completedDate());

    writeTextElement(t_PercentCompleted, QString::number(task.percentCompleted()));

    writeTextElement(t_Notes, task.notes());
    writeCategoryElements(task.categories());
    writeCustomFieldElements(task.customFields());

    writeEndElement();
}

void QPimXmlStreamWriter::writeBaseAppointmentFields(const QAppointment &appointment)
{
    writeTextElement(t_Description, appointment.description());
    writeTextElement(t_Location, appointment.location());
    writeTextElement(t_TimeZone, appointment.timeZone().id());
    writeStartElement(t_When);
    if (appointment.isAllDay()) {
        writeDateElement(t_StartDate, appointment.start().date());
        writeDateElement(t_EndDate, appointment.end().date());
    } else {
        bool utc = !appointment.timeZone().id().isEmpty();
        writeDateTimeElement(t_Start, appointment.start(), utc);
        writeDateTimeElement(t_End, appointment.end(), utc);
    }
    writeEndElement();
    writeStartElement(t_Alarm);
    if (appointment.hasAlarm()) {
        if (appointment.alarm() == QAppointment::Audible)
            writeTextElement(t_Type, t_Audible);
        else
            writeTextElement(t_Type, t_Visible);
        writeTextElement(t_Delay, QString::number(appointment.alarmDelay()));
    }
    writeEndElement();
}

void QPimXmlStreamWriter::writeAppointment(const QAppointment &appointment,
        const QList<QPimXmlException> &list)
{
    setCategoryScope("Calendar");
    writeStartElement(t_Appointment);


    writeTextElement(t_Identifier, appointment.uid().toString());
    writeBaseAppointmentFields(appointment);

    writeStartElement(t_Repeat);
    if (appointment.hasRepeat()) {
        switch(appointment.repeatRule()) {
            default:
            case QAppointment::Daily:
                writeTextElement(t_Type, t_Daily);
                break;
            case QAppointment::Weekly:
                writeTextElement(t_Type, t_Weekly);
                break;
            case QAppointment::MonthlyDate:
                writeTextElement(t_Type, t_MonthlyDate);
                break;
            case QAppointment::MonthlyDay:
                writeTextElement(t_Type, t_MonthlyDay);
                break;
            case QAppointment::MonthlyEndDay:
                writeTextElement(t_Type, t_MonthlyEndDay);
                break;
            case QAppointment::Yearly:
                writeTextElement(t_Type, t_Yearly);
                break;
        }
        writeTextElement(t_Frequency, QString::number(appointment.frequency()));
        writeDateElement(t_Until, appointment.repeatUntil());
        if (appointment.showOnNearest())
            writeBooleanElement(t_Nearest, true);
        if (appointment.weekFlags() != 0) {
            QAppointment::WeekFlags f = appointment.weekFlags();
            QStringList days;
            if (f & QAppointment::OccurMonday)
                days.append(t_Monday);
            if (f & QAppointment::OccurTuesday)
                days.append(t_Tuesday);
            if (f & QAppointment::OccurWednesday)
                days.append(t_Wednesday);
            if (f & QAppointment::OccurThursday)
                days.append(t_Thursday);
            if (f & QAppointment::OccurFriday)
                days.append(t_Friday);
            if (f & QAppointment::OccurSaturday)
                days.append(t_Saturday);
            if (f & QAppointment::OccurSunday)
                days.append(t_Sunday);
            writeTextElement(t_WeekMask, days.join(" "));
        }
        foreach(const QPimXmlException &e, list) {
            writeStartElement(t_Exception);
            writeDateElement(t_OriginalDate, e.originalDate);
            if (e.replacement) {
                writeStartElement(t_Appointment);
                writeTextElement(t_Identifier, e.appointment.uid().toString());
                writeBaseAppointmentFields(e.appointment);
                writeTextElement(t_Notes, e.appointment.notes());
                writeCategoryElements(e.appointment.categories());
                writeCustomFieldElements(e.appointment.customFields());

                writeEndElement();
            }
            writeEndElement();
        }
    }
    writeEndElement(); // end Repeat;

    writeTextElement(t_Notes, appointment.notes());
    writeCategoryElements(appointment.categories());
    writeCustomFieldElements(appointment.customFields());

    writeEndElement();
}

void QPimXmlStreamWriter::writeStartContactList()
{
    writeStartElement(t_Contacts);
}

void QPimXmlStreamReader::readStartContactList()
{
    if (readStartElement(t_Contacts))
        readNext();
    else
        raiseError("Invalid tag");
}

void QPimXmlStreamWriter::writeStartTaskList()
{
    writeStartElement(t_Tasks);
}

void QPimXmlStreamReader::readStartTaskList()
{
    if (readStartElement(t_Contacts))
        readNext();
    else
        raiseError("Invalid tag");
}

void QPimXmlStreamWriter::writeStartAppointmentList()
{
    writeStartElement(t_Appointments);
}

void QPimXmlStreamReader::readStartAppointmentList()
{
    if (readStartElement(t_Contacts))
        readNext();
    else
        raiseError("Invalid tag");
}

};

