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


#ifndef QSSLKEY_P_H
#define QSSLKEY_P_H

#include "qsslkey.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qsslcertificate.cpp.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include <openssl/rsa.h>
#include <openssl/dsa.h>

QT_BEGIN_NAMESPACE

class QSslKeyPrivate
{
public:
    inline QSslKeyPrivate()
        : rsa(0)
        , dsa(0)
    {
        clear();
        ref = 1;
    }

    inline ~QSslKeyPrivate()
    { clear(); }

    void clear(bool deep = true);

    void decodePem(const QByteArray &pem, const QByteArray &passPhrase,
                   bool deepClear = true);
    QByteArray pemHeader() const;
    QByteArray pemFooter() const;
    QByteArray pemFromDer(const QByteArray &der) const;
    QByteArray derFromPem(const QByteArray &pem) const;

    bool isNull;
    QSsl::KeyType type;
    QSsl::KeyAlgorithm algorithm;
    RSA *rsa;
    DSA *dsa;

    QAtomicInt ref;
};

QT_END_NAMESPACE

#endif // QSSLKEY_P_H
