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

#ifndef DOMAININFO_H
#define DOMAININFO_H

#include <QString>
#include <QStringList>

class DomainInfo
{
    private:
        DomainInfo();
        QStringList m_domainList;
        QStringList m_sensitiveDomains;
        QString m_defaultDomain;
        static const char *domainStrings[];
        static DomainInfo& getInstance();

public:
    static QString defaultDomain();
    static bool isDomainValid( const QString & );
    static bool hasSensitiveDomains( const QString & );
    static QString explain( const QString &dom, const QString &packageName );
    static QString getWarningResource( const QString & );
};

#endif
