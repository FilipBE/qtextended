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

#include <qserviceloadingreader.h>

/*!
    \class QServiceLoadingReader
    \inpublicgroup QtTelephonyModule

    \brief The QServiceLoadingReader class reads Service Loading (SL) documents

    The QServiceLoadingReader class extends QWbXmlReader to provide support
    for reading documents in the WAP Service Loading (SL) language.
    This language is defined in the WAP standard
    \l{http://www.openmobilealliance.org/tech/affiliates/wap/wap-168-serviceload-20010731-a.pdf}{wap-168-serviceload-20010731-a.pdf}.

    \ingroup telephony
    \sa QWbXmlReader
*/

static void set(QWbXmlTagSet& s, int i, const char* v)
{
    s[i] = v;
}

/*!
    Construct a WBXML reader and initialize it to process WAP Service
    Loading (SL) documents.
*/
QServiceLoadingReader::QServiceLoadingReader()
{
    QWbXmlTagSet tags;
    QWbXmlTagSet attrs;

    set(tags, 0x05, "sl");

    set(attrs, 0x05, "action=execute-low");
    set(attrs, 0x06, "action=execute-high");
    set(attrs, 0x07, "action=cache");
    set(attrs, 0x08, "href");
    set(attrs, 0x09, "href=http://");
    set(attrs, 0x0A, "href=http://www.");
    set(attrs, 0x0B, "href=https://");
    set(attrs, 0x0C, "href=https://www.");
    set(attrs, 0x85, ".com/");
    set(attrs, 0x86, ".edu/");
    set(attrs, 0x87, ".net/");
    set(attrs, 0x88, ".org/");

    setTagSets( tags, attrs );
}

/*!
    Destruct a WAP Service Loading reader.
*/
QServiceLoadingReader::~QServiceLoadingReader()
{
    // Nothing to do here.
}
