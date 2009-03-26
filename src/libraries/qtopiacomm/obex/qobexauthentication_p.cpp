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
#include "qobexauthentication_p.h"

#include <QFile>
#include <QCryptographicHash>
#include <QDateTime>

/*!
    \internal
    Stores a randomly generated nonce into \a buf.
*/
void QObexAuth::generateNonce(QByteArray &buf)
{
    QFile f("/dev/urandom");
    f.open(QFile::ReadOnly);
    QByteArray random(f.read(16));
    f.close();

    buf = QCryptographicHash::hash(
        random + ':' + QByteArray::number(QDateTime::currentDateTime().toTime_t()),
        QCryptographicHash::Md5);
}
