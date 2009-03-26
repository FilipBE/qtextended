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

#include "phonesecurity.h"

/* IN Phone mode...

   Handles pins for the SIM Card lock (e.g. access to the sim card, ability to make calls, etc),
   and Phone To Sim Lock (e.g. can access data on the phone while a different sim card is inserted).

   As these seem to be standard across all Ericssons at least.

   Other codes to handle might include:

   P2, (SIM Card 2 pin)
   CS, (Lock Control surface).

   Does NOT currently do the normal Security.conf password protection.

   GUI needs to change to 'list then item' selection.  hrm.
*/

// combo item to phone lock kind map.
static const char * const lockKind[] = {
    "SIM PIN",
    "PH-SIM PIN"
};

PhoneSecurity::PhoneSecurity(QObject * parent) :
    QObject(parent),
    locktype(0)
{
    pinManager = new QPinManager(QString(), this );
    connect( pinManager, SIGNAL(lockStatus(QString,bool)),
             this, SLOT(lockStatus(QString,bool)) );
    connect( pinManager, SIGNAL(setLockStatusResult(QString,bool)),
             this, SLOT(setLockStatusResult(QString,bool)) );
    connect( pinManager, SIGNAL(changePinResult(QString,bool)),
             this, SLOT(changePinResult(QString,bool)) );
}

void PhoneSecurity::setLockType(int t)
{
    locktype = t;
    pinManager->requestLockStatus(lockKind[t]);
}

void PhoneSecurity::lockStatus(const QString& type, bool enabled )
{
    if ( type == lockKind[locktype] ) {
        emit lockDone(enabled);
    }
}

void PhoneSecurity::setLockStatusResult(const QString& type, bool valid )
{
    if ( type == lockKind[0] || type == lockKind[1] ) {
        emit locked(valid);
    }
}

void PhoneSecurity::changePinResult(const QString& type, bool valid )
{
    if ( type == lockKind[0] || type == lockKind[1] ) {
        emit changed(valid);
    }
}

void PhoneSecurity::markProtected(int t, bool b, const QString& pw)
{
    locktype = t;
    pinManager->setLockStatus(lockKind[t], pw, b);
    pinManager->requestLockStatus(lockKind[t]);
}

void PhoneSecurity::changePassword(int t, const QString& old, const QString& new2)
{
    locktype = t;
    pinManager->changePin(lockKind[t], old, new2 );
}


