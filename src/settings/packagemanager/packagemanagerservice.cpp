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

#include "packagemanagerservice.h"
#include "packageinformationreader.h"
#include <qtopianamespace.h>
#include <QtopiaApplication>
#include "ui_packagedetails.h"
#include "packageview.h"
#include "packagemodel.h"
#include "packagecontroller.h" 
#include "domaininfo.h"
#include <QUrl>
#include <QDSData>
#include <QDir>
#include <QScrollArea>
#include "installedpackagescanner.h"

/*!
    \service PackageManagerService PackageManager
    \inpublicgroup QtPkgManagementModule
    \brief The PackageManagerService class provides the Package Manager service.

    The \i{Package Manager} service enables package installations to be triggered remotely.

    A package install can be started by invoking the service message \c{installPackageConfirm(QString)} with
    URL of the descriptor of the package to install.  Package manager will then download the descriptor and
    after gaining the users consent download the package and install it.


    \sa QtopiaAbstractService
*/
PackageManagerService::PackageManagerService( PackageView *parent )
    : QtopiaAbstractService( QLatin1String( "PackageManager" ), parent )
    , m_installer( 0 )
    , m_packageView( parent )
{
    publishAll();
}

/*!
    Download a package descriptor from the URL \a url and after obtaining the users consent download
    and install the package it refers to.

    This slot corresponds to the QCop message \c{PackageManager::installPackageConfirm(QString)}.
*/
void PackageManagerService::installPackageConfirm( const QString &url )
{
    if( !m_installer )
    {
        m_installer = new PackageServiceInstaller( m_packageView );

        connect( m_installer, SIGNAL(finished(int)), this, SLOT(installFinished()) );

        m_installer->installPackage( url );
    }
    else if( !m_installer->installActive() )
    {
        m_installer->installPackage( url );
    }
    else
        m_pendingUrls.append( url );
}

/*!
    Initiate the package installation process using the package descriptor embedded within
    \a request.

    This slot corresponds to the QCop message \c{PackageManager::installPackage(QDSActionRequest)}.
*/
void PackageManagerService::installPackage(const QDSActionRequest& request)
{
    QDSActionRequest requestCopy( request );

    if( !m_installer )
    {
        m_installer = new PackageServiceInstaller( m_packageView );

        connect( m_installer, SIGNAL(finished(int)), this, SLOT(installFinished()) );

        m_installer->installPackage( requestCopy.requestData().data() );
    }
    else if( !m_installer->installActive() )
    {
        m_installer->installPackage( requestCopy.requestData().data() );
    }
    else
        m_pendingDescriptors.append( requestCopy.requestData().data() );

    requestCopy.respond();
}

void PackageManagerService::installFinished()
{
    if( !m_pendingUrls.isEmpty() )
        m_installer->installPackage( m_pendingUrls.takeFirst() );
    else if( !m_pendingDescriptors.isEmpty() )
        m_installer->installPackage( m_pendingDescriptors.takeFirst() );
}

class ServicePackageDetails : public QDialog, public Ui::PackageDetails
{
public:
    ServicePackageDetails( QWidget *parent )
        : QDialog( parent )
    {
        setupUi(this);
    }
};


PackageServiceInstaller::PackageServiceInstaller( PackageView *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
    , m_scanner( 0 )
    , m_installActive( false )
    , m_packageView( parent )
    , m_expectedPackageSize( 0 )
    , m_maxDescriptorSize( 4096 )
    , currentUrl( new QUrl() ) 
{
    QVBoxLayout *progressLayout = new QVBoxLayout( this );

    progressLayout->addWidget( m_progressTextEdit = new QTextEdit( this ) );
    m_progressTextEdit->setReadOnly( true );
    m_progressTextEdit->setFocusPolicy( Qt::NoFocus );
    m_progressTextEdit->setTextInteractionFlags( Qt::NoTextInteraction );
    m_progressTextEdit->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    progressLayout->addWidget( m_progressBar = new QProgressBar( this ) );

    m_packageDetails = new ServicePackageDetails( this );

    QtopiaApplication::setMenuLike( m_packageDetails, true );

    QSettings pkgManagerConf( "Trolltech", "PackageManager" );

    pkgManagerConf.beginGroup( QLatin1String( "Configuration" ));
    m_maxDescriptorSize = pkgManagerConf.value( QLatin1String( "MaxDescriptorSize" )).toInt();

    pkgManagerConf.endGroup();
}

void PackageServiceInstaller::doReportError( const QString &simpleError, const QString &detailedError )
{
    m_progressTextEdit->setText( simpleError );
    qWarning( qPrintable(simpleError + QLatin1String("\n") + detailedError) );
}

void PackageServiceInstaller::confirmInstall( const InstallControl::PackageInfo &package )
{
    QString errReason;
    if ( package.url.isEmpty() )
    {
        errReason = "URL field is empty/non-existent"; 
        reportError( tr("Package descriptor Incomplete: %1", "%1 = reason" ).arg( errReason ),
                       QLatin1String( "PackageServiceInstaller::confirmInstall qpd incomplete: ")
                            + errReason );
        return;
    }
    else if ( !package.isComplete(InstallControl::PackageInfo::PkgList, &errReason ) ) 
    {
        reportError( tr("Package descriptor Incomplete: %1", "%1 = reason" ).arg( errReason ), 
                       QLatin1String( "PackageServiceInstaller::confirmInstall qpd incomplete: " )
                            + errReason );
        return;
    }

    bool toIntOk = true;

    m_expectedPackageSize = package.downloadSize.toInt( &toIntOk );

    if( !toIntOk )
    {
        m_progressTextEdit->setText( tr( "Cannot determine package size." ) );

        return;
    }

    m_pendingPackage =  package;

    m_packageDetails->setWindowTitle( m_pendingPackage.name );

#ifndef QT_NO_SXE
    if( DomainInfo::hasSensitiveDomains( m_pendingPackage.domain ) )
    {
        QMessageBox::warning(this, tr("Install Error"), tr("%1 utilizes protected resources")
            .arg( m_pendingPackage.name ) );
    }
    else
#endif
    if ( InstalledPackageScanner::isPackageInstalled(m_pendingPackage) )
    {
        QMessageBox::warning(this, tr("Install Error"),
                            tr("%1 has already been installed").arg(m_pendingPackage.name) );
    }
    else
    {
        if ( QMessageBox::warning(this, tr("Installing Package"), tr("%1?","%1=package name")
#ifndef QT_NO_SXE
            .arg( DomainInfo::explain( m_pendingPackage.domain, m_pendingPackage.name ) ),
#else
            .arg( m_pendingPackage.name), 
#endif
             QMessageBox::Yes | QMessageBox::No )
         == QMessageBox::Yes) 
        {
        installPendingPackage();
        }
    }

}

void PackageServiceInstaller::installPendingPackage()
{
    m_packageFile.setFileName( InstallControl::downloadedPkgPath() );

    m_packageFile.unsetError();

    if( m_packageFile.open( QIODevice::WriteOnly ) )
    {
        connect( &m_http, SIGNAL(done(bool)), this, SLOT(packageDownloadDone(bool)) );
        connect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updatePackageProgress(int,int)) );

        currentUrl->setUrl( m_pendingPackage.url );

        m_http.setHost( currentUrl->host(), currentUrl->port(80) );
        m_http.get( currentUrl->path(), &m_packageFile );

        m_progressTextEdit->setText( tr( "Downloading %1...", "%1 = package name" ).arg( m_pendingPackage.name ) );
    }
    else
    {
        m_progressTextEdit->setText(
                tr( "Package download failed due to file error: %1", "%1 = file error description" )
                .arg( m_packageFile.errorString() ) );

        QtopiaApplication::setMenuLike( this, false );
    }
}

void PackageServiceInstaller::installPackage( const QString &url )
{
    m_installActive = true;

    m_progressTextEdit->setText( tr( "Downloading package header..." ) );

    QtopiaApplication::setMenuLike( this, true );

    showMaximized();

    connect( &m_http, SIGNAL(done(bool)), this, SLOT(headerDownloadDone(bool)) );
    connect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updateHeaderProgress(int,int)) );

    m_headerBuffer.open( QIODevice::ReadWrite );

    currentUrl->setUrl( url );

    m_http.setHost( currentUrl->host(), currentUrl->port(80) );
    m_http.get( currentUrl->path(), &m_headerBuffer );
}

void PackageServiceInstaller::installPackage( const QByteArray &descriptor )
{
    QTextStream stream( descriptor );

    PackageInformationReader reader( stream );

    showMaximized();

    confirmInstall( reader.package() );
}

void PackageServiceInstaller::headerDownloadDone( bool error )
{
    disconnect( &m_http, SIGNAL(done(bool)), this, SLOT(headerDownloadDone(bool)) );
    disconnect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updateHeaderProgress(int,int)) );

    QHttpResponseHeader responseHeader = m_http.lastResponse();
    if( responseHeader.statusCode() >= 400 || error )
    {
        m_headerBuffer.close();
        m_headerBuffer.setData( QByteArray() );

        QString simpleError;
        QString detailedError;
        if ( error && m_http.error() != QHttp::Aborted )
        {
            simpleError =  tr( "Header (qpd descriptor) download failed for URL-%1 : %2", "%1 = URL, %2 = reason" )
                            .arg( currentUrl->toString() )
                            .arg( m_http.errorString() );
            detailedError = QLatin1String( "PackageServiceInstaller::headerDownloadDone: URL = %1, http error = %2" );
            detailedError = detailedError.arg( currentUrl->toString() ).arg( m_http.errorString() );
            reportError( simpleError, detailedError );
        }
        else if ( responseHeader.statusCode() >= 400 )
        { 

            simpleError = tr( "Header(qpd) download failed: Error connecting to URL-%1 : %2, status code = %3",
                                        "%1 = URL, %2 = reason, %3 = status code  " )
                                    .arg( currentUrl->toString() )
                                    .arg( responseHeader.reasonPhrase() )
                                    .arg( QString::number(responseHeader.statusCode()) );
            detailedError = QString( "PackageServiceInstaller::headerDownloadDone: statusCode > 400 "
                                       "URL = %1, reason = %2, status code = %3" )
                                    .arg( currentUrl->toString() )
                                    .arg( responseHeader.reasonPhrase() )
                                .arg( QString::number(responseHeader.statusCode()) );
            reportError( simpleError, detailedError );
        }
        
        QtopiaApplication::setMenuLike( this, false );
        return;
    }

    m_progressBar->setValue( m_progressBar->maximum() );

    m_headerBuffer.seek( 0 );

    QTextStream stream( &m_headerBuffer );

    PackageInformationReader reader( stream );

    m_headerBuffer.buffer().clear();
    confirmInstall( reader.package() );
}

void PackageServiceInstaller::packageDownloadDone( bool error )
{
    disconnect( &m_http, SIGNAL(done(bool)), this, SLOT(packageDownloadDone(bool)) );
    disconnect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updatePackageProgress(int,int)) );

    m_packageFile.close();

    m_progressBar->setValue( m_progressBar->maximum() );

    QHttpResponseHeader responseHeader = m_http.lastResponse();

    if( responseHeader.statusCode() >= 400 ) 
    {
        QString simpleError = tr( "Package download failed: Error connecting to URL-%1 : %2, status code = %3", 
                                    "%1 = URL, %2 = reason, %3 = status code  " )
                                .arg( currentUrl->toString() )
                                .arg( responseHeader.reasonPhrase() )
                                .arg( QString::number(responseHeader.statusCode()) );
        QString detailedError = QString( "PackageServiceInstaller::packageDownloadDone: statusCode > 400 "
                                       "URL = %1, reason = %2, status code = %3" )
                                .arg( currentUrl->toString() )
                                .arg( responseHeader.reasonPhrase() )
                                .arg( QString::number(responseHeader.statusCode()) );
        reportError( simpleError, detailedError );
    }
    else if ( error && m_http.error() != QHttp::Aborted )
    {
        QString simpleError =  tr( "Package download failed for URL-%1 : %2" )
                                    .arg( currentUrl->toString() )
                                    .arg( m_http.errorString() );
        QString detailedError( "PackageServiceInstaller::packageDownloadDone: URL = %1, http error = %2" );
        detailedError = detailedError.arg( currentUrl->toString() ).arg( m_http.errorString() );
        reportError( simpleError, detailedError );
    }
    else
    {
        if( m_installer.installPackage( m_pendingPackage, m_packageFile.md5Sum(), this ) )
        {
            m_progressTextEdit->setText( tr( "%1 installed", "%1 = package name" ).arg( m_pendingPackage.name ) );
            qobject_cast<InstalledPackageController *>(m_packageView->model->installed )
                ->reloadInstalledLocations( QStringList( Qtopia::packagePath() + "controls/" ) );
        }
        else
        {
            m_installer.uninstallPackage( m_pendingPackage, this );
        }
    }

    m_packageFile.remove();

    QtopiaApplication::setMenuLike( this, false );
}

void PackageServiceInstaller::updateHeaderProgress( int done, int total )
{
    if( m_progressBar->maximum() != total )
        m_progressBar->setMaximum( total );

    m_progressBar->setValue( done );

    if( total > m_maxDescriptorSize )
    {
        m_progressTextEdit->setText(
                tr( "Package header exceeds maximum size of %1 bytes. Download cancelled" ).arg( m_maxDescriptorSize ) );

        m_http.abort();
    }
}

void PackageServiceInstaller::updatePackageProgress( int done, int total )
{
    if( m_progressBar->maximum() != total )
        m_progressBar->setMaximum( total );

    m_progressBar->setValue( done );

    if( total > m_expectedPackageSize )
    {
        m_progressTextEdit->setText(
                tr( "Package size exceeds the expected size of %1 KB. Download cancelled" ).arg( m_expectedPackageSize / 1024 ) );

        m_http.abort();
    }
    else if( m_packageFile.error() != QFile::NoError )
    {
        m_progressTextEdit->setText(
                tr( "Package download failed due to file error: %1", "%1 = file error description" ).arg( m_packageFile.errorString() ) );

        m_http.abort();
    }
}

void PackageServiceInstaller::accept()
{
    m_installActive = false;

    QDialog::accept();
}

void PackageServiceInstaller::reject()
{
    m_installActive = false;

    m_http.abort();

    QDialog::reject();
}

