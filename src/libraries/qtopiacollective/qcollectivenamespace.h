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

#ifndef QCOLLECTIVENAMESPACE_H
#define QCOLLECTIVENAMESPACE_H

#include <qtopiaglobal.h>
#include <qtopiaipcmarshal.h>

#ifndef Q_QDOC
// syncqtopia header QCollective
namespace QCollective
{
#else
class QCollective
{
public:
#endif
    QTOPIACOLLECTIVE_EXPORT QString protocolIdentifier();
    QTOPIACOLLECTIVE_EXPORT QString encodeUri(const QString& provider, const QString& identity);
    QTOPIACOLLECTIVE_EXPORT bool decodeUri(const QString& uri, QString& provider, QString& identity);
};

#endif
