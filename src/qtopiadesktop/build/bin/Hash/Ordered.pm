################################################################################
#
# This third party code implements ordered hashes in perl.
#
# http://www.foad.org/~abigail/Perl/Hash/Ordered.pm
#
################################################################################

package Hash::Ordered;

################################################################################
#
# $Author: abigail $
#
# $Date: 1998/03/07 10:17:37 $
#
# $Id: Ordered.pm,v 1.1 1998/03/07 10:17:37 abigail Exp $
#
# $Log: Ordered.pm,v $
# Revision 1.1  1998/03/07 10:17:37  abigail
# Initial revision
#
#
# Retrieve keys in the same order as you inserted them.
#
################################################################################

use strict;

use constant PREV => 0;
use constant DATA => 1;
use constant NEXT => 2;

use vars qw /$VERSION/;
$VERSION   = ('$Revision: 1.1 $' =~ /(\d+\.\d+)/) [0];

sub TIEHASH {
    my $proto = shift;
    my $class = ref $proto || $proto;

    my $self  = bless {} => $class;

    $self -> {hash} = {};
    $self;
}


sub STORE {
    my $self = shift;
    my ($key, $data) = @_;

    # Key already exits. Just replace value.
    if ($self -> {hash} -> {$key}) {
        $self -> {hash} -> {$key} -> [DATA] = $data;
    }
    else {
        $self -> {hash} -> {$key} = [undef, $data, undef];
        if (exists $self -> {Last}) {
            $self -> {hash} -> {$self -> {Last}} -> [NEXT] = $key;
            $self -> {hash} -> {$key} -> [PREV] =   $self -> {Last};
            $self -> {Last} =   $key;
        }
        else {
            $self -> {Last}  = $key;
            $self -> {first} = $key;
        }
    }

    $self -> {hash} -> {$key} -> [DATA];
}



sub FETCH {
    my $self = shift;
    my $key  = shift;

    exists $self -> {hash} -> {$key} ? $self -> {hash} -> {$key} -> [DATA]
                                     : undef;
}


sub DELETE {
    my $self = shift;
    my $key  = shift;

    my $this = $self -> {hash} -> {$key} or return;
    my $next = $this -> [NEXT];
    my $prev = $this -> [PREV];

    if (defined $next) {$self -> {hash} -> {$next} -> [PREV] = $prev;}
    else {$self -> {Last} = $prev;}
    if (defined $prev) {$self -> {hash} -> {$prev} -> [NEXT] = $next;}
    else {$self -> {first} = $next;}

    delete $self -> {hash} -> {$key};
}


sub EXISTS {
    my $self = shift;
    my $key  = shift;

    exists $self -> {hash} -> {$key};
}



sub FIRSTKEY {
    my $self = shift;

    $self -> {first};
}


sub NEXTKEY {
    my $self = shift;
    my $last = shift;

    defined $self -> {hash} -> {$last} ? $self -> {hash} -> {$last} -> [NEXT]
                                       : undef;
}


sub CLEAR {
    my $self = shift;

    $self -> {hash} = {};
    delete $self -> {Last};
    delete $self -> {first};
}


1;


__END__

=head1 NAME

Hash::Ordered - Remember the order in which you inserted in a hash.

=head1 SYNOPSIS

    use Hash::Ordered;
    tie my %hash, 'Hash::Ordered';

    foreach my $key ('a' .. 'z') {
        $hash {$key} = 1;
    }

    print keys %hash;     # prints 'abcdefghijklmnopqrstuvwxyz'

=head1 DESCRIPTION

This package allows you to retrieve the keys in the same order as they
were inserted. Note that if you assign to an already existing key in
the hash, the package does B<not> classify the key as new. If you want
that behaviour, delete the key first.

=head1 HISTORY

    $Date: 1998/03/07 10:17:37 $

    $Log: Ordered.pm,v $
    Revision 1.1  1998/03/07 10:17:37  abigail
    Initial revision


=head1 AUTHOR

This package was written by Abigail.

=head1 BUGS

The hash uses the keys as next/previous pointers in an embedded linked
list. If the keys are large, this is a memory waste; perhaps references
to the keys are better.

=head1 COPYRIGHT

Copyright 1998 by Abigail.

You may use, distribute and modify this package under the same
terms as Perl.

=cut
