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

#include <qserviceindicationreader.h>

/*!
    \class QServiceIndicationReader
    \inpublicgroup QtTelephonyModule

    \brief The QServiceIndicationReader class reads Service Indication (SI) documents

    The QServiceIndicationReader class extends QWbXmlReader to provide support
    for reading documents in the WAP Service Indication (SI) language.
    This language is defined in the WAP standard
    \l{http://www.openmobilealliance.org/tech/affiliates/wap/wap-167-serviceind-20010731-a.pdf}{wap-167-serviceind-20010731-a.pdf}.

    \ingroup telephony
    \sa QWbXmlReader
*/

static void set(QWbXmlTagSet& s, int i, const char* v)
{
    s[i] = v;
}

/*!
    Construct a WBXML reader and initialize it to process WAP Service
    Indication (SI) documents.
*/
QServiceIndicationReader::QServiceIndicationReader()
{
    QWbXmlTagSet tags;
    QWbXmlTagSet attrs;

    set(tags, 0x05, "si");
    set(tags, 0x06, "indication");
    set(tags, 0x07, "info");
    set(tags, 0x08, "item");

    set(attrs, 0x05, "action=signal-none");
    set(attrs, 0x06, "action=signal-low");
    set(attrs, 0x07, "action=signal-medium");
    set(attrs, 0x08, "action=signal-high");
    set(attrs, 0x09, "action=delete");
    set(attrs, 0x0A, "created");
    set(attrs, 0x0B, "href");
    set(attrs, 0x0C, "href=http://");
    set(attrs, 0x0D, "href=http://www.");
    set(attrs, 0x0E, "href=https://");
    set(attrs, 0x0F, "href=https://www.");
    set(attrs, 0x10, "si-expires");
    set(attrs, 0x11, "si-id");
    set(attrs, 0x12, "class");
    set(attrs, 0x85, ".com/");
    set(attrs, 0x86, ".edu/");
    set(attrs, 0x87, ".net/");
    set(attrs, 0x88, ".org/");

    setTagSets( tags, attrs );
}

/*!
    Destruct a WAP Service Indication reader.
*/
QServiceIndicationReader::~QServiceIndicationReader()
{
    // Nothing to do here.
}

/*!
    \reimp
*/
QString QServiceIndicationReader::resolveOpaque( const QString& attr, const QByteArray& data )
{
    if ( attr == "created" || attr == "si-expires" ) {
        QString digits;
        for ( int posn = 0; posn < data.size(); ++posn ) {
            digits += QChar('0' + ((data[posn] & 0xF0) >> 4));
            digits += QChar('0' + (data[posn] & 0x0F));
        }
        while ( digits.length() < 14 ) {
            digits += QChar('0');
        }
        return digits.mid(0, 4) + "-" + digits.mid(4, 2) + "-" +
               digits.mid(6, 2) + "T" + digits.mid(8, 2) + ":" +
               digits.mid(10, 2) + ":" + digits.mid(12, 2) + "Z";
    } else {
        return QWbXmlReader::resolveOpaque( attr, data );
    }
}
