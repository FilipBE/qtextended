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

#ifndef TELEPATHYNAMESPACE_H
#define TELEPATHYNAMESPACE_H

#include <QtGlobal>
#include <QMetaType>
#include <QFlags>

#ifndef Q_QDOC
namespace Telepathy
{
#else
class Telepathy
{
public:
#endif
    enum Error {
        NoError,
        DisconnectedError,
        NetworkError,
        NotImplementedError,
        NotAvailableError,
        InvalidArgumentError,
        InvalidHandleError,
        ChannelBannedError,
        ChannelFullError,
        ChannelInviteOnlyError,
        PermissionDeniedError,
    };

    enum HandleType {
        None = 0,
        Contact,
        Room,
        List,
        Group
    };
};

#endif
