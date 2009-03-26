/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /home/cvsroot/qtopia2/src/3rdparty/libraries/gsm/gsm_destroy.c,v 1.1 2002/10/28 00:28:29 rweather Exp $ */

#include "gsm.h"

#	include	<stdlib.h>

void gsm_destroy (gsm S)
{
	if (S) free((char *)S);
}
