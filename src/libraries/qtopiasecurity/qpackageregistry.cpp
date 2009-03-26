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

#include "qpackageregistry.h"
#include "keyfiler_p.h"
#include "qsxepolicy.h"

#include <qtopianamespace.h>
#ifndef SXE_INSTALLER
#include <qstorage.h>
#endif
#include <qtopialog.h>
#include <qtopiasxe.h>

#ifdef Q_WS_QWS
#include <qtopiaipcenvelope.h>
#endif

#include <QFile>
#include <QTextStream>

#include <unistd.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 1024
#endif

const QString QPackageRegistry::installsFileName = "installs";
const QString QPackageRegistry::manifestFileName = "manifest";
const QString QPackageRegistry::policyFileName = "sxe.policy";
const QString QPackageRegistry::profilesFileName = "sxe.profiles";
const QString QPackageRegistry::binInstallPath = "/bin/";
const QString QPackageRegistry::qlLibInstallPath = "/plugins/application/";
const QString QPackageRegistry::packageDirectory = "packages";
const QString QPackageRegistry::procLidsKeysPath = "/proc/lids/keys";
const int QPackageRegistry::maxProgId = 2;

////////////////////////////////////////////////////////////////////////
/////
/////  Data transfer objects
/////

/**
  \internal
  \class ProgramInfo
    \inpublicgroup QtPkgManagementModule

  \ingroup installedsoftware
  Track data records from the manifest file.  This file contains one
  record for each installed binary in a program.
*/
struct ProgramInfo
{
    union {
        char data[ sizeof( IdBlock )];
        IdBlock values;
    };
    char *fullpath;
    char *fileSystem;
    off_t pos;
    AuthRecord *ar;
};

////////////////////////////////////////////////////////////////////////
/////
/////  QPackageRegistry implementation
/////

/*!
  \class QPackageRegistry
    \inpublicgroup QtPkgManagementModule

  \brief The QPackageRegistry class provides a mechanism for managing software installed on the qtopia device

  QPackageRegistry provides several methods for managing the database
  files which track installed software as part of the \l {Safe Execution Environment (SXE)}.
  \ingroup installedsoftware

  When downloaded software is installed the
  \l {SXE - Installer/Package Manager Overview}{SXE Software Installer}
  calls methods here, to register the software with SXE and thus enable SXE for
  those applications, and to allow later uninstallation.

  \section1 Program Management

  All programs installed on the device are assigned a \bold{program id}.  A program
  will often consist of a number of binaries providing related functionality,
  for example a game may contain a small server, a gui client and a map editor
  which make up the package.  In Qtopia, binaries which provide similar
  functionality and require the same security domain, are grouped into one
  program id.

  The program id is always written into the binary at install time, along
  with a secret key.

  For \underline {Qt Extended programs} when the image is created the sxe_installer is run which
  calls the packageRegistry in bootstrap mode to setup the database files and
  install the program id, and keys for those Qt Extended programs.  This occurs prior
  to shipping & delivery of the Qt Extended device.

  For \underline {downloaded programs} the package installer will call the registerBinary()
  method to install the program id.  This occurs during normal runtime of the
  Qt Extended device.

  When the program id and key is created and installed, these are recorded
  in the \bold{Keyfile}, and the \bold {Sequence} file is used to track issue of new program ids.

  Note that rekeying and key time-outs are no longer used.  This mechanism has
  been superseded by the /proc/lids/keys method.

  See \l {Key File Change Notes} for important details on these changes.

  \section1 Binary Management

    Installed binaries by inode & device id are mapped
    to their program id in the \bold {Manifest} file.  Many binaries can map to a
    single program id.

    This format is designed for fast lookup of programs.  Given a binary's
    path, eg from /proc/<pid>/exe, the inode and device id could be obtained,
    and the correct program id read from this file.

    The manifest file is only accessed by the packagemanager
    and the server (qpe).  Its contents are not as sensitive
    as the keyfile, but should not be accessible by other
    processes.

    If the string version of the paths for a binary are
    needed based on install id, they can be looked up in
    the \bold {Installs} file.  This is required mainly for uninstallation
    of packages, but could be used for reporting and other purposes.

    The manifest and installs files are accessed by the packagemanager
    and the server (qpe).  Their contents are not as sensitive
    as the keyfile, but should not be accessible by other
    processes.

  \section1 SXE Package Database Files

  As discussed in the previous two sections, QPackageRegistry tracks several
  files which comprise the SXE database.  This table summarizes these
  files and their format:
  \table
    \header
        \o Keyfile
        \o Sequence
        \o Manifest
        \o Installs
    \row
        \o $QPEDIR/etc/keyfile
        \o $QPEDIR/etc/keyfile.sequence
        \o $QPEDIR/etc/manifest
        \o $QPEDIR/etc/installs
    \row
        \o {4, 1}
            Pseudo-code for binary file data record formats: See
            \c $QT_SOURCE_PATH/src/gui/embedded/qtransportauthdefs_qws.h for
            concrete details.  \c usr_key_entry is from the kernel file
            include/linux/lidsext.h and is duplicated in the qtransportauthdefs_qws.h
    \row
        \o
            \code
                struct usr_key_entry
                    char key[QSXE_KEY_LEN];
                    ino_t ino;              // typically 4 bytes
                    dev_t dev;              // typically 8 bytes
            \endcode
        \o
            \code
                struct AuthRecord
                  16 byte key
                  program id
                  change time   // deprecated
            \endcode
        \o
            \code
                struct IdBlock
                    quint64 inode
                    quint64 device
                    unsigned char progId
                    unsigned short installId
                    unsigned int keyOffset
                    qint64 install_time
            \endcode
        \o
            \code
                install record
                    install id
                    ":"
                    path
            \endcode
    \row
        \o
            Contains the mapping between keys and files on disk.

            When the kernel contains the LIDS MAC Qt Extended patch the key file
            is implemented as the kernel /proc pseudo-file /proc/lids/keys.

            In this case this keyfile on disk storage is ignored.

        \o

            This file is used simply to ensure a sequence of program id numbers.

            For historical and compatibility reasons, the first key issued with a number and the time
            are also recorded in this file.

        \o
            Map Installed binaries by inode & device id
            to their program id - many binaries can map to a
            single program id.  Records an install id for each binary.
        \o
            Map install id to installed path of the binary.

            This is required mainly for uninstallation of packages.

  \endtable

  To query these files after a \c {qbuild image} use the tool \c {$QTOPIA_SOURCE_PATH/scripts/dumpsec.pl}

  \sa {RegistryFile}

  \section1 Key File Change Notes

      Both the proc pseudo file and the disk file have the same format, and
      if the /proc/lids/keys file is not present, disk files are used instead.

      The \c {struct usr_key_entry} is defined in SXE kernel patch file
      include/linux/lidsif.h

      \c {struct usr_key_entry} is the (new) data record format for the key file
      superseding the format used in Qtopia 4.1

      The key file is (now) either \c {/proc/lids/keys} (and the per-process
      keys in \c {/proc/<pid>/lids_key} ) OR for desktop/development ONLY (not
      for production) it is \c {$QPEDIR/etc/keyfile}

      The key file maps keys to files.

      File are identified by inode and device numbers, not paths.

      (See the "installs" file for path to inode/device mapping)

      Disk keyfiles are to allow desktop development and testing only, since
      on desktop development machines the kernel patch will typically not be installed.

      \bold{It is not supported on the device to operate without /proc/lids/keys}

      Where binary files are writeable (all downloaded
              programs must be writeable) the key is embedded in
      the binary at install time.

      In the development and testing case, the embedded key is used by the
      binary to authenticate itself to the server.

      When the kernel contains the LIDS MAC Qt Extended patch the binary
      reads its key from the /proc pseudo-file /proc/self/lids_key.

*/


/*!
  \enum QPackageRegistry::RegistryFile
  This enum specifies one of the registry files
  \value Keyfile
         keyfile
  \value Manifest
         manifest
         keyfile
  \value KeyfileSequence
         key file
  \value Installs
         installs file
  \value Policy
         policy file
*/

/*!
  \variable QPackageRegistry::installsFileName
  returns the QString name of the \i {installs file}, part of the SXE database
  \sa sxeConfPath()
*/


/*!
  \variable QPackageRegistry::manifestFileName
  returns the QString name of the \i {manifest file}, part of the SXE database
  \sa sxeConfPath()
*/

/*!
  \variable QPackageRegistry::profilesFileName
  returns the QString name of the \i {profiles file}, part of the SXE database
  \sa sxeConfPath()
*/

/*!
  \variable QPackageRegistry::binInstallPath
  returns the  QString name of the \bold relative \i {binary path}, where SXE controlled
  executables are installed.  Concatenate this with paths from installPaths() to find
  full paths which may contain binaries.

  \sa Qtopia::installPaths()
*/

/*!
  \variable QPackageRegistry::qlLibInstallPath
  returns the  QString name of the \bold relative \i {quicklaunch binary path}, where SXE controlled
  quicklaunched executables are installed.  Concatenate this with paths from installPaths() to find
  full paths which may contain \c {.so} files.

  \sa Qtopia::installPaths()
*/

/*!
  Construct new QPackageRegistry object

  Do not use this method, instead call the instance method getInstance()

  \sa getInstance()
  */
QPackageRegistry::QPackageRegistry()
    : manifestFd( -1 )
    , installFd( -1 )
    , curInstallId( 0 )
    , installStrm( NULL )
    , connectionCount( 0 )
#ifndef SXE_INSTALLER
    , storageInfo( 0 )
#endif
    , manifestLockFile( 0 )
{
#ifndef SXE_INSTALLER
    storageInfo = QStorageMetaInfo::instance();
#endif
}

/*!
  Destroy the QPackageRegistry object
*/
QPackageRegistry::~QPackageRegistry()
{
    clearInstalls();
    if ( manifestFd != -1 )
        ::close( manifestFd );
    if ( installStrm != NULL )
        ::fclose( installStrm );
#ifndef SXE_INSTALLER
#endif
}

Q_GLOBAL_STATIC(QPackageRegistry, packageRegistryInstance );

/*!
  Return a singleton instance of the QPackageRegistry object for
  this process.
*/
QPackageRegistry *QPackageRegistry::getInstance()
{
    return packageRegistryInstance();
}

/*!
  Return the authoritative path for where the SXE database files, such as
  installs, and manifest, are stored.

  The directory must be writeable.

  Current implementation is simply Qtopia::qtopiaDir() + "etc"
*/
QString QPackageRegistry::sxeConfPath() const
{
    static QString theConfPath;
    if ( theConfPath.isEmpty() ) {
        QSettings storage( QLatin1String("Trolltech"), QLatin1String("Storage") );
        if ( storage.childGroups().contains( "SxeDatabase" ) ) {
            storage.beginGroup( "SxeDatabase" );
            theConfPath = storage.value( QLatin1String("Path") ).toString();
            storage.endGroup();
        } else {
            theConfPath = getQtopiaDir() + "etc";
        }
    }
    return theConfPath;
}

/*!
  Obtain a Qtopia::lockFile() on the manifest file.  This represents locking
  the QPackageRegistry database system to allow write operations to be done
  by this process.

  If successfully locked, the method sets \a success to true; otherwise it
  sets it to false.

  Call this method to obtain the lock before performing any write operations
  to the SXE database system.

  This should be matched by a call to unlockManifest()

  \sa unlockManifest()
*/
void QPackageRegistry::lockManifest( bool &success )
{
     success = false;
     if ( manifestLockFile == 0 )  // not thread safe
     {
         manifestLockFile = new QFile( sxeConfPath() + "/" + manifestFileName + ".lock" );
         if ( !manifestLockFile->open( QIODevice::ReadWrite ))
             return;
         if ( !Qtopia::lockFile( *manifestLockFile, Qtopia::LockBlock ))
             qFatal( "Could not lock manifest for SXE install" );
     }
    success = true;
}

/*!
  Release the lock on the manifest file.

  \sa lockManifest()
*/
void QPackageRegistry::unlockManifest()
{
     if ( manifestLockFile != 0 )
     {
         Qtopia::unlockFile( *manifestLockFile );
         delete manifestLockFile;
         manifestLockFile = 0;
     }
}

#ifndef SXE_INSTALLER
/*!
  Return true if the \a connectionData connection is from an installed program, ie
  not one which is part of the base software.

  The program is installed software if it is installed in Qtopia::packagePath()

  \deprecated
*/
bool QPackageRegistry::isInstalledProgram( QTransportAuth::Data &connectionData )
{
    QString packageDirectory(Qtopia::packagePath());
    QHash<unsigned short, ProgramInfo*>::iterator instIter = installs.begin();
    while ( instIter != installs.end() )
    {
        ProgramInfo *pi = instIter.value();
        Q_ASSERT( pi );
        Q_ASSERT( pi->fullpath );
        Q_ASSERT( pi->ar );
        if ( pi->ar->auth.progId == connectionData.progId )
        {
            qLog(SXE) << "Checking install status for program" << pi->ar->auth.progId << "at" << pi->fullpath
                << ( QString( pi->fullpath ).startsWith( packageDirectory ) ? "installed" : "base" );
            if ( QString( pi->fullpath ).startsWith( packageDirectory ))
                return true;
            else
                return false;
        }
        ++instIter;
    }
    return true;
}
#endif

bool QPackageRegistry::openSystemFiles()
{
    bool ok;
    lockManifest( ok );
    if ( !ok )
    {
        qWarning( "Could not lock manifest database" );
        return false;
    }
    if ( manifestFd == -1 && installFd == -1 )
    {
        QStringList sxeSystemFiles;
        sxeSystemFiles << QString( sxeConfPath() + "/" + manifestFileName );
        sxeSystemFiles << QString( sxeConfPath() + "/" + installsFileName );
        for ( int f = 0; f < 2; f++ )
        {
            int rfd = ::open( sxeSystemFiles[f].toLocal8Bit(), O_RDWR );
            if ( rfd < 0 )
            {
                // that's OK, we might be using this to see if bootstrap is required
                if ( errno != ENOENT )
                    qWarning( "open %s: %s", qPrintable(sxeSystemFiles[f]), strerror( errno ));
                return false;
            }
            if ( f == 0 )
                manifestFd = rfd;
            else
                installFd = rfd;
        }
    }
    if ( installStrm == NULL )
    {
        installStrm = fdopen( installFd, "w+" );
        if ( installStrm == NULL )
            qWarning( "stream install: %s", strerror( errno ));
    }
    return true;
}

void QPackageRegistry::closeSystemFiles()
{
    if ( manifestFd == -1 || installFd == -1 )
        return;
    if (installStrm != NULL)
    {
        fclose( installStrm );
        installStrm = NULL;
        installFd = -1;
    }
    close( manifestFd );
    close( installFd );
    manifestFd = -1;
    installFd = -1;
    installStrm = NULL;
    unlockManifest();
}

void QPackageRegistry::clearInstalls()
{
    QHash<unsigned char,AuthRecord*>::iterator progIter = progDict.begin();
    while ( progIter != progDict.end() )
    {
        AuthRecord * &ar = progIter.value();
        if ( ar )
        {
            delete ar;
            ar = 0;
        }
        ++progIter;
    }
    progDict.clear();
    profileDict.clear();
    installsByPath.clear();
    QHash<unsigned short, ProgramInfo*>::iterator instIter = installs.begin();
    while ( instIter != installs.end() )
    {
        ProgramInfo *pi = instIter.value();
        if ( pi == NULL ) continue;
        if ( pi->fullpath ) free( pi->fullpath );
        if ( pi->fileSystem ) free( pi->fileSystem );
        free( pi );
        ++instIter;
    }
    installs.clear();
}

/**
  \internal
  Load the registry with the data from the manifest file.
*/
void QPackageRegistry::readManifest()
{
    ProgramInfo *pi;
    ProgramInfo piBuf;
    off_t filpos;
    // always do it in this order to prevent deadlock
    struct stat manStat;
    Q_ASSERT( manifestFd != -1 );
    fstat( manifestFd, &manStat );
    clearInstalls();
    curInstallId = 0;
    initialiseInstallDicts();
    if ( manStat.st_size == 0 ) // degenerate initial case
        return;
    ::memset( &piBuf, '\0', sizeof( piBuf ));
    filpos = 0;
    while ( read( manifestFd, piBuf.data, sizeof( IdBlock )) > 0 )
    {
        pi = reinterpret_cast<ProgramInfo*>( malloc( sizeof( piBuf )));
        Q_CHECK_PTR( pi );
        ::memcpy( pi, &piBuf, sizeof piBuf );
        pi->pos = filpos;
        installs.insert( pi->values.installId, pi );
        if ( pi->values.installId >= curInstallId )
            curInstallId = pi->values.installId + 1;
        if ( progDict.contains( pi->values.progId ))
            pi->ar = progDict.value( pi->values.progId );
        else
            qWarning( "####### manifest file lists prog id %d without matching key",
                    pi->values.progId );
        filpos += sizeof( struct IdBlock );
    }
    qLog(SXE) << "\tread" << curInstallId << "manifest entries";
}

/*!
  Load the installs file, updating the registry with the path names
  of installed programs
*/
void QPackageRegistry::readInstalls()
{
    char pathBuf[ MAX_PATH_LEN ];
    unsigned int lineNo = 0;
    unsigned short installId;
    char *pathPtr = 0;
    ProgramInfo *pi;
    char pad;
    Q_ASSERT( installStrm != NULL );
    ::rewind( installStrm );
    while ( ::fread( &installId, sizeof( installId ), 1, installStrm ))
    {
        lineNo++;
        pad = ::fgetc( installStrm );
        if ( !pad == ':' )
            qWarning( "expected \":\" reading installs - line %d", lineNo );
        fgets( pathBuf, MAX_PATH_LEN, installStrm );
        int lf = strlen( pathBuf );
        lf--;
        if ( pathBuf[lf] == '\n' ) pathBuf[lf] = '\0';
        if ( !installs.contains( installId ))
        {
            qWarning( "manifest file corrupt - does not list install id %d from line %d",
                    installId, lineNo );
        }
        else
        {
            pi = installs[installId];
            if ( pi->fullpath == 0 )
            {
                pathPtr = strndup( pathBuf, MAX_PATH_LEN );
                Q_CHECK_PTR( pathPtr );
                pi->fullpath = pathPtr;
                QString pathPtrs( pathPtr );

                installsByPath.insert( pi->fullpath, pi );
#ifndef SXE_INSTALLER
                const QFileSystem *fs = 0;
                fs = storageInfo->fileSystemOf( pathPtrs );
                if ( fs )
                    pi->fileSystem = strndup( fs->path().toLocal8Bit(), MAX_PATH_LEN );
#endif
            }
            // qLog(SXE) << "\tinstall #" << installId << pi->fullpath;
        }
    }
    qLog(SXE) << "\tread" << lineNo << "installs entries";
}

/**
  Add the file which has just had its SXE parameters calculated, into
  the SXE control files.  The file descriptor is passed so that fstat
  can be used instead of stat, for speed
*/
void QPackageRegistry::addToManifest( const SxeProgramInfo &spi, int sfd )
{
    ProgramInfo pi;
    ProgramInfo *toWrite;
    struct stat st;
    QString runPath;
    if ( manifestFd == -1 )
    {
        qWarning ("\tManifest file not open, ignoring add request\n");
        return;
    }
    if ( installStrm == NULL )
    {
        qWarning ("\tInstall stream not open, ignoring add request\n");
        return;
    }
    if ( fstat( sfd, &st ) == -1 ) goto err_out;
    ::memset( &pi, 0, sizeof( struct IdBlock ));
    runPath = spi.runRoot + (spi.runRoot.endsWith("/")? "" : "/") + spi.relPath + "/" + spi.fileName;

    if ( installsByPath.contains( runPath ))
    {
        // update of pre-existing
        toWrite = installsByPath[runPath];
        ::lseek( manifestFd, toWrite->pos, SEEK_SET );
    }
    else
    {
        // new install
        toWrite = &pi;
        pi.values.installId = curInstallId++;
        pi.values.progId = spi.id;
        ::lseek( manifestFd, 0, SEEK_END );

        // endianess here not a problem since either way around the value is just
        // a key between the installs and manifest files
        ::fwrite( &toWrite->values.installId,
            sizeof(unsigned short), 1, installStrm );
        ::fputc( ':', installStrm );
        ::fputs( qPrintable( runPath ), installStrm );
        ::fputc( '\n', installStrm );
    }
    // either way update the inode/device numbers
    toWrite->values.inode = st.st_ino;
    toWrite->values.device = st.st_dev;
    toWrite->values.install_time = (qint64)(time(0));
    toWrite->values.progId = spi.id;
    ::write( manifestFd, toWrite->data, sizeof(struct IdBlock) );
    qLog(SXE) << "\tadd" << sizeof(struct IdBlock) << "bytes to manifest: InstallId" << pi.values.installId << "-" << runPath;
    qLog(SXE) << "\t\tinode:" << pi.values.inode << "  device:" << pi.values.device;
    return;
err_out:
    qWarning( "\t\terror: %s", strerror( errno ));
}

/*!
  \internal
  initialize the progDict and profileDict structures from the keyfile.
Precondition:
   The keyfile must exist and be initialized with at least [a dummy entry for]
   the qpe server, which occupies id 0
*/
void QPackageRegistry::initialiseInstallDicts()
{
    KeyFiler kf;
    const unsigned char *authData;
    qLog(SXE) << "Initialise install dictionaries";
    for ( unsigned char progId = 0; progId <= maxProgId; progId++ )
    {
        authData = getClientKey( progId );
        // qLog(SXE) << "\ttrying to load keys for progId" << progId;
        if ( authData == 0 )
        {
            break;
        }
        QStringList profileList = SXEPolicyManager::getInstance()->findPolicy( progId );
        if ( profileList.isEmpty() )
        {
            if ( progId == 0 )
                profileDict[QString("qpe_placeholder")] = 0;
            else
                qLog(SXE) << "WARNING!!!  Profile empty for" << progId;
        }
        else
        {
            QString domainString = profileList.join( "," );
            // qLog(SXE) << "Found policy for prog id" << progId << " - " << domainString;
            profileDict[domainString] = progId;
        }
        AuthRecord *ar = new AuthRecord;
        Q_CHECK_PTR( ar );
        ::memcpy( ar, authData, sizeof(AuthCookie) ); // ignored deprecated time field
        progDict[ progId ] = ar;
    }
    qLog(SXE) << "...loaded" << progDict.count() << "keys";
    Q_ASSERT( progDict.contains( 0 ));  // must at least have the server entry
}

#ifndef SXE_INSTALLER

QString QPackageRegistry::getQtopiaDir()
{
    return Qtopia::qtopiaDir();
}

#else

QString QPackageRegistry::currentInstallRoot = "/opt/Qtopia";

QString QPackageRegistry::getQtopiaDir()
{
    return currentInstallRoot + "/";
}

#endif

/*!
  Initialise an SxeProgramInfo \a pi which can then be used to register a sequence
  of same-domain binaries.

  A new key and a new program id is created and stored in \a pi.  The other fields
  are unchanged.

  The keyfile.sequence file is used to track and issue new program id numbers.
*/
void QPackageRegistry::initProgramInfo( SxeProgramInfo &pi )
{
    int res;
    AuthRecord ar;
    KeyFiler::randomizeKey( pi.key );
    QFile pk( sxeConfPath() + "/" QSXE_KEYFILE ".sequence" );
    if ( !pk.open( QIODevice::ReadWrite ))
        qFatal( "Could not open %s\n", qPrintable( pk.fileName() ));
    if ( !Qtopia::lockFile( pk, Qtopia::LockBlock ))
    {
        qWarning( "Could not lock %s\n", qPrintable( pk.fileName() ));
        return;
    }
    // initialize controls
    unsigned char currentId = 0;
    int recordsRead = 0;
    ::memset( &ar, 0, sizeof(struct AuthRecord));
    ar.change_time = time( 0 );

    // read to the end of the file
    while (( res = pk.read( (char*)(&ar), sizeof(struct AuthRecord))) > 0 )
    {
        if ( (size_t)res < sizeof(struct AuthRecord) )
            qLog(SXE) << "read error" << pk.fileName() << pk.errorString();
        currentId = ar.auth.progId;
        ++recordsRead;
    }

    // this prog's id is the next one in the sequence
    ar.auth.progId = currentId + 1;
    pi.id = ar.auth.progId;
    if ( pi.id > maxProgId )
        qFatal( "QPackageRegistry::initProgramInfo(): maxProgId exceeded" );

    // write the key and program id for this prog into the file
    ::memcpy( ar.auth.key, pi.key, QSXE_KEY_LEN );
    res = pk.write( (char*)&ar, sizeof( AuthRecord ));

    qLog(SXE) << "initProgramInfo:" << pi;

    // release the lock
    pk.close();
}

/*!
  Register a qtopia binary with the SXE system.  The path to the binary is:

  \c{ pi.absolutePath() }

  where the SxeProgramInfo \a pi contains all the information about the binary.

  Note that binaries with the same domain (SXE policy profile) and binaries from the
  same package are coalesced into the same Program Id, and thus use the same secret key.

  The SxeProgramInfo \a pi if uninitialised is modified by creating a new random
  key and program id.  The id and key fields if initialised are used to register the
  binary.
  \sa SxeProgramInfo
*/
void QPackageRegistry::registerBinary( SxeProgramInfo &pi )
{
    struct AuthCookie kr;
    memset( &kr, 0, sizeof(struct AuthCookie) );

    if ( pi.domain.isEmpty() )
        pi.domain = "none";

    if ( !pi.isValid() )
    {
        qLog(Package) << "Invalid program info" << pi;
        pi.fileName.prepend( "lib" );
        pi.fileName.append( ".so" );
        if ( !pi.isValid() )
        {
            qLog(Package) << "Could not find" << pi;
            qWarning( "registerBinary: Couldnt find %s", qPrintable( pi.fileName ));
            return;
        }
    }

    qLog(SXE) << "QPackageRegistry::registerBinary" << pi;

    openSystemFiles();
    readManifest();
    readInstalls();

    QString fpath = pi.absolutePath();
    QStringList normalizedDomain = pi.domain.split( ",", QString::SkipEmptyParts );
    normalizedDomain.sort();
    pi.domain = normalizedDomain.join( "," );

#ifdef SXE_INSTALLER
    if ( pi.fileName == "qpe" )
    {
        AuthRecord ar;
        ::memset( &ar, 0, sizeof( struct AuthRecord ));
        KeyFiler::randomizeKey( (char*)pi.key );
        pi.id = 0;
        qLog(SXE) << fpath << "qpe install - assigning progId 0 with new key";
        if ( profileDict.contains( QLatin1String( "qpe_placeholder" )))
            profileDict.remove( QLatin1String( "qpe_placeholder" ));
        profileDict[pi.domain] = 0;
        QFile pk( sxeConfPath() + "/" QSXE_KEYFILE ".sequence" );
        if ( !pk.open( QIODevice::ReadWrite ))
            qFatal( "Could not open %s\n", qPrintable( pk.fileName() ));
        ::memcpy( ar.auth.key, pi.key, QSXE_KEY_LEN );
        pk.write( (char*)&ar, sizeof( struct AuthRecord ));
    } else
#endif
    if ( profileDict.contains( pi.domain ))
    {
        pi.id = profileDict.value( pi.domain );
        Q_ASSERT( progDict.contains( pi.id ));
        KeyFiler::randomizeKey( (char*)pi.key );
        qLog(SXE) << "\tprofile" << pi.domain << "matched, using existing progId" << pi.id;
    }
    else
    {
        initProgramInfo( pi );
    }

    ::memcpy( &kr.key, pi.key, QSXE_KEY_LEN );
    kr.progId = pi.id;

    KeyFiler kf;
    kf.setFileName( fpath );
    if ( !kf.scanFile( &kr ))
    {
        // if a package declares a sxe domain but does not
        // #include qtopiaapplication.h is not necessarily an error
        qWarning( "\tno safe-exec install on %s", qPrintable( fpath ));
    }

    //only modify Keyfile if not on a lids enabled device
    if ( !QFile::exists( procLidsKeysPath ))
        updateProcKeyFile( pi, kf.fileDescriptor() );

    addToManifest( pi, kf.fileDescriptor() );
    if ( !profileDict.contains( pi.domain ))
    {
        qLog(SXE) << "Adding new domain to profiles list:" << kr.progId << "[" << pi.domain << "]";
        profileDict[pi.domain] = kr.progId;
    }
    else
    {
        qLog(SXE) << "Profiles list already contains domain [" << pi.domain << "], under prog id" << profileDict[pi.domain];
    }

    // do this while still have lock on system
    outputPolicyFile();

    // release lock
    closeSystemFiles();
}

void QPackageRegistry::updateProcKeyFile( const SxeProgramInfo &spi, int sfd )
{
    qLog(SXE) << "updateProcKeyFile id:" << spi.id << "path" << spi.fileName;
    bool found = false;
    qint64 fpos = 0;
    struct stat kfstat;
    int res = ::fstat( sfd, &kfstat );
    if ( res == -1 )
    {
        qLog(SXE) << "could not stat" << spi.fileName << strerror( errno );
        return;
    }
    struct usr_key_entry ke;
    QFile pk( sxeConfPath() + "/" QSXE_KEYFILE );
    if ( !pk.open( QIODevice::ReadWrite ))
        qFatal( "Could not open %s\n", qPrintable( pk.fileName() ));
    while ( spi.id && ( res = pk.read( (char*)(&ke), sizeof(struct usr_key_entry))) > 0 )
    {
        if ( ke.ino == kfstat.st_ino && ke.dev == kfstat.st_dev )
        {
            found = true;
            break;
        }
        fpos = pk.pos();
    }
    if ( res == -1 )
        qLog(SXE) << "read error" << pk.fileName() << pk.errorString();
    ::memcpy( ke.key, spi.key, QSXE_KEY_LEN );
    ke.dev = kfstat.st_dev;
    ke.ino = kfstat.st_ino;
    if ( found )
        pk.seek( fpos );
    res = pk.write( (char*)(&ke), sizeof(struct usr_key_entry) );
    if ( (size_t)res < sizeof(struct usr_key_entry) )
        qLog(SXE) << "write error" << pk.fileName() << pk.errorString();
    pk.close();
}

/*!
  Unregisters a package's binaries from the SXE system for the given \a packagePath

  returns true if the removal of the registration was successful; otherwise returns false
*/
bool QPackageRegistry::unregisterPackageBinaries( const QString &packagePath )
{
    bool result = false;
    QList<QVariant> checkList;
    if ( !openSystemFiles() )
        goto endUnregisterPackageBinaries;

    if ( manifestFd == -1 )
    {
        qWarning( "Manifest file not open, ignoring unregister request" );
        goto endUnregisterPackageBinaries;
    }

    if ( installStrm == NULL )
    {
        qWarning( "Install stream not open, ignoring unregister request" );
        goto endUnregisterPackageBinaries;
    }

    checkList = removeFromRegistry( packagePath, Installs, result ).toList();
    if ( !result )
    {
        qLog(SXE) << "QPackageRegistry::unregisterPackageBinaries could not unregister from installs "
                  << "(Note:if the package hasn't been registered yet, this warning can be ignored)";
        goto endUnregisterPackageBinaries;
    }

    checkList = removeFromRegistry( checkList, Manifest, result ).toList();
    if ( !result)
    {
        goto endUnregisterPackageBinaries;
        qWarning("QPackageRegistry::unregisterPackageBinaries could not unregister from manifest");
    }

    //only modify Keyfile if not on a lids enabled device
    if ( !QFile::exists( procLidsKeysPath ))
    {
        removeFromRegistry( checkList, Keyfile, result );
        if ( !result )
            qWarning("QPackageRegistry::unregisterPackageBinaries could not unregister from keyfile");
    }

endUnregisterPackageBinaries:
    closeSystemFiles();
    return result;
}


/*!
  \internal
  Removes the entries of a package's binaries from the installs or manifest file
  depending on \a type.

  If \a type is Installs, then \a checkList is the path to the directory of
  the package being removed.  The package's entries in the installs file
  is deleted and a QList<QVariant> of all the installId's associated with
  the package is returned.

  If \a type is Manifest, then \a checkList is a QList<QVariant> of the
  installId's associated with a package.  The package's entries in the
  manifest file is deleted and an empty QList<variant> is returned.

  If \a type is Keyfile, then \a checkList is a QList<QVariant> of the
  inode and device numbers of the package binaries.  The inodes and device
  numbers are listed the following pattern: inode1, device1, inode2, device2...

  The param \a result is modified depending on whether the removal operation
  was successful or not.

  This method should not be called when the \a type is KeyFile
*/
QVariant QPackageRegistry::removeFromRegistry( const QVariant &checkList, RegistryFile type, bool &result )
{
    qLog(SXE) << "##################### removeFromRegistry() ##################";
    off_t beginPos= -1; //beginning position of entries to be removed
    off_t afterEndPos = -1; //position just after the end position of entries to be removed
    struct stat st;
    FILE * stream;
    QList<QVariant> ret;
    int fd;
    result = false;

    switch ( type )
    {
        case ( Installs ):
            stream = installStrm;
            fd = installFd;
            break;
        case ( Manifest ):
            stream = fdopen( manifestFd, "w+" );
            fd = manifestFd;
            break;
        case ( Keyfile ):
            fd = ::open( qPrintable(sxeConfPath() + "/" QSXE_KEYFILE), O_RDWR );
            stream = fdopen( fd, "w+" );
            break;
        default:
            qWarning( "QPackageRegistry::removeFromRegistry invalid control type" );
            return ret;
    }

    if ( fstat( fd, &st ) == -1 )
    {
        qWarning("QPackageRegistry::removeFromRegistry error; %s", strerror( errno ));
        return ret;
    }
    off_t initSize = st.st_size;

    Q_ASSERT ( stream != NULL );
    ::rewind( stream );

    //specific variables for Installs
    unsigned int lineNo = 0;
    char pad;
    char charBuf[ MAX_PATH_LEN ];
    unsigned short installId;

    //specific variables for Manifest
    IdBlock idBuf;

    //specific variables for Keyfile
    struct usr_key_entry kBuf;

    off_t currPos;
    bool match;
    //iterate through the registry file and depending on a match with the checklist
    //set beginPos or afterEndPos appropriately.
    do
    {
        match = false;
        if ( (currPos = ftello(stream)) ==  -1)
        {
            qWarning( "QPackageRegistry::removeFromRegistry: Cannot get position of stream" );
            return ret;
        }

        if ( type == Installs )
        {
            if( !fread( &installId, sizeof( installId ), 1, stream ) )
                break;

            lineNo++;
            pad =  ::fgetc ( stream );
            if ( pad != ':' )
                qWarning( "expected \":\" reading installs - line %d", lineNo );

            fgets( charBuf, MAX_PATH_LEN, installStrm );
            if ( QString( charBuf ).contains( checkList.toString() ) )
            {
                ret.append( installId );
                match = true;
            }

        } else if ( type == Manifest )
        {
            if ( !::fread( &idBuf, sizeof( struct IdBlock ), 1, stream ) )
                break;

            if ( checkList.toList().contains( idBuf.installId ) )
            {
                match = true;
                ret.append( idBuf.inode );
                ret.append( idBuf.device );
            }

        } else if ( type == Keyfile )
        {
            if ( !::fread( &kBuf, sizeof( struct usr_key_entry ), 1, stream ) )
                    break;
            QList<QVariant> ino_devs = checkList.toList();
            for ( int i = 0; i < ino_devs.count(); i=i+2 )
            {
                if ( kBuf.ino == (ino_t)ino_devs[i].toLongLong()
                        && kBuf.dev == (dev_t)ino_devs[i+1].toLongLong() )
                {
                    match = true;
                }
            }
        }

        if ( match ) //if a match has been made
        {
            if ( beginPos == -1 ) //set beginPos if not already set
                beginPos = currPos;
        } else
        {
            if ( beginPos == -1 ) // have not even made one match yet
                continue;
            else
                if ( afterEndPos == -1 ) // we already have a match and we've reached the
                {                       // end of the series of entries we want to remove
                    afterEndPos = currPos;
                    break;
                }
        }
    } while ( 1 );

    if ( beginPos == -1 )
    {
        qLog(SXE) << "QPackageRegistry::Package to be removed not found in" << (type == Installs ? "installs" : "manifest");
        return ret;
    }

    if ( afterEndPos == -1 )
        afterEndPos = initSize;
    off_t newSize = initSize - (afterEndPos - beginPos);
    Q_UNUSED(newSize);

    //try to shift up any entries from afterEndPos to
    //begin pos
    fseek( stream, afterEndPos, SEEK_SET );
    unsigned int numItems;
    while( ( numItems = ::fread(charBuf, sizeof(char), MAX_PATH_LEN, stream) ) )
    {
        afterEndPos = ftello( stream );

        fseek ( stream, beginPos, SEEK_SET );
        ::fwrite(charBuf, sizeof( char ), numItems, stream );

        beginPos =  ftello( stream );
        fseek ( stream, afterEndPos, SEEK_SET );
    }

    //registry file could become corrupt if not properly truncated

    int r = ftruncate( fd, newSize );
    Q_UNUSED(r);//assert is removed in release mode so r becomes unused.
    Q_ASSERT( r == 0 );

    if ( type == Keyfile )
    {
       fclose( stream );
       close( fd );
    }
    result = true;
    return ret;
}


/*!
  Write the (possibly updated) policy file out to the file system.

  Pre-requisite: lock on the system files
*/
void QPackageRegistry::outputPolicyFile()
{
    if ( profileDict.count() == 0 ) return;
    QString policyFn = sxeConfPath() + "/" + policyFileName;
    QFile pf( policyFn );
    if ( !pf.open( QIODevice::WriteOnly | QIODevice::Truncate ))
    {
        qWarning( "error opening %s", qPrintable( policyFn ));
        return;
    }
    qLog(SXE) << "Writing SXE policy file" << policyFn;
    QTextStream ts( &pf );
    QHash<QString,unsigned char>::iterator pit = profileDict.begin();
    while ( pit != profileDict.end() )
    {
        QStringList profs = pit.key().split( "," );
        unsigned char progId = pit.value();
        ts << "[" << progId << "]" << endl;
        if ( profs.count() == 0 || profs[0].isEmpty() )
        {
            ts << "none" << endl;
        }
        else
        {
            for ( int i = 0; i < profs.count(); i++ )
                ts << profs[i] << endl;
        }
        ++pit;
    }
    pf.close();
}


/*!
  Bootstrap the qtopia image with sxe control files.

  Creates (or truncates) new empty SXE control files, ready to be updated
  by the scanner.  If the \a installRoot already contains non-empty files
  then it will refuse to overwrite them.

  The return code of this is used as a unix exec return code, ie 0 is success
  non-zero is failure.
*/
int QPackageRegistry::bootstrap( const QString &installRoot )
{
#ifdef SXE_INSTALLER
    currentInstallRoot = installRoot;
#endif
    QDir installRootDir( installRoot );
    if ( !installRootDir.exists() )
    {
        qWarning( "sxe_installer: %s does not exist", qPrintable( installRoot ));
        return 1;
    }

    if ( openSystemFiles() )
    {
        closeSystemFiles();
        return 0;
    }

    QStringList sxeSystemFiles;
    sxeSystemFiles << QString( sxeConfPath() + "/" QSXE_KEYFILE );
    sxeSystemFiles << QString( sxeConfPath() + "/" + manifestFileName );
    sxeSystemFiles << QString( sxeConfPath() + "/" + installsFileName );
    sxeSystemFiles << QString( sxeConfPath() + "/" + policyFileName );
    sxeSystemFiles << QString( sxeConfPath() + "/" QSXE_KEYFILE + ".sequence" );

    qLog(SXE) << "Bootstrap required - creating SXE system files\n";

    struct usr_key_entry kr;
    ::memset( &kr, 0, sizeof( kr ));

    struct AuthRecord ar;
    ::memset( &ar, 0, sizeof( ar ));

    struct IdBlock id;
    ::memset( &id, 0, sizeof( id ));

    for ( int f = 0; f < sxeSystemFiles.count(); f++ )
    {
        int rfd = ::open( sxeSystemFiles[f].toLocal8Bit(),
                O_CREAT | O_EXCL | O_TRUNC | O_RDWR,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
        if ( rfd == -1 )
        {
            if ( errno == EEXIST )  // something very wrong here
                qFatal( "File %s should not exist in bootstrap", qPrintable( sxeSystemFiles[f] ));
            else
                qWarning( "Create in bootstrap %s: %s", qPrintable( sxeSystemFiles[f] ), strerror( errno ));
        }
        // close the keyfile & let qtransportauth's methods handle it
        if ( f == 0 )
        {
            // reserve progId 0 for the server
            ::write( rfd, &kr, sizeof( kr ));
            ::close( rfd );
        }
        else if ( f == 1 )
        {
            manifestFd = rfd;
        }
        else if ( f == 2 )
        {
            installFd = rfd;
        }
        else if ( f == 4 )
        {
            // reserve progId 0 for the server
            ::write( rfd, &ar, sizeof( ar ));
            ::close( rfd );
        }
        if ( rfd == -1 )
            qWarning( "Lock failed in bootstrap %s", strerror( errno ));
    }

    closeSystemFiles();
    return 0; // success
}

/*!
  Return the key associated the specified program id \a progId.

  As a host binary sxe_installer doesnt have access to this method in
  qtransportauth_qws.cpp, so reimplement it here.

  In the SXE installer case assume there is a one-to-one mapping from
  keys to progId's since the installer ensures that.
*/
const unsigned char *QPackageRegistry::getClientKey( unsigned char progId )
{
#ifdef SXE_INSTALLER
    AuthRecord recbuf;
    if ( clientKeys.contains( progId ))
        return clientKeys[progId];
    QFile kf( sxeConfPath() + "/" QSXE_KEYFILE ".sequence" );
    if ( !kf.open( QIODevice::ReadOnly ))
        qFatal( "Couldnt open key file %s", qPrintable(kf.fileName()));
    while ( kf.read( (char*)&recbuf, sizeof(struct AuthRecord) ) > 0)
        if ( recbuf.auth.progId == progId )
            break;
    if ( recbuf.auth.progId == progId )
    {
        unsigned char *ar = (unsigned char*)( malloc( sizeof( recbuf )));
        Q_CHECK_PTR( ar );
        ::memset( ar, 0, sizeof( struct AuthRecord ));
        ::memcpy( ar, &recbuf, sizeof( struct AuthRecord ));
        clientKeys.insert( progId, ar );
        return ar;
    }
    return 0;
#else
    return QTransportAuth::getInstance()->getClientKey( progId );
#endif
}

