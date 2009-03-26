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

#include "hierarchicaldocumentview.h"
#include "uifactory.h"
#include <QMimeType>
#include <QKeyEvent>
#include <QtopiaApplication>
#include <QLabel>
#include <QCategoryManager>
#include <QVBoxLayout>
#include <QSoftMenuBar>
#include <QMenu>

////////////////////////////////////////////////////////////////
//
// AbstractContentFilterPropertyListModel implementation

QVariant AbstractContentFilterPropertyListModel::data ( const QModelIndex &index, int role) const 
{
    if (!index.isValid()) 
		return QVariant();
    
	if (role == Qt::DisplayRole || role == Qt::DecorationRole) {
		const ContentFilterProperty &property = mList[index.row()];
    	
		if (role == Qt::DisplayRole) return property.name();
    	if (role == Qt::DecorationRole) return property.icon();
	}
    
    return QVariant();
}

int AbstractContentFilterPropertyListModel::rowCount ( const QModelIndex &parent) const 
{
    Q_UNUSED(parent);
    return mList.count();
}

// Finds the index of a property having a certain filter. If not found -1 is returned.
int AbstractContentFilterPropertyListModel::findIndex(const QContentFilter &filter) const 
{
    for (int i=0;i<mList.count();++i) {
        if (mList[i].value() == filter) {
            // index found
            return i;
        }
    }
    
    // nothing found
    return -1;
}

QModelIndex AbstractContentFilterPropertyListModel::createIndex(const QContentFilter &filter) const 
{
    int propertyIndex = findIndex(filter);
    if (propertyIndex!=-1) 
		return index(propertyIndex);

	return QModelIndex();
}

QContentFilter AbstractContentFilterPropertyListModel::value(int index) const 
{
	return mList[index].value();
}

// Returns the name of the first property with a certain filter. If not found, an empty string is returned.
QString AbstractContentFilterPropertyListModel::findPropertyName(const QContentFilter &filter) const 
{
    int propertyIndex = findIndex(filter);
    if (propertyIndex!=-1) 
		return mList[propertyIndex].name();
    
	return QString();
}

////////////////////////////////////////////////////////////////
//
// MimeTypeFilterModel implementation

// Creates and appends a new property from the parameters.
void MimeTypeFilterModel::addProperty(const QString &label, const QMimeType &mimeType,const QIcon &icon) 
{
    mList.append(ContentFilterProperty(label, QContentFilter(mimeType), icon));
}

// Loads all mime types as properties into the model.
void MimeTypeFilterModel::reload() 
{
    mList.append(ContentFilterProperty(tr( "All" ), QContentFilter(), QIcon(":icon/stop")));

    addProperty(tr( "Text" ),QMimeType("text/*"), QIcon(":icon/txt"));
    addProperty(tr( "Audio" ),QMimeType("audio/*"), QIcon(":icon/sound"));
    addProperty(tr( "Image" ),QMimeType("image/*"), QIcon(":icon/camera"));
    addProperty(tr( "Video" ),QMimeType("video/*"), QIcon(":icon/multimedia"));
}

////////////////////////////////////////////////////////////////
//
// CategoryFilterListModel implementation

CategoryFilterListModel::CategoryFilterListModel(QObject *parent)
    : AbstractContentFilterPropertyListModel( parent ) 
{
    categoryManager = new QCategoryManager(QLatin1String("Documents"), this);
    
    // reload the model each time cats change
    connect(categoryManager, SIGNAL(categoriesChanged()), SLOT(reload()));
}

void CategoryFilterListModel::reload() 
{
    mList.clear();
    
    // Add "All" and "Unfiled" categories first
    mList.append(ContentFilterProperty(tr("All"), QContentFilter(), QIcon()));
    mList.append(ContentFilterProperty(tr("Unfiled"), 
                 QContentFilter( QContentFilter::Category, QLatin1String("Unfiled")),
                 categoryManager->icon(QLatin1String("unfiled"))));
    
    QList<QString> categoryIds = categoryManager->categoryIds();
    QList<QString> categoryLabels = categoryManager->labels(categoryIds);
    
    // Add all categories that are document related
    for (int i=0;i<categoryIds.count();++i) {
        mList.append(ContentFilterProperty(categoryLabels[i],
            QContentFilter( QContentFilter::Category, categoryIds[i]), 
            categoryManager->icon(categoryIds[i])));
    }
}

/*!
  \internal
  \class HierarchicalDocumentLauncherView
  \inpublicgroup QtUiModule
  \ingroup QtopiaServer::GeneralUI
  \brief The HierarchicalDocumentLauncherView class provides an hierarchical viwe of the
  user documents. 

  This document launcher is a replacement for the DocumentLauncherView. It distiniguishes itself from 
  the standard DocumentLauncherView by ordering documents according to their categories and (mime) types.
  This may provide a faster way of finding a document.

  This document launcher view can be enabled by setting the 
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \sa LauncherView, DocumentLauncherView
  */

/*!
  \enum HierarchicalDocumentLauncherView::FilterNavigation
  \internal
  */

/*!
  Creates a new HierarchicalDocumentLauncherView class with the given \a parent and \a flags. To increase 
  the component separation an instance of this class should be created via LauncherView::createLauncherView().

  \code
    LauncherView* view = LauncherView::createLauncherView("HierarchicalDocumentLauncherView", p, fl);
  \endcode

  \sa LauncherView::createLauncherView()
*/ 
HierarchicalDocumentLauncherView::HierarchicalDocumentLauncherView(QWidget* parent, Qt::WFlags flags)
    : DocumentLauncherView(parent, flags)
    , modelTypes(NULL)
    , modelCategories(NULL)
    , categoryManager(NULL) 
{
    init();
}

void HierarchicalDocumentLauncherView::init() 
{
    // Show dummy menu on first view (menu containing only "Help")
    dummyMenu = new QMenu(this);
    QSoftMenuBar::addMenuTo(this, dummyMenu);
    
    // Show a label for the hierarchy
    selectedFilterLabel = new QLabel();
    mainLayout->insertWidget(0, selectedFilterLabel);
    
    // initialize model for types
    modelTypes = new MimeTypeFilterModel(this);
    modelTypes->reload();
    
    // initialize model for categories
    modelCategories = new CategoryFilterListModel(this);
    modelCategories->reload();

    // show the types first
    icons->setModel(modelTypes);
    
    // install a filter to intercept the back button
    icons->installEventFilter(this);
}

/*!
  \internal
// performs the navigation i.e. goes from mime type view to category view to document view and back
*/
void HierarchicalDocumentLauncherView::filterNavigate(FilterNavigation navigation, QModelIndex selectedItem) 
{
    QContentFilter lastFilter;
    
    if (navigation == NavigateBackward) {
        if (!selectedFilters.isEmpty()) {
            // pop the last filter from the list
            lastFilter = selectedFilters.takeLast();
            enterNavigationMode();
        } else
            exitNavigationMode();
    } else if (navigation == NavigateForward) {        
        if (selectedItem.isValid()) {
            
            // push the newly selected filter to the list
            AbstractContentFilterPropertyListModel *model = 
                    qobject_cast<AbstractContentFilterPropertyListModel *>(icons->model());
            if (model)
                selectedFilters.append(model->value(selectedItem.row()));
        }
    }
    
    switch (selectedFilters.count()) {
        case 0:
            // show types
            icons->setModel(modelTypes);
            selectedFilterLabel->setText(QString());
            break;
        case 1:
            // show categories
            icons->setModel(modelCategories);
            selectedFilterLabel->setText(modelTypes->findPropertyName(selectedFilters.first()));
            break;
        default:
            // both type and category have been selected
            
            selectedFilterLabel->setText(modelTypes->findPropertyName(selectedFilters.first()) 
                    + " / " + modelCategories->findPropertyName(selectedFilters.at(1)));
            
            // refresh filters of contentset
            QContentFilter newFilter(QContent::Document);
            foreach (QContentFilter filter, selectedFilters) {
                if (!filter.isValid()) 
					continue;
                
                newFilter &= filter;
            }
            
            // filter the content set
            contentSet->setCriteria(newFilter);
            
            // show filtered docs
            if ( model )
                icons->setModel(model);
            
            exitNavigationMode();
            break;
             
    }
    
    setCurrentItem(lastFilter);
}

/*!
  \internal
// highlights a list view item with a certain filter
*/
void HierarchicalDocumentLauncherView::setCurrentItem(const QContentFilter& pFilter) 
{
    // get current model, if not AbstractContentFilterPropertyListModel, do nothing
    AbstractContentFilterPropertyListModel *model = qobject_cast<AbstractContentFilterPropertyListModel *>(icons->model());
    if (!model) 
		return;
    
    // find the index of the filter
    QModelIndex index = model->createIndex(pFilter);
    if (!index.isValid()) 
		return;
 
    icons->setCurrentIndex(index);
}

/*!
  \reimp
  */
bool HierarchicalDocumentLauncherView::eventFilter(QObject *obj, QEvent *event) 
{
    Q_UNUSED(obj);

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);

        // navigate back on "back" or "no" key
        if ((e->key() == Qt::Key_Back || e->key() == Qt::Key_No) 
             && !selectedFilters.isEmpty()) {

            filterNavigate(NavigateBackward);
            return true;
        }
    }
    
    return false;
}

/*!
  Destroys the HierarchicalDocumentLauncherView instance.
  */
HierarchicalDocumentLauncherView::~HierarchicalDocumentLauncherView() 
{
}

/*!
  \reimp
  */
void HierarchicalDocumentLauncherView::handleReturnPressed(const QModelIndex &item) 
{
    if (selectedFilters.count()<2)
        // if in mime type or cat view go forward in hierarchy
        filterNavigate(NavigateForward, item);
    else 
    	LauncherView::handleReturnPressed(item);
}

/*!
  \reimp
  */
void HierarchicalDocumentLauncherView::handleItemClicked(const QModelIndex & item, bool setCurrentIndex) 
{
    Q_UNUSED(setCurrentIndex);
    
    if(QApplication::mouseButtons () == Qt::LeftButton) {
        icons->setCurrentIndex( item );
        if (selectedFilters.count()<2)
            filterNavigate(NavigateForward, item);
        else 
            emit LauncherView::handleItemClicked(item, false);
    }
}

/*!
  \reimp
  */
void HierarchicalDocumentLauncherView::resetSelection() 
{
    if (icons && model && icons->model()->rowCount()) {
        LauncherView::resetSelection();
        
        selectedFilterLabel->setText(QString());
        selectedFilters.clear();
        icons->setModel(modelTypes);
        enterNavigationMode();
    }
}

/*! \internal
    enters navigation mode i.e. activates the dummy menu
    */
void HierarchicalDocumentLauncherView::enterNavigationMode() 
{
   QSoftMenuBar::removeMenuFrom(this, softMenu);
   QSoftMenuBar::addMenuTo(this, dummyMenu); 
}

/*!
  \internal
// exits navi mode i.e. it gets called when the document list is shown
// and activates the normal menu
*/
void HierarchicalDocumentLauncherView::exitNavigationMode() 
{
   QSoftMenuBar::removeMenuFrom(this, dummyMenu);
   QSoftMenuBar::addMenuTo(this, softMenu);
}

UIFACTORY_REGISTER_WIDGET(HierarchicalDocumentLauncherView);
