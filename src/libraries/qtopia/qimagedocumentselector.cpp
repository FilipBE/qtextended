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

/*!
  \class QImageDocumentSelector
    \inpublicgroup QtBaseModule
  \ingroup documentselection

  \brief The QImageDocumentSelector widget allows the selection an image from
  a list of image documents available on the device.

  The QImageDocumentSelector widget builds a list of documents by
  locating all images in the device document directories.
  Alternatively, the list can be built with images which match a custom content filter.

  Some of the commonly used functionality is:
  \table
  \header
    \o Function/slot 
    \o Usage
  \row
    \o setFilter()
    \o filter the list of image documents using a QContentFilter.
  \row
    \o filter()
    \o retrieve the current QContentFilter.
  \row
    \o documents()
    \o Returns image documents listed.
  \row 
    \o documentSelected()
    \o notifies which document was chosen.
  \endtable 
 
  In addition documents may be viewed in two modes:
  \list
  \o QImageDocumentSelector::Single - presents a thumbnail of the current image
  contained within the dimensions of the QImageDocumentSelector widget.
  \o  QImageDocumentSelector::Thumbnail - presents thumbnails of the images
  in an icon-type scroll view allowing multiple image thumbnails to be
  viewed concurrently.

  The size of thumbnails is manipulated using:
    \list
    \o setThumbnailSize() - sets the size of the thumbnail
    \o thumbnailSize() - retrieves the size of the current thumbnail.
    \endlist
  \endlist

  Images are highlighted one at a time and a articular image can be chosen vi
  the stylus or select key and arrow keys are used to navigate through the list 
  of image documents.

  When an image is chosen the following occurs:
  \list
    \o The image document selector emits a documentSelected() signal
    \o a QContent for the chosen document is passed with the signal.
  \endlist

  Whenever the list of documents is changed as a result of a filter change 
  or a file system change, QImageDocumentSelector will emit a
  documentsChanged() signal.

  The following code allows the user to choose from all image documents
  available on the device using the thumbnail view mode with a custom thumbnail
  size:

  \code
    QImageDocumentSelector *selector = new QImageDocumentSelector( this );
    selector->setThumbnailSize( QSize( 100, 100 ) );

    connect( selector, SIGNAL(documentSelected(QContent)),
        this, SLOT(openImage(QContent)) );
  \endcode

  QImageDocumentSelector is often the first widget seen in a \l {Main Document Widget}{document-oriented application}. When used
  with \l QStackedWidget, an application
  allows choosing of a document using the selector before revealing
  the document viewer or editor.

  \sa QImageDocumentSelectorDialog, QDocumentSelector, QDocumentSelectorDialog
*/

#include "qimagedocumentselector_p.h"

#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qsoftmenubar.h>
#include <qdrmcontent.h>
#include <QLayout>
#include <QDesktopWidget>
#include <QMenu>

/*!
  Construct an image document selector widget with the given \a parent.
*/
QImageDocumentSelector::QImageDocumentSelector( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    d = new QImageDocumentSelectorPrivate( this );
    layout->addWidget( d );

    connect( d, SIGNAL(documentSelected(QContent)),
             this, SIGNAL(documentSelected(QContent)) );
    connect( d, SIGNAL(documentHeld(QContent,QPoint)),
        this, SIGNAL(documentHeld(QContent,QPoint)) );

    connect( d, SIGNAL(documentsChanged()), this, SIGNAL(documentsChanged()) );

    setFocusProxy( d );

    QSoftMenuBar::addMenuTo( this, QSoftMenuBar::menuFor( d ) );
}

/*!
  Destroys the widget.
*/
QImageDocumentSelector::~QImageDocumentSelector()
{
    delete d;
}

/*!
  \enum QImageDocumentSelector::ViewMode

  This enum describes the types of viewing modes.
  \value Single Images are displayed one at a time.
  \value Thumbnail Multiple images are displayed in an icon type scroll view.
*/

/*!
  Sets the viewing \a mode 

  The default mode is QImageDocumentSelector::Thumbnail.

  See QImageDocumentSelector::ViewMode for a listing of supported view modes.

  \sa viewMode()
*/
void QImageDocumentSelector::setViewMode( ViewMode mode )
{
    d->setViewMode( mode );
}

/*!
  Returns the current view mode.

  See QImageDocumentSelector::ViewMode for a listing of supported view modes.

  \sa setViewMode()
*/
QImageDocumentSelector::ViewMode QImageDocumentSelector::viewMode() const
{
    return d->viewMode();
}

/*!
  Sets the maximum \a size of a thumbnail.

  The default size is QSize( 65, 65 ).

  \sa thumbnailSize()
*/
void QImageDocumentSelector::setThumbnailSize( const QSize& size )
{
    d->setThumbnailSize( size );
}

/*!
  Returns the current maximum size of a thumbnail.

  \sa setThumbnailSize()
*/
QSize QImageDocumentSelector::thumbnailSize() const
{
    return d->thumbnailSize();
}

/*!
  Returns a \l QContent for the currently selected image, or an invalid \l QContent
  if there is no current selection.

  \sa documents()
*/
QContent QImageDocumentSelector::currentDocument() const
{
    return d->selectedDocument();
}

/*!
  Returns the content set of image documents listed by the selector

  \sa currentDocument()
*/
const QContentSet &QImageDocumentSelector::documents() const
{
    return d->documents();
}

/*!
  Returns the current documents filter.

  The filter defines the subset of image documents on the device the user can select from.
    
  \sa setFilter(), QContentSet::filter()
*/
QContentFilter QImageDocumentSelector::filter() const
{
    return d->filter();
}

/*!
    Sets the \a filter which defines the subset of image documents on the device the user can select from.

    \sa filter(), QContentSet::filter() 
 */
void QImageDocumentSelector::setFilter( const QContentFilter &filter )
{
    d->setFilter( filter );
}

/*!
  Returns the current document sort mode.

  \sa setSortMode()
 */
QDocumentSelector::SortMode QImageDocumentSelector::sortMode() const
{
    return d->sortMode();
}

/*!
  Sets the document \a sortMode 

  The default mode is QDocumentSelector::Alphabetical.

  \sa sortMode()
 */
void QImageDocumentSelector::setSortMode( QDocumentSelector::SortMode sortMode )
{
    d->setSortMode( sortMode );
}

/*!
    Returns the current document sort criteria.
*/
QContentSortCriteria QImageDocumentSelector::sortCriteria() const
{
    return d->sortCriteria();
}

/*!
    Sets the document \a sort criteria.

    This will set the document selector sort mode to SortCriteria.

    \sa setSortMode()
*/
void QImageDocumentSelector::setSortCriteria( const QContentSortCriteria &sort )
{
    d->setSortCriteria( sort );
}

/*! \internal */
QSize QImageDocumentSelector::sizeHint() const
{
    QDesktopWidget *desktop = QApplication::desktop();
    return QSize(width(),
                desktop->availableGeometry(desktop->screenNumber(this)).height());
}

/*!
    Gives the keyboard input focus to this widget (or its focus
    proxy) if this widget or one of its parents is the
    \l{isActiveWindow()}{active window}.
 */
void QImageDocumentSelector::setFocus()
{
    d->setFocus();
}


/*!
    Sets the \a categories checked by default in the document selector's category filter dialog.

    If a supplied category does not match those available in the category filter dialog, the document
    selector will not filter with that category. Upon invocation this function will set the default checked
    categories within the category filter dialog, and filter according to the supplied \a categories.
    \bold {Note:} Once the dialog has been shown once, this function no longer has any effect.

    Filtering according to \a categories is applied after the filter defined by filter()
    
    \sa defaultCategories(), filter(), setFilter()
 */
void QImageDocumentSelector::setDefaultCategories( const QStringList &categories )
{
    d->setDefaultCategories( categories );
}

/*!
    Returns the categories checked by default in the document selector's category filter dialog.

    \sa setDefaultCategories()
 */
QStringList QImageDocumentSelector::defaultCategories() const
{
    return d->defaultCategories();
}

/*!
    Sets the \a permission a document should give in order to be choosable.
    The permissions effectively specify the intended usage for that document.

    If a document does not have provide the given \a permission, the document
    selector will try to activate and thus acquire permissions for the
    document.  If the document cannot be activated with that \a permission, it
    will not be choosable from the list and visual indication of this is given.

    If the \a permission is QDrmRights::InvalidPermission the default
    permissions for the document is used.

    \sa selectPermission(), setMandatoryPermissions(), mandatoryPermissions()

 */
void QImageDocumentSelector::setSelectPermission( QDrmRights::Permission permission )
{
    d->setSelectPermission( permission );
}

/*!
    Returns the permissions a document should give in order to be choosable.

    The permissions effectively describe the document's intended usage.

    \sa setSelectPermission(), mandatoryPermissions(), setMandatoryPermissions()
 */
QDrmRights::Permission QImageDocumentSelector::selectPermission() const
{
    return d->selectPermission();
}

/*!
    Sets the \a permissions a document must have in order to be choosable from the document
    selector.

    Unlike select permissions, if a document is missing a mandatory permission it will not be activated,
    and the document cannot be chosen.

    Because the \a permissions are mandatory, passing QDrmRights::InvalidPermission as a parameter
    does not exhibit the same behavior as in setSelectPermission().

    \sa mandatoryPermissions(), setSelectPermission(), selectPermission()

 */
void QImageDocumentSelector::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    d->setMandatoryPermissions( permissions );
}

/*!
    Returns the permissions a document must have in order to be choosable by the document selector.

    \sa setMandatoryPermissions(), setSelectPermission(), selectPermission()
*/
QDrmRights::Permissions QImageDocumentSelector::mandatoryPermissions() const
{
    return d->mandatoryPermissions();
}

/*! \fn void QImageDocumentSelector::documentSelected( const QContent& image );

  This signal is emitted when the user chooses an image. A \l QContent for the
  image document is given in \a image.
*/

/*! \fn void QImageDocumentSelector::documentHeld( const QContent& image, const QPoint& pos );

  \internal

  Not currently supported.

  This signal is emitted when the user has held down on an image. A QContent to
  the image is given in \a image. The global position of the hold is given
  in \a pos.
*/

/*! 
  \fn void QImageDocumentSelector::documentsChanged();

  This signal is emitted when the list of documents changes as
  a result of a filter change or a file system change.
*/

/*!
  \class QImageDocumentSelectorDialog
    \inpublicgroup QtBaseModule
  \ingroup documentselection

  \brief The QImageDocumentSelectorDialog class allows the user to select an image
  from a list of image documents available on the device.

  QImageDocumentSelectorDialog is a convenience class that presents the \l
  QImageDocumentSelector widget in a dialog.

  The following code uses QImageDocumentSelectorDialog to allow the user
  to select a picture taken by the camera:

  \code
    QImageDocumentSelectorDialog dialog( this );

    dialog.setFilter( QContentFilter::category( "Camera" ) );

    if( QtopiaApplication::execDialog( &dialog ) ) {
        // Accept
        QContent picture = dialog.selectedDocument();
    } else {
        // Reject
    }
  \endcode

  \sa QImageDocumentSelector, QDocumentSelector, QDocumentSelectorDialog
*/

/*!
  Construct an image document selector the given \a parent.

  The dialog lists all documents with the image mime type.
  The dialog is modal by default.
*/
QImageDocumentSelectorDialog::QImageDocumentSelectorDialog( QWidget* parent )
    : QDialog( parent )
{
    selector = new QImageDocumentSelector( this );

    init();
}

void QImageDocumentSelectorDialog::init()
{
    setWindowTitle( tr( "Select Image" ) );
    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setContentsMargins(0, 0, 0, 0);

    connect( selector, SIGNAL(documentSelected(QContent)), this, SLOT(accept()) );
    connect( selector, SIGNAL(documentsChanged()), this, SLOT(setContextBar()) );

    vb->addWidget( selector );

    // Set thumbnail view
    selector->setViewMode( QImageDocumentSelector::Thumbnail );

    QAction *viewAction = new QAction( QIcon( ":icon/view" ), tr( "View" ), this );
    connect( viewAction, SIGNAL(triggered()), this, SLOT(viewImage()) );
    QMenu *menu = QSoftMenuBar::menuFor( selector );
    menu->actions().count() ? menu->insertAction(menu->actions().at(0), viewAction)
                    : menu->addAction(viewAction);
    QSoftMenuBar::addMenuTo( this, menu );

    setContextBar();

    setModal( true );
    QtopiaApplication::setMenuLike( this, true );
}

/*!
  Destroys the widget.
*/
QImageDocumentSelectorDialog::~QImageDocumentSelectorDialog()
{
}

/*!
  Sets the maximum \a size of a thumbnail.

  The default size is QSize( 65, 65 ).

  \sa thumbnailSize()
*/
void QImageDocumentSelectorDialog::setThumbnailSize( const QSize& size )
{
    selector->setThumbnailSize( size );
}

/*!
  Returns the current maximum size of a thumbnail.

  \sa setThumbnailSize()
*/
QSize QImageDocumentSelectorDialog::thumbnailSize() const
{
    return selector->thumbnailSize();
}

/*!
  Returns a \l QContent for the currently selected image, or an invalid \l QContent
  if there is no current selection.

  \sa documents()
*/
QContent QImageDocumentSelectorDialog::selectedDocument() const
{
    return selector->currentDocument();
}

/*!
  Returns the content set of image documents listed by the selector.

  \sa selectedDocument()
*/
const QContentSet &QImageDocumentSelectorDialog::documents() const
{
    return selector->documents();
}

/*!
  Returns the current documents filter.

  The filter defines the subset of image documents on the device the user can select from.
  \sa setFilter(), QContentSet::filter()
 
 */
QContentFilter QImageDocumentSelectorDialog::filter() const
{
    return selector->filter();
}

/*!
  Sets the \a filter which defines the subset of image documents on the device the user can select from.
    
  \sa filter(), QContentSet::filter()
 */
void QImageDocumentSelectorDialog::setFilter( const QContentFilter &filter )
{
    selector->setFilter( filter );
}

/*!
    Returns current document sort mode. 
*/
QDocumentSelector::SortMode QImageDocumentSelectorDialog::sortMode() const
{
    return selector->sortMode();
}

/*!
  Sets the document \a sortMode.

  The default mode is QDocumentSelector::Alphabetical.

  \sa sortMode()
*/
void QImageDocumentSelectorDialog::setSortMode( QDocumentSelector::SortMode sortMode )
{
    selector->setSortMode( sortMode );
}

/*!
    Returns the current document sort criteria.
*/
QContentSortCriteria QImageDocumentSelectorDialog::sortCriteria() const
{
    return selector->sortCriteria();
}

/*!
    Sets the document \a sort criteria.

    This will set the document selector sort mode to SortCriteria.

    \sa setSortMode()
*/
void QImageDocumentSelectorDialog::setSortCriteria( const QContentSortCriteria &sort )
{
    selector->setSortCriteria( sort );
}

/*! \internal */
void QImageDocumentSelectorDialog::accept()
{
    QDialog::accept();
}

/*! \internal */
void QImageDocumentSelectorDialog::reject()
{
    // If single view, switch to thumbnail view
    // Otherwise, reject
    if( selector->viewMode() == QImageDocumentSelector::Single && selector->documents().count() ) {
        selector->setViewMode( QImageDocumentSelector::Thumbnail );
        setContextBar();
    } else
        QDialog::reject();
}

void QImageDocumentSelectorDialog::setContextBar()
{
    bool hasImages = selector->documents().count();
    QMenu *menu = QSoftMenuBar::menuFor( selector );
    switch( selector->viewMode() )
    {
    case QImageDocumentSelector::Single:
        if (menu->actions().count())
            menu->actions().at(0)->setVisible(false);   //warning: assumes "view" is first item
        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Back );
        if( hasImages ) {
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Ok );
        } else {
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
        }
        break;
    case QImageDocumentSelector::Thumbnail:
        if (menu->actions().count())
            menu->actions().at(0)->setVisible(true);   //warning: assumes "view" is first item
        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
        if( hasImages ) {
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Ok );
        } else {
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
        }
        break;
    }
}

void QImageDocumentSelectorDialog::viewImage()
{
    selector->setViewMode( QImageDocumentSelector::Single );
    setContextBar();
}

/*!
    Sets the \a categories checked by default in the document selector's category filter dialog.

    If a supplied category does not match those available in the category filter dialog, the document
    selector will not filter with that category. Upon invocation this function will set the default checked
    categories within the category filter dialog, and filter according to the supplied \a categories.
    \bold {Note:} Once the dialog has been shown once, this function no longer has any effect.

    Filtering according to \a categories is applied after the filter defined by filter()

    \sa defaultCategories(), filter(), setFilter()
*/
void QImageDocumentSelectorDialog::setDefaultCategories( const QStringList &categories )
{
    selector->setDefaultCategories( categories );
}

/*!
    Returns the categories checked by default in the document selector's category filter dialog.

    \sa setDefaultCategories()
*/
QStringList QImageDocumentSelectorDialog::defaultCategories() const
{
    return selector->defaultCategories();
}

/*!
    Sets the \a permission a document should give in order to be choosable.
    The permissions effectively specify the intended usage for that document.

    If a document does not have provide the given \a permission, the document
    selector will try to activate and thus acquire permissions for the
    document.  If the document cannot be activated with that \a permission, it
    will not be choosable from the list and visual indication of this is given.

    If the \a permission is QDrmRights::InvalidPermission the default
    permissions for the document is used.

    \sa selectPermission(), setMandatoryPermissions(), mandatoryPermissions()
*/
void QImageDocumentSelectorDialog::setSelectPermission( QDrmRights::Permission permission )
{
    selector->setSelectPermission( permission );
}

/*!
    Returns the permissions a document should give in order to be choosable.

    The permissions effectively describe the document's intended usage.

    \sa setSelectPermission(), mandatoryPermissions(), setMandatoryPermissions()
*/
QDrmRights::Permission QImageDocumentSelectorDialog::selectPermission() const
{
    return selector->selectPermission();
}

/*!
    Sets the \a permissions a document must have in order to be choosable from the document
    selector.

    Unlike select permissions, if a document is missing a mandatory permission it will not be activated,
    and the document cannot be chosen.

    Because the \a permissions are mandatory, passing QDrmRights::InvalidPermission as a parameter
    does not exhibit the same behavior as in setSelectPermission().

    \sa mandatoryPermissions(), setSelectPermission(), selectPermission()
*/
void QImageDocumentSelectorDialog::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    selector->setMandatoryPermissions( permissions );
}

/*!
    Returns the permissions a document must have in order to be choosable by the document selector.

    \sa setMandatoryPermissions(), setSelectPermission(), selectPermission()
*/
QDrmRights::Permissions QImageDocumentSelectorDialog::mandatoryPermissions() const
{
    return selector->mandatoryPermissions();
}

