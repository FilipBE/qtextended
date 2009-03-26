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

#include <qirnamespace.h>

#if defined(Q_OS_LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/irda.h>
#elif defined(Q_OS_WIN32)
#include <winsock2.h>
#define _WIN32_WINDOWS
#include <AF_Irda.h>
#endif

#include <QByteArray>
#include <QString>
#include <QVariant>
#include <QTextCodec>

using namespace QIr;

/*!
    \class QIr

    \brief The QIr namespace contains miscellaneous Infrared functionality.

    The QIr namespace defines various functions and enums
    that are used globally by the Infrared library.
*/

/*!
    \enum QIr::DeviceClass
    Defines possible DeviceTypes as defined by the IrDA standard.

    \value PlugNPlay Device supports PlugNPlay.
    \value PDA Device is a PDA.
    \value Computer Device is a Computer device.
    \value Printer Device is a printer.
    \value Modem Device is a modem.
    \value Fax Device is a fax.
    \value LAN Device is a LAN.
    \value Telephony Device is a telephony device.
    \value FileServer Device is a file server device.
    \value Communications Device is a communications device.
    \value Message Device is a messaging device.
    \value HTTP Device supports HTTP.
    \value OBEX Device supports the OBEX protocol.
    \value All Special value representing all possible service types.
 */

/*!
    \internal Converts a QIr::CharacterSet to a string useable by
    QTextCoded::codecForName
*/
QByteArray QIr::convert_charset_to_string(int set)
{
#if defined(Q_OS_LINUX)
    switch (set) {
        case CS_ASCII:
            return "ASCII";
        case CS_ISO_8859_1:
            return "ISO-8859-1";
        case CS_ISO_8859_2:
            return "ISO-8859-2";
        case CS_ISO_8859_3:
            return "ISO-8859-3";
        case CS_ISO_8859_4:
            return "ISO-8859-4";
        case CS_ISO_8859_5:
            return "ISO-8859-5";
        case CS_ISO_8859_6:
            return "ISO-8859-6";
        case CS_ISO_8859_7:
            return "ISO-8859-7";
        case CS_ISO_8859_8:
            return "ISO-8859-8";
        case CS_ISO_8859_9:
            return "ISO-8859-9";
        case CS_UNICODE:
            return "UTF-16";
        default:
            qWarning("Charset not found!");
            return QByteArray();
    };
#elif defined(Q_OS_WIN32)
    switch (set) {
        case LmCharSetASCII:
            return "ASCII";
        case LmCharSetISO_8859_1:
            return "ISO-8859-1";
        case LmCharSetISO_8859_2:
            return "ISO-8859-2";
        case LmCharSetISO_8859_3:
            return "ISO-8859-3";
        case LmCharSetISO_8859_4:
            return "ISO-8859-4";
        case LmCharSetISO_8859_5:
            return "ISO-8859-5";
        case LmCharSetISO_8859_6:
            return "ISO-8859-6";
        case LmCharSetISO_8859_7:
            return "ISO-8859-7";
        case LmCharSetISO_8859_8:
            return "ISO-8859-8";
        case LmCharSetISO_8859_9:
            return "ISO-8859-9";
        case LmCharSetUNICODE:
            return "UTF-16";
        default:
            qWarning("Charset not found!");
            return QByteArray();
    };
#else
	return QByteArray();
#endif
}

#if defined(Q_OS_LINUX)
QVariant convert_ias_entry(struct irda_ias_set &entry)
{
    // Class name and attrib name are always in ASCII
    QString className = QString::fromAscii(entry.irda_class_name);
    QString attribName = QString::fromAscii(entry.irda_attrib_name);
    switch (entry.irda_attrib_type) {
        case IAS_STRING:
        {
            QByteArray value(reinterpret_cast<const char *>(entry.attribute.irda_attrib_string.string),
                             entry.attribute.irda_attrib_string.len);
            int charset = entry.attribute.irda_attrib_string.charset;

            QString val;

            if (charset == CS_ASCII) {
                val = QString::fromAscii(value.constData());
            }
            else if (charset == CS_ISO_8859_1) {
                val = QString::fromLatin1(value.constData());
            }
            else {
                QByteArray codecName = convert_charset_to_string(charset);
                QTextCodec *codec = QTextCodec::codecForName(codecName);
                val = codec->toUnicode(value);
            }

            return QVariant::fromValue(val);
        }

        case IAS_INTEGER:
        {
            return QVariant::fromValue(static_cast<uint>(entry.attribute.irda_attrib_int));
        }

        case IAS_OCT_SEQ:
        {
            QByteArray value(reinterpret_cast<const char *>(entry.attribute.irda_attrib_octet_seq.octet_seq),
                             entry.attribute.irda_attrib_octet_seq.len);
            return QVariant::fromValue(value);
        }

        default:
            return QVariant();
    };
}
#endif

#if defined(Q_OS_WIN32)
QVariant convert_ias_entry(IAS_SET &entry)
{
    // Class name and attrib name are always in ASCII
    QString className = QString::fromAscii(entry.irdaClassName);
    QString attribName = QString::fromAscii(entry.irdaAttribName);
    switch (entry.irdaAttribType) {
        case IAS_ATTRIB_STR:
        {
            QByteArray value(reinterpret_cast<const char *>(entry.irdaAttribute.irdaAttribUsrStr.UsrStr),
                             entry.irdaAttribute.irdaAttribUsrStr.Len);
            int charset = entry.irdaAttribute.irdaAttribUsrStr.CharSet;

            QString val;

            if (charset == LmCharSetASCII) {
                val = QString::fromAscii(value.constData());
            }
            else if (charset == LmCharSetISO_8859_1) {
                val = QString::fromLatin1(value.constData());
            }
            else {
                QByteArray codecName = convert_charset_to_string(charset);
                QTextCodec *codec = QTextCodec::codecForName(codecName);
                val = codec->toUnicode(value);
            }

            return QVariant::fromValue(val);
        }

        case IAS_ATTRIB_INT:
        {
            return QVariant::fromValue(static_cast<uint>(entry.irdaAttribute.irdaAttribInt));
        }

        case IAS_ATTRIB_OCTETSEQ:
        {
            QByteArray value(reinterpret_cast<const char *>(entry.irdaAttribute.irdaAttribOctetSeq.OctetSeq),
                             entry.irdaAttribute.irdaAttribOctetSeq.Len);
            return QVariant::fromValue(value);
        }

        default:
            return QVariant();
    };
}
#endif

#if defined(Q_OS_WIN32)
// Windows defines its own set of hints, but not as many as we'd like
#define HINT_PNP		1
#define HINT_PDA		2
#define HINT_COMPUTER		4
#define HINT_PRINTER		8
#define HINT_MODEM		16
#define HINT_FAX		32
#define HINT_LAN		64
#define HINT_EXTENSION	128

#define HINT_TELEPHONY			1
#define HINT_FILE_SERVER		2
#define HINT_COMM			4
#define HINT_MESSAGE			8
#define HINT_HTTP			16
#define HINT_OBEX			32
#endif

void convert_to_hints(QIr::DeviceClasses classes, unsigned char hints[])
{
    if (classes & QIr::PlugNPlay)
        hints[0] |= HINT_PNP;
    if (classes & QIr::PDA)
        hints[0] |= HINT_PDA;
    if (classes & QIr::Computer)
        hints[0] |= HINT_COMPUTER;
    if (classes & QIr::Printer)
        hints[0] |= HINT_PRINTER;
    if (classes & QIr::Modem)
        hints[0] |= HINT_MODEM;
    if (classes & QIr::Fax)
        hints[0] |= HINT_FAX;
    if (classes & QIr::LAN)
        hints[0] |= HINT_LAN;

    if (classes & QIr::Telephony) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_TELEPHONY;
    }
    if (classes & QIr::FileServer) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_FILE_SERVER;
    }
    if (classes & QIr::Communications) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_COMM;
    }
    if (classes & QIr::Message) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_MESSAGE;
    }
    if (classes & QIr::HTTP) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_HTTP;
    }
    if (classes & QIr::OBEX) {
        hints[0] |= HINT_EXTENSION;
        hints[1] |= HINT_OBEX;
    }
}
