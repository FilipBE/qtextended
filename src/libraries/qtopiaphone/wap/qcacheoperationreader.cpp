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

#include <qcacheoperationreader.h>

/*!
    \class QCacheOperationReader
    \inpublicgroup QtTelephonyModule

    \brief The QCacheOperationReader class reads WAP Cache Operation (CO) documents

    The QCacheOperationReader class extends QWbXmlReader to provide support
    for reading documents in the WAP Cache Operation (CO) language.
    This language is defined in the WAP standard
    \l{http://www.openmobilealliance.org/tech/affiliates/wap/wap-175-cacheop-20010731-a.pdf}{wap-175-cacheop-20010731-a.pdf}.

    \ingroup telephony
    \sa QWbXmlReader
*/

static void set(QWbXmlTagSet& s, int i, const char* v)
{
    s[i] = v;
}

/*!
    Construct a WBXML reader and initialize it to process WAP Cache
    Operation (CO) documents.
*/
QCacheOperationReader::QCacheOperationReader()
{
    QWbXmlTagSet tags;
    QWbXmlTagSet attrs;

    set(tags, 0x05, "co");
    set(tags, 0x06, "invalidate-object");
    set(tags, 0x07, "invalidate-service");

    set(attrs, 0x05, "uri");
    set(attrs, 0x06, "uri=http://");
    set(attrs, 0x07, "uri=http://www.");
    set(attrs, 0x08, "uri=https://");
    set(attrs, 0x09, "uri=https://www.");
    set(attrs, 0x85, ".com/");
    set(attrs, 0x86, ".edu/");
    set(attrs, 0x87, ".net/");
    set(attrs, 0x88, ".org/");

    setTagSets( tags, attrs );
}

/*!
    Destruct a WAP Service Loading reader.
*/
QCacheOperationReader::~QCacheOperationReader()
{
    // Nothing to do here.
}
