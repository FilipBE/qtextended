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

#ifndef QOBEXNAMESPACE_H
#define QOBEXNAMESPACE_H

#include <qobexglobal.h>

#ifndef Q_QDOC
namespace QObex
{
#else
class QObex
{
public:
#endif

    enum Request {
        Connect,
        Disconnect,
        Put,
        PutDelete,
        Get,
        SetPath,
        NoRequest = 100
    };

    enum ResponseCode {
        Success = 0x20,
        Created = 0x21,
        Accepted = 0x22,
        NonAuthoritative = 0x23,
        NoContent = 0x24,
        ResetContent = 0x25,
        PartialContent = 0x26,

        MultipleChoices = 0x30,
        MovedPermanently = 0x31,
        MovedTemporarily = 0x32,
        SeeOther = 0x33,
        NotModified = 0x34,
        UseProxy = 0x35,

        BadRequest = 0x40,
        Unauthorized = 0x41,
        PaymentRequired = 0x42,
        Forbidden = 0x43,
        NotFound = 0x44,
        MethodNotAllowed = 0x45,
        NotAcceptable = 0x46,
        ProxyAuthenticationRequired = 0x47,
        RequestTimeOut = 0x48,
        Conflict = 0x49,
        Gone = 0x4a,
        LengthRequired = 0x4b,
        PreconditionFailed = 0x4c,
        RequestedEntityTooLarge = 0x4d,
        RequestedUrlTooLarge = 0x4e,
        UnsupportedMediaType = 0x4f,

        InternalServerError = 0x50,
        NotImplemented = 0x51,
        BadGateway = 0x52,
        ServiceUnavailable = 0x53,
        GatewayTimeout = 0x54,
        HttpVersionNotSupported = 0x55,

        DatabaseFull = 0x60,
        DatabaseLocked = 061
    };

    enum SetPathFlag {
        BackUpOneLevel = 0x1,
        NoPathCreation = 0x2
    };
    Q_DECLARE_FLAGS(SetPathFlags, SetPathFlag)

    enum AuthChallengeOption {
        UserIdRequired = 0x01,
        ReadOnlyAccess = 0x2
    };
    Q_DECLARE_FLAGS(AuthChallengeOptions, AuthChallengeOption)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QObex::SetPathFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(QObex::AuthChallengeOptions);

#endif
