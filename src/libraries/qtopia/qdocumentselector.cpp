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

#include "qdocumentselector.h"

#include <QMenu>
#include <QVBoxLayout>
#include <QListView>
#include <QAction>
#include <qcontentfiltermodel.h>
#include <qdocumentproperties.h>
#include <qtopiaapplication.h>
#include <QAbstractProxyModel>
#include <qcontentfilterselector.h>
#include <qdrmcontent.h>
#include <qwaitwidget.h>
#include "drmcontent_p.h"
#include <QFocusEvent>
#include <qsoftmenubar.h>
#include <qtopiaitemdelegate.h>
#include <QContentSortCriteria>
#include <QPainter>
#include <QScrollBar>
#include <QLabel>
#include <QDesktopWidget>

class NewDocumentProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    NewDocumentProxyModel( QIcon newIcon, QObject *parent = 0 )
        : QAbstractProxyModel( parent )
        , m_newEnabled( false )
        , m_hasDocuments( true )
        , m_newIcon( newIcon )
    {
    }

    virtual ~NewDocumentProxyModel()
    {
    }

    virtual QModelIndex mapFromSource( const QModelIndex &index ) const
    {
        if( index.isValid() )
        {
            return m_newEnabled
                    ? createIndex( index.row() + 1, index.column() )
                    : createIndex( index.row()    , index.column() );
        }
        else
            return index;
    }

    virtual QModelIndex mapToSource( const QModelIndex &index ) const
    {
        if( index.isValid() && sourceModel() )
        {
            return m_newEnabled
                    ? sourceModel()->index( index.row() - 1, index.column() )
                    : sourceModel()->index( index.row()    , index.column() );
        }
        else
            return index;
    }

    void setSourceModel( QAbstractItemModel *model )
    {
        QAbstractItemModel *oldModel = sourceModel();

        if( oldModel )
        {
            disconnect( oldModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                        this    , SLOT (_columnsAboutToBeInserted(QModelIndex,int,int)) );
            disconnect( oldModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                        this    , SLOT (_columnsAboutToBeRemoved(QModelIndex,int,int)) );
            disconnect( oldModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
                        this    , SLOT (_columnsInserted(QModelIndex,int,int)) );
            disconnect( oldModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                        this    , SLOT (_columnsRemoved(QModelIndex,int,int)) );
            disconnect( oldModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                        this    , SLOT (_dataChanged(QModelIndex,QModelIndex)) );
            disconnect( oldModel, SIGNAL(layoutAboutToBeChanged()),
                        this    , SIGNAL(layoutAboutToBeChanged()) );
            disconnect( oldModel, SIGNAL(layoutChanged()),
                        this    , SIGNAL(layoutChanged()) );
            disconnect( oldModel, SIGNAL(modelReset()),
                        this    , SIGNAL(modelReset()) );
            disconnect( oldModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                        this    , SLOT (_rowsAboutToBeInserted(QModelIndex,int,int)) );
            disconnect( oldModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                        this    , SLOT (_rowsAboutToBeRemoved(QModelIndex,int,int)) );
            disconnect( oldModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                        this    , SLOT (_rowsInserted(QModelIndex,int,int)) );
            disconnect( oldModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                        this    , SLOT (_rowsRemoved(QModelIndex,int,int)) );
            disconnect( model, SIGNAL(updateFinished()),
                 this , SLOT  (updateFinished()) );
        }

        QAbstractProxyModel::setSourceModel( model );

        connect( model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                 this , SLOT (_columnsAboutToBeInserted(QModelIndex,int,int)) );
        connect( model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                 this , SLOT (_columnsAboutToBeRemoved(QModelIndex,int,int)) );
        connect( model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                 this , SLOT (_columnsInserted(QModelIndex,int,int)) );
        connect( model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                 this , SLOT (_columnsRemoved(QModelIndex,int,int)) );
        connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                 this , SLOT (_dataChanged(QModelIndex,QModelIndex)) );
        connect( model, SIGNAL(layoutAboutToBeChanged()),
                 this , SIGNAL(layoutAboutToBeChanged()) );
        connect( model, SIGNAL(layoutChanged()),
                 this , SIGNAL(layoutChanged()) );
        connect( model, SIGNAL(modelReset()),
                 this , SIGNAL(modelReset()) );
        connect( model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                 this , SLOT (_rowsAboutToBeInserted(QModelIndex,int,int)) );
        connect( model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                 this , SLOT (_rowsAboutToBeRemoved(QModelIndex,int,int)) );
        connect( model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                 this , SLOT (_rowsInserted(QModelIndex,int,int)) );
        connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                 this , SLOT (_rowsRemoved(QModelIndex,int,int)) );
        connect( model, SIGNAL(updateFinished()),
                 this , SLOT  (updateFinished()) );
    }

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const
    {
        return !parent.isValid() && sourceModel() ? 1 : 0;
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
        if( m_newEnabled && index.row() == 0 )
        {
            if( role == Qt::DisplayRole )
                return tr( "New" );
            else if( role == Qt::DecorationRole )
                return m_newIcon;
        }
        else if( !m_hasDocuments && index.row() == 0 )
        {
            if( role == Qt::DisplayRole )
                return tr( "No documents found" );
        }
        else if( index.isValid() )
        {
            return sourceModel() ? sourceModel()->data( mapToSource( index ), role ) : QVariant();
        }

        return QVariant();
    }

    virtual Qt::ItemFlags flags( const QModelIndex & index ) const
    {
        if( m_newEnabled && index.row() == 0 )
        {
            return QAbstractItemModel::flags( index );
        }
        else if( !m_hasDocuments && index.row() == 0 )
        {
            return Qt::ItemIsEnabled;
        }
        else if( index.isValid() )
        {
            QAbstractItemModel *model = sourceModel();

            if( sourceModel() )
                return model->flags( mapToSource( index ) );
        }

        return QAbstractProxyModel::flags( index );
    }

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const
    {
        return !parent.isValid() ? createIndex( row, column ) : QModelIndex();
    }

    virtual QModelIndex parent( const QModelIndex &index ) const
    {
        Q_UNUSED( index );

        return QModelIndex();
    }

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
        if( !parent.isValid() )
        {
            int count = sourceModel()->rowCount();

            if( m_newEnabled )
                count++;
            else if( !m_hasDocuments )
                count = 1;

            return count;
        }

        return 0;
    }

    void setNewEnabled( bool enabled )
    {
        if( enabled && !m_newEnabled )
        {
            if( m_hasDocuments )
            {
                beginInsertRows( QModelIndex(), 0, 0 );
                m_newEnabled = true;
                endInsertRows();
            }
            else
            {
                m_newEnabled = true;
                QModelIndex index = createIndex( 0, 0 );
                emit dataChanged( index, index );
            }
        }
        else if( !enabled && m_newEnabled )
        {
            if( m_hasDocuments )
            {
                beginRemoveRows( QModelIndex(), 0, 0 );
                m_newEnabled = false;
                endRemoveRows();
            }
            else
            {
                m_newEnabled = false;
                QModelIndex index = createIndex( 0, 0 );
                emit dataChanged( index, index );
            }
        }
    }

    bool hasDocuments() const
    {
        return m_hasDocuments;
    }

private slots:
    void _columnsAboutToBeInserted( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_newEnabled )
                beginRemoveColumns( QModelIndex(), start + 1, end + 1 );
            else
                beginRemoveColumns( QModelIndex(), start, end );
        }
    }

    void _columnsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_newEnabled )
                beginRemoveColumns( QModelIndex(), start + 1, end + 1 );
            else
                beginRemoveColumns( QModelIndex(), start, end );
        }
    }

    void _columnsInserted( const QModelIndex &parent, int start, int end )
    {
        Q_UNUSED( parent );
        Q_UNUSED( start );
        Q_UNUSED( end );

        endInsertColumns();
    }

    void _columnsRemoved( const QModelIndex &parent, int start, int end )
    {
        Q_UNUSED( parent );
        Q_UNUSED( start );
        Q_UNUSED( end );

        endRemoveColumns();
    }

    void _dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
    {
        if( m_newEnabled )
            emit dataChanged( createIndex( topLeft    .row() + 1, topLeft    .column() ),
                              createIndex( bottomRight.row() + 1, bottomRight.column() ) );
        else
            emit dataChanged( createIndex( topLeft    .row(), topLeft    .column() ),
                              createIndex( bottomRight.row(), bottomRight.column() ) );
    }

    void _rowsAboutToBeInserted( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( !m_hasDocuments )
            {
                if( !m_newEnabled )
                {
                    beginRemoveRows( QModelIndex(), 0, 0 );

                    m_hasDocuments = true;

                    endRemoveRows();
                }
                else
                    m_hasDocuments = false;
            }

            if( m_newEnabled )
                beginInsertRows( QModelIndex(), start + 1, end + 1 );
            else
                beginInsertRows( QModelIndex(), start, end );
        }
    }

    void _rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_newEnabled )
                beginRemoveRows( QModelIndex(), start + 1, end + 1 );
            else
                beginRemoveRows( QModelIndex(), start, end );
        }
    }

    void _rowsInserted( const QModelIndex &parent, int start, int end )
    {
        Q_UNUSED( parent );
        Q_UNUSED( start );
        Q_UNUSED( end );

        endInsertRows();
    }

    void _rowsRemoved( const QModelIndex &parent, int start, int end )
    {
        Q_UNUSED( parent );
        Q_UNUSED( start );
        Q_UNUSED( end );

        endRemoveRows();
    }

    void updateFinished()
    {
        if( m_hasDocuments && sourceModel()->rowCount() == 0 )
        {
            if( !m_newEnabled )
            {
                beginInsertRows( QModelIndex(), 0, 0 );

                m_hasDocuments = false;

                endInsertRows();
            }
            else
                m_hasDocuments = false;
        }
    }

private:
    bool m_newEnabled;
    bool m_hasDocuments;
    QIcon m_newIcon;
};

class DocumentView : public QListView
{
    Q_OBJECT
public:
    DocumentView( QWidget *parent = 0 );
    virtual ~DocumentView();

    QContentFilter baseFilter() const;
    void setBaseFilter( const QContentFilter &filter );

    void setDefaultCategories( const QStringList &categories );
    QStringList defaultCategories() const;

    void setSelectPermission( QDrmRights::Permission permission );
    QDrmRights::Permission selectPermission() const;

    void setMandatoryPermissions( QDrmRights::Permissions permissions );
    QDrmRights::Permissions mandatoryPermissions() const;

    void setSortMode( QDocumentSelector::SortMode mode );
    QDocumentSelector::SortMode sortMode() const;

    void setSortCriteria( const QContentSortCriteria &sort );
    QContentSortCriteria sortCriteria() const;

    QDocumentSelector::Options options() const;
    void setOptions( QDocumentSelector::Options options );

    QContent currentDocument() const;

    bool newCurrent() const;

    const QContentSet &documents() const;

    QStringList selectedCategories() const
            { return m_categoryFilter.arguments(QContentFilter::Category); }

signals:
    void documentSelected( const QContent &content );
    void currentChanged();
    void newSelected();
    void documentsChanged();
    void typesSelected( bool selected );
    void setTypeLabel( const QString &text );
    void categoriesSelected( bool selected );
    void setCategoryLabel( const QString &text );

protected slots:
    virtual void currentChanged( const QModelIndex &current, const QModelIndex &previous );
    virtual void focusInEvent( QFocusEvent *event );
    virtual void rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end );
    virtual void rowsInserted( const QModelIndex &parent, int start, int end );
private slots:
    void indexActivated( const QModelIndex &index );
    void selectTypeFilter();
    void selectCategoryFilter();
    void showProperties();
    void deleteCurrent();
    void itemPressed(const QModelIndex &index);

private:
    void setCombinedFilter();

    static QContentFilterModel::Template typeTemplate();

    void filterDefaultCategories();

    QContentFilter m_baseFilter;
    QContentFilter m_typeFilter;
    QContentFilter m_categoryFilter;

    QContentSet m_contentSet;

    QContentFilterDialog *m_typeDialog;
    QContentFilterDialog *m_categoryDialog;

    QDocumentPropertiesDialog *m_propertiesDialog;

    QWaitWidget *m_waitWidget;

    QContentSetModel *m_contentModel;
    NewDocumentProxyModel *m_proxyModel;

    QDocumentSelector::Options m_options;

    QDocumentSelector::SortMode m_sortMode;

    QStringList m_defaultCategories;
    QStringList m_filteredDefaultCategories;
    bool m_defaultCategoriesDirty;

    QMenu *m_softMenu;
    QMenu *m_itemMenu;

    QAction *m_newAction;
    QAction *m_deleteAction;
    QAction *m_propertiesAction;
    QAction *m_typeAction;
    QAction *m_categoryAction;
};

DocumentView::DocumentView( QWidget *parent )
    : QListView( parent )
    , m_baseFilter( QContent::Document )
    , m_contentSet( QContentSet::Asynchronous )
    , m_typeDialog( 0 )
    , m_categoryDialog( 0 )
    , m_propertiesDialog( 0 )
    , m_waitWidget( 0 )
    , m_options( QDocumentSelector::ContextMenu )
    , m_sortMode( QDocumentSelector::Alphabetical )
    , m_defaultCategoriesDirty( false )
{
    QIcon newIcon( ":icon/new" );

    setItemDelegate(new QtopiaItemDelegate( this ));

    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setFrameStyle(NoFrame);
    setResizeMode( QListView::Fixed );
    setSelectionMode( QAbstractItemView::SingleSelection );
    setSelectionBehavior( QAbstractItemView::SelectItems );
    setUniformItemSizes( true );

    m_softMenu = QSoftMenuBar::menuFor( this );

    m_newAction = m_softMenu->addAction( newIcon, tr( "New" ) );
    m_deleteAction = m_softMenu->addAction( QIcon( ":icon/trash" ), tr( "Delete" ) );

    m_propertiesAction = m_softMenu->addAction( QIcon(":icon/info"), tr( "Properties..." ) );

    m_softMenu->addSeparator();

    m_typeAction = m_softMenu->addAction( tr( "View Type..." ) );
    m_categoryAction = m_softMenu->addAction( QIcon( ":icon/viewcategory" ), tr( "View Category..." ) );

    m_newAction->setVisible( false );
    m_deleteAction->setVisible( false );
    m_propertiesAction->setVisible( false );
    m_typeAction->setVisible( false );

    connect( m_newAction, SIGNAL(triggered()), this, SIGNAL(newSelected()) );
    connect( m_deleteAction, SIGNAL(triggered()), this, SLOT(deleteCurrent()) );
    connect( m_propertiesAction, SIGNAL(triggered()), this, SLOT(showProperties()) );
    connect( m_typeAction, SIGNAL(triggered()), this, SLOT(selectTypeFilter()) );
    connect( m_categoryAction, SIGNAL(triggered()), this, SLOT(selectCategoryFilter()) );

    connect( this, SIGNAL(activated(QModelIndex)), this, SLOT(indexActivated(QModelIndex)) );

    connect( &m_contentSet, SIGNAL(changed()), this, SIGNAL(documentsChanged()));

    m_contentModel = new QContentSetModel( &m_contentSet, this );
    m_proxyModel = new NewDocumentProxyModel( newIcon, this );

    m_proxyModel->setSourceModel( m_contentModel );

    setModel( m_proxyModel );

    QtopiaApplication::setStylusOperation(viewport(), QtopiaApplication::RightOnHold);

    m_itemMenu = new QMenu(this);
    m_itemMenu->addAction(m_deleteAction);
    m_itemMenu->addAction(m_propertiesAction);

    connect(this, SIGNAL(pressed(QModelIndex)),
            this, SLOT(itemPressed(QModelIndex)));
}

DocumentView::~DocumentView()
{
}

QContentFilter DocumentView::baseFilter() const
{
    return m_baseFilter;
}

void DocumentView::setBaseFilter( const QContentFilter &filter )
{
    m_baseFilter = filter;

    if( m_typeDialog )
        m_typeDialog->setFilter( filter );

    if( m_categoryDialog )
        m_categoryDialog->setFilter( filter );

    m_typeFilter = QContentFilter();

    filterDefaultCategories();

    setCombinedFilter();
}

void DocumentView::setSortMode( QDocumentSelector::SortMode mode )
{
    if( mode != m_sortMode )
    {
        m_sortMode = mode;

        switch( mode )
        {
        case QDocumentSelector::Alphabetical:
            m_contentSet.setSortCriteria( QContentSortCriteria( QContentSortCriteria::Name, Qt::AscendingOrder ) );
            break;
        case QDocumentSelector::ReverseAlphabetical:
            m_contentSet.setSortCriteria( QContentSortCriteria( QContentSortCriteria::Name, Qt::DescendingOrder ) );
            break;
        case QDocumentSelector::Chronological:
            m_contentSet.setSortCriteria( QContentSortCriteria( QContentSortCriteria::LastUpdated, Qt::AscendingOrder ) );
            break;
        case QDocumentSelector::ReverseChronological:
            m_contentSet.setSortCriteria( QContentSortCriteria( QContentSortCriteria::LastUpdated, Qt::DescendingOrder ) );
            break;
        case QDocumentSelector::SortCriteria:
            break;
        }
    }
}

QDocumentSelector::SortMode DocumentView::sortMode() const
{
    return m_sortMode;
}

void DocumentView::setSortCriteria( const QContentSortCriteria &sort )
{
    m_sortMode = QDocumentSelector::SortCriteria;

    m_contentSet.setSortCriteria( sort );
}

QContentSortCriteria DocumentView::sortCriteria() const
{
    return m_contentSet.sortCriteria();
}

QDocumentSelector::Options DocumentView::options() const
{
    return m_options;
}

void DocumentView::setOptions( QDocumentSelector::Options options )
{
    QDocumentSelector::Options changes = m_options ^ options;

    m_options = options;

    if( m_typeDialog && changes & QDocumentSelector::NestTypes )
    {
        if( options & QDocumentSelector::NestTypes )
            m_typeDialog->setModelTemplate( typeTemplate() );
        else
            m_typeDialog->setModelTemplate(
                    QContentFilter::MimeType,
                    QString(),
                    QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        m_typeFilter = QContentFilter();

        setCombinedFilter();
    }

    if( changes & QDocumentSelector::NewDocument )
    {
        m_proxyModel->setNewEnabled( options & QDocumentSelector::NewDocument );
        m_newAction->setVisible( options & QDocumentSelector::NewDocument );
    }

    if( changes & QDocumentSelector::TypeSelector )
    {
        m_typeAction->setVisible( options & QDocumentSelector::TypeSelector );
    }

    if( changes & QDocumentSelector::ContextMenu )
    {
        if( options & QDocumentSelector::ContextMenu )
            QSoftMenuBar::addMenuTo( this, m_softMenu );
        else
            QSoftMenuBar::removeMenuFrom( this, m_softMenu );
    }
}

void DocumentView::setDefaultCategories( const QStringList &categories )
{
    m_defaultCategories = categories;

    filterDefaultCategories();

    setCombinedFilter();
}

QStringList DocumentView::defaultCategories() const
{
    return m_defaultCategories;
}

void DocumentView::setSelectPermission( QDrmRights::Permission permission )
{
    m_contentModel->setSelectPermission( permission );
}

QDrmRights::Permission DocumentView::selectPermission() const
{
    return m_contentModel->selectPermission();
};

void DocumentView::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    m_contentModel->setMandatoryPermissions( permissions );
}

QDrmRights::Permissions DocumentView::mandatoryPermissions() const
{
    return m_contentModel->mandatoryPermissions();
}

QContent DocumentView::currentDocument() const
{
    return m_contentModel->rowCount()
            ? m_contentModel->content( m_proxyModel->mapToSource( currentIndex() ) )
            : QContent();
}

bool DocumentView::newCurrent() const
{
    return m_options & QDocumentSelector::NewDocument && currentIndex().row() == 0;
}

const QContentSet &DocumentView::documents() const
{
    return m_contentSet;
}

void DocumentView::indexActivated( const QModelIndex &index )
{
    if( m_options & QDocumentSelector::NewDocument && index.row() == 0 )
        emit newSelected();
    else if( !(m_contentModel->rowCount() == 0 && index.row() == 0) && index.flags() & Qt::ItemIsSelectable )
    {
        QContent content = m_contentModel->content( m_proxyModel->mapToSource( index ) );

        if( DrmContentPrivate::activate( content, m_contentModel->selectPermission(), this ) )
            emit documentSelected( content );
    }
}

void DocumentView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    QListView::currentChanged( current, previous );

    int minRow = m_options & QDocumentSelector::NewDocument || !m_proxyModel->hasDocuments() ? 1 : 0;

    if( previous.row() < minRow && current.row() >= minRow )
    {
        m_deleteAction->setVisible( true );
        m_propertiesAction->setVisible( true );
    }
    else if( previous.row() >= minRow && current.row() < minRow )
    {
        m_deleteAction->setVisible( false );
        m_propertiesAction->setVisible( false );
    }

    emit currentChanged();
}

void DocumentView::focusInEvent( QFocusEvent *event )
{
    QListView::focusInEvent( event );

    setCurrentIndex( currentIndex() );
}

void DocumentView::rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
{
    QModelIndex index = currentIndex();

    int scrollValue = verticalScrollBar()->value();

    QListView::rowsAboutToBeRemoved(parent, start, end);

    if( index.row() >= start && index.row() <= end && end + 1 < model()->rowCount( parent ) )
    {
        selectionModel()->setCurrentIndex(
                model()->index( end + 1, index.column(), parent ),
                QItemSelectionModel::ClearAndSelect );
    }

    if(index.row() >= start)
    {
        int adjustedValue = index.row() > end
                ? scrollValue - end + start - 1
                : scrollValue - index.row() + start;

        verticalScrollBar()->setValue( adjustedValue > 0 ? adjustedValue : 0 );
    }
}

void DocumentView::rowsInserted( const QModelIndex &parent, int start, int end )
{
    QListView::rowsInserted( parent, start, end );

    if( !Qtopia::mousePreferred() )
    {
        if( !currentIndex().isValid() )
            selectionModel()->setCurrentIndex( model()->index( 0, 0, parent ), QItemSelectionModel::ClearAndSelect );
    }
    else
        selectionModel()->clearSelection();

    scrollTo(currentIndex());
}

void DocumentView::selectTypeFilter()
{
    if( !m_typeDialog )
    {
        m_typeDialog = new QContentFilterDialog( this );

        if( m_options & QDocumentSelector::NestTypes )
            m_typeDialog->setModelTemplate( typeTemplate() );
        else
            m_typeDialog->setModelTemplate(
                    QContentFilter::MimeType,
        QString(),
        QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        m_typeDialog->setWindowTitle( tr("View Type") );
        m_typeDialog->setFilter( m_baseFilter );
        m_typeDialog->setObjectName( QLatin1String( "documents-type" ) );
    }

    QtopiaApplication::execDialog( m_typeDialog );

    m_typeFilter = m_typeDialog->checkedFilter();

    setCombinedFilter();

    QString label = m_typeDialog->checkedLabel();

    if( !m_typeFilter.isValid() || label.isEmpty() )
    {
        emit typesSelected( false );
    }
    else
    {
        emit setTypeLabel( tr("Type: %1").arg( label ) );
        emit typesSelected( true );
    }
}

void DocumentView::selectCategoryFilter()
{
    if( m_defaultCategoriesDirty && m_categoryDialog )
    {
        delete m_categoryDialog;

        m_categoryDialog = 0;
    }

    if( !m_categoryDialog )
    {
        m_defaultCategoriesDirty = false;

        QContentFilterModel::Template categoryPage;

        categoryPage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        categoryPage.addList( QContentFilter::Category, QString(), m_filteredDefaultCategories );
        categoryPage.addList( QContentFilter::Category, QLatin1String( "Documents" ), m_filteredDefaultCategories );

        m_categoryDialog = new QContentFilterDialog( categoryPage, this );

        m_categoryDialog->setWindowTitle( tr("View Category") );
        m_categoryDialog->setFilter( m_baseFilter );
        m_categoryDialog->setObjectName( QLatin1String( "documents-category" ) );

    }

    QtopiaApplication::execDialog( m_categoryDialog );

    m_categoryFilter = m_categoryDialog->checkedFilter();

    setCombinedFilter();

    QString label = m_categoryDialog->checkedLabel();

    if( !m_categoryFilter.isValid() || label.isEmpty() )
    {
        emit categoriesSelected( false );
    }
    else
    {
        emit setCategoryLabel( tr("Category: %1").arg( label ) );
        emit categoriesSelected( true );
    }
}

void DocumentView::deleteCurrent()
{
    QContent content = currentDocument();

    if( content.id() != QContent::InvalidId && Qtopia::confirmDelete( this, tr("Delete"), content.name() ) )
        content.removeFiles();
}

void DocumentView::showProperties()
{
    QContent content = currentDocument();

    if( content.id() != QContent::InvalidId && content.isValid() )
    {
        if( m_propertiesDialog )
            delete m_propertiesDialog;

        m_propertiesDialog = new QDocumentPropertiesDialog( content, this );
        m_propertiesDialog->setObjectName( QLatin1String( "document-properties" ) );
        QtopiaApplication::showDialog( m_propertiesDialog );
        m_propertiesDialog->setWindowState( m_propertiesDialog->windowState() | Qt::WindowMaximized );
    }
}

void DocumentView::setCombinedFilter()
{
    m_contentSet.setCriteria( m_baseFilter & m_typeFilter & m_categoryFilter );

    emit documentsChanged();
}

void DocumentView::filterDefaultCategories()
{
    if( m_defaultCategories.isEmpty() )
    {
        m_categoryFilter = QContentFilter();

        return;
    }

    m_defaultCategoriesDirty = true;
    m_categoryFilter = QContentFilter();

    const QString unfiled = QLatin1String("Unfiled");

    QStringList categories
            = m_baseFilter.argumentMatches(QContentFilter::Category, QString())
            + m_baseFilter.argumentMatches(QContentFilter::Category, QLatin1String("Documents"));

    foreach (QString category, categories)
        if (m_defaultCategories.contains(category))
            m_filteredDefaultCategories.append(category);

    if (m_defaultCategories.contains(unfiled))
        m_filteredDefaultCategories.append(unfiled);

    if (m_filteredDefaultCategories.count() > 0) {
        foreach(QString category, m_filteredDefaultCategories)
            m_categoryFilter |= QContentFilter( QContentFilter::Category, category );


        QString label;

        if (m_filteredDefaultCategories.count() > 1)
            label = tr("(Multi)");
        else if (m_filteredDefaultCategories.first() == unfiled)
            label = tr("Unfiled");
        else
            label = QCategoryManager().label(m_filteredDefaultCategories.first());

        emit setCategoryLabel(tr("Category: %1").arg(label));
        emit categoriesSelected(true);
    } else {
        emit categoriesSelected(false);
    }
}

void DocumentView::itemPressed(const QModelIndex &index)
{
    if (QApplication::mouseButtons() & Qt::RightButton && index.row() != 0) {
        setCurrentIndex(index);
        m_itemMenu->popup(QCursor::pos());
    }
}

QContentFilterModel::Template DocumentView::typeTemplate()
{
    QContentFilterModel::Template subTypePage(
            QContentFilter::MimeType,
            QString(),
            QContentFilterModel::CheckList );

    QContentFilterModel::Template typePage;

    typePage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

    typePage.addLabel( subTypePage, tr( "Audio" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "audio/*" ) ) );
    typePage.addLabel( subTypePage, tr( "Image" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "image/*" ) ) );
    typePage.addLabel( subTypePage, tr( "Text"  ), QContentFilter( QContentFilter::MimeType, QLatin1String( "text/*"  ) ) );
    typePage.addLabel( subTypePage, tr( "Video" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "video/*" ) ) );
    typePage.addList( ~( QContentFilter( QContentFilter::MimeType, QLatin1String( "audio/*" ) )
                       | QContentFilter( QContentFilter::MimeType, QLatin1String( "image/*" ) )
                       | QContentFilter( QContentFilter::MimeType, QLatin1String( "text/*"  ) )
                       | QContentFilter( QContentFilter::MimeType, QLatin1String( "video/*" ) ) ),
                      QContentFilter::MimeType );

    return typePage;
}

class QDocumentSelectorPrivate : public DocumentView
{
public:
    QDocumentSelectorPrivate( QWidget *parent = 0 )
    : DocumentView( parent )
    {
    }

    virtual ~QDocumentSelectorPrivate()
    {
    }
};

/*!
    \class QDocumentSelector
    \inpublicgroup QtBaseModule
    \ingroup documentselection

    \brief The QDocumentSelector widget allows the user choose a document from
            a list of documents available on the device.

    The QDocumentSelector widget builds the list of documents by using a supplied \l QContent filter.  If no
    filter is set a default filter, which searches for all documents on the device, is used.

    Some of the most commonly used functionality is:
    \table
    \header
        \o Function/Enum
        \o Usage
    \row
        \o setFilter() 
        \o filter the list of documents using a QContentFilter.
    \row
        \o filter()
        \o retrieve the current QContentFilter.
    \row
        \o setSortMode()
        \o control the sorting of the list of documents.
    \row
        \o QDocumentSelector::SortMode
        \o Provides a list of supported sort modes.
    \row 
        \o currentDocument()
        \o retrieve the currently selected/highlighted document in the list.
    \row
        \o QDocumentSelector::Option
        \o provides a list of document selector configuration options.
    \row
        \o  options()
        \o  retrieve the enabled document selector options.
    \row
        \o  setOptions()
        \o  set the enabled document selector options.
    \row
        \o  enableOptions()
        \o  enable one or more options in addition to those already enabled.    
    \row    
        \o  disableOptions()
        \o  disable one or more document selector options.
    \endtable 
        
    When the QDocumentSelector::NewDocument option is enabled, QDocumentSelector adds a new document item to the selection widget.
    When the item is chosen by the user, the newSelected() signal is emitted.

    When a document is chosen, QDocumentSelector emits a
    documentSelected() signal and a QContent associated with document is passed with
    the signal.  

    The currently selected document can be retrieved using currentDocument().

    The following code is an example of how to:
    \list
        \o  chronologically order a list of documents
        \o  receive notification when the user chooses to create a document
        \o  receive notification when the user chooses a document
    \endlist

    \code
    QDocumentSelector *selector = new QDocumentSelector( this );
    selector->setSortMode( QDocumentSelector::Chronological );
    selector->enableOptions( QDocumentSelector::NewDocument );

    connect( selector, SIGNAL(newSelected()), this, SLOT(newDocument()) );
    connect( selector, SIGNAL(documentSelected(QContent)),
             this, SLOT(openDocument(QContent)) );
    \endcode

    The document selector also complies with DRM requirements so that the user can only
    choose documents which give the permissions needed by the application.  There
    are two sets of permissions:
    \list
        \o Select Permissions 
        \o Mandatory Permissions
    \endlist

    Both sets of permissions effectively define the intended usage for the document. 
    For example, the QDrmRights::Print permission  is required to print the document.  
    When the documents are displayed in the list, the selector will try to acquire
    the permissions needed to use the document.  If the document does not give a
    permission in the select permissions set, the selector will activate in doing so try 
    to acquire the permissions for that document.  If the selector is unsuccessful, the 
    document is still displayed but visually "grayed" out to indicate the document cannot 
    be selected.  Mandatory permissions differ from select permissions in that the activation
    step is not taken, the document must immediately give the mandatory permission in order 
    to be choosable.

    In the case of a generic document selector where the permissions required by the application
    depends on the document type, QDrmRights::InvalidPermission can be passed to setSelectPermission(),
    the selector will then use the default permissions of the document. 
     


    QDocumentSelector is often the first widget seen in a \l {Main Document Widget}{document-oriented application }. Used in combination with
    \l {QStackedWidget}, a typical application allows
    choosing of a document using the QDocumentSelector, before revealing the document in a viewer or editor.

   For a complete example that uses QDocumentSelector, see the tutorial \l {Tutorial: A Notes Application} {Using QDocumentSelector to Write a Notes Application}.

    \sa QDocumentSelectorDialog, QImageDocumentSelector, QImageDocumentSelectorDialog
*/
/*!
    \enum QDocumentSelector::Option
    Options for configuring a QDocumentSelector.

    \value None No special configuration options.
    \value NewDocument The first item in the documents list is the new document item
    \value TypeSelector A 'Select Type' menu item is available in the context menu which allows the user to restrict the visible document types.
    \value NestTypes The 'Select Type' dialog groups similar types into a single selectable filter which the specific types are children of,
           similar to the list below:
           \list
                \o Audio
                \list
                    \o mp3
                    \o wav
                \endlist
                \o Image
                \list
                    \o png
                    \o ...
                \endlist
                \o ...
            \endlist
    \value ContextMenu The QDocumentSelector has a context menu.
 */

/*!
    \enum QDocumentSelector::SortMode

    This enum specifies the sort order of the documents.
    \value Alphabetical The document set is ordered alphabetically on the names of the documents.
    \value ReverseAlphabetical The document set is reverse ordered alphabetically on the names of the documents.
    \value Chronological The document set is ordered on the date and time the documents were last edited.
    \value ReverseChronological The document set is reverse ordered on the date and time the documents were last edited.
    \value SortCriteria The document set is ordered by a QContentSortCriteria specified by sortCriteria().
 */

/*!
    \enum QDocumentSelector::Selection

    This enum indicates the result of displaying a document selector dialog. See select().

    \value NewSelected The user has chosen the new document option.
    \value DocumentSelected The user has chosen an existing document.
    \value Cancelled The user cancelled the dialog.
*/

/*!
    \typedef QDocumentSelector::Options

    Synonym for \c QFlags<QDocumentSelector::Option>
 */

/*!
    Constructs a new document selector widget with the given \a parent.
 */
QDocumentSelector::QDocumentSelector( QWidget *parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );

    d = new QDocumentSelectorPrivate( this );

    QLabel *typeLabel = new QLabel( this );
    typeLabel->setVisible( false );

    QLabel *categoryLabel = new QLabel( this );
    categoryLabel->setVisible( false );

    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( d );
    layout->addWidget( typeLabel );
    layout->addWidget( categoryLabel );

    connect( d, SIGNAL(documentSelected(QContent)), this, SIGNAL(documentSelected(QContent)) );
    connect( d, SIGNAL(currentChanged()), this, SIGNAL(currentChanged()) );
    connect( d, SIGNAL(newSelected()), this, SIGNAL(newSelected()) );
    connect( d, SIGNAL(documentsChanged()), this, SIGNAL(documentsChanged()) );
    connect( d, SIGNAL(typesSelected(bool)), typeLabel, SLOT(setVisible(bool)) );
    connect( d, SIGNAL(setTypeLabel(QString)), typeLabel, SLOT(setText(QString)) );
    connect( d, SIGNAL(categoriesSelected(bool)), categoryLabel, SLOT(setVisible(bool)) );
    connect( d, SIGNAL(setCategoryLabel(QString)), categoryLabel, SLOT(setText(QString)) );

    setFocusProxy( d );
}

/*!
    Destroys the selector.
 */
QDocumentSelector::~QDocumentSelector()
{
}

/*!
    Returns the current documents filter.

    The filter defines the subset of content on the device the user can select from.

    \sa setFilter(), QContentSet::filter()
 */
QContentFilter QDocumentSelector::filter() const
{
    return d->baseFilter();
}

/*!
    Sets the \a filter which defines the subset of content on the device the user can select from.

    The document list is filtered when control returns to the event loop.

    \sa filter(), QContentSet::filter()
 */
void QDocumentSelector::setFilter( const QContentFilter &filter )
{
    d->setBaseFilter( filter );
}

/*!
  Sets the document sort \a mode, the default is QDocumentSelector::Alphabetical.

  The document list is sorted when control returns to the event loop.

  \sa sortMode()
 */
void QDocumentSelector::setSortMode( SortMode mode )
{
    d->setSortMode( mode );
}

/*!
  Returns the current document sort mode.

  \sa setSortMode()
 */
QDocumentSelector::SortMode QDocumentSelector::sortMode() const
{
    return d->sortMode();
}

/*!
    Sets the document \a sort criteria.

    This will set the document selector sort mode to SortCriteria.

    The document list is sorted when control returns to the event loop.

    \sa setSortMode()
*/
void QDocumentSelector::setSortCriteria( const QContentSortCriteria &sort )
{
    d->setSortCriteria( sort );
}

/*!
    Returns the current document sort criteria.
*/
QContentSortCriteria QDocumentSelector::sortCriteria() const
{
    return d->sortCriteria();
}

/*!
    Returns the enabled document selector options.

    \sa setOptions(), enableOptions(), disableOptions()
 */
QDocumentSelector::Options QDocumentSelector::options() const
{
    return d->options();
}

/*!
    Sets the enabled set of selector \a options. 
    \sa enableOptions(), disableOptions(), options()
 */
void QDocumentSelector::setOptions( Options options )
{
    d->setOptions( options );
}

/*!
    Enables the document selector \a options, in addition to those already enabled.
    \sa disableOptions(), options(), setOptions()
*/
void QDocumentSelector::enableOptions( Options options )
{
    d->setOptions( d->options() | options );
}

/*!
    Disables the given \a options.
    \sa enableOptions(), options(), setOptions()
 */
void QDocumentSelector::disableOptions( Options options )
{
    d->setOptions( d->options() & ~options );
}

/*!
    Sets the \a categories checked by default in the document selector's category filter dialog.

    The categories displayed in a document selector's category filter dialog are restricted to categories assigned
    to documents that would appear in the document selector with no categories checked.  If a default category does
    not match any visible in the category filter dialog it will be ignored.

    Setting a default category will filter the visible documents by that category (assuming there are documents belonging
    to that category) but the user is able to remove the filtering by unchecking the category in the filter dialog.  To
    apply a filter that cannot be removed use setFilter() instead.

    \bold {Note:} Once the dialog has been shown once, this function no longer has any effect.

    Filtering according to \a categories is applied after the filter defined by filter().

    \sa defaultCategories(), filter(), setFilter(), Categories
*/
void QDocumentSelector::setDefaultCategories( const QStringList &categories )
{
    d->setDefaultCategories( categories );
}

/*!
    Returns the categories checked by default in the document selector's category filter dialog.
    
    This function is only relevant if the QDocumentSelector::ContextMenu option has been set or enabled.
    \sa setDefaultCategories(), Categories
*/
QStringList QDocumentSelector::defaultCategories() const
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
*/ void QDocumentSelector::setSelectPermission( QDrmRights::Permission
permission ) {
    d->setSelectPermission( permission );
}

/*!
    Returns the permissions a document should give in order to be choosable.

    The permissions effectively describe the document's intended usage. 

    \sa setSelectPermission(), mandatoryPermissions(), setMandatoryPermissions()
*/
QDrmRights::Permission QDocumentSelector::selectPermission() const
{
    return d->selectPermission();
};

/*!
    Sets the \a permissions a document must have in order to be choosable from the document
    selector. 

    Unlike select permissions, if a document is missing a mandatory permission it will not be activated,
    and the document cannot be chosen.

    Because the \a permissions are mandatory, passing QDrmRights::InvalidPermission as a parameter
    does not exhibit the same behavior as in setSelectPermission().

    \sa mandatoryPermissions(), setSelectPermission(), selectPermission()
*/
void QDocumentSelector::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    d->setMandatoryPermissions( permissions );
}

/*!
    Returns the permissions a document must have in order to be choosable by the document selector. 

    \sa setMandatoryPermissions(), setSelectPermission(), selectPermission()
*/
QDrmRights::Permissions QDocumentSelector::mandatoryPermissions() const
{
    return d->mandatoryPermissions();
}

/*!
    Returns a QContent for the currently selected document. 

    A null QContent is returned if there is no current selection or 
    if the new document item is currently selected.

    \sa documents()
 */
QContent QDocumentSelector::currentDocument() const {
    return d->currentDocument();
}

/*!
    Returns true if the new document item is currently selected.

    \sa setOptions()
 */
bool QDocumentSelector::newCurrent() const
{
    return d->newCurrent();
}

/*!
    Returns the content set of documents listed by the selector.
    
    \sa currentDocument() 
*/
const QContentSet &QDocumentSelector::documents() const
{
    return d->documents();
}

/*!
    Returns a list of categories selected in a document selector's category filter dialog.

    \sa setDefaultCategories()
*/
QStringList QDocumentSelector::selectedCategories() const
{
    return d->selectedCategories();
}

/*!
    \fn QDocumentSelector::documentSelected( const QContent &content )

    Signals that the user has chosen \a content from the document list.
    \sa newSelected() 
 */

/*!
    \fn QDocumentSelector::currentChanged()

    Signals that the currently selected document has changed.

    The current selection can be determined using newCurrent() and currentDocument().

    \sa newCurrent(), currentDocument()
*/

/*!
    \fn QDocumentSelector::newSelected()

    Signals that the user chosen the new document item.
    \sa documentSelected()
 */

/*!
    \fn QDocumentSelector::documentsChanged()

    This signal is emitted when the list of documents changes as
    a result of a filter change or a file system change.
*/

/*!
    Displays a modal QDocumentSelectorDialog with the given \a parent and \a title.  The
    dialog will be configured with the provided \a filter, \a options, \a sortMode and \a permission.  Choosing a 
    document will assign that document to \a content if it can be rendered with the given \a permission.

    Returns the user's chosen action.
*/
QDocumentSelector::Selection QDocumentSelector::select( QWidget *parent, QContent *content, QDrmRights::Permission permission, const QString &title, const QContentFilter &filter, Options options, SortMode sortMode )
{
    QDocumentSelectorDialog dialog( parent );

    dialog.setWindowTitle( title );
    dialog.setFilter( filter );
    dialog.setSortMode( sortMode );
    dialog.setOptions( options );
    dialog.setSelectPermission( permission );

    if( QtopiaApplication::execDialog( &dialog ) == QDialog::Accepted )
    {
        if( dialog.newSelected() )
        {
            *content = QContent();
            return NewSelected;
        }
        else
        {
            *content = dialog.selectedDocument();
            return DocumentSelected;
        }
    }
    else
        return Cancelled;
}

/*!
    Displays a modal QDocumentSelectorDialog with the given \a parent and \a title.  The
    dialog will be configured with the provided \a filter, \a options and \a sortMode.  Choosing a 
    document will assign that document to \a content.

    Returns the user's chosen action.
 */
QDocumentSelector::Selection QDocumentSelector::select( QWidget *parent, QContent *content, const QString &title, const QContentFilter &filter, Options options, SortMode sortMode )
{
    QDocumentSelectorDialog dialog( parent );

    dialog.setWindowTitle( title );
    dialog.setFilter( filter );
    dialog.setSortMode( sortMode );
    dialog.setOptions( options );

    if( QtopiaApplication::execDialog( &dialog ) == QDialog::Accepted )
    {
        if( dialog.newSelected() )
        {
            *content = QContent();

            return NewSelected;
        }
        else
        {
            *content = dialog.selectedDocument();

            return DocumentSelected;
        }
    }
    else
        return Cancelled;
}


class QDocumentSelectorDialogPrivate : public DocumentView
{
public:
    QDocumentSelectorDialogPrivate( QWidget *parent = 0 )
        : DocumentView( parent )
    {
    }

    virtual ~QDocumentSelectorDialogPrivate()
    {
    }
};

/*!
    \class QDocumentSelectorDialog
    \inpublicgroup QtBaseModule
    \ingroup documentselection

    \brief The QDocumentSelectorDialog class provides a user with the ability to select documents from
            a list of documents available on the device.

    The QDocumentSelectorDialog is a convenience class which is built around a QDocumentSelector,
    which provides all the base functionality for choosing a document.

    The document chosen in the dialog is retrieved using the selectedDocument() method.

    \sa QDocumentSelector, QImageDocumentSelector, QImageDocumentSelectorDialog
 */

/*!
    Constructs a new document selector dialog with the given \a parent.
 */
QDocumentSelectorDialog::QDocumentSelectorDialog( QWidget *parent )
    : QDialog( parent )
{
    QtopiaApplication::setMenuLike( this, true );
    QVBoxLayout *layout = new QVBoxLayout( this );

    d = new QDocumentSelectorDialogPrivate( this );

    QLabel *typeLabel = new QLabel( this );
    typeLabel->setVisible( false );

    QLabel *categoryLabel = new QLabel( this );
    categoryLabel->setVisible( false );

    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( d );
    layout->addWidget( typeLabel );
    layout->addWidget( categoryLabel );

    connect( d, SIGNAL(documentSelected(QContent)), this, SLOT(accept()) );
    connect( d, SIGNAL(newSelected()), this, SLOT(accept()) );
    connect( d, SIGNAL(typesSelected(bool)), typeLabel, SLOT(setVisible(bool)) );
    connect( d, SIGNAL(setTypeLabel(QString)), typeLabel, SLOT(setText(QString)) );
    connect( d, SIGNAL(categoriesSelected(bool)), categoryLabel, SLOT(setVisible(bool)) );
    connect( d, SIGNAL(setCategoryLabel(QString)), categoryLabel, SLOT(setText(QString)) );
}

/*!
    Destroys the dialog.
 */
QDocumentSelectorDialog::~QDocumentSelectorDialog()
{
}

/*!
    Returns the current documents filter.

    The filter defines the subset of content on the device the user can select from.

    \sa setFilter(), QContentSet::filter()
 */
QContentFilter QDocumentSelectorDialog::filter() const
{
    return d->baseFilter();
}

/*!
    Sets the \a filter which defines the subset of content on the device the user can select from.

The document list is filtered when control returns to the event loop.

    \sa filter(), QContentSet::filter()
 */
void QDocumentSelectorDialog::setFilter( const QContentFilter &filter )
{
    d->setBaseFilter( filter );
}

/*!
  Sets the document sort \a mode.

  The default mode is QDocumentSelector::Alphabetical.

  Setting the sort mode to SortCriteria will not change the sorting of the documents.

  \sa sortMode(), setSortCriteria()
 */
void QDocumentSelectorDialog::setSortMode( QDocumentSelector::SortMode mode )
{
    d->setSortMode( mode );
}

/*!
  Returns the current document sort mode.

  If the sort mode is SortCriteria sortCriteria() must be queried to get the actual document sorting.

  \sa setSortMode(), sortCriteria()
 */
QDocumentSelector::SortMode QDocumentSelectorDialog::sortMode() const
{
    return d->sortMode();
}

/*!
    Sets the document \a sort criteria.

    This will set the document selector sort mode to SortCriteria.

    The document list is sorted when control returns to the event loop.

    \sa setSortMode()
*/
void QDocumentSelectorDialog::setSortCriteria( const QContentSortCriteria &sort )
{
    d->setSortCriteria( sort );
}

/*!
    Returns the current document sort criteria.
*/
QContentSortCriteria QDocumentSelectorDialog::sortCriteria() const
{
    return d->sortCriteria();
}

/*!
    Returns the enabled document selector options.

    \sa setOptions(), enableOptions(), disableOptions()
 */
QDocumentSelector::Options QDocumentSelectorDialog::options() const
{
    return d->options();
}

/*!
    Sets the enabled set of selector \a options.
    \sa enableOptions(), disableOptions(), options()
 */
void QDocumentSelectorDialog::setOptions( QDocumentSelector::Options options )
{
    d->setOptions( options );
}

/*!
    Enables the document selector \a options, in addition to those already enabled.
    \sa disableOptions(), setOptions(), options()
 */
void QDocumentSelectorDialog::enableOptions( QDocumentSelector::Options options )
{
    d->setOptions( d->options() | options );
}

/*!
    Disables the the given \a options.

    \sa enableOptions(), options(), setOptions()

 */
void QDocumentSelectorDialog::disableOptions( QDocumentSelector::Options options )
{
    d->setOptions( d->options() & ~options );
}

/*!
    Sets the \a categories checked by default in the document selector's category filter dialog.

    The categories displayed in a document selector's category filter dialog are restricted to categories assigned
    to documents that would appear in the document selector with no categories checked.  If a default category does
    not match any visible in the category filter dialog it will be ignored.

    Setting a default category will filter the visible documents by that category (assuming there are documents belonging
    to that category) but the user is able to remove the filtering by unchecking the category in the filter dialog.  To
    apply a filter that cannot be removed use setFilter() instead.

    \bold {Note:} Once the dialog has been shown once, this function no longer has any effect.

    Filtering according to \a categories is applied after the filter defined by filter().

    \sa defaultCategories(), filter(), setFilter(), Categories
 */
void QDocumentSelectorDialog::setDefaultCategories( const QStringList &categories )
{
    d->setDefaultCategories( categories );
}

/*!
    Returns the categories checked by default in the document selector's category filter dialog.

    \sa setDefaultCategories(), Categories
 */
QStringList QDocumentSelectorDialog::defaultCategories() const
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
    permission for the document is used.

    \sa selectPermission(), setMandatoryPermissions(), mandatoryPermissions() 
 */
void QDocumentSelectorDialog::setSelectPermission( QDrmRights::Permission permission )
{
    d->setSelectPermission( permission );
}

/*!
    Returns the permissions a document should give in order to be choosable.

    The permissions effectively describe the document's intended usage.

    \sa setSelectPermission(), setMandatoryPermissions(), mandatoryPermissions()
 */
QDrmRights::Permission QDocumentSelectorDialog::selectPermission() const
{
    return d->selectPermission();
};

/*!
    Sets the \a permissions a document must have in order to be choosable from the document selector.

    Unlike select permissions, if a document is missing a mandatory permission it will not be activated,
    and the document cannot be chosen.
    
    Because the \a permissions are mandatory, passing QDrmRights::InvalidPermission as a parameter
    does not exhibit the same behavior as in setSelectPermission().

    \sa mandatoryPermissions(), setSelectPermission(), selectPermission()
  */
void QDocumentSelectorDialog::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    d->setMandatoryPermissions( permissions );
}

/*!
    Returns the permissions a document must have in order to be choosable by the document selector.
    
    \sa setMandatoryPermissions(), setSelectPermission(), selectPermission()
 */
QDrmRights::Permissions QDocumentSelectorDialog::mandatoryPermissions() const
{
    return d->mandatoryPermissions();
}

/*!
    Returns a QContent for the currently selected document.

    A null QContent is returned if there is no current selection or
    if the new document item is currently selected.
    
    \sa newSelected(), documents()
 */
QContent QDocumentSelectorDialog::selectedDocument() const
{
    return d->currentDocument();
}

/*!
    Returns true if the new document item from the list is selected.
    
    \sa selectedDocument()
 */
bool QDocumentSelectorDialog::newSelected() const
{
    return d->newCurrent();
}

/*!
    Returns the content set of documents listed by the selector.

    \sa selectedDocument()
 */
const QContentSet &QDocumentSelectorDialog::documents() const
{
    return d->documents();
}

/*!
    \reimp
*/
QSize QDocumentSelectorDialog::sizeHint() const
{
    QDesktopWidget *desktop = QApplication::desktop();

    return desktop->screenGeometry(desktop->primaryScreen()).size();
}

/*!
    Returns a list of categories selected in a document selector's category filter dialog.

    \sa setDefaultCategories()
*/
QStringList QDocumentSelectorDialog::selectedCategories() const
{
    return d->selectedCategories();
}

#include "qdocumentselector.moc"

