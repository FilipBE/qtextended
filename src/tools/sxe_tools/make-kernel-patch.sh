#!/bin/sh

TOP=$PWD

test -z $SXE && SXE=`dirname $0`

# Like perl's die function
function die()
{
    echo "Fatal error: $1"
    exit 1 ;
}

# Current versions used
test -L linux || die "expected \"linux\" symlink to current linux dir"
LINUX=`readlink linux`
test -L lids || die "expected \"lids\" symlink to current lids dir"
LIDS=`readlink lids`

test -f $TOP/$LIDS.tar.gz || die "expected \"$TOP/$LIDS.tar.gz original source tarball"
test -f $TOP/$LINUX.tar.bz2 || die "expected \"$TOP/$LINUX.tar.bz2 original source tarball"

test -d $TOP/$LINUX.modified.save && rm -Rf $TOP/$LINUX.modified.save
test -d $TOP/$LINUX.modified && mv -f $TOP/$LINUX.modified $TOP/$LINUX.modified.save
mv -f $TOP/$LINUX $TOP/$LINUX.modified
tar xjf $LINUX.tar.bz2

cd $TOP/$LINUX                                 || die "cd $LINUX"
patch -p1 < ../$LIDS/$LIDS.patch          || die "applying patch $LIDS/$LIDS.patch"

# Create the patch
diff -dur $LINUX/security $LINUX.modified/security | grep -v -e '^Only in' >$LINUX.patch

# Put things back
rm -Rf $LIDS
mv -f $TOP/$LIDS.modified $TOP/$LIDS

echo "Created $LINUX.patch"
