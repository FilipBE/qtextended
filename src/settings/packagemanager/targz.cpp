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
#include "targz.h"

#include <libtar.h>

#include <zlib.h>

#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <QHash>
#include <QFile>
#include <QDataStream>
#include <qlog.h>
/////////////////////////////////////////////////////////////////
//
//  This should be thread safe on Unixes where a file handle is unique
//  to a process/thread.
//
QHash<int, gzFile> gzHandles;

static int wrap_gzopen( const char *fn, int flags, ... )
{
    const char *mode = ( flags & O_WRONLY ) ? "w" : "r";
    int fd = ::open( fn, flags );
    gzFile fh = gzdopen( fd, mode );
    gzHandles[ fd ] = fh;
    return fd;
}

static int wrap_gzclose( int fd )
{
    if ( !gzHandles.contains( fd )) return -1;
    int result = gzclose( gzHandles[ fd ] );
    gzHandles.remove( fd );
    return result;
}

static ssize_t wrap_gzread( int fd, void *buf, size_t count )
{
    if ( !gzHandles.contains( fd )) return -1;
    gzFile fh = gzHandles[ fd ];
    return gzread( fh, buf, count );
}

static ssize_t wrap_gzwrite( int fd, const void *buf, size_t count )
{
    if ( !gzHandles.contains( fd )) return -1;
    gzFile fh = gzHandles[ fd ];
    return gzwrite( fh, (void *)buf, count );
}

tartype_t zlibtype = { wrap_gzopen, wrap_gzclose, wrap_gzread, wrap_gzwrite };

/**
  Extract a tar or tar.gz archive in \arg tarfile, to the path in \a destpath.
  If \a verbose is true, echo a line on stdout for each file extracted.
  The libtar and libz implementations are used.  The file \a tarfile is tested
  for the magic number indicating a gzip file, and if present gz stream i/o is
  used, otherwise the file is assumed to be a normal .tar archive.  Note that
  the file extension is ignored since many files such as package format files,
  use other extensions.  The return result is true if the operation succeeded.
*/
bool targz_extract_all( const QString &tarfile, const QString &destpath, bool verbose )
{
    QByteArray pathnameArr = tarfile.toLocal8Bit();
    char *pathname = pathnameArr.data();
    QByteArray prefixArr = destpath.toLocal8Bit();
    char *prefix = destpath.isEmpty() ? 0 : prefixArr.data();

    TAR *tarHandle;

    int options = TAR_GNU;
    if ( verbose ) options |= TAR_VERBOSE;

    int filemode = 0;  // only care about this if creating files (ie untar)

    tartype_t *arctype = 0;

    {
        // QFile and stream go away at end of scope
        QFile tfs( tarfile );
        if ( !tfs.exists() )
        {
            qWarning( "Targz_extract_all: file %s doesnt exist", pathname );
            return false;
        }
        tfs.open(QIODevice::ReadOnly);
        QDataStream tarbytes( &tfs );
        quint8 b1, b2;

        // if the first two bytes are the magic numbers for a gzip format
        // file, use the above zlib wrapped i/o function pointers, otherwise
        // assume normal uncompressed tar format (default)
        tarbytes >> b1 >> b2;
        if ( b1 == 0x1f && b2 == 0x8b ) arctype = &zlibtype;
    }

    int result = tar_open( &tarHandle, pathname, arctype, O_RDONLY, filemode, options);

    if ( result < 0 )
    {
        qWarning( "error opening tar file %s: %s", pathname, strerror(errno) );
        return false;
    }

    result = tar_extract_all( tarHandle, prefix );

    if ( result < 0 )
        qWarning( "error extracting tar file %s: %s", pathname, strerror(errno) );

    tar_close( tarHandle );
    return ( result >= 0 );
}

/**
  Archive a tar or tar.gz archive in \arg tarfile, from the path in \a srcpath.
  If \a verbose is true, echo a line on stdout for each file archived.
  The libtar and libz implementations are used.  If the filename \a tarfile
  ends with the characters "gz" then gz stream i/o is used for storing the tar.
  If the \arg gzip is true, then gz will be used regardless of the filename.
  Otherwise the file is created as a normal .tar archive.  The return result is
  true if the operation succeeded.
*/
bool targz_archive_all( const QString &tarfile, const QString &srcpath, bool gzip, bool verbose )
{
    QByteArray pathnameArr = tarfile.toLocal8Bit();
    char *pathname = pathnameArr.data();
    QByteArray prefixArr = srcpath.toLocal8Bit();
    char *prefix = prefixArr.data();

    TAR *tarHandle;

    int options = TAR_GNU;
    if ( verbose ) options |= TAR_VERBOSE;

    int filemode = 0;  // only care about this if creating files (ie untar)

    tartype_t *arctype = 0;

    if ( gzip || tarfile.endsWith( "gz" )) arctype = &zlibtype;

    int result = tar_open( &tarHandle, pathname, arctype, O_WRONLY, filemode, options);

    if ( result < 0 )
    {
        qWarning( "error opening tar file %s: %s", pathname, strerror(errno) );
        return false;
    }

    result = tar_append_tree( tarHandle, prefix, prefix );

    if ( result < 0 )
        qWarning( "error archiving to tar file %s: %s", pathname, strerror(errno) );

    tar_close( tarHandle );
    return ( result >= 0 );
}

qulonglong targz_archive_size( const QString &tarfile )
{
    TAR *tarHandle = get_tar_ptr( tarfile );

    qulonglong size = 0;
    int i;
    while ((i = th_read(tarHandle)) == 0)
    {
        size += th_get_size(tarHandle);
        if( TH_ISREG(tarHandle) )
            tar_skip_regfile(tarHandle);
    }

    tar_close( tarHandle );
    return size;
}
/* \internal
   Checks whether a tar file is valid.
   A valid tar file does not contains files
   that start with ./ or contain ..

   This is intended to prevent malicious packages
   placing binaries outside of the sandbox directory
*/
bool check_tar_valid( const QString &tarfile )
{
    TAR *tarHandle = get_tar_ptr( tarfile );

    bool ret=true;
    QString filename;
    int i;
    while ( (i = th_read(tarHandle)) == 0)
    {
        filename = th_get_pathname( tarHandle );

        if ( !filename.startsWith("./") || filename.contains( "..") )
        {
            ret = false;
            qWarning() << "check_tar_valid:- tar contains invalid file path: "
                << filename << "\nAll paths must begin with ./ and not contain .."  ;
            break;
        }
        else if ( TH_ISBLK(tarHandle) || TH_ISCHR(tarHandle) )
        {
            ret = false;
            qWarning() << "check_tar_valid:-tar invalid, contains device special file:"
                << filename;
            break;
        }
        else if ( TH_ISLNK(tarHandle) )
        {
            ret = false;
            qWarning() << "check_tar_valid:-tar invalid, contains hard link:"
                       << filename;
            break;
        } else if (TH_ISSYM(tarHandle) )
        {
            QString target;
            if ((tarHandle->options & TAR_GNU) && tarHandle->th_buf.gnu_longlink != NULL)
                target = tarHandle->th_buf.gnu_longlink;
            else
                target = tarHandle->th_buf.linkname;
            if ( target.startsWith("/") || target.contains( "..") )
            {
                ret = false;
                qWarning() << "check_tar_valid:tar invalid, contains symlink whose target"
                           << (target.startsWith("/")?"is an absolute path.":"references "
                              "a parent directory.")
                           << "Link:" << filename << "Target:" << target;
                break;
            }
        }


        if( TH_ISREG(tarHandle) )
            tar_skip_regfile(tarHandle);
    }

    tar_close( tarHandle );
    return ret;
}

TAR * get_tar_ptr( const QString &tarfile )
{
       QByteArray pathnameArr = tarfile.toLocal8Bit();
       char *pathname = pathnameArr.data();

       TAR *tarHandle;
       int options = TAR_GNU;
       int filemode = 0;  // only care about this if creating files (ie untar)
       tartype_t *arctype = 0;
       {
           // QFile and stream go away at end of scope
           QFile tfs( tarfile );
           if ( !tfs.exists() )
           {
               qWarning( "get_tar_ptr(): file %s doesnt exist", pathname );
               return false;
           }
           tfs.open(QIODevice::ReadOnly);
           QDataStream tarbytes( &tfs );
           quint8 b1, b2;

           // if the first two bytes are the magic numbers for a gzip format
           // file, use the above zlib wrapped i/o function pointers, otherwise
           // assume normal uncompressed tar format (default)
           tarbytes >> b1 >> b2;
           if ( b1 == 0x1f && b2 == 0x8b ) arctype = &zlibtype;
       }


    int result = tar_open( &tarHandle, pathname, arctype, O_RDONLY, filemode, options);
    if ( result < 0 )
    {
        qWarning( "error opening tar file %s: %s", pathname, strerror(errno) );
        return false;
    }
    return tarHandle;
}
