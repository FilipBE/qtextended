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

#ifndef EMAILPROPERTYSETTER_H
#define EMAILPROPERTYSETTER_H

#include <QMap>

class QMailMessage;

// An object that sets the properties of an Email object, given property information in key/value form
class EmailPropertySetter
{
public:
    typedef void (EmailPropertySetter::*SetterFunction)(const QString&);
    typedef QMap<QString, SetterFunction> PropertySetterMap;

    // Create a setter to modify the supplied email object
    explicit EmailPropertySetter(QMailMessage& email);

    // Set the email to have the supplied property
    void setProperty(const QString& key, const QString& value);

    // Set the email to have all the supplied properties
    void setProperties(const QMap<QString, QString>& properties);

private:
    void invoke(SetterFunction fn, const QString& s);

    void setFrom(const QString& s);
    void setSubject(const QString& s);
    void setDate(const QString& s);
    void setTo(const QString& s);
    void setCc(const QString& s);
    void setBcc(const QString& s);
    void setReplyTo(const QString& s);
    void setMessageId(const QString& s);
    void setInReplyTo(const QString& s);
    void setPlainTextBody(const QString& s);
    void setAttachment(const QString& s);

    static PropertySetterMap setterMapInit();

private:
    QMailMessage& target;
};

#endif

