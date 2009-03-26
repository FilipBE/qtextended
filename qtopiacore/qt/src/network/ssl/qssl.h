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


#ifndef QSSL_H
#define QSSL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Network)

namespace QSsl {
    enum KeyType {
        PrivateKey,
        PublicKey
    };

    enum EncodingFormat {
        Pem,
        Der
    };

    enum KeyAlgorithm {
        Rsa,
        Dsa
    };

    enum AlternateNameEntryType {
        EmailEntry,
        DnsEntry
    };

    enum SslProtocol {
        SslV3,
        SslV2,
        TlsV1,
        AnyProtocol,
        UnknownProtocol = -1
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSSL_H
