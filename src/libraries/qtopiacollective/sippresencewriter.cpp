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

#include "sippresencewriter_p.h"
#include "sippresencereader_p.h"
#include <QCollectivePresenceInfo>
#include <QString>
#include <QBuffer>
#include <qcollectivenamespace.h>
#include <QDebug>
#include <QDateTime>

SipPresenceWriter::SipPresenceWriter()
{
}

SipPresenceWriter::~SipPresenceWriter()
{
}

SipPresenceWriter * SipPresenceWriter::instance()
{
    static SipPresenceWriter writer;
    return &writer;
}

QByteArray SipPresenceWriter::pidfData(const QCollectivePresenceInfo &localInfo)
{
    QByteArray data;
    QBuffer buffer( &data );
    buffer.open( QIODevice::WriteOnly );

    setDevice( &buffer );

    writeStartDocument( "1.0" );

    writeStartElement( "presence" );
    writeAttribute("xmlns", "urn:ietf:params:xml:ns:pidf");
    writeAttribute("xmlns:im", "urn:ietf:params:xml:ns:pidf:im");
    writeAttribute("xmlns:dm", "urn:ietf:params:xml:ns:pidf:data-model"); // RFC 4479
    writeAttribute("xmlns:c", "urn:ietf:params:xml:ns:pidf:cipid"); //RFC 4482
    writeAttribute("entity", QString("pres:") + localInfo.uri());

    writeStartElement( "tuple" );
    QString id = localInfo.uri();
    id = id.left( id.lastIndexOf( "@" ) );
    writeAttribute( "id", id );

    writeStartElement( "status" );

    if (SipPresenceReader::statusTypes().contains(localInfo.presence())) {
        if (SipPresenceReader::statusTypes()[localInfo.presence()] == QCollectivePresenceInfo::Offline) {
            writeTextElement("basic", "closed");
        } else {
            writeTextElement("basic", "open");
            writeTextElement("im:im", localInfo.presence());
        }
    } else {
        qWarning() << "Don't know the status type:" << localInfo.presence();
        writeTextElement("basic", "closed");
    }

    writeEndElement();  // end status element

    // contact details
    writeStartElement( "contact" );
    writeAttribute( "priority", "0.8" );
    QString contact = localInfo.uri();
    contact = contact.left( contact.lastIndexOf( "@" ) );
    writeCharacters( "tel:" + contact );
    writeEndElement();

    // timestamp element according to RFC 3339
    QDateTime utcTime = QDateTime::currentDateTime().toUTC();
    writeTextElement( "timestamp",
            utcTime.date().toString( "yyyy-MM-dd" ) + "T" +
            utcTime.time().toString( "HH:mm:ss" ) + "Z" );

    writeEndElement();

    writeStartElement( "dm:person" );
    writeAttribute("id", "p1");
    writeTextElement( "c:display-name", localInfo.displayName() );
    writeEndElement();  // end dm element

    writeEndElement();  // end presence element
    writeEndDocument();

    return data;
}

QByteArray SipPresenceWriter::xpidfData(const QCollectivePresenceInfo &localInfo)
{
    QByteArray data;
    QBuffer buffer( &data );
    buffer.open( QIODevice::WriteOnly );

    setDevice( &buffer );

    writeStartDocument("1.0");
    writeDTD( "<!DOCTYPE presence PUBLIC \"-//IETF//DTD RFCxxxx XPIDF 1.0//EN\" \"xpidf.dtd\">" );

    writeStartElement("presence");

    writeStartElement("presentity");
    writeAttribute("uri", QString("sip:") + localInfo.uri() + ":5060;method=SUBSCRIBE");
    writeEndElement();

    writeStartElement( "atom" );
    QString id = localInfo.uri();
    id = id.left( id.lastIndexOf( "@" ) );
    writeAttribute( "id", id );

    writeStartElement( "address" );
    writeAttribute( "uri", QString("sip:") + localInfo.uri() + ";user=ip" );
    writeAttribute( "priority", "0.800000" );

    writeStartElement( "status" );

    if (SipPresenceReader::statusTypes().contains(localInfo.presence())) {
        if (SipPresenceReader::statusTypes()[localInfo.presence()] == QCollectivePresenceInfo::Offline) {
            writeAttribute("status", "closed");
        } else {
            writeAttribute("status", "open");
            writeEndElement();

            writeStartElement( "msnsubstatus" );

            QString substatus;
            if (localInfo.presence() == "Away")
                substatus = "away";
            else if (localInfo.presence() == "Online")
                substatus = "online";
            else if (localInfo.presence() == "Idle")
                substatus = "idle";
            else if (localInfo.presence() == "Busy")
                substatus = "busy";
            else if (localInfo.presence() == "Be Right Back")
                substatus = "berightback";
            else if (localInfo.presence() == "On the Phone")
                substatus = "onthephone";
            else if (localInfo.presence() == "Out to Lunch")
                substatus = "outtolunch";

            writeAttribute("substatus", substatus);
        }
    } else {
        qWarning() << "Don't know the status type:" << localInfo.presence();
        writeAttribute("status", "closed");
    }

    writeEndElement(); 

    writeEndElement();  // end address element
    writeEndElement();  // end atom element
    writeEndElement();  // end presence element

    return data;
}
