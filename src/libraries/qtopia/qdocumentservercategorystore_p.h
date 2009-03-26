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
#ifndef QDOCUMENTSERVERCATEGORYSTORE_P_H
#define QDOCUMENTSERVERCATEGORYSTORE_P_H

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

class QDocumentServerCategoryStorePrivate;

class QDocumentServerCategoryStore : public QCategoryStore
{
public:
    QDocumentServerCategoryStore( QObject *parent = 0 );

    virtual ~QDocumentServerCategoryStore();

    virtual bool addCategory( const QString &categoryId, const QString &scope, const QString &label, const QString &icon, bool isSystem );

    virtual bool categoryExists( const QString &categoryId );

    virtual QCategoryData categoryFromId( const QString &categoryId );

    virtual QCategoryDataMap scopeCategories( const QString &scope );

    virtual bool removeCategory( const QString &categoryId );

    virtual bool setCategoryScope( const QString &categoryId, const QString &scope );

    virtual bool setCategoryIcon( const QString &categoryId, const QString &icon );

    virtual bool setCategoryRingTone( const QString &id, const QString &fileName );

    virtual bool setCategoryLabel( const QString &categoryId, const QString &label );

    virtual bool setSystemCategory( const QString &categoryId );

private:
    QDocumentServerCategoryStorePrivate *d;
};

#endif
