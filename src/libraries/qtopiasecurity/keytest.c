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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

/* from $QT_DEPOT_PATH/src/gui/embedded/qtransportauth_qws_p.h */
#define QSXE_KEY_LEN 16

/* from $QTOPIA_DEPOT_PATH/src/libraries/qtopiasecurity/keyfiler.cpp */
static const size_t kf_buf_sz = 64 * QSXE_KEY_LEN;

#define RAND_DEV "/dev/urandom"

/* from $QTOPIA_DEPOT_PATH/src/libraries/qtopia/qtopiaapplication.h */
#define QSXE_KEY_TEMPLATE "XOXOXOauthOXOXOX99"

void addElf( char buf[] )
{
    int e;
    const char elfMagic[] = { '\177', 'E', 'L', 'F' };
    const char *mptr = elfMagic;
    for ( e = 0; e < 4; e++, mptr++ )
        buf[e] = *mptr;
}

int main( int argc, char *argv[] )
{
    int wfd = open( "testfile_span", O_WRONLY | O_CREAT, S_IRWXU );
    if ( wfd == -1 )
    {
        perror( "write \"testfile_span\"" );
        return 1;
    }
    int rfd = open( RAND_DEV, O_RDONLY );
    if ( rfd == -1 )
    {
        perror( "read " RAND_DEV );
        return 1;
    }

    int wrs;
    int rrs;
    int patlen = strlen( QSXE_KEY_TEMPLATE );

    // write a repeating pattern of this many bytes as padding
    const int repeatlen = QSXE_KEY_LEN;

    char buf[kf_buf_sz];
    char randbuf[repeatlen];

    int i;

    int bufferCount = 0;  /* number of buffers */

    /* get some random bytes to write into the file */
    rrs = read( rfd, randbuf, repeatlen );
    close( rfd );

    int bufSplit = patlen >> 1;
    const char* pattern = QSXE_KEY_TEMPLATE;


    while( bufferCount < 4 )
    {
        for ( i = 0; i < 64; i++ )
            memcpy( buf + ( i * repeatlen ), randbuf, repeatlen );

        /* make it look like an ELF binary */
        if ( bufferCount == 0 )
            addElf( buf );

        /* in the second bufferful, copy the first half of the key into
           the end of the buffer before writing it */
        if ( bufferCount == 1 )
            memcpy( buf + ( kf_buf_sz - bufSplit ), pattern, bufSplit );

        /* in the third bufferful, copy the last half of the key into
           the beginning of the buffer before writing it */
        if ( bufferCount == 2 )
            memcpy( buf, pattern + ( patlen - bufSplit ), bufSplit );

        write( wfd, buf, kf_buf_sz );

        bufferCount++;
    }

    close( wfd );
}
