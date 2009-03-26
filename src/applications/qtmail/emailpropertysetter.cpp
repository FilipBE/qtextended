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

#include "emailpropertysetter.h"
#include <QMailMessage>

#include <qmimetype.h>
#include <QMap>

EmailPropertySetter::EmailPropertySetter(QMailMessage& email) :
    target(email)
{
}

void EmailPropertySetter::setProperty(const QString& key, const QString& value)
{
    // Create a mapping of property names to functions that set that property
    static PropertySetterMap setterMap(setterMapInit());

    // If we can set this property of the email, then do so; otherwise set it as an Extra Header
    PropertySetterMap::const_iterator setterIt = setterMap.find( key.toLower() );
    if (setterIt != setterMap.end())
        invoke( setterIt.value(), value );
    else
        target.setHeaderField( key, value );
}

void EmailPropertySetter::setProperties(const QMap<QString, QString>& properties)
{
    // Set each of the properties in the map on the target email
    QMap<QString, QString>::ConstIterator it = properties.begin(), end = properties.end();
    for ( ; it != end; ++it)
        setProperty( it.key(), it.value() );
}

void EmailPropertySetter::invoke(EmailPropertySetter::SetterFunction fn, const QString& s)
{
    (this->*fn)(s);
}

void EmailPropertySetter::setFrom(const QString& s) 
{ 
    target.setFrom(QMailAddress(s)); 
}

void EmailPropertySetter::setSubject(const QString& s) 
{ 
    target.setSubject(s); 
}

void EmailPropertySetter::setDate(const QString& s) 
{ 
    target.setDate(QMailTimeStamp(s)); 
}

void EmailPropertySetter::setTo(const QString& s) 
{ 
    target.setTo(QMailAddress::fromStringList(s.split(','))); 
}

void EmailPropertySetter::setCc(const QString& s) 
{ 
    target.setCc(QMailAddress::fromStringList(s.split(','))); 
}

void EmailPropertySetter::setBcc(const QString& s) 
{ 
    target.setBcc(QMailAddress::fromStringList(s.split(','))); 
}

void EmailPropertySetter::setReplyTo(const QString& s) 
{ 
    target.setReplyTo(QMailAddress(s)); 
}

void EmailPropertySetter::setInReplyTo(const QString& s) 
{ 
    target.setInReplyTo(s); 
}

void EmailPropertySetter::setPlainTextBody(const QString& s) 
{ 
    QMailMessageContentType type("text/plain; charset=UTF-8");
    target.setBody( QMailMessageBody::fromData(s, type, QMailMessageBody::Base64) ); 
}

void EmailPropertySetter::setMessageId(const QString& s) 
{ 
    target.setHeaderField(QString("Message-ID"), s); 
}

void EmailPropertySetter::setAttachment(const QString& s)
{
    QFileInfo fi( s );

    QMailMessageContentDisposition disposition(QMailMessageContentDisposition::Attachment);
    disposition.setFilename( fi.absoluteFilePath().toLatin1() );

    QMailMessageContentType type( QMimeType( fi.absoluteFilePath() ).id().toLatin1() );
    type.setName(fi.baseName().toLatin1());

    QMailMessagePart attachmentPart;
    attachmentPart.setBody( QMailMessageBody::fromFile(fi.absoluteFilePath(), type, QMailMessageBody::Base64, QMailMessageBody::RequiresEncoding) );
    attachmentPart.setContentDisposition(disposition);

    target.appendPart( attachmentPart );
}

EmailPropertySetter::PropertySetterMap EmailPropertySetter::setterMapInit()
{
    PropertySetterMap map;

    map.insert("from", &EmailPropertySetter::setFrom);
    map.insert("subject", &EmailPropertySetter::setSubject);
    map.insert("date", &EmailPropertySetter::setDate);
    map.insert("to", &EmailPropertySetter::setTo);
    map.insert("cc", &EmailPropertySetter::setCc);
    map.insert("bcc", &EmailPropertySetter::setBcc);
    map.insert("reply-to", &EmailPropertySetter::setReplyTo);
    map.insert("message-id", &EmailPropertySetter::setMessageId);
    map.insert("in-reply-to", &EmailPropertySetter::setInReplyTo);
    map.insert("body", &EmailPropertySetter::setPlainTextBody);
    map.insert("attachment", &EmailPropertySetter::setAttachment);

    return map;
}

