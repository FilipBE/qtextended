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

#ifndef PHONESERVER_H
#define PHONESERVER_H

#include <QObject>
#include <QTelephonyService>
#include "qtopiaserverapplication.h"

class QValueSpaceObject;

class PhoneServer : public QObject
{
    Q_OBJECT
public:
    PhoneServer( QObject *parent = 0 );
    ~PhoneServer();

private:
    QValueSpaceObject *status;
};

/*
   Internal interface for backwards compatibility.
   New TelephonyServices should be made available via service infrastructure.
   An example would be the sipagent/VoIP service.
   */
class TelephonyServiceFactory : public QObject
{
    Q_OBJECT
public:
    virtual QTelephonyService* service() = 0;
    virtual QByteArray serviceName() const = 0;
};
QTOPIA_TASK_INTERFACE( TelephonyServiceFactory );

#endif
