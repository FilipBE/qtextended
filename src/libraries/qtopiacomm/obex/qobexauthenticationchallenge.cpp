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
#include "qobexauthenticationchallenge_p.h"
#include "qobexauthentication_p.h"
#include "qobexheader_p.h"
#include <qobexheader.h>

#include <QFile>
#include <QCryptographicHash>
#include <QTextCodec>


/*!
    \class QObexAuthenticationChallenge
    \inpublicgroup QtBaseModule
    \brief The QObexAuthenticationChallenge class contains a received OBEX authentication challenge and allows the receiver to set a username and password that should be used for authentication.

    This class is used in the QObexClientSession::authenticationRequired() and
    QObexServerSession::authenticationRequired() signals to pass on the
    data from a received OBEX authentication challenge. The programmer can
    then call setUser() and setPassword() to specify the username and
    password that should be returned in the authentication response.

    \sa QObexClientSession, QObexServerSession, QObexAuthenticationResponse
*/


/*!
    Constructs an empty authentication challenge.
*/
QObexAuthenticationChallenge::QObexAuthenticationChallenge()
    : m_data(new QObexAuthenticationChallengePrivate)
{
    m_data->m_modified = false;
}

/*!
    Constructs an authentication challenge with the contents from \a other.
*/
QObexAuthenticationChallenge::QObexAuthenticationChallenge(const QObexAuthenticationChallenge &other)
{
    operator=(other);
}

/*!
    Destroys the challenge.
*/
QObexAuthenticationChallenge::~QObexAuthenticationChallenge()
{
    delete m_data;
}

/*!
    Assigns the contents of \a other to this object and returns a reference
    to this challenge.
*/
QObexAuthenticationChallenge &QObexAuthenticationChallenge::operator=(const QObexAuthenticationChallenge &other)
{
    if (&other == this)
        return *this;

    m_data->m_user = other.m_data->m_user;
    m_data->m_password = other.m_data->m_password;
    m_data->m_options = other.m_data->m_options;
    m_data->m_realm = other.m_data->m_realm;
    m_data->m_nonce = other.m_data->m_nonce;
    m_data->m_modified = other.m_data->m_modified;
    return *this;
}

/*!
    Sets the username to be used for authentication to \a userName.

    \warning The username must not be longer than 20 characters long, or
    authentication will fail.
*/
void QObexAuthenticationChallenge::setUser(const QString &userName)
{
    m_data->m_user = userName;
    m_data->m_modified = true;
}

/*!
    Returns the username to be used for authentication.
*/
QString QObexAuthenticationChallenge::user() const
{
    return m_data->m_user;
}

/*!
    Sets the password to be used for authentication to \a password.
*/
void QObexAuthenticationChallenge::setPassword(const QString &password)
{
    m_data->m_password = password;
    m_data->m_modified = true;
}

/*!
    Returns the password to be used for authentication.
*/
QString QObexAuthenticationChallenge::password() const
{
    return m_data->m_password;
}

/*!
    Returns the authentication options specified by the party that issued
    the authentication challenge.
*/
QObex::AuthChallengeOptions QObexAuthenticationChallenge::options() const
{
    return m_data->m_options;
}

/*!
    Returns the realm specified by the party that issued the authentication
    challenge. The realm is a user-displayable description that indicates
    the user and/or password to be used for authentication.
*/
QString QObexAuthenticationChallenge::realm() const
{
    return m_data->m_realm;
}


//================================================================


/*!
    \internal
    Sets \a dest to the realm string stored in raw byte form in \a data.
    The \a data should have its first byte set to the charset encoding
    for the realm. The realm string will only be set if the charset
    encoding is supported by the system.

    The \a size is the size of \a data.
*/
static bool readRealm(const uchar *data, uint size, QString &dest)
{
    if (size <= 1 )      // no data to read other than charset code
        return false;

    QTextCodec *codec = 0;

    switch (data[0]) {

    // some codecs have built in support:
    case QObexAuth::CharSetAscii:
        dest = QString::fromAscii(reinterpret_cast<const char*>(&data[1]), size-1);
        break;
    case QObexAuth::CharSetISO8859_1:
        dest = QString::fromLatin1(reinterpret_cast<const char*>(&data[1]), size-1);
        break;
    case QObexAuth::CharSetUnicode:
        QObexHeaderPrivate::stringFromUnicodeBytes(dest, &data[1], size-1);
        break;

    // go through other codecs:
    case QObexAuth::CharSetISO8859_2:
        codec = QTextCodec::codecForName("ISO-8859-2");
        break;
    case QObexAuth::CharSetISO8859_3:
        codec = QTextCodec::codecForName("ISO-8859-3");
        break;
    case QObexAuth::CharSetISO8859_4:
        codec = QTextCodec::codecForName("ISO-8859-4");
        break;
    case QObexAuth::CharSetISO8859_5:
        codec = QTextCodec::codecForName("ISO-8859-5");
        break;
    case QObexAuth::CharSetISO8859_6:
        codec = QTextCodec::codecForName("ISO-8859-6");
        break;
    case QObexAuth::CharSetISO8859_7:
        codec = QTextCodec::codecForName("ISO-8859-7");
        break;
    case QObexAuth::CharSetISO8859_8:
        codec = QTextCodec::codecForName("ISO-8859-8");
        break;
    case QObexAuth::CharSetISO8859_9:
        codec = QTextCodec::codecForName("ISO-8859-9");
        break;
    }

    if (codec)
        dest = codec->toUnicode(reinterpret_cast<const char*>(&data[1]), size-1);

    if (dest.isEmpty())
        return false;
    return true;        
}


/*!
    \internal
    Stores \a realm into \a dest according to the string encoding for
    \a realm, and returns the QObexAuth::RealmCharSetCode for the
    encoding.

    Only Unicode, Latin-1 and ASCII encodings are supported.
*/
static uchar realmBytesFromString(QByteArray &dest, const QString &realm)
{
    // Transmit everything as either Ascii, Latin1 or Unicode.

    const QChar *str = realm.constData();
    for (int i=0; i<realm.size(); i++) {
        if (str[i] > 255) {
            QObexHeaderPrivate::unicodeBytesFromString(dest, realm);
            return QObexAuth::CharSetUnicode;
        }
        if (str[i] > 127) {
            dest = realm.toLatin1();
            return QObexAuth::CharSetISO8859_1;
        }
    }

    dest = realm.toAscii();
    return QObexAuth::CharSetAscii;
}

/*!
    \internal
    Sets the contents of \a challenge according to the raw Authentication
    Challenge data read from \a bytes.
*/
bool QObexAuthenticationChallengePrivate::parseRawChallenge(const QByteArray &bytes, QObexAuthenticationChallenge &challenge)
{
    if (bytes.size() == 0)
        return false;

    const uchar *data = reinterpret_cast<const uchar *>(bytes.constData());
    uint size = bytes.size();

    uchar tagId;
    uint tagSize;
    uint i = 0;

    QObex::AuthChallengeOptions options;
    QString realm;
    QByteArray nonce;

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
        case (QObexAuth::ChallengeNonceTag):
            if (tagSize != QObexAuth::NonceSize)
                return false;
            nonce.resize(QObexAuth::NonceSize);
            memcpy(nonce.data(), &data[i], QObexAuth::NonceSize);
            break;
        case (QObexAuth::OptionsTag):
            if (tagSize != QObexAuth::OptionsSize)
                return false;
            options = static_cast<QObex::AuthChallengeOptions>(data[i]);
            break;
        case (QObexAuth::RealmTag):
            if (!readRealm(&data[i], tagSize, realm))
                return false;
            break;
        default:
            break;
        };

        // move onto next tag id
        i += tagSize;
    }

    // if no valid nonce was provided, the whole challenge is invalid
    if (nonce.size() != int(QObexAuth::NonceSize)) 
        return false;

    challenge.m_data->m_nonce = nonce;
    challenge.m_data->m_options = options;
    challenge.m_data->m_realm = realm;
    return true;
}

/*!
    \internal
    Sets \a dest to the raw bytes of an Authentication Challenge 
    created with \a nonce, \a options, and \a realm.
*/
void QObexAuthenticationChallengePrivate::writeRawChallenge(const QByteArray &nonce, QObex::AuthChallengeOptions options, const QString &realm, QByteArray &dest)
{
    int len = 0;
    QByteArray realmBytes;
    uchar charSetEncoding = uchar(0);
    if (!realm.isEmpty())
        charSetEncoding = realmBytesFromString(realmBytes, realm);

    // Calculate size of challenge (made up of tag-length-value triplets).
    // There are 3 tags: nonce, options and realm.
    // For each tag, make room for tag size + value length size + value
    len += (1 + 1 + QObexAuth::NonceSize);        // nonce is mandatory
    if (options != 0)
        len += (1 + 1 + QObexAuth::OptionsSize);
    if (!realmBytes.isEmpty())
        len += (1 + 1 + 1 + realmBytes.size());  // extra 1 on end for charset code

    // prepare bytes
    dest.resize(len);
    uchar *buf = reinterpret_cast<uchar *>(dest.data());
    int i = 0;

    // insert nonce (0x00, 16 bytes)
    buf[i++] = QObexAuth::ChallengeNonceTag;
    buf[i++] = QObexAuth::NonceSize;
    memcpy(&buf[i], nonce.constData(), QObexAuth::NonceSize);
    i += QObexAuth::NonceSize;

    // insert options (0x01, 1 byte)
    if (options != 0) {
        buf[i++] = QObexAuth::OptionsTag;
        buf[i++] = QObexAuth::OptionsSize;
        buf[i++] = options;
    }

    // insert options (0x02, n bytes)
    if (!realmBytes.isEmpty()) {
        buf[i++] = QObexAuth::RealmTag;
        buf[i++] = realmBytes.size() + 1;      // +1 for charset
        buf[i++] = charSetEncoding;
        memcpy(&buf[i], realmBytes.constData(), realmBytes.size());
        i += realmBytes.size();
    }
}


bool QObexAuthenticationChallengePrivate::toRawResponse(QByteArray &dest, const QByteArray &nonce) const
{
    QByteArray digest = QCryptographicHash::hash(
                    m_nonce + ':' + m_password.toLatin1(),
                    QCryptographicHash::Md5);
    if (digest.isEmpty())
        return false;

    QByteArray userBytes = m_user.toLatin1();
    if (userBytes.size() > 20) 
        return false;

    if (!nonce.isEmpty() && nonce.size() != QObexAuth::NonceSize)
        return false;

    int len = 0;
    len += (1 + 1 + QObexAuth::RequestDigestSize);   // request digest is mandatory
    if (!m_user.isEmpty())
        len += (1 + 1 + userBytes.size());
    if (!nonce.isEmpty())
        len += (1 + 1 + QObexAuth::NonceSize);

    dest.resize(len);
    uchar *buf = reinterpret_cast<uchar *>(dest.data());
    int i = 0;

    // insert digest
    buf[i++] = QObexAuth::RequestDigestTag;
    buf[i++] = QObexAuth::RequestDigestSize;
    memcpy(&buf[i], digest.constData(), QObexAuth::RequestDigestSize);
    i += QObexAuth::RequestDigestSize;

    // insert user id
    if (!m_user.isEmpty()) {
        buf[i++] = QObexAuth::UserIdTag;
        buf[i++] = userBytes.size();
        memcpy(&buf[i], userBytes.constData(), userBytes.size());
        i += userBytes.size();
    }

    // insert nonce
    if (!nonce.isEmpty()) {
        buf[i++] = QObexAuth::ResponseNonceTag;
        buf[i++] = QObexAuth::NonceSize;
        memcpy(&buf[i], nonce.constData(), QObexAuth::NonceSize);
        i += QObexAuth::NonceSize;
    }

    return true;
}

