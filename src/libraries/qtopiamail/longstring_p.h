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

#ifndef LONGSTRING_P_H
#define LONGSTRING_P_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QSharedDataPointer>
#include <qtopiaglobal.h>

class QDataStream;
class QTextStream;

class LongStringPrivate;

class QTOPIAMAIL_EXPORT LongString
{
public:
    LongString();
    LongString(const LongString &other);
    LongString(const QByteArray &ba);
    LongString(const QString &fileName);
    virtual ~LongString();

    LongString &operator=(const LongString &);

    bool isEmpty() const;
    int length() const;

    int indexOf(const QByteArray &ba, int from = 0) const;

    LongString mid(int i, int len = -1) const;
    LongString left(int len) const;
    LongString right(int len) const;

    // WARNING the QByteArray returned may become invalid when this LongString
    // is destroyed
    const QByteArray toQByteArray() const;

    QDataStream* dataStream() const;
    QTextStream* textStream() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSharedDataPointer<LongStringPrivate> d;
};

template <typename Stream>
Stream& operator<<(Stream &stream, const LongString& ls) { ls.serialize(stream); return stream; }

template <typename Stream>
Stream& operator>>(Stream &stream, LongString& ls) { ls.deserialize(stream); return stream; }

#endif
