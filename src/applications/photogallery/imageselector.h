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
#ifndef IMAGESELECTOR_H
#define IMAGESELECTOR_H

#include <QTabWidget>
#include <QModelIndex>
#include <QContentSetModel>

class ThumbCache;
class HomeActionButton;
class QLabel;
class QStackedWidget;
class QListView;
class QSmoothList;
class QSmoothIconView;
class QItemSelectionModel;

#ifdef USE_PICTUREFLOW
class PictureFlowView;
#endif

class ImageSelector : public QWidget
{
    Q_OBJECT
public:
    enum View
    {
#ifdef USE_PICTUREFLOW
        IconView,
        FlowView
#else
        IconView
#endif
    };

    ImageSelector(View view = IconView, QWidget *parent = 0);
    ~ImageSelector();

    View view() const;
    void setView(View view);

    QContent currentImage() const;
    int count() const;

    QContentSetModel *model() const { return m_model; }
    QItemSelectionModel *selectionModel() const { return m_selectionModel; }

public slots:
    void setAlbum(const QString &name, const QString &categoryId);

    void moveNext();
    void movePrevious();

    void rotateAnticlockwise();
    void rotateClockwise();
    void startSlideShow();

signals:
    void viewChanged(ImageSelector::View view);
    void selected(const QContent &image);
    void startSlideShow(const QContent &image);
    void currentChanged(const QContent &image);
    void back();

private slots:
#ifdef USE_PICTUREFLOW
    void toggleView()
        { setView(View(1 - m_view)); }

    void flowIndexChanged(const QModelIndex &index);
#endif

    void imageSelected(const QModelIndex &index)
        { emit selected(qvariant_cast<QContent>(index.data(QContentSetModel::ContentRole))); }

    void setCurrentIndex( const QModelIndex& );

private:
    QModelIndex currentIndex() const;

    QContentSet m_contentSet;
    View m_view;
#ifdef USE_PICTUREFLOW
    HomeActionButton *m_viewButton;
#endif
    QLabel *m_title;

    QStackedWidget *m_stack;
    QSmoothIconView *m_iconView;
#ifdef USE_PICTUREFLOW
    PictureFlowView *m_flowView;
    QLabel *m_flowLabel;
    QWidget *m_flowWidget;
#endif

    QContentSetModel *m_model;
    QItemSelectionModel *m_selectionModel;
};

#endif

