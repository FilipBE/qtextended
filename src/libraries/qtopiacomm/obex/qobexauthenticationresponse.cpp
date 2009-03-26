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
#include "qobexauthenticationresponse_p.h"
#include "qobexauthentication_p.h"

#include <QCryptographicHash>


/*!
    \class QObexAuthenticationResponse
    \inpublicgroup QtBaseModule
    \brief The QObexAuthenticationResponse class contains a received OBEX authentication response.

    This class is used in the QObexClientSession::authenticationResponse()
    and QObexServerSession::authenticationResponse() signals to pass
    on a OBEX authentication response that has been received by an OBEX client
    or server. The programmer can use user() and match() to determine whether
    the authentication details are correct.

    \sa QObexClientSession, QObexServerSession, QObexAuthenticationChallenge
*/

/*!
    Constructs an empty authentication response.
*/
QObexAuthenticationResponse::QObexAuthenticationResponse()
    : m_data(new QObexAuthenticationResponsePrivate)
{
}

/*!
    Constructs an authentication response with the contents from \a other.
*/
QObexAuthenticationResponse::QObexAuthenticationResponse(const QObexAuthenticationResponse &other)
{
    operator=(other);
}

/*!
    Destroys the response.
*/
QObexAuthenticationResponse::~QObexAuthenticationResponse()
{
    delete m_data;
}

/*!
    Assigns the contents of \a other to this object and returns a reference
    to this response.
*/
QObexAuthenticationResponse &QObexAuthenticationResponse::operator=(const QObexAuthenticationResponse &other)
{
    if (&other == this)
        return *this;

    m_data->m_user = other.m_data->m_user;
    return *this;
}

/*!
    Returns the username specified in the authentication response.
*/
QString QObexAuthenticationResponse::user() const
{
    return m_data->m_user;
}

/*!
    Returns whether \a password matches the password specified in the
    authentication response.
*/
bool QObexAuthenticationResponse::match(const QString &password) const
{
    QByteArray hash = QCryptographicHash::hash(
                        m_data->m_nonce + ':' + password.toLatin1(),
                        QCryptographicHash::Md5);
    return (m_data->m_requestDigest == hash);
}


//==========================================================


/*!
    \internal
    Creates an Authentication Response. The \a challengeNonce is the 16-byte
    nonce that was originally sent in the Authentication Challenge to the 
    other party, who has now replied with an Authentication Response.

    The \a challengeNonce is used to determine whether passwords given to the
    match() function are valid (i.e. whether they match the nonce sent in the
    original challenge).
*/
QObexAuthenticationResponse QObexAuthenticationResponsePrivate::createResponse(const QByteArray &challengeNonce)
{
    QObexAuthenticationResponse r;
    r.m_data->m_nonce = challengeNonce;
    return r;
}


/*!
    \internal
    Sets the contents of \a response according to the raw Authentication
    Response data read from \a bytes.

    Note that the any nonce included in the response is currently ignored
    as this does not support multiple challenge headers at the moment.
*/
bool QObexAuthenticationResponsePrivate::parseRawResponse(const QByteArray &bytes,
QObexAuthenticationResponse &response)
{
    if (bytes.size() == 0)
        return false;

    const uchar *data = reinterpret_cast<const uchar *>(bytes.constData());
    uint size = bytes.size();

    uchar tagId;
    uint tagSize;
    uint i = 0;

    QByteArray digest;
    QString user;

    while (i < size) {

        // read tag id
        tagId = data[i++];
        if (i == size)
            return false;

        // read tag length
        tagSize = data[i++];
        if (tagSize < 1)
            continue;
        if ((i + tagSize) > size)
            return false;

        // read tag value
        switch (tagId) {
        case (QObexAuth::RequestDigestTag):
            if (tagSize != QObexAuth::RequestDigestSize)
                return false;
            digest.resize(QObexAuth::RequestDigestSize);
            memcpy(digest.data(), &data[i], QObexAuth::RequestDigestSize);
            break;
        case (QObexAuth::UserIdTag):
            if (tagSize > 0) {
                user = QString::fromLatin1(
                        reinterpret_cast<const char *>(&data[i]), tagSize);
            } else {
                user = QLatin1String("");
            }
            break;
        case (QObexAuth::ResponseNonceTag):
            // Ignore the Nonce tag as we don't support multiple 
            // Authentication Challenge headers anyway (and QObexHeader
            // doesn't support duplicate headers at the moment).
            break;
        default:
            break;
        };

        // move onto next tag id
        i += tagSize;
    }

    // if no valid digest was provided, the whole challenge is invalid
    if (digest.size() != int(QObexAuth::RequestDigestSize)) 
        return false;

    response.m_data->m_requestDigest = digest;
    response.m_data->m_user = user;
    return true;
}

