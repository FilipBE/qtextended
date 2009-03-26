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

#ifndef OBEXTESTSGLOBAL_H
#define OBEXTESTSGLOBAL_H

#include <qtopiaipcmarshal.h>
#include <qobexclientsession.h>
#include <qobexserversession.h>
#include <qobexnamespace.h>
#include <qobexheader.h>


Q_DECLARE_USER_METATYPE_ENUM(QObex::Request);
Q_DECLARE_USER_METATYPE_ENUM(QObex::ResponseCode);
Q_DECLARE_USER_METATYPE_ENUM(QObex::SetPathFlag);
Q_DECLARE_USER_METATYPE_ENUM(QObex::AuthChallengeOption);

Q_DECLARE_USER_METATYPE_ENUM(QObexHeader::HeaderId);
Q_DECLARE_USER_METATYPE_ENUM(QObexClientSession::Error);
Q_DECLARE_USER_METATYPE_ENUM(QObexServerSession::Error);


#endif

