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

#ifndef QIMAGEDOCUMENTSELECTOR_P_H
#define QIMAGEDOCUMENTSELECTOR_P_H

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

#include "qimagedocumentselector.h"
#include "thumbnailview_p.h"

#include <qcontent.h>
#include <qsoftmenubar.h>

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QPoint>
#include <QLabel>
#include <QStackedLayout>
#include <QDrmContent>

class QContentFilterDialog;
class SingleView;
class QAbstractListModel;
class ThumbnailLoader;

class QImageDocumentSelectorPrivate : public QWidget
{
    Q_OBJECT
public:
    explicit QImageDocumentSelectorPrivate( QWidget* parent = 0 );
    ~QImageDocumentSelectorPrivate();

    // Images may be displayed as a collection of thumbnails
    // or singularly, default thumbnail mode
    void setViewMode( QImageDocumentSelector::ViewMode mode );
    QImageDocumentSelector::ViewMode viewMode() const { return current_view; }

    // Set the maximum size of thumbnails
    void setThumbnailSize( const QSize& size );
    QSize thumbnailSize() const;

    void setFilter( const QContentFilter &filter , bool force = false );
    QContentFilter filter() const;

    void setSortMode( QDocumentSelector::SortMode, bool force = false );
    QDocumentSelector::SortMode sortMode() const;

    void setSortCriteria( const QContentSortCriteria &sort );
    QContentSortCriteria sortCriteria() const;

    // Return current selected document
    QContent selectedDocument() const;
    // Return list of currently visible images
    const QContentSet &documents() const;

    // Return number of currently visible images
    int count() const;

    void setDefaultCategories( const QStringList &categories );
    QStringList defaultCategories() const;

    void setSelectPermission( QDrmRights::Permission permission );
    QDrmRights::Permission selectPermission() const;

    void setMandatoryPermissions( QDrmRights::Permissions permissions );
    QDrmRights::Permissions mandatoryPermissions() const;

public slots:
    void setFocus();

signals:
    void documentSelected( const QContent& image );
    void documentHeld( const QContent&, const QPoint& );

    void documentsChanged();

private slots:
    // Select current image
    void emitSelected();

    // Hold current image
    void emitHeld( const QPoint& );

    // Set view mode to single
    void setViewSingle();

    // Set view mode to thumbnail
    void setViewThumbnail();

    // Only Qtopia Phone
    // Launch view category dialog
    void launchCategoryDialog();

    void updateFinished();

protected:
    void showEvent( QShowEvent *event );
    void hideEvent( QHideEvent *event );

private:
    void init();
    void applyFilters(bool force = false);

    QContentFilter content_filter;
    QContentFilter category_filter;
    QContentSet image_collection;
    QContentSetModel *model;
    QDocumentSelector::SortMode sort_mode;
    QDrmContent drm_content;

    QImageDocumentSelector::ViewMode current_view;

    // Only Qtopia Phone
    QContentFilterDialog *category_dialog;
    QLabel *category_label;

    QStringList default_categories;
    QStringList filtered_default_categories;
    bool        default_categories_dirty;

    QStackedLayout *widget_stack;

    SingleView *single_view;
    ThumbnailView *thumbnail_view;
    QLabel *message_view;

    ThumbnailLoader *loader;

    void filterDefaultCategories();
};

#endif
