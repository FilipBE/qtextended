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

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <qtopianamespace.h>
#include <qdebug.h>

#include <QCoreApplication>
#include <QProcess>

#include <sys/types.h>
#include <unistd.h>
#include <iostream>

//TESTED_CLASS=Qtopia
//TESTED_FILES=src/libraries/qtopiabase/qtopianamespace.cpp

/*
  Tests the main functionality of the Qt Extended namespace methods
  which provide miscellaneous functionality.
*/
class tst_QtopiaNamespace : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void tst_sandboxDir();
    void tst_launchPseudoPackage();
    void tst_respondWithPackagePath();
private:
    void setupPseudoPackage();
    void teardownPseudoPackage();
    void removeDir(QDir dir);
    QString packageSymlinkPath;
    QString packageSymlinkDir;
    QString packageDirPath;
    QString packagePath;
    QString dummyPackageRoot;
    QString pseudoHome;
    QString prefix;
};

QTEST_MAIN(tst_QtopiaNamespace);


/*?
  Setup before each unit test.
*/
void tst_QtopiaNamespace::init()
{
}

/*?
  Teardown after each unit test
*/
void tst_QtopiaNamespace::cleanup()
{
}

QString withTrailingSlash(QString const &path) {
    if (!path.endsWith(QDir::separator()))
        return path + QDir::separator();
    else
        return path;
}

/*?
  Helper method (non-test)

  Create a pseudo-package by copying this binary into a suitable
  path structure under the /tmp directory.

  Save relevant paths and so on into member vars for testing later.
*/
void tst_QtopiaNamespace::setupPseudoPackage()
{
    // create a pseudo package directory
    prefix = QString( "packages_%1" ).arg( getpid() );
    pseudoHome = withTrailingSlash(QDir::tempPath()) + prefix + QDir::homePath();
    packageSymlinkDir = prefix + Qtopia::packagePath() + "bin/";
    dummyPackageRoot = prefix + Qtopia::packagePath() + "a2345adaf3fdefeac53754feafffffff";
    packageDirPath = dummyPackageRoot + "/bin";

    if ( !QDir::temp().exists( packageDirPath ))
        QDir::temp().mkpath( packageDirPath );
    packageDirPath.prepend( withTrailingSlash(QDir::tempPath()) );
    dummyPackageRoot.prepend( withTrailingSlash(QDir::tempPath()) );

    if ( !QDir::temp().exists( packageSymlinkDir ))
        QDir::temp().mkpath( packageSymlinkDir );
    packageSymlinkDir.prepend( withTrailingSlash(QDir::tempPath()) );

    // Get the name of the app
    QString appPath = QCoreApplication::arguments().at(0);
    appPath = QFileInfo( appPath ).absoluteFilePath();
    int slash = appPath.lastIndexOf( QDir::separator() );
    if ( slash == -1 )
        qFatal( "application path %s makes no sense (\"%c\" missing)",
                qPrintable( appPath ), QDir::separator().toAscii() );
    QString appName = appPath.mid( slash + 1 );

    // copy and symlink ourselves into it
    packagePath = packageDirPath + "/" + appName;

    // maybe a previous test died before cleaning up, if so it must point
    // to the right thing so lets re-use it
    if ( !QFile::exists( packagePath ))
    {
        if ( !QFile::copy( appPath, packagePath ))
            qFatal( "Could not copy %s -> %s", qPrintable( appPath ),
                    qPrintable( packagePath ));
    }
    packageSymlinkPath = packageSymlinkDir + "/" + appName;
    if ( !QFile::exists( packageSymlinkPath ))
    {
        if ( !QFile::link( packagePath, packageSymlinkPath ))
            qWarning( "Could not create link: %s -> %s", qPrintable( packageSymlinkPath ),
                    qPrintable( packagePath ));
    }
}

/*?
  Helper method (non-test)

  Get rid of all the temporary files created by the pseudo-package
*/
void tst_QtopiaNamespace::teardownPseudoPackage()
{
    if ( QDir::temp().exists( prefix ))
        removeDir(QDir::temp().filePath(prefix));
}


/*?
  Helper method (non-test)

  Delete a directory and all it's contents
*/
void tst_QtopiaNamespace::removeDir(QDir dir)
{
    QFileInfoList fileList= dir.entryInfoList( QDir::System | QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs );
    QFile f;
    foreach( QFileInfo fi, fileList )
    {
        if (fi.isDir()) {
            removeDir( QDir(fi.absoluteFilePath()));
        }
        else {
            f.setFileName(fi.absoluteFilePath());
            if(!f.remove())
                qWarning() << "Could not delete " << fi.absoluteFilePath() << " during test cleanup";
        }
    }
    if (!dir.rmdir(dir.absolutePath()))
        qWarning() << "Could not remove " << dir.absolutePath() << " during test cleanup";
}

/*?
  In a non-sandboxed method the sandboxDir() method should return an empty string
*/
void tst_QtopiaNamespace::tst_sandboxDir()
{
    QString sb = Qtopia::sandboxDir();
    QVERIFY( sb.isEmpty() );
}

/*?
  Test the operation of the sandboxDir() method by relaunching this test binary
  from a new path which satisfies the rules for the sandbox directory structure

  The new process should echo a value of the dummyPackageRoot as determined by
  the setupPseudoPackage() method above.
*/
void tst_QtopiaNamespace::tst_launchPseudoPackage()
{
    setupPseudoPackage();
    QVERIFY( !packageDirPath.isEmpty() );
    QVERIFY( !packagePath.isEmpty() );

    QProcess pkgProc;
    QStringList environ = QProcess::systemEnvironment();
    QRegExp homeExpr("HOME=.*");
    QString homeEnv=QString("HOME=") + pseudoHome;
    int idx;
    if ( (idx = environ.indexOf(homeExpr)) != -1)
        environ.replace(idx, homeEnv);
    else
        environ.append(homeEnv);
    pkgProc.setEnvironment(environ);

    QStringList args;
    args << "-qws" << "tst_respondWithPackagePath";
    pkgProc.start( packageSymlinkPath, args );

    QVERIFY( pkgProc.waitForStarted() );
    QVERIFY( pkgProc.waitForFinished( 500000 ) );

    QTextStream ts( &pkgProc );
    QString output = "\n================== std output ==================\n";
    QByteArray out = pkgProc.readAllStandardOutput();
    output.append( out );
    output += "================== err output ==================\n";
    out = pkgProc.readAllStandardError();
    output.append( out );
    output += "================== output ==================\n";
    //qLog(Autotest) << output;
    QString expectString = QString( "@@@ sandbox dir: %1 @@@" ).arg(
#ifdef QT_NO_SXE
        QString()
#else
        withTrailingSlash(dummyPackageRoot)
#endif
    );
    //qLog(Autotest) << expectString;

    QVERIFY2( output.indexOf( expectString ) != -1, qPrintable(QString(
        "Output string did not refer to sandbox dir as expected!\n"
        "output: %1\n"
        "expectString: %2").arg(output).arg(expectString))
    );
    teardownPseudoPackage();
}

/*?
  If running as the relaunched copy then echo out what this process thinks is the
  sandboxDir() for collection by the real test.  Otherwise, sandboxDir() should
  return empty as per the above test
*/
void tst_QtopiaNamespace::tst_respondWithPackagePath()
{
    QString sb = Qtopia::sandboxDir();
    QString appPath = QCoreApplication::arguments().at(0);
    if ( appPath.startsWith( "/tmp" ))
        std::cout << "@@@ sandbox dir: " << qPrintable( sb ) << " @@@" << std::endl;
    else
        QVERIFY( sb.isEmpty() );
}

#include "tst_qtopianamespace.moc"

