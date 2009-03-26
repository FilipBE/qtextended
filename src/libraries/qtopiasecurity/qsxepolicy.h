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

#ifndef QSXEPOLICY_H
#define QSXEPOLICY_H

#include <QStringList>
#include <QCache>
#include <QHash>
#include <QMap>
#include <QMutex>

#include <qtopiaglobal.h>

#include <qtransportauth_qws.h>

#include <time.h>

class QTOPIASECURITY_EXPORT SXEPolicyManager : public QObject
{
    Q_OBJECT
public:
    static SXEPolicyManager *getInstance();
    ~SXEPolicyManager();
    QStringList findPolicy( unsigned char progId );
    QString findRequest( QString request, QStringList prof );
public slots:
    void policyCheck( QTransportAuth::Data &, const QString & );
    void resetDateCheck();
private:
    SXEPolicyManager();
    bool readProfiles();
    QString checkWildcards( const QString &, const QStringList & );
    QMultiHash<QString,QString> requestHash;
    QCache<unsigned char,QStringList> policyCache;
    QMap<QString,QString> wildcards;
    bool checkDate;
    QMutex policyMutex;
};

#endif
