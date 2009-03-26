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

#include "photoeditui.h"

#include "thumbnailmodel.h"
#include "imageviewer.h"
#include "effectdialog.h"
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qdocumentproperties.h>
#include <qtopiaipcenvelope.h>
#include <qtopiasendvia.h>
#include <qtopiaservices.h>
#include <qcontent.h>
#include <qdrmcontentplugin.h>
#include <QtopiaChannel>

#include <QPushButton>
#include <QPoint>
#include <QMessageBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSize>
#include <QCloseEvent>
#include <QDSActionRequest>
#include <QDSData>
#include <QPixmap>
#include <QMimeType>
#include <QStackedLayout>
#include <QMenu>
#include <QListView>
#include <QContentFilterDialog>
#include <QDesktopWidget>
#include <QToolButton>
#include <QActionGroup>
#include <QScrollBar>
#include <QScreenInformation>
#include <QWaitWidget>
#include <QFileSystem>
#include <QtopiaService>
#include <QItemSelectionModel>
#include <QImageWriter>

#include <cmath>

static const qreal q_imageZoomScales[] = {
        0.01, 0.0125, 0.025, 0.033, 0.05, 0.0667, 0.075,
        0.1, 0.125, 0.25, 0.333, 0.5, 0.667, 0.75, 0.9,
        1.0, 1.1, 1.25, 1.5, 1.75, 2.0, 2.5, 3.0, 4.0};

PhotoEditUI::PhotoEditUI( QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f )
    , service_requested( false )
    , is_fullscreen(false)
    , edit_canceled( false )
    , m_selectorActions(0)
#ifndef QTOPIA_HOMEUI
    , m_selectorEditAction(0)
    , m_viewerEditAction(0)
#endif
    , m_imageScaler(0)
    , image_viewer(0)
    , tv_image_viewer(0)
    , selector_view(0)
    , type_label(0)
    , category_label(0)
    , selector_widget(0)
    , m_effectDialog(0)
    , m_viewerZoomSlider(0)
    , region_selector(0)
    , navigator(0)
    , brightness_slider(0)
    , zoom_slider(0)
    , brightness_widget(0)
    , zoom_widget(0)
    , image_ui(0)
    , image_processor(0)
    , image_io(0)
    , editor_stack(0)
    , slide_show_dialog(0)
    , slide_show_ui(0)
    , slide_show(0)
    , widget_stack(0)
    , type_dialog( 0 )
    , category_dialog( 0 )
    , currEditImageRequest(0)
    , list_init_timer_id(-1)
    , m_fullScreenWidgetsVisible(false)
    , m_imageWidget(0)
#ifdef QTOPIA_HOMEUI
    , m_imageCaption(0)
#endif
    , m_viewerWaitWidget(0)
    , m_tvScreen(0)
{
    setWindowTitle( tr( "Pictures" ) );

    QDrmContentPlugin::initialize();

    widget_stack = new QStackedLayout( this );
    connect(widget_stack, SIGNAL(currentChanged(int)), this, SLOT(stackItemChanged(int)));

    // Respond to service requests
    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
        this, SLOT(appMessage(QString,QByteArray)) );

    new PhotoEditService( this );

    // Respond to file changes
    connect( qApp,
        SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
        this,
        SLOT(contentChanged(QContentIdList,QContent::ChangeType)));

    list_init_timer_id = startTimer( 0 );

    // Find the television output, if we have one.
    m_tvScreen = new QScreenInformation(QScreenInformation::Television);
    if (m_tvScreen->screenNumber() != -1) {
        connect(m_tvScreen, SIGNAL(changed()), this, SLOT(tvScreenChanged()));
        QTimer::singleShot(0, this, SLOT(tvScreenChanged()));
    } else {
        delete m_tvScreen;
        m_tvScreen = 0;
    }
}

PhotoEditUI::~PhotoEditUI()
{
    delete currEditImageRequest;
    currEditImageRequest = 0;
    delete tv_image_viewer;
}

bool PhotoEditUI::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::MouseButtonPress) {
        exitCurrentEditorState();

        return true;
    }

    return false;
}

QWidget *PhotoEditUI::imageViewer()
{
    if( !image_viewer )
    {
#ifdef QTOPIA_HOMEUI
        QToolButton *backButton = new QToolButton;
        backButton->setText(tr("Back"));
        backButton->setFocusPolicy(Qt::NoFocus);
        connect(backButton, SIGNAL(clicked()), this, SLOT(exitCurrentUIState()));
        connect(this, SIGNAL(fullScreenDisabled(bool)), backButton, SLOT(setVisible(bool)));

        // Caption
        m_imageCaption = new QLabel;
        m_imageCaption->setAlignment(Qt::AlignCenter);
        QPalette palette = m_imageCaption->palette();
        palette.setBrush(QPalette::Window, palette.brush(QPalette::Button));
        m_imageCaption->setPalette(palette);
        m_imageCaption->setAutoFillBackground(true);

        connect(this, SIGNAL(fullScreenDisabled(bool)), m_imageCaption, SLOT(setVisible(bool)));

        QBoxLayout *titleLayout = new QHBoxLayout;
        titleLayout->setMargin(0);
        titleLayout->setSpacing(0);
        titleLayout->addWidget(m_imageCaption);
        titleLayout->addWidget(backButton);
#endif

        QBoxLayout *viewerLayout = new QVBoxLayout;
        viewerLayout->setMargin(0);
        viewerLayout->setSpacing(0);

        if (Qtopia::mousePreferred()) {
            QAbstractButton *exitFullscreenButton = new QToolButton;
            exitFullscreenButton->setText(tr("Back"));
            exitFullscreenButton->setFocusPolicy(Qt::NoFocus);
            exitFullscreenButton->setVisible(false);

            connect(exitFullscreenButton, SIGNAL(clicked()), this, SLOT(exitFullScreen()));
            connect(this, SIGNAL(showFullScreenWidgets(bool)),
                    exitFullscreenButton, SLOT(setVisible(bool)));

            viewerLayout->addWidget(exitFullscreenButton, 0, Qt::AlignRight);
        }

        m_viewerZoomSlider = new Slider(0, 0, 1, 0, 0);
        m_viewerZoomSlider->setVisible(false);

        connect(m_viewerZoomSlider, SIGNAL(selected()), this, SLOT(exitCurrentUIState()));
        connect(m_viewerZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setViewerZoom(int)));

        if (Qtopia::mousePreferred()) {
            connect(this, SIGNAL(showFullScreenWidgets(bool)),
                    m_viewerZoomSlider, SLOT(setVisible(bool)));
        }

        QBoxLayout *sliderLayout = new QVBoxLayout;
        sliderLayout->setMargin(style()->pixelMetric(QStyle::PM_ScrollBarExtent) * 2);
        sliderLayout->addStretch();
        sliderLayout->addWidget(m_viewerZoomSlider);

        viewerLayout->addLayout(sliderLayout);

        QMenu *sliderMenu = QSoftMenuBar::menuFor(m_viewerZoomSlider);
        sliderMenu->addAction(tr("Actual Size"), this, SLOT(zoomViewerToActualSize()));
        sliderMenu->addAction(tr("Best Fit"), this, SLOT(zoomViewerToScreenSize()));

        m_imageScaler = new ImageScaler(this);

        connect(m_imageScaler, SIGNAL(imageInvalidated()), this, SLOT(exitCurrentUIState()));
        connect(m_imageScaler, SIGNAL(imageChanged()), this, SLOT(viewerImageChanged()));

        m_viewerWaitWidget = new QWaitWidget(this);
        connect(m_imageScaler, SIGNAL(imageChanged()), m_viewerWaitWidget, SLOT(hide()));

        image_viewer = new ImageViewer(m_imageScaler);
        image_viewer->setScaleMode(ImageViewer::ScaleToFit);
        image_viewer->setLayout(viewerLayout);

        connect(image_viewer, SIGNAL(tapped()), this, SLOT(viewerTapped()));
        connect(this, SIGNAL(scaleViewer(qreal,qreal)), image_viewer, SLOT(setScale(qreal,qreal)));
        connect(this, SIGNAL(setViewerScaleMode(ImageViewer::ScaleMode)),
                image_viewer, SLOT(setScaleMode(ImageViewer::ScaleMode)));

        QMenu *viewer_menu = QSoftMenuBar::menuFor( image_viewer );

        viewer_menu->addAction(
                QIcon(":icon/slideshow"), tr("Slide Show..."), this, SLOT(launchSlideShowDialog()));
        viewer_menu->addSeparator();

        viewer_menu->addSeparator();

#ifndef QTOPIA_HOMEUI
        m_viewerEditAction = viewer_menu->addAction(
                QIcon(":icon/edit"), tr("Edit"), this, SLOT(editCurrentSelection()));
#endif
        viewer_menu->addAction(
                QIcon(":icon/info"), tr("Properties"), this, SLOT(launchPropertiesDialog()));
        viewer_menu->addAction(QIcon(":icon/beam"), tr("Send"), this, SLOT(beamImage()));
        if (!QtopiaService::apps(QLatin1String("Print")).isEmpty())
            viewer_menu->addAction(QIcon(":icon/print"), tr("Print"), this, SLOT(printImage()));
        viewer_menu->addAction(QIcon(":icon/trash" ), tr("Delete"), this, SLOT(deleteImage()));
        viewer_menu->addAction(
                QIcon(":icon/home"), tr("Set as Background"), this, SLOT(setHomeScreenImage()));
        if (!QtopiaService::apps(QLatin1String("Contacts")).isEmpty()) {
            viewer_menu->addAction(tr("Save to Contact..."), this, SLOT(setContactImage()));
            viewer_menu->addAction(tr("Set as Avatar"), this, SLOT(setPersonalImage()));
        }
        viewer_menu->addSeparator();

        foreach (QString path, Qtopia::installPaths()) {
            if (!QDir(path + QLatin1String("/etc/photoedit")).entryList(QDir::Files).isEmpty()) {
                viewer_menu->addAction(tr("Effects..."), this, SLOT(selectEffect()));
                break;
            }
        }

        QAction *fullScreenAction = viewer_menu->addAction(
                QIcon(":icon/fullscreen"), tr("Full Screen"), this, SLOT(enterFullScreen()));

        connect(this, SIGNAL(fullScreenDisabled(bool)), fullScreenAction, SLOT(setVisible(bool)));

        viewer_menu->addAction(QIcon(":/icon/view"), tr("Zoom..."), this, SLOT(zoomViewer()));

#ifndef QTOPIA_HOMEUI
        connect( viewer_menu, SIGNAL(aboutToShow()), this, SLOT(viewerMenuAboutToShow()) );
#endif

        QSoftMenuBar::setLabel(image_viewer, Qt::Key_Select, QLatin1String(":icon/view"), tr("Zoom"));

        QBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);
        layout->setSpacing(0);
#ifdef QTOPIA_HOMEUI
        layout->addLayout(titleLayout);
#endif
        layout->addWidget(image_viewer);

        m_imageWidget = new QWidget;
        m_imageWidget->setLayout(layout);

        widget_stack->addWidget(m_imageWidget);

    }
    return m_imageWidget;
}

QWidget *PhotoEditUI::selectorWidget()
{
    if( !selector_widget )
    {
        QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

        QStringList types = settings.value( QLatin1String( "Types" ) ).toString().split( QLatin1Char( ';' ), QString::SkipEmptyParts );
        QStringList categories = settings.value( QLatin1String( "Categories" ) ).toString().split( QLatin1Char( ';' ), QString::SkipEmptyParts );

        foreach( QString type, types )
            type_filter |= QContentFilter::mimeType( type );
        foreach( QString category, categories )
            category_filter |= QContentFilter::category( category );

        QString typeLabel;
        if( types.count() == 1 )
            typeLabel = tr("Type: %1").arg( types.first() );
        else if( types.count() > 1 )
            typeLabel = tr("Type: %1").arg( tr( "(Multi)" ) );

        QString categoryLabel;
        if( categories.count() == 1 )
        {
            if( categories.first() == QLatin1String( "Unfiled" ) )
            {
                categoryLabel = tr("Category: %1").arg( tr( "Unfiled" ) );
            }
            else
            {
                QCategoryManager manager;

                categoryLabel = tr("Category: %1").arg( manager.label( categories.first() ) );
            }
        }
        else if( categories.count() > 1 )
            categoryLabel = tr("Category: %1").arg( tr( "(Multi)" ) );


        image_set = new QContentSet( QContentSet::Asynchronous, this );

        image_set->setCriteria(
                QContentFilter( QContent::Document )
            & category_filter
            & (type_filter.isValid() ? type_filter : QContentFilter::mimeType( QLatin1String( "image/*" ) )) );


        int iconSize = (36 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;

        image_model = new ThumbnailContentSetModel( image_set, this );
        image_model->setThumbnailSize( QSize( iconSize, iconSize ) );

        image_model->setView( selector_view = new ContentThumbnailView );

        selector_view->setModel( image_model );
        selector_view->setViewMode( QListView::IconMode );
        selector_view->setIconSize( QSize( iconSize, iconSize ) );
        selector_view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        selector_view->setFrameStyle( QFrame::NoFrame );
        selector_view->setResizeMode(QListView::Adjust);
        selector_view->setSelectionMode( QAbstractItemView::SingleSelection );
        selector_view->setSelectionBehavior( QAbstractItemView::SelectItems );
        selector_view->setUniformItemSizes( true );
        selector_view->setItemDelegate( new ContentThumbnailDelegate( selector_view ) );

        connect(selector_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentChanged(QModelIndex,QModelIndex)));

        connect( image_model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                 this,          SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );

        connect( image_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                 this,          SLOT(rowsInserted(QModelIndex,int,int)) );

        QMenu *selector_menu = QSoftMenuBar::menuFor( selector_view );

        m_selectorActions = new QActionGroup(this);

        m_selectorActions->addAction(selector_menu->addAction(
                QIcon(":icon/slideshow"), tr("Slide Show..."), this, SLOT(launchSlideShowDialog())));
        m_selectorActions->addAction(selector_menu->addSeparator());
#ifndef QTOPIA_HOMEUI
        m_selectorActions->addAction(m_selectorEditAction = selector_menu->addAction(
                QIcon(":icon/edit"), tr("Edit"), this, SLOT(editCurrentSelection())));
#endif
        m_selectorActions->addAction(selector_menu->addAction(
                QIcon(":icon/info"), tr("Properties"), this, SLOT(launchPropertiesDialog())));
        m_selectorActions->addAction(selector_menu->addAction(
                QIcon(":icon/beam"), tr("Send"), this, SLOT(beamImage())));
        if (!QtopiaService::apps(QLatin1String("Print")).isEmpty()) {
            m_selectorActions->addAction(selector_menu->addAction(
                    QIcon(":icon/print"), tr("Print"), this, SLOT(printImage())));
        }
        m_selectorActions->addAction(selector_menu->addAction(
                QIcon(":icon/trash"), tr("Delete"), this, SLOT(deleteImage())));
        m_selectorActions->addAction(selector_menu->addAction(
                QIcon(":icon/home"), tr("Set as Background"), this, SLOT(setHomeScreenImage())));

        if (!QtopiaService::apps(QLatin1String("Contacts")).isEmpty()) {
            m_selectorActions->addAction(selector_menu->addAction(
                    tr("Save to Contact..."), this, SLOT(setContactImage())));
            m_selectorActions->addAction(selector_menu->addAction(
                    tr("Set as Avatar"), this, SLOT(setPersonalImage())));
        }
        m_selectorActions->addAction(selector_menu->addSeparator());

        m_selectorActions->setVisible(false);

        selector_menu->addAction( tr( "View Type..." ), this, SLOT(selectType()) );
        selector_menu->addAction( QIcon( QLatin1String( ":icon/viewcategory" ) ), tr( "View Category..." ), this, SLOT(selectCategory()) );

#ifndef QTOPIA_HOMEUI
        connect( selector_menu, SIGNAL(aboutToShow()), this, SLOT(selectorMenuAboutToShow()) );
#endif

        connect( selector_view, SIGNAL(activated(QModelIndex)),
                 this, SLOT(imageSelected(QModelIndex)) );

        type_label = new QLabel( typeLabel );
        type_label->setVisible( !typeLabel.isEmpty() );

        category_label = new QLabel( categoryLabel );
        category_label->setVisible( !categoryLabel.isEmpty() );

        QVBoxLayout *layout = new QVBoxLayout;

        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( selector_view );
        layout->addWidget( type_label );
        layout->addWidget( category_label );

        selector_widget = new QWidget;
        selector_widget->setLayout( layout );
        selector_widget->setFocusProxy( selector_view );

        widget_stack->addWidget( selector_widget );
    }

    return selector_widget;
}

ImageUI *PhotoEditUI::imageEditor()
{
    if( !image_io )
    {
        // Construct image io
        image_io = new ImageIO( this );

        // Construct image processor
        image_processor = new ImageProcessor( image_io, this );

        // Construct image_ui or editor ui
        image_ui = new ImageUI(image_processor);

        image_ui->setObjectName( QLatin1String( "editmode" ) );

        editor_stack = new QStackedLayout;

        // Construct navigator
        navigator = new Navigator( image_ui );

        editor_stack->addWidget( navigator );

        // Construct region selector
        region_selector = new RegionSelector( image_ui );
        region_selector->setEnabled( true );
        region_selector->setObjectName( QLatin1String( "dimensions" ) );

        if( Qtopia::mousePreferred() )
            QSoftMenuBar::menuFor( region_selector );

        editor_stack->addWidget( region_selector );

        connect( region_selector, SIGNAL(selected()),
            this, SLOT(cropImage()) );
        connect( region_selector, SIGNAL(selected()),
            this, SLOT(exitCurrentEditorState()) );

        // Construct brightness control
        brightness_slider = new Slider( -70, 70, 0, 0, 0 );

        connect( brightness_slider, SIGNAL(selected()),
            this, SLOT(exitCurrentEditorState()) );
        connect( brightness_slider, SIGNAL(valueChanged(int)),
            this, SLOT(setBrightness(int)) );


        QVBoxLayout *brightness_layout = new QVBoxLayout;
        brightness_layout->addStretch();
        brightness_layout->addWidget( brightness_slider );

        brightness_widget = new QWidget;
        brightness_widget->setLayout( brightness_layout );
        brightness_widget->setObjectName( QLatin1String( "brightness" ) );
        brightness_widget->installEventFilter(this);

        editor_stack->addWidget( brightness_widget );

        // Construct zoom control
        zoom_slider = new Slider( 0, 0, 1, 0, 0 );

        connect( zoom_slider, SIGNAL(selected()),
            this, SLOT(exitCurrentEditorState()) );
        connect( zoom_slider, SIGNAL(valueChanged(int)),
            this, SLOT(setZoom(int)) );

        QVBoxLayout *zoom_layout = new QVBoxLayout;
        zoom_layout->addStretch();
        zoom_layout->addWidget( zoom_slider );

        zoom_widget = new QWidget;
        zoom_widget->setLayout( zoom_layout );
        zoom_widget->installEventFilter(this);

        editor_stack->addWidget( zoom_widget );

        image_ui->setLayout( editor_stack );

        // Clear context bar
        QSoftMenuBar::setLabel( image_ui, Qt::Key_Select, QLatin1String( ":icon/view" ), tr( "Zoom" ) );

        // Construct context menu for image ui
        QMenu *context_menu = QSoftMenuBar::menuFor( image_ui );
        QSoftMenuBar::setHelpEnabled( image_ui, true );

        context_menu->addAction( QIcon( ":icon/cut" ), tr( "Crop" ), this, SLOT(enterCrop()) );
        context_menu->addAction( QIcon( ":icon/color" ), tr( "Brightness" ), this, SLOT(enterBrightness()) );
        context_menu->addAction( QIcon( ":icon/rotate" ), tr( "Rotate" ), image_processor, SLOT(rotate()) );
        context_menu->addSeparator();
        context_menu->addAction( QIcon( ":icon/view" ), tr( "Zoom" ), this, SLOT(enterZoom()) );

        QAction *fullScreenAction = context_menu->addAction(
                QIcon(":icon/fullscreen"), tr("Full Screen"), this, SLOT(enterFullScreen()));

        connect(this, SIGNAL(fullScreenDisabled(bool)), fullScreenAction, SLOT(setVisible(bool)));

        context_menu->addSeparator();
        context_menu->addAction(QIcon(":icon/save"), tr("Save As..."), this, SLOT(saveImageAs()));
        context_menu->addAction( QIcon( ":icon/cancel" ), tr( "Cancel" ), this, SLOT(cancelEdit()) );

        widget_stack->addWidget(image_ui);
    }

    return image_ui;
}

void PhotoEditUI::timerEvent( QTimerEvent *event )
{
    if (event->timerId() == list_init_timer_id) {
        killTimer( list_init_timer_id );

        list_init_timer_id = -1;

        QWidget *selector = selectorWidget();

        widget_stack->setCurrentWidget( selector );

        navigation_stack.append( selector );

        selector->setFocus();

        event->accept();
    }
}

bool PhotoEditUI::event( QEvent *event )
{
    if( event->type() == QEvent::WindowDeactivate && is_fullscreen )
    {
        lower();
    }
    else if( event->type() == QEvent::WindowActivate && is_fullscreen )
    {
        QString title = windowTitle();

        setWindowTitle( QLatin1String( "_allow_on_top_" ) );

        raise();

        setWindowTitle( title );
    }

     return QWidget::event( event );
}

void PhotoEditUI::setDocument( const QString& lnk )
{
    if( list_init_timer_id != -1 )
    {
        killTimer( list_init_timer_id );

        list_init_timer_id = -1;
    }

    viewImage( QContent( lnk, false ) );
}

void PhotoEditUI::editImage( const QDSActionRequest& request )
{
    if( list_init_timer_id != -1 )
    {
        killTimer( list_init_timer_id );

        list_init_timer_id = -1;
    }
    else
        hide();

    showMaximized();
    currEditImageRequest = new QDSActionRequest( request );
    QDataStream stream( currEditImageRequest->requestData().toIODevice() );
    QPixmap orig;
    stream >> orig;
    service_image = orig.toImage();
    clearEditor();
    service_requested = true;

    if (!navigation_stack.isEmpty() && navigation_stack.last() == m_imageWidget) {
        current_image = m_imageScaler->content();

        setViewerImage(QContent());
    }

    enterEditor();
}

void PhotoEditUI::imageSelected( const QModelIndex &index )
{
    viewImage( qvariant_cast< QContent >( index.data( Qt::UserRole + 1 ) ) );
}

void PhotoEditUI::viewImage( const QContent &content )
{
    QWidget *viewer = imageViewer();

    setViewerImage(content);

    widget_stack->setCurrentWidget( viewer );

    navigation_stack.append( viewer );
}

void PhotoEditUI::setViewerImage(const QContent &content)
{
    m_viewerWaitWidget->show();

    m_imageScaler->setContent(content);
}

void PhotoEditUI::selectType()
{
    if( !type_dialog )
    {
        QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

        QStringList types = settings.value( QLatin1String( "Types" ) ).toString().split( QLatin1Char( ';' ), QString::SkipEmptyParts );

        QContentFilterModel::Template typePage;

        typePage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        typePage.addList( QContentFilter::MimeType, QString(), types );

        type_dialog = new QContentFilterDialog( typePage, this );

        type_dialog->setWindowTitle( tr("View Type") );
        type_dialog->setFilter( QContentFilter( QContent::Document ) & QContentFilter::mimeType( QLatin1String( "image/*" ) ) );
        type_dialog->setObjectName( QLatin1String( "documents-type" ) );
    }

    QtopiaApplication::execDialog( type_dialog );

    type_filter = type_dialog->checkedFilter();

    image_set->setCriteria(
            QContentFilter( QContent::Document )
          & category_filter
          & (type_filter.isValid() ? type_filter : QContentFilter::mimeType( QLatin1String( "image/*" ) )) );

    QString label = type_dialog->checkedLabel();

    if( !type_filter.isValid() || label.isEmpty() )
    {
        type_label->setVisible( false );
    }
    else
    {
        type_label->setText( tr("Type: %1").arg( label ) );
        type_label->setVisible( true );
    }

    QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

    settings.setValue( QLatin1String( "Types" ), type_filter.arguments( QContentFilter::MimeType ).join( QLatin1String( ";" ) ) );
}

void PhotoEditUI::selectCategory()
{
    if( !category_dialog )
    {
        QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

        QStringList categories = category_filter.arguments( QContentFilter::Category );

        QContentFilterModel::Template categoryPage;

        categoryPage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        categoryPage.addList( QContentFilter::Category, QString(), categories );
        categoryPage.addList( QContentFilter::Category, QLatin1String( "Documents" ), categories );

        category_dialog = new QContentFilterDialog( categoryPage, this );

        category_dialog->setWindowTitle( tr("View Category") );
        category_dialog->setFilter( QContentFilter( QContent::Document ) & QContentFilter::mimeType( QLatin1String( "image/*" ) ) );
        category_dialog->setObjectName( QLatin1String( "documents-category" ) );
    }

    QtopiaApplication::execDialog( category_dialog );

    category_filter = category_dialog->checkedFilter();

    image_set->setCriteria(
            QContentFilter( QContent::Document )
          & category_filter
          & (type_filter.isValid() ? type_filter : QContentFilter::mimeType( QLatin1String( "image/*" ) )) );

    QString label = category_dialog->checkedLabel();

    if( !category_filter.isValid() || label.isEmpty() )
    {
        category_label->setVisible( false );
    }
    else
    {
        category_label->setText( tr("Category: %1").arg( label ) );
        category_label->setVisible( true );
    }

    QSettings settings( QLatin1String( "Trolltech" ), QLatin1String( "photoedit" ) );

    settings.setValue( QLatin1String( "Categories" ), category_filter.arguments( QContentFilter::Category ).join( QLatin1String( ";" ) ) );
}

void PhotoEditUI::selectEffect()
{
    if (!m_effectDialog) {
        m_effectDialog = new EffectDialog(this);
        m_effectDialog->setWindowModality(Qt::WindowModal);

        connect(m_effectDialog, SIGNAL(effectSelected(QString,QString,QMap<QString,QVariant>)),
                m_imageScaler, SLOT(setEffect(QString,QString,QMap<QString,QVariant>)));
    }

    QtopiaApplication::showDialog(m_effectDialog);
}

void PhotoEditUI::zoomViewer()
{
    m_viewerZoomSlider->setVisible(true);

    m_viewerZoomSlider->setEditFocus(true);
}

void PhotoEditUI::zoomViewerToActualSize()
{
    emit scaleViewer(1.0, 1.0);

    int value = m_viewerZoomSlider->minimum();

    for (; q_imageZoomScales[value] < 1.0; ++value);

    m_viewerZoomSlider->setValue(value);
}

void PhotoEditUI::zoomViewerToScreenSize()
{
    QSize viewSize = image_viewer->size();
    QSize desktopSize = QApplication::desktop()->availableGeometry(image_viewer).size();

    viewSize = QSize(
            qMin(viewSize.width(), desktopSize.width()),
            qMin(viewSize.height(), desktopSize.height()));

    QSize imageSize = m_imageScaler->size();

    int value = m_viewerZoomSlider->minimum();

    for (; value < m_viewerZoomSlider->maximum()
        && q_imageZoomScales[value] < 1.0
        && q_imageZoomScales[value] * imageSize.width() < viewSize.width()
        && q_imageZoomScales[value] * imageSize.height() < viewSize.height();
        ++value);

    m_viewerZoomSlider->setValue(value);

    emit setViewerScaleMode(ImageViewer::ScaleToFit);
}

void PhotoEditUI::setViewerZoom(int zoom)
{
    emit scaleViewer(q_imageZoomScales[zoom], q_imageZoomScales[zoom]);
}

void PhotoEditUI::updateViewerZoomRange()
{
    QSize viewSize = image_viewer->size();
    QSize desktopSize = QApplication::desktop()->size();

    viewSize = QSize(
            qMin(viewSize.width(), desktopSize.width()),
            qMin(viewSize.height(), desktopSize.height()));

    QSize imageSize = m_imageScaler->size();

    QPair<int, int> range = calculateZoomRange(imageSize, viewSize);

    m_viewerZoomSlider->setRange(range.first, range.second);
}

QPair<int, int> PhotoEditUI::calculateZoomRange(const QSize &imageSize, const QSize &viewSize) const
{
    int minimum = 0;

    for (; q_imageZoomScales[minimum] < 1.0
        && q_imageZoomScales[minimum + 1] * imageSize.width() < viewSize.width()
        && q_imageZoomScales[minimum + 1] * imageSize.height() < viewSize.height();
        ++minimum);

    int maximum = minimum;

    for (; q_imageZoomScales[maximum] < 1.0; ++maximum);

    for (; maximum + 1 < int(sizeof(q_imageZoomScales) / sizeof(qreal))
        && q_imageZoomScales[maximum + 1] * imageSize.width() < 4096
        && q_imageZoomScales[minimum + 1] * imageSize.height() < 4096;
        ++maximum);

    return qMakePair(minimum, maximum);
}

void PhotoEditUI::viewerImageChanged()
{
    updateViewerZoomRange();
    zoomViewerToScreenSize();

    QContent content = m_imageScaler->content();

#ifdef QTOPIA_HOMEUI
    m_imageCaption->setText(content.name());
#else
    if (content.isNull())
        setWindowTitle(tr("Pictures"));
    else
        setWindowTitle(content.name());
#endif
}

void PhotoEditUI::viewerTapped()
{
    if (is_fullscreen)
        emit showFullScreenWidgets(m_fullScreenWidgetsVisible = !m_fullScreenWidgetsVisible);
}

#ifndef QTOPIA_HOMEUI
void PhotoEditUI::viewerMenuAboutToShow()
{
    m_viewerEditAction->setEnabled(m_imageScaler->content().drmState() != QContent::Protected);
}

void PhotoEditUI::selectorMenuAboutToShow()
{
    m_selectorEditAction->setEnabled(qvariant_cast<QContent>(selector_view->currentIndex().data(
            QContentSetModel::ContentRole)).drmState() != QContent::Protected);
}
#endif
void PhotoEditUI::appMessage( const QString& msg, const QByteArray& data )
{
    if( msg == "getImage(QString,QString,int,int,QString)" ) {
        QDataStream stream( data );
        QString filename;
        stream >> service_channel >> service_id >> service_width >> service_height >> filename;
        if ( filename.isEmpty() ) {
            service_image = QImage();
        } else {
            service_image = QImage( filename );
            QFile::remove( filename );
        }

        QTimer::singleShot( 0, this, SLOT(processGetImage()) );
    }
}

void PhotoEditUI::enterSlideShow()
{
    if ( !selector_view ) {
        qWarning("PhotoEditUI::enterSlideShow() being called when there is NO image selector.");
        return;
    }

    // Set slide show collection from currently visible collection in selector
    slide_show->setCollection( *image_set );
    // Set first image in slideshow to currently selected image in selector
    slide_show->setFirstImage( qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) ) );

    widget_stack->setCurrentWidget( slide_show_ui );

    navigation_stack.append( slide_show_ui );

    // Start slideshow
    slide_show->start();
    QtopiaApplication::setPowerConstraint( QtopiaApplication::Disable );
}

void PhotoEditUI::enterEditor()
{
#define LIMIT( X, Y, Z ) ( (X) > (Y) ? (X) > (Z) ? (Z) : (X) : (Y) )
#define REDUCTION_RATIO( dw, dh, sw, sh ) \
    ( (dw)*(sh) > (dh)*(sw) ? (double)(dh)/(double)(sh) : \
    (double)(dw)/(double)(sw) )

    ImageUI *editor = imageEditor();

    editor->setEnabled( false );

    // Raise editor to top of widget stack
    widget_stack->setCurrentWidget( editor );

    navigation_stack.append( editor );

    // Update image io with current image
    ImageIO::Status status;
    if( service_requested && !service_image.isNull() ) {
        status = image_io->load( service_image );
    } else if ( !current_image.isValid() ) {
        // I believe this is because it is possible to get in here before
        // an image has been established.
        status = ImageIO::LOAD_ERROR;
    } else {
        status = image_io->load(current_image);
    }

    switch( status ) {
    case ImageIO::REDUCED_SIZE:
        {
            QMessageBox mb(
                    QMessageBox::Information,
                    tr("Scaled Image"),
                    tr("<qt>Unable to load complete image.  A scaled copy has been opened instead.</qt>"),
                    QMessageBox::Ok);

            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();
        }
    case ImageIO::NORMAL:
        {
            // Initialize editor controls
            brightness_slider->setValue( 0 );
            // Zoom to fit image in screen
            QSize size = image_io->size();
            QSize view = image_ui->size();
            disconnect( zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)) );

            QPair<int, int> range = calculateZoomRange(size, view);

            zoom_slider->setRange(range.first, range.second);

            int value = zoom_slider->minimum();

            for (; value < zoom_slider->maximum()
                && q_imageZoomScales[value] < 1.0
                && q_imageZoomScales[value] * size.width() < view.width()
                && q_imageZoomScales[value] * size.height() < view.height();
                ++value);

            zoom_slider->setValue(value);

            if( size.width() > view.width() || size.height() > view.height() ) {
                double ratio = REDUCTION_RATIO( view.width(), view.height(), size.width(), size.height() );
                ratio = LIMIT( ratio, 0.1, 1.0 );
                image_processor->setZoom( ratio );
            } else {
                double ratio = REDUCTION_RATIO( view.width(), view.height(), size.width(), size.height() );
                ratio = LIMIT( ratio, 1.0, 10.0 );
            }

            connect( zoom_slider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)) );
            image_ui->reset();
            image_ui->setEnabled( true );
            editor_stack->setCurrentWidget( navigator );
        }
        break;

    case ImageIO::SIZE_ERROR:
    case ImageIO::LOAD_ERROR:
        if (status == ImageIO::LOAD_ERROR) {
            QMessageBox mb(
                    QMessageBox::Warning,
                    tr("Load Error"),
                    tr("<qt>Unable to load image.</qt>"),
                    QMessageBox::Ok);

            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();
        } else if (image_io->size().isValid()) {
            QMessageBox mb(
                    QMessageBox::Warning,
                    tr("Size Error"),
                    tr("<qt>Unable to edit image of size %1x%2.  "
                        "The maximum editable size is %3x%4</qt>",
                        "%1x%2 = image width and height, %3x%4 = maximum width and height")
                        .arg(image_io->size().width())
                        .arg(image_io->size().height())
                        .arg(ImageIO::maxSize().width())
                        .arg(ImageIO::maxSize().height()),
                    QMessageBox::Ok);

            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();
        } else {
            QMessageBox mb(
                    QMessageBox::Warning,
                    tr("Size Error"),
                    tr("<qt>Unable to edit image.  The maximum editable size is %1x%2</qt>",
                        "%1x%2 = maximum width and height")
                        .arg(ImageIO::maxSize().width())
                        .arg(ImageIO::maxSize().height()),
                    QMessageBox::Ok);

            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();
        }
        navigation_stack.removeLast();
        if( !navigation_stack.isEmpty() )
        {
            if (navigation_stack.last() == m_imageWidget)
                setViewerImage(current_image);

            widget_stack->setCurrentWidget( navigation_stack.last() );
        }
        else
            close();
        break;
    case ImageIO::DEPTH_ERROR:
        {
            QMessageBox mb(
                    QMessageBox::Warning,
                    tr("Depth Error"),
                    tr("<qt>Image depth is not supported.</qt>"),
                    QMessageBox::Ok);
            mb.setEscapeButton(QMessageBox::Ok);
            mb.exec();

            navigation_stack.removeLast();
            if( !navigation_stack.isEmpty() )
            {
                if (navigation_stack.last() == m_imageWidget)
                    setViewerImage(current_image);

                widget_stack->setCurrentWidget( navigation_stack.last() );
            }
            else
                close();
            break;
        }
    }
}

void PhotoEditUI::enterZoom()
{
    editor_stack->setCurrentWidget( zoom_widget );
}

void PhotoEditUI::enterBrightness()
{
    editor_stack->setCurrentWidget( brightness_widget );
}

void PhotoEditUI::enterCrop()
{
    // Enable selection in region selector
    editor_stack->setCurrentWidget( region_selector );

    region_selector->reset();
}

void PhotoEditUI::enterFullScreen()
{
    // Show editor view in full screen
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setWindowState(Qt::WindowFullScreen);

    raise();

    emit fullScreenDisabled(false);

    is_fullscreen = true;
}

void PhotoEditUI::exitFullScreen()
{
    is_fullscreen = false;

    emit showFullScreenWidgets(m_fullScreenWidgetsVisible = false);
    emit fullScreenDisabled(true);

    QTimer::singleShot(0, this, SLOT(showMaximized()));
}

void PhotoEditUI::setViewThumbnail()
{
    if ( !selector_view ) {
        qWarning("PhotoEditUI::setViewThumbnail() being called when there is NO image selector.");
        return;
    }

    // If image selector not in multi, change to single and update context menu
    widget_stack->setCurrentWidget( selector_widget );
}

void PhotoEditUI::setViewSingle()
{
    if ( !selector_view ) {
        qWarning("PhotoEditUI::setViewSingle() being called when there is NO image selector.");
        return;
    }

    // If image selector not in single, change to single and update context menu
    widget_stack->setCurrentWidget(m_imageWidget);
}

void PhotoEditUI::launchSlideShowDialog()
{
    if ( !slide_show_dialog ) {
        slide_show_dialog = new SlideShowDialog( this );
        slide_show_dialog->setObjectName( QLatin1String( "slideshow" ) );
    }

    // If slide show dialog accepted, start slideshow
    if( QtopiaApplication::execDialog( slide_show_dialog, true ) ) {
        if ( !slide_show ) {
            imageViewer();

            slide_show = new SlideShow( this );
            slide_show_ui = new SlideShowUI(m_imageScaler, this);
            slide_show_ui->setWindowTitle( windowTitle() );

            widget_stack->addWidget( slide_show_ui );

            // Update image when slide show has changed
            connect(slide_show, SIGNAL(changed(QContent)),
                    m_imageScaler, SLOT(setContent(QContent)));
            connect(m_imageScaler, SIGNAL(imageChanged()),
                    slide_show, SLOT(imageLoaded()));
            // Stop slide show when slide show ui pressed
            connect( slide_show_ui, SIGNAL(pressed()), slide_show, SLOT(stop()) );
            // Show selector when slide show has stopped
            connect( slide_show, SIGNAL(stopped()), this, SLOT(exitCurrentUIState()) );
        }
        // Set slide show options
        slide_show_ui->setDisplayName( slide_show_dialog->isDisplayName() );
        slide_show->setSlideLength( slide_show_dialog->slideLength() );
        slide_show->setLoopThrough( slide_show_dialog->isLoopThrough() );
        enterSlideShow();
    }
}

void PhotoEditUI::launchPropertiesDialog()
{
    QContent image;

    if (widget_stack->currentWidget() == m_imageWidget)
        image = m_imageScaler->content();
    else if( widget_stack->currentWidget() == selector_widget )
        image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );
    else
        return;

    QDocumentPropertiesDialog dialog( image );
    dialog.setObjectName( QLatin1String( "properties" ) );
    // Launch properties dialog with current image
    QtopiaApplication::execDialog( &dialog );
}

bool PhotoEditUI::exitCurrentUIState()
{
    if( navigation_stack.isEmpty() )
        return true;

    if (widget_stack->currentWidget() == m_imageWidget) {
        if (m_viewerZoomSlider->isVisible()) {
            m_viewerZoomSlider->setVisible(false);
        } else {
            setViewerImage(QContent());

            navigation_stack.removeLast();
        }
    } else {
        navigation_stack.removeLast();

        if (widget_stack->currentWidget() == image_ui) {
            if (service_requested) {
                if (!edit_canceled) {
                    sendValueSupplied();
                }
                setWindowState(windowState() | Qt::WindowMinimized);
            } else if (!edit_canceled) {
                saveChanges();
            }
            edit_canceled = false;

            if (!navigation_stack.isEmpty() && navigation_stack.last() == m_imageWidget)
                setViewerImage(current_image);
        } else if( widget_stack->currentWidget() == slide_show_ui ) {
            QtopiaApplication::setPowerConstraint( QtopiaApplication::Enable );
        }
    }

    if (!navigation_stack.isEmpty()) {
        widget_stack->setCurrentWidget(navigation_stack.last());

        return false;
    } else {
        return true;
    }
}

void PhotoEditUI::exitCurrentEditorState()
{
    if (editor_stack->currentWidget() != navigator) {
        if (editor_stack->currentWidget() == region_selector)
            region_selector->update();

        editor_stack->setCurrentWidget(navigator);
    }
}

void PhotoEditUI::setZoom( int x )
{
    image_processor->setZoom(q_imageZoomScales[x]);
}

void PhotoEditUI::setBrightness( int x )
{
    image_processor->setBrightness( (double)x / 100.0 );
}

void PhotoEditUI::editCurrentSelection()
{
    if (widget_stack->currentWidget() == m_imageWidget)
    {
        current_image = m_imageScaler->content();

        setViewerImage(QContent());

        enterEditor();
    }
    else if( widget_stack->currentWidget() == selector_widget )
    {
        current_image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );

        enterEditor();
    }
}

void PhotoEditUI::cancelEdit()
{
    if( is_fullscreen ) {
        exitFullScreen();
    } else {
        edit_canceled = true;

        if( exitCurrentUIState() )
            close();
    }
}

void PhotoEditUI::saveImageAs()
{
    QContent content;
    content.setName(current_image.name());
    content.setType(image_io->saveType());
    content.setMedia(current_image.media());
    content.setCategories(current_image.categories());

    QDocumentPropertiesDialog properties(content, this);

    if (QtopiaApplication::execDialog(&properties)) {
        content = properties.document();

        if (saveImage(image_processor->image(), &content)) {
            image_processor->setCheckpoint();

            m_savedImage = current_image;
        } else {
            content.removeFiles();
        }
    }
}

void PhotoEditUI::cropImage()
{
    // Ensure cropping region is valid
    QRect region( region_selector->region() );
    if( region.isValid() ) {
        image_processor->crop(region);
        // Reset viewport
        image_ui->reset();
    }
}

void PhotoEditUI::beamImage()
{
    // Send current image over IR link
    QContent image;

    if (widget_stack->currentWidget() == m_imageWidget)
        image = m_imageScaler->content();
    else if( widget_stack->currentWidget() == selector_widget )
        image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );
    else
        return;

    QtopiaSendVia::sendFile(this, image);
}

void PhotoEditUI::printImage()
{
    QContent image;

    if (widget_stack->currentWidget() == m_imageWidget)
        image = m_imageScaler->content();
    else if( widget_stack->currentWidget() == selector_widget )
        image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );
    else
        return;

    QtopiaServiceRequest srv( "Print", "print(QString,QString)" );
    srv << image.fileName();
    srv << (image.mimeTypes().count() ? image.mimeTypes().at(0) : QString());
    srv.send();
}

void PhotoEditUI::deleteImage()
{
    // Retrieve currently highlighted image from selector
    QContent image;

    if (widget_stack->currentWidget() == m_imageWidget)
        image = m_imageScaler->content();
    else if( widget_stack->currentWidget() == selector_widget )
        image = qvariant_cast< QContent >( selector_view->currentIndex().data( Qt::UserRole + 1 ) );
    else
        return;

    // Launch confirmation dialog
    // If deletion confirmed, delete image
    QMessageBox mb(
            QMessageBox::Warning,
            tr("Delete"),
            tr("<qt>Are you sure you want to delete %1?</qt>"," %1 = file name").arg(image.name()),
            QMessageBox::Yes | QMessageBox::No);

    mb.setEscapeButton(QMessageBox::No);

    if( mb.exec() == QMessageBox::Yes ) {
        image.removeFiles();

        if (widget_stack->currentWidget() == m_imageWidget && exitCurrentUIState()) {
            close();
        }
    }
}

#define HOMESCREEN_IMAGE_NAME QLatin1String(".HomescreenImage")
#define HOMESCREEN_IMAGE_PATH Qtopia::documentDir() + HOMESCREEN_IMAGE_NAME

void PhotoEditUI::setHomeScreenImage()
{
    // Retrieve currently highlighted image from selector
    QContent image;

    if (widget_stack->currentWidget() == m_imageWidget)
        image = m_imageScaler->content();
    else if (widget_stack->currentWidget() == selector_widget)
        image = qvariant_cast<QContent>(selector_view->currentIndex().data(Qt::UserRole + 1));

    if (image.isNull())
        return;

    QString fileName;

    QImageReader reader(image.fileName());

    QSize imageSize = reader.size();
    QSize desktopSize = qApp->desktop()->size();

    if (imageSize.height() > imageSize.width() == desktopSize.height() > desktopSize.height())
        desktopSize.transpose();

    if (imageSize.height() > desktopSize.height() * 3 / 2
        && imageSize.width() > desktopSize.width() * 3 / 2) {
        if (reader.supportsOption(QImageIOHandler::ScaledSize)) {
            imageSize.scale(desktopSize, Qt::KeepAspectRatioByExpanding);

            reader.setScaledSize(imageSize);
            reader.setQuality(49);

            QImage image;

            if (reader.read(&image)) {
                QImageWriter writer(HOMESCREEN_IMAGE_PATH, "PNG");

                if (writer.write(image))
                    fileName = writer.fileName();
            }
        }
    } else if (QFileSystem::fromFileName(image.fileName()).isRemovable()) {
        fileName = HOMESCREEN_IMAGE_PATH;

        if (!copyImage(image, fileName)) {
            fileName = QString();
        }
    } else {
        fileName = image.fileName();
    }

    if (fileName.isNull()) {
        QMessageBox message(
                QMessageBox::Warning,
                tr("Background Image"),
                tr("Could not save image to permanent storage"),
                QMessageBox::Ok,
                this);
        message.setWindowModality(Qt::WindowModal);
        message.exec();
    } else {
        QSettings cfg("Trolltech","qpe");
        cfg.beginGroup("HomeScreen"); {
            cfg.setValue("HomeScreenPicture", fileName);
        }

        QtopiaChannel::send("QPE/System", "updateHomeScreenImage()");
    }
}

bool PhotoEditUI::copyImage(const QContent &image, const QString &target)
{
    bool copied = false;

    if (QIODevice *source = image.open()) {
        QFile destination(target + QLatin1String(".part"));

        if (destination.open(QIODevice::WriteOnly)) {
            char buffer[65536];

            if (source->size() > 524288) {
                while (!source->atEnd()) {
                    qint64 bytesRead = source->read(buffer, 65536);

                    if (!(copied = (destination.write(buffer, bytesRead) == bytesRead)))
                        break;
                }
            } else {
                QWaitWidget wait(this);

                wait.setCancelEnabled(true);
                wait.show();

                while (!source->atEnd()) {
                    QCoreApplication::processEvents();

                    if (wait.wasCancelled()) {
                        copied = false;
                        break;
                    }

                    qint64 bytesRead = source->read(buffer, 65536);

                    if (!(copied = (destination.write(buffer, bytesRead) == bytesRead)))
                        break;
                }
            }
            destination.close();

            if (copied)
                copied = destination.rename(target);

            if (!copied)
                destination.remove();
        }

        source->close();

        delete source;
    }

    return copied;
}


void PhotoEditUI::setContactImage()
{
    QContent image;

    if (widget_stack->currentWidget() == m_imageWidget)
        image = m_imageScaler->content();
    else if (widget_stack->currentWidget() == selector_widget)
        image = qvariant_cast<QContent>(selector_view->currentIndex().data(Qt::UserRole + 1));

    QString fileName = image.fileName();
    if (!fileName.isNull()) {
        QtopiaServiceRequest request(QLatin1String("Contacts"), QLatin1String("setContactImage(QString)"));
        request << fileName;
        request.send();
    }
}

void PhotoEditUI::setPersonalImage()
{
    QContent image;

    if (widget_stack->currentWidget() == m_imageWidget)
        image = m_imageScaler->content();
    else if (widget_stack->currentWidget() == selector_widget)
        image = qvariant_cast<QContent>(selector_view->currentIndex().data(Qt::UserRole + 1));

    QString fileName = image.fileName();

    if (!fileName.isNull()) {
        QtopiaServiceRequest request(
                QLatin1String("Contacts"),
                QLatin1String("setPersonalImage(QString)"));
        request << fileName;
        request.send();
    }
}

void PhotoEditUI::contentChanged( const QContentIdList &idList, const QContent::ChangeType type )
{
    // If content is current image and has been deleted, show selector
    if (QContent::Removed == type
        && widget_stack->currentWidget() == m_imageWidget
        && idList.contains(current_image.id())) {
        navigation_stack.removeLast();

        if( !navigation_stack.isEmpty() )
            widget_stack->setCurrentWidget( navigation_stack.last() );
        else
            close();
    }
}

void PhotoEditUI::rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
{
    if( end - start + 1 == selector_view->model()->rowCount( parent ) )
    {
        QSoftMenuBar::setLabel( selector_view, Qt::Key_Select, QSoftMenuBar::NoLabel );

        m_selectorActions->setVisible(false);
    }
}

void PhotoEditUI::rowsInserted( const QModelIndex &parent, int start, int end )
{
    if( end - start + 1 == selector_view->model()->rowCount( parent ) )
    {
        QSoftMenuBar::setLabel( selector_view, Qt::Key_Select, QSoftMenuBar::View );

        m_selectorActions->setVisible(true);
    }

    if (!m_savedImage.isNull()) {
        for (int i = start; i <= end; ++i) {
            QModelIndex index = image_model->index(i, 0);

            if (image_model->content(index) == m_savedImage) {
                if (!Qtopia::mousePreferred())
                    selector_view->setCurrentIndex(index);
                selector_view->scrollTo(index);
            }
        }
    }
}

void PhotoEditUI::currentChanged(const QModelIndex &, const QModelIndex &)
{
    m_savedImage = QContent();
}

void PhotoEditUI::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Back:
        if (widget_stack->currentWidget() == image_ui
            && editor_stack->currentWidget() != navigator) {
            exitCurrentEditorState();

            e->accept();
        } else if (is_fullscreen) {
            exitFullScreen();

            e->accept();
        } else if (exitCurrentUIState()) {
            close();

            e->accept();
        }
        break;
    case Qt::Key_Select:
        if (widget_stack->currentWidget() == image_ui && editor_stack->currentWidget() == navigator) {
            enterZoom();

            e->accept();
        } else if (widget_stack->currentWidget() == m_imageWidget) {
            zoomViewer();

            e->accept();
        }
        break;
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        if (!e->isAutoRepeat() && selector_view && widget_stack->currentWidget() == m_imageWidget) {
            QModelIndex nextIndex;
            QModelIndex currentIndex = selector_view->currentIndex();

            const int nextKey = QApplication::keyboardInputDirection() == Qt::LeftToRight
                    ? Qt::Key_Left
                    : Qt::Key_Right;

            if (e->key() == nextKey || e->key() == Qt::Key_Down)
                nextIndex = currentIndex.sibling(currentIndex.row() + 1, currentIndex.column());
            else
                nextIndex = currentIndex.sibling(currentIndex.row() - 1, currentIndex.column());

            if (nextIndex.isValid()) {
                selector_view->setCurrentIndex(nextIndex);
                selector_view->scrollTo(nextIndex);

                setViewerImage(qvariant_cast<QContent>(nextIndex.data(QContentSetModel::ContentRole)));

                e->accept();
            }
        }
        break;
    default:
        QWidget::keyPressEvent( e );
    }
}

void PhotoEditUI::clearEditor()
{
    if ( image_ui ) {
        image_ui->setEnabled( false );
        image_ui->repaint();
    }
}

void PhotoEditUI::saveChanges()
{
    // If image was changed, prompt user to save changes
    if (image_processor->isChanged()) {
        QMessageBox mb(
                QMessageBox::Question,
                tr("Save Changes "),
                tr("<qt>Do you want to save your changes?</qt>"),
                QMessageBox::Yes | QMessageBox::No);
        mb.setEscapeButton(QMessageBox::Discard);

        if (mb.exec() == QMessageBox::Yes) {
            if (image_io->isReadOnly()) {
                QMessageBox mb(
                        QMessageBox::Information,
                        tr("Read-Only File"),
                        tr("<qt>Saving a copy of the read-only file.</qt>"),
                        QMessageBox::Ok);
                mb.setEscapeButton(QMessageBox::Ok);
                mb.exec();

                saveImageAs();
            } else if (!image_io->isSaveSupported()) {
                QByteArray format = image_io->format();

                QMessageBox mb(
                        QMessageBox::Warning,
                        tr( "Saving %1" ).arg(format.constData()),
                        tr("<qt>Saving as %1 is not supported. Using the default format instead.</qt>"),
                        QMessageBox::Ok);
                mb.setEscapeButton(QMessageBox::Ok);
                mb.exec();

                saveImageAs();
            } else {
                // Attempt to save changes
                saveImage(image_processor->image(), &current_image);
            }
        }
    }
}

bool PhotoEditUI::saveImage(const QImage &image, QContent *content)
{
    if (image_io->save(image, content)) {
        current_image = *content;

        return true;
    } else {
        QMessageBox mb(
                QMessageBox::Warning,
                tr("Save failed"),
                tr("<qt>Your edits were not saved.</qt>"),
                QMessageBox::Ok);
        mb.setEscapeButton(QMessageBox::Ok);
        mb.exec();

        return false;
    }
}

void PhotoEditUI::sendValueSupplied()
{
    if ( currEditImageRequest != 0 ) {
        // Create edited picture data object
        QByteArray editedArray;
        {
            QDataStream stream( &editedArray, QIODevice::WriteOnly );
            stream << QPixmap::fromImage( image_processor->image() );
        }
        QDSData edited( editedArray, QMimeType( "image/x-qpixmap" ) );

        // Respond to request, and cleanup the request
        currEditImageRequest->respond( edited );
        delete currEditImageRequest;
        currEditImageRequest = 0;
    } else {
        QtopiaIpcEnvelope e( service_channel, "valueSupplied(QString,QString)" );
        QString path = Qtopia::applicationFileName("Temp", service_id);
        QImage img = image_processor->image( QSize( service_width, service_height ) );
        img.save(path,"JPEG");
        e << service_id << path;
    }
}

void PhotoEditUI::tvScreenChanged()
{
    if (!m_tvScreen)
        return;

    if (m_tvScreen->isVisible() && m_tvScreen->clonedScreen() == -1) {
        // The separate TV display mode has been enabled and the video cable is present.
        QRect rect = QApplication::desktop()->availableGeometry(m_tvScreen->screenNumber());

        ImageViewer *viewer = tvImageViewer();
        viewer->setGeometry(rect);
        viewer->show();
    } else {
        // The video cable was unplugged or the separate TV display mode is not enabled.
        if (tv_image_viewer) {
            tv_image_viewer->hide();
        }
    }
}

ImageViewer *PhotoEditUI::tvImageViewer()
{
    if (!tv_image_viewer) {
        imageViewer();

        tv_image_viewer = new ImageViewer(m_imageScaler);
        tv_image_viewer->setScaleMode(ImageViewer::ScaleToFit);
        tv_image_viewer->setWindowState(Qt::WindowFullScreen);
        tv_image_viewer->setFocusPolicy(Qt::NoFocus);
        tv_image_viewer->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tv_image_viewer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tv_image_viewer->setWindowTitle(QLatin1String("_ignore_"));
        tv_image_viewer->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

        connect(this, SIGNAL(scaleViewer(qreal,qreal)),
                tv_image_viewer, SLOT(setScale(qreal,qreal)));
        connect(this, SIGNAL(setViewerScaleMode(ImageViewer::ScaleMode)),
                tv_image_viewer, SLOT(setScaleMode(ImageViewer::ScaleMode)));

        connect(image_viewer->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                tv_image_viewer->horizontalScrollBar(), SLOT(setValue(int)));
        connect(image_viewer->verticalScrollBar(), SIGNAL(valueChanged(int)),
                tv_image_viewer->verticalScrollBar(), SLOT(setValue(int)));
    }
    return tv_image_viewer;
}

void PhotoEditUI::stackItemChanged(int item)
{
    // Switch the television into the separate display mode if
    // we are showing the image widget or slide show UI.  Otherwise
    // clone the main LCD display onto the television for navigation.
    QWidget *widget = widget_stack->widget(item);
    if (!widget || !m_tvScreen)
        return;
    if (widget == m_imageWidget || widget == slide_show_ui) {
        m_tvScreen->setClonedScreen(-1);
    } else {
        m_tvScreen->setClonedScreen(QApplication::desktop()->primaryScreen());
    }
}

/*!
    \service PhotoEditService PhotoEdit
    \inpublicgroup QtEssentialsModule
    \brief The PhotoEditService class provides the PhotoEdit service.

    The \i PhotoEdit service enables applications to access features
    within the photo editing application.
*/

/*!
    \internal
*/
PhotoEditService::~PhotoEditService()
{
}

/*!
    Display the photo editor's main document list and show the documents
    within \a category.

    This slot corresponds to the QCop service message
    \c{PhotoEdit::showCategory(QString)}.
*/
void PhotoEditService::showCategory( const QString& category )
{
    mParent->hide();
    mParent->showMaximized();

    while( !mParent->exitCurrentUIState() );

    if( mParent->category_dialog )
    {
        delete mParent->category_dialog;

        mParent->category_dialog = 0;
    }

    QWidget *selector = mParent->selectorWidget();

    mParent->navigation_stack.append( selector );

    mParent->widget_stack->setCurrentWidget( selector );

    mParent->category_filter = QCategoryFilter(category);

    mParent->image_set->setCriteria(
            QContentFilter( QContent::Document )
            & mParent->category_filter
            & (mParent->type_filter.isValid() ? mParent->type_filter : QContentFilter::mimeType( QLatin1String( "image/*" ) )) );
}

/*!
    Allows users to edit the image contained within \a request using the photo
    editor dialog.

    This slot corresponds to a QDS service with a request and response data type
    of "image/x-qpixmap".

    This slot corresponds to the QCop service message
    \c{PhotoEdit::editImage(QDSActionRequest)}.
*/
void PhotoEditService::editImage( const QDSActionRequest& request )
{
    mParent->editImage( request );
}
