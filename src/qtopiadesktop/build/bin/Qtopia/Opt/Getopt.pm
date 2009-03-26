package Qtopia::Opt::Getopt;

use strict;
use warnings;

use File::Basename;
use Getopt::Long;
use FileHandle;
use Qtopia::Opt ':all';

use constant DEBUG => 0;

# help variables
our $optiondesc;
our $optionhelp;
our $optionstar;
our $optiondef;
our $optionavail;

my $cols = $ENV{COLUMNS};
$cols = 80 unless ( $cols );
# the width of the first column (calculated in _init_formats)
my $fcwidth;

# Only warn about deprecated options once (even if they appear multiple times)
my %deprecated;

my @optl;

# Get options
sub opt_get_options
{
    my $dohelp = 1;
    my $dieextra = 0;
    my $dovalidate = 1;
    for ( @_ ) {
        if ( $_ eq "nohelp" ) {
            $dohelp = 0;
        }
        if ( $_ eq "noextra" ) {
            $dieextra = 1;
        }
        if ( $_ eq "novalidate" ) {
            $dovalidate = 0;
        }
    }

    if ( !@optl ) {
        setup_optl();
    }

    Getopt::Long::Configure("bundling_override");
    my $ok = GetOptions(@optl);
    if ( $dieextra && @ARGV ) {
        print "Unhandled arguments: ".join(" ",@ARGV)."\n";
        $ok = 0;
    }

    if ( $dovalidate && !Qtopia::Opt::validate() ) {
        $ok = 0;
    }

    if ( !$dohelp ) {
        return $ok;
    }
    if ( !$ok || (exists($optvar_storage{"help"}) && opt("help")) ) {
        get_help();
    }
}

# Setup the GetOptions info
sub setup_optl
{
    for my $optname ( @ordered_optvar_keys ) {
        DEBUG and print "opt_get_options $optname\n";
        my $optref = $optvar_storage{$optname};
        my $vis = _resolve_to_scalar($optref->{"visible"});
        my %ignore; map { $ignore{$_} = 1; } _resolve_to_array($optref->{"silentignore"});
        # Ignore hidden items (does not include items for which the silentignore attribute is set)
        next if ( !$vis && !keys %ignore );
        # Each type has a number of setters that can be set
        my $type = $optref->{"type"};
        DEBUG and print "    type $type\n";
        for my $set ( @{$setters_for_type{$type}} ) {
            # Ignore missing setters
            next if ( !exists($optref->{$set}) );
            DEBUG and print "    setter $set\n";
            my $ref = $optref->{$set};
            my $paramstring;
            if ( ref($ref) eq "" ) {
                $paramstring = paramstring_for($ref, $optname);
            } elsif ( ref($ref) eq "ARRAY" ) {
                $paramstring = paramstring_for($ref->[0], $optname);
            }
            DEBUG and print "        switch -$paramstring\n";
            # Ignore hidden items
            next if ( !$vis && !$ignore{$paramstring} );

            # This is the function that we pass to getopt
            #print "SETUP FUNC $optname $optref $type $set $paramstring\n";
            my $func = sub {
                #print "CALL FUNC $optname $optref $type $set $paramstring $_[0] $_[1]\n";
                # Phase detection (only accept options at certain times)
                if ( exists($optref->{"allowed_phases"}) ) {
                    my $ok = 0;
                    for ( @{$optref->{"allowed_phases"}} ) {
                        if ( $_ eq $phase ) {
                            $ok = 1;
                            last;
                        }
                    }
                    if ( !$ok ) {
                        my $p = $paramstring;
                        if ( $p =~ s/=.*// ) {
                            $p .= " ".$_[1];
                        }
                        warn "WARNING: -$p ignored in $phase phase!\n";
                        return;
                    }
                }

                if ( exists($optvar_storage{"allowfail"}) ) {
                    if ( $optname ne "allowfail" ) {
                        # If the user specifies something, it is implcitily no longer allowed to fail.
                        # eg. configure -add-mediaengine gstreamer will fail if GStreamer cannot be
                        # enabled even though devices/reference/configure-qvfb has
                        # -allow-fail mediaengines,gstreamer
                        #print "allowfail remove $optname,$_[1]\n";
                        opt_call("allowfail", "remove", $optname);
                        opt_call("allowfail", "remove", $optname.",".$_[1]);
                    } elsif ( $set eq "add" ) {
                        #print "allowfail $set $_[1]\n";
                    }
                }

                # Set the value
                if ( $type eq "bool" ) {
                    # Boolean set/unset
                    if ( $set eq "set" ) {
                        $optref->{"value"} = 1;
                    } elsif ( $set eq "unset" ) {
                        $optref->{"value"} = 0;
                    }
                } elsif ( $type eq "value" ) {
                    # set value
                    if ( $set eq "set" ) {
                        $optref->{"value"} = $_[1];
                    }
                } elsif ( $type eq "list" ) {
                    if ( $set eq "set" ) {
                        # set value (comma or space separated)
                        my $val = $_[1];
                        $val =~ s/,/ /g;
                        $optref->{"value"} = [ split(/\s+/, $val) ];
                        #print "Set the list $optname (".join(" ", @{$optref->{"value"}}).")\n";
                    } elsif ( $set eq "add" ) {
                        # add value (to the end of the list)
                        #print "adding $_[1] to the list $optname\n";
                        my $found = 0;
                        for ( @{$optref->{"value"}} ) {
                            if ( $_ eq $_[1] ) {
                                # Don't add it again!
                                $found = 1;
                                last;
                            }
                        }
                        if ( !$found ) {
                            push(@{$optref->{"value"}}, $_[1]);
                            #print "Added $_[1] to the list $optname (".join(" ", @{$optref->{"value"}}).")\n";
                        } else {
                            #print "Didn't add $_[1] to the list $optname (".join(" ", @{$optref->{"value"}}).")\n";
                        }
                    } elsif ( $set eq "remove" ) {
                        # remove value
                        my @tmp;
                        for ( @{$optref->{"value"}} ) {
                            next if ( $_ eq $_[1] );
                            push(@tmp, $_);
                        }
                        $optref->{"value"} = \@tmp;
                        #print "Removed $_[1] from the list $optname (".join(" ", @{$optref->{"value"}}).")\n";
                        if ( $optname eq "mediaengines" ) {
                            #print "because of $paramstring\n";
                        }
                    }
                }

                # Call the func if there is one
                if ( exists($optref->{$set."func"}) ) {
                    &{$optref->{$set."func"}}(@_);
                }
            };

            my $ignorefunc;

            # Ignored items use this function instead
            if ( !$vis && $ignore{$paramstring} ) {
                DEBUG and print "        !vis && ignore\n";
                $func = $ignorefunc = sub {
                    my $p = $paramstring;
                    if ( $p =~ s/=.*// ) {
                        $p .= " ".$_[1];
                    }
                    warn "WARNING: -$p has no effect in this configuration.\n";
                };
            }

            # Let Getopt see it
            push(@optl, $paramstring => $func);
            # Keep a reference so it can be called via opt_call()
            $func_storage{$paramstring} = $func;

            # Aliases (for backwards compatibility)
            for ( @{$optref->{$set."aliases"}} ) {
                # copy, so that changes to this one don't apply to the list!
                my $alias = $_;
                DEBUG and print "        alias $alias\n";
                # If an alias is used we warn and then call the original function
                my $aliasfunc = sub {
                    if ( $optref->{"deprecated"} ) {
                        # Option-supplied deprecation message
                        my $msg = $optref->{"deprecated"};
                        if ( ref($msg) eq "HASH" ) {
                            $msg = $msg->{$set};
                        }
                        warn "$msg\n";
                    } else {
                        # Automatic deprecation warning
                        if ( $alias =~ s/=.*// ) {
                            #$alias .= " ".$_[1];
                            $alias .= " ".get_arg($optref->{"arg"}, $type, $set);
                        }
                        if ( $paramstring =~ s/=.*// ) {
                            #$paramstring .= " ".$_[1];
                            $paramstring .= " ".get_arg($optref->{"arg"}, $type, $set);
                        }
                        if ( !$deprecated{$alias} ) {
                            $deprecated{$alias}++;
                            warn "WARNING: -$alias is deprecated. Please use -$paramstring instead.\n";
                        }
                    }
                    &$func(@_);
                };

                # An alias for an ignored switch does the same as the ignored switch.
                if ( !$vis && $ignore{$paramstring} ) {
                    DEBUG and print "        !vis && ignore\n";
                    $aliasfunc = $ignorefunc;
                }

                push(@optl, $alias => $aliasfunc);
            }
        }
    }
}

sub get_help
{
    my $doexit = 1;
    for ( @_ ) {
        if ( $_ eq "noexit" ) {
            $doexit = 0;
        }
    }

    # print out some help
    my @helpinfo;
    for my $optname ( @help_data ) {
        if ( $optname =~ /^---/ ) {
            push(@helpinfo, $optname);
            next;
        }
        my $optref = $optvar_storage{$optname};
        # Skip invisible items
        next if ( !_resolve_to_scalar($optref->{"visible"}) );
        my @info = ();
        my $count = 0;
        # Each type has a number of setters that can be set
        my $type = $optref->{"type"};
        for my $set ( @{$setters_for_type{$type}} ) {
            # Ignore missing setters
            next if ( !exists($optref->{$set}) );
            my $ref = $optref->{$set};
            my $paramhelp;
            my $paramstring;
            if ( ref($ref) eq "" ) {
                $paramstring = paramstring_for($ref, $optname);
            } elsif ( ref($ref) eq "ARRAY" ) {
                $paramstring = paramstring_for($ref->[0], $optname);
                $paramhelp = $ref->[1];
                # Ignore hidden options
                next if ( $paramhelp eq "hidden" );
            }
            my @paramavail = ();
            my $paramdef = "";
            my $paramstar = " ";
            if ( $paramstring =~ s/=.*// ) {
                $paramstring .= " ".get_arg($optref->{"arg"}, $type, $set);
                push(@paramavail, _resolve_to_array($optref->{"available"}));
                if ( $type eq "list" ) {
                    $paramdef = join(",", _resolve_to_array($optref->{"default"}));
                } else {
                    $paramdef = _resolve_to_scalar($optref->{"default"});
                }
                $paramdef = "" if ( !defined($paramdef) );
                if ( $optref->{"default_tested"} ) {
                    $paramstar = "+";
                }
            } else {
                my $def;
                $def = _resolve_to_scalar($optref->{"default"});
                if ( ($set eq "set" && $def) ||
                     ($set eq "unset" && defined($def) && !$def) ) {
                    $paramstar = "*";
                    if ( $set eq "set" && $optref->{"default_tested"} ) {
                        $paramstar = "+";
                    }
                }
            }
            if ( exists($optvar_storage{"help"}) && opt("help") ) {
                if ( !$paramhelp ) {
                    $paramhelp = "";
                    $paramdef = "";
                    @paramavail = ();
                }
                push(@info, "-$paramstring", $paramhelp, $paramstar, $paramdef, join(", ", @paramavail));
            } else {
                push(@info, "-$paramstring");
            }
            $count++;
        }
        if ( scalar(@info) == 1 ) {
            push(@helpinfo, "[ ".$info[0]." ]");
        } elsif ( scalar(@info) == 2 ) {
            push(@helpinfo, "[ ".$info[0]." | ".$info[1]." ]");
        } else {
            push(@helpinfo, "---") if ( $count >= 2 );
            push(@helpinfo, @info);
            push(@helpinfo, "---") if ( $count >= 2 );
        }
    }
    # Setup the terminal-width-dependant formats;
    if ( exists($optvar_storage{"help"}) && opt("help") ) { 
        _init_formats(@helpinfo);
        print "Usage:  ".basename($0)." [options]

The defaults (*) are usually acceptable. A plus (+) denotes a default
value that needs to be evaluated. If the evaluation succeeds, the
feature is included.

Here is a short explanation of each option:

";

        my $spacer = 0;
        while ( @helpinfo ) {
            $optiondesc = shift(@helpinfo);
            if ( $optiondesc =~ s/^---// ) {
                if ( $optiondesc ) {
                    $spacer = 0;
                    format_name STDOUT "NOTE";
                    write;
                } elsif ( !$spacer ) {
                    $spacer = 1;
                    print "\n";
                }
                next;
            }
            $spacer = 0;
            $optionhelp = shift(@helpinfo);
            $optionstar = shift(@helpinfo);
            $optiondef = shift(@helpinfo);
            $optionavail = shift(@helpinfo);
            if ( $optionhelp ) {
                if ( length($optiondesc) < $fcwidth ) {
                    $optiondesc .= " ";
                    while ( length($optiondesc) < $fcwidth ) {
                        $optiondesc .= ".";
                    }
                }
            }
            format_name STDOUT "LONGHELP";
            write;
        }
        print "\n";
        exit 0 if ( $doexit );
    } else {
        my $header = "Usage:  ".basename($0)." ";
        my $leader = "                  ";
        my $col = length($header);
        print $header;
        for my $option ( @helpinfo ) {
            if ( $option =~ s/^---// ) {
                # skip notes
                next;
            }
            if ( length($option) + $col > ($cols-1) ) {
                print "\n$leader";
                $col = length($leader);
            }
            print "$option ";
            $col += length($option) + 1;
        }
        print "\n\n        Pass -help for a detailed explanation of each option.\n\n";
    }

    exit 2 if ( $doexit );
}

sub _init_formats
{
    my @helpinfo = @_;

    # work out a good width for the first column
    $fcwidth = 24;

    my $qscr = $cols / 4;
    if ( $qscr > 24 ) {
        while ( @helpinfo ) {
            $optiondesc = shift(@helpinfo);
            next if ( $optiondesc =~ s/^---// );

            $optionhelp = shift(@helpinfo);
            $optionstar = shift(@helpinfo);
            $optiondef = shift(@helpinfo);
            $optionavail = shift(@helpinfo);

            if ( length($optiondesc) > $fcwidth ) {
                $fcwidth = length($optiondesc);
            }
        }
        if ( $fcwidth > $qscr ) {
            $fcwidth = $qscr;
        }
    }

    my $scwidth = $cols - $fcwidth - 7;
    my $fmt = "format LONGHELP =\n".
              '  @ ^'.'<'x($fcwidth).' ^'.'<'x($scwidth)."\n".
              '  $optionstar, $optiondesc,  $optionhelp'."\n".
              '~~   ^'.'<'x($fcwidth-1).' ^'.'<'x($scwidth)."\n".
              '     $optiondesc,            $optionhelp'."\n".
              '~     '.' 'x($fcwidth-1).' Available: ^'.'<'x($scwidth-11)."\n".
              '                             $optionavail'."\n".
              '~~    '.' 'x($fcwidth-1).' ^'.'<'x($scwidth)."\n".
              '                             $optionavail'."\n".
              '~     '.' 'x($fcwidth-1).' Default: ^'.'<'x($scwidth-9)."\n".
              '                             $optiondef'."\n".
              '~~    '.' 'x($fcwidth-1).' ^'.'<'x($scwidth)."\n".
              '                             $optiondef'."\n".
              ".\n";
    eval $fmt;
    die $@ if ( $@ );

    $fmt = "format NOTE =\n".
           '~~      ^'.'<'x($cols-10)."\n\$optiondesc\n.";
    eval $fmt;
    die $@ if ( $@ );

    # Only split on spaces, not the - character
    $: = " ";
}

sub get_arg
{
    my ( $arg, $type, $set ) = @_;
    if ( $type eq "value" || $set eq "add" || $set eq "remove" ) {
        $arg =~ s/,.*//;
    }
    $arg;
}

# Make this file require()able.
1;
