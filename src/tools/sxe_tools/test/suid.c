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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "lidsif_usr.h"

#define LIST_LEN 50

static struct usr_key_entry zero_ent;

int keys_read = 0;
int keys_fd = 0;

char key_display[LIDS_KEY_SIZE*2+1];

static struct usr_key_entry k_ent_list[LIST_LEN];

const char *call_name = 0;

/*!
  sprintf key in hex

  The target buf should be [ key_len * 2 + 1 ] in size
*/
void hexstring( char *buf, const char* key, size_t key_len )
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

int read_all_keys()
{
    int res = 0;
    keys_fd = open( "/proc/lids/keys", O_RDONLY );
    if ( keys_fd == -1 )
    {
        perror( "opening /proc/lids/keys" );
        return 1;
    }
    else
    {
        res = read( keys_fd, k_ent_list, LIST_LEN * sizeof( struct usr_key_entry ));
        close( keys_fd );
    }

    keys_read = res / sizeof( struct usr_key_entry );
    printf( "Read %d key entries\n", keys_read );
    return 0;
}

int read_k_ent( int fd, struct usr_key_entry *k_ent_ptr )
{
    int res = 0;
    memset( k_ent_ptr, 0, sizeof( struct usr_key_entry ));
    res = read( fd, k_ent_ptr, sizeof( struct usr_key_entry ));
    if ( memcmp( k_ent_ptr, &zero_ent, sizeof( struct usr_key_entry )) != 0 )
        printf( "\tread buffer %p was altered\n", k_ent_ptr );
    if ( res == 0 ) /* eof */
    {
        fprintf( stderr, "\tRead zero bytes\n" );
        return 0;
    }
    if ( res < sizeof( struct usr_key_entry ))
    {
        fprintf( stderr, "\tShort read %d bytes\n", res );
        return 0;
    }
    return res;
}

int print_proc_key_list( const char* search_key )
{
    int i;
    for ( i = 0; i < keys_read; ++i )
    {
        if ( search_key &&
                memcmp( k_ent_list[i].key, search_key, LIDS_KEY_SIZE ) != 0 )
            continue;
        printf( "Record %d\n", i );
        hexstring( key_display, k_ent_list[i].key, LIDS_KEY_SIZE );
        printf( "\tKey:\t%s\n", key_display );
        printf( "\tDev-maj:\t%i\n", major(k_ent_list[i].dev) );
        printf( "\tDev-min:\t%i\n", minor(k_ent_list[i].dev) );
        printf( "\tInode:\t%lu\n", k_ent_list[i].ino );
    }
    return i;
}

int print_proc_key( const char *path )
{
    int fd;
    char proc_key[LIDS_KEY_SIZE];
    int res = 0;
    fd = open( path, O_RDONLY );
    if ( fd == -1 )
    {
        perror( path );
        return 1;
    }
    memset( proc_key, 0, LIDS_KEY_SIZE );
    res = read( fd, proc_key, LIDS_KEY_SIZE );
    if ( res < LIDS_KEY_SIZE )
    {
        fprintf( stderr, "short read %d bytes", res );
        res = 1;
    }
    hexstring( key_display, proc_key, LIDS_KEY_SIZE );
    printf( "Path %s\n", path );
    printf( "\tKey:\t%s\n\n", key_display );
    if ( print_proc_key_list( proc_key ))
        res = 1;
    close( fd );
    return res;
}

void die( const char *msg, const char *file )
{
    if ( msg )
    {
        if ( file )
            fprintf( stderr, "Fatal error: %s - %s\n", file, msg );
        else
            fprintf( stderr, "Fatal error: %s\n", msg );
    }
    else
    {
        fprintf( stderr, "Fatal error\n" );
    }
    exit( 1 );
}


void usage( const char *msg, const char *file )
{
    fprintf( stderr, "\nUsage: %s <path/to/suid_binary>\n", call_name );
    die( msg, file );
}

void print_arg_files( int arg_count, char **arg_files )
{
    int fno;
    int fd;
    char test_file_line[1024];

    if ( arg_count > 2 ) /* subsequent args are test files */
    {
        for ( fno = 2; fno < arg_count; ++fno )
        {
            printf( "\tTesting opening file %s\n", arg_files[fno] );
            fd = open( arg_files[fno], O_RDONLY );
            if ( fd == -1 )
            {
                perror( arg_files[fno] );
            }
            else
            {
                memset( test_file_line, 0, 1024 );
                while ( read( fd, test_file_line, 1023 ) > 0 )
                {
                    printf( "\t\t%s", test_file_line );
                    memset( test_file_line, 0, 1024 );
                }
            }
        }
        printf( "\n\n" );
    }
}


/* ***********************************************************************
   *****
   *****    main function
   *****
   ***********************************************************************
   */
int main( int argc, char *argv[] )
{
    call_name = argv[0];
    struct stat target_stat;
    struct usr_key_entry target_kent;
    int fd;
    int keys_rc = 0;

    printf( "Testing suid binaries\n" );
    if ( argc < 2 )
        usage( 0, 0 );

    if ( stat( argv[1], &target_stat ) != 0 )
        die( strerror( errno ), argv[1] );

    /* ***********************************************************************
     *****
     *****    tests before transit to new SUID domain
     *****
     ***********************************************************************
     */
    printf( "Pre-SUID condition tests\n" );

    print_arg_files( argc, argv );

    printf( "\tTesting reading /proc/self/lids_key\n" );

    memset( &zero_ent, 0, sizeof( struct usr_key_entry ));
    print_proc_key( "/proc/self/lids_key" );


    printf( "\tTesting reading /proc/lids/keys\n" );

    if ( read_all_keys() == 0 )
        print_proc_key_list( 0 );

    /* ***********************************************************************
     *****
     *****    attempt transit via suid
     *****
     ***********************************************************************
     */
    fd = open( "/proc/lids/suid", O_WRONLY );
    if ( fd == -1 )
        die( strerror( errno ), "open /proc/lids/suid" );
    target_kent.ino = target_stat.st_ino;
    target_kent.dev = target_stat.st_dev;
    keys_rc = write( fd, &target_kent, sizeof( struct usr_key_entry ));
    if ( keys_rc < sizeof( struct usr_key_entry ))
        die( strerror( errno ), "write /proc/lids/suid" );
    close( fd );

    /* ***********************************************************************
     *****
     *****    tests after transit to new SUID domain
     *****
     ***********************************************************************
     */
    printf( "Post-SUID condition tests\n" );

    print_arg_files( argc, argv );

    printf( "\tTesting reading /proc/self/lids_key\n" );

    print_proc_key( "/proc/self/lids_key" );


    printf( "\tTesting reading /proc/lids/keys\n" );

    if ( read_all_keys() == 0 )
        print_proc_key_list( 0 );

    printf( "suid test exiting\n" );
    return 0;
}
