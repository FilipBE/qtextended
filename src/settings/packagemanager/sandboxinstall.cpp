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

#include "sandboxinstall.h"
#include "packageview.h"
#include "utils.h"

#include <qtopianamespace.h>
#include <qcontent.h>

#ifdef QT_NO_SXE
#define PACKAGE_DIRECTORY "packages"
#else
#include <qpackageregistry.h>
#define PACKAGE_DIRECTORY QPackageRegistry::packageDirectory
#include <qmutex.h>
#endif

#include <errno.h>
#include <qtopialog.h>

#include <QDir>
#include <QProcess>
#include <QtopiaServiceRequest>

#include <qcontentfilter.h>
#include <qcontentset.h>
#include <qcontent.h>
#include <qtopiachannel.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>

//directories found in the packages root path
#define BIN "bin"
#define PICS "pics"
#define SOUNDS "sounds"
#define HELP "help"
#define CONTROLS "controls"
#define I18N "i18n"

/*!
  Construct a new sandbox install job object, ensuring the sandbox
  root exists, and creating the destination directory for the
  package to be expanded into.

  The destination directory is below Qtopia::packagePath() and is formed from
  the md5sum of the package.

  The \a pkg points to the package which will be installed.

  If \a m is non-empty it specifies the file-system path mount-point
  of a media card (or other writeable file-system) on which the package
  will be located.

  In this case the destination directory will be a symlink from the
  system root to the location on the media card.
  */
SandboxInstallJob::SandboxInstallJob( const InstallControl::PackageInfo *pkg, const QString &m, ErrorReporter *errorReporter )
    : package( pkg )
    , media( m )
    , abort( false )
    , reporter( errorReporter )
{

    QDir sandboxRoot( Qtopia::packagePath() );
    destination = sandboxRoot.path() + "/" + pkg->md5Sum;
    Q_ASSERT( sandboxRoot.exists() );

    QString errReason;
    if ( !pkg->isComplete(InstallControl::PackageInfo::PkgList, &errReason ) )   // must have the md5Sum value, plus content!
    {

        if( reporter )
        {
            QString simpleError =  QObject::tr( "Invalid package, contact package supplier" );
            QString detailedError( "SandboxInstallJob::SandboxInstallJob:- PackageInfo pkg is incomplete: %1" );
            detailedError = detailedError.arg( errReason );
            reporter->reportError( simpleError, detailedError );
        }

        abort = true;
        return;
    }

    if (sandboxRoot.exists( pkg->md5Sum ))
    {
        qWarning( QString("SandboxInstallJob::SandboxInstallJob:- "
                          "Package directory matching %1 already exists, clearing out "
                          "residual package elements" ).arg(pkg->md5Sum).toLatin1() );

        SandboxUninstallJob job( pkg, m, reporter );
        job.terminateApps();
        job.unregisterPackageFiles();
        job.dismantleSandbox();
    }

    if ( media.isEmpty() )
    {
        if ( !sandboxRoot.mkdir( pkg->md5Sum ))
        {
            if( reporter )
            {
                QString simpleError = QObject::tr( "Error occurred during installation" );
                QString detailedError = QString( "SandboxInstallJob::SandboxInstallJob:- "
                                               "Could not create directory %1")
                                       .arg( pkg->md5Sum );
                reporter->reportError( simpleError, detailedError );
            }

            abort = true;
            return;
        }
    }
    else
    {
        mediaSandboxRoot();
    }

    QStringList dirs;
    dirs << BIN << PICS << SOUNDS << HELP << CONTROLS << I18N;
    foreach ( QString dir, dirs )
    {
        if ( !QDir::root().exists( Qtopia::packagePath() + dir) )
            QDir::root().mkpath( Qtopia::packagePath() + dir );
    }
}

/*!
  \fn QString SandboxInstallJob::destinationPath() const

  Return the path within the Qtopia::packagePath() directory where the sandbox
  directories will be set up for this install.

  If this is an install to a media card, then these will be symlinks.

  The name of the directory is based on the md5sum of the package.
*/

/*!
  \fn bool SandboxInstallJob::isAborted() const

  Return true if the install job has been aborted.  A warning will have been
  displayed on stderr, and possibly to the screen of the device.
*/

/*!
  Create a sandbox install directory on the media card for this package
  and if necessary create the media card "package" directory parent.
  The sandbox install directory is symlinked to the main package root.
*/
void SandboxInstallJob::mediaSandboxRoot()
{
    QDir mediaSandbox( media + "/" + PACKAGE_DIRECTORY );
    if ( !mediaSandbox.exists() )  // try creating it from parent
    {
        QDir mediaDir( media );
        if ( !mediaDir.exists() )
        {
            //TODO: Proper error reporting for media card errors
            qWarning( "### %s (the media path) doesnt exist! ###", qPrintable( media ));
            abort = true;
            return;
        }
        qLog(Package) << "Creating directory" << mediaSandbox.path() << "in path" << media;
        if ( !mediaDir.mkdir( PACKAGE_DIRECTORY ))
        {
            //TODO: Proper error reporting for media card errors
            qWarning( "mkdir failed : %s", strerror( errno ));
            abort = true;
            return;
        }
    }
    QString linkTarget = mediaSandbox.path() + "/" + package->md5Sum;
    QFile::link( linkTarget, destination );
    destination = linkTarget;
    if ( mediaSandbox.exists( package->md5Sum ))
    {
        // overwriting with reinstalled package
        removeDestination();
    }
    mediaSandbox.mkdir( package->md5Sum );
}

/*!
  Remove the destination directory for the sandboxed package and
  everything below it.

  Use this method with care.
  */
void SandboxInstallJob::removeDestination() const
{
    QDir destDir( destination );
    if ( !destDir.exists() )
    {
        qWarning( "Request to remove non-existent directory %s", qPrintable( destination ));
        return;
    }
    QString cmd( "rm -Rf " );
    cmd += destination;
    Q_ASSERT( !destination.isEmpty() && destination != "/" );
    qLog(Package) << "Removing destination by executing:" << cmd;
    ScriptRunner::runScript( cmd );

    QDir dir( Qtopia::packagePath() );
    if ( dir.cd("tmp") )
    {
        QFileInfoList fileList= dir.entryInfoList( QDir::Files );
        QFile f;
        foreach( QFileInfo fi, fileList )
        {
            f.setFileName( fi.absoluteFilePath() );
            f.remove();
        }
    }
    SandboxUninstallJob::clearMiscFiles(package);
}

/*!
  Recurse through a given directory to find any desktop files
  and add them to the desktopPaths variable for later use.

  Also make any executables found writable (the need for doing
  this is deprecated and should eventaully be removed)
  */
bool SandboxInstallJob::preprocessPackageFiles( const QString &path )
{
    QDir directory( path.isEmpty() ? destination : path );

    qLog(Package) << "registering package files in" << directory.path();
    QFileInfoList fileList = directory.entryInfoList();
    for (int fileI = 0; fileI < fileList.count(); ++fileI)
    {
        QFileInfo fileInfo = fileList.at(fileI);

        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        // recurse to other directories
        if (fileInfo.isDir()) {
            preprocessPackageFiles( fileInfo.filePath() );
        }
        else if ( fileInfo.isFile() )
        {
            if ( fileInfo.suffix() == "desktop" )
            {
                qLog(Package) << "Adding to dot.desktop files" << fileInfo.path();
                desktopPaths << fileInfo.filePath();
            }
            else if ( fileInfo.isExecutable() )
            {//Note: the need for making executables writable is deprecated
                if ( !fileInfo.isWritable() )
                {
                    QFile::Permissions perms = QFile::permissions( fileInfo.filePath() );
                    if ( !QFile::setPermissions( fileInfo.filePath(), perms | QFile::WriteOwner ) )
                    {
                        if ( reporter )
                        {
                            QString detailedError( "SandboxInstallJob::registerPackageFiles:- "
                                                "%1 is not writable and could not change permissions");
                            detailedError = detailedError.arg( qPrintable(fileInfo.filePath()) );
                            reporter->reportError( tr( "Error occurred during installation" ),
                                                    detailedError );
                        }
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool SandboxInstallJob::installContent()
{
    if ( desktopPaths.count() == 0 )
    {
        qLog(Package) << "SandboxInstallJob::installContent - No desktop files!";
        return true;
    }
    QList<QContentId> cids;
    foreach ( QString d, desktopPaths )
    {

        QContentId cid = QContent::install( d );
        if ( cid == QContent::InvalidId )
        {
            if ( reporter )
            {
                QString detailedError( "Unable to install %1 to database" );
                detailedError = detailedError.arg( d );
                reporter->reportError( QObject::tr("Error occurred during installation"),
                                        detailedError );
            }

            //uninstall previously install desktop files
            foreach( QContentId id, cids )
               QContent::uninstall( id );

            return false;

        }
        cids << cid;

        QContent content( cid );
        content.setCategories( content.categories() << QLatin1String("Packages") );

        if( !content.commit() )
        {
            if ( reporter )
            {
                QString detailedError( "Unable to commit %1 to database" );
                detailedError = detailedError.arg( d );
                reporter->reportError( QObject::tr("Error occurred during installation"),
                                        detailedError );
            }

            //uninstall previously install desktop files
            foreach( QContentId id, cids )
               QContent::uninstall( id );

            return false;
        }
    }
    return true;
}

/*!
   Execute the systems MAC kernel sandboxing rule on the binary
   by invoking the "sandbox" shell script, from the qtopia etc directory.

   The \a binPath is passed as an argument to the shell script, and is
   the full path to the binary.

   The second argument to the shell script is \a sandboxDir which is the full
   path to the directory into which the binary is installed, and to which it
   has write permissions.

   Without additional permissions, the binary will
   not be able to write any directories outside this one.
*/
void SandboxInstallJob::runSandbox( const QString &binPath, const QString &sandboxDir )
{
    static QString sandboxCmd;
    static bool commandUnavailable = false;

    if ( !LidsUtils::isLidsEnabled() )
        return;

    if ( commandUnavailable )
        return;

    if ( sandboxCmd.isEmpty() )
    {
        QStringList dirs = Qtopia::installPaths();
        for ( int d = 0; d < dirs.count(); ++d )
        {
            if ( QFile::exists( dirs[d] + "etc/sxe_qtopia/sxe_sandbox" ))
            {
                sandboxCmd = dirs[d] + "etc/sxe_qtopia/sxe_sandbox %1 %2";
                break;
            }
        }
        if ( sandboxCmd.isEmpty() )
        {
            commandUnavailable = true;
            qLog(Package) << "Warning: Could not find sxe_sandbox script!";
            return;
        }
    }
    QString cmd = sandboxCmd.arg( binPath ).arg( sandboxDir );
    qLog(Package) << "Running command" << cmd;
    ScriptRunner::runScript( cmd );
}

/*!
  \deprecated
  Execute the domain specific MAC kernel rules for the \a domain, by invoking the
  shell scripts in the qtopia \c {etc/sxe_domains} directory.

  If that directory cannot be found in any of the Qtopia::installPaths() then
  a warning message is raised in the package log.

  Each such directory is searched for a script named \c {sxe_qtopia_domain} until
  one is found, and executed.  If no matching script is found, this method will
  exit without raising any warning.

  The script is called with the full path \a binPath to the binary.

  The effect is to award additional privileges to the binary, based on domains
  which have been authorised at SXE download time.
*/
void SandboxInstallJob::applyDomainRules( const QString &binPath, const QString &domain )
{
    static QMap<QString,QString> domainScripts;
    static bool scriptsNotFound = false;
 
    if ( !LidsUtils::isLidsEnabled() )
                return;

    if ( scriptsNotFound == true )
        return;

    if ( domainScripts.count() == 0 )
    {
        QString scriptPath;
        QStringList dirs = Qtopia::installPaths();
        QString scriptDirPath;
        for ( int d = 0; d < dirs.count(); ++d )
        {
            QDir scriptDir( dirs[d] + "etc/sxe_domains" );
            if ( !scriptDir.exists() )
                continue;
            scriptDirPath = scriptDir.path();
            QStringList scriptEntries = scriptDir.entryList( QDir::Files );
            foreach( QString scr, scriptEntries )
            {
                if( !scr.startsWith( "sxe_qtopia_") )
                    qFatal( "script entry bad (does not begin with sxe_qtopia_): %s",
                            qPrintable( scr ));

                QString domain = scr.mid( 11 );//11 is size of sxe_qtopia_
                if ( !domainScripts.contains( domain ))
                    domainScripts.insert( domain, scriptDir.absolutePath() + "/" + scr );
            }
        }
        if ( scriptDirPath.isEmpty() )
        {
            scriptsNotFound = true;
            qLog(Package) << "Warning: Could not find \"etc/sxe_domains\" directory";
            return;
        }
        if ( domainScripts.count() == 0 )
        {
            scriptsNotFound = true;
            qLog(Package) << "Warning:" << scriptDirPath << "was empty";
            return;
        }
    }
    if ( !domainScripts.contains( domain ))
        return;
    //set binary path for domain script
    ::setenv( "BIN", qPrintable(binPath), 1 );

    //set qcop message file for domain script
    ::setenv( "QCOP_FILE", qPrintable( Qtopia::tempDir()
                + QLatin1String("qcop-msg-")
                + package->md5Sum + QLatin1Char('_')
                + binPath.mid( binPath.lastIndexOf('/') + 1 )),1);
    //set the package path for domain script
    //so permissions can be given to acces help etc
    ::setenv( "PACKAGES_DIR", qPrintable(Qtopia::packagePath()),1 );

    QString domainCommand = domainScripts[domain];
    qLog(Package) << "Running domain script: " << ( domainCommand );
    ScriptRunner::runScript( domainCommand );
}

/*!
    Enforce and ensure that MAC kernel rules are in effect
    by reloading the MAC configuration files.
*/
void SandboxInstallJob::reloadRules()
{
    static QString reloadCmd;
    static bool commandUnavailable = false;

    if ( !LidsUtils::isLidsEnabled() )
                return;

    if ( commandUnavailable )
        return;

    if ( reloadCmd.isEmpty() )
    {
        QStringList dirs = Qtopia::installPaths();
        for ( int d = 0; d < dirs.count(); ++d )
        {
            if ( QFile::exists( dirs[d] + "etc/sxe_qtopia/sxe_reloadconf" ))
            {
                reloadCmd = dirs[d] + "etc/sxe_qtopia/sxe_reloadconf";
                break;
            }
        }
        if ( reloadCmd.isEmpty() )
        {
            commandUnavailable = true;
            qLog(Package) << "Warning: Could not find sxe_reloadconf script!!";
            return;
        }
    }
    qLog(Package) << "Running command" << reloadCmd;
    ScriptRunner::runScript( reloadCmd );
}


/*!
  Create the sandbox directory structure and other settings for the untrusted
  package.  The directory structure assumes a Qt Extended file system similar to
  the following:

  \image package.png

  Note that the installPath() includes the "package" directory, so that eg searches
  for binaries will include "package/bin".

  Here the package "BomberMan" is a game with two binaries, a library, an icon
  and several help files (not shown but assumed to be in the help/html directory).

  It has has the following entries in its bomb.desktop file:

  \code
  Name[]=Bomber Man
  Exec = game
  Icon=BomberMan/bomb
  \endcode

  The Qt Extended launcher must be able to find the binary, which therefore must be
  in the installPath() and must not clash with other binary names.

  So that the untrusted packages are found last (preventing an untrusted package
  masquerading as for example "netsetup") they are placed in the "packages"
  directory heierarchy via links.  Here they are returned at the end of the
  installPath() during search for binaries to launch.

  Note also that package authors could easily create names which clash with
  each other - in the example the "game" executable in "BomberMan" clashes with
  an executable from the "CannonMan" package.

  For this reason binaries are linked into the installPath() using the packages md5sum.
  This way unless the packages are actually the same package, the binary names will not clash.
  The value of the "Exec" field is altered accordingly during installation.

  This is also important for IPC messaging since the binary name (and argv[0]) is
  used to create messaging endpoints, and therefore must be unique Qtopia-wide.

  The Qt Extended launcher must also be able to find the icon.  Here again the
  installPath() is searched automatically for the value of the "Icon=" field
  so hence the BomberMan directory is symlinked from the "pic" directory.

  The items which the Qt Extended system must be able to locate within a sandboxed package
  are as follows, where the paths given are relative to the packages sandbox directory:
  \list
  \o executable binary, in "bin" as listed in the "Exec=" field of .desktop
  \o quicklaunched binary, in  "plugins/applications"
  \o package launcher icon (as listed in the "Icon=" field of the .desktop file)
  \o package help files (not shown in above diagram, usually "help/html")
  \endlist

  During package installation the following files are referenced:
  \list
  \o desktop file, eg "apps/Applications/bomb.desktop"
  \o certificates, eg "etc/crt/somecorp.pem"
  \endlist

  A package can quite likely have more than one .desktop file, so there
  might be a game and a level editor, with a .desktop file each.  This will
  create an icon in the launcher view for each.

  On execution the packaged binary is launched with the following environment,
  where the path to the sandbox directory is represented by \c $SANDBOX:
  \list
  \o $SANDBOX added to its installPath()
  \o $SANDBOX/bin added to PATH
  \o $SANDBOX/lib added to LD_LIBRARY_PATH
  \o chdir() to $SANDBOX
  \endlist

  Note that since the binary is launched via the symlink named after its md5sum,
  this same string must be linked in the sandbox "pic" directory to locate image resources
  The Qt Extended resource system will search in "pic/" + argv[0] for images.
*/
bool SandboxInstallJob::setupSandbox()
{
    QList<QSettings*> desktops;
    qLog(Package) << "Desktop files are" << desktopPaths;

    if ( desktopPaths.count() == 0 )
    {
        if ( reporter )
        {
            QString detailedError( "SandboxInstalLJob::setupSandbox:- "
                            "No desktop files");
            reporter->reportError( tr( "Invalid package, contact package supplier"),
                                    detailedError ) ;
        }
        return false;
    }

    //ensure desktop files are writable and
    //populate the desktops variable
    for ( int x = 0; x < desktopPaths.count(); ++x )
    {
        QFile::Permissions perms = QFile::permissions( desktopPaths[x] );
        if ( !QFile::setPermissions(desktopPaths[x], perms | QFile::WriteOwner) )
        {

            if ( reporter )
            {
                QString detailedError( "SandboxInstallJob::setupSandbox:- "
                                "%1 is not writable and could not change permissions");
                detailedError = detailedError.arg( qPrintable( desktopPaths[x]) );
                reporter->reportError( tr( "Error occurred during installation" ),
                                                    detailedError );
            }
            return false;
        }

        QSettings *dc = new QSettings( desktopPaths[x], QSettings::IniFormat );
        dc->beginGroup( QLatin1String( "Desktop Entry" ));
        QString exec = dc->value( QLatin1String("Exec"), QLatin1String("") ).toString();
        qLog(Package) << "found Exec field" << exec << "in" << desktopPaths[x];
        if ( exec.isEmpty() )
            delete dc;
        else
            desktops.append( dc );
    }

    QString installSystemBinPath = Qtopia::packagePath() + '/' + BIN;
    QString sandboxBinPath = destination + '/' + BIN;
    QDir packageBin( sandboxBinPath );

    if ( !packageBin.exists() )
    {
        if ( reporter )
        {
            reporter->reportError( tr("Invalid package, contact package supplier"),
                            QLatin1String("Package did not contain bin directory "
                            "at package root level") );
        }
        return false;
    }


    // the package bin directory MUST actually be a directory
    // as opposed to a symlink otherwise it is possible
    // to spoof binaries
    QFileInfo packageBinInfo( sandboxBinPath );
    if ( !packageBinInfo.isDir() || packageBinInfo.isSymLink() )
    {
        if ( reporter )
        {
            reporter->reportError( tr("Invalid package, contact package supplier"),
                                QString("SandboxInstallJob::setupSandbox:- ")
                                + sandboxBinPath + " is not a directory" );
        }
        return false;
    }

    //create binary, icon and sound links
    //these are done on a per binary basis
    QStringList binaries = packageBin.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

#ifndef QT_NO_SXE
    SxeProgramInfo pi;
#endif
    QString origBin;
    foreach( QSettings *dc, desktops )
    {
        origBin = dc->value(QLatin1String("Exec")).toString();
        if ( binaries.contains( origBin ) )
        {
#ifndef QT_NO_SXE
            QString registerPath = destination + "/" + BIN;
            registerPath.remove( Qtopia::packagePath() );
            QString binPath = sandboxBinPath + "/" + origBin;
            pi.fileName = origBin;
            pi.relPath = registerPath;
            pi.runRoot = Qtopia::packagePath();
            pi.domain = package->domain;
            emit newBinary( pi );
            runSandbox( binPath, destination );
            QStringList domainList = package->domain.split(",");
            foreach ( QString dom, domainList )
                applyDomainRules( binPath, dom );

            if  (LidsUtils::isLidsEnabled() && LidsUtils::maxRulesExceeded())
            {
                if (reporter)
                {
                    QString detailedMessage("Maximum number of LIDS rules exceeded");
                    reporter->reportError(tr("Cannot install package, maximum number of packages reached"),
                                            detailedMessage);
                }
                return false;
            }

            reloadRules();
#endif
        }
        else
        {
            if ( reporter )
            {
                QString detailedMessage( "SandboxInstallJob::setupSandbox:- desktop file %1 "
                                    "references binary %2 that does not exist in the package "
                                    "bin directory: %3");
                detailedMessage = detailedMessage.arg( dc->fileName() )
                                    .arg( dc->value(QLatin1String("Exec")).toString() )
                                    .arg( sandboxBinPath );
                reporter->reportError( tr("Invalid package, contact package supplier" ),
                                    detailedMessage );
            }
            return false;
        }

        QString newBin = package->md5Sum + "_" + origBin;
        QString link = Qtopia::packagePath() + '/' + BIN + '/' + newBin;
        if ( !createLink( sandboxBinPath + '/' + origBin, link) )
        {
            if ( reporter )
            {
                QString detailedMessage( "SandboxInstallJob::setupSandbox:- could not "
                        "create link, " + link + " to " + sandboxBinPath + '/' + origBin );
            }
            return false;
        }

        dc->setValue( "Exec", newBin);
        qLog(Package) << "Re-writing desktop file Exec from" << origBin << "to" << newBin;

        QDir packagePic( destination + '/' + PICS + '/' + origBin );
        if ( packagePic.exists() ) // there is a pics directory
        {
            link = Qtopia::packagePath() + '/' +  PICS + '/' + newBin;
            createLink( packagePic.path(), link );

            QString icon = dc->value( "Icon" ).toString();
            if ( icon.startsWith( origBin + '/' ) )
            {
                QString newIcon = newBin + icon.mid( origBin.length() );
                dc->setValue( "Icon", newIcon );
                qLog(Package) << "Re-writing desktop file Icon from" << origBin << "to" << newBin;
            }
        }

        QDir packageSounds ( destination + '/' + SOUNDS + '/' + origBin );
        if ( packageSounds.exists () )
        {
            link = Qtopia::packagePath() + '/' + SOUNDS + '/' + newBin;
            createLink( packageSounds.path(), link );
        }

        dc->endGroup();
        dc->beginGroup( QLatin1String( "Package Data" ));
        dc->setValue( QLatin1String( "Path" ), destination );
        dc->sync();

        qLog(Package) << "deleting desktop settings object for" << origBin;
        // delete QSettings object, causes .desktop file to be persisted
        delete dc;
    }

    //create help links
    QString packageHelpDir( destination + '/' + HELP + '/' );
    if ( QDir::root().exists(packageHelpDir) )
        registerHelpFiles(packageHelpDir );

    //create i18n links
    QString packageI18NDir( destination + '/' + I18N );
    if ( QDir::root().exists(packageI18NDir) )
        registerI18NFiles( packageI18NDir);

    //create settings links
    if ( !setupSettingsFiles() )
        return false;

    QString origControl( destination +  QLatin1String( "/control") );
    QString newControl = Qtopia::packagePath() + CONTROLS + '/'
                            + package->md5Sum + QLatin1String( ".control" );
    QFile::rename( origControl, newControl );
    QFile newControlFile( newControl );
    if( !newControlFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
    {
        if ( reporter )
        {
            reporter->reportError( tr( "Error occurred during installation" ),
                QLatin1String("SandboxInstallJob::setupSandbox:-Could not modify control file" ) );
        }
        return false;
    }
    QString md5Entry("MD5Sum: " );
    md5Entry.append( package->md5Sum + "\n" );
    newControlFile.write( md5Entry.toLatin1() );

    return true;
}

/*!
  Create a symlink which points to \a target.  The symlink has the path \a link.

  If possible a relative path, ie ../../path/to/link is used for the target.

  If any of the directories making up the /path/to/link are missing, they
  are created.

  No test for existence of the target file is made - thus a "dangling" link
  can be made.
*/
bool SandboxInstallJob::createLink( const QString &target, const QString &link )
{
    QStringList linkPath = link.split( "/", QString::SkipEmptyParts );
    QStringList targetPath = target.split( "/", QString::SkipEmptyParts );

    // ensure the directory that will contain the link exists
    linkPath.removeLast();
    QDir dirCheck( linkPath.join( "/" ).prepend( "/" ));
    if ( !dirCheck.exists() )
    {
        qLog(Package) << "Building directory path" << dirCheck.path();
        QString linkCheck = "";
        QDir prevDir( "/" );
        for ( int i = 0; i < linkPath.count(); ++i )
        {
            linkCheck = linkCheck + "/" + linkPath[i];
            dirCheck.setPath( linkCheck );
            if ( !dirCheck.exists() )
            {
                if ( !prevDir.mkdir( linkPath[i] ))
                {
                    if ( reporter )
                    {
                        QString detailedError( "SandboxInstallJob::createLink:- "
                                    "While making link %1, in dir %2 could not mkdir %3 - for %4");
                        detailedError = detailedError.arg( link ).arg( prevDir.path() ).arg( linkPath[i] ).arg( dirCheck.path());
                        reporter->reportError( tr("Error occurred during installation"), detailedError );

                    }
                    return false;
                }
                qLog(Package) << "Making dir" << linkPath[i] << "in" << prevDir.path();
            }
            prevDir = dirCheck;
        }
    }

    QString targetBuf;
    int upCount = linkPath.count();
    int fullCount = upCount;
    int maxDots = qMin( linkPath.count(), targetPath.count() - 1 );
    int pathDivergePtr = 0;
    for ( ; pathDivergePtr < maxDots; ++pathDivergePtr, --upCount )
        if ( linkPath[pathDivergePtr] != targetPath[pathDivergePtr] )
            break;
    if ( fullCount > upCount ) // target relative path with dots will save space
    {
        for ( int i = 0; i < upCount; ++i )
            targetBuf += "../";
        for ( int i = pathDivergePtr; i < targetPath.count(); ++i )
        {
            targetBuf += targetPath[i];
            if ( i < targetPath.count() - 1 )
                targetBuf += "/";
        }
    }
    else
    {
        targetBuf = target;
    }

    if ( !QFile::link( targetBuf, link ))
    {
        if ( reporter )
        {
            QString detailedError( "SandboxInstallJob::createLink:- "
                    "Could not create link %1 pointing to file %2" );
            detailedError = detailedError.arg( targetBuf ).arg( target );
            reporter->reportError( tr("Error occurred during installation"), detailedError );
        }
        return false;
    }
    qLog(Package) << "Created link" << link << "pointing to" << targetBuf;
    return true;
}

/*
    \internal
    Register help files in the \a helpDir so they can be found by the help system.
    This function will recurse and register files in any subdirectories.

    Implementation Note:
    Registration occurs by adding symlinks in Qtopia::packagePath()/help to
    the application's help files.

    The help files are found according to the application name,
    For a downloaded application executable <app>,
    the QtopiaApplication::applicationName is <md5sum>_<app>, therefore the help
    symlinks must be prepended with <md5sum>_.(Typically help files are
    of the form <app>[-<suffix>].html)
    Because of this prepended md5sum, html links in the help files must be modified.
*/
void SandboxInstallJob::registerHelpFiles( const QString &helpDir  )
{
    QDir dir( helpDir  );
    if ( !dir.exists() )
        qWarning() << "SandboxInstallJob::registerHelpFiles called with param, "
                      "helpDir that is not a directory:" << helpDir;
    QFileInfoList files = dir.entryInfoList( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );

    QRegExp rx;
    rx.setMinimal( true );
    rx.setPattern("(<\\s*[a|A].*[h|H][r|R][e|E][f|F]\\s*=\\s*\")(.*)(\".*>)");

    QString contents;
    foreach( QFileInfo file, files )
    {
        //recurse if subdirectory is found
        if ( file.isDir() )
            registerHelpFiles( file.filePath() );
        else
        {
            //get the contents of a file
            QFile origFile ( file.absoluteFilePath() );
            if ( !origFile.open( QFile::ReadOnly ) )
            {
                qWarning() << "SandboxInstallJob::registerHelpFile:- Could not open file"
                           << "for registration" << file.absoluteFilePath();
                break;
            }

            //make a copy of the file contents and adjust links so that the md5sum is prepended
            QTextStream tsOrig( &origFile );
            contents = tsOrig.readAll();
            contents.replace( rx, QString("\\1") + package->md5Sum + "_" + QLatin1String("\\2\\3") );

            //create a new file and fill it with the modified contents
            QString newFilePath = file.absolutePath() + "/" + package->md5Sum + "_" + file.fileName();
            QFile newFile( newFilePath );
            if ( !newFile.open( QFile::WriteOnly ) )
            {
                qWarning() << "SandboxInstallJob::registerHelpFile:- Could not create file"
                           << newFile.fileName();
            }

            QTextStream tsNewFile( &newFile );
            tsNewFile << contents;
            origFile.close();
            newFile.close();

            //remove the original file and rename the new file
            origFile.remove();
            newFile.rename( file.absoluteFilePath());

            //make a link in the appropriate location within Qtopia::packagePath()/help
            //the links in this directory have the md5sum prepended to the file names
            QString linkName = newFilePath;
            linkName.remove( 0, destination.length() );
            linkName.prepend( Qtopia::packagePath() );
            createLink( file.absoluteFilePath(), linkName );
        }
    }
}

/*
   \internal
   Register i18n files in the \a i18nDir.
   This function will recurse and register files in any subdirectories.

   Implementation Note:
   Registration occurs by adding symlinks in Qtopia::packagePath()/i18n to
   the application's i18n files.

   The i18n files are found according to application name.
   For a downloaded application executable <app>, the QtopiaApplication::applicationName
   is <md5sum>_<app>, therefore the i18n symlinks
   must be prepended with <md5sum>_. (Typically the i18n files are of the form
   <app>.qm)
*/
void SandboxInstallJob::registerI18NFiles( const QString &i18nDir )
{
    QDir dir( i18nDir  );
    if ( !dir.exists() )
        qWarning() << "SandboxInstallJob::registerI18NFiles called with param, "
                      "i18nDir that is not a directory:" << i18nDir;
    QFileInfoList files = dir.entryInfoList( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
    foreach( QFileInfo file, files )
    {
        //recurse if subdirectory is found
        if ( file.isDir() )
        {
            registerI18NFiles( file.filePath() );
        }
        else
        {
           QString linkName = file.absolutePath() + QLatin1Char('/')
                            + package->md5Sum + "_" + file.fileName();
           linkName.remove( 0, destination.length() );
           linkName.prepend( Qtopia::packagePath() );
           createLink( file.absoluteFilePath(), linkName );
        }
    }
}

/*
  \internal
  Sets up the settings directory of the package.

  Implementation Note:
  The qtopia settings files are symlinked from the package's settings directory
  so that they can be found by the package.  These symlinks are made according
  to the organization(s) specified in the RestrictedOrganizations field in
  PackageManager.conf.

  Typically the Trolltech organization is restricted, indicating that
  3rd party applications cannot write to QSettings files using that organization.
  (reading those settings files is still allowed)
  Symlinks are then made to the Trolltech directory and Trolltech.conf from
  the package settings directory.

  A System integrator could add the restricted organization Acme to
  PackageManager.conf resulting in the Acme and Acme.conf symlinks being created.
  If Acme.conf (or the even the Acme directory) does not exist, a broken symlink
  is made.  This is not a cause of concern however, if Acme.conf is created during
  runtime then the link will be valid, if not there's no negative consequences.
  A resricted organization should only be added if its settings files need
  to be read by almost every application.
  */
bool SandboxInstallJob::setupSettingsFiles()
{
    QString settingsPath = destination + "/Settings";
    if ( !QDir::root().mkpath( settingsPath ) )
    {
        if ( reporter )
        {
            QString detailedError( "Unable to make directory: %s" );
            detailedError = detailedError.arg( settingsPath );
            reporter->reportError( QObject::tr("Error occurred during installation"),
                detailedError );
        }
        return false;
    }

    QSettings pkgManagerConf( "Trolltech", "PackageManager" );
    pkgManagerConf.beginGroup( "Configuration" );
    QStringList orgs =  pkgManagerConf.value( "RestrictedOrganizations" ).toStringList();
    foreach( QString org, orgs )
    {
        if ( QFile::exists( settingsPath + QLatin1Char('/') + org )
             || QFile::exists( settingsPath + QLatin1Char('/') + org + QLatin1String(".conf" )) )
        {
            if ( reporter )
            {
                QString detailedError( "Settings file/directory for (restricted) organisation, %1 "
                                       "aleady exists in %2" );
                detailedError = detailedError.arg(org).arg( settingsPath );
                reporter->reportError( QObject::tr( "Invalid package, contact package supplier" ),
                        detailedError );
            }
            return false;
        }

        //this code is derived from qsettings.cpp
        //so that we can obtain the location of qtopia's
        //settings files at user scope
        char *env = getenv("XDG_CONFIG_HOME");
        QString qtopiaSettingsPath;
        if (env == 0)
        {
            qtopiaSettingsPath = QDir::homePath();
            qtopiaSettingsPath += QLatin1Char('/');
            qtopiaSettingsPath += QLatin1String("Settings");
        } else if (*env == '/')
        {
            qtopiaSettingsPath = QLatin1String(env);
        }
        else
        {
            qtopiaSettingsPath = QDir::homePath();
            qtopiaSettingsPath += QLatin1Char('/');
            qtopiaSettingsPath += QLatin1String(env);
        }

        bool ok;
        //create link for organisation directory
        ok = createLink( qtopiaSettingsPath + QLatin1Char('/') + org,
                          settingsPath + QLatin1Char('/') + org );
        if ( !ok ) //note: error reporting already handled in createLink
            return false;

        //create link for organisation settings file
        ok = createLink( qtopiaSettingsPath + QLatin1Char('/')
                        + org + QLatin1String(".conf"),
                    settingsPath + QLatin1Char('/')
                        + org + QLatin1String(".conf") );
        if ( !ok )
            return false;
    }

    return true;
}

/*
  \internal
  \class SandboxUninstallJob
    \inpublicgroup QtPkgManagementModule
  \brief Uninstalls a package

  The SandboxUninstallJob is responsible for the particulars in deleting
  a sandboxed application, these include:

  \list
  \o Unregistering binaries from the SXE system
  \o Unregistering binaries from the Document system
  \o Deleteing the package's control file in the controls directory
  \o Removing symlinks from the bin and pics directories
     of Qtopia::packagePath
  \o Removing the package directory
  \o Rolling back the package's MAC sandbox rules
  \endlist

  The following calls should be used to uninstall a package:

  \code
  SandboxUninstallJob job( &pkg, m_installMedia, reporter );
  job.unregisterPackageFiles();
  job.dismantleSandbox();
  \endcode
*/

/*!
  \internal
  SandboxUninstallJob constructor
*/
SandboxUninstallJob::SandboxUninstallJob( const InstallControl::PackageInfo *pkg,
                                const QString &media, ErrorReporter *errorReporter)
    : package( pkg ),
      reporter ( errorReporter )
{
    packagePath = media.isEmpty() ? Qtopia::packagePath() : media + "/" + PACKAGE_DIRECTORY + "/";
    packagePath += package->md5Sum;
}

/*!
  \internal
   terminate all running instances of a package
*/
void SandboxUninstallJob::terminateApps() const
{
    QString path;
        path = Qtopia::packagePath() + package->md5Sum;
    qLog(Package) << "SandboxUninstallJob::terminateApps() terminating all applications "
                  << " starting with path: " << path;

    QDir proc( "/proc" );
    QStringList procDirs = proc.entryList( QDir::AllDirs, QDir::Name | QDir::Reversed );
    QString dir;
    QString exePath;
    bool ok;
    long pid;

    int passMax = 4;
    for ( int passNum=0; passNum < passMax; passNum++ )
    {
        foreach ( dir, procDirs )
        {
            pid = dir.toLong( &ok ); //only want processes
            if ( ok )
            {
                exePath = QFile::symLinkTarget( "/proc/" + dir + "/exe" );
                if ( exePath.startsWith(path) )
                {
                    QString app = package->md5Sum + "_" + exePath.mid(exePath.lastIndexOf("/")+1);

                    switch( passNum )
                    {
                        case 0:  //on first pass try to get applications to quit gracefully
                            QtopiaChannel::send(QLatin1String("QPE/Application/") + app, "quit()");
                            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                            break;
                        case 1: {//on the second pass kill the application using the launcher
                            QtopiaServiceRequest req( "Launcher","kill(QString)");
                            req << app;
                            req.send();
                            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                            break;
                        }
                        default: //on subsequent passes just kill the application
                                 //we do this for multiple passes to kill all instances
                                 //of the application, which may continually fork
                        kill( pid, 9 );
                        break;
                    }
                }
            }
        }
        //on the first and second pass wait a certain amount of time
        //because IPC is being used to terminate the application
        if ( passNum == 0 || passNum == 1 )
        {
            //in this case hard code the waiting time to 300ms
            QDateTime stopTime = QDateTime::currentDateTime().addMSecs(300);
            while( QDateTime::currentDateTime() < stopTime )
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }

}
/*!
  \internal
  Unregisters a package's binary files from the SXE and Document System.
*/
void SandboxUninstallJob::unregisterPackageFiles() const
{
#ifndef QT_NO_SXE
    QPackageRegistry::getInstance()->unregisterPackageBinaries( packagePath );
#endif

    QContentFilter appFilter = QContentFilter( QContentFilter::Role, "Application" );
    QContentSet set( appFilter);
    QContentList list = set.items();
    for ( int i = 0; i < list.count(); ++i )
    {
        if ( list.at( i ).executableName().startsWith( package->md5Sum ) )
            QContent::uninstall( list.at(i).id() );
    }
}

/*!
  \internal
  Deletes the sandboxed package and any files associated with it
  and rolls back any MAC kernel rules applied to the package's
  binaries
*/
void SandboxUninstallJob::dismantleSandbox() const
{
#ifndef QT_NO_SXE
    QStringList binaryPaths = getPackageBinaryPaths();
#endif
    clearMiscFiles( package );

    //Delete the package directory
    removePackage();

#ifndef QT_NO_SXE
    //roll back sandbox rules for each binary in the package
    for ( int i = 0; i < binaryPaths.count(); ++i )
       rollBackSandboxRule( binaryPaths[i] );
    SandboxInstallJob::reloadRules();
#endif
}

/*!
  \internal
  Rolls back the MAC sandbox rules for a single binary
*/
void SandboxUninstallJob::rollBackSandboxRule( const QString &binPath ) const
{
    static QString unsandboxCmd;
    static bool commandUnavailable = false;

    if ( !LidsUtils::isLidsEnabled() )
        return;

    if ( commandUnavailable )
        return;

    if ( unsandboxCmd.isEmpty() )
    {
        QStringList dirs = Qtopia::installPaths();
        for ( int d = 0; d < dirs.count(); ++d )
        {
            if ( QFile::exists( dirs[d] + "etc/sxe_qtopia/sxe_unsandbox" ))
            {
                unsandboxCmd = dirs[d] + "etc/sxe_qtopia/sxe_unsandbox %1 %2";
                break;
            }
        }
        if ( unsandboxCmd.isEmpty() )
        {
            commandUnavailable = true;
            qLog(Package) << "Unsandboxing script not found";
            return;
        }
    }
    QString sandboxDir;
    if ( binPath.startsWith( Qtopia::packagePath()) )
        sandboxDir = binPath.left( Qtopia::packagePath().length() + 32 );
    else
    {
        qWarning( "Packagemanager:- SandboxUninstallJob::rollBackSandboxrule trying to roll back "
                  "sandbox rule for non-downloaded application; binPath parameter "
                  "does not start with Qtopia::packagePath()" );
        return;
    }

    qLog(Package) << "Running unsandboxing script: "<< unsandboxCmd.arg( binPath ).arg(sandboxDir);
    ScriptRunner::runScript(unsandboxCmd.arg(binPath).arg(sandboxDir));
}

/*!
  \internal
  Obtains the paths to all the binaries in a directory and it's sub-directories
*/
QStringList SandboxUninstallJob::getPackageBinaryPaths( const QString &path) const
{
    QDir directory( path.isEmpty() ? packagePath : path );
    QStringList ret;

    QFileInfoList fileList = directory.entryInfoList();
    for ( int i = 0; i < fileList.count(); ++i )
    {
        QFileInfo fileInfo = fileList.at(i);

        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        // recurse to other directories
        if (fileInfo.isDir())
            ret += getPackageBinaryPaths( fileInfo.filePath() );

        if ( fileInfo.isFile() && fileInfo.isExecutable() )
            ret += fileInfo.absoluteFilePath();
    }
    return ret;
}

/*!
  \internal
  Removes the package directory and everything below it
*/
void SandboxUninstallJob::removePackage() const
{
    QDir destDir( packagePath );
    if ( !destDir.exists() )
        return;

    QString cmd( "rm -Rf " );
    cmd += packagePath;
    Q_ASSERT( !packagePath.isEmpty() && packagePath != "/" );
    qLog(Package) << "Removing destination by executing:" << cmd;
    ScriptRunner::runScript( cmd );
}

/*!
  \internal
  Removes files related to a \a package from the Qtopia::packagePath's
  bin, pics, control,help etc directories and from the tmp directory
*/
void SandboxUninstallJob::clearMiscFiles( const InstallControl::PackageInfo *pkg )
{
    QList<QDir> dirs;
    dirs << Qtopia::packagePath() + BIN;
    dirs << Qtopia::packagePath() + PICS;
    dirs << Qtopia::packagePath() + SOUNDS;
    dirs << Qtopia::packagePath() + HELP;
    dirs << Qtopia::packagePath() + CONTROLS;
    dirs << Qtopia::packagePath() + I18N;

    foreach( QDir dir, dirs )
        clearMiscFilesHelper( dir, pkg );

    //remove any left over package qcop-msg files in tmp
    QDir tempDir( Qtopia::tempDir() );
    QFileInfoList fileList = tempDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot );
    foreach( QFileInfo fi, fileList )
    {
        if( fi.fileName().startsWith(QString("qcop-msg-") + pkg->md5Sum ) )
            QFile::remove( fi.absoluteFilePath() );
    }
}

/*!
  \internal
  Removes all the files related to the package \a pkg from \a dir.
*/
void SandboxUninstallJob::clearMiscFilesHelper( const QDir &dir, const InstallControl::PackageInfo *pkg )
{
    if ( !dir.exists() )
    {
        qLog( Package ) << "SandboxUninstallJob::clearMiscFilesHelper():- dir does not exist: "
                        << dir.absolutePath();
        return;
    }

    QFileInfoList fileList= dir.entryInfoList( QDir::System | QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs );
    QFile f;
    foreach( QFileInfo fi, fileList )
    {
        if ( fi.isDir() )
        {
            clearMiscFilesHelper( QDir(fi.absoluteFilePath()), pkg );
        }

        if ( fi.completeBaseName().startsWith( pkg->md5Sum ) )
        {
            qLog(Package) << "SandboxUninstallJob::clearMiscFilesHelper removing file: " << fi.absoluteFilePath();
            f.setFileName( fi.absoluteFilePath() );
            f.remove();
        }
    }
}
