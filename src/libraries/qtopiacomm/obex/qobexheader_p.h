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

#ifndef QOBEXHEADER_P_H
#define QOBEXHEADER_P_H

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

#include <qobexheader.h>

#include <QHash>
#include <QDateTime>

typedef void* obex_t;
typedef void* obex_object_t;

/*!
    \internal
    \class QObexHeaderPrivate
    \inpublicgroup QtBaseModule
    Private implementation class for QObexHeader.
*/

class QTOPIA_AUTOTEST_EXPORT QObexHeaderPrivate
{
public:
    enum HeaderEncoding {
        HeaderEncodingMask = 0xc0,
        HeaderUnicodeEncoding = 0x00,
        HeaderByteSequenceEncoding = 0x40,
        HeaderByteEncoding = 0x80,
        HeaderIntEncoding = 0xc0
    };

    QObexHeaderPrivate();
    ~QObexHeaderPrivate();

    void setValue(int headerId, const QVariant &data);
    QVariant value(int headerId) const;
    bool remove(int headerId);
    bool contains(int headerId) const;
    void clear();

    QList<int> keys() const;
    int size() const;

    QList<int> m_ids;
    QHash<int, QVariant> m_hash;
    QByteArray m_challengeNonce;

    inline static QByteArray getChallengeNonce(const QObexHeader &header) { return header.m_data->m_challengeNonce; }

    static void dateTimeFromString(QDateTime &dateTime, const QString &timeString);
    static void stringFromDateTime(QString &timeString, const QDateTime &dateTime);

    static void unicodeBytesFromString(QByteArray &bytes, const QString &s);
    static void stringFromUnicodeBytes(QString &string, const uchar *data, uint size);

    static bool readOpenObexHeaders(QObexHeader &header, obex_t* handle, obex_object_t *obj);
    static bool writeOpenObexHeaders(obex_t* handle, obex_object_t *obj, bool fitOnePacket, const QObexHeader &header);

    static bool requestShouldFitOnePacket(QObex::Request request);

    static QString headerIdToString(int id);
};

#endif
