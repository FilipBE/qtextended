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

#ifndef QPIMRECORD_H
#define QPIMRECORD_H

#include <quniqueid.h>
#include <qmap.h>
#include <qlist.h>
#include <quuid.h>
#include <qset.h>

class QPimRecordPrivate;

class QTOPIAPIM_EXPORT QPimRecord
{
public:
    virtual ~QPimRecord();
    // The enum maps should internally UID and Category as keys, but the internal "private" enums should
    // be set using the above values as the keys. The #define below allows the enum declerations in
    // subclass to know which ID number is starting for their use

    bool operator==( const QPimRecord &other ) const;
    bool operator!=( const QPimRecord &other ) const;

    void setCategories( const QList<QString> &categories );
    void setCategories( const QString &id );
    void reassignCategoryId( const QString & oldId, const QString & newId );
    bool pruneDeadCategories(const QList<QString> &validCats);
    QList<QString> categories() const;

    virtual QString customField(const QString &) const;
    virtual void setCustomField(const QString &, const QString &);
    virtual void removeCustomField(const QString &);
    QMap<QString, QString> customFields() const { return customFieldsRef(); }
    void setCustomFields(const QMap<QString, QString> &fields);

    QUniqueId uid() const { return uidRef(); }
    void setUid( const QUniqueId & uid ) { uidRef() = uid; }

    virtual void setNotes(const QString &) = 0;
    virtual QString notes() const = 0;

    // Dependencies
    QUniqueId parentDependency() const;
    QString parentDependencyType() const;
    QList<QUniqueId> dependentChildrenOfType(const QString& type) const;
    QMap<QString, QUniqueId> dependentChildren() const;

protected:
    // expect overridden classes?  const and non-const refes?
    virtual QUniqueId &uidRef() = 0;
    virtual const QUniqueId &uidRef() const = 0;

    virtual QList<QString> &categoriesRef() = 0;
    virtual const QList<QString> &categoriesRef() const = 0;

    virtual QMap<QString, QString> &customFieldsRef() = 0;
    virtual const QMap<QString, QString> &customFieldsRef() const = 0;
};

#endif
