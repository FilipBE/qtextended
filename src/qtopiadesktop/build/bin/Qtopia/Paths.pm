package Qtopia::Paths;

use strict;
use warnings;

use Cwd;
use File::Basename;
use Qtopia::Cache;
use Qtopia::Vars;
use Qtopia::File;
use Carp;
#perl2exe_include Carp::Heavy
$Carp::CarpLevel = 1;

require Exporter;
our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Qtopia::opt ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
    $QPEDIR
    $SDKROOT
    $depotpath
    $QTEDIR
    $DQTDIR
    $qt_depot_path
    $cwd
    $HOST_QT
    $HOST_QT_LIBS
    $HOST_QT_BINS
    $HOST_QMAKE
    $TARGET_QMAKE
    $qbs_bin

    $shadow
);

our $VERSION = '0.01';

# imported variables
our $shadow;


use constant TRACE => 0;
use constant DEBUG => 0;

# PATH variables
our $QPEDIR;
our $SDKROOT;
our $depotpath;
our $QTEDIR;
our $DQTDIR;
our $qt_depot_path;
our $cwd = Qtopia::File::fixpath(getcwd());
our $HOST_QT; # The path to Qt that the host tools use (may or may not be built by us)
our $HOST_QT_LIBS; # The path to Qt libraries
our $HOST_QT_BINS; # The path to Qt binaries
our $HOST_QMAKE; # The path to qmake
our $TARGET_QMAKE; # The path to qmake
our $qbs_bin;

our %cache_vars = (
    "QTOPIA_DEPOT_PATH" => \$depotpath,
    "QPEDIR" => \$QPEDIR,
    "SDKROOT" => \$SDKROOT,
    "QT_DEPOT_PATH" => \$qt_depot_path,
    "QTEDIR" => \$QTEDIR,
    "DQTDIR" => \$DQTDIR,
    "HOST_QT" => \$HOST_QT,
    "HOST_QT_LIBS" => \$HOST_QT_LIBS,
    "HOST_QT_BINS" => \$HOST_QT_BINS,
    "HOST_QMAKE" => \$HOST_QMAKE,
    "TARGET_QMAKE" => \$TARGET_QMAKE,
    "QBS_BIN" => \$qbs_bin,
);

sub get_paths()
{
    TRACE and print "Qtopia::Paths::get_paths()\n";

    if ( basename($0) eq "configure" || $0 =~ /configure\.exe$/i ) {
        # The build tree is here
        $QPEDIR = $cwd;

        # Find out where the depot is (based on the location of configure)
        $depotpath = undef;
        for my $dir (
            # Running src/qtopiadesktop/build/bin/configure
            dirname(dirname(dirname(dirname(dirname($0))))),
            # Running bin/configure.exe
            dirname(dirname($0)) ) {
            if ( -f "$dir/src/qtopiadesktop/build/bin/configure" ) {
                $depotpath = $dir;
                last;
            }
        }
        if ( !defined($depotpath) ) {
            die "Can't locate the depot!\n";
        }

        # Use real paths
        for ( $depotpath, $QPEDIR ) {
            if ( $_ ) {
                chdir $_ or die "Can't enter $_\n";
                $_ = Qtopia::File::fixpath(getcwd());
            }
        }

        # The SDK defaults to $QPEDIR (but it can be changed)
        $SDKROOT = $QPEDIR;
        $shadow = ( $QPEDIR ne $depotpath );
    } else {

        ###
        ### WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
        ###
        ### This code must be kept in sync with src/qtopiadesktop/build/bin/getpaths.sh
        ###
        ### WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
        ###

        # Try to figure out where the build tree is
        $QPEDIR = undef;
        $depotpath = undef;
        LOC: for my $check ( dirname($0), $ENV{PWD}, $cwd ) {
            # Ignore null/empty values
            next if ( !$check );
            DEBUG and print "Checking $check\n";
            # Locate config.cache so we can be sure that we've actually found QPEDIR
            while ( $check ) {
                DEBUG and print "Checking $check\n";
                if ( -f "$check/config.cache" ) {
                    $QPEDIR = $check;
                    last LOC;
                }
                if ( -f "$check/src/qtopiadesktop/build/bin/configure" ) {
                    DEBUG and print "Found depot path\n";
                    $depotpath = $check;
                    next LOC;
                }
                if ( $check eq dirname($check) ) {
                    next LOC;
                }
                $check = dirname($check);
            }
        }
        # The SDK defaults to $QPEDIR (but it can be changed)
        $SDKROOT = $QPEDIR;
        if ( !defined($QPEDIR) ) {
            my $msg = "ERROR: Could not locate the Qtopia build tree.\n";
            $msg .=   "       Did you run configure?\n";
            if ( $depotpath ) {
                my $altscript = $0;
                $altscript =~ s/^\Q$depotpath\E//;
                $altscript =~ s/^\///;
                $msg .= "       Please try running ".basename($0)." from the build tree.\n";
                $msg .= "       eg. /path/to/build/$altscript\n";
            }
            croak $msg;
        }
        # Use the values in config.cache
        my @cache_data = Qtopia::Cache::load("paths");
        for ( @cache_data ) {
            if ( /^paths\.([^=]+)=(.*)/ ) {
                my $var = $1;
                my $value = $2;
                if ( $value eq "undef" ) {
                    $value = undef;
                }
                if ( exists($cache_vars{$var}) ) {
                    my $ref = $cache_vars{$var};
                    DEBUG and print "$var = $value\n";
                    $$ref = $value;
                }
            }
        }
        $shadow = 1;
    }

    if ( $isWindows ) {
        $qbs_bin = "$SDKROOT/bin";
        $shadow = 0;
    } else {
        $qbs_bin = "$SDKROOT/src/qtopiadesktop/build/bin";
    }
}

sub write_config_cache()
{
    # Save the paths to config.cache so that other scripts can get them
    my $ret = "";
    for my $var ( keys %cache_vars ) {
        my $ref = $cache_vars{$var};
        my $value = $$ref;
        if ( !defined($value) ) {
            $value = "undef";
        }
        $ret .= "paths.$var=$value\n";
    }
    Qtopia::Cache::replace("paths", $ret);
}

# Make this file require()able.
1;
