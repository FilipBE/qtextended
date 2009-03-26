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

#include "instantmessageservice.h"
#include "qtopialog.h"

#include <QString>

/*!
    \service InstantMessageService InstantMessage
    \brief Provides the Qt Extended InstantMessage service.

    The \i InstantMessage service enables applications to access features of
    the system's instant messaging application.
*/

/*! \internal */
InstantMessageService::InstantMessageService(QObject* parent)
    : QtopiaAbstractService("InstantMessage", parent)
{
    publishAll();
}

/*!
    \internal
*/
InstantMessageService::~InstantMessageService()
{
}

/*!
    \fn InstantMessageService::write(const QString&)
    \internal 
*/

/*!
    Direct the \i InstantMessage service to interact with the user to compose a new
    instant message, and then, if confirmed by the user, send the message.
    The message is sent to \a uri.

    This slot corresponds to the QCop service message
    \c{InstantMessage::writeMessage(QString)}.
*/
void InstantMessageService::writeMessage(const QString &uri)
{
    qLog(Messaging) << "InstantMessageService::writeMessage(" << uri << ")";

    emit write(uri);
}

