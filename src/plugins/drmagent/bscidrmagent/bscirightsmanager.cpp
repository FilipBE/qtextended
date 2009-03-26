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

#include "bscirightsmanager.h"
#include <bsciLibMgmt.h>
#include <qtopiaapplication.h>
#include <QTimer>
#include <QListView>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>
#include <QtopiaApplication>
#include <QLineEdit>
#include <QDialog>
#include <QPushButton>
#include <QThread>
#include <QStorageMetaInfo>
#include <QWaitWidget>
#include <QLabel>
#include <QListWidget>
#include <QHeaderView>
#include <QKeyEvent>
#include <QCheckBox>
#include <QtopiaServiceRequest>

#include <QSoftMenuBar>

class BSciRightsModelPrivate
{
public:
    BSciRightsModelPrivate()
        : row( 0 )
        , parent( 0 )
    {
    }

    BSciRightsModelPrivate( int row, BSciRightsModelPrivate *parent )
        : row( row )
        , parent( parent )
    {
    }

    ~BSciRightsModelPrivate()
    {
        qDeleteAll( children.values() );
    }

    int row;
    BSciRightsModelPrivate *parent;
    QMap< int, BSciRightsModelPrivate * > children;
};

/*!
    \class BSciRightsModel
    \inpublicgroup QtDrmModule
    \brief The BSciRightsModel class provides a model for a representing a list of rights objects.

    The OMA DRM rights model is QAbstractItemModel wrapper around a flat list of rights objects.  Rights objects
    are identified by the content they provide rights for, valid render permissions are listed as children of the 
    rights objects and constraints as children of the permissions.

    A simple list of rights objects may be formatted as below:

    \list
        \o A song
        \list
            \o Play: Valid in future
            \list
                \o Valid after: 12/08/2007 12:00 AM
                \o Valid until: 12/09/2007 12:00 AM
            \endlist
        \endlist
        \o A picture
        \list
            \o Display: Valid
            \o Print: Valid
            \list
                \o Uses: 10
            \endlist
        \endlist
        \o ...
    \endlist

    \internal
*/

/*!
    Creates a new drm rights model with the given \a parent.
*/
BSciRightsModel::BSciRightsModel( QObject *parent )
    : QAbstractItemModel( parent )
    , m_rightsCount( 0 )
    , m_rightsInfoList( 0 )
{
    d = new BSciRightsModelPrivate;

    init();
}

/*!
    Destroys a drm rights model.
 */
BSciRightsModel::~BSciRightsModel()
{
    if( m_rightsInfoList )
    {
        if( !BSciDrm::context )
            qWarning() << "~BSciRightsModel() BSci context uninitialised";
        else
            BSCIReleaseRightsList( BSciDrm::context, m_rightsCount, m_rightsInfoList );
    }

    qDeleteAll( m_contentItems.values() );

    delete d;
}

/*!
    \reimp
*/
QVariant BSciRightsModel::data( const QModelIndex &index, int role ) const
{
    if( index.isValid() )
    {
        BSciRightsModelPrivate *parent = static_cast< BSciRightsModelPrivate * >( index.internalPointer() );

        if( parent == d )
        {
            QContent c = content( index.row() );

            if( role == Qt::DisplayRole )
                return c.id() != QContent::InvalidId ? c.name() : tr( "Unknown content" );
            else if( role == Qt::DecorationRole )
                return c.icon();
        }
        else if( parent->parent == d )
        {
            if( role == Qt::DisplayRole )
            {
                QDrmRights r = rights( parent->row )[ index.row() ];

                if( index.column() == 0 )
                    return QDrmRights::toString( r.permission() );
                if( index.column() == 1 )
                {
                    switch( r.status() )
                    {
                    case QDrmRights::Valid:
                        return tr( "Valid" );
                    case QDrmRights::Invalid:
                        return tr( "Invalid" );
                    case QDrmRights::ValidInFuture:
                        return tr( "Valid in the future" );
                    }
                }
            }
        }
        else if( parent->parent->parent == d )
        {
            if( role == Qt::DisplayRole )
            {
                QDrmRights::Constraint c = rights( parent->parent->row )[ parent->row ].constraints()[ index.row() ];

                if( index.column() == 0 )
                    return c.name();
                else if( index.column() == 1 )
                    return c.value();
            }
        }
        else
        {
            if( role == Qt::DisplayRole )
            {
                QDrmRights::Constraint c = rights( parent->parent->parent->row )[ parent->parent->row ].constraints()[ parent->row ];

                if( index.column() == 0 )
                    return c.attributeName( index.row() );
                else if( index.column() == 1 )
                    return c.attributeValue( index.row() );
            }
        }
    }
    return QVariant();
}

/*!
    \reimp
*/
QModelIndex BSciRightsModel::parent( const QModelIndex &index ) const
{
    if( index.isValid() )
    {
        BSciRightsModelPrivate *parent = static_cast< BSciRightsModelPrivate * >( index.internalPointer() );

        if( parent == d )
            return QModelIndex();
        else
            return createIndex( parent->row, 0, parent->parent );
    }
    return QModelIndex();
}

/*!
    \reimp
*/
int BSciRightsModel::columnCount( const QModelIndex &parent ) const
{
    if( parent.isValid() )
        return parent.column() == 0 ? 2 : 0;
    else
        return 1;
}

/*!
    \reimp
*/
int BSciRightsModel::rowCount( const QModelIndex &parent ) const
{
    if( parent.isValid() && parent.column() == 0 )
    {
        BSciRightsModelPrivate *p = static_cast< BSciRightsModelPrivate * >( parent.internalPointer() );

        if( p == d )
        {
            return rights( parent.row() ).count();
        }
        else if( p->parent == d )
        {
            QDrmRights r = rights( p->row )[ parent.row() ];

            return r.constraints().count();
        }
        else if( p->parent->parent == d )
        {
            QDrmRights::Constraint c = rights( p->parent->row )[ p->row ].constraints()[ parent.row() ];

            return c.attributeCount();
        }
        else
        {
            return 0;
        }
    }
    else if( !parent.isValid() )
        return rightsCount();
    else
        return 0;
}

/*!
    \reimp
*/
QModelIndex BSciRightsModel::index(int row, int column, const QModelIndex &parent ) const
{
    if( parent.isValid() )
    {
        BSciRightsModelPrivate *p = static_cast< BSciRightsModelPrivate * >( parent.internalPointer() );

        QMap< int, BSciRightsModelPrivate * >::const_iterator it = p->children.find( parent.row() );

        if( it == p->children.end() )
        {
            BSciRightsModelPrivate *newParent = new BSciRightsModelPrivate( parent.row(), p );

            p->children.insert( parent.row(), newParent );

            p = newParent;
        }
        else
            p = *it;

        return createIndex( row, column, p );
    }
    else
    {
        return createIndex( row, column, d );
    }
}

/*!
    \reimp
*/
QVariant BSciRightsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    Q_UNUSED( section );
    Q_UNUSED( orientation );
    Q_UNUSED( role );

    return QVariant();
}

/*!
    Returns the content object associated with the rights object at \a index.
 */
QContent BSciRightsModel::content( const QModelIndex &index ) const
{
    if( index.isValid() )
    {
        BSciRightsModelPrivate *parent = static_cast< BSciRightsModelPrivate * >( index.internalPointer() );
        int row = index.row();

        while( parent != d )
        {
            row = parent->row;
            parent = parent->parent;
        }

        return content( row );
    }
    else
        return QContent();
}

/*!
    Return the rights object at \a index.
 */
QList< QDrmRights > BSciRightsModel::rights( const QModelIndex &index ) const
{
    if( index.isValid() )
    {
        BSciRightsModelPrivate *parent = static_cast< BSciRightsModelPrivate * >( index.internalPointer() );
        int row = index.row();

        while( parent != d )
        {
            row = parent->row;
            parent = parent->parent;
        }

        return rights( row );
    }
    else
        return QList< QDrmRights >();
}


/*!
    Deletes the rights object at \a index.
*/
void BSciRightsModel::remove( const QModelIndex &index )
{
    if( index.isValid() )
    {
        BSciRightsModelPrivate *parent = static_cast< BSciRightsModelPrivate * >( index.internalPointer() );
        int row = index.row();

        while( parent != d )
        {
            row = parent->row;
            parent = parent->parent;
        }

        remove( row );

        delete d;

        d = new BSciRightsModelPrivate;

        reset();
    }
}

void BSciRightsModel::init()
{
    loadList();

    reset();
}

void BSciRightsModel::loadList()
{
    if( !BSciDrm::context )
    {
        qWarning() << "BSci context uninitialised";
        return;
    }

    int error = BSCIGetRightsList( BSciDrm::context, 0, &m_rightsCount, &m_rightsInfoList, INCL_ALL );

    if( error != BSCI_NO_ERROR )
    {
        BSciDrm::printError( error, "DRMRightsList::DRMRightsList( ... )" );

        m_rightsCount = 0;
        m_rightsInfoList = 0;
    }
}

/*!
    Returns the number of rights objects in the list.
*/
int BSciRightsModel::rightsCount() const
{
    return m_rightsCount;
}

/*!
    Returns a const reference to a cached ContentItem at \a row.  If no ContentItem is
    cached one is created and added to the cache.
 */
const BSciRightsModel::ContentItem *BSciRightsModel::contentItem( int row ) const
{
    if( !m_contentItems.contains( m_rightsInfoList[ row ]->rightsId ) )
    {
        ContentItem *item = new ContentItem;

        item->count = 0;
        item->rightsId = m_rightsInfoList[ row ]->rightsId;

        if( QFile::exists( m_rightsInfoList[ row ]->fileURI ) )
            item->content = QContent( m_rightsInfoList[ row ]->fileURI );

        if( !BSciDrm::context )
        {
            qWarning() << "BSci context uninitialised";

            const_cast< BSciRightsModel * >( this )->m_contentItems.insert( item->rightsId, item );

            return item;
        }

        SBSciRights bsciRights;

        memset( &bsciRights, 0, sizeof(bsciRights) );

        int error = BSCIGetRightsDetails( BSciDrm::context, item->rightsId, &bsciRights );

        if( error == BSCI_NO_ERROR )
        {
            for( uint i = 0; i < bsciRights.numPermissions; i++ )
            {

                if( bsciRights.permissions[ i ].play && bsciRights.permissions[ i ].play->status != RS_NoRights )
                {
                    QDrmRights playRights = BSciDrm::constraints(
                            QDrmRights::Play, bsciRights.permissions[ i ].play->status, bsciRights.permissions[ i ].play );

                    item->rights.append( playRights );

                    item->count += 1 + playRights.constraints().count();
                }

                if( bsciRights.permissions[ i ].display && bsciRights.permissions[ i ].display->status != RS_NoRights )
                {
                    QDrmRights displayRights = BSciDrm::constraints(
                            QDrmRights::Display, bsciRights.permissions[ i ].display->status, bsciRights.permissions[ i ].display );

                    item->rights.append( displayRights );

                    item->count += 1 + displayRights.constraints().count();
                }

                if( bsciRights.permissions[ i ].execute && bsciRights.permissions[ i ].execute->status != RS_NoRights )
                {
                    QDrmRights executeRights = BSciDrm::constraints(
                            QDrmRights::Execute, bsciRights.permissions[ i ].execute->status, bsciRights.permissions[ i ].execute );

                    item->rights.append( executeRights );

                    item->count += 1 + executeRights.constraints().count();
                }

                if( bsciRights.permissions[ i ].print && bsciRights.permissions[ i ].print->status != RS_NoRights )
                {
                    QDrmRights printRights = BSciDrm::constraints(
                            QDrmRights::Print, bsciRights.permissions[ i ].print->status, bsciRights.permissions[ i ].print );

                    item->rights.append( printRights );

                    item->count += 1 + printRights.constraints().count();
                }
                if( bsciRights.permissions[ i ].xport && bsciRights.permissions[ i ].xport->status != RS_NoRights )
                {
                    QDrmRights printRights = BSciDrm::constraints(
                            QDrmRights::Export, bsciRights.permissions[ i ].xport->status, bsciRights.permissions[ i ].xport );

                    item->rights.append( printRights );

                    item->count += 1 + printRights.constraints().count();
                }
            }
        }
        else
            BSciDrm::printError( error, "DRMRightsList::contentItem( ... )" );

        const_cast< BSciRightsModel * >( this )->m_contentItems.insert( item->rightsId, item );

        return item;
    }

    return m_contentItems[ m_rightsInfoList[ row ]->rightsId ];
}

/*!
    Returns the QContent associated with the rights object at \a index.
*/
QContent BSciRightsModel::content( qint32 index ) const
{
    return contentItem( index )->content;
}

/*!
    Returns the list of rights associated with the rights object at \a index.
*/
QList< QDrmRights > BSciRightsModel::rights( qint32 index ) const
{
    return contentItem( index )->rights;
}

/*!
    Deletes the rights object at \a index.
*/
void BSciRightsModel::remove( qint32 index )
{
    int error = BSCIDeleteRights( BSciDrm::context, m_rightsInfoList[ index ]->rightsId );

    if( error != BSCI_NO_ERROR )
    {
        BSciDrm::printError( error, "BSciRightsModel::remove()" );
        return;
    }

    // Save rights change
    QContent alteredContent = contentItem( index )->content;

    if( alteredContent.isValid() )
        alteredContent.commit();

    delete m_contentItems.take( m_rightsInfoList[ index ]->rightsId );

    BSCIReleaseRightsList( BSciDrm::context, m_rightsCount, m_rightsInfoList );

    loadList();
}


/*!
    \class BSciRightsView
    \inpublicgroup QtDrmModule
    \brief The BSciRightsView class provides an item view for navigating a QBSciRightsModel.

    The OMA DRM rights view is an item view specialized for browsing an OMA DRM rights model.  In it's default display
    it shows a list of a rights objects in the model; choosing a rights object will switch to a view of the
    permissions, constraints and current validity of that rights object.

    \internal
 */

/*!
    Creates a new QOmaDrmRightsView object with the given \a parent.

    \sa setModel() 
 */
BSciRightsView::BSciRightsView( QWidget *parent )
    : QTreeView( parent )
{
    QMenu *menu = QSoftMenuBar::menuFor( this );

    viewAction   = menu->addAction( QIcon( ":icon/view" ), tr( "View license" )   );
    deleteAction = menu->addAction( QIcon( ":icon/trash" ), tr( "Delete license" ) );

    viewAction->setVisible( false );
    deleteAction->setVisible( false );

    connect( viewAction  , SIGNAL(triggered()), this, SLOT(selectCurrent()) );
    connect( deleteAction, SIGNAL(triggered()), this, SLOT(deleteCurrent()) );

    connect( this, SIGNAL(activated(QModelIndex)), this, SLOT(licenseSelected(QModelIndex)) );

    setRootIsDecorated( false );
    setItemsExpandable( false );
    setSelectionBehavior( QAbstractItemView::SelectRows );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    header()->hide();
}

/*!
    Destroys a QOmaDrmRightsView.
 */
BSciRightsView::~BSciRightsView()
{
}

/*!
    Reverts from displaying the properties of a rights object back to an item view.
 */
void BSciRightsView::back()
{
    if( rootIndex().isValid() )
    {
        QModelIndex index = rootIndex();

        setRootIndex( index.parent() );
        collapseAll();

        scrollTo( index, QAbstractItemView::PositionAtCenter );
        setCurrentIndex( index );
    }
}

/*!
    \internal
*/
void BSciRightsView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    QTreeView::currentChanged( current, previous );

    bool visible = canSelect( current );

    if( viewAction->isVisible() && !visible || !viewAction->isVisible() && visible )
        viewAction->setVisible( visible );

    visible = canDelete( current );

    if( deleteAction->isVisible() && !visible || !deleteAction->isVisible() && visible )
        deleteAction->setVisible( visible );

}

/*!
    \reimp
*/
void BSciRightsView::keyPressEvent( QKeyEvent *e )
{
    if( e->key() == Qt::Key_Back && rootIndex().isValid() )
    {
        back();

        e->accept();
    }
    else
        QTreeView::keyPressEvent( e );
}

/*!
    \reimp
*/
void BSciRightsView::focusInEvent( QFocusEvent *e )
{
//     setEditFocus( true );

    QTreeView::focusInEvent( e );

    ensureVisible();
}

void BSciRightsView::ensureVisible()
{
    if( selectionModel() && !currentIndex().isValid() )
        selectionModel()->setCurrentIndex(
                moveCursor( MoveHome, Qt::NoModifier ), // first visible index
                QItemSelectionModel::Select );
}

void BSciRightsView::licenseSelected( const QModelIndex &index )
{
    if( canSelect( index ) )
    {
        setRootIndex( index );
        setCurrentIndex( model()->index( 0, 0, rootIndex() ) );
        expandAll();
        resizeColumnToContents( 0 );
    }
    else
    {
        QString title = index.data().toString();

        QString message = model()->index( index.row(), 1, index.parent() ).data().toString();

        QMessageBox::information( indexWidget( index ), title, title + QLatin1String( ": " ) + message, QMessageBox::Ok );
    }
}

/*!
    Displays the children of the currently selected index.
 */
void BSciRightsView::selectCurrent()
{
    licenseSelected( currentIndex() );
}

/*!
    Deletes the rights associated with the currently selected index.
 */
void BSciRightsView::deleteCurrent()
{
    if( canDelete( currentIndex() ) )
    {
        QContent content = static_cast< BSciRightsModel * >( model() )->content( currentIndex() );

        int response = QMessageBox::question(
                this,
                tr( "Delete?" ),
                tr( "<qt>Deleting license may render %1 inaccessible. Continue?</qt>", "%1 = name of content" )
                        .arg( content.isValid() ? content.name() : tr( "Unknown content" ) ),
                QMessageBox::Yes,
                QMessageBox::No );

        if( response == QMessageBox::Yes )
            static_cast< BSciRightsModel * >( model() )->remove( currentIndex() );
    }
}

/*!
    Indicates whether an index can be selected to view it's children.
*/
bool BSciRightsView::canSelect( const QModelIndex &index ) const
{
    return index.isValid() && !index.parent().isValid();
}

/*!
    Indicates whether the rights object at an index can be deleted.
*/
bool BSciRightsView::canDelete( const QModelIndex &index ) const
{
    return index.isValid();
}

BSciSettings::BSciSettings( QWidget *parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , m_backupButton( 0 )
    , m_restoreButton( 0 )
    , m_backupDialog( 0 )
    , m_backupThread( 0 )
    , m_waitWidget( 0 )
    , m_storage( 0 )
    , m_agentSettings( "/OmaDrmAgent" )
{
    QCheckBox *silentRoapBox = new QCheckBox( 
            tr( "Silent license acquisition",
                "Silent licence aquisition is a way to download license without asking the user for permission" ), this );
    QCheckBox *transactionTrackingBox = new QCheckBox( tr( "Transaction tracking" ), this );

    {
        QSettings conf( "Trolltech", "OmaDrm" );

        silentRoapBox->setCheckState( conf.value( "silentroap", false ).toBool() ? Qt::Checked : Qt::Unchecked );
        transactionTrackingBox->setCheckState( conf.value( "transactiontracking", false ).toBool() ? Qt::Checked : Qt::Unchecked );
    }

    connect( silentRoapBox         , SIGNAL(stateChanged(int)), this, SLOT(enableSilentRoap(int))          );
    connect( transactionTrackingBox, SIGNAL(stateChanged(int)), this, SLOT(enableTransactionTracking(int)) );

    QVBoxLayout *layout = new QVBoxLayout( this );

    setLayout( layout );

    layout->addWidget( silentRoapBox          );
    layout->addWidget( transactionTrackingBox );

    m_backupButton = new QPushButton( tr( "Backup license store" ), this );
    m_restoreButton = new QPushButton( tr( "Restore license store" ) );

    layout->addWidget( m_backupButton );
    layout->addWidget( m_restoreButton );
    layout->addStretch();

    connect( m_backupButton, SIGNAL(clicked(bool)), this, SLOT(backup()) );
    connect( m_restoreButton, SIGNAL(clicked(bool)), this, SLOT(restore()) );

    m_storage = QStorageMetaInfo::instance();
}

/*!
    Enables or disables transaction tracking according to the Qt::CheckState value of \a enable.
*/
void BSciSettings::enableTransactionTracking( int enable )
{
    m_agentSettings.setAttribute( "TransactionTracking", enable == Qt::Checked );

    QSettings conf( "Trolltech", "OmaDrm" );

    conf.setValue( "transactiontracking", enable == Qt::Checked );
}

/*!
    Enables or disables silent ROAP according to the Qt::CheckState value of \a enable.
 */
void BSciSettings::enableSilentRoap( int enable )
{
    m_agentSettings.setAttribute( "SilentRoap", enable == Qt::Checked );

    QSettings conf( "Trolltech", "OmaDrm" );

    conf.setValue( "silentroap", enable == Qt::Checked );
}

void BSciSettings::backup()
{
    if( m_backupThread || m_backupDialog )
        return;

    QList< QFileSystem * > locations = backupLocations();

    if( locations.isEmpty() )
    {
        QMessageBox::warning(
                this,
                tr( "No back-up location" ),
                tr( "There is no removeable media inserted to back-up the license store to. Insert a media card and try again" ) );
    }
    else
    {
        m_backupDialog = new QDialog( this );

        m_backupDialog->setWindowTitle( tr( "Select a back-up location" ) );

        QLabel *label = new QLabel( tr( "Select the location to back-up the license store to:" ), m_backupDialog );

        QListWidget *list = new QListWidget( m_backupDialog );

        foreach( QFileSystem *fs, locations )
            list->addItem( fs->name() );

        QVBoxLayout *layout = new QVBoxLayout( m_backupDialog );

        layout->setMargin( 0 );

        layout->addWidget( label );
        layout->addWidget( list );

        connect( list, SIGNAL(activated(QModelIndex)), m_backupDialog, SLOT(accept()) );

        QtopiaApplication::setMenuLike( m_backupDialog, true );

        if( QtopiaApplication::execDialog( m_backupDialog ) == QDialog::Accepted )
        {
            if( !m_waitWidget )
                m_waitWidget = new QWaitWidget( this );

            m_waitWidget->show();

            QString backupPath = locations[ list->currentRow() ]->path() + QLatin1String( "/.bsci_license_backup" );

            m_backupThread = new BSciBackupThread( backupPath, this );

            connect( m_backupThread, SIGNAL(complete(int)), this, SLOT(backupComplete(int)) );

            m_backupThread->start();
        }

        delete m_backupDialog;

        m_backupDialog = 0;
    }
}

void BSciSettings::restore()
{
    if( m_backupThread || m_backupDialog )
        return;

    QList< QFileSystem * > locations = backupLocations();

    if( locations.isEmpty() )
    {
        QMessageBox::warning(
                this,
        tr( "No back-ups found" ),
        tr( "No license store back-ups were found. Insert a media card with a license store back-up and try again" ) );
    }
    else
    {
        m_backupDialog = new QDialog( this );

        m_backupDialog->setWindowTitle( tr( "Select a back-up location" ) );

        QLabel *label = new QLabel(
                tr( "Select the location with the back-up to restore the license store from:" ),
                m_backupDialog );

        QListWidget *list = new QListWidget( m_backupDialog );

        foreach( QFileSystem *fs, locations )
            list->addItem( fs->name() );

        QVBoxLayout *layout = new QVBoxLayout( m_backupDialog );

        layout->setMargin( 0 );

        layout->addWidget( label );
        layout->addWidget( list );

        connect( list, SIGNAL(activated(QModelIndex)), m_backupDialog, SLOT(accept()) );

        QtopiaApplication::setMenuLike( m_backupDialog, true );

        if( QtopiaApplication::execDialog( m_backupDialog ) == QDialog::Accepted )
        {
            if( !m_waitWidget )
                m_waitWidget = new QWaitWidget( this );

            m_waitWidget->show();

            QString backupPath = locations[ list->currentRow() ]->path() + QLatin1String( "/.bsci_license_backup" );

            m_backupThread = new BSciRestoreThread( backupPath, this );

            connect( m_backupThread, SIGNAL(complete(int)), this, SLOT(restoreComplete(int)) );

            m_backupThread->start();
        }

        delete m_backupDialog;

        m_backupDialog = 0;
    }
}

void BSciSettings::backupComplete( int result )
{
    m_backupThread = 0;

    m_waitWidget->hide();

    if( result != BSCI_NO_ERROR )
    {
        QMessageBox::warning(
                this,
        tr( "Back-up failed" ),
        tr( "The license store could not be backed-up due to an error. Error number: %1" )
                .arg( result ) );
    }
}

void BSciSettings::restoreComplete( int result )
{
    m_backupThread = 0;

    m_waitWidget->hide();

    if( result != BSCI_NO_ERROR )
    {
        QMessageBox::warning(
                this,
                tr( "Restore failed" ),
                tr( "The license store could not be restored due to an error. Error number: %1" )
                        .arg( result ) );
    }
}

QList< QFileSystem * > BSciSettings::backupLocations()
{
    QFileSystemFilter filter;

    filter.removable = QFileSystemFilter::Set;

    QList< QFileSystem * > backupLocations;

    foreach( QFileSystem *fs, m_storage->fileSystems( &filter ) )
        if( fs->isWritable() )
            backupLocations.append( fs );

    return backupLocations;
}

QList< QFileSystem * > BSciSettings::restoreLocations()
{
    QFileSystemFilter filter;

    filter.removable = QFileSystemFilter::Set;

    QList< QFileSystem * > restoreLocations;

    foreach( QFileSystem *fs, m_storage->fileSystems( &filter ) )
        if( QFile::exists( fs->path() + QLatin1String( "/.bsci_license_backup" ) ) )
            restoreLocations.append( fs );

    return restoreLocations;
}

BSciBackupThread::BSciBackupThread( const QString &fileName, QObject *parent )
    : QThread( parent )
    , m_fileName( fileName )
{
    connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

void BSciBackupThread::run()
{
    int error = BSCIBackUp( BSciDrm::context, 0, m_fileName.toLocal8Bit().constData() );

    emit complete( error );
}

BSciRestoreThread::BSciRestoreThread( const QString &fileName, QObject *parent )
    : QThread( parent )
    , m_fileName( fileName )
{
    connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

void BSciRestoreThread::run()
{
    int error = BSCIRestore( BSciDrm::context, m_fileName.toLocal8Bit().constData() );

    emit complete( error );
}
