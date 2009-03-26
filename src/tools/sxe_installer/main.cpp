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

#include <QFileInfo>
#include <QTextStream>
#include <QDir>

#include <qtopiasxe.h>
#include <qpackageregistry.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include <qdebug.h>
#include <qtopialog.h>

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 1024
#endif

class SafeExecInstaller
{
public:
    SafeExecInstaller();
    ~SafeExecInstaller() {}
    int runScan();
    void setInstallRoot( const QString &s ) { progInfo.installRoot = s; }
    void setTarget( const QString & );
    void setDomainString( const QString &s ) { progInfo.domain = s; }
    void setRunDir( const QString &s ) { progInfo.runRoot = s; }
    void usage( const QString &s = QString() );
    QString domainString() { return progInfo.domain; }
    QString fileName(){ return progInfo.fileName; }
private:
    SxeProgramInfo progInfo;
};

SafeExecInstaller::SafeExecInstaller()
{
}

/*!
  \internal
  Set the target information for the install - this is the path
  to the target, relative to installDir, and its target name.

    eg:  "/bin/camera" -> "/bin/" - "camera"

  even though if quicklaunched it might be libcamera.so

  Note that the installRoot must be set before calling this
  method, so the install can be checked.
*/
void SafeExecInstaller::setTarget( const QString &s )
{
    Q_ASSERT( !progInfo.installRoot.isEmpty() );
    QString path;
    QString imagepath;
    if (s.startsWith(progInfo.installRoot)) {
        path = s;
        imagepath = path;
        imagepath.replace(progInfo.installRoot, "");
    } else {
        path = progInfo.installRoot + "/" + s;
        imagepath = s;
    }
    path.replace("//", "/");
    QFileInfo fi( path );
    progInfo.fileName = fi.fileName();
    progInfo.relPath = QFileInfo( imagepath ).path();
    while( progInfo.relPath.startsWith( "/" ))
        progInfo.relPath.remove( 0, 1 );
    if ( !fi.exists() )
        usage(QString("ERROR: %1 does not exist").arg(fi.filePath()));
}

/*!
  \internal

  The build system - $QTOPIA_DEPOT_PATH/src/build/installs.prf is
  responsible for passing in the correct arguments in the correct order:

    domain.commands=$$QPEDIR/bin/sxe_installer $(PREFIX) $(INSTALL_ROOT) $${target.path}/$$TARGET "$${pkg.domain}"
*/
void SafeExecInstaller::usage( const QString &msg )
{
    if ( !msg.isEmpty() )
        qWarning() << msg.toLocal8Bit().constData() << endl;

    qWarning() << "Usage:  sxe_installer PREFIX IMAGE file domain" << endl << endl
               << "  - file is an absolute path to a file in the image or a relative path." << endl
               << "    eg. /path/to/image/bin/qpe, /bin/qpe, bin/qpe are all valid." << endl
               << "  - domain must be equal to 'trusted'. Untrusted applications can" << endl
               << "    only be installed by the package manager.";
    ::exit(1);
}

int SafeExecInstaller::runScan()
{
    if ( progInfo.installRoot.isEmpty() )
        usage( "install root not supplied" );
    if ( !QDir( progInfo.installRoot ).exists() )
        usage( QString( "install root %1 not a directory" ).arg( progInfo.installRoot ));
    if ( progInfo.domain.isEmpty() )
        progInfo.domain = "none";
    QPackageRegistry *p = QPackageRegistry::getInstance();
    int result = p->bootstrap( progInfo.installRoot );
    if ( result != 0 )
        return 1;
    p->registerBinary( progInfo );
    return 0;
}

int main( int argc, char** argv )
{
    if ( QSysInfo::ByteOrder == QSysInfo::BigEndian )
        qFatal( "Host tool sxe_installer does not currently support\n"
                "writing SXE database files on Big Endian architectures\n" );
    SafeExecInstaller se;
    if ( argc != 5 )
        se.usage("ERROR: Wrong number of arguments");
    se.setRunDir( argv[1] );
    se.setInstallRoot( argv[2] );
    se.setTarget( argv[3] );
    se.setDomainString( argv[4] );

    if ( se.fileName() != "qpe" && se.domainString() != "trusted" ) {
        qFatal( "ERROR: %s does not declare the domain: trusted\n"
                "All pre-installed programs must declare the trusted domain "
                "in their project(.pro) file", argv[3] );
    } else if ( se.fileName() == "qpe" && se.domainString() != "qpe" ) {
        qFatal( "ERROR: qpe must declare the domain: qpe\n"
                "inside its project(.pro) file" );
    }

    return se.runScan();
}
