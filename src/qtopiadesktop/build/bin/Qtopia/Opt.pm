package Qtopia::Opt;

use strict;
use warnings;

use File::Basename;
use Qtopia::Cache;
use Carp;
#perl2exe_include Carp::Heavy
$Carp::CarpLevel = 1;

require Exporter;
our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Qtopia::Opt ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
    %optvar_storage
    @ordered_optvar_keys
    @help_data
    _resolve_to_scalar
    _resolve_to_array
    opt
    validate
    %setters_for_type
    paramstring_for
    %func_storage
    $phase
    opt_call
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
    opt_get_options
    opt_apply_defaults
    opt_resolve
    set_optvar
    opt
    add_separator
    add_note
    opt_call
    die_if_not_allowed
);

our $VERSION = '0.01';

# the engine to use
our $engine = 'Getopt';

# opt variables
our %optvar_storage;
our @ordered_optvar_keys;
our @help_data;
our %func_storage;
our $phase = "user";

# Different types have different setters
our %setters_for_type = (
    "bool" => [ qw(set unset) ],
    "value" => [ qw(set) ],
    "list" => [ qw(add remove set) ],
);

# Do a sanity check on the opt_ variables
my %registered_switches;
my $sane = 1;
sub insane
{
    print @_, "\n";
    $sane = 0;
}
sub sanity_check
{
    my @keys = @_;
    if ( !@keys ) {
        @keys = @ordered_optvar_keys;
    }
    OPT: for my $optname ( @keys ) {
        my $optref = $optvar_storage{$optname};
        if ( scalar(keys %$optref) == 0 ) {
            insane("$optname has no attributes!");
            next OPT;
        }
        my $type = $optref->{"type"};
        if ( !$type ) {
            # type is mandatory
            insane("$optname has no type!");
            next OPT;
        }
        if ( $type eq "bool" ) {
        } elsif ( $type eq "value" ) {
        } elsif ( $type eq "list" ) {
            if ( !exists($optref->{"value"}) ) {
                # Lists must have an initial value (a reference to an empty array)
                $optref->{"value"} = [];
            }
        } elsif ( $type eq "placeholder" ) {
            # don't check placeholders
            next OPT;
        } else {
            insane("Unhandled type $type for $optname!");
            next OPT;
        }
        my %checked_setters;
        for my $set ( @{$setters_for_type{$type}} ) {
            $checked_setters{$set}++;
            next if ( !exists($optref->{$set}) );
            my $ref = $optref->{$set};
            if ( ref($ref) eq "" ) {
            } elsif ( ref($ref) eq "ARRAY" ) {
                if ( scalar(@$ref) != 2 ) {
                    insane("$optname->$set must be a 2-element array!");
                    next;
                }
                $ref = $ref->[0];
            } else {
                insane("$optname->$set has unhandled type ".ref($ref));
                next;
            }
            my $paramstring = paramstring_for($ref, $optname);
            if ( $paramstring =~ /=.*/ ) {
                if ( $type eq "bool" ) {
                    insane("$optname->$set $paramstring has =s but is a $type!");
                }
            } else {
                if ( $type ne "bool" ) {
                    insane("$optname->$set $paramstring is a $type but does not have =s!");
                }
            }
            $paramstring =~ s/=.*//;
            if ( exists($registered_switches{$paramstring}) ) {
                insane("$optname is registering $paramstring for $set but $registered_switches{$paramstring} has already registered it!");
            }
            $registered_switches{$paramstring} = $optname;
            # Aliases
            for ( @{$optref->{$set."aliases"}} ) {
                $paramstring = $_;
                if ( $paramstring =~ /%/ ) {
                    insane("$optname is registering alias $paramstring that has a %. Aliases cannot use %!");
                }
                if ( $paramstring =~ /=.*/ ) {
                    if ( $type eq "bool" ) {
                        insane("$optname->$set $paramstring has =s but is a $type!");
                    }
                } else {
                    if ( $type ne "bool" ) {
                        insane("$optname->$set $paramstring is a $type but does not have =s!");
                    }
                }
                $paramstring =~ s/=.*//;
                if ( exists($registered_switches{$paramstring}) ) {
                    insane("$optname is registering alias $paramstring $set but $registered_switches{$paramstring} has already registered it!");
                }
                $registered_switches{$paramstring} = $optname;
            }
        }
        if ( ! keys %checked_setters ) {
            insane("$optname has no setters!");
        }
        for my $set ( qw(set unset add remove) ) {
            next if ( $checked_setters{$set} || !exists($optref->{$set}) );
            insane("$optname->$set should not exist (for type $type)!");
        }

        if ( !exists($optref->{"visible"}) ) {
            $optref->{"visible"} = 1;
        }

        if ( $type eq "list" && exists($optref->{"default"}) ) {
            if ( ref($optref->{"default"}) ne "CODE" && ref($optref->{"default"}) ne "ARRAY" ) {
                insane("$optname->default is the wrong type! (".ref($optref->{"default"}).", should be CODE or ARRAY).");
            }
            $optref->{"visible"} = 1;
        }

        if ( !exists($optref->{"autodep"}) ) {
            $optref->{"autodep"} = 1;
        }

        if ( !exists($optref->{"arg"}) ) {
            $optref->{"arg"} = "arg,arg";
        }
    }
}

# Get options and print help
sub opt_get_options
{
    if ( !$sane ) {
        die "Cannot get options while insane!\n";
    }
    my $ret;
    eval "require Qtopia::Opt::$engine;";
    die $@ if ( $@ );
    eval "\$ret = Qtopia::Opt::${engine}::opt_get_options(\@_)";
    die $@ if ( $@ );
    return $ret;
}

sub get_help
{
    my $ret;
    eval "require Qtopia::Opt::$engine;";
    die $@ if ( $@ );
    eval "\$ret = Qtopia::Opt::${engine}::get_help(\@_)";
    die $@ if ( $@ );
    return $ret;
}

# Apply defaults
sub opt_apply_defaults
{
    for my $optname ( @_, keys %optvar_storage ) {
        my $optref = $optvar_storage{$optname};
        my $auto = _resolve_to_scalar($optref->{"autodep"});
        my $vis = _resolve_to_scalar($optref->{"visible"});
        my $force_default = _resolve_to_scalar($optref->{"force_default"});
        my $def;
        my $ok = ( !exists($optref->{"value"}) && exists($optref->{"default"}) );
        if ( $optref->{"type"} eq "list" ) {
            my $val = $optref->{"value"};
            my @def = _resolve_to_array($optref->{"default"});
            $ok = 0;
            if ( scalar(@$val) == 0 && scalar(@def) > 0 ) {
                $ok = 1;
                $def = \@def;
            }
        }
        if ( $ok ) {
            if ( ( $auto && $vis ) || $force_default ) {
                if ( !defined($def) ) {
                    $def = _resolve_to_scalar($optref->{"default"});
                }
                $optref->{"value"} = $def;
                my $setfunc = $optref->{"setfunc"};
                if ( $optref->{"type"} eq "bool" && defined($def) && $def eq "0" ) {
                    $setfunc = $optref->{"unsetfunc"};
                }
                if ( $setfunc && ref($setfunc) eq "CODE" ) {
                    &$setfunc("dummy", $def);
                }
                $optref->{"auto"} = 1;
                # defaults need to be validated too
                $ok = validate($optname);
                if ( !$ok ) {
                    die "The default value for $optname is invalid!";
                }
            }
        }
    }
}

# Return the value that the option would have, if defaults had been applied
sub opt_resolve($)
{
    my ( $optname ) = @_;
    my $optref = $optvar_storage{$optname};
    my $auto = _resolve_to_scalar($optref->{"autodep"});
    if ( exists($optref->{"value"}) ) {
        return $optref->{"value"};
    }
    if ( !exists($optref->{"value"}) && exists($optref->{"default"}) ) {
        my $defref = $optref->{"default"};
        my $def;
        if ( ref($defref) ne "CODE" ) {
            $def = $defref;
        } elsif ( $auto ) {
            if ( ref($defref) eq "CODE" ) {
                $def = &$defref();
            }
        }
        return $def;
    }
    return undef;
}

# Create an opt_ variable
sub set_optvar($$)
{
    my ( $optname, $hashref ) = @_;
    if ( exists($optvar_storage{$optname}) ) {
        croak "opt var $optname already exists!";
    }
    $optvar_storage{$optname} = $hashref;
    push(@ordered_optvar_keys, $optname);
    push(@help_data, $optname);

    # Some features are controlled by modules.
    # They exist as options for legacy reasons.
    if ( $hashref->{"module"} ) {
        # Force the options to be visible (their switches are set to
        # "hidden" so they won't show up in configure -help).
        $hashref->{"visible"} = 1;
        my $set = $hashref->{"set"};
        if ( !$hashref->{"module_help"} ) {
            # If there's no module-specific help, use the help for the "set" switch instead.
            $hashref->{"module_help"} = $set->[1];
        }
        # In each case, warn about receiving the switch and then use opt_call to
        # simulate the correct -add-module module.
        $set->[1] = "hidden";
        $hashref->{"setfunc"} = sub {
            my $paramstring = $set->[0];
            $paramstring =~ s/%/$optname/;
            $paramstring =~ s/_/-/g;
            warn "WARNING: -$paramstring is deprecated. Please use -add-module ".$hashref->{"module"}." instead.\n";
            opt_call("modules", "add", $hashref->{"module"});
        };
        my $unset = $hashref->{"unset"};
        $unset->[1] = "hidden";
        $hashref->{"unsetfunc"} = sub {
            my $paramstring = $unset->[0];
            $paramstring =~ s/%/$optname/;
            $paramstring =~ s/_/-/g;
            warn "WARNING: -$paramstring is deprecated. Please use -remove-module ".$hashref->{"module"}." instead.\n";
            opt_call("modules", "remove", $hashref->{"module"});
        };
    }

    sanity_check($optname);
}

# Shorthand to access the value
sub opt : lvalue
{
    if ( scalar(@_) < 1 ) {
        croak "You must supply a name to opt()!";
    }
    my $optname = $_[0];
    my $key = "value";
    if ( scalar(@_) == 2 ) {
        $key = $_[1];
    }
    if ( !exists($optvar_storage{$optname}) ) {
        croak "opt var $optname does not exist!";
    }
    my $optref = $optvar_storage{$optname};
    $optref->{$key};
}

sub add_separator()
{
    if ( $help_data[$#help_data] ne "---" ) {
        push(@help_data, "---");
    }
}

sub add_note($)
{
    my ( $note ) = @_;
    add_separator();
    push(@help_data, "---$note");
    add_separator();
}

# Dump the opt_ variables to config.cache
sub write_config_cache()
{
    my %ignored_attributes = map { $_ => 1 } qw(module_setfunc arg visible autodep);
    for my $set ( qw(set unset add remove) ) {
        $ignored_attributes{$set} = 1;
        $ignored_attributes{$set."aliases"} = 1;
        $ignored_attributes{$set."func"} = 1;
    }
    my $ret = "";
    OPT: for my $optname ( @ordered_optvar_keys ) {
        my $noted = 0;
        my $optref = $optvar_storage{$optname};
        ATTR: for my $attribute ( grep { !exists($ignored_attributes{$_}) } keys %$optref ) {
            my $ref = $optref->{$attribute};
            my $value;
            if ( !defined($ref) ) {
                $value = "undef";
            } elsif ( ref($ref) eq "" ) {
                $value = $ref;
            } elsif ( ref($ref) eq "ARRAY" ) {
                for ( my $i = 0; $i < scalar(@$ref); $i++ ) {
                    $value = $ref->[$i];
                    $value = "undef" unless defined($value);
                    $noted = 1;
                    $value =~ s/\\n/\\\\n/g; # \n is likely in path names on Windows
                    $value =~ s/\n/\\n/g;
                    $ret .= "opt.$optname.$attribute.\[$i\]=$value\n";
                }
                next ATTR;
            } elsif ( ref($ref) eq "CODE" ) {
                my $auto = _resolve_to_scalar($optref->{"autodep"});
                if ( $attribute ne "default" || $auto ) {
                    $value = &$ref();
                    my @test = &$ref();
                    if ( $value && @test && $value eq scalar(@test) ) {
                        # The function returned an array
                        for ( my $i = 0; $i < scalar(@test); $i++ ) {
                            $value = $test[$i];
                            $value = "undef" unless defined($value);
                            $noted = 1;
                            $value =~ s/\\n/\\\\n/g; # \n is likely in path names on Windows
                            $value =~ s/\n/\\n/g;
                            $ret .= "opt.$optname.$attribute.\[$i\]=$value\n";
                        }
                        next ATTR;
                    }
                } else {
                    $value = "undef";
                }
            } else {
                $value = $ref;
            }
            if ( defined($value) ) {
                $noted = 1;
                $value =~ s/\\n/\\\\n/g; # \n is likely in path names on Windows
                $value =~ s/\n/\\n/g;
                $ret .= "opt.$optname.$attribute=$value\n";
            }
        }
        if ( !$noted ) {
            # Add a bogus entry for this one so it appears in config.cache
            $ret .= "opt.$optname.value=undef\n";
        }
    }
    Qtopia::Cache::replace("opt", $ret);
    return $ret;
}

# Load the opt_ variables from config.cache
sub read_config_cache()
{
    # clear out anything that's in there now
    %optvar_storage = ();

    my @cache = Qtopia::Cache::load("opt");
    for ( @cache ) {
        if ( /^opt\.([^\.=]+)\.([^\.=]+)([^=]*)=(.*)/ ) {
            my $optname = $1;
            my $attribute = $2;
            my $extra = $3;
            my $value = $4;
            $value =~ s/\\\\/__LITERAL__BACK__SLASH__/g;
            $value =~ s/\\n/\n/g;
            $value =~ s/__LITERAL__BACK__SLASH__/\\\\/g;
            $value =~ s/\\\\n/\\n/g;
            if ( $value eq "undef" ) {
                $value = undef;
            }
            my $optref = $optvar_storage{$optname};
            if ( !$optref ) {
                $optref = $optvar_storage{$optname} = +{};
                push(@ordered_optvar_keys, $optname);
            }
            if ( $extra ) {
                # currently we only support @
                if ( $extra =~ /\.\[([0-9]+)\]/ ) {
                    my $i = $1;
                    my $ref = $optref->{$attribute};
                    if ( !$ref ) {
                        $ref = $optref->{$attribute} = [];
                    }
                    $ref->[$i] = $value;
                }
            } else {
                $optref->{$attribute} = $value;
                if ( $attribute eq "type" && $value eq "list" ) {
                    if ( ref($optref->{"value"}) ne "ARRAY" ) {
                        $optref->{"value"} = [];
                    }
                }
            }
        }
    }
}

sub _resolve_to_scalar
{
    my ( $ref ) = @_;

    my $ret;

    if ( !defined($ref) ) {
        $ret = undef;
    } elsif ( ref($ref) eq "ARRAY" ) {
        $ret = join(" ", @$ref);
    } elsif ( ref($ref) eq "CODE" ) {
        $ret = &$ref();
    } else {
        $ret = $ref;
    }

    $ret;
}

sub _resolve_to_array
{
    my ( $ref ) = @_;

    my @ret;

    if ( !defined($ref) ) {
        @ret = ();
    } elsif ( ref($ref) eq "CODE" ) {
        @ret = &$ref();
    } elsif ( ref($ref) eq "ARRAY" ) {
        @ret = @$ref;
    } else {
        push(@ret, $ref);
    }

    @ret;
}

sub setEngine
{
    ( $engine ) = @_;
}

# Validate one (or all) options
sub validate
{
    my @check = @_;
    if ( scalar(@check) == 0 ) {
        @check = keys %optvar_storage
    }

    my $ok = 1;

    # check input against "available", if present
    AVAILCHECK: for my $optname ( @check ) {
        my $optref = $optvar_storage{$optname};
        my $type = $optref->{"type"};
        next if ( !exists($optref->{"value"}) );
        next if ( !exists($optref->{"available"}) );
        my @value;
        if ( $type eq "bool" || $type eq "value" ) {
            push(@value, $optref->{"value"});
        } elsif ( $type eq "list" ) {
            push(@value, @{$optref->{"value"}});
        }
        my @available = _resolve_to_array($optref->{"available"});
        AVAILWORD: for my $word ( @value ) {
            #next if ( !$word );
            for my $a ( @available ) {
                if ( $word eq $a ) {
                    next AVAILWORD;
                }
            }
            warn "ERROR: Invalid value for option \"$optname\": $word\n".
                 "       Valid values: ".join(",", @available)."\n";
            $ok = 0;
            next AVAILCHECK;
        }
    }

    $ok;
}

sub get_feature_modules
{
    my @ret;
    OPT: for my $optname ( @ordered_optvar_keys ) {
        my $optref = $optvar_storage{$optname};
        if ( $optref->{"module"} ) {
            push(@ret, $optname, $optref->{"module"});
        }
    }
    @ret;
}

sub get_features
{
    my @ret;
    OPT: for my $optname ( @ordered_optvar_keys ) {
        my $optref = $optvar_storage{$optname};
        if ( $optref->{"feature"} ) {
            push(@ret, $optname);
        }
    }
    @ret;
}

sub paramstring_for
{
    my ( $paramstring, $optname ) = @_;
    if ( index($paramstring, "%") != -1 ) {
        my $paramname = $optname;
        $paramname =~ s/_/-/g;
        $paramstring =~ s/%/$paramname/g;
    }
    $paramstring;
}

sub opt_call
{
    my ( $optname, $set, $value ) = @_;
    if ( !exists($optvar_storage{$optname}) ) {
        croak "opt $optname does not exist!";
    }
    my $optref = $optvar_storage{$optname};
    if ( !exists($optref->{$set}) ) {
        croak "opt $optname does not have setter $set!";
    }
    my $ref = $optref->{$set};
    my $paramstring;
    if ( ref($ref) eq "" ) {
        $paramstring = paramstring_for($ref, $optname);
    } elsif ( ref($ref) eq "ARRAY" ) {
        $paramstring = paramstring_for($ref->[0], $optname);
    }
    if ( !exists($func_storage{$paramstring}) ) {
        die "func for $paramstring does not exist!";
    }
    my $func = $func_storage{$paramstring};
    &$func("dummy", $value);
}

sub die_if_not_allowed
{
    my ( $optname, $value ) = @_;
    if ( !exists($optvar_storage{$optname}) ) {
        croak "opt $optname does not exist!";
    }
    my $optref = $optvar_storage{$optname};
    my $die = 0;
    my $ref = $optref->{"auto"};
    if ( ref($ref) eq "ARRAY" ) {
        for ( @$ref ) {
            if ( $value eq $_ ) {
                return;
            }
        }
        $die = 1;
    } else {
        if ( !$optref->{"auto"} ) {
            $die = 1;
        }
    }
    if ( $die ) {
        my $word = $optref->{"die_if_not_allowed"};
        if ( !$word ) {
            $word = "option '$optname'";
            my $type = $optref->{"type"};
            my @setters;
            if ( $value && $type eq "list" ) {
                push(@setters, "add");
            }
            push(@setters, "set");
            for my $set ( @setters ) {
                next if ( !exists($optref->{$set}) );
                my $ref = $optref->{$set};
                if ( ref($ref) eq "ARRAY" ) {
                    $ref = $ref->[0];
                }
                my $switch = paramstring_for($ref, $optname);
                $switch = "-$switch";
                $word = $switch;
                if ( $value && $word =~ s/=.*// ) {
                    $word .= " $value";
                }
                last;
            }
        }
        die "ERROR: You requested $word but it was disabled.\n";
    }
}

sub generate_module_help
{
    my %feature_modules = get_feature_modules();
    my $width = 0;
    my @notes;
    for my $optname ( @ordered_optvar_keys ) {
        next if ( !exists($feature_modules{$optname}) );
        my $module = $feature_modules{$optname};
        my $help = opt($optname, "module_help");
        push(@notes, $module, $help);
        if ( $width < length($module) ) {
            $width = length($module);
        }
    }
    my @tmp = @help_data;
    @help_data = ();
    for ( @tmp ) {
        if ( /MODULE_HELP_GOES_HERE/ ) {
            while ( @notes ) {
                my $module = shift(@notes);
                my $help = shift(@notes);
                push(@help_data, sprintf("---    %-${width}s : %s", $module, $help ));
            }
            next;
        }
        push(@help_data, $_);
    }
}

sub set_features_from_modules
{
    my %feature_modules = Qtopia::Opt::get_feature_modules();
    my %modules;
    map { $modules{$_}++ } @{opt("modules")};
    for my $optname ( keys %feature_modules ) {
        my $module = $feature_modules{$optname};
        if ( $modules{$module} ) {
            if ( !opt($optname) ) {
                opt($optname) = 1;
                my $func = opt($optname, "module_setfunc");
                if ( $func ) {
                    &$func();
                }
            }
        } else {
            opt($optname) = 0;
        }
    }
}

sub remove_modules_for_disabled_features
{
    # The master list of modules
    my @list =  @{opt("modules")};

    my %feature_modules = Qtopia::Opt::get_feature_modules();
    my %modules;
    map { $modules{$_}++ } @{opt("modules")};
    for my $optname ( keys %feature_modules ) {
        my $module = $feature_modules{$optname};
        if ( !opt($optname) ) {
            # Remove the module from the list
            my @tmp = @list;
            @list = ();
            for ( @tmp ) {
                next if ( $_ eq $module );
                push(@list, $_);
            }
        }
    }
    opt("modules") = \@list;
}

# Make this file require()able.
1;
