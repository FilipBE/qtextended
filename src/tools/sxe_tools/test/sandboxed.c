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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>


#define BUF_SZ 1024

#define HELP "SXE sandbox test program.\n" \
    "Usage:\n   sandboxed\n" \
    "      Create a file \"test.txt\" with text \"Hello world\"\n" \
    "   sandboxed <filenames>\n" \
    "      Attempt to stat, open for read and open for write each file\n" \
    "      or if a directory, fork a new process, cd into it and continue\n" \
    "      hence a forking test can be done by creating many directories\n" \
    "   sandboxed <#port number> [ip.addr] [message]\n" \
    "      try to connect on this port number\n"

#define SIMPLE_MESSAGE "Hello world\n"

#define DEFAULT_ADDR "127.0.0.1"

#define NET_MESSAGE "GET / HTTP/1.0\r\n\r\n"

int indent = 0;


/*!
  Error and warning report functions.

  These are annotated with the process id, and indented to aid
  in diagnosis.
*/
void _message( const char *msg, const char *fn, FILE *strm )
{
    char buf[BUF_SZ];
    int i = indent;
    memset( buf, 0, BUF_SZ );
    for ( i = 0; i < indent && i < BUF_SZ; ++i )
        buf[i] = ' ';
    if ( fn )
        fprintf( strm, "%sPID %d - %s : %s\n", buf, getpid(), msg, fn );
    else
        fprintf( strm, "%sPID %d - %s\n", buf, getpid(), msg );
}

void fatal( const char *msg )
{
    _message( msg, 0, stderr );
    exit( 1 );
}

void warning( const char *msg )
{
    _message( msg, 0, stderr );
}

void message( const char *msg )
{
    _message( msg, 0, stdout );
}

void file_message( const char *msg, const char *fn )
{
    _message( msg, fn, stdout );
}

/*!
  Send an internet message to \a port on the default host.

  If \a addr is non-zero, use that as the host.

  If \a msg is non-zero, use that as the message
*/
void send_message( int port, const char *addr, const char *msg )
{
    int sock;
    int rc;
    char buf[BUF_SZ];
    struct sockaddr_in client;
    struct sockaddr_in server;
    const int sa_sz = sizeof(struct sockaddr_in);
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = INADDR_ANY;
    server.sin_family = AF_INET;
    if ( addr == 0 )
        addr = DEFAULT_ADDR;
    server.sin_addr.s_addr = inet_addr( addr );
    server.sin_port = htons( port );
    file_message( "Creating socket", addr );
    if (( sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
        fatal( (char*)strerror( errno ));
    if ( connect( sock, (struct sockaddr*)&server, sa_sz ) == -1 )
        fatal( (char*)strerror( errno ));
    if ( msg == 0 )
        msg = NET_MESSAGE;
    int msg_sz = strlen( msg ) + 1;
    if ( send( sock, NET_MESSAGE, msg_sz, 0 ) == -1 )
        fatal( (char*)strerror( errno ));
    while (( rc = recv( sock, buf, BUF_SZ, 0 )))
        printf( buf );
    if ( rc == -1 )
        warning( (char*)strerror( errno ));
}

/*!
  Do a range of I/O processing on the files in \a f_names

  If a file is a directory, then fork a new process to
  do that directory.

  Do good error reporting on failure.
*/
void process_files( char **file_names, int file_count )
{
    int fd;
    int f_index;
    int f_ptr;
    int doit = 1;
    struct stat statbuf;
    pid_t proc = -1;
    int dealloc_file_names = 0;
    DIR *dir = 0;
    struct dirent *entry = 0;
    while ( doit-- )
    {
        for ( f_index = 0; f_index < file_count; ++f_index )
        {
            if ( strcmp( file_names[f_index], ".." ) == 0 )
                continue;
            if ( strcmp( file_names[f_index], "." ) == 0 )
                continue;
            file_message( "accessing for stat", file_names[f_index] );
            fd = stat( file_names[f_index], &statbuf );
            if ( fd == -1 )
            {
                perror( file_names[f_index] );
                continue;
            }
            /* follow into subdirs by forking - unless its a symlink */
            if ( S_ISDIR( statbuf.st_mode ) && ! S_ISLNK( statbuf.st_mode ))
            {
                if (( proc = fork()) == 0 )
                {
                    /* zero = child */
                    doit = 1;
                    indent += 3;
                    file_message( "to handle dir", file_names[f_index] );
                    if (( fd = chdir( file_names[f_index] )) == -1 )
                        fatal( (char*)strerror( errno ));
                    if (( dir = opendir( "." )) == NULL )
                        fatal( (char*)strerror( errno ));
                    file_count = 0;
                    while (( entry = readdir( dir ))) { file_count++; }
                    /* setup so file_names & file_count reflect new directory */
                    file_names = (char **)malloc( file_count * sizeof( char * ));
                    if ( file_names == NULL )
                        fatal( "allocating file names array" );
                    rewinddir( dir );
                    f_ptr = 0;
                    while (( entry = readdir( dir )) && f_ptr < file_count )
                        file_names[ f_ptr++ ] = (char*)strdup( entry->d_name );
                    closedir( dir );
                    dealloc_file_names = 1;
                    break;
                }
                if ( proc == -1 ) /* fork failed */
                    warning( (char*)strerror( errno ));
            }
            else
            {
                file_message( "accessing for read", file_names[f_index] );
                fd = open( file_names[f_index], O_RDONLY );
                if ( fd == -1 )
                    perror( file_names[f_index] );
                else
                    close( fd );
                file_message( "accessing for read/write", file_names[f_index] );
                fd = open( file_names[f_index], O_RDWR );
                if ( fd == -1 )
                    warning( (char*)strerror( errno ));
                else
                    close( fd );
                sleep( 5 );
            }
        }
    }
    if ( dealloc_file_names )
    {
        for ( f_index = 0; f_index < file_count; ++f_index )
            free( file_names[f_index] );
        free( file_names );
    }
}

/*!
  Main function

  Based on arguments to main, either
  \list
    \o access files
    \o send an internet message
    \o write one simple file
  \endlist
*/
int main( int argc, char *argv[] )
{
    int fd;
    printf( "%s (PID %d) running", argv[0], getpid() );
    if ( argc > 1 )
    {
        const char *ptr = argv[1];
        while ( *ptr && isdigit( *ptr++ )) {}
        if ( *ptr == '\0' )
            send_message( atoi( argv[1] ), argc > 2 ? argv[2] : NULL,
                        argc > 3 ? argv[3] : NULL );
        else
            process_files( argv+1, argc-1 );
    }
    else
    {
        printf( "Open and write \"test.txt\"" );
        fd = open( "test.txt", O_WRONLY | O_CREAT, S_IWUSR );
        if ( fd == -1 )
            fatal( (char*)strerror( errno ));
        write( fd, SIMPLE_MESSAGE, strlen( SIMPLE_MESSAGE ));
        close( fd );
    }
    return 0;
}
