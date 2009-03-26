#!/usr/bin/perl

use strict;
use warnings;

my $infile = shift(@ARGV) or die;
my $outfile = shift(@ARGV) or die;
my $common = shift(@ARGV) or die;

open IN, $common or die "Can't read $common";
my @common = <IN>;
close IN;

open IN, $infile or die "Can't read $infile";
my @data = <IN>;
close IN;

for ( @data ) {
    if ( /COMMON_TEST_CASES/ ) {
        $_ = join("", @common);
    }
}

open OUT, ">$outfile" or die "Can't write $outfile";
print OUT @data;
close OUT;

