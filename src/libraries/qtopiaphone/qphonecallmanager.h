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
#ifndef QPHONECALLMANAGER_H
#define QPHONECALLMANAGER_H

#include <qphonecall.h>

class QPhoneCallManagerPrivate;

class QTOPIAPHONE_EXPORT QPhoneCallManager : public QObject
{
    Q_OBJECT
public:
    explicit QPhoneCallManager( QObject *parent = 0 );
    ~QPhoneCallManager();

    QList<QPhoneCall> calls() const;

    QPhoneCall create( const QString& type );
    QPhoneCall create( const QString& type, const QString& service );

    QStringList services() const;
    QStringList services( const QString& type ) const;

    QStringList callTypes() const;
    QStringList callTypes( const QString& service ) const;

    QPhoneCall fromModemIdentifier( int id ) const;

signals:
    void newCall( const QPhoneCall& call );
    void callTypesChanged();
    void statesChanged( const QList<QPhoneCall>& calls );

private:
    QPhoneCallManagerPrivate *d;
};

#endif
