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

#ifndef QOBEXAUTHENTICATIONCHALLENGE_H
#define QOBEXAUTHENTICATIONCHALLENGE_H

#include <qobexnamespace.h>
#include <qobexglobal.h>

class QObexAuthenticationChallengePrivate;

class QTOPIAOBEX_EXPORT QObexAuthenticationChallenge
{
public:
    QObexAuthenticationChallenge();
    QObexAuthenticationChallenge(const QObexAuthenticationChallenge &other);
    ~QObexAuthenticationChallenge();

    QObexAuthenticationChallenge &operator=(const QObexAuthenticationChallenge &other);

    void setUser(const QString &userName);
    QString user() const;

    void setPassword(const QString &password);
    QString password() const;

    QObex::AuthChallengeOptions options() const;
    QString realm() const;

private:
    friend class QObexAuthenticationChallengePrivate;
    QObexAuthenticationChallengePrivate *m_data;
};

#endif
