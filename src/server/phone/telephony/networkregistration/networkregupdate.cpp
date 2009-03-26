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
#include "networkregupdate.h"

#include <qtopialog.h>

#include "qtopiaserverapplication.h"
#include "qabstractcallpolicymanager.h"
#include "messageboard.h"

/*!
    \class NetworkRegistrationUpdate
    \inpublicgroup QtTelephonyModule
    \brief The NetworkRegistrationUpdate class provides notifications for the user when
    the network registration of any network (e.g. VoIP or Cell) changes.
    \ingroup QtopiaServer::Telephony

    The notifications are posted to the MessageBoard. Therefore any component that displays
    the message board content will receive the registration updates.

    This class is a Qt Extended server task and cannot be used by other Qt Extended applications.

    \sa MessageBoard
*/

/*!
    Creates a new NetworkRegistrationUpdate instance with the given \a parent.
*/
NetworkRegistrationUpdate::NetworkRegistrationUpdate(QObject *parent)
    : QObject(parent), registrationMsgId(-1), msgBoard(0)
{
    // Hook onto registration state changes for all call policy managers.
    // The standard ones are CellModemManager and VoIPManager.
    QList<QAbstractCallPolicyManager *> managers;
    managers = qtopiaTasks<QAbstractCallPolicyManager>();
    foreach (QAbstractCallPolicyManager *manager, managers) {
        connect(manager,
                SIGNAL(registrationChanged(QTelephony::RegistrationState)),
                this,
                SLOT(registrationChanged()));
    }

    msgBoard = qtopiaTask<MessageBoard>();
    if ( !msgBoard )
        qLog(Component) << "NetworkRegistrationUpdate: Messageboard not available";

    registrationChanged();
}

void NetworkRegistrationUpdate::registrationChanged()
{

    QList<QAbstractCallPolicyManager *> managers;
    QString pix;
    QString msg;

    managers = qtopiaTasks<QAbstractCallPolicyManager>();
    foreach (QAbstractCallPolicyManager *manager, managers) {
        if (pix.isEmpty())
            pix = manager->registrationIcon();
        QString newMsg = manager->registrationMessage();
        if (!newMsg.isEmpty()) {
            if (!msg.isEmpty())
                msg += "<br>";
            msg.append(newMsg);
        }
    }

    if (registrationMsgId > -1  && msgBoard ) {
        msgBoard->clearMessage(registrationMsgId);
        registrationMsgId = -1;
    }

    if (!msg.isEmpty() && msgBoard)
        registrationMsgId = msgBoard->postMessage(pix, msg);
}

QTOPIA_TASK(NetworkRegistrationUpdate,NetworkRegistrationUpdate);
