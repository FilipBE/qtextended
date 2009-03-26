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

#include "installcontrol.h"
#include "packagecontroller.h"
#include "sandboxinstall.h"
#include "packageversion.h"
#ifndef QT_NO_SXE
#include <qtransportauth_qws.h>
#include "qpackageregistry.h"
#endif
#include "targz.h"
#include "utils.h"

#include <QDir>
#include <QDebug>
#include <QProcess>

#include "packageinformationreader.h"

#include <qcontent.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qstorage.h>
#include <qtopiaipcadaptor.h>

#include <errno.h>
#include <unistd.h>
#include <QMessageBox>
#include <qtopiabase/version.h>

SimpleErrorReporter::SimpleErrorReporter( ReporterType type, const QString &pkgName )
{
    packageName = pkgName;
    switch ( type )
    {
        case ( Install ):
            subject = QObject::tr( "Install Error" );
            prefix = QObject::tr("%1:", "%1 = package name")
                 .arg( packageName ) + QLatin1String( " " );
            break;
        case ( Uninstall ):
            subject = QObject::tr( "Uninstall Error" );
            prefix = QObject::tr("%1:", "%1 = package name")
                    .arg( packageName ) + QLatin1String(" ");
            break;
        case ( Other ):
            subject = QObject::tr( "Warning" );
            prefix = "";
            break;
        default:
            qWarning() << "SimpleErrorReporter constructor invalid ReporterType parameter";
    }
}

void SimpleErrorReporter::doReportError( const QString &simpleError, const QString &detailedError )
{
    QString userVisibleError = prefix + simpleError;
    QString logError = userVisibleError + "\n" + detailedError;

    QMessageBox::warning( 0, subject, userVisibleError );
    qWarning() << qPrintable(logError);
}

InstallControl::InstallControl()
{
//Need to set keyfile path for registering binaries
#ifndef QT_NO_SXE
    QString confPath = QPackageRegistry::getInstance()->sxeConfPath();
    QTransportAuth *a = QTransportAuth::getInstance();
    a->setKeyFilePath( confPath );
#endif
}

InstallControl::~InstallControl()
{
}

/* Returns the path of where the package was downloaded to.
   (The entire file path is returned not just the directory)
*/
QString InstallControl::downloadedPkgPath()
{
    static QString path;
    if (path.isEmpty()) {
        path = Qtopia::packagePath() + "/tmp";
        QDir::root().mkdir(Qtopia::packagePath() + "/tmp");
        path += "/package";
    }
    return path;
}

/*!
  Install the package \a pkg onto the device.

  The package is installed into the Qtopia::packagePath() directory
  which is an internal writable location.

  If the Qtopia::packagePath() directory is not on the file-system of
  the media chosen by setInstallMedia() then after the install to the
  media file-system, links are created in the Qtopia::packagePath()
  so that the Qt Extended launching and resource systems will work.

  If the package is a system package then is then re-installed under the
  standard Qt Extended directory tree, under the install destination, after the
  system package verification has been done.  This includes certificate
  validation and other checks.
*/
bool InstallControl::installPackage( const InstallControl::PackageInfo &pkg, const QString &md5Sum, ErrorReporter *reporter ) const
{
    QString packageFile = downloadedPkgPath();

    if (  QFile( packageFile).size() != pkg.downloadSize.toLongLong() )
    {
        if ( reporter )
        {
            reporter->reportError( QObject::tr("Downloaded package size does not match advertised package size" ),
                            QString( "InstallControl:installPackage:- Downloaded package size does not match "
                                     "advertised package size, downloaded size=%1, advertised size=%2" )
                                .arg(QFile(packageFile).size() ).arg( pkg.downloadSize ) );
        }
        return false;
    }

    if( pkg.md5Sum != md5Sum || md5Sum.length() != 32 )
    {
        qLog(Package) << "MD5 Sum mismatch! Header MD5:" << pkg.md5Sum << "Package MD5:" << md5Sum;

        QString simpleError = QObject::tr( "Invalid package or package was corrupted. "
                                            "Re-download or contact package supplier");
        QString detailedError = QObject::tr( "InstallControl::installPackage:- MD5 Sum mismatch, "
                                                    "Descriptor MD5: %1, Package MD5: %2")
                                .arg( pkg.md5Sum ).arg( md5Sum ); 
        if( reporter )
            reporter->reportError( simpleError, detailedError );

        return false;
    }

    SandboxInstallJob job( &pkg, m_installMedia, reporter );
    if ( job.isAborted() )
        return false;

#ifndef QT_NO_SXE
    QObject::connect( &job, SIGNAL(newBinary(SxeProgramInfo&)),
            QPackageRegistry::getInstance(), SLOT(registerBinary(SxeProgramInfo&)) );
#endif

    QString dataTarGz = job.destinationPath() + "/data.tar.gz";

    // install to directory
    qulonglong expectedSize = SizeUtils::parseInstalledSize( pkg.installedSize );
    qulonglong extractedSize =targz_archive_size( packageFile );


    if ( extractedSize > expectedSize )
    {
        if ( reporter )
        {
            reporter->reportError( QObject::tr("Uncompressed package larger than advertised size" ),
                            QString( "InstallControl:installPackage:- (maximum)expected uncompressed size: %1 "
                                     "uncompressed size: %2 (prior to extracting data.tar.gz)")
                                .arg( expectedSize ).arg( extractedSize ) );
        }
        return false;
    }


    if ( !check_tar_valid( packageFile ) )
    {
        if ( reporter )
        {
            reporter->reportError( QObject::tr("Unable to unpack package, package is invalid" ),
                    QString( "Installcontrol:installpackage:- package qpk file is invalid, "
                        "Package must not contain device nodes, hard links, absolute paths or path references to parent "
                        "directories or symlinks which refer to absolute paths or parent directories" ) );
        }
        return false;
    }

    if( !targz_extract_all( packageFile, job.destinationPath(), false ) )
    {
        if( reporter )
        {
            reporter->reportError( QObject::tr( "Unable to unpack package, package may be invalid" ),
                        QString( "InstallControl::installPackage:- Could not untar %1 to %2" )
                            .arg( packageFile )
                            .arg( job.destinationPath() ));
        }
        return false;
    }

    QFile tmpPackage( packageFile );
    tmpPackage.remove();

    // extract data part
    extractedSize += targz_archive_size( dataTarGz ) - QFile( dataTarGz).size();

    if ( extractedSize > expectedSize )
    {
        if ( reporter )
        {
            reporter->reportError( QObject::tr("Uncompressed package larger than advertised size" ),
                    QString( "Installcontrol:installpackage:- (maximum)expected uncompressed size: %1 "
                        "uncompressed size of qpk: %2 (after extracting data.tar.gz)")
                    .arg( expectedSize ).arg( extractedSize ) );
        }
        return false;
    }

    if ( !check_tar_valid( dataTarGz ) )
    {
        if ( reporter )
        {
            reporter->reportError( QObject::tr("Unable to unpack package, package is invalid" ),
                    QString( "Installcontrol:installpackage:- package's data.tar.gz is invalid "
                        "Package must not contain devices nodes, hard links, absolute paths or path references to parent "
                        "directories or symlinks which refer to absolute paths or parent directories" ) );
        }
        return false;
    }

    if( !targz_extract_all( dataTarGz, job.destinationPath(), false ) )
    {
        if( reporter )
        {
            reporter->reportError( QObject::tr( "Unable to unpack package, package may be invalid" ),
                        QString( "InstallControl::installPackage:- Could not untar %1 to %2" )
                            .arg( dataTarGz )
                            .arg( job.destinationPath() ));
        }
        return false;
    }

    // remove data package
    QFile dataPackage( dataTarGz );
    dataPackage.remove();


    if ( !verifyPackage( job.destinationPath(), pkg, reporter ))
        return false;

    job.preprocessPackageFiles();

    if ( pkg.isSystemPackage() )
    {
        QStringList dirs = Qtopia::installPaths();
        QString systemRootPath;
        for ( int i = 0; i < dirs.count(); ++i )
        {
            const QFileSystem *f = QStorageMetaInfo::instance()->fileSystemOf( dirs[i] );
            if ( f->isWritable() )
                systemRootPath = dirs[i];
        }
        if ( systemRootPath.isEmpty() )
        {
            qWarning( "******* No writeable system path for system package *******" );
            if ( !job.setupSandbox() )
                return false;
        }
        else
        {
            qLog(Package) << "promoting to system package:  move from" << job.destinationPath()
                << "to" << systemRootPath;
            if ( !targz_extract_all( dataTarGz, systemRootPath ))
            {
                if( reporter )
                {
                    reporter->reportError( QObject::tr( "Unable to unpack package, package may be invalid" ),
                                QString( "InstallControl::installPackage:- Could not untar %1 to %2" )
                                    .arg( dataTarGz )
                                    .arg( job.destinationPath() ));
                }
                return false;
            }
            job.removeDestination();
        }
    }
    else
    {
        if (!job.setupSandbox())
            return false;
    }

    if( !job.installContent() )
        return false;

    return true;
}
/*!
  Uninstall the package \a pkg off the device.
*/
void InstallControl::uninstallPackage( const InstallControl::PackageInfo &pkg, ErrorReporter * /*reporter*/ ) const
{
    SandboxUninstallJob job( &pkg, m_installMedia );
    job.terminateApps();
    job.unregisterPackageFiles();
    job.dismantleSandbox();
}

/*!
  Verify the contents of the unzipped package before finally registering the
  installation.  If the contents fail verification a QMesssageBox is displayed
  to the user, and the package is marked as disabled.

  Checks made in the verification step are:
  \list
  \o Is the MD5 sum and/or GPG signature valid? (Not currently implemented)
  \o Is the control file valid, ie not corrupt?
  \o Do the security domains declared in the package information (received
  at browse time eg via http) match the domains declared in the control file?
  \o If not certified, are the domains within the set allowed for untrusted applications? (Not currently implemented)
  \o If shipped with a certificate, is it valid against our root store? (Not currently implemented)
  \endlist
  */
bool InstallControl::verifyPackage( const QString &packagePath, const InstallControl::PackageInfo &pkg, ErrorReporter *reporter ) const
{
    PackageInformationReader infoReader( packagePath + QDir::separator() +
            AbstractPackageController::INFORMATION_FILE, InstallControl::PackageInfo::Control );
    if ( infoReader.getIsError() )
    {
        if( reporter )
        {
            QString simpleError = QObject::tr("Invalid package, contact package supplier");
            QString detailedError = QLatin1String("InstallControl::verifyPackage:-" 
                                    "Error during reading of package information file: ") 
                                    + infoReader.getError();
            reporter->reportError( simpleError, detailedError );
        }
        return false;
    }

#ifdef QT_NO_SXE
    if ( infoReader.type().toLower() == "sxe-only" )
    {
        if( reporter )
        {
            QString simpleError = QObject::tr( "Incompatible package, contact package supplier");
            QString detailedError("InstallControl::verifyPackage:- Trying to install sxe package "
                                  "with a non-sxe configured qtopia" );
            reporter->reportError( simpleError, detailedError );
        }
        return false;
    }
#endif

    if ( infoReader.domain() != pkg.domain )
    {
        if( reporter )
        {
            QString simpleError = QObject::tr( "Invalid package, security requirements differ from declared "
                                                   "security requirements. Contact package supplier");
            QString detailedError("InstallControl::verifyPackage:- descriptor domain(s): %1, control file domain(s): %2");
            detailedError = detailedError.arg( pkg.domain ).arg( infoReader.domain() );
            reporter->reportError( simpleError, detailedError );
        }
        return false;
    }

    if ( infoReader.installedSize() != pkg.installedSize )
    {
        if( reporter )
        {
            QString simpleError = QObject::tr( "Invalid package, contact package supplier" );
            QString detailedError("InstallControl::verifyPackage:- installed sizes inconsistent "
                                  " descriptor installed size: %1, control file installed size: %2 " );
            detailedError = detailedError.arg( pkg.installedSize ).arg( infoReader.installedSize() );
            reporter->reportError( simpleError, detailedError );
        }
        return false;
    }

    if ( !infoReader.md5Sum().isEmpty() )
    {
        if ( reporter )
        {
            reporter->reportError( QObject::tr( "Invalid package, contact package supplier" ),
                QObject::tr( "InstallControl::verifyPackage:- Control file should not contain md5" ));
        }
        return false;
    }

    if ( !VersionUtil::checkVersionLists( QLatin1String(QTOPIA_COMPATIBLE_VERSIONS), infoReader.qtopiaVersion() ) )
    {
        if ( reporter )
        {
            QString detailedError( "InstallControl::verifyPackage:- Control file's qtopia version incompatible. "
                                   "Package's compatible Qtopia Versions %1, Qtopia Version %2");
            detailedError = detailedError.arg( infoReader.qtopiaVersion(), Qtopia::version() );
            reporter->reportError( QObject::tr( "Incompatible package, contact package supplier"), detailedError );
        }
        return false;
    }

    //disallow packages not compatible with device
    if ( !DeviceUtil::checkDeviceLists( QLatin1String(QTOPIA_COMPATIBLE_DEVICES), infoReader.devices()) )
    {
        if ( reporter )
        {
            QString detailedError( "InstallControl::verifyPackage:- Package is not device compatible "
                                   "Package's compatible devices: %1, Qtopia's compatible devices %2");
            detailedError = detailedError.arg( infoReader.devices(), QTOPIA_COMPATIBLE_DEVICES );
            reporter->reportError( QObject::tr( "Package is not device compatible" ),
                            detailedError );
        }
        return false;
    }

    QString certPath = infoReader.trust();
    if ( certPath.isEmpty() || certPath == "Untrusted" ) // untrusted package
        return true;
    QFileInfo certFile( packagePath + QDir::separator() + certPath );
    if ( !certFile.exists() )
    {
        qWarning( "Failed verification: cert file %s not found", qPrintable( certFile.filePath() ));

        return false;
    }
    if ( certFile.isFile() )
    {
        return verifyCertificate( certFile.filePath() );
    }
    else if ( certFile.isDir() )  // directory with cert alternatives, one at least must verify
    {
        QDir certDir( certFile.filePath() );
        QFileInfoList flist = certDir.entryInfoList();
        for ( int f = 0; f < flist.count(); ++f )
        {
            if ( flist[f].isFile() && verifyCertificate( flist[f].filePath() ))
                return true;
        }
        return false;
    }
    else
    {
        // was something other than a file or a directory, that's got to be bad
        return false;
    }

    // TODO - Security Monitor / Launcher needs to mark packages as disabled
    return true;
}

bool InstallControl::verifyCertificate( const QString &certPath ) const
{
    // TODO - Implement certificate verification
    Q_UNUSED( certPath );
    return false;
}

bool InstallControl::PackageInfo::isComplete( InstallControl::PackageInfo::Source source, QString *reason ) const
{
    if ( name.isEmpty() )
    {   
        if ( reason )    
            (*reason) = QLatin1String( "Name field is empty/non-existent" );
        return false; 
    } else if ( description.isEmpty() )
    {
        if ( reason )
            (*reason) = QLatin1String( "Description field is empty/non-existent" );
        return false;
    } else if ( qtopiaVersion.isEmpty() )
    {
        if ( reason )
            (*reason) = QLatin1String( "QtopiaVersion field is empty/non-existent" );
        return false; 
    } else if ( installedSize.isEmpty() )
    {
        if ( reason )
            (*reason) = QLatin1String( "Installed-Size field is empty/non-existent" );
        return false; 
    }
#ifndef QT_NO_SXE
    else if ( domain.isEmpty() )
    {
        if ( reason )
            (*reason) = QLatin1String( "Domain field is empty/non-existent" );
        return false;
    }
#endif

    //if pkg info is read off packages.list (as opposed to control file)
    if ( source == InstallControl::PackageInfo::PkgList  )
    {
        if ( md5Sum.isEmpty() )
        {
            if ( reason )
                (*reason) = QLatin1String( "Md5Sum field is empty/non-existent" );
            return false;
        }
        else if ( downloadSize.isEmpty() )
        {
            if ( reason )
                (*reason) = QLatin1String( "Size field is empty/non-existent" );
            return false;
        }
    }
    return true;
}

