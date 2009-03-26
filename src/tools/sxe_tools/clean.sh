#!/bin/sh

# Like perl's die function
function die()
{
    echo "Fatal error: $1"
    exit 1;
}

# The operations need root privileges
test `id -u` == 0 || die "Run this script as root"

# Clean up all SXE temporary files
rm -f *.bak *.log *.gz *.bz2 *.asc *.sign progress dev.map

if [ -d target ]; then
    umount target 2>&1 >/dev/null
    rmdir target
fi
