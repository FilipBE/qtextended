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

#include "qcollectivenamespace.h"

namespace QCollective {

static const QString tag("qcollective");

/*!
    Returns the protocol identifier value used in QCollective addresses.

    \sa decodeUri
*/
QString protocolIdentifier()
{
    return tag;
}

/*!
    Given the service name of QCollective interface \a provider, and an \a identity
    within that provider, return a URI representation suitable for passing between
    applications.  This is typically in the form of "<protocol-ID>:<provider>/<identity>".

    If either parameter is empty, an empty URI will be returned.

    \sa decodeUri, protocolIdentifier
*/
QString encodeUri(const QString& provider, const QString& identity)
{
    if (provider.isEmpty() || identity.isEmpty())
        return QString();

    return QString("%1:%2/%3").arg(tag).arg(provider).arg(identity);
}

/*!
    Given an encoded \a uri, decode the URI into a QCollective interface name (\a provider)
    and an \a identity.  This URI should be formatted in the manner of URIs returned by
    \l encodeUri.

    Returns true if successful (the URI appeared to be a QCollective URI) or false
    otherwise.

    \sa encodeUri
*/

bool decodeUri(const QString& uri, QString& provider, QString& identity)
{
    static const int tagLength = tag.length() + 1;

    // We need at least one char for each of provider and identity, plus a delimiter
    if (uri.length() >= (tagLength + 3)) {
        if (uri.startsWith(tag + ':')) {
            int slashidx = uri.indexOf('/', tagLength);
            if (slashidx != -1 && uri.length() > (slashidx + 1)) {
                provider = uri.mid(tagLength, (slashidx - tagLength));
                identity = uri.mid(slashidx + 1);
                return true;
            }
        }
    }

    return false;
}

} // namespace QCollective
