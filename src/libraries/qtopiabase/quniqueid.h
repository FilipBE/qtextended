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

#ifndef QUNIQUEID_H
#define QUNIQUEID_H

#include <qtopiaglobal.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>
#include <QString>
#include <QUuid>

class QUniqueIdGenerator;
class QTOPIA_EXPORT QUniqueId
{
    friend class QUniqueIdGenerator;
public:
    QUniqueId();
    QUniqueId(const QUniqueId &o);
    explicit QUniqueId(const QString &s);
    explicit QUniqueId(const QByteArray &);

    uint index() const;
    QUuid context() const;

    uint mappedContext() const;

    QUniqueId operator=(const QUniqueId &o);

    bool operator==(const QUniqueId &o) const { return mId == o.mId; }
    bool operator!=(const QUniqueId &o) const { return mId != o.mId; }

    // these are not really useful, and could be harmful.
    bool operator<(QUniqueId o) const { return mId < o.mId; }
    bool operator>(QUniqueId o) const { return mId > o.mId; }
    bool operator<=(QUniqueId o) const { return mId <= o.mId; }
    bool operator>=(QUniqueId o) const { return mId >= o.mId; }

    bool isNull() const { return mId == 0; }

    bool isTemporary() const;

    QString toString() const;
    QByteArray toByteArray() const;

    uint toUInt() const;
    static QUniqueId fromUInt(uint);

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    QString toLocalContextString() const;
    QByteArray toLocalContextByteArray() const;

protected:
    // automatically reversable via constructors, which can detect format.
#ifndef QT_NO_DATASTREAM
    QDataStream &fromLocalContextDataStream( QDataStream & );
    QDataStream &toLocalContextDataStream( QDataStream & ) const;
#endif

private:
    void setContext(uint context);
    void setIndex(uint index);
    void setIdentity(uint context, uint index);
    uint mId;

    static QUuid legacyIdContext();
    static QUuid temporaryIdContext();

    friend uint qHash(const QUniqueId &uid);
};

inline uint qHash(const QUniqueId &uid) { return uid.mId; }

class QTOPIA_EXPORT QUniqueIdGenerator
{
public:
    QUniqueIdGenerator(const QUuid &context);
    QUniqueIdGenerator(const QUniqueIdGenerator &other);

    ~QUniqueIdGenerator();

    bool isValid() const { return mContext != 0; }
    QUniqueId createUniqueId();

    QUniqueId constructUniqueId(uint);

    static QUniqueId constructTemporaryId(uint);
    static QUniqueId createTemporaryId();

    static uint mappedContext(const QUuid &context);
private:
    uint mContext; // a combination of device mapped and scope mapped context.
    uint mLastId;
    uint mReserved;
};

Q_DECLARE_USER_METATYPE(QUniqueId)

#endif
