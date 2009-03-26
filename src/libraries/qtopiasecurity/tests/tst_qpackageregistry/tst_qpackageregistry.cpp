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
#include <qtopiaglobal.h>
#include <qpackageregistry.h>
#include <qtopianamespace.h>
#include <qtransportauth_qws.h>
#include <sys/mman.h>
#include <errno.h>
#include <QDir>
#include <QProcess>
#include <QTemporaryFile>

//TESTED_CLASS=QPackageRegistry
//TESTED_FILES=src/libraries/qtopiasecurity/qpackageregistry.cpp

/*
    Sub-class to allow testing of QPackageRegistry from the temp directory

    Simply overrides the sxeConfPath() method to allow a different store for
    the QPackageRegistry data
*/
class TmpQPackageRegistry : public QPackageRegistry
{
    Q_OBJECT
public:
    static TmpQPackageRegistry *theInstance;
    QString testTempPath() const;
    QString sxeConfPath() const;
private:
    mutable QString m_testTempPath;
    mutable QString m_sxeConfPath;
};

TmpQPackageRegistry *TmpQPackageRegistry::theInstance = NULL;

/*?
  Return the directory path where this test run will put files
*/
QString TmpQPackageRegistry::testTempPath() const
{
    if ( !m_testTempPath.isEmpty() )
        return m_testTempPath;

    // use the temp file just to get a unique path name then
    // destructor will auto remove it
    {
        QTemporaryFile tf(QString("%1/%2").arg(QDir::tempPath()).arg(metaObject()->className()));
        if ( tf.open() )
        {
            m_testTempPath = tf.fileName();
        }
        else
        {
            qFatal( "could not write in the temp directory: %s", qPrintable( QDir::tempPath() ));
        }
    }
    QDir::root().mkpath( m_testTempPath );
    qDebug() << "**********" << m_testTempPath;
    return m_testTempPath;
}

/*?
  \reimp
*/
QString TmpQPackageRegistry::sxeConfPath() const
{
    if ( !m_sxeConfPath.isEmpty() )
        return m_sxeConfPath;

    m_sxeConfPath = testTempPath() + "/etc";
    QDir::root().mkpath( m_sxeConfPath );
    return m_sxeConfPath;
}

/*
    Set of tests for the QPackageRegistry class.
    
    Tests the main functionality of the QPackageRegistry class
    which manages the database of SXE packages.
*/
class tst_QPackageRegistry : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void tst_registerBinary_PackageDownload();
private:
    void deletePath( const QFileInfo &p );
    static bool isByteStringInFile( const char *bytes, int len, const QString &fileName );

    QPackageRegistry *pkr;
    TmpQPackageRegistry *tmpPkr;

    QString keyfilePath;
    QString keyfileSeqPath;
    QString installsPath;
    QString manifestPath;
};

QTEST_APP_MAIN(tst_QPackageRegistry, QApplication)
#include "tst_qpackageregistry.moc"

/*?
  Setup before each unit test.  Gets an instance of the QPackageRegistry object
  and constructs the paths relative to the test directory.

  Also set up boot-strapped database files from the test data.
*/
void tst_QPackageRegistry::init()
{
    tmpPkr = new TmpQPackageRegistry();
    TmpQPackageRegistry::theInstance = tmpPkr;
    pkr = tmpPkr;

    QString etc = QtopiaUnitTest::baseDataPath() + "/etc";
    QDir etcDir( etc );
    QStringList etcFiles = etcDir.entryList( QDir::Files );
    foreach ( QString f, etcFiles )
    {
        qDebug() << "QFile::copy(" << etcDir.filePath( f ) << ","
            << ( pkr->sxeConfPath() + "/" + f ) << " )";
        QFile::copy( etcDir.filePath( f ), pkr->sxeConfPath() + "/" + f );
        QVERIFY( QFile::setPermissions( pkr->sxeConfPath() + "/" + f, QFile::ReadOwner|QFile::WriteOwner ) );
    }

    QTransportAuth::getInstance()->setKeyFilePath( pkr->sxeConfPath() );

    keyfilePath = pkr->sxeConfPath() + "/" QSXE_KEYFILE;
    keyfileSeqPath = keyfilePath + ".sequence";
    installsPath = pkr->sxeConfPath() + "/" + QPackageRegistry::installsFileName;
    manifestPath = pkr->sxeConfPath() + "/" + QPackageRegistry::manifestFileName;
}

/*?
  Delete a tree of files.  Calls itself recursively.
*/
void tst_QPackageRegistry::deletePath( const QFileInfo &p )
{
    qDebug() << "deletePath()" << p.filePath();
    if ( p.isDir() )
    {
        QDir dir( p.filePath() );
        qDebug() << "retrieving and deleting all entries from" << dir.path();
        QFileInfoList flist = dir.entryInfoList( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot );
        qDebug() << "got files:" << flist.count();
        foreach ( QFileInfo subFi, flist )
        {
            qDebug() << "fi:" << subFi.filePath();
            deletePath( subFi );
        }
        if ( !p.dir().rmdir( p.fileName() ))
            qWarning( "Could not rmdir %s/%s", qPrintable( p.dir().path() ),
                    qPrintable( p.fileName() ));
    }
    else
    {
        if ( !QFile::remove( p.filePath() ))
            qWarning( "Could not delete %s", qPrintable( p.filePath() ));
    }
}

/*?
  Tear-down after each unit test

  Removes all files under and including the testTempPath() directory
*/
void tst_QPackageRegistry::cleanup()
{
    qDebug() << "cleanup()";
    QDir testTemp( tmpPkr->testTempPath() );
    if ( testTemp.exists() )
    {
        if ( QProcess::systemEnvironment().indexOf( QRegExp( "^PKRGTEST_NOCLEANUP.*" )) > -1 )
            qDebug() << "NOCLEANUP specified - test files remain in" << tmpPkr->testTempPath();
        else
            deletePath( testTemp.path() );
    }
    delete pkr;
    pkr = 0;
    tmpPkr = 0;
}

/*?
  Return true if the \a bytes are in the file \a fileName, return false otherwise
*/
bool tst_QPackageRegistry::isByteStringInFile( const char *bytes, int len, const QString &fileName )
{
    QFile bf( fileName );
    qint64 fileSize = bf.size();;
    const char *match_start;
    if ( fileSize == 0 )
    {
        qWarning( "file %s was zero length", qPrintable(fileName) );
        return false;
    }
    if ( bytes == NULL || len == 0 )
    {
        qWarning( "bogus values in isByteStringInFile() - bytes %p, len %d, filename %s",
                bytes, len, qPrintable( fileName ));
        return false;
    }
    if ( !bf.open( QIODevice::ReadOnly ))
    {
        qWarning( "Could not read from %s", qPrintable( fileName ));
        return false;
    }
    const char *fileBytes = (char*)::mmap( 0, bf.size(), PROT_READ, MAP_PRIVATE, bf.handle(), 0 );
    if ( fileBytes == MAP_FAILED )
    {
        qWarning( "Could not mmap %s : %s", qPrintable( fileName ), strerror( errno ));
        return false;
    }
    match_start = fileBytes;
    const char *eof = fileBytes + fileSize;
    while (( match_start + len ) < eof )
    {
        if ( ::memcmp( match_start, bytes, len ) == 0 )
        {
            return true;
        }
        match_start++;
    }
    return false;
}

/*?
  Test registration of an SXE binary in the way a downloaded package
  would work.
*/
void tst_QPackageRegistry::tst_registerBinary_PackageDownload()
{
    char comparisonNullKey[ QSXE_KEY_LEN ];
    SxeProgramInfo thePackage;

    // These are the fields required for a downloaded package
    thePackage.fileName         = "tst_registerBinary_PackageDownload";   // eg calculator, bomber
    thePackage.relPath          = "packages/bin";                         // eg bin, packages/bin
    thePackage.runRoot          = tmpPkr->testTempPath();    // eg /opt/Qtopia.rom, /opt/Qtopia.user
    thePackage.domain           = "window,test_domain";                   // security domains, csv

    QString src = QtopiaUnitTest::baseDataPath() + "/compiled_unkeyed_sxe_binary";

    QVERIFY( QDir::root().mkpath( thePackage.runRoot + "/" + thePackage.relPath ));
    QVERIFY( QFile::copy( src, thePackage.absolutePath() ));
    QVERIFY( QFile::setPermissions( thePackage.absolutePath(), QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) );
    QVERIFY( thePackage.isValid() );

    // leave this as null, since packages are installed at run time, not "qbuild image" time
    // thePackage.installRoot; // eg $HOME/build/qtopia/42-phone/image

    // set these to sentinel values
    ::memset( thePackage.key, '\0', QSXE_KEY_LEN ); // key
    thePackage.id = 254;                            // program identity

    ::memset( comparisonNullKey, '\0', QSXE_KEY_LEN );

    pkr->registerBinary( thePackage );

    // Key should now contain something other than all zeros
    // (well strictly speaking its within the realm of possibility but I'd be
    // ordering a new /dev/urandom from Mr Torvalds if it happened to me...
    QVERIFY( ::memcmp( thePackage.key, comparisonNullKey, QSXE_KEY_LEN ) != 0 );

    // Program id should now not equal the sentinel, and it should probably equal 1
    QVERIFY( thePackage.id != 254 );
    QVERIFY( thePackage.id == 1 );
    QVERIFY( isByteStringInFile( thePackage.key, QSXE_KEY_LEN, thePackage.absolutePath() ));
}
