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
#ifndef QDOCUMENTSERVERCONTENTSETENGINE_P_H
#define QDOCUMENTSERVERCONTENTSETENGINE_P_H

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

#include "qcontentsetengine_p.h"
#include <QCache>

class QDocumentServerContentStore;
class QDocumentServerContentStorePrivate;

class QDocumentServerContentSetEngine : public QContentSetEngine
{
    Q_OBJECT
public:
    QDocumentServerContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &sortOrder, QContentSet::UpdateMode mode, int setId, QDocumentServerContentStore *store );

    virtual ~QDocumentServerContentSetEngine();

    virtual QContent content( int index ) const;
    virtual int count() const;
    virtual bool isEmpty() const;

    virtual void insertContent( const QContent &content );
    virtual void removeContent( const QContent &content );

    virtual void clear();

    virtual void commitChanges();

    virtual bool contains( const QContent &content ) const;

    void insertContent( int start, int end );
    void removeContent( int start, int end );
    void refreshContent( int start, int end );

signals:
    void releaseContentSet( int setId );

protected:
    virtual void filterChanged( const QContentFilter &filter );
    virtual void sortCriteriaChanged( const QContentSortCriteria &sort );

private:
    virtual QContentList values( int index, int count );
    virtual int valueCount() const;

    int m_setId;

    QDocumentServerContentStore *m_store;

    int m_count;

    friend class QDocumentServerContentStorePrivate;
};

#endif
