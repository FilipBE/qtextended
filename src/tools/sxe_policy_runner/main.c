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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

/* Uncomment following line to prevent policy from being applied
   to any lib*.so files */
// #define DISABLE_LIB_POLICY 1

#define ID_MAX 255

#define INSTALLS_IX   0
#define POLICY_IX     1
#define MANIFEST_IX   2

#define INSTALLS "installs"
#define MANIFEST "manifest"
#define POLICY "sxe.policy"

#define DEFAULT_SXE_DATABASE_LEN 5
#define DEFAULT_SXE_DATABASE "/etc/"

/* Length of this string - its also the longest of these suffix paths */
#define DOMAINS_LEN 16
#define DOMAINS "/etc/sxe_domains"

/*
   The scripts in the DOMAINS directory are named SCRIPT_PREFIX + domain, eg:

       sxe_qtopia_phonecomm

   where "phonecomm" is the domain searched for
*/
#define SCRIPT_PREFIX "sxe_qtopia_"
#define SCRIPT_PREFIX_LEN 11

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 1024
#endif

/* Most installs will only have 2 paths */
#define MAX_Q_PATH 4

int q_path_count = 0;

char prog_name[MAX_PATH_LEN];

char qtopia_dir_path[MAX_PATH_LEN*MAX_Q_PATH];
char *qtopia_paths[MAX_Q_PATH];

char sxe_database_path[MAX_PATH_LEN];

char notional_install_path[MAX_PATH_LEN];

int nip_not_eq_dir_path = 0;

/*
    From src/libraries/qtopiasecurity/qpackageregistry.cpp

    This is the record format for the manifest file.
*/

struct IdBlock
{
    unsigned long long inode;
    unsigned long long device;
    unsigned char pad;
    unsigned char progId;
    unsigned short installId;
    unsigned int keyOffset;
    long long install_time;
};

/*
   Format of sxe.policy file:

[34]         -- program id
pim          -- domain name
window
qdl
beaming
phonecomm
pictures
msg
docapi
cardreader
camera
pictures
mediarecorder
[26]
phonecomm
window
[6]
window
alarm
datetime
screensaver
launcher

*/

/* some of these fields are not used and are deprecated*/
struct domain_run
{
    const char *dom;      /* the name of the domain for this run (Deprecated)        */
    int fds[3];           /* the file descriptors as above                           */
    FILE *streams[2];     /* file streams for policy and installs files              */
    const char *script;   /* the path to the script to run (Deprecated)              */
    int prog_ids[ID_MAX]; /* list of M prog ids which have domain "dom"(Deprecated)  */
    int ids_count;        /* length M of program_ids list (Deprecated)               */
};

void usage( const char *msg )
{
    fprintf( stderr, "Usage: %s /path/to/Qtopia1:/path/to/Qtopia2 [/install/path]\n\n", prog_name );
    fprintf( stderr, "\t/path/to/Qtopia1 must contain etc/manifest, etc\n\n" );
    fprintf( stderr, "\t/install/path is config time install path (if different from /path/to/Qtopia1)\n\n" );
    if ( qtopia_dir_path )
        fprintf( stderr, "\t\tpath: %s\n\n", qtopia_dir_path );
    if ( msg )
        fprintf( stderr, "%s - exiting...\n", msg );
    exit( 1 );
}

/*
   scan through buf, and remove trailing whitespace by writing a nul '\0'
   into \a buf, after the last non-space character.

   return a pointer to the first non-space character in the string
   or if the string contains only spaces, return 0
*/
char *trimmed( char *buf )
{
    char *str;
    char *end_str;
    str = buf;
    while ( *str && isspace( *str ))
        ++str;
    if ( *str == '\0' )
        return 0;
    end_str = str;
    while ( *end_str && !isspace( *end_str ))
        end_str++;
    *end_str = '\0';
    return str;
}

/*
   Try to stat the path in \a path_buf.  If the file at that path
   cannot be stat'ed, then replace the portion of the path_buf
   which matches notional_install_path with each of the elements
   of qtopia_paths in turn (except not the first element if that
   was the same as notional_install_path), and try to stat
   the resulting path_buf.

   Return 0 on success (one of the stats was OK) or non-zero
   on failure.

   If \a bin_stat is non-zero then that stat struct is used
   to hold the stat information.

   Prints warnings on failure, and if file is non-executable,
   or not a regular file.

   Note that path_buf may be modified in all cases.
*/
int get_actual_path( char *path_buf, struct stat *bin_stat )
{
    struct stat bin_stat_buf;
    int rc;
    char rel_buf[MAX_PATH_LEN];
    char *set_rel_buf = 0;
    char errbuf[MAX_PATH_LEN];
    int rel_len = 0;
    int path = nip_not_eq_dir_path ? 0 : 1;

    if( bin_stat == 0 )
        bin_stat = &bin_stat_buf;

    /* printf( "trying %s for actual path\n", trimmed( path_buf )); */
    while (( rc = stat( trimmed( path_buf ), bin_stat )) == -1 )
    {
        /* perror( path_buf ); */
        if ( path >= q_path_count )
            break;
        if ( !set_rel_buf )
        {
            set_rel_buf = strstr( path_buf, notional_install_path );
            if ( !set_rel_buf )
            {
                sprintf( errbuf, "%s does not start with install path %s",
                        path_buf, notional_install_path );
                usage( errbuf );
            }
            rel_len = strlen( notional_install_path );
            strncpy( rel_buf, path_buf + rel_len, MAX_PATH_LEN );
        }
        strncpy( path_buf, qtopia_paths[path++], MAX_PATH_LEN );
        strncat( path_buf, rel_buf, MAX_PATH_LEN );
        /* printf( "trying %s for actual path\n", trimmed( path_buf )); */
    }
    /* is a regular file and everyone can execute it, or warn */
    if ( rc == 0 )
    {
        if ( !S_ISREG( bin_stat->st_mode ))
            fprintf( stderr, "\t\twarning - not a regular file: --->>>>%s<<<<---\n", path_buf );
        if ( !( bin_stat->st_mode & S_IXOTH ))
            fprintf( stderr, "\t\twarning - no execute other perms for --->>>>%s<<<<---\n", path_buf );
    }
    else
    {
        fprintf( stderr, "\t\twarning - could not find file --->>>>%s<<<<--- in qtopia path\n", path_buf );
    }
    return rc;
}

/*
   Run the script stored in run, passing binary as the env var "BIN"
   (This function is deprecated)
*/
void run_scripts_for_binary( struct domain_run *run, const char *bin )
{
    char binary[MAX_PATH_LEN];
    strncpy( binary, bin, MAX_PATH_LEN );
    int rc = 0;
#ifdef DISABLE_LIB_POLICY
    char *libptr = 0;
    char *soptr = 0;
    libptr = strstr( bin, "lib" );
    soptr = strstr( bin, ".so" );
    if ( libptr && soptr && libptr < soptr )
        return;
#endif
    if ( get_actual_path( binary, 0 ))
        return;
    /* script looks for the name of the binary in the ${BIN} env var */
    setenv( "BIN", binary, 1 );
    /* printf( "Running %s - BIN = %s -disabled for debugging\n", run->script, binary ); */
    rc = system( run->script );
    if ( rc == -1 )
    {
        fprintf( stderr, "Could not fork shell to exec %s\n", run->script );
    }
    else
    {
        if ( WIFSIGNALED( rc ))
        {
            /* died due to a signal */
            fprintf( stderr, "%s caught signal %d\n", run->script, WTERMSIG( rc ));
        }
        else if ( WIFEXITED( rc ))
        {
            /* has a valid exit status */
            if ( WEXITSTATUS( rc ) != 0 )
            {
                fprintf( stderr, "%s returned result code %d\n", run->script, WEXITSTATUS( rc ));
            }
        }
    }
}

/*
   (This function is deprecated)
*/
void clear_run( struct domain_run *dr )
{
    dr->ids_count = 0;
}


/*
   Pre-req's:
   - ids_count & prog_ids zero'ed out
   - run->dom set to desired domain string
   - Opened stream on the policy file in run->streams[POLICY_IX]

   Find all program ids which have the domain run->dom;
   (this function is deprecated)
*/
void get_prog_ids_for_domain( struct domain_run *run )
{
    char buf[MAX_PATH_LEN];
    int cur_prog_id = -1;
    int line_count = 0;

    /* read thru the file */
    rewind( run->streams[POLICY_IX] );
    while ( fgets( buf, MAX_PATH_LEN, run->streams[POLICY_IX] ))
    {
        line_count++;
        if ( buf[0] == '[' ) /* is a prog id */
        {
            cur_prog_id = atoi( &buf[1] );
        }
        else     /* is a domain */
        {
            if ( strcmp( run->dom, trimmed( buf )) == 0 )
            {
                /* if buf matches our domain, check alls well, then add
                   the current prog id to our list */
                if ( cur_prog_id < 0 || cur_prog_id > ID_MAX )
                {
                    fprintf( stderr, "file: %s - line: %d - prog id %i bad for %s",
                            POLICY, line_count, cur_prog_id, buf );
                    exit( 1 );
                }
                run->prog_ids[ run->ids_count++ ] = cur_prog_id;
            }
        }
    }
}

/*
   pre-reqs:
   - run->prog_ids array contains prog_ids to search for
   - fds[MANIFEST_IX] contains valid file descriptor for manifest file
   - streams[INSTALLS_IX] contains valid stream for installs file
   - binaries and bin_count zero'ed out

   Search for prog_ids in the manifest file and find corresponding
   install ids.  Look up install ids in installs file to find binaries
   and add to binaries list.
   (This function is deprecated)
*/
void process_binaries_for_prog_ids( struct domain_run *run )
{
    int p, i;
    char pad;
    int install_ids[100];
    int install_count = 0;
    struct IdBlock man_rec;
    unsigned short install_id;
    int line_no = 0;
    char buf[MAX_PATH_LEN];
    int checked_prog_count = 0;
    int check_prog_ids[ID_MAX];
    int checked_install_count = 0;
    int check_install_ids[100];
    int end_buf;

    /* track if we got all the ids matched or if something went wrong */
    memset( check_prog_ids, 0, ID_MAX * sizeof( int ));
    memset( check_install_ids, 0, 100 * sizeof( int ));

    /* sanity checks */
    if ( run->ids_count <= 0 )
    {
        fprintf( stderr, "On run %s, no prog ids - exiting\n", run->dom );
        return;
    }
    lseek( run->fds[MANIFEST_IX], 0L, SEEK_SET );
    while ( read( run->fds[MANIFEST_IX], &man_rec, sizeof( struct IdBlock )))
    {
        for ( p = 0; p < run->ids_count; ++p )
        {
            if ( man_rec.progId == run->prog_ids[p] )  /* found one we're interested in */
            {
                install_ids[install_count++] = man_rec.installId;
                check_prog_ids[p] = 1;                 /* tick that one off */
                checked_prog_count++;
            }
        }
    }
    if ( checked_prog_count < run->ids_count )         /* didn't match them all */
    {
        fprintf( stderr, "Domain %s had %d program ids not listed in %s:",
                run->dom, run->ids_count - checked_prog_count, MANIFEST );
        for ( p = 0; p < run->ids_count; ++p )
            if ( !check_prog_ids[p] )                  /* missing */
                fprintf( stderr, "\t\t%d\n", run->prog_ids[p] );
    }
    rewind( run->streams[INSTALLS_IX] );
    while( fread( &install_id, sizeof( install_id ), 1, run->streams[INSTALLS_IX] ))
    {
        line_no++;
        pad = fgetc( run->streams[INSTALLS_IX] );
        if ( !pad == ':' )
            fprintf( stderr, "expected \":\" reading installs - line %d", line_no );
        fgets( buf, MAX_PATH_LEN, run->streams[INSTALLS_IX] );
        end_buf = strlen( buf );
        for ( i = 0; i < install_count; ++i )
        {
            if ( install_ids[i] == install_id )
            {
                check_install_ids[i] = 1;  /* tick that one off */
                checked_install_count++;
            }
        }
    }
    if ( checked_install_count < install_count )         /* didn't match them all */
    {
        fprintf( stderr, "Domain %s had %d install ids not listed in %s:",
                run->dom, install_count - checked_install_count, INSTALLS );
        for ( i = 0; i < install_count; ++i )
            if ( !check_install_ids[i] )                  /* missing */
                fprintf( stderr, "\t\t%d\n", install_ids[i] );
    }
}

/*
   check for errors while managing the manifest file
*/
int __check_error( int rc, off_t pos )
{
    if ( rc < (int)sizeof( struct IdBlock ))
    {
        fprintf( stderr, "bad read on manifest file" );
        if ( rc < 0 )
            perror( "err" );
        return 1;
    }
    if ( pos == (off_t)-1 )
    {
        perror( "seeking manifest file" );
        return 1;
    }
    return 0;
}

/*
   pre-reqs:
   - fds[MANIFEST_IX] contains valid file descriptor for manifest file
   - streams[INSTALLS_IX] contains valid stream for installs file

   Iterate over all binaries in the installs list, and update their device
   and inode numbers in the manifest file.
*/
void update_dev_and_ino( struct domain_run *run )
{
    char path_buf[MAX_PATH_LEN];
    struct stat bin_stat;
    unsigned short install_id;
    struct IdBlock man_rec;
    int man_updated_cnt = 0;
    char pad;
    off_t pos;
    int line_no = 0;
    int rc;

    while( fread( &install_id, sizeof( install_id ), 1, run->streams[INSTALLS_IX] ))
    {
        line_no++;
        pad = fgetc( run->streams[INSTALLS_IX] );
        if ( !pad == ':' )
            fprintf( stderr, "expected \":\" reading installs - line %d", line_no );
        fgets( path_buf, MAX_PATH_LEN, run->streams[INSTALLS_IX] );
        if ( get_actual_path( path_buf, &bin_stat ))
        {
            fprintf( stderr, "error updating manifest %s : %s\n",
                    path_buf, strerror( errno ));
        }
        else
        {
            lseek( run->fds[MANIFEST_IX], 0L, SEEK_SET );
            pos = 0;
            while (( rc = read( run->fds[MANIFEST_IX], &man_rec, sizeof( struct IdBlock ))) > 0 )
            {
                if ( rc < (int)sizeof( struct IdBlock ))
                    break;
                if ( man_rec.installId == install_id )  /* found one we're interested in */
                {
                    if ( man_rec.inode != bin_stat.st_ino || man_rec.device != bin_stat.st_dev )
                    {
                        /* needs updating, so seek back */
                        pos = lseek( run->fds[MANIFEST_IX], pos, SEEK_SET );
                        if ( pos == (off_t)-1 )
                            break;
                        man_rec.inode = bin_stat.st_ino;
                        man_rec.device = bin_stat.st_dev;
                        rc = write( run->fds[MANIFEST_IX], &man_rec, sizeof( struct IdBlock ));
                        if ( rc < (int)sizeof( struct IdBlock ))
                            break;
                        ++man_updated_cnt;
                    }
                    break;
                }
                pos += sizeof( struct IdBlock );
            }
            if ( rc )
                __check_error( rc, pos );
            else
                if ( man_rec.installId != install_id ) /* didn't find it */
                    fprintf( stderr, "Could not find install %d for %s - no update\n",
                            install_id, path_buf );
        }
    }
    printf( "\tUpdated %d records in manifest\n\n", man_updated_cnt );
}


void sanity_checks( struct domain_run *run )
{
    struct IdBlock man_rec;
    if ( run->fds[MANIFEST_IX] == -1 )
    {
        fprintf( stderr, "Fatal: Unexpected manifest file null\n" );
        exit( 1 );
    }
    if ( run->streams[INSTALLS_IX] == NULL )
    {
        fprintf( stderr, "Fatal: Unexpected installs file null\n" );
        exit( 1 );
    }
    if ( sizeof( man_rec.inode ) != 8 )
    {
        fprintf( stderr, "Fatal: Platform has wrong manifest inode field size\n" );
        exit( 1 );
    }
    if ( sizeof( man_rec.device ) != 8 )
    {
        fprintf( stderr, "Fatal: Platform has wrong manifest inode field size\n" );
        exit( 1 );
    }
}

int main( int argc, char *argv[] )
{
    /* the root path of all data and script files */
    struct stat qtopia_dir;
    int dir_path_len = 0;
    int rc = 0;
    int i;

    /* 3 cross-related files form the database */
    char *data_paths[3];
    char installs_path[ MAX_PATH_LEN ];
    char manifest_path[ MAX_PATH_LEN ];
    char policy_path[ MAX_PATH_LEN ];

    struct domain_run run;

    /* handle the qtopia paths */
    char errbuf[MAX_PATH_LEN];
    char *colon_sep;

    strncpy( prog_name, argv[0], MAX_PATH_LEN );

    if ( argc != 2 && argc != 3 )
        usage( "Wrong number of arguments" );

    /* split the qtopia_dir_path into seperate strings on ':' */
    memset( qtopia_dir_path, 0, MAX_PATH_LEN*MAX_Q_PATH );
    strncpy( qtopia_dir_path, argv[1], MAX_PATH_LEN*MAX_Q_PATH - 1 );
    colon_sep = qtopia_dir_path;
    qtopia_paths[q_path_count++] = qtopia_dir_path;
    while( *colon_sep )
    {
        if ( *colon_sep == ':' )
        {
            *colon_sep = '\0';
            qtopia_paths[q_path_count++] = colon_sep + 1;
        }
        ++colon_sep;
    }

    if ( argc == 3 )
    {
        strncpy( notional_install_path, argv[2], MAX_PATH_LEN );
        nip_not_eq_dir_path = strcmp( notional_install_path,
                qtopia_dir_path );
    }
    else
    {
        strncpy( notional_install_path, qtopia_dir_path, MAX_PATH_LEN );
    }

    for ( i = 0; i < q_path_count; ++i )
    {
        rc = stat( qtopia_paths[i], &qtopia_dir );
        if ( rc != 0 )
        {
            sprintf( errbuf, "Cannot stat %s", qtopia_paths[i] );
            usage( errbuf );
        }
        if ( !S_ISDIR( qtopia_dir.st_mode ))
        {
            sprintf( errbuf, "%s is not a directory", qtopia_paths[i] );
            usage( errbuf );
        }
    }

    /*  We only look in the first element of the paths for the conf files */
    /*  DOMAINS path is the longest, has to be room  */
    dir_path_len = strlen( qtopia_dir_path );
    if ( dir_path_len > MAX_PATH_LEN - DOMAINS_LEN )
        usage( "Too long" );

    /* Find the SXE Database path */
    char *sxe_db_env = getenv("SXE_DATABASE");
    size_t env_db_len = strlen( sxe_db_env );
    if (env_db_len > MAX_PATH_LEN - DOMAINS_LEN)
        usage("SXE_DATABASE too long");

    if (sxe_db_env == NULL) {
        strncpy(sxe_database_path, qtopia_dir_path, dir_path_len+1);
        strncat(sxe_database_path, DEFAULT_SXE_DATABASE, DEFAULT_SXE_DATABASE_LEN);
    } else {
        strncpy(sxe_database_path, sxe_db_env, env_db_len+1);
        if (sxe_database_path[env_db_len-1] != '/') 
            strcat(sxe_database_path, "/");
    }

    size_t sxe_database_path_len = strlen(sxe_database_path);

    data_paths[INSTALLS_IX] = installs_path;
    data_paths[MANIFEST_IX] = manifest_path;
    data_paths[POLICY_IX] = policy_path;
    for ( i = 0; i < 3; ++i )
    {
        strncpy( data_paths[i], sxe_database_path, sxe_database_path_len+1 );
        strncat( data_paths[i], i == INSTALLS_IX ? INSTALLS : ( i == MANIFEST_IX ? MANIFEST : POLICY ), DOMAINS_LEN );
        run.fds[i] = open( data_paths[i], O_RDWR );
        if ( run.fds[i] == -1 )
        {
            perror( data_paths[i] );
            usage( "Could not open data file" );
        }
        if ( i != MANIFEST_IX )  /* binary records, no stream for manifest file */
        {
            run.streams[i] = fdopen( run.fds[i], "r" );
            if ( run.streams[i] == NULL )
            {
                perror( data_paths[i] );
                usage( "Could not read from data file" );
            }
        }
    }

    sanity_checks( &run );

    printf( "SXE policy runner updating database\n" );

    update_dev_and_ino( &run );

    exit( 0 );
}
