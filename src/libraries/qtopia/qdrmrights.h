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

#ifndef QDRMRIGHTS_H
#define QDRMRIGHTS_H

#include <qtopiaglobal.h>
#include <qtopiaipcmarshal.h>
#include <QSharedData>
#include <QStringList>
#include <QFlags>
#include <QPair>

class QDrmRightsPrivate;
class QDrmRightsConstraintPrivate;


class QTOPIA_EXPORT QDrmRights
{
public:
    enum Status
    {
        Invalid,
        Valid,
        ValidInFuture
    };

    enum Permission
    {
        NoPermissions     = 0x0000,
        Play              = 0x0001,
        Display           = 0x0002,
        Execute           = 0x0004,
        Print             = 0x0008,
        Export            = 0x0010,
        Distribute        = 0x0020,
        Preview           = 0x0040,
        Automated         = 0x0080,
        BrowseContents    = 0x0100,
        Unrestricted      = 0x7F00 | Play | Display | Execute | Print | Export | Distribute | Preview | Automated | BrowseContents,
        InvalidPermission = 0x8000
    };

    Q_DECLARE_FLAGS( Permissions, Permission );

    class QTOPIA_EXPORT Constraint
    {
    public:
        Constraint();
        Constraint( const QString &name, const QVariant &value );
        Constraint( const QString &name, const QVariant &value, const QList< QPair< QString, QVariant > > &attributes );
        Constraint( const Constraint &other );
        ~Constraint();

        Constraint &operator =( const Constraint &other );

        QString name() const;
        QVariant value() const;

        int attributeCount() const;
        QString attributeName( int index ) const;
        QVariant attributeValue( int index ) const;

        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);

    private:
        QSharedDataPointer< QDrmRightsConstraintPrivate > d;
    };

    typedef QList< Constraint > ConstraintList;

    QDrmRights();
    QDrmRights( Permission permission, Status status, const ConstraintList &constraints = ConstraintList() );
    QDrmRights( const QDrmRights &other );
    ~QDrmRights();

    QDrmRights &operator =( const QDrmRights &other );

    Permission permission() const;

    Status status() const;

    ConstraintList constraints() const;

    static QString toString( Permission permission );
    static QString toString( Permission permission, Status status );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSharedDataPointer< QDrmRightsPrivate > d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDrmRights::Permissions);
Q_DECLARE_USER_METATYPE_ENUM(QDrmRights::Permission);
Q_DECLARE_USER_METATYPE_ENUM(QDrmRights::Permissions);
Q_DECLARE_USER_METATYPE_ENUM(QDrmRights::Status);
Q_DECLARE_USER_METATYPE(QDrmRights);
Q_DECLARE_USER_METATYPE(QDrmRights::Constraint);
Q_DECLARE_USER_METATYPE_TYPEDEF(QDrmRightsConstraintList,QDrmRights::ConstraintList);

#endif
