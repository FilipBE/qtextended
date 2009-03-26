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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define STRIP_ARGS_MAX 255

int main( int argc, char **argv )
{
    char *teambuilder_system;
    char *tb_strip_list[STRIP_ARGS_MAX];
    char *tb_strip_args;
    char *iter;
    int listsize = 0;
    int i;
    char **argv2;
    int argc2;
    int ret;

    // Tokenize TB_STRIP_ARGS
    tb_strip_args = strdup(TB_STRIP_ARGS);
    iter = strtok(tb_strip_args, " ");
    if ( iter ) do {
        if ( listsize >= STRIP_ARGS_MAX ) {
            fprintf(stderr, "wrapper.c: ERROR: TB_STRIP_ARGS cannot have more than %d parameters!\n", STRIP_ARGS_MAX);
            return 1;
        }
        tb_strip_list[listsize++] = iter;
    } while ( (iter = strtok(0, " ")) );

    teambuilder_system = getenv("TEAMBUILDER_SYSTEM");
    if ( teambuilder_system && strcmp(teambuilder_system, TB_SYS) == 0 ) {
        // The job came back from teambulider (ie. wasn't handled by a remote host)
        // Run the real compiler, adding back the stripped arguments
        argv2 = malloc(sizeof(char*)*(argc+listsize+1));
        argv2[0] = TOOLCHAIN "/bin/" COMPILER;
        int argc2 = 1;
        for ( i = 0; i < listsize; ++i )
            argv2[argc2++] = tb_strip_list[i];
        for ( i = 1; i < argc; ++i )
            argv2[argc2++] = argv[i];
        argv2[argc2++] = 0;
        ret = execv(argv2[0], argv2);
    } else {
        // Fix TEAMBUILDER_SYSTEM and send the job off to teambulider
        setenv("TEAMBUILDER_SYSTEM", TB_SYS, 1);

        // Remove any arguments that break on remote machines
        argv2 = malloc(sizeof(char*)*(argc+1));
        argv2[0] = TB_DIR "/bin/" COMPILER;
        int argc2 = 1;
        for ( i = 1; i < argc; ++i ) {
            int j;
            int found = 0;
            for ( j = 0; j < listsize; j++ ) {
                if ( strcmp(argv[i], tb_strip_list[j]) == 0 ) {
                    found = 1;
                    break;
                }
            }
            if ( !found )
                argv2[argc2++] = argv[i];
        }
        argv2[argc2++] = 0;
        ret = execv(argv2[0], argv2);
    }

    return ret;
}

