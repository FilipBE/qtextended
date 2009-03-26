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

#ifndef PACKAGECONTROLLER_H
#define PACKAGECONTROLLER_H

#include <QString>
#include <QIcon>
#include <QList>
#include <QModelIndex>
#include <QSize>
#include <QMap>
#include <QSignalMapper>
#include "installcontrol.h"
#include "domaininfo.h"

class QProgressDialog;
class HttpFetcher;
class InstalledPackageScanner;
class PackageModel;

/**
  \class AbstractPackageController
    \inpublicgroup QtPkgManagementModule
  \internal
  Abstract base class for package controllers
*/
class AbstractPackageController : public QObject
{
    Q_OBJECT
public:
    enum PCType {
        local,
        network,
        installed
    };

    enum Role {
        Domains = Qt::UserRole + 1, 
        Md5Sum = Qt::UserRole + 2,
        Filtered = Qt::UserRole + 3
    };

    static const QString INFORMATION_FILE;
    static const QString PACKAGE_SUMMARY_FILE;
    static const QString INSTALLED_INFO_FILES_LOC;
    static AbstractPackageController *factory( PCType t, PackageModel *parent = 0 );

    int numberPackages() const;
    InstallControl::PackageInfo packageInfo( int pkgId ) const;
    QList<InstallControl::PackageInfo> packageList() const { return pkgList; }

    virtual QVariant data( int ) const;
    virtual QVariant data( int, int, int ) const;

    QIcon dataIcon( int pkgId = -1 ) const { return getDataIcon( pkgId ); }
    virtual QString controllerName() const = 0;
    virtual QString controllerDescription() const = 0;
    virtual QIcon controllerIcon() const = 0;
    virtual QString packageDetails( int pkgId ) const;
    virtual QString operationName() const;
    virtual void install( int pkgId ) = 0;
    virtual void setPackageFilter( const QList<InstallControl::PackageInfo> &filter );
public slots:
    virtual void addPackage( const InstallControl::PackageInfo & );
    virtual void clearPackages();
signals:
    void packageInstalled( const InstallControl::PackageInfo & );
    void rowsAboutToBeRemoved( const QModelIndex &, int, int );
    void rowsRemoved( const QModelIndex &, int, int );
    void updated();
protected:
    AbstractPackageController( QObject *parent = 0 );
    virtual ~AbstractPackageController();

    QList<InstallControl::PackageInfo> pkgList;
    QList<InstallControl::PackageInfo> filteredOutPkgList;
    QProgressDialog *progressDisplay;
    InstallControl *installControl;

private:
    virtual QIcon getDataIcon( int pkgId ) const = 0;
};

class PackageInformationReader;

#define PKG_WIZ_HTTP_INFO_FILE "packages.list"

/**
  \class LocalPackageController
    \inpublicgroup QtPkgManagementModule
  \internal
  Implement package controller for case of local packages, already
  available via the device filesystem
*/
class LocalPackageController : public AbstractPackageController
{
    Q_OBJECT
public:
    virtual QString controllerName() const;
    virtual QIcon controllerIcon() const;
    virtual QString controllerDescription() const;
    virtual QString packageDetails( int pkgId ) const;
    virtual void install( int pkgId );
protected:
    LocalPackageController( QObject *parent = 0 );
    ~LocalPackageController();
private slots:
    void packageComplete();
private:
    virtual QIcon getDataIcon( int pkgId ) const;

    static const QString LOCAL_PACKAGE_DIRECTORY;

    QString currentPackageDirectory;
    QMap<InstallControl::PackageInfo, QString> pkgLoc;

    void initialiseLocalPackageInfo();

    PackageInformationReader *reader;
    friend class AbstractPackageController;
};

/**
  \class NetworkPackageController
    \inpublicgroup QtPkgManagementModule
  \internal
  Implement package controller for case of packages available via the
  http network and which must be downloaded before installation
*/
class NetworkPackageController : public AbstractPackageController
{
    Q_OBJECT
public:
    virtual QString controllerName() const;
    virtual QIcon controllerIcon() const;
    virtual QString controllerDescription() const;
    virtual void install(int packageI);
    virtual QString packageDetails( int pkgId ) const;

    QString& networkServer();
    void setNetworkServer( const QString & );

signals:
    void serverStatus( const QString & );

protected:
    NetworkPackageController( QObject *parent = 0 );
    ~NetworkPackageController();

private slots:
    void packageFetchComplete();
    void insertNetworkPackageItems();
    void listFetchComplete();

private:
    virtual QIcon getDataIcon( int pkgId ) const;

    QString currentNetworkServer;
    QSignalMapper *signalMapper;
    HttpFetcher *hf;
    friend class AbstractPackageController;
    QString currentPackageName;
};

class QFileSystem;

/**
  \class InstalledPackageController
    \inpublicgroup QtPkgManagementModule
  \internal
  Implement package controller for packages already installed
*/
class InstalledPackageController : public AbstractPackageController
{
    Q_OBJECT
public:
    virtual QString controllerName() const;
    virtual QIcon controllerIcon() const;
    virtual QString controllerDescription() const;
    virtual QString operationName() const;
    virtual void install(int packageI);
    virtual QVariant data( int ) const;
    virtual QVariant data( int, int, int ) const;
    virtual QString packageDetails( int pkgId ) const;

    bool reenable( int pkgId );

    static const QString DISABLED_TAG;
public slots:
    void reloadInstalledLocations( const QStringList & );
protected:
    InstalledPackageController( QObject *parent = 0 );
    ~InstalledPackageController();
private slots:
    void initialize();
private:
    virtual QIcon getDataIcon( int pkgId ) const;
    friend class AbstractPackageController;
    InstalledPackageScanner *installedScanner;
};

#include <QDialog>

class QProgressBar;
class QTextBrowser;
class QPushButton;
class QVBoxLayout;

class QProgressDialog : public QDialog
{
    Q_OBJECT
public:
    QProgressDialog( QWidget *p = 0, Qt::WFlags f = 0 );
    ~QProgressDialog();
    void setLabelText( const QString & );
    void setMaximum( int );
    virtual QSize sizeHint() const;
signals:
    void canceled();
public slots:
    void setValue( int );
    void reset();
private slots:
    void init();
private:
    QProgressBar *bar;
    QTextBrowser *label;
    QPushButton *cancelButton;
    QVBoxLayout *vb;
};

////////////////////////////////////////////////////////////////////////
/////
/////  AbstractPackageController inline method implementations
/////

inline int AbstractPackageController::numberPackages() const
{
    return pkgList.count();
}

inline InstallControl::PackageInfo AbstractPackageController::packageInfo( int pkgId ) const
{
    return pkgList[ pkgId ];
}

inline QVariant AbstractPackageController::data( int role ) const
{
    if ( role == Qt::DisplayRole )
        return controllerName();
    if ( role == Qt::DecorationRole )
        return controllerIcon();
    if ( role == Qt::WhatsThisRole )
        return controllerDescription();
    return QVariant();
}

inline QVariant AbstractPackageController::data( int row, int column, int role ) const
{
    InstallControl::PackageInfo pi = packageInfo( row );
    if ( role == Qt::DisplayRole )
        return ( column == 0 ) ? pi.name : pi.description;
    if ( role == Qt::DecorationRole )
        return dataIcon( row );
    if ( role == Qt::WhatsThisRole )//package metadata is returned
        return packageDetails(row); //in string format
    if ( role == Qt::StatusTipRole )//is packge enabled
        return true; //always true for general case
    if ( role == AbstractPackageController::Domains )
        return pi.domain;
    if ( role == AbstractPackageController::Md5Sum )
        return pi.md5Sum;
    if ( role == AbstractPackageController::Filtered )
        return QVariant(filteredOutPkgList.contains( pi ));
    return QVariant();
}

inline void AbstractPackageController::addPackage( const InstallControl::PackageInfo &pkg )
{
    pkgList.append( pkg );
    qSort( pkgList );
    emit updated();
}

inline void AbstractPackageController::clearPackages()
{
    pkgList.clear();
    emit updated();
}

inline QString AbstractPackageController::operationName() const
{
    return tr( "Install" );
}

////////////////////////////////////////////////////////////////////////
/////
/////  LocalPackageController inline method implementations
/////

inline QIcon LocalPackageController::getDataIcon( int pkgId ) const
{
    Q_UNUSED( pkgId );
    return QIcon( ":icon/uninstalled" );
}

inline QString LocalPackageController::controllerName() const
{
    return tr( "Available locally" );
}

inline QIcon LocalPackageController::controllerIcon() const
{
    return QIcon( ":icon/folder" );
}

inline QString LocalPackageController::controllerDescription() const
{
    return tr( "Packages already downloaded available for installation" );
}

inline QString LocalPackageController::packageDetails( int pkgId ) const
{
    return pkgList[ pkgId ].domain;
}

////////////////////////////////////////////////////////////////////////
/////
/////  NetworkPackageController inline method implementations
/////

inline QIcon NetworkPackageController::getDataIcon( int pkgId ) const
{
    Q_UNUSED( pkgId );
    return QIcon( ":icon/uninstalled" );
}

inline QString NetworkPackageController::controllerName() const
{
    return tr( "Downloads available" );
}

inline QIcon NetworkPackageController::controllerIcon() const
{
    return QIcon( ":icon/irreceive" );
}

inline QString& NetworkPackageController::networkServer()
{
    return currentNetworkServer;
}

inline QString NetworkPackageController::controllerDescription() const
{
    return tr( "Network packages for download from servers:<br/>%1" )
        .arg( currentNetworkServer );
}

////////////////////////////////////////////////////////////////////////
/////
/////  InstalledPackageController inline method implementations
/////

inline QString InstalledPackageController::controllerName() const
{
    return tr( "Installed" );
}

inline QIcon InstalledPackageController::controllerIcon() const
{
    return QIcon( ":icon/install" );
}

inline QString InstalledPackageController::controllerDescription() const
{
    return tr( "Currently installed packages which may be removed to regain space" );
}

inline QString InstalledPackageController::operationName() const
{
    return tr( "Uninstall" );
}

inline QVariant InstalledPackageController::data( int role ) const
{
    return AbstractPackageController::data( role ); 
}

inline QVariant InstalledPackageController::data( int row, int column, int role ) const
{
    if ( role == Qt::StatusTipRole )
        return packageInfo( row ).isEnabled;
    else
        return AbstractPackageController::data( row, column, role );
}

#endif
