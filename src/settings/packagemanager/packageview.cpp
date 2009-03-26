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

#include "packageview.h"
#include "packagemodel.h"
#include "serveredit.h"

#include <QBoxLayout>
#include <QTextEdit>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <QTimer>
#include <QtopiaApplication>
#include <QKeyEvent>
#include <QWaitWidget>
#include <qsoftmenubar.h>
#include <qtopialog.h>

#ifndef QT_NO_SXE
#include "domaininfo.h"
#endif

#include "packagemanagerservice.h"

/*\internal
The purpose of the keyfilter class is a workaround for the issue
where pressing the right and left key selects the first item in the
installed and download treeviews, rather than navigating
between tabs.

To avoid this we intercept the left and right key presses and let the
tabWidget handle the event to correctly handle tab navigation.
*/
KeyFilter::KeyFilter(QTabWidget *tab, QObject *parent)
    :QObject(parent),m_tab(tab)
{
}

bool KeyFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if ( event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Right || keyEvent->key() == Qt::Key_Left )
        {
            QCoreApplication::sendEvent( m_tab, event);
            return true;
        }
    }
    return false;
}

/*! \internal
    Case sensitive less than operation for two QActions.
    Intended to be used with qSort on a list of QActions.
*/
bool actionLessThan(QAction *a, QAction *b)
{
    return a->text() < b->text();
}

/*!
  \internal
  \enum PackageDetails::Type

  This enum specifies the type of dialog to be shown

  \value Info
         A information dialog should be shown
  \value Confirm
         A confirmation dialog should be shown (only for install)
*/

/*!
  \internal
  \enum PackageDetails::Option

  This enum specifies options to appear in the context menu.
  These flags are mutually exclusive so only one should
  be used at a time.

  \value None
         specifies whether there should be no extra options available
         in the context menu.  Only for use with the \c Info type.
  \value Install
         Specifies whether Install should be available in the context menu.
         Only for use with the \c Info type.
  \value Uninstall
         Specifies whether Uninstall should be available in the context menu
         Only for use with the \c Info type.
  \value Allow
         Specifies whether Confirm and Cancel should be available in the context
         menu.  Only for use with the \c Confirm type.
  \value Disallow
         Specifies that Confirm and Cancel should \i not be available in
         the context menu.  Only for use with the \c Confirm type.
*/

/*!
  \internal
  \enum PackageDetails::Result

  This enum specifies extra result codes for the dialog

  \value Proceed
         Specifies whether the dialog should proceed to the next stage.
         Depending ont the \c Type and \c Options, this can take on different meanings.
         For a \c Confirm dialog, proceed means to install.
         For a \c Info dialog, proceed means to confirm install/uninstall depending
         on the \c Options
*/

/*! \internal
    Constructs a package details dialog with the specified \a title, \a text
    \a type, and \a options.  Only one of the possible flags and not
    a combination should be passed to \a options.
*/
PackageDetails::PackageDetails(QWidget *parent,
    const QString &title,
    const QString &text,
    Type type,
    Options options)
    :
    QDialog(parent), m_type(type),m_options(options),
    m_acceptAction(0), m_rejectAction(0),m_contextMenu(0)
{
    setupUi(this);
    setModal(true);

    setWindowTitle( title );
    description->setHtml( text );

    QSoftMenuBar::setLabel(this->description, Qt::Key_Select, QSoftMenuBar::NoLabel); //shouldn't be need once
    QSoftMenuBar::setLabel(this->description, Qt::Key_Back, QSoftMenuBar::Back);      //softkey helpers are in place
    QtopiaApplication::setMenuLike( this, true );
    init();
}

void PackageDetails::init()
{
    m_contextMenu = new QMenu( this );
    QSoftMenuBar::addMenuTo( this->description, m_contextMenu );

    QSoftMenuBar::setHelpEnabled( this->description,true);

    if ( m_type == PackageDetails::Info )
    {
        if (m_options & Install)
            m_acceptAction = new QAction( PackageView::tr("Install"), this );
        else if ( m_options & Uninstall )
            m_acceptAction = new QAction( PackageView::tr("Uninstall"), this );
    }
    else if ( m_type == PackageDetails::Confirm )
    {
        if ( m_options & Allow )
        {
            m_acceptAction = new QAction( tr("Confirm"), this );
        }
        else if (m_options & Disallow )
        {
            QSoftMenuBar::setLabel(this->description, Qt::Key_Context1, QSoftMenuBar::NoLabel);
            QSoftMenuBar::setHelpEnabled( this->description,false);
        }
    }

    if ( m_acceptAction )
    {
        QSignalMapper *signalMapper = new QSignalMapper(this);
        connect( m_acceptAction, SIGNAL(triggered()), signalMapper,SLOT(map()) );
        signalMapper->setMapping( m_acceptAction, PackageDetails::Proceed );
        connect( signalMapper, SIGNAL(mapped(int)), this, SLOT(done(int)) );
        m_contextMenu->addAction( m_acceptAction );

        if ( m_type == PackageDetails::Confirm  && (m_options & Allow) )
        {
            m_rejectAction = new QAction( tr("Cancel"), this );
            connect( m_rejectAction, SIGNAL(triggered()), this, SLOT(reject()) );
            m_contextMenu->addAction( m_rejectAction );
            QSoftMenuBar::setLabel(this->description, Qt::Key_Back, QSoftMenuBar::Cancel);
        }
    }
}

/*
    \internal
    The ViewDelegate lets us correctly show
    focused items as highlighted
   */
ViewDelegate::ViewDelegate( QObject *parent ):
    QtopiaItemDelegate( parent )
{
}

void ViewDelegate::paint( QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index ) const
{
    QModelIndex parent = index.parent();
    if ( !parent.isValid() )
        return;

    QStyleOptionViewItem options = option;

    //if a package has focus, ensure that it's state is selected
    //so that it will always show with a filled in background highlight
    //(as opposed to a border/box)
    if (options.state & QStyle::State_HasFocus)
    {
        options.showDecorationSelected = true;
        options.state = options.state | QStyle::State_Selected;
    }

    QtopiaItemDelegate::paint( painter, options, index );
}

/* \internal
    The download view delegate lets us show
    installed packages as semi-transparent(/faded) text. */
DownloadViewDelegate::DownloadViewDelegate(QObject *parent)
    :ViewDelegate(parent)
{
}

void DownloadViewDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index)  const
{
    QModelIndex parent = index.parent();
    if (!parent.isValid())
        return;

    QStyleOptionViewItem options = option;
    bool isInstalled = (qobject_cast<const PackageModel*>(index.model()))
                        ->isInstalled(index);
    if (isInstalled)
    {
        QColor c = options.palette.color(QPalette::Text);
        c.setAlphaF(0.4);
        options.palette.setColor(QPalette::Text, c);
        options.palette.setColor(QPalette::HighlightedText, c);
    }
    ViewDelegate::paint(painter, options, index);
}

const int PackageView::InstalledIndex = 0;
const int PackageView::DownloadIndex = 1;

PackageView::PackageView( QWidget *parent, Qt::WFlags flags )
    : QMainWindow( parent, flags )
{
    setWindowTitle( tr( "Package Manager" ));
    model = new PackageModel( this );

    connect(model, SIGNAL(targetsUpdated(QStringList)),
            this, SLOT(targetsChanged(QStringList)));
    connect(model, SIGNAL(serversUpdated(QStringList)),
            this, SLOT(serversChanged(QStringList)));
    connect(this, SIGNAL(targetChoiceChanged(QString)),
            model, SLOT(userTargetChoice(QString)));
    connect(model, SIGNAL(serverStatus(QString)),
            this, SLOT(postServerStatus(QString)));
    connect(model,  SIGNAL(newlyInstalled(QModelIndex)),
            this, SLOT(selectNewlyInstalled(QModelIndex)));

    //setup view for installed packages
    installedView = new QTreeView( this );
    installedView->setModel( model  );
    installedView->setRootIndex( model->index(InstalledIndex,0,QModelIndex()) );
    installedView->setRootIsDecorated( false );
    connect( installedView, SIGNAL(activated(QModelIndex)),
            this, SLOT(activateItem(QModelIndex)) );
    connect( model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            installedView, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            installedView, SLOT(rowsRemoved(QModelIndex,int,int)));

    //setup page for installed packages
    QWidget *installedPage = new QWidget;
    QVBoxLayout *vbInstalledPage = new QVBoxLayout( installedPage );
    vbInstalledPage->setSpacing( 2 );
    vbInstalledPage->setMargin( 2 );
    vbInstalledPage->addWidget( installedView );

    //setup view for downloadable packages
    downloadView = new QTreeView( this );
    downloadView->setModel( model );
    downloadView->setRootIndex( model->index(DownloadIndex,0,QModelIndex()) );
    downloadView->setRootIsDecorated( false );
    connect( downloadView, SIGNAL(activated(QModelIndex)),
            this, SLOT(activateItem(QModelIndex)) );
    connect( model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            downloadView, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            downloadView, SLOT(rowsRemoved(QModelIndex,int,int)));
    downloadView->setItemDelegate(new DownloadViewDelegate(this));
    installedView->setItemDelegate(new ViewDelegate(this));

    //setup page for downloadable packages
    QWidget *downloadPage = new QWidget(this);
    QVBoxLayout *vbDownloadPage = new QVBoxLayout( downloadPage );
    vbDownloadPage->setSpacing( 2 );
    vbDownloadPage->setMargin( 2 );
    vbDownloadPage->addWidget( downloadView );
    statusLabel = new QLabel( tr("No Server Chosen"), this );
    statusLabel->setWordWrap( true );
    vbDownloadPage->addWidget( statusLabel );

    installedView->hideColumn( 1 );
    installedView->header()->hide();
    downloadView->hideColumn( 1 );
    downloadView->header()->hide();
//TODO: install to media card
     menuTarget = new QMenu( tr( "Install to" ), this );
    new PackageManagerService( this );

    tabWidget = new QTabWidget( this );
    tabWidget->addTab( installedPage, tr( "Installed" ) );
    tabWidget->addTab( downloadPage, tr( "Downloads" ) );
    setCentralWidget( tabWidget );

    KeyFilter *keyFilter = new KeyFilter(tabWidget, this);
    installedView->installEventFilter(keyFilter);
    downloadView->installEventFilter(keyFilter);

    QTimer::singleShot( 0, this, SLOT(init()) );
}

PackageView::~PackageView()
{
}

/**
  Post construction initialization, done after event loop
  displays the gui
*/
void PackageView::init()
{
    //setup context menu for installed packages
    QWidget * installedPage = tabWidget->widget( 0 );
    QMenu *installedContext = QSoftMenuBar::menuFor( installedPage );
    detailsAction = new QAction( tr("Details"), this);
    detailsAction->setVisible( false );
    connect( detailsAction, SIGNAL(triggered()), this, SLOT(displayDetails()) );

    uninstallAction = new QAction( tr("Uninstall"), installedPage );
    uninstallAction->setVisible( false );
    connect( uninstallAction, SIGNAL(triggered()), this, SLOT(startUninstall()) );

    reenableAction = new QAction( tr("Re-enable"), installedPage );
    reenableAction->setVisible( false );
    connect( reenableAction, SIGNAL(triggered()), this, SLOT(confirmReenable()) );

    installedContext->addAction( detailsAction );
    installedContext->addAction( uninstallAction );
    installedContext->addAction( reenableAction );
    connect( installedContext, SIGNAL(aboutToShow()),
                this,   SLOT(contextMenuShow()) );

    //setup context menu for downloadable packages
    QWidget * downloadPage = tabWidget->widget( 1 );
    QMenu *downloadContext = QSoftMenuBar::menuFor( tabWidget->widget(1) );
    installAction = new QAction( tr("Install"), downloadPage );
    installAction->setVisible( false );
    connect( installAction, SIGNAL(triggered()), this, SLOT(startInstall()) );

    menuServers = new QMenu( tr( "Connect" ), this );

    QAction* editServersAction = new QAction( tr( "Edit servers" ), this );
    connect( editServersAction, SIGNAL(triggered()), this, SLOT(editServers()) );

    downloadContext->addMenu( menuServers );
    downloadContext->addAction( detailsAction );
    downloadContext->addAction( installAction );
    downloadContext->addAction( editServersAction );

    connect( downloadContext, SIGNAL(aboutToShow()),
                this,   SLOT(contextMenuShow()) );

    model->populateServers();
    QStringList servers = model->getServers();
    qSort( servers );
    QAction *sa;
    serversActionGroup = new QActionGroup( this );
    serversActionGroup->setExclusive( true );
    connect( serversActionGroup, SIGNAL(triggered(QAction*)),
             this, SLOT(serverChoice(QAction*)) );
    for ( int i = 0; i < servers.count(); i++ )
    {
        sa = new QAction( servers[i], serversActionGroup );
        serversActionGroup->addAction( sa );
    }
    menuServers->addActions( serversActionGroup->actions() );

    targetActionGroup = new QActionGroup( this );
    targetActionGroup->setExclusive( true );
    connect( targetActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(targetChoice(QAction*)) );

     waitWidget = new QWaitWidget( this );
}

void PackageView::activateItem( const QModelIndex &item )
{
    if ( model->rowCount( item ) > 0 ) // has children
    {
        if ( installedView->isExpanded( item ) )
            installedView->collapse( item );
        else
            installedView->expand( item );
    }

    QModelIndex ix = item.parent();
    if ( ix.isValid() ) // is a child node
        showDetails( item, PackageDetails::Info );
}

void PackageView::serverChoice( QAction *a )
{
    QString server = a->text();
    model->setServer( server );
}

void PackageView::targetChoice( QAction * )
{
    if ( targetActionGroup->checkedAction() == 0 )
        return;
    QString newChoice = targetActionGroup->checkedAction()->text();
    if ( !prevTarget.isEmpty() && prevTarget == newChoice )
        return;
    prevTarget = newChoice;
    emit targetChoiceChanged( newChoice );
}

/*! \internal
    Updates list of available servers to connect to under the
    "Connect" option of the context menu.
*/
void PackageView::serversChanged( const QStringList &servers )
{
    QList<QAction*> actionList = serversActionGroup->actions();
    QStringList serverList = servers;
    qSort( serverList );
    QAction *sa;

    // remove from the list everything thats in the menu, and
    // if something is not in the list, remove it from the menu
    for ( int i = 0; i < actionList.count(); i++ )
    {
        sa = actionList[i];
        if ( serverList.contains( sa->text() ) )
        {
            serverList.removeAll( sa->text() );
        }
        else
        {
            serversActionGroup->removeAction( sa );
            menuServers->removeAction( sa );
            delete sa;
        }
    }

    // now the list contains just what wasnt in the menu so add
    // them if there are any
    for ( int i = 0; i < serverList.count(); i++ )
        sa = new QAction( serverList[i], serversActionGroup );

    actionList = serversActionGroup->actions();
    qSort( actionList.begin(), actionList.end(), actionLessThan );

    menuServers->clear();
    menuServers->addActions( actionList );
}

/**
  Slot to receive disk/storage add or removal messages from the model
  and alter the menus accordingly
*/
void PackageView::targetsChanged( const QStringList &targets )
{
    QList<QAction*> actionList = targetActionGroup->actions();
    QStringList targetList = targets;
    QAction *sa;
    // remove from the list everything thats in the menu, and
    // if something is not in the list, remove it from the menu
    for ( int i = 0; i < actionList.count(); i++ )
    {
        sa = actionList[i];
        if ( targetList.contains( sa->text() ))
        {
            targetList.removeAll( sa->text() );
        }
        else
        {
            targetActionGroup->removeAction( sa );
            menuTarget->removeAction( sa );
            delete sa;
        }
    }
    // now the list contains just what wasnt in the menu so add
    // them if there are any
    for ( int i = 0; i < targetList.count(); i++ )
        sa = new QAction( targetList[i], targetActionGroup );

    if ( targetActionGroup->checkedAction() == 0 )
    {
        actionList = targetActionGroup->actions();
        actionList[0]->setChecked( true );
    }
    menuTarget->addActions( targetActionGroup->actions() );
}

void PackageView::editServers()
{
    ServerEdit *edit = new ServerEdit( this );

    int result = edit->exec();

    if ( result == QDialog::Accepted && edit->wasModified() )
        model->setServers( edit->serverList() );

    delete edit;
}

/*!
    \internal
    \fn void PackageView::targetChoiceChanged( const QString & )
    Emitted when the user choice of installation target changes, eg
    from "Internal Storage" to "CF Card"
*/


/* \a type must be either be Install or Info */
void PackageView::showDetails( const QModelIndex &item , PackageDetails::Type type )
{
    if( !item.isValid() || !item.parent().isValid()
        || !model->hasIndex(item.row(), item.column(), item.parent()) )
        return;

    if ( type != PackageDetails::Confirm && type != PackageDetails::Info )
        return;

    QString name = model->data( item, Qt::DisplayRole ).toString(); //package name
    PackageDetails::Options options;
    QString text;
    if ( type == PackageDetails::Confirm )
    {
        options = PackageDetails::Allow;
#ifndef QT_NO_SXE
        if( DomainInfo::hasSensitiveDomains(model->data(item,AbstractPackageController::Domains).toString()) )
        {
            text = QLatin1String( "<font color=\"#0000FF\">");
            text += tr( "The package <font color=\"#0000FF\">%1</font> <b>cannot be installed</b> as it utilizes protected resources" ).arg( name );
            text += QLatin1String("</font>");
            type = PackageDetails::Confirm;
            options = PackageDetails::Disallow;
        }
        else
        {
            text = QString("<font color=\"#000000\">") + "<center><b><u>" + tr( "Security Alert" ) + "</u></b></center><p>"
                    + QString("%1")
                    .arg( DomainInfo::explain( model->data( item, AbstractPackageController::Domains ).toString(), name ));
        }
#else
        text = QString("<font color=\"#000000\">") + "<center><b><u>" + tr( "Confirm Install") + " </u></b></center><p>"
                    + tr("About to install <font color=\"#0000CC\"><b> %1 </b></font>", "%1 = package name")
                        .arg( name );
#endif
        if ( options == PackageDetails::Allow )
            text += QString("<br>") + tr( "Confirm Install?" ) + QString("</font>");
    }
    else //must be requesting package information
    {
        text = QString("<font color=\"#000000\">") + model->data(item, Qt::WhatsThisRole).toString() +"</font>";

        if ( tabWidget->currentIndex() == InstalledIndex )
            options = PackageDetails::Uninstall;
        else //must be DownloadIndex
        {
            //check if package has been installed
            if (model->isInstalled(item))
                options= PackageDetails::None;  //don't allow install option to appear
                                                //as a context menu option
            else
                options = PackageDetails::Install;  //let install option appear
                                                    //as a context menu option
        }
#ifndef QT_NO_SXE
        if( DomainInfo::hasSensitiveDomains(model->data(item,AbstractPackageController::Domains).toString()) )
            options = PackageDetails::None;
#endif

    }

    qLog(Package) << "show details" << ( name.isNull() ? "no valid name" : name );

    PackageDetails *pd = new PackageDetails( this, name, text, type, options );
    pd->showMaximized();
    int result = pd->exec();
    delete pd;

    //see if the user wants to proceed to the next stage
    if ( result == PackageDetails::Proceed )
    {
        if ( type == PackageDetails::Confirm )
            model->activateItem( item ); //for a confirm dialog this means installing
        else if ( type == PackageDetails::Info && (options & PackageDetails::Install) )
            showDetails( item,  PackageDetails::Confirm ); //for an  (install) info dialog this means proceeding to confirm installation
        else if ( type == PackageDetails::Info && (options & PackageDetails::Uninstall) )
            startUninstall(); //for an (uninstall) info dialog this means starting to uninstall (which implicitly asks for confirmation)
    }
}

void PackageView::startInstall()
{
    const char *dummyStr = QT_TRANSLATE_NOOP( "PackageView", "Installing..." );
    Q_UNUSED( dummyStr );

    showDetails( downloadView->currentIndex(), PackageDetails::Confirm );
}

void PackageView::startUninstall()
{
    if ( QMessageBox::warning(this, tr("Confirm Uninstall?"), tr("Are you sure you wish to uninstall %1?","%1=package name")
                .arg( model->data( installedView->currentIndex(), Qt::DisplayRole ).toString()) + QLatin1String("<BR>")
                + tr("All running instances will be terminated."), QMessageBox::Yes | QMessageBox::No )
         == QMessageBox::Yes)
    {
            waitWidget->show();
            waitWidget->setText(tr("Uninstalling..."));
            model->activateItem( installedView->currentIndex() );
            installedView->setCurrentIndex(QModelIndex());
            waitWidget->hide();
    }
}

void PackageView::displayDetails()
{
    if ( tabWidget->currentIndex() == InstalledIndex )
        showDetails( installedView->currentIndex(), PackageDetails::Info );
    else
        showDetails( downloadView->currentIndex(), PackageDetails::Info );
}

void PackageView::confirmReenable()
{
    if ( QMessageBox::warning(this, tr("Confirm Re-enable?"), tr("Are you sure you wish to re-enable %1?","%1=package name")
            .arg( model->data( installedView->currentIndex(), Qt::DisplayRole ).toString()), QMessageBox::Yes | QMessageBox::No )
         == QMessageBox::Yes)
    {
        model->reenableItem( installedView->currentIndex() );
    }
}

/*!
    \internal
    Slot to be called whenever the context menu is shown.
    Sets the appropriate options available in the context menu.
*/
void PackageView::contextMenuShow()
{
    QModelIndex currIndex = ( tabWidget->currentIndex() == InstalledIndex ) ? (installedView->currentIndex())
                                                               : (downloadView->currentIndex());
    //if the current model index is not valid
    //switch off options in the context menu as appropriate
    if( !currIndex.isValid() || !currIndex.parent().isValid()
        || !model->hasIndex(currIndex.row(), currIndex.column(), currIndex.parent()) )
    {
        detailsAction->setVisible( false );

        if ( tabWidget->currentIndex() == InstalledIndex )
        {
            uninstallAction->setVisible( false );
            reenableAction->setVisible( false );
        }
        else //otherwise dealing with Downloads tab
        {
            installAction->setVisible( false );
        }
    }
    else //make available the appropriate context menu options.
    {
        detailsAction->setVisible( true );
        if (tabWidget->currentIndex() == InstalledIndex) //Installed tab
        {
            uninstallAction->setVisible( true );
            if ( model->data( currIndex, Qt::StatusTipRole ).toBool() ) //if package is enabled
                reenableAction->setVisible( false );
            else
                reenableAction->setVisible( true );
        }
        else //Downloads tab
        {
            if (model->isInstalled(currIndex))
                installAction->setVisible(false);
            else
                installAction->setVisible(true);
        }
    }

    if (tabWidget->currentIndex() == DownloadIndex) //Download tab
    {
        if (menuServers->isEmpty() )
            menuServers->setEnabled(false);
        else
            menuServers->setEnabled(true);
    }
}

/*!
  \internal
  Displays the status of the server
*/
void PackageView::postServerStatus( const QString &status )
{
    statusLabel->setText( status );
}

/*!
  \internal
  Show a newly installed package as the currently selected package.
*/
void PackageView::selectNewlyInstalled( const QModelIndex &index)
{
    tabWidget->setCurrentIndex( PackageView::InstalledIndex );
    installedView->setCurrentIndex( index );
    QMessageBox::information( this, tr("Successful Installation"), tr("Package successfully installed" ) );
}
