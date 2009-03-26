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

#include "qtopiasxe.h"

#include <qtopialog.h>
#ifndef SXE_INSTALLER
#include <qtransportauth_qws.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <QStringList>
#include <qtopianamespace.h>

#if !defined(QT_NO_SXE) || defined(SXE_INSTALLER) || defined(Q_QDOC)

struct SxeProgramInfoPrivate
{
    struct stat sxe_stat;       // cached value of the last stat
    bool valid;        // does this point to a valid binary?
    QString binary;    // cached value of the absolute path
};

/*!
  \class SxeProgramInfo
    \inpublicgroup QtBaseModule

  \brief The SxeProgramInfo class is a data transfer object that models a program on disk.

  Used for registration of binaries with the SXE system.

  Registration can happen at run-time of the qtopia device
  or at install-time, when building the qtopia ROM image on
  a development host.

  For example, consider the following binaries installed onto
  a device:
  \code
  /opt/Qtopia.rom/bin/calculator
  /opt/Qtopia.user/packages/bin/bomber
  \endcode

  These examples are referenced in the documentation of public variables
  below.  The table below summarizes the difference between "qbuild image"
  time and run time.

  \table
  \header
    \o install root on dev host
    \o run root on device
  \row
    \o $HOME/build/qtopia42/image
    \o /opt/Qtopia.rom
  \endtable

  The runRoot must be one of the elements which will be
  returned by the Qtopia::installPaths() method at runtime.

  The installRoot is optional and will be empty in the case of
  a binary installed at run-time by the package manager.
*/

/*!

  \variable SxeProgramInfo::fileName
    the binaries file name, eg calculator, bomber
*/

/*!

  \variable SxeProgramInfo::relPath
    for example \c bin, \c packages/bin, \c plugings/application
*/

/*!

  \variable SxeProgramInfo::runRoot
    for example \c /opt/Qtopia.rom, \c /opt/Qtopia.user
*/

/*!

  \variable SxeProgramInfo::installRoot
    for example \c {$HOME/build/qtopia/42-phone/image}
    this should only ever be set when running under sxe_installer
*/

/*!

  \variable SxeProgramInfo::domain
    security domains, a list of comma seperated values
*/

/*!

  \variable SxeProgramInfo::id
    SXE program identity
*/

/*!

  \variable SxeProgramInfo::key
    SXE shared secret key
*/

/*!
  \fn SxeProgramInfo::SxeProgramInfo()

  Construct a new SxeProgramInfo
*/
SxeProgramInfo::SxeProgramInfo()
    : id( 0 )
    , d( new SxeProgramInfoPrivate )
{
    ::memset( key, 0, QSXE_KEY_LEN );
}

/*!
  \fn SxeProgramInfo::~SxeProgramInfo()

  Destroy this SxeProgramInfo object
*/
SxeProgramInfo::~SxeProgramInfo()
{
    delete d;
}


void sxeInfoHexstring( char *buf, const unsigned char* key, size_t key_len )
{
    unsigned int i, p;
    for ( i = 0, p = 0; i < key_len; i++, p+=2 )
    {
        unsigned char lo_nibble = key[i] & 0x0f;
        unsigned char hi_nibble = key[i] >> 4;
        buf[p] = (int)hi_nibble > 9 ? hi_nibble-10 + 'A' : hi_nibble + '0';
        buf[p+1] = (int)lo_nibble > 9 ? lo_nibble-10 + 'A' : lo_nibble + '0';
    }
    buf[p] = '\0';
}

/*!
  \relates SxeProgramInfo

  Sends the SxeProgramInfo \a progInfo to the \a debug stream.
*/
QDebug operator<<(QDebug debug, const SxeProgramInfo &progInfo)
{
    debug << "Sxe Program Info:" << progInfo.fileName << endl;
    debug << "\trelative path:" << progInfo.relPath << endl;
    debug << "\tinstall to:" << progInfo.installRoot << endl;
    debug << "\trun from:" << progInfo.runRoot << endl;
    debug << "\tSXE domains:" << progInfo.domain << endl;
    debug << "\tSXE id:" << progInfo.id << endl;
    char keydisp[QSXE_KEY_LEN*2+1];
    sxeInfoHexstring( keydisp, (const unsigned char *)progInfo.key, QSXE_KEY_LEN );
    debug << "\tSXE key:" << keydisp << endl;
    return debug;
}

/*!
  Return the QString of the absolute path to the binary for this SxeProgramInfo object.

  This is simply the concatenation of installRoot, relPath and fileName.  If the installRoot
  is null, the runRoot is used instead.

  isValid() can be called to check that the returned path will point to a valid binary.
*/
QString SxeProgramInfo::absolutePath() const
{
    return ( installRoot.isEmpty() ? runRoot : installRoot ) +
        "/" + relPath + "/" + fileName;
}

/*!
  Return true if the SxeProgramInfo object represents a valid binary for SXE registration,
  and otherwise return false.

  The binary is valid if all of the components of absolutePath() above are non-empty, and
  the resulting path is to a file which exists
*/
bool SxeProgramInfo::isValid() const
{
    if ( d->binary == absolutePath() )
        return d->valid;
    if ( fileName.isEmpty() || relPath.isEmpty() )
        return false;
#ifdef SXE_INSTALLER
    if ( installRoot.isEmpty() )
        return false;
#else
    if ( runRoot.isEmpty() )
        return false;
#endif
    d->binary = absolutePath();
    d->valid = false;
    int result = ::stat( QFile::encodeName( d->binary ), &d->sxe_stat );
    qLog(SXE) << "stat'ed binary" << d->binary << " - result:" << result;
    if ( result == 0 )
    {
        d->valid = true;
    }
    else if ( result != -ENOENT )
    {
        qLog(SXE) << "Could not stat" << d->binary << strerror( errno );
    }
    return d->valid;
}

/*!
  Try to find this binary in the current file-system.  First a check is made to
  see if isValid() returns true:  if so this method simply returns true.

  Otherwise an attempt to find the binary is made by traversing the
  entries in Qtopia::installPaths().

  The fileName member should be set to non-empty before calling this method
  otherwise it simply returns false.

  If the relPath member is set, then only that path is checked under each of
  the installPaths().

  If the relPath member is empty, then the path searched is decided by checking if
  the filename refers to a shared library or not: if the fileName starts with "lib"
  and ends with ".so" then "application/plugins" are checked, otherwise only "bin"
  is checked.

  If the binary is found, the relPath and runRoot entries are set to the
  the location, and the method returns \c true.

  If the binary is not found, no entries are altered and the method returns
  false.

  For example:
  \code
  SxeProgramInfo calcBin;
  calcBin.fileName = "calculator";
  if ( calcBin.locateBinary() )
      qDebug() << "Calculator is a standalone app at" << calcBin.absolutePath();
  else
  {
      calcBin.fileName = "libcalculator.so";
      if ( calcBin.locateBinary() )
          qDebug() << "Calculator is a quicklaunched app at" << calcBin.absolutePath();
      else
          qDebug() << "Calculator is not available";
  }
  \endcode
*/
bool SxeProgramInfo::locateBinary()
{
#ifdef SXE_INSTALLER
    qFatal( "SxeProgramInfo::locateBinary called by sxe_installer!!" );
    return false;
#else
    if ( isValid() )
        return true;
    if ( fileName.isEmpty() )
        return false;
    QStringList paths = Qtopia::installPaths();
    QString saveRunRoot = runRoot;
    QString saveRelPath = relPath;
    foreach ( QString p, paths )
    {
        runRoot = p;
        if ( !relPath.isEmpty() && isValid() )
            return true;
        if ( relPath.isEmpty() )
            relPath = ( fileName.startsWith( "lib" ) && fileName.endsWith( ".so" ))
                ? "plugins/application" : "bin";
        if ( isValid() )
            return true;
        relPath = saveRelPath;
    }
    runRoot = saveRunRoot;
    return false;
#endif
}

/*!
  Make the current process assume the SXE identity of the executable
  represented by this SxeProgramInfo.

  Internally this is done by writing to the /proc/lids/suid
  pseudo file.

  This method only succeeds on a suitably patched SXE linux kernel.
*/
void SxeProgramInfo::suid()
{
#ifndef SXE_INSTALLER
    int res = 0;
    QFile procSuid( "/proc/lids/suid" );
    struct usr_key_entry k_ent;
    if ( !procSuid.exists() )
        return;
    if ( !isValid() )
    {
        qWarning( "Could not suid to invalid binary: %s", qPrintable( d->binary ));
        return;
    }
    if ( procSuid.open( QIODevice::WriteOnly ))
    {
        k_ent.ino = d->sxe_stat.st_ino;
        k_ent.dev = d->sxe_stat.st_dev;
        res = procSuid.write( (char*)&k_ent, sizeof( k_ent ));
        if ( res != sizeof( struct usr_key_entry ))
            qWarning( "Could not write /proc/lids/suid: %s", strerror( errno ));
        else
            qLog(SXE) << "SUID to" << d->binary << "device" << d->sxe_stat.st_dev << "inode" << d->sxe_stat.st_ino;
    }
#endif
}





/*!
  \macro QSXE_APP_KEY
  \relates SxeProgramInfo

  This macro causes the key storage to be allocated and initialized.
  An application without this macro cannot be used on a SXE-enabled system.
  It is needed if you are not using the QTOPIA_MAIN macro.

  \code
    QSXE_APP_KEY
    int main( int argc, char **argv )
    {
        QSXE_SET_APP_KEY(argv[0]);
        ...
  \endcode

  \sa Applications, QTOPIA_APP_KEY
*/

/*!
  \macro QSXE_SET_APP_KEY(name)
  \relates SxeProgramInfo

  This macro causes the SXE key to be copied into memory. It is needed if you are not using
  the QTOPIA_MAIN macro. It must be the first line of your main() function. The \a name value
  should be set to the binary's name.

  \code
    QSXE_APP_KEY
    int main( int argc, char **argv )
    {
        QSXE_SET_APP_KEY(argv[0]);
        ...
  \endcode

  \sa Applications, QTOPIA_SET_KEY()
*/

/*!
  \macro QSXE_QL_APP_KEY
  \relates SxeProgramInfo

  This macro causes the key storage to be allocated and initialized.
  An application without this macro cannot be used on a SXE-enabled system.
  It is needed if you are not using the QTOPIA_MAIN macro.

  Note that this macro only applies to quicklaunch plugins.

  \sa Applications, QTOPIA_APP_KEY
*/

/*!
  \macro QSXE_SET_QL_KEY(name)
  \relates SxeProgramInfo

  This macro causes the SXE key to be copied into memory. It is needed if you are not using
  the QTOPIA_MAIN macro. It must be called before constructing QtopiaApplication. The \a name value
  should be set to the binary's name.

  Note that this macro only applies to quicklaunch plugins.

  \sa Applications, QTOPIA_SET_KEY()
*/



#endif

/*!
  \relates SxeProgramInfo

  Ensure the SXE key for this executable is valid, and then call the transport
  authorizer method to set it.

  After this call all QWS calls by \a app will be authorized with this \a key.

  A check is made that the binary has been keyed, and if not then the method
  will qFatal, with the message "SXE key has not been set".

  (This function is stubbed out with an empty implementation if Qt Extended is configured without SXE.)
*/
void checkAndSetProcessKey( const char *key, const char *app )
{
#if !defined(QT_NO_SXE)

    #ifdef SXE_INSTALLER
        Q_UNUSED(key);
        Q_UNUSED(app);
    #else
        if ( strncmp( QSXE_KEY_TEMPLATE, key, QSXE_KEY_LEN ) == 0 )
        {
            qFatal( "SXE key has not been set.  Has \"qbuild image\" been run on %s?", app );
        }
        else
        {
            QTransportAuth::getInstance()->setProcessKey( key, app );
        }
    #endif
#else
        Q_UNUSED(key);
        Q_UNUSED(app);
#endif
}

