################################################################################
#
# This third party code implements fuser in perl. It's probably not portable to
# non-Linux systems.
#
# http://www.perlmonks.org/?node_id=36349
#
################################################################################

package File::Fuser;

use strict;
use vars qw( @ISA @EXPORT );
use File::Spec;
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(fuser);

sub fuser { 
    my($file) = File::Spec->rel2abs($_[0]);
    my($f,@pids);
    foreach(glob("/proc/*")) { 
        if ($_ =~ /^\/proc\/\d+$/) { 
            foreach $f(glob("$_/fd/*")) { 
                if ((reverse(File::Spec->splitpath($f)))[0] > 3 && -l $f) { 
                    if (readlink($f) eq $file) { 
                        push(@pids,(reverse(File::Spec->splitpath($_)))[0]);
                    }
                }
            }
        }
    }
    @pids;
}

1;
