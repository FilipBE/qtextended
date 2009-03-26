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

#ifndef QOBEXAUTHENTICATIONCHALLENGE_P_H
#define QOBEXAUTHENTICATIONCHALLENGE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qobexauthenticationchallenge.h>
#include <QObject>

class QTOPIA_AUTOTEST_EXPORT QObexAuthenticationChallengePrivate
{
public:
    QString m_user;
    QString m_password;
    QObex::AuthChallengeOptions m_options;
    QString m_realm;
    QByteArray m_nonce;
    bool m_modified; // whether user/password have been set at all

    inline static const QObexAuthenticationChallengePrivate *getPrivate(const QObexAuthenticationChallenge &challenge) { return challenge.m_data; }

    static bool parseRawChallenge(const QByteArray &bytes,
                                  QObexAuthenticationChallenge &challenge);

    static void writeRawChallenge(const QByteArray &nonce,
                                  QObex::AuthChallengeOptions options,
                                  const QString &realm,
                                  QByteArray &dest);

    bool toRawResponse(QByteArray &dest,
                       const QByteArray &nonce = QByteArray()) const;
};

#endif
