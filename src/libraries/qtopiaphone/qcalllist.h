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

#ifndef QCALLLIST_H
#define QCALLLIST_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>

#include <quniqueid.h>
#include <qtopiaipcmarshal.h>

class QCallListItem;
class QCallListItemPrivate;
class QCallListPrivate;


class QTOPIAPHONE_EXPORT QCallListItem
{
public:
    enum CallType { Dialed, Received, Missed };

    QCallListItem();
    QCallListItem( CallType type, const QString& number, const QDateTime& start,
                   const QDateTime &end, const QUniqueId& contact = QUniqueId(),
                   const QString& serviceType = QString(),
                   const QString& timeZoneId = QString(), bool simRecord = false );
    QCallListItem( CallType type, const QString& number, bool simRecord = true );
    QCallListItem( const QCallListItem& info );
    ~QCallListItem();

    bool operator== ( const QCallListItem& other ) const;
    bool operator!= ( const QCallListItem& other ) const
    {
        return !operator==(other);
    }
    QCallListItem& operator= ( const QCallListItem& other );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    CallType type() const;
    QString number() const;
    QDateTime start() const;
    QDateTime end() const;
    QDateTime originalTimeZoneStart() const;
    QDateTime originalTimeZoneEnd() const;
    QUniqueId contact() const;
    QString serviceType() const;
    QString timeZoneId() const;
    bool isSIMRecord() const;

    bool isNull() const;

public:
    mutable QCallListItemPrivate *d;
};


class QTOPIAPHONE_EXPORT QCallList : public QObject
{
    Q_OBJECT
public:
    enum DuplicateBehaviour { AllowDuplicates, OverwriteDuplicates };
    enum ListType { All, Dialed, Received, Missed };
    explicit QCallList( int l = -1 );
    ~QCallList();

    struct QTOPIAPHONE_EXPORT SearchOptions {
        QString number;
        QUniqueId contactId;
    };

    QList<QCallListItem> allCalls() const;
    QList<QCallListItem> searchCalls( SearchOptions& options ) const;

    void record( QCallListItem item, DuplicateBehaviour = AllowDuplicates );

    uint count() const;
    QCallListItem at( uint posn ) const;

    void setMaxItems( const int &l );
    int maxItems() const;

    void clear();
    void removeAt( uint posn );
    void removeAll( const QString& number );
    void removeAll( QCallListItem::CallType type );

private slots:
    void callListMessage( const QString&, const QByteArray& );

signals:
    void updated();

private:
    QCallListPrivate *d;
};

Q_DECLARE_USER_METATYPE(QCallListItem);
Q_DECLARE_USER_METATYPE_ENUM(QCallList::ListType);

#endif
