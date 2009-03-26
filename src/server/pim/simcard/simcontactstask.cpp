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
#include "simcontactstask.h"

#include <qtopiaserverapplication.h>
#include <private/qsimsync_p.h>
#include <QPhoneBook>

/*
   Syncs the contacts from the sim card into the database.
   Only attempts this once, not per contact model construction.
*/

SimContactsTask::SimContactsTask(QObject *parent)
: QObject(parent)
{
    sm = new QContactSimSyncer("SM", this);
    sn = new QContactSimSyncer("SN", this);
}

SimContactsTask::~SimContactsTask()
{
}

QTOPIA_TASK(SimContacts, SimContactsTask);
