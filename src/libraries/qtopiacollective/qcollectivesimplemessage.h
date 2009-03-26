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

#ifndef QCOLLECTIVESIMPLEMESSAGE_H
#define QCOLLECTIVESIMPLEMESSAGE_H

#include <qtopiaglobal.h>
#include <qcollectivenamespace.h>
#include <QSharedData>

class QCollectiveSimpleMessageData;

class QTOPIACOLLECTIVE_EXPORT QCollectiveSimpleMessage
{
public:

    enum Type { Normal = 0, AutoReply };

    QCollectiveSimpleMessage();
    QCollectiveSimpleMessage(const QCollectiveSimpleMessage &other);
    ~QCollectiveSimpleMessage();

    void setFrom(const QString &uri);
    QString from() const;

    void setTo(const QString &uri);
    QString to() const;

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &timestamp);

    void setText(const QString &text);
    QString text() const;

    QCollectiveSimpleMessage::Type type() const;
    void setType(QCollectiveSimpleMessage::Type type);

    bool operator==(const QCollectiveSimpleMessage &other) const;
    bool operator!=(const QCollectiveSimpleMessage &other) const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    QCollectiveSimpleMessage &operator=(const QCollectiveSimpleMessage &other);

private:
    QSharedDataPointer<QCollectiveSimpleMessageData> d;
};

Q_DECLARE_USER_METATYPE(QCollectiveSimpleMessage)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QCollectiveSimpleMessage>)

#ifndef QT_NO_DEBUG_STREAM
QTOPIACOLLECTIVE_EXPORT QDebug operator<<(QDebug, const QCollectiveSimpleMessage &);
#endif

#endif
