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

#include "presenceservice.h"
#include "messageboard.h"
#include "qabstractcallpolicymanager.h"
#include "qtopiaserverapplication.h"
#include "uifactory.h"

#include <QCollective>

#include <QValueSpaceItem>

#include <QCollectivePresenceInfo>

/*!
    \service PresenceService Presence
    \inpublicgroup QtIPCommsModule
    \brief The PresenceService class provides the Presence service.

    The \i Presence service enables applications to modify the local presence.
*/

/*!
    \internal
*/
PresenceService::PresenceService( QObject *parent )
    : QtopiaAbstractService( "Presence", parent )
{
    mMsgBoard = qtopiaTask<MessageBoard>();
    if ( !mMsgBoard )
        qLog(Component) << "PresenceService: Messageboard not available";

    mVsItem = new QValueSpaceItem("/Telephony/Status/VoIP/Presence/Local/Presence", this);
    connect(mVsItem, SIGNAL(contentsChanged()), this, SLOT(updateDndMessage()));
    publishAll();
}

/*!
    \internal
*/
PresenceService::~PresenceService()
{
}

/*!
    Toggles between Do not disturb state and online state.
    If the current presence is available, change it to away.
    It also updates the profile to the silence mode and direct calls to the voice mail.

    This slot corresponds to the QCop service message
    \c{Presence::dnd()}.
*/
void PresenceService::dnd()
{
    static QList<QAbstractCallPolicyManager *> managers = qtopiaTasks<QAbstractCallPolicyManager>();

    foreach( QAbstractCallPolicyManager* man, managers ) {
        man->doDnd();
    }
}

/*!
    Shows options to edit local presence status.

    This slot is corresponds to the QCop service message
    \c{Presence::editPresence()}.
*/
void PresenceService::editPresence()
{
    QDialog* editor = UIFactory::createDialog("PresenceEditor");
    if ( !editor ) {
        qLog(Component) << "PhoneLauncher: PresenceEditor component not available. Aborting call.";
        return;
    }
    QtopiaApplication::execDialog(editor);
    delete editor;
}

/*!
    \internal
*/
void PresenceService::updateDndMessage()
{
    if (!mMsgBoard) return;

    static int dndMsgId = -1;
    if ( dndMsgId > -1 ) {
        mMsgBoard->clearMessage( dndMsgId );
        dndMsgId = -1;
    }

    if (static_cast<QCollectivePresenceInfo::PresenceType>(mVsItem->value().toInt()) == QCollectivePresenceInfo::Busy)
        dndMsgId = mMsgBoard->postMessage( QString(), "Do Not Disturb" );
}

QTOPIA_TASK(PresenceService,PresenceService);
