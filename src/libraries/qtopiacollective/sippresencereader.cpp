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

#include <QString>
#include <QDateTime>
#include <QStringList>

#include <QDebug>
#include <qtopianamespace.h>

#include "sippresencereader_p.h"

// SipPresenceReader
SipPresenceReader::SipPresenceReader(const QString &contentType, const QByteArray &data)
    : QXmlStreamReader()
      , m_contentType(contentType)
{
    // clear all
    clear();

    addData(data);

    while (!atEnd()) {
        readNext();

        if (isStartElement() && name() == "presence")
            readPresence();
    }
    // clear all
    clear();

    QString note = obtainLangNote(m_globalLangNotes);
    if (!note.isEmpty())
        m_info.setMessage(note);

    QStringList capabilities;
    QVariantMap properties;
    foreach (SipPresenceData data, m_presenceList) {
        if (!statusTypes().contains(data.status))
            continue;

        QCollectivePresenceInfo::PresenceType type = statusTypes()[data.status];

        if (m_info.presence().isEmpty() || statusTypes()[m_info.presence()] < type)
            m_info.setPresence(data.status, type);

        int colonIndex = data.uri.indexOf(':');
        if (colonIndex == -1)
            continue;

        QString uriSpec = data.uri.left(colonIndex);
        QString uri = data.uri.mid(colonIndex + 1);

        if (type != QCollectivePresenceInfo::Offline) {
            if (uriSpec == QString("im")) {
                capabilities.append(QString("IM"));
                properties.insert("InstantMessagingUri", uri);
            } else if (uriSpec == QString("tel")) {
                capabilities.append(QString("Voice"));
                properties.insert("TelephoneUri", uri);
            } else if (uriSpec == QString("mailto")) {
                capabilities.append(QString("Email"));
                properties.insert("MailtoUri", uri);
            } else if (uriSpec == QString("sip")) {
                capabilities.append(QString("Voice"));
                properties.insert("SipUri", uri);
                if (m_info.message().isEmpty()) {
                    QString note = obtainLangNote(data.langNotes);
                    if (!note.isEmpty())
                        m_info.setMessage(note);
                }
            }
        }
    }

    m_info.setCapabilities(capabilities);
    m_info.setProperties(properties);
}

SipPresenceReader::~SipPresenceReader()
{
}

QString SipPresenceReader::obtainLangNote(const QMap<QString, QString> &langNotes) const
{
    if (langNotes.size() == 0)
        return QString();

    QString note;

    QStringList languageList = Qtopia::languageList();
    bool found = false;
    foreach (QString language, languageList) {
        if (langNotes.contains(language)) {
            note = langNotes[language];
            found = true;
            break;
        }
    }

    if (!found && langNotes.contains("en"))
        note = langNotes["en"];

    return note;
}

const QMap<QString, QCollectivePresenceInfo::PresenceType> &SipPresenceReader::statusTypes()
{
    static bool first = true;
    static QMap<QString, QCollectivePresenceInfo::PresenceType> map;

    if (first) {
        map.insert("Offline", QCollectivePresenceInfo::Offline);
        map.insert("Online", QCollectivePresenceInfo::Online);
        map.insert("Idle", QCollectivePresenceInfo::Away);
        map.insert("Away", QCollectivePresenceInfo::Away);
        map.insert("Out to Lunch", QCollectivePresenceInfo::Away);
        map.insert("On the Phone", QCollectivePresenceInfo::Away);
        map.insert("Be Right Back", QCollectivePresenceInfo::Away);
        map.insert("Busy", QCollectivePresenceInfo::Busy);
    }

    return map;
}

const QCollectivePresenceInfo &SipPresenceReader::info() const
{
    return m_info;
}

void SipPresenceReader::readPresence()
{
    if (m_contentType == "application/pidf+xml") {
        QXmlStreamAttributes attrs = attributes();
        QString uri = attrs.value( "entity" ).toString();
        if (uri.startsWith( "pres:")) {
            uri = uri.remove("pres:");
            if (uri.lastIndexOf( ":" ) > 0)
                uri = uri.left(uri.lastIndexOf( ":" ));
            if (uri.lastIndexOf( ";" ) > 0 )
                uri = uri.left(uri.lastIndexOf( ";" ));
            m_info.setUri(uri);
        }
    }

    // <presence> element contains 0 or more <tuple> elements
    // plus, any number of <note> elements
    // plus, any number of OPTIONAL extension elements from other namespaces
    // MUST have 'entity' attribute, entity = "pres:URL"
    // MUST contain a namespace declaration 'xmlns'
    // compliant ns is 'urn:ietf:params:xml:ns:pidf:'
    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == "presence")
            break;

        if (isStartElement()) {
            if (name() == "tuple")
                readTuple();
            else if (name() == "note")
                readNote();
            else if (name() == "person")
                readPerson();
            else if (name() == "presentity")
                readPresentity();
            else if (name() == "atom")
                readAtom();
        }
    }
}

void SipPresenceReader::readPresentity()
{
    QXmlStreamAttributes attrs = attributes();
    QString uri = attrs.value("uri").toString();

    if (m_contentType == "application/xpidf+xml") {
        if (uri.startsWith("sip:")) {
            uri = uri.remove("sip:");
            if (uri.lastIndexOf( ":" ) > 0)
                uri = uri.left(uri.lastIndexOf( ":" ));
            if (uri.lastIndexOf( ";" ) > 0 )
                uri = uri.left(uri.lastIndexOf( ";" ));
            m_info.setUri(uri);
        }
    }

    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == "presentity")
            break;
    }
}

void SipPresenceReader::readAtom()
{
    // Xpidf+xml has atom element instead of tuple
    QXmlStreamAttributes attrs = attributes();
    QString atomId = attrs.value( "id" ).toString();

    m_curPresence.id = atomId;

    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == "atom")
            break;

        if (isStartElement()) {
            if (name() == "address")
                readAddress();
        }
    }

    m_presenceList.append(m_curPresence);
    m_curPresence = SipPresenceData();
}

void SipPresenceReader::readAddress()
{
    // Read priority
    QXmlStreamAttributes attrs = attributes();
    double priority = attrs.value( "priority" ).toString().toDouble();

    if (priority < 0 || priority > 1)
        priority = 0.0;

    QString uri = attrs.value("uri").toString();

    m_curPresence.priority = priority;
    m_curPresence.uri = uri;

    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == "address")
            break;

        if (isStartElement()) {
            if (name() == "status")
                readStatusAttributes();
            else if (name() == "msnsubstatus")
                readMsnSubStatus();
            else if (name() == "note")
                readNote();
        }
    }
}

void SipPresenceReader::readStatusAttributes()
{
    QXmlStreamAttributes attrs = attributes();
    QString basic = attrs.value("status").toString();

    if (basic == "open")
        m_curPresence.status = "Online";
    else if (basic == "closed")
        m_curPresence.status = "Offline";
}

void SipPresenceReader::readMsnSubStatus()
{
    QXmlStreamAttributes attrs = attributes();
    QString substatus = attrs.value("substatus").toString();

    if (substatus == "unknown")
        m_curPresence.status = "Offline";
    else if (substatus == "away")
        m_curPresence.status = "Away";
    else if (substatus == "online")
        m_curPresence.status = "Online";
    else if (substatus == "idle")
        m_curPresence.status = "Idle";
    else if (substatus == "busy")
        m_curPresence.status = "Busy";
    else if (substatus == "berightback")
        m_curPresence.status = "Be Right Back";
    else if (substatus == "onthephone")
        m_curPresence.status = "On the Phone";
    else if (substatus == "outtolunch")
        m_curPresence.status = "Out to Lunch";
}

void SipPresenceReader::readTuple()
{
    // each <tuple> contains
    // mandatory <status>, plus optional extension elements
    // plus optional <contact> element
    // plus any number of optional <note> elements
    // plus optional <timestamp> element
    // each <tuple> MUST contain 'id' attribute, 'id' is a unique string
    // 'id' used to identify the corresponding tuple in the previously acquired presence info
    // <tuple> containing <basic> element SHOULD contain a <contact> address.

    QXmlStreamAttributes attrs = attributes();
    QString tupleId = attrs.value("id").toString();
    m_curPresence.id = tupleId;

    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == "tuple")
            break;

        if ( isStartElement() ) {
            if (name() == "status")
                readStatus();
            else if (name() == "contact")
                readContact();
            else if (name() == "note")
                readNote();
            else if (name() == "timestamp")
                readTimestamp();
        }
    }

    m_presenceList.append(m_curPresence);
    m_curPresence = SipPresenceData();
}

void SipPresenceReader::readStatus()
{
    // <status> element contains OPTIONAL <basic> element plus OPTIONAL extension elements
    // at least one child element appears in the <status> element, no need to be <basic>
    // may ignore unrecognized element unless they carry 'mustUnderstand=true' or 'mustUnderstand=1' attribute

    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == "status")
            break;

        if (isStartElement()) {
            if (name() == "basic")
                readBasicStatus();
            else if (name() == "im")
                readExtendedStatus();
            else if (name() == "location")
                readCustomProperty();
        }
    }
}

void SipPresenceReader::readBasicStatus()
{
    // <basic> contains either 'open' or 'closed'
    // 'open' indicates availability for communications e.g. instant messages

    QString basic = readElementText();

    if (basic == "open")
        m_curPresence.status = "Online";
    else if (basic == "closed")
        m_curPresence.status = "Offline";
}

void SipPresenceReader::readExtendedStatus()
{
    QString extended = readElementText();

    if (statusTypes().contains(extended))
        m_curPresence.status = extended;
}

void SipPresenceReader::readCustomProperty()
{
    QString key = name().toString();
    QString value = readElementText();

    if (!key.isEmpty() && !value.isEmpty())
        m_curPresence.customProperties.insert(key, value);
}

void SipPresenceReader::readContact()
{
    // <contact> element may have 'priority' attribute which is decimal number between 0 and 1
    // recognize at most 3 digits after the decimal point
    // higher values indicate higher priority
    // when omitted, applications must assign the contact addrss the lowest priority.
    // if the value is out of range, ignore the attribute

    if (!isStartElement() || name() != "contact")
        return;

    QXmlStreamAttributes attrs = attributes();
    double priority = attrs.value("priority").toString().toDouble();

    if (priority < 0 || priority > 1)
        priority = 0.0;

    m_curPresence.priority = priority;
    m_curPresence.uri = readElementText();
}

void SipPresenceReader::readNote()
{
    // <note> contains string value
    // may appear as a child of <presence> or a child of <tuple>
    // SHOULD NOT be used as a substitute for status
    // SHOULD have a special attribute 'xml:lang', but can be omitted if language is implied

    if (!isStartElement() || name() != "note")
        return;

    QXmlStreamAttributes attrs = attributes();
    QString lang = attrs.value("xml:lang").toString();
    if (lang.isEmpty())
        lang = "en";

    if (m_curPresence.id.isEmpty())
        m_globalLangNotes.insert(lang, readElementText());
    else
        m_curPresence.langNotes.insert(lang, readElementText());
}

void SipPresenceReader::readTimestamp()
{
    // <timestamp> contains a string the dat and time of the status change of this tuple.
    // MUST follow the IMPP datetime format RFC 3339
    // for security <timestamp> SHOULD be included in all tuples unless cannot be determined
    // PRESENTITY MUST NOT generate <presence> element containing the same timestamp.

    if (!isStartElement() || name() != "timestamp")
        return;

    QDateTime ts = QDateTime::fromString( readElementText(), Qt::ISODate );

    if (!m_info.lastUpdateTime().isValid() || (ts > m_info.lastUpdateTime()))
        m_info.setLastUpdateTime(ts);
}

void SipPresenceReader::readPerson()
{
    // <person> element is defined in RFC 4482
    while (!atEnd()) {
        readNext();

        if (isEndElement() && name() == "person")
            break;

        if (isStartElement()) {
            if (name() == "display-name")
                readDisplayName();
        }
    }
}

void SipPresenceReader::readDisplayName()
{
    if (!isStartElement() || name() != "display-name")
        return;

    QString dn = readElementText();
    m_info.setDisplayName( dn );
}
