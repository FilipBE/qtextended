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

#include "packagescanner.h"
#include "deviceconnector.h"

#include <qdebug.h>

#include <QMessageBox>
#include <QIcon>

/*!
  Construct a new \c PackageScanner object, which will scan the \a dir
  directory, when the refresh() call is made.
*/
PackageScanner::PackageScanner( const QString &dir, QObject *parent )
    : QAbstractListModel( parent )
    , mDir( dir )
    , mScanner( 0 )
    , mConnector( 0 )
{
    refresh();
    mConnector = new DeviceConnector();
    QObject::connect( mConnector, SIGNAL(finishedConnect()),
            this, SLOT(connectorComplete()) );
    QObject::connect( mConnector, SIGNAL(deviceConnMessage(QString)),
            this, SIGNAL(progressMessage(QString)) );
    QObject::connect( mConnector, SIGNAL(sendProgress(int)),
            this, SIGNAL(progressValue(int)) );
}

/*!
  Destroy this \c PackageScanner
*/
PackageScanner::~PackageScanner()
{
}

void PackageScanner::connectorComplete()
{
}

/*!
  \reimp
*/
int PackageScanner::rowCount( const QModelIndex &parent ) const
{
    if ( !parent.isValid() ) // top level
        return mPackageList.count();
    return 0;
}

/*!
  \reimp
*/
Qt::ItemFlags PackageScanner::flags( const QModelIndex & index ) const
{
    if ( !index.isValid() || index.row() >= mPackageList.count() )
        return static_cast<Qt::ItemFlag>( 0 );
    QMutexLocker ml( &mPackageListMutex );
    PackageItem *pkg = mPackageList[index.row()];
    if ( !pkg->hasBeenUploaded )
        return Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    return static_cast<Qt::ItemFlag>( 0 );
 }

/*!
  \reimp
*/
QVariant PackageScanner::data( const QModelIndex &index, int role ) const
{
    QVariant retVal;
    if ( !index.isValid() || index.row() >= mPackageList.count() )
        return retVal;
    QMutexLocker ml( &mPackageListMutex );
    PackageItem *pkg = mPackageList[index.row()];
    if ( role == Qt::DisplayRole )
        return pkg->display;
    if ( pkg->hasBeenUploaded )
    {
        // QPalette pal = QApplication::palette();
        switch ( role )
        {
            case Qt::DecorationRole:
                retVal = QIcon( "packages_yellow" );
                break;
                /*
            case Qt::ForegroundRole:
                retVal = pal.brush( QPalette::Disabled, QPalette::Foreground );
            case Qt::BackgroundRole:
                retVal = pal.brush( QPalette::Disabled, QPalette::Background );
                */
            case Qt::ToolTipRole:
                retVal = tr( "Already uploaded.  Click refresh to find any updates." );
                break;
            default:
                ; // fall thru with empty variant
        }
    }
    else
    {
        switch ( role )
        {
            case Qt::DecorationRole:
                retVal = QIcon( "packages_green" );
                break;
            case Qt::ToolTipRole:
                retVal = tr( "Check this package and click send to push to device." );
                break;
            default:
                ; // fall thru with empty variant
        }
    }
    return retVal;
}

/*!
  Locate the package identified by the name, or part of name \a nameStr and send
  a request to the device to initiate fetching and installing of the package.
*/
void PackageScanner::sendPackage( const QString &nameStr )
{
    PackageItem *pkg = findPackageByName( nameStr );
    if ( pkg == NULL )
    {
        QString warning = tr( "Could not find a package: %1" ).arg( nameStr );
        QMessageBox::warning( 0, tr("No Such Packages"), warning );
        return;
    }
    sendIt( pkg );
}

/*!
  Locate the package identified by the \c QModelIndex \a ix and send a request
  to the device to initiate fetching and installing of the package.
*/
void PackageScanner::sendPackage( const QModelIndex &ix )
{
    if ( !ix.isValid() )
        return;
    PackageItem *pkg = mPackageList[ix.row()];
    sendIt( pkg );
}

/*!
  Do the actual sending via the connector.
*/
void PackageScanner::sendIt( PackageItem *pkg )
{
    if ( pkg->qpdPath.isEmpty() )
    {
        // TODO: implement the mkPackages shell script "buildpkgs()" function
        // and do this automatically instead of asking the user to do it
        QString warning = tr( "Create descriptors using bin/mkPackages before sending packages" );
        QMessageBox::warning( 0, tr("Missing Descriptors"), warning );
        return;
    }

    // TODO:  make this configurable
    QString serverName = "10.10.10.21:8080";
    QString url = QString( "http://%1/%2" ).arg( serverName ).arg( pkg->qpdPath );

    QByteArray fileNameData;
    {
        QDataStream ds( &fileNameData, QIODevice::WriteOnly );
        ds << url;
    }
    mConnector->sendQcop( "QPE/Application/packagemanager", "PackageManager::installPackageConfirm(QString)", fileNameData );
}

/*!
  Search through the list of packages for one matching \a name.

  Internally the stored base names are checked to see if they contain the
  substring \a name, so "calc" would find "calculator_1.0.0_arm".

  If an exact match is required name should specify the full version and
  architecture eg "calculator_1.0.0_arm".
*/
PackageItem *PackageScanner::findPackageByName( const QString &name )
{
    QMutexLocker ml( &mPackageListMutex );
    foreach ( PackageItem *pkg, mPackageList )
        if ( pkg->name.indexOf( name ) != -1 )
            return pkg;
    return 0;
}

void PackageScanner::refresh()
{
    emit progressMessage( tr( "Scanning for packages..." ));
    emit progressValue( 0 );
    mScanner = new ScannerThread( this );
    connect( mScanner, SIGNAL(finished()),
            this, SLOT(scannerDone()) );
    connect( mScanner, SIGNAL(terminated()),
            this, SLOT(scannerDone()) );
    mScanner->start();
}

void PackageScanner::scannerDone()
{
    emit progressValue( DEFAULT_PROGRESS_MAX );
    emit progressMessage( tr( "Found %1 packages" ).arg( mPackageList.count() ));
    emit updated();
    delete mScanner;
    mScanner = 0;
    reset();
}

void PackageScanner::appendPackage( PackageItem *package )
{
    mPackageListMutex.lock();
    mPackageList.append( package );
    mPackageListMutex.unlock();
}
