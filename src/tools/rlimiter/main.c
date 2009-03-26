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

#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* The rlimiter binary exists merely to setup process based resource limits
   and then exec into a sandboxed executable.  The expected argument list
   is: rlimiter execPath numResources [resourceType resourceLimit ]... [additional args]...

   execPath- must be the absolute path of the executable to be run.
   numResources - the number of resources to be limited.
   resourceType - is an integer, representing the resource type as per
                  sys/resource.h.  The currently supported type is RLIMIT_AS.
   resourceLimit - is a long representing limit of the resource
   additional args - commandline arguments to be passed to the sandboxed executable

   The first 3 arguments of the expected argument list are required and may be
   optionally followed by a number of resource type and limit pairs, which is again
   optionally followed by a number of arguments to be passed into the executable.
*/
int main( int argc, char *argv[] )
{
    const int POS_EXEC_PATH = 1;
    const int POS_NUM_RESOURCES = 2;
    const int MAX_PATH_LEN = 1024;

    if ( argc < 3 )
    {
        fprintf( stderr, "rlimiter: Invalid number of arguments, must have at least 3 "
        "args:rlimiter execPath numResources, followed by a number of resource type "
        "and limit pairs (if any) and any additional arguments\n" );
        int i;
        for ( i= 0; i < argc; ++i )
            fprintf( stderr, "rlimiter: arg %i: %s\n", i, argv[i] );
        exit( 1 );
    }

    char execPath[MAX_PATH_LEN];
    strncpy( execPath, argv[POS_EXEC_PATH], MAX_PATH_LEN );

    int numResources = atoi( argv[POS_NUM_RESOURCES] );
    if ( numResources < 0 || numResources > 100 )
    {
        fprintf( stderr, "rlimiter: Non-sensical value has been specified for number of resources "
                "argv[%i] = %i\n", POS_NUM_RESOURCES, numResources);
        exit( 1 );
    }

    if ( execPath[0] != '/' || strlen(execPath) < 2 )
    {
        fprintf( stderr, "rlimiter: Invalid execPath (i.e. argv[%i]): %s \nThe execPath "
                         "must always be an absolute path to an executable.  Sandboxed "
                         "application not lauanched\n", POS_EXEC_PATH,execPath );
        exit( 1 );
    }

    rlim_t limitValue;
    struct rlimit limit;
    char *resourceTypeStr = 0;
    int isError=0;
    int i;

    if ( numResources > 0 )
    {
        int posFirstResource = POS_NUM_RESOURCES + 1;
        for ( i = 0; i < numResources; ++i )
        {
            //assumption is made that the resource limit is a long.
            limitValue = atol( argv[posFirstResource + i + 1] );
            limit.rlim_cur = limitValue;
            limit.rlim_max = limitValue;
            switch( atoi(argv[posFirstResource + i]) )
            {
                case( RLIMIT_AS ):
                    //sanity check ensure limit not less than 1MB and not greater than 1GB
                    if ( limitValue <  1048576 || limitValue > 1073741824 )
                    {
                        fprintf( stderr, "rlimiter: non-sensical values for RLIMIT_AS have been specified. "
                                 "Sandboxed application will not be launched\n" );
                        exit( 1 );
                    }
                    if( (isError = setrlimit( RLIMIT_AS, &limit )) == -1 )
                        resourceTypeStr = "RLIMIT_AS";
                    break;
                default:
                    fprintf( stderr, "rlimiter: trying to limit invalid resource type: %s\n"
                             "proposed limit: %s\nSandboxed application will not be "
                             "launched\n", argv[posFirstResource+i], argv[posFirstResource+i+1] );
                    exit( 1 );
            }

            if ( isError == -1 )
            {
                fprintf( stderr, "rlimiter: Could not set %s to %s - %s \nSandboxed application "
                         "will not be launched", resourceTypeStr, argv[posFirstResource+i+1], strerror(errno) );
                exit( 1 );
            }
        }
    }

    //create new arguments array to be passed into execvp
    int newArgc = argc - 2 - (2 * numResources); //ignoring argv[0], numResources and all resource pairs
    char *newArgv[ newArgc + 1 ]; //+1 to allow room for null pointer terminator
    newArgv[0] = strrchr( execPath, '/' ) + 1; //the "new" argv[0] should be executable name
    for ( i = 1; i < newArgc; ++i )
        newArgv[i] = argv[ POS_NUM_RESOURCES + 2 * numResources + i ];
    newArgv[ newArgc ] = (char *)0;

    if( -1 == execvp( execPath, newArgv ) )
    {
        fprintf( stderr, "rlimiter: rlimiter could not exec %s: %s \nSandboxed application not launched.",
                execPath, strerror(errno) );
        exit( 1 );
    }
    return -1;
}
