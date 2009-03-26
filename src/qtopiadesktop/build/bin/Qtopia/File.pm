package Qtopia::File;

use strict;
use warnings;

use File::Basename;
use File::stat;
use File::Path;
use File::Glob;
use File::Copy;
use Qtopia::Vars;
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
    symlink_file
    rmrf
    resolveHeader
    needCopy
    overwriteIfChanged
    fixpath

    $shadow
    $isWindows
);

our $VERSION = '0.01';

# imported variables
our $shadow;
our $isWindows;

# file is symlinked to dest
sub symlink_file
{
    my ($file, $dest, $copy) = @_;

    if ( !defined($copy) && ! -e $file ) {
        carp "WARNING: $file does not exist!";
        return;
    }

    my ( $ext ) = ( $file =~ /.*\.(.*)/ );
    if ( !$copy && defined($ext) && $ext eq "h" ) {
        #debugMsg( "magic_include " . $file . " " . $dest );
        my $print_line = "#include \"$file\"\n";
	my $delete_dest = 1;
	if ( -f $dest && ! -l $dest ) {
	    open DEST, "$dest" or die "Can't read $dest";
	    my $line = <DEST>;
	    close DEST;
	    if ( $line eq $print_line ) { 
		# Don't update the file (it hasn't changed)
		return;
	    }
	}

	if ( -d $dest && ! -l $dest ) {
	    rmrf($dest);
	} else {
	    unlink $dest;
	}

        mkpath(dirname($dest));
	open DEST, ">$dest" or die "Can't write $dest";
	print DEST $print_line;
	close DEST;
        my $s = stat($file);
        if ( defined($s) ) {
            my $now = $s->mtime;
            utime $now, $now, $dest;
        }
    } else {
	if (-d $dest && ! -l $dest) {
	    rmrf($dest);
	} else {
	    unlink $dest;
	}

        mkpath(dirname($dest));
	if ( $isWindows || $copy ) {
	    for ( $file, $dest ) {
		$_ = fixpath($_);
	    }
            #debugMsg( "copy " . $file . " " . $dest );
	    copy($file,$dest);
	} else {
            #debugMsg( "ln -sf " . $file . " " . $dest );
	    system("ln", "-sf", "$file", "$dest");
	}
    }
}

# rm -rf in perl (so that it works in Windows too)
sub rmrf
{
    my ( $path ) = @_;
    if ( -d $path && ! -l $path ) {
        # Since filehandles are global and this function calls recursively we have a slight problem.
        # Luckily we can tie a filehandle to the local scope using the 'local' keyword to avoid this problem.
        local *DIR;
        opendir(DIR, $path) or die "Can't opendir $path";
        for ( readdir(DIR) ) {
            if ( $_ eq "." or $_ eq ".." ) {
                next;
            }
            $_ = "$path/$_";
            if ( -d $_ && ! -l $_ ) {
                rmrf($_);
            } else {
                unlink($_) || die "Couldn't unlink $_";
            }
        }
        closedir DIR;
        rmdir $path || die "Couldn't rmdir $path";
    } elsif ( -e $path || -l $path ) {
        unlink($path) || die "Couldn't unlink $path";
    }
}

# Track down a "real" header
sub resolveHeader
{
    my ( $file ) = @_;
    for (;;) {
	open IN, $file or croak "Can't read $file";
	my $orig = <IN>; 
	if ( !defined($orig) ) {
	    warn "Qtopia::File::resolveHeader() undefined value read from $file";
	    return $file;
	}
	chomp $orig;
        # Allow "symlink" header files to indicate that the include is not the first line
        if ( $orig =~ /resolveHeader skip (\d+)/ ) {
            my $skip = $1;
            for ( my $i = 0; $i < $skip; $i++ ) {
                $orig = <IN>;
            }
        }
	close IN;
	if ( $orig =~ /^#include "(.*)"\s*$/ ) {
            my $nf = $1;
            if ( substr($1,0,1) ne "/" ) {
                $nf = dirname($file)."/".$1;
            }
            if ( -e $nf ) {
                $file = $nf;
            } else {
                last;
            }
        } else {
            last;
        }
    }
    return $file;
}

sub needCopy
{
    my ( $srcfile, $dest ) = @_;
    my $src_s = stat($srcfile);
    my $dest_s = stat($dest);
    return ( ! -f $srcfile || ! -f $dest || !defined($src_s) || !defined($dest_s) || $src_s->mtime > $dest_s->mtime );
}

sub overwriteIfChanged
{
    my ( $source, $dest ) = @_;
    open IN, "<$source" or croak "Can't open $source";
    my $file1 = join("", <IN>);
    close IN;
    my $overwrite = 1;
    if ( -f "$dest" ) {
        open IN, "<$dest" or croak "Can't open $dest";
        my $file2 = join("", <IN>);
        close IN;
        if ( $file1 eq $file2 ) {
            $overwrite = 0;
        }
    }
    if ( $overwrite ) {
        open OUT, ">$dest" or croak "Can't open $dest";
        print OUT $file1;
        close OUT;
    }
    unlink $source;
}

sub fixpath
{
    my ( $path ) = @_;
    if ( $isWindows && $path ) {
        $path =~ s,/,\\,g;
        $path = lc(substr($path, 0, 1)).substr($path, 1);
    }
    return $path;
}

sub unixpath
{
    my ( $path ) = @_;
    if ( $isWindows && $path ) {
        $path =~ s,\\,/,g;
        $path = lc(substr($path, 0, 1)).substr($path, 1);
    }
    return $path;
}

# cp -R in perl (so it works on Windows too)
sub cpR
{
    my ( $srcfile, $dest ) = @_;
    if ( -d $dest ) {
        $dest = "$dest/".basename($srcfile);
    }
    if ( -f $srcfile ) {
	if ( needCopy($srcfile, $dest) ) {
            #print "copy $srcfile $dest\n";
	    copy($srcfile, $dest);
	}
    } else {
	if ( ! -d $dest ) {
            #print "mkpath $dest\n";
	    mkpath($dest);
	}
        #print "glob($srcfile/*)\n";
        my @tocp = glob("$srcfile/*");
        foreach my $file ( @tocp ) {
            cpR($file, $dest);
        }
    }
}

# Make this file require()able.
1;
