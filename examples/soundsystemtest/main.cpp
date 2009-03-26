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
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>


int main(int /*argc*/, char ** /*argv*/) {

    printf("Check current kernel system volume:\n");
    int mixerHandle = ::open( "/dev/mixer", O_RDWR|O_NONBLOCK );
    if ( mixerHandle >= 0 ) {
        uint volume;
        ioctl( mixerHandle, MIXER_READ(SOUND_MIXER_VOLUME), &volume );
        ::close( mixerHandle );
        volume &= 0x0000ffff; // mask to only least sig 16 bits.
        uint left = volume & 0x00ff;
        uint right = (volume & 0xff00) >> 8;
        printf("Volume: %d, %d\n", left, right);
        if (left < 50 || right < 50)
            printf("System volume appears too low\n");
    } else
        printf("/dev/mixer not available\n");

    printf("Open device nonblocking: ");
    int fd;
    if ((fd = ::open("/dev/dsp", O_WRONLY|O_NONBLOCK)) != -1) {
	printf("true\n");

	int flags = fcntl(fd, F_GETFL);

	flags &= ~O_NONBLOCK;

	printf("Change device flags to blocking write: ");
	if (fcntl(fd, F_SETFL, flags) == 0) {
	    printf("true\n");
	} else {
	    printf("false\n");
	    ::close(fd);
	    return -1;
	}
    } else {
	printf("false\n");
	::close(fd);
	return -1;
    }

    // Test if setting size of device buffer.
    int v;
    v=0x10000 * 4 + 12;
    int o = v;
    printf("Set sound fragments %08x: ", v);
    if ( ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &v) )
	printf("false\n");
    else
	printf("true\n");
    if (o != v)
	printf("Driver reported alternate fragments %08x\n", v);

    // Test 16/8bit.

    v=AFMT_S16_LE;
    o = v;
    printf("Set audio format 16bit, little endian %08x: ", v);
    if ( ioctl(fd, SNDCTL_DSP_SETFMT, &v) )
	printf("false\n");
    else
	printf("true\n");
    if ( o != v )
	printf("Want format %d got %d\n", o, v);


    printf("Set stereo: ");
    v=1;
    o=v;
    if ( ioctl(fd, SNDCTL_DSP_STEREO, &v) )
	printf("false\n");
    else
	printf("true\n");
    if ( o != v )
	printf("Want stereo %d got %d", o, v);


    v=44100;
    o=v;
    printf("Set sound speed: ");
    if ( ioctl(fd, SNDCTL_DSP_SPEED, &v) )
	printf("false\n");
    else
	printf("true\n");
    if ( v != o )
	printf("Want speed %d got %d", o, v);

    printf("Can get avail space: ");
    audio_buf_info info;
    if ( ioctl(fd,SNDCTL_DSP_GETOSPACE,&info) )
	printf("false\n");
    else
	printf("true\n");

    ::close(fd);


    return 0;
}

