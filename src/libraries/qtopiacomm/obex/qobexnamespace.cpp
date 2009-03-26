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
#include <qobexnamespace.h>

/*!
    \class QObex
    \inpublicgroup QtBaseModule

    \brief The QObex namespace contains miscellaneous OBEX functionality.

    The QObex namespace defines various functions and enums
    that are used globally by the OBEX library.
*/

/*!
    \enum QObex::Request
    Defines the types of requests that can be made by an OBEX client to an
    OBEX server.

    \value Connect Initiate the OBEX session.
    \value Disconnect Close the OBEX session.
    \value Put Send an object.
    \value PutDelete Delete an object.
    \value Get Get an object.
    \value SetPath Change the current path on the OBEX server.
    \value NoRequest
*/

/*!
    \enum QObex::ResponseCode
    Defines the response codes that may be sent by an OBEX server in response
    to a client request.

    \value Success OK, Success
    \value Created Created
    \value Accepted Accepted
    \value NonAuthoritative Non-authoritative information
    \value NoContent No content
    \value ResetContent Reset content
    \value PartialContent Partial content

    \value MultipleChoices Multiple choices
    \value MovedPermanently Moved permanently
    \value MovedTemporarily Moved temporarily
    \value SeeOther See other
    \value NotModified Not modified
    \value UseProxy Use proxy

    \value BadRequest Bad request - server couldn't understand request
    \value Unauthorized Unauthorized
    \value PaymentRequired Payment required
    \value Forbidden Forbidden - operation is understood but refused
    \value NotFound Not found
    \value MethodNotAllowed Method not allowed
    \value NotAcceptable Not acceptable
    \value ProxyAuthenticationRequired Proxy authentication required
    \value RequestTimeOut Request time out
    \value Conflict Conflict
    \value Gone Gone
    \value LengthRequired Length required
    \value PreconditionFailed Precondition failed
    \value RequestedEntityTooLarge Requested entity too large
    \value RequestedUrlTooLarge Request URL too large
    \value UnsupportedMediaType Unsupported media type

    \value InternalServerError Internal server error
    \value NotImplemented Not implemented
    \value BadGateway Bad Gateway
    \value ServiceUnavailable Service unavailable
    \value GatewayTimeout Gateway timeout
    \value HttpVersionNotSupported HTTP version not supported

    \value DatabaseFull Database full
    \value DatabaseLocked Database locked
*/

/*!
    \enum QObex::SetPathFlag
    Defines the option flags that can be used for an OBEX \c {Set Path}
    operation. An OBEX client can use these options to specify a
    \c {Set Path} request in greater detail.

    \value BackUpOneLevel The OBEX server should go back up one directory level before applying the path.
    \value NoPathCreation The OBEX server should \i not automatically create the path if it does not exist.

    \sa QObexClientSession::setPath(), QObexServerSession::setPath()
 */

/*!
    \enum QObex::AuthChallengeOption
    Defines the option flags that can be used in an OBEX authentication
    challenge.

    \value UserIdRequired The party receiving the authentication challenge must provide a user ID value in its authentication response.
    \value ReadOnlyAccess The sender of the authentication challenge will offer read-only access to its data. If this option is not set, the sender will permit full access (both read and write).
 */
