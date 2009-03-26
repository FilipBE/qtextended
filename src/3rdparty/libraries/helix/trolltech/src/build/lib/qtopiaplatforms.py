# Author: Qt Extended
#
# Licensees holding a valid license agreement for the use of the
# Helix DNA code base may use this file in accordance with that license.
#
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
# Contact info@qtextended.org if any conditions of this licensing are
# not clear to you.
#

def initialize():

    AddPlatform(Platform(
        id = 'linux-2.2-libc6-arm9-cross-gcc4',
        platform = 'linux2',
        arch = 'arm',
        distribution_id = 'linux-2.2-libc6-arm9-cross-gcc4',
        family_list = ['unix', 'linux', 'linux2', 'linux-glibc-2.0',
                       'linux-arm', 'linux-2.2-libc6-arm9-cross-gcc4'] ))

    AddPlatform(Platform(
        id = 'linux-2.2-libc6-xscale-cross-gcc32',
        platform = 'linux2',
        arch = 'xscale',
        distribution_id = 'linux-2.2-libc6-xscale-cross-gcc32',
        family_list = ['unix', 'linux', 'linux2', 'linux-glibc-2.0',
                       'linux-arm', 'linux-2.2-libc6-xscale-cross-gcc32'] ))

    AddPlatform(Platform(
        id = 'linux-2.2-libc6-armv5te-cross-gcc3.3-iwmmxt-vfp',
        platform = 'linux2',
        arch = 'armv5te',
        distribution_id = 'linux-2.2-libc6-armv5te-cross-gcc3.3-softfloat',
        family_list = ['unix', 'linux', 'linux2', 'linux-glibc-2.0',
                       'linux-arm', 'linux-2.2-libc6-armv5te-gcc3.3-softfloat'] ))

    AddPlatform(Platform(
        id = 'linux-2.2-libc6-gcc32-i586-gcc4',
        platform = 'linux2',
        arch = 'i386',
        distribution_id = 'linux-2.2-libc6-gcc32-i586',
        family_list = ['unix', 'linux', 'linux2','gcc3', 'linux-glibc-2.0',
                       'linux-2.2-libc6-i386'] ))

    AddPlatform(Platform(
        id = 'linux-2.2-libc6-gcc32-i686-i686fb',
        platform = 'linux2',
        arch = 'i386',
        distribution_id = 'linux-2.2-libc6-gcc32-i586',
        family_list = ['unix', 'linux', 'linux2','gcc3', 'linux-glibc-2.0',
                       'linux-2.2-libc6-i386'] ))

    AddPlatform(Platform(
        id = 'linux-2.6-libc6-arm11-cross-gcc4',
        platform = 'linux2',
        arch = 'arm',
        distribution_id = 'linux-2.6-libc6-arm11-cross-gcc4',
        family_list = ['unix', 'linux', 'linux2', 'linux-glibc-2.0',
                       'linux-arm', 'linux-2.6-libc6-arm11-cross-gcc4'] ))

    AddPlatform(Platform(
        id = 'linux-2.2-libc6-armv6j-cross-gcc32',
        platform = 'linux2',
        arch = 'arm',
        distribution_id = 'linux-2.2-libc6-armv6j-cross-gcc32',
        family_list = ['unix', 'linux', 'linux2', 'linux-glibc-2.0',
                       'linux-arm', 'linux-2.2-libc6-armv6j-cross-gcc32'] ))

