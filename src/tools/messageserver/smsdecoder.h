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

#ifndef SMSDECODER_H
#define SMSDECODER_H
#ifndef QTOPIA_NO_SMS

#include <qstring.h>

class QSMSMessage;
class QMailMessage;
class QWbXmlReader;

class SMSDecoder
{
public:
    SMSDecoder();
    virtual ~SMSDecoder();

    // Convert an SMS application port number into a MIME type.
    // Returns null if no corresponding MIME type.
    static QString mimeTypeForPort( int port );

    // Get an SMS message decoder for a particular port number.
    // Returns null if there is no special decoding support.
    static SMSDecoder *decoder( int port );

    // Decode the contents of an SMS message into a mail message,
    // according to the rules of a port number handler.
    virtual void decode( QMailMessage& mail, const QSMSMessage& msg ) = 0;

    // Format all of the body parts in an SMS message.
    static void formatMessage( QMailMessage& mail, const QSMSMessage& msg );

};

class SMSMultipartDecoder : public SMSDecoder
{
public:
    SMSMultipartDecoder();
    virtual ~SMSMultipartDecoder();

    virtual void decode( QMailMessage& mail, const QSMSMessage& msg );
};

class SMSLogoDecoder : public SMSDecoder
{
public:
    SMSLogoDecoder( bool operatorHeader );
    virtual ~SMSLogoDecoder();

    virtual void decode( QMailMessage& mail, const QSMSMessage& msg );
private:
    bool operatorHeader;
};

class SMSWbXmlDecoder : public SMSDecoder
{
public:
    SMSWbXmlDecoder( QWbXmlReader *reader, const QString& mimeType, bool pushHeader );
    virtual ~SMSWbXmlDecoder();

    virtual void decode( QMailMessage& mail, const QSMSMessage& msg );
private:
    QWbXmlReader *reader;
    QString mimeType;
    bool pushHeader;
};

class SMSWapPushDecoder : public SMSDecoder
{
public:
    SMSWapPushDecoder();
    virtual ~SMSWapPushDecoder();

    virtual void decode( QMailMessage& mail, const QSMSMessage& msg );
};

#endif // QTOPIA_NO_SMS
#endif
