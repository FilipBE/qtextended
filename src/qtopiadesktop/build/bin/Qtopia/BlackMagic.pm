package Qtopia::BlackMagic;

use strict;
use warnings;

use File::Basename;
use Hash::Ordered;
use Qtopia::Paths;
use Qtopia::Opt;

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
);

our $VERSION = '0.01';

# imported variables
our $depotpath;


use constant DEBUG => 0;

my %projectCache;

sub process_variables($$)
{
    my ( $input, $vars ) = @_;
    $_ = $input;

    if ( /^\s*([A-Za-z0-9_.]+)\s*([\+\*]=|-=|=)(.*)/ ) {
        my $var = $1;
        my $op = $2;
        my $line = $3;
        # I can't handle variables!
        return if ( $line =~ /\$\$/ );
        #DEBUG and print "$_:\nVAR $var OP $op LINE $line\n";
        my @values;
        $line =~ s/^\s+//;
        $line =~ s/\s+$//;
        if ( $line =~ /"\s+"/ || $line =~ /^"/ ) {
            $line =~ s/^"//;
            $line =~ s/"$//;
            @values = split(/"\s+"/, $line);
        } else {
            @values = split(/\s+/, $line);
        }
        #DEBUG and print "$var=".join(" ", @values)."\n";
        if ( !exists($$vars{$var}) || $op eq "=" ) {
            $$vars{$var} = newhash();
        }
        if ( $op eq "-=" ) {
            map {
                return if ( !$_ || !exists($$vars{$var}->{$_}) );
                delete($$vars{$var}->{$_});
            } @values;
        } else {
            map { $$vars{$var}->{$_}++ } @values;
        }
        DEBUG and print "$var=".join(" ", keys %{$$vars{$var}})."\n";
    }
}

sub parse_pro_file
{
    my ( $root, $proFile, $prohash ) = ( @_ );

    my $vars;
    if ( $prohash ) {
        # use this storage instead (to load multiple files into one storage)
        $vars = $prohash;
    } else {
        # We need to emulate qmake's processing which means taking values from tree_config.pri
        my $tree_config = "$root/tree_config.pri";
        $tree_config =~ s/^\Q$QPEDIR\E/$depotpath/;
        if ( !exists($projectCache{$root}) ) {
            $projectCache{$root} = newhash();
            # The all-pervasive default (probably comes in from mkspecs)
            process_variables("CONFIG+=qt", $projectCache{$root});
            DEBUG and print "\n\n";
            if ( -f $tree_config ) {
                parse_pro_file($root, $tree_config, $projectCache{$root});
            }
        }

        # Just copy them since other projects might also need to do this
        $vars = deep_copy($projectCache{$root});
    }

    # Clear these out and ensure they exist
    $$vars{PROJECT_TYPE} = newhash();
    $$vars{QTOPIA_DEPENDS} = newhash();
    $$vars{DEP_CMDS} = newhash();
    $$vars{DEP_I_CMDS} = newhash();

    DEBUG and print "Reading $proFile\n";
    open PRO, "$proFile" or die "Can't read $proFile";
    my @data = <PRO>;
    close PRO;
    while ( @data ) {
        $_ = shift(@data);
        # Handle a special message for this parser
        if ( s/^\s*#BlackMagic\.pm:// ) {
            DEBUG and print $_;
            # Dump the contents of another file into the processing stream
            if ( /include (.*)/ ) {
                my $file;
                eval "\$file = \"$1\"";
                if ( open PRO, "$file" ) {
                    my @pro = <PRO>;
                    close PRO;
                    DEBUG and print "Handling include of file $file\n";
                    DEBUG and print "Before:\n", @data;
                    unshift(@data, @pro);
                    DEBUG and print "After:\n", @data;
                } else {
                    DEBUG and die "Missing file $file";
                }
            }
            # Skip a block based on some condition
            if ( /skipBlock (.*)/ ) {
                my $statement = $1;
                my $skip = 0;
                DEBUG and print "\$skip = 1 if ( $statement );\n";
                eval "\$skip = 1 if ( $statement );";
                if ( $skip ) {
                    DEBUG and print "Skipping a block\n";
                    my $find = 0;
                    my $line = $data[1];
                    while ( @data ) {
                        $_ = shift(@data);
                        # Strip comments
                        if ( s/\s*#.*// ) {
                            # Handle comments in continuations
                            next if ( $line && $_ eq "\n" );
                        }
                        # Collapse continuations
                        if ( s/\\\s*$// ) {
                            $line .= $_;
                            next;
                        } elsif ( $line ) {
                            $_ = $line.$_;
                            $line = "";
                        }
                        # Fix lineendings
                        chomp;
                        #DEBUG and print "skipping block line: $_";
                        $find++ while ( s/{// );
                        $find-- while ( s/}// );
                        #DEBUG and print "find $find\n";
                        last if ( $find == 0 );
                    }
                }
            }
            next;
        }
        # strip out comments
        s/#.*$//;
        # remove trailing whitespace
        s/\s+$//;
        # handle continuations (lines ending with \)
        if ( s/\\$// ) {
            if ( scalar(@data) == 0 ) {
                warn "Unterminated \\ on the last line of $proFile\n";
                push(@data, $_);
            } else {
                $data[0] = $_.$data[0];
            }
            next;
        }
        if ( !keys %{$$vars{PROJECT_TYPE}} && /qtopia_project\((.*)\)/ ) {
	    map { $$vars{PROJECT_TYPE}->{$_}++ } split(/\s+/,$1);
        }
        if ( $$vars{PROJECT_TYPE}->{qtopiacore} && /qt=(host|target)/ ) {
            $$vars{PROJECT_TYPE}->{($1 eq "host")?"desktop":"embedded"}++;
        }
        if ( /depends\((.*)\)/ ) {
	    map { $$vars{QTOPIA_DEPENDS}->{$_}++ } split(/\s+/,$1);
            DEBUG and print "QTOPIA_DEPENDS=".join(" ", keys %{$$vars{QTOPIA_DEPENDS}})."\n";
        }
        if ( /(dep|idep)\((.*)\)/ ) {
            my $var = ($1 eq "dep")?"DEP_CMDS":"DEP_I_CMDS";
	    $$vars{$var}->{$2}++;
        }
        process_variables($_, $vars);
    }
    close PRO;

    DEBUG and print "\n\n";

    if ( !$prohash ) {
        Qtopia::BlackMagic::qtopia_project($root, $proFile, $vars);
    }

    return $vars;
}

sub qtopia_project($$$)
{
    my ( $root, $proFile, $prohash ) = ( @_ );

    DEBUG and print "qtopia_project(".join(" ", keys %{$prohash->{PROJECT_TYPE}}).")\n\n\n";

    my $keywords = "$depotpath/src/qtopiadesktop/build/keywords.prf";
    if ( !exists($projectCache{$keywords}) ) {
        $projectCache{$keywords} = newhash();
        parse_pro_file($root, $keywords, $projectCache{$keywords});
    }

    # Get the equiv map
    my %equiv;
    for my $vars ( $projectCache{$root}, $projectCache{$keywords}, $prohash ) {
        for my $var ( keys %{$vars} ) {
            if ( $var =~ /^keyword\.(.*)\.equiv$/ ) {
                my $val = $1;
                if ( !exists($equiv{$val}) ) {
                    $equiv{$var} = newhash();
                }
                map { $equiv{$val}->{$_}++; } keys %{$vars->{$var}};
                DEBUG and print "equiv{$val}=".join(" ", keys %{$equiv{$val}})."\n";
            }
        }
    }

    # This logic should match qtopia_project() in src/qtopiadesktop/build/functions.prf
    # This was rewritten as a recusive loop because the other version had bugs

    my $PROJECT_TYPE = $$prohash{PROJECT_TYPE};
    my $recurse;
    $recurse = sub {
        my ( $key ) = @_;
        if ( exists($equiv{$key}) ) {
            map {
                DEBUG and print "$proFile is also a $_\n";
                $$PROJECT_TYPE{$_}++;
                &$recurse($_);
            } keys %{$equiv{$key}};
        }
    };
    for ( keys %$PROJECT_TYPE ) {
        DEBUG and print "$proFile is a $_\n";
        &$recurse($_);
    }

    for my $vars ( $projectCache{$root}, $projectCache{$keywords}, $prohash ) {
        for my $var ( keys %{$vars} ) {
            if ( $var =~ /^keyword\.(.*)\.commands$/ ) {
                my $val = $1;
                if ( $$PROJECT_TYPE{$val} ) {
                    DEBUG and print "$proFile is a $val\n";
                    for my $cmd ( keys %{$vars->{$var}} ) {
                        DEBUG and print "$cmd\n";
                        process_variables($cmd, $prohash);
                    }
                }
            }
        }
    }

    Qtopia::BlackMagic::implicit_deps($root, $proFile, $prohash);
}

sub implicit_deps($$$)
{
    my ( $root, $proFile, $prohash ) = ( @_ );
    my $CONFIG = $$prohash{CONFIG};
    my $PROJECT_TYPE = $$prohash{PROJECT_TYPE};
    my $QTOPIA_DEPENDS = $$prohash{QTOPIA_DEPENDS};
    my $QTOPIA_ID = dirname($proFile);
    $QTOPIA_ID =~ s/^\Q$depotpath\E/$QPEDIR/;
    $QTOPIA_ID =~ s/^\Q$root\E\///;
    $QTOPIA_ID =~ s/^\Q$root\E//;

    # This logic should match src/qtopiadesktop/build/implcit_deps.pri

    if ( $$CONFIG{qt} ) {
        if ( $$CONFIG{part_of_qtopiadesktop} || $$PROJECT_TYPE{desktop} ) {
            if ( $QTOPIA_ID !~ /^libraries\/qt\// ) {
                $$QTOPIA_DEPENDS{"libraries/qt/*"}++;
            }
        } else {
            if ( $QTOPIA_ID !~ /^libraries\/qtopiacore\// ) {
                $$QTOPIA_DEPENDS{"libraries/qtopiacore/*"}++;
            }
        }
    }

    if ( $$CONFIG{qtopia} ) {
        if ( !$$CONFIG{no_qtopiabase} ) {
            $$QTOPIA_DEPENDS{"libraries/qtopiabase"}++;
        }
        if ( !$$PROJECT_TYPE{core} ) {
            $$QTOPIA_DEPENDS{"libraries/qtopia"}++;
        }
    }

    if ( $$CONFIG{qtopiadesktop} && !$$PROJECT_TYPE{core} ) {
        $$QTOPIA_DEPENDS{"libraries/qtopiadeskop"}++
    }

    #FIXME handle <root>/features/implicit_deps.prf
}

sub newhash()
{
    my $hashref = +{};
    tie %$hashref, 'Hash::Ordered';
    return $hashref;
}

sub deep_copy
{
    my $this = shift;
    if (not ref $this) {
        return $this;
    } elsif (ref $this eq "HASH") {
        my $hash = newhash();
        map { $hash->{$_} = deep_copy($this->{$_}); } keys %$this;
        return $hash;
    } else {
        die "what type is $_?";
    }
}

sub dumphash
{
    my $this = shift;
    if (not ref $this) {
        return $this;
    } elsif (ref $this eq "HASH") {
        my @vals;
        map { push(@vals, "$_ => ".dumphash($this->{$_})); } keys %$this;
        return "+{ ".join(", ", @vals)." }";
    } else {
        die "what type is $_?";
    }
}

# Make this file require()able.
1;
