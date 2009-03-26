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

#ifndef DOMAINMANAGER_H
#define DOMAINMANAGER_H

#include <QObject>

namespace mediaserver
{

class DomainManagerPrivate;

class DomainManager : public QObject
{
    Q_OBJECT

    friend class DomainManagerPrivate;

public:
    ~DomainManager();

    bool activateDomain(QString const& name);
    void deactivateDomain(QString const& name);

    bool isActiveDomain(QString const& name);

    QStringList activeDomains();
    QStringList inactiveDomains();

    static DomainManager* instance();

signals:
    void domainStatusChange(QStringList const& activeDomains, QStringList const& inactiveDomains);

private:
    DomainManager();

    DomainManagerPrivate*   d;
};

}   // ns mediaserver

#endif
