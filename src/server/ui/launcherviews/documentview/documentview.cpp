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

#include "documentview.h"
#include "uifactory.h"
#include <QtopiaSendVia>
#include <QDialog>
#include <QSoftMenuBar>
#include <QDocumentPropertiesWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDrmContent>
#include <QtopiaApplication>
#include <QMenu>
#include <QLabel>
#include <QTimer>
#include <QCategoryDialog>
#include <QProgressBar>
#include <QValueSpaceItem>
#include "qabstractmessagebox.h"
#include <qtopiaservices.h>
#include <QContentFilterDialog>
#include <QTextEntryProxy>
#include <QMimeType>
#include <QtopiaItemDelegate>

////////////////////////////////////////////////////////////////
//
// DocumentLauncherView implementationrescan


class QLabeledProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    QLabeledProgressBar(QWidget *parent=0) : QProgressBar(parent) {};
    virtual ~QLabeledProgressBar() {};
    virtual QString text () const {return labelText;};
    virtual void setText(const QString &label) {labelText=label;};

public slots:
    void increment()
    {
        if (value() == maximum()) {
            setInvertedAppearance(!invertedAppearance());
            setValue(minimum());
        } else {
            setValue(value() + 1);
        }
    }

private:
    QString labelText;
};

/*!
  \class DocumentLauncherView
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::GeneralUI
  \brief The DocumentLauncherView class provides a view of user documents.

  It allows the user to browse through the list of documents and the user can open
  single documents by selecting them. The documents are opened based on mime types.

  In addition to the base implementation Qt Extended provides the HierarchicalDocumentLauncherView.
  The exact mapping can be configured by setting the \c {Mappings/DocumentLauncherView} key
  in the \c {Trolltech/UIFactory} configuration file. The value of the key is the name of 
  the class that implements the new DocumentLauncherView. The new class must be a subclass
  of DocumentLauncherView.

  If the above setting is missing, the DocumentLauncherView will be used.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \sa LauncherView
*/

/*!
  Creates a new DocumentLauncherView class with the given \a parent and \a flags. To increase 
  the component separation an instance of this class should be created via LauncherView::createLauncherView().

  \code
    LauncherView* view = LauncherView::createLauncherView("DocumentLauncherView", p, fl);
  \endcode

  \sa LauncherView::createLauncherView()
  */
DocumentLauncherView::DocumentLauncherView(QWidget* parent, Qt::WFlags flags)
    : LauncherView(parent, flags), typeLbl(0), actionDelete(0), actionProps(0),
    deleteMsg(0), propDlg(0), rightMenu(0), actionBeam(0), actionRightsIssuer(0),
    typeDlg(0), categoryLbl(0), categoryDlg(0)
{
    init();
}

/*!
  \fn DocumentLauncherView::~DocumentLauncherView()

  Destroys the DocumentLauncherView instance.
  */

void DocumentLauncherView::init() {
    softMenu = new QMenu(this);

    QSoftMenuBar::addMenuTo(this, softMenu);

    rightMenu = new QMenu(this);

    actionProps = new QAction( QIcon(":icon/info"), tr("Properties..."), this );
    actionProps->setVisible(false);
    QObject::connect(actionProps, SIGNAL(triggered()),
                     this, SLOT(propertiesDoc()));
    softMenu->addAction(actionProps);
    rightMenu->addAction(actionProps);

    if (QtopiaSendVia::isFileSupported()) {
        actionBeam = new QAction( QIcon(":icon/beam"), tr("Send"), this );
        actionBeam->setVisible( false );
        QObject::connect(actionBeam, SIGNAL(triggered()), this, SLOT(beamDoc()));
        softMenu->addAction(actionBeam);
        rightMenu->addAction(actionBeam);
    }

    actionDelete = new QAction( QIcon(":icon/trash"), tr("Delete..."), this );
    actionDelete->setVisible(false);
    connect(actionDelete, SIGNAL(triggered()), this, SLOT(deleteDoc()));
    softMenu->addAction(actionDelete);
    rightMenu->addAction(actionDelete);

    actionRightsIssuer = new QAction( QIcon( ":image/drm/Drm" ), "Get license", this );
    QObject::connect(actionRightsIssuer, SIGNAL(triggered()),
                     this, SLOT(openRightsIssuerURL()) );
    actionRightsIssuer->setVisible( false );
    softMenu->addAction(actionRightsIssuer);
    rightMenu->addAction(actionRightsIssuer);

    actionOpenWith = new QAction( tr("Open with..."), this );
    connect(actionOpenWith, SIGNAL(triggered()), this, SLOT(showOpenWith()));
    softMenu->addAction(actionOpenWith);
    rightMenu->addAction(actionOpenWith);
    actionOpenWith->setVisible(false);

    separatorAction = softMenu->addSeparator();
    separatorAction->setVisible(false);

    QAction *a = new QAction( tr("View Type..."), this );
    connect(a, SIGNAL(triggered()), this, SLOT(selectDocsType()));
    softMenu->addAction(a);

    a = new QAction( QIcon(":icon/viewcategory"), tr("View Category..."), this );
    connect(a, SIGNAL(triggered()), this, SLOT(selectDocsCategory()));
    softMenu->addAction(a);

    softMenu->addSeparator();

    a = new QAction( tr("Rescan System"), this );
    connect(a, SIGNAL(triggered()), this, SLOT(rescan()));
    softMenu->addAction(a);

    typeLbl = new QLabel(this);
    layout()->addWidget(typeLbl);
    typeLbl->hide();

    categoryLbl = new QLabel(this);
    layout()->addWidget(categoryLbl);
    categoryLbl->hide();// TODO: ifdef

    scanningBar = new QLabeledProgressBar(this);
    layout()->addWidget(scanningBar);
    scanningBar->setText(tr("Scanning", "Scanner is searching for documents"));
    scanningBar->setMinimum(0);
    scanningBar->setMaximum(10);
    scanningBar->hide();

    scanningBarUpdateTimer = new QTimer(this);
    scanningBarUpdateTimer->setInterval(1500);
    scanningBarUpdateTimer->setSingleShot(false);
    scanningVSItem=new QValueSpaceItem("/Documents/Scanning", this);
    updatingVSItem=new QValueSpaceItem("/Documents/Updating", this);
    installingVSItem=new QValueSpaceItem("/Documents/Installing", this);
    connect(scanningVSItem, SIGNAL(contentsChanged()), this, SLOT(updateScanningStatus()));
    connect(updatingVSItem, SIGNAL(contentsChanged()), this, SLOT(updateScanningStatus()));
    connect(installingVSItem, SIGNAL(contentsChanged()), this, SLOT(updateScanningStatus()));
    connect(scanningBarUpdateTimer, SIGNAL(timeout()), scanningBar, SLOT(increment()));

    updateScanningStatus();

    connect(this, SIGNAL(rightPressed(QContent)),
            this, SLOT(launcherRightPressed(QContent)));

    QContentFilter filter( QContent::Document );

    setFilter(filter);
    setViewMode(QListView::ListMode);
    if (!style()->inherits("QThumbStyle")) {
        textEntry = new QTextEntryProxy(this, m_smoothList);

        int mFindHeight = textEntry->sizeHint().height();
        findIcon = new QLabel;
        findIcon->setPixmap(QIcon(":icon/find").pixmap(mFindHeight-2, mFindHeight-2));
        findIcon->setMargin(2);
        findIcon->setFocusPolicy(Qt::NoFocus);

        findLayout = new QHBoxLayout;
        findLayout->addWidget(findIcon);
        findLayout->addWidget(textEntry);
        qobject_cast<QVBoxLayout*>(layout())->addLayout(findLayout);

        connect(textEntry, SIGNAL(textChanged(QString)), this, SLOT(textEntrytextChanged(QString)));
        QtopiaApplication::setInputMethodHint(this, "text nohandwriting");
        setAttribute(Qt::WA_InputMethodEnabled);
    }
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
}

DocumentLauncherView::~DocumentLauncherView()
{
}

/*!
  \reimp
  */
void DocumentLauncherView::setFilter( const QContentFilter &filter )
{
    LauncherView::setFilter( filter );

    if( typeDlg )
        typeDlg->setFilter( filter );

    if( categoryDlg )
        categoryDlg->setFilter( filter );
}

void DocumentLauncherView::launcherRightPressed(QContent lnk)
{
    if(lnk.id() != QContent::InvalidId && lnk.isValid())
        rightMenu->popup(QCursor::pos());
}

void DocumentLauncherView::beamDoc()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        QtopiaSendVia::sendFile(0, doc);
    }
}

void DocumentLauncherView::deleteDocWorker()
{
    deleteLnk.removeFiles();
    if (QFile::exists(deleteLnk.fileName())) {
        if(deleteMsg)
            delete deleteMsg;
        deleteMsg = QAbstractMessageBox::messageBox( this, tr("Delete"),
                "<qt>" + tr("File deletion failed.") + "</qt>",
                QAbstractMessageBox::Warning, QAbstractMessageBox::Ok );
        QtopiaApplication::showDialog(deleteMsg);
    }
}

void DocumentLauncherView::deleteDoc(int r)
{
    if (r == QAbstractMessageBox::Yes) {
        // We can't delete the deleteMsg object directly in the deleteDoc(int) function
        // because it is in response to the done() signal emitted by the deleteMsg object
        // which is still in use. This happens when trying to delete a read only file.
        QTimer::singleShot(10,this,SLOT(deleteDocWorker()));
    }
}

void DocumentLauncherView::deleteDoc()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        deleteLnk = doc;
        if(deleteMsg)
            delete deleteMsg;
        deleteMsg = QAbstractMessageBox::messageBox( this, tr("Delete"),
                "<qt>" + tr("Are you sure you want to delete %1?").arg(deleteLnk.name()) + "</qt>",
                QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No );
        connect(deleteMsg, SIGNAL(finished(int)), this, SLOT(deleteDoc(int)));
        QtopiaApplication::showDialog(deleteMsg);
    }
}

void DocumentLauncherView::propertiesDoc()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        propLnk = doc;
        if (propDlg)
            delete propDlg;
        propDlg = new QDocumentPropertiesDialog(propLnk, this);
        propDlg->setObjectName("document-properties");
        propDlg->showMaximized();
    }
}

void DocumentLauncherView::openRightsIssuerURL()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid())
        QDrmContent::activate( doc );
}

void DocumentLauncherView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    if( current.isValid() && !previous.isValid() )
    {
        actionProps->setVisible(true);
        actionDelete->setVisible(true);
        separatorAction->setVisible(true);
        actionOpenWith->setVisible(true);

        QContent content = currentItem();

        if( actionBeam && content.permissions() & QDrmRights::Distribute )
            actionBeam->setVisible( true );
        if( actionRightsIssuer && QDrmContent::canActivate( content ) )
            actionRightsIssuer->setVisible( true );

        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Select);
    }
    else if( !current.isValid() )
    {
        actionProps->setVisible(false);
        actionDelete->setVisible(false);
        separatorAction->setVisible(false);
        actionOpenWith->setVisible(false);

        if( actionBeam )
            actionBeam->setVisible(false);
        if( actionRightsIssuer )
            actionRightsIssuer->setVisible( false );

        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }
    else
    {
        QContent content = currentItem();

        bool distribute = content.permissions() & QDrmRights::Distribute;
        bool activate = QDrmContent::canActivate( content );

        if( actionBeam && actionBeam->isEnabled() != distribute )
            actionBeam->setVisible( distribute );
        if( actionRightsIssuer && actionRightsIssuer->isVisible() != activate )
            actionRightsIssuer->setVisible( activate );
    }
}

void DocumentLauncherView::selectDocsType()
{
    if( !typeDlg )
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

        typeDlg = new QContentFilterDialog( typePage, this );

        typeDlg->setWindowTitle( tr( "View Type" ) );
        typeDlg->setFilter( mainFilter );
        typeDlg->setObjectName( QLatin1String( "documents-type" ) );

        connect(typeDlg, SIGNAL(finished(int)), this, SLOT(docsTypeSelected()));
    }

    QtopiaApplication::showDialog( typeDlg );
}

void DocumentLauncherView::docsTypeSelected()
{
    showType( typeDlg->checkedFilter() );

    QString label = typeDlg->checkedLabel();

    if( !typeFilter.isValid() || label.isEmpty() )
        typeLbl->hide();
    else
    {
        typeLbl->setText( tr("Type: %1").arg( label ) );
        typeLbl->show();
    }
}

void DocumentLauncherView::selectDocsCategory()
{
    if( !categoryDlg )
    {
        QContentFilterModel::Template categoryPage;

        categoryPage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        categoryPage.addList( QContentFilter::Category );
        categoryPage.addList( QContentFilter::Category, QLatin1String( "Documents" ) );

        categoryDlg = new QContentFilterDialog( categoryPage, this );

        categoryDlg->setWindowTitle( tr( "View Category" ) );
        categoryDlg->setFilter( mainFilter );
        categoryDlg->setObjectName( QLatin1String( "documents-category" ) );

        connect(categoryDlg, SIGNAL(finished(int)), this, SLOT(docsCategorySelected()));
    }

    QtopiaApplication::showDialog( categoryDlg );
}

void DocumentLauncherView::docsCategorySelected()
{
    showCategory( categoryDlg->checkedFilter() );

    QString label = categoryDlg->checkedLabel();

    if( !categoryFilter.isValid() || label.isEmpty() )
        categoryLbl->hide();
    else
    {
        categoryLbl->setText( tr("Category: %1").arg( label ) );
        categoryLbl->show();
    }
}

void DocumentLauncherView::updateScanningStatus()
{
    const bool scanning = scanningVSItem && scanningVSItem->value().toBool();
    const bool updating = updatingVSItem && updatingVSItem->value().toBool();
    const bool installing = installingVSItem && installingVSItem->value().toBool();

    if (scanning || updating || installing) {
        if (!scanningBarUpdateTimer->isActive()) {
            scanningBarUpdateTimer->start();
            scanningBar->show();
        }
    } else if (scanningBarUpdateTimer->isActive()) {
        scanningBarUpdateTimer->stop();
        scanningBar->hide();
        scanningBar->setValue(scanningBar->minimum());
        scanningBar->setInvertedAppearance(false);
    }
}

void DocumentLauncherView::textEntrytextChanged(const QString &text)
{
    if (text.length() == 0)
        setAuxiliaryFilter(QContentFilter());
    else
        setAuxiliaryFilter(QContentFilter(QContentFilter::Name, '*'+text+'*'));
}

void DocumentLauncherView::rescan()
{
    QContentSet::scan("all");
}

// copied/pasted from NewDocumentProxyModel
class OtherProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    enum
    {
        IsDefaultRole = 1000
    };

    OtherProxyModel( QIcon newIcon, QObject *parent = 0 )
        : QAbstractProxyModel( parent )
        , m_otherEnabled( false )
        , m_hasDocuments( true )
        , m_newIcon( newIcon )
    {
    }

    virtual ~OtherProxyModel()
    {
    }

    QContent defaultApplication() const
    {
        return m_defaultApplication;
    }

    void setDefaultApplication(const QContent &application)
    {
        QContent oldDefault = m_defaultApplication;

        m_defaultApplication = application;

        for (int i = 0; i < rowCount(); ++i) {
            QModelIndex idx = index(i, 0);

            QContent content = qvariant_cast<QContent>(idx.data(QContentSetModel::ContentRole));

            if (content == oldDefault || content == m_defaultApplication)
                emit dataChanged(idx, idx);
        }
    }

    virtual QModelIndex mapFromSource( const QModelIndex &index ) const
    {
        if( index.isValid() )
        {
            return m_otherEnabled
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
            return m_otherEnabled
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
        if( m_otherEnabled && index.row() == 0 )
        {
            if( role == Qt::DisplayRole )
                return tr( "Other..." );
            else if( role == Qt::DecorationRole )
                return m_newIcon;
        }
        else if( !m_hasDocuments && index.row() == 0 )
        {
            if( role == Qt::DisplayRole )
                return tr( "No documents found" );
        }
        else if( index.isValid() && sourceModel() )
        {
            if (role == IsDefaultRole) {
                QContent application = qvariant_cast<QContent>(
                        sourceModel()->data(mapToSource(index), QContentSetModel::ContentRole));

                return application == m_defaultApplication;
            } else {
                return sourceModel()->data(mapToSource(index), role);
            }
        }

        return QVariant();
    }

    virtual Qt::ItemFlags flags( const QModelIndex & index ) const
    {
        if( m_otherEnabled && index.row() == 0 )
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

            if( m_otherEnabled )
                count++;
            else if( !m_hasDocuments )
                count = 1;

            return count;
        }

        return 0;
    }

    void setOtherEnabled( bool enabled )
    {
        if( enabled && !m_otherEnabled )
        {
            if( m_hasDocuments )
            {
                beginInsertRows( QModelIndex(), 0, 0 );
                m_otherEnabled = true;
                endInsertRows();
            }
            else
            {
                m_otherEnabled = true;
                QModelIndex index = createIndex( 0, 0 );
                emit dataChanged( index, index );
            }
        }
        else if( !enabled && m_otherEnabled )
        {
            if( m_hasDocuments )
            {
                beginRemoveRows( QModelIndex(), 0, 0 );
                m_otherEnabled = false;
                endRemoveRows();
            }
            else
            {
                m_otherEnabled = false;
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
            if( m_otherEnabled )
                beginRemoveColumns( QModelIndex(), start + 1, end + 1 );
            else
                beginRemoveColumns( QModelIndex(), start, end );
        }
    }

    void _columnsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_otherEnabled )
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
        if( m_otherEnabled )
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
                if( !m_otherEnabled )
                {
                    beginRemoveRows( QModelIndex(), 0, 0 );

                    m_hasDocuments = true;

                    endRemoveRows();
                }
                else
                    m_hasDocuments = false;
            }

            if( m_otherEnabled )
                beginInsertRows( QModelIndex(), start + 1, end + 1 );
            else
                beginInsertRows( QModelIndex(), start, end );
        }
    }

    void _rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_otherEnabled )
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
            if( !m_otherEnabled )
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
    bool m_otherEnabled;
    bool m_hasDocuments;
    QIcon m_newIcon;
    QContent m_defaultApplication;
};

class OpenWithItemDelegate : public QtopiaItemDelegate
{
public:
    OpenWithItemDelegate(QObject *parent = 0)
        : QtopiaItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (index.data(OtherProxyModel::IsDefaultRole).toBool()) {
            QStyleOptionViewItem opt = option;

            opt.font.setBold(true);

            QtopiaItemDelegate::paint(painter, opt, index);
        } else {
            QtopiaItemDelegate::paint(painter, option, index);
        }
    }
};

class QOpenWithDialog : public QDialog
{
    Q_OBJECT
public:
    QOpenWithDialog(const QContent &document, QWidget *parent=0);
    ~QOpenWithDialog();
    const QContent currentItem() const;

public slots:
    void accept();

private slots:
    void itemClicked(const QModelIndex& index);
    void setAsDefault();
    void addAssociation();
    void removeAssociation();
    void menuAboutToShow();

private:
    QContentSet *viewSet;
    QContentSetModel *viewModel;
    OtherProxyModel *m_proxyModel;
    QListView *listView;
    QAction *associationAction;
    QAction *removeAction;
    QAction *defaultAction;
    QAbstractMessageBox *removeFailedMsg;
    QContent document;
    QContentList applications;
    QContent defaultApplication;
};

QOpenWithDialog::QOpenWithDialog(const QContent &adocument, QWidget *parent)
    : QDialog(parent)
    , associationAction(0)
    , removeAction(0)
    , defaultAction(0)
    , removeFailedMsg(0)
    , document(adocument)
{
    setObjectName("document-openwith");

    applications = QMimeType::applicationsFor(document);
    defaultApplication = QMimeType::defaultApplicationFor(document);

    setWindowTitle(tr("Open %1", "Open <document name>").arg(document.name()));
    QtopiaApplication::setMenuLike( this, true );
    listView = new QListView(this);
    listView->setFrameStyle( QFrame::NoFrame );
    listView->setItemDelegate(new OpenWithItemDelegate(listView));
    listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setLayout(new QVBoxLayout(this));
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(new QLabel(tr("Select application:")));
    layout()->addWidget(listView);
    viewSet = new QContentSet(this);

    QMenu *menu = QSoftMenuBar::menuFor(this);
    defaultAction = menu->addAction(tr("Set as default"), this, SLOT(setAsDefault()));
    associationAction = menu->addAction(tr("Add association"), this , SLOT(addAssociation()));
    removeAction = menu->addAction(tr("Remove association"), this, SLOT(removeAssociation()));

    connect(menu, SIGNAL(aboutToShow()), this, SLOT(menuAboutToShow()));

    viewModel = new QContentSetModel(viewSet, this);
    m_proxyModel = new OtherProxyModel( QIcon(":icon/qpe/AppsIcon"), this );
    m_proxyModel->setSourceModel( viewModel );
    m_proxyModel->setDefaultApplication(defaultApplication);
    listView->setModel(m_proxyModel);

    if(!applications.isEmpty())
    {
        foreach(QContent content, applications)
        {
            if(!viewSet->contains(content))
                viewSet->add(content);
        }

        m_proxyModel->setOtherEnabled(true);
    }
    else
    {
        viewSet->clear();
        QContentFilter filters = (QContentFilter( QContent::Application ))
                & QContentFilter( QContentFilter::Category, "Applications" );
        viewSet->setCriteria(filters);
        m_proxyModel->setOtherEnabled(false);
    }

    connect(listView, SIGNAL(activated(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));

    listView->selectionModel()->setCurrentIndex(
            m_proxyModel->index(0, 0), QItemSelectionModel::ClearAndSelect);
}

QOpenWithDialog::~QOpenWithDialog()
{
}

void QOpenWithDialog::itemClicked(const QModelIndex& index)
{
    if(index.data(Qt::DisplayRole) == tr("Other..."))
    {
        viewSet->clear();
        QContentFilter filters = QContentFilter(QContent::Application)
                & QContentFilter( QContentFilter::Category, "Applications" );
        viewSet->setCriteria(filters);
        m_proxyModel->setOtherEnabled(false);
    }
    else
        accept();
}

const QContent QOpenWithDialog::currentItem() const
{
    return viewModel->content(listView->currentIndex());
}

void QOpenWithDialog::setAsDefault()
{
    QContent application = qvariant_cast<QContent>(
            listView->currentIndex().data(QContentSetModel::ContentRole));

    if (application.id() != QContent::InvalidId) {
        QMimeType::setDefaultApplicationFor(document.type(), application);

        m_proxyModel->setDefaultApplication(application);
        defaultApplication = application;
    }
}

void QOpenWithDialog::addAssociation()
{
    QContent application = qvariant_cast<QContent>(
            listView->currentIndex().data(QContentSetModel::ContentRole));

    if (application.id() != QContent::InvalidId) {
        QMimeType::addAssociation(
                document.type(),
                application.fileName(),
                application.iconName(),
                QDrmRights::Unrestricted);

        applications.append(application);

        defaultApplication = QMimeType::defaultApplicationFor(document);

        m_proxyModel->setDefaultApplication(defaultApplication);
    }
}

void QOpenWithDialog::removeAssociation()
{
    QContent application = qvariant_cast<QContent>(
            listView->currentIndex().data(QContentSetModel::ContentRole));

    if (application.id() != QContent::InvalidId) {
        QMimeType::removeAssociation(document.type(), application.fileName());

        applications = QMimeType::applicationsFor(document);

        defaultApplication = QMimeType::defaultApplicationFor(document);
        m_proxyModel->setDefaultApplication(defaultApplication);

        if (applications.contains(application)) {
            delete removeFailedMsg;

            removeFailedMsg = QAbstractMessageBox::messageBox(
                    this,
                    tr("Association read-only"),
                    tr("The association between %1 and %2 cannot be removed",
                        "%1 = application name, %2 = document name")
                            .arg(application.name())
                            .arg(document.name()),
                    QAbstractMessageBox::QAbstractMessageBox::Information,
                    QAbstractMessageBox::Ok);
            removeFailedMsg->setWindowModality(Qt::WindowModal);

            QtopiaApplication::showDialog(removeFailedMsg);
        } else {
            viewSet->remove(application);
        }

    }
}

void QOpenWithDialog::accept()
{
    currentItem().execute(QStringList() << document.fileName());

    QDialog::accept();
}

void QOpenWithDialog::menuAboutToShow()
{
    QContent application = qvariant_cast<QContent>(
            listView->currentIndex().data(QContentSetModel::ContentRole));

    if (application.id() == QContent::InvalidId) {
        associationAction->setVisible(false);
        removeAction->setVisible(false);
        defaultAction->setVisible(false);
    } else if (applications.contains(application)) {
        associationAction->setVisible(false);
        removeAction->setVisible(true);
        defaultAction->setVisible(application.id() != defaultApplication.id());
    } else {
        associationAction->setVisible(true);
        removeAction->setVisible(false);
        defaultAction->setVisible(false);
    }
}

/*!
  This function opens a dialog that allows the user to select an application that 
  should be used to open the currently selected document. This would be required if the
  current document does not have a valid mime type.
  */
void DocumentLauncherView::showOpenWith()
{
    const QContent doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        QOpenWithDialog *openWithDlg = new QOpenWithDialog(doc, this);
        openWithDlg->setAttribute(Qt::WA_DeleteOnClose);
        openWithDlg->setWindowModality(Qt::WindowModal);

        QtopiaApplication::showDialog(openWithDlg);
    }
}

#include "documentview.moc"

UIFACTORY_REGISTER_WIDGET(DocumentLauncherView);
