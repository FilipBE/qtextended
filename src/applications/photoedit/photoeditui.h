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

#ifndef PHOTOEDITUI_H
#define PHOTOEDITUI_H

#include "editor/slider.h"
#include "editor/navigator.h"
#include "editor/regionselector.h"
#include "editor/imageui.h"
#include "editor/imageprocessor.h"
#include "editor/imageio.h"
#include "slideshow/slideshowdialog.h"
#include "slideshow/slideshowui.h"
#include "slideshow/slideshow.h"
#include "imageviewer.h"

#include <qcategorymanager.h>
#include <qtopiaabstractservice.h>
#include <QString>

class QDSActionRequest;
class QResizeEvent;
class QMenu;
class QStackedLayout;
class ThumbnailContentSetModel;
class QListView;
class QContentFilterDialog;
class EffectDialog;
class QActionGroup;
class QScreenInformation;
class QWaitWidget;

class PhotoEditUI : public QWidget
{
    Q_OBJECT
    friend class PhotoEditService;
public:
    PhotoEditUI( QWidget* parent, Qt::WFlags f );
    ~PhotoEditUI();

    bool eventFilter(QObject *parent, QEvent *event);

public slots:
    // Open image for editing
    void setDocument( const QString& lnk );

signals:
    void fullScreenDisabled(bool enabled);
    void showFullScreenWidgets(bool show);

    void scaleViewer(qreal sx, qreal sy);
    void setViewerScaleMode(ImageViewer::ScaleMode mode);

private slots:
    // Respond to service request
    void appMessage( const QString&, const QByteArray& );

    // Raise slide show to top of widget stack and start
    void enterSlideShow();

    // Raise editor to top of widget stack and load current image
    void enterEditor();

    // Show zoom control
    void enterZoom();

    // Show brightness control
    void enterBrightness();

    // Enable region selector and hide naviagtor
    void enterCrop();

    // Show editor view in full screen
    void enterFullScreen();
    void exitFullScreen();

    // Change to single view in image selector
    void setViewSingle();

    // Change to multi view in image selector
    void setViewThumbnail();

    // Launch slide show dialog
    void launchSlideShowDialog();

    // Launch properties dialog
    void launchPropertiesDialog();

    // Move to previous UI state
    // Enable application to be closed if no previous state exists
    bool exitCurrentUIState();

    // Move to previous editor state
    void exitCurrentEditorState();

    // Set zoom factor in image processor
    void setZoom( int );

    // Set brightness factor in image processor
    void setBrightness( int );

    // Open currently highlighted image in image selector for editing
    void editCurrentSelection();

    // Ignore changes to image and exit from editor
    void cancelEdit();

    void saveImageAs();

    // Perform crop on current image using region from region selector
    void cropImage();

    // Send current image over IR link
    void beamImage();

    // Print current image
    void printImage();

    // Delete current image
    void deleteImage();

    void setHomeScreenImage();
    void setContactImage();
    void setPersonalImage();

    // Show selector if image currently being edited is deleted
    void contentChanged( const QContentIdList &id, const QContent::ChangeType type );

    void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );
    void rowsInserted( const QModelIndex &parent, int start, int end );
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

    // Edit an image contained within the QDS request
    void editImage( const QDSActionRequest& request );

    void imageSelected( const QModelIndex &index );

    void viewImage( const QContent &content );

    void setViewerImage(const QContent &content);

    void selectType();

    void selectCategory();

    void selectEffect();

    void zoomViewer();
    void zoomViewerToActualSize();
    void zoomViewerToScreenSize();
    void setViewerZoom(int zoom);

    void viewerImageChanged();

    void viewerTapped();

#ifndef QTOPIA_HOMEUI
    void viewerMenuAboutToShow();
    void selectorMenuAboutToShow();
#endif

    // TV screen mode changed.
    void tvScreenChanged();

    // Current item in the stacked widget has changed.
    void stackItemChanged(int item);

protected:
    // Move to previous state, close application if no previous state exists
    void keyPressEvent( QKeyEvent * );

    void timerEvent( QTimerEvent *event );

    bool event( QEvent *event );
private:

    QWidget *imageViewer();
    ImageViewer *tvImageViewer();
    QWidget *selectorWidget();
    ImageUI *imageEditor();

    void updateViewerZoomRange();
    QPair<int, int> calculateZoomRange(const QSize &imageSize, const QSize &viewSize) const;
    // Hide editor controls, clear and show editor
    void clearEditor();

    // Prompt user to save changes to image if image was modified
    void saveChanges();
    bool saveImage(const QImage &image, QContent *content);
    bool copyImage(const QContent &image, const QString &target);

    // Send modified image back in qcop message
    void sendValueSupplied();

    bool service_requested;

    bool is_fullscreen, edit_canceled;

    QContent service_lnk;
    QCategoryFilter service_category;
    QString service_channel;
    QString service_id;
    int service_width, service_height;
    QImage service_image;

    QContent current_image;
    QContent m_savedImage;

    QActionGroup *m_selectorActions;
#ifndef QTOPIA_HOMEUI
    QAction *m_selectorEditAction;
    QAction *m_viewerEditAction;
#endif

    ImageScaler *m_imageScaler;
    ImageViewer *image_viewer;
    ImageViewer *tv_image_viewer;
    QListView *selector_view;
    QLabel *type_label;
    QLabel *category_label;
    QWidget *selector_widget;
    QContentSet *image_set;
    ThumbnailContentSetModel *image_model;

    EffectDialog *m_effectDialog;

    QSlider *m_viewerZoomSlider;

    RegionSelector *region_selector;
    Navigator *navigator;
    QSlider *brightness_slider, *zoom_slider;
    QWidget *brightness_widget;
    QWidget *zoom_widget;

    ImageUI *image_ui;
    ImageProcessor *image_processor;
    ImageIO *image_io;
    QStackedLayout *editor_stack;

    SlideShowDialog *slide_show_dialog;
    SlideShowUI *slide_show_ui;
    SlideShow *slide_show;

    QStackedLayout *widget_stack;
    QList< QWidget * > navigation_stack;

    QContentFilterDialog *type_dialog;
    QContentFilterDialog *category_dialog;

    QContentFilter type_filter;
    QContentFilter category_filter;

    QDSActionRequest* currEditImageRequest;

    int list_init_timer_id;

    bool m_fullScreenWidgetsVisible;

    QWidget *m_imageWidget;
#ifdef QTOPIA_HOMEUI
    QLabel *m_imageCaption;
#endif
    QWaitWidget *m_viewerWaitWidget;

    QScreenInformation *m_tvScreen;
};

class PhotoEditService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class PhotoEditUI;
private:
    PhotoEditService( PhotoEditUI *parent )
        : QtopiaAbstractService( "PhotoEdit", parent ),
        mParent( parent ) { publishAll(); }

public:
    ~PhotoEditService();

public slots:
    void showCategory( const QString& category );
    void editImage( const QDSActionRequest& request );

private:
    PhotoEditUI* mParent;
};

#endif
