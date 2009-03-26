#!/bin/sh

TOP=$PWD

test -z $SXE && SXE=`dirname $0`

# Like perl's die function
function die()
{
    echo "Fatal error: $1"
    exit 1 ;
}

test -L lids || die "expected \"lids\" symlink to current lids dir"

# Current versions used
LIDS=`readlink lids`

test -L $TOP/$LIDS/lidstools || die "expected \"$LIDS/lidstools\" symlink to current tools dir"
LIDSTOOLS=`readlink $LIDS/lidstools`

test -f $TOP/$LIDS.tar.gz || die "expected \"$TOP/$LIDS.tar.gz original source tarball"

test -d $TOP/$LIDS.modified.save && rm -Rf $TOP/$LIDS.modified.save
test -d $TOP/$LIDS.modified && mv -f $TOP/$LIDS.modified $TOP/$LIDS.modified.save
mv -f $TOP/$LIDS $TOP/$LIDS.modified
tar zxf $LIDS.tar.gz
cd $TOP/$LIDS/$LIDSTOOLS/src  || die "cd $LIDS/$LIDSTOOLS"
patch -p3 < $SXE/lids-2.2.1-2.6.13.patch       || die "applying patch lids-2.2.1-2.6.13.patch"
cd -
 
# Create the patch
diff -dur $LIDS $LIDS.modified | grep -v -e '^Only in' >$LIDS.patch

# Put things back
rm -Rf $LIDS
mv -f $TOP/$LIDS.modified $TOP/$LIDS

echo "Created $LIDS.patch"
