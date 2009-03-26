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

#ifndef QOBEXAUTHENTICATION_P_H
#define QOBEXAUTHENTICATION_P_H

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

#include <qobexglobal.h>

#include <QByteArray>

class QTOPIA_AUTOTEST_EXPORT QObexAuth
{
public:
    enum DigestChallengeTag
    {
        ChallengeNonceTag = 0x00,
        OptionsTag = 0x01,
        RealmTag = 0x02
    };

    enum DigestResponseTag
    {
        RequestDigestTag = 0x00,
        UserIdTag = 0x01,
        ResponseNonceTag = 0x02
    };

    enum RealmCharSetCode
    {
        CharSetAscii = 0,
        CharSetISO8859_1 = 1,
        CharSetISO8859_2 = 2,
        CharSetISO8859_3 = 3,
        CharSetISO8859_4 = 4,
        CharSetISO8859_5 = 5,
        CharSetISO8859_6 = 6,
        CharSetISO8859_7 = 7,
        CharSetISO8859_8 = 8,
        CharSetISO8859_9 = 9,
        CharSetUnicode = 0xFF
    };

    enum TagValueSize
    {
        NonceSize = 16,
        OptionsSize = 1,
        RequestDigestSize = 16
    };

    static void generateNonce(QByteArray &buf);
};

#endif
