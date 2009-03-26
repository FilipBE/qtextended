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

#ifndef QCONTENTSORTCRITERIA_H
#define QCONTENTSORTCRITERIA_H

#include <QContentFilter>
#include <QtGlobal>

class QContentSortCriteriaPrivate;

class QTOPIA_EXPORT QContentSortCriteria
{
public:

    enum Attribute
    {
        Name,
        MimeType,
        Property,
        FileName,
        LastUpdated
    };

    QContentSortCriteria();
    QContentSortCriteria( Attribute attribute, Qt::SortOrder order = Qt::AscendingOrder );
    QContentSortCriteria( Attribute attribute, const QString &scope, Qt::SortOrder order = Qt::AscendingOrder );
    QContentSortCriteria( QContent::Property property, Qt::SortOrder order = Qt::AscendingOrder );
    QContentSortCriteria( const QContentSortCriteria &other );

    ~QContentSortCriteria();

    QContentSortCriteria &operator =( const QContentSortCriteria &other );

    bool operator ==( const QContentSortCriteria &other ) const;
    bool operator !=( const QContentSortCriteria &other ) const;

    void addSort( Attribute attribute, Qt::SortOrder order = Qt::AscendingOrder );
    void addSort( Attribute attribute, const QString &scope, Qt::SortOrder order = Qt::AscendingOrder );
    void addSort( QContent::Property property, Qt::SortOrder order = Qt::AscendingOrder );

    void clear();

    int sortCount() const;

    Attribute attribute( int index ) const;
    QString scope( int index ) const;
    Qt::SortOrder order( int index ) const;

    int compare( const QContent &left, const QContent &right ) const;

    bool lessThan( const QContent &left, const QContent &right ) const;
    bool greaterThan( const QContent &left, const QContent &right ) const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSharedDataPointer< QContentSortCriteriaPrivate > d;
};

Q_DECLARE_USER_METATYPE(QContentSortCriteria);
Q_DECLARE_USER_METATYPE_ENUM(QContentSortCriteria::Attribute);

#endif
