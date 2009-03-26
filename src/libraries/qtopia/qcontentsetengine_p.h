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
#ifndef QCONTENTSETENGINE_P_H
#define QCONTENTSETENGINE_P_H

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

#include <QStringList>
#include <QContentFilter>
#include <QContentSortCriteria>
#include <QContentSet>
#include "qsparselist_p.h"

class QStringList;
class QContent;

class QContentSetEngine : public QObject, protected QSparseList< QContent, 20, 5 >
{
    Q_OBJECT
public:
    QContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &sort, QContentSet::UpdateMode mode )
        : m_filter( filter )
        , m_sortCriteria( sort )
        , m_updateMode( mode )
        , m_flushTimerId(-1)
        , m_updateInProgress( false )
    {
        if (updateMode() == QContentSet::Asynchronous)
            connect(this, SIGNAL(updateFinished()), this, SIGNAL(contentChanged()));
    }

    virtual ~QContentSetEngine();

    virtual QContent content( int index ) const;
    virtual QContentId contentId( int index ) const;
    virtual int count() const;
    virtual bool isEmpty() const;

    void setSortCriteria( const QContentSortCriteria &sort );
    void setFilter( const QContentFilter &filter );

    void setSortOrder( const QStringList &sortOrder );

    virtual void insertContent( const QContent &content ) = 0;
    virtual void removeContent( const QContent &content ) = 0;

    virtual void clear() = 0;

    bool updatePending() const { return m_flushTimerId != -1; }
    void flush();
    virtual void commitChanges() = 0;

    virtual bool contains( const QContent &content ) const = 0;

    QStringList sortOrder() const{ return m_sortOrder; }
    QContentSortCriteria sortCriteria() const{ return m_sortCriteria; }
    QContentFilter filter() const{ return m_filter; }

    QContentSet::UpdateMode updateMode() const{ return m_updateMode; }

    static QContentSortCriteria convertSortOrder( const QStringList &sortOrder );

    bool updateInProgress() const{ return m_updateInProgress; }

public slots:
    void update();

signals:
    void contentAboutToBeRemoved( int start, int end );
    void contentAboutToBeInserted( int start, int end );
    void contentInserted();
    void contentRemoved();
    void contentChanged( int start, int end );
    void contentChanged();

    void updateStarted();
    void updateFinished();

    void reset();

protected:

    virtual void sortCriteriaChanged( const QContentSortCriteria &sort ) = 0;
    virtual void filterChanged( const QContentFilter &filter ) = 0;

    void timerEvent(QTimerEvent *event);

protected slots:
    void startUpdate();
    void finishUpdate();

private:
    QContentFilter m_filter;
    QStringList m_sortOrder;
    QContentSortCriteria m_sortCriteria;
    QContentSet::UpdateMode m_updateMode;
    int m_flushTimerId;
    bool m_updateInProgress;
};

#endif
