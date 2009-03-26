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
#ifndef QSQLCATEGORYSTORE_P_H
#define QSQLCATEGORYSTORE_P_H

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

#include "qcategorystore_p.h"
#include <QCache>

class QSqlRecord;

class QSqlCategoryStore : public QCategoryStore
{
    Q_OBJECT
public:
    QSqlCategoryStore();
    virtual ~QSqlCategoryStore();

    virtual bool addCategory( const QString &id, const QString &scope, const QString &label, const QString &icon, bool isSystem );
    virtual bool categoryExists( const QString &id );
    virtual QCategoryData categoryFromId( const QString &id );
    virtual QMap< QString, QCategoryData > scopeCategories( const QString &scope );
    virtual bool removeCategory( const QString &id );
    virtual bool setCategoryScope( const QString &id, const QString &scope );
    virtual bool setCategoryIcon( const QString &id, const QString &icon );
    virtual bool setCategoryRingTone( const QString &id, const QString &icon );
    virtual bool setCategoryLabel( const QString &id, const QString &label );
    virtual bool setSystemCategory( const QString &id );

private slots:
    void reloadCategories();

private:
    QCategoryData categoryFromRecord( const QString &id, const QSqlRecord &record );

    void categoryAddedLocal( const QString &id );
    void categoryRemovedLocal( const QString &id );
    void categoryChangedLocal( const QString &id );

    QCache< QString, QCategoryData > m_cache;
};

#endif
