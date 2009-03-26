/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Nokia gives permission to link
** the code of its release of Qt with the OpenSSL project's "OpenSSL"
** library (or modified versions of it that use the same license as the
** "OpenSSL" library), and distribute the linked executables.  You must
** comply with the GNU General Public License versions 2.0 or 3.0 in all
** respects for all of the code used other than the "OpenSSL" code.  If
** you modify this file, you may extend this exception to your version
** of the file, but you are not obligated to do so.  If you do not wish
** to do so, delete this exception statement from your version of this
** file.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/


#include "qsslkey.h"

QT_BEGIN_NAMESPACE

/*! \namespace QSsl

    \brief The QSsl namespace declares enums common to all SSL classes in QtNetwork.
    \since 4.3

    \ingroup io
    \module network
*/

/*!
    \enum QSsl::KeyType

    Describes the two types of keys QSslKey supports.
    
    \value PrivateKey A private key.
    \value PublicKey A public key.
*/

/*!
    \enum QSsl::KeyAlgorithm

    Describes the different key algorithms supported by QSslKey.

    \value Rsa The RSA algorithm.
    \value Dsa The DSA algorithm.
*/

/*!
    \enum QSsl::EncodingFormat

    Describes supported encoding formats for certificates and keys.

    \value Pem The PEM format.
    \value Der The DER format.
*/

/*!
    \enum QSsl::AlternateNameEntryType

    Describes the key types for alternate name entries in QSslCertificate.

    \value EmailEntry An email entry; the entry contains an email address that
    the certificate is valid for.

    \value DnsEntry A DNS host name entry; the entry contains a host name
    entry that the certificate is valid for. The entry may contain wildcards.

    \sa QSslCertificate::alternateSubjectNames()

*/

/*!
    \enum QSsl::SslProtocol

    Describes the protocol of the cipher.

    \value SslV3 SSLv3 - the default protocol.
    \value SslV2 SSLv2
    \value TlsV1 TLSv1
    \value UnknownProtocol The cipher's protocol cannot be determined.
    \value AnyProtocol The socket understands SSLv2, SSLv3, and TLSv1. This
    value is used by QSslSocket only.

    Note: most servers using SSL understand both versions (2 and 3),
    but it is recommended to use the latest version only for security
    reasons. However, SSL and TLS are not compatible with each other:
    if you get unexpected handshake failures, verify that you chose
    the correct setting for your protocol.
*/

QT_END_NAMESPACE
