#!/usr/bin/perl

use strict;
use warnings;

=head1 DESCRIPTION

Create header file for inclusion in packagemanager, with all
descriptions of security profiles.

=head1 SYNOPSIS

extract_sxe_profiles.pl doc/src/sxe/sxe-pkgmgr-profiles.qdoc > src/settings/packagemanager/sxe_profiles.h

=cut

# Assumes the table cells are laid out in this order:

# \o SXE Profile name \o Access Controls Effect \o Information display \o Risk level

my @cell_map = qw/
    PROFILE 
    EFFECT
    DISPLAY
    RISK
    /;

my $found_table = undef;
my %profile = ();
my $lines = "";
my $first_time_thru = 1;
my $cell = 0;
my $row = 0;
my $device = $ENV{'DEVICE'};

sub output_code()
{

    my $dom_str = $profile{'PROFILE'} || warn "No domain string in row $row";
    my $colour = '#000000';
    if ( $profile{'RISK'} eq 'High' )
    {
        $colour = '#CC0000';
    }
    elsif ( $profile{'RISK'} eq 'Medium' )
    {
        $colour = '#FF9900';
    }
    elsif ( $profile{'RISK'} eq 'Low' )
    {
        $colour = '#66CC00';
    }
    my $text = $profile{'DISPLAY'};
    if ( $text =~ m/\\i \{(.*?)\}/ )
    {
        $text = $1;
    }
    my $terminal_type = $device ? "device" : "phone";
    $text =~ s/\[phone\|device\]/$terminal_type/ ;

    print <<END_TEXT;
    "$dom_str",
    QT_TRANSLATE_NOOP( "PackageView", "<font color=\"$colour\"><b>$text</b></font>" ),
END_TEXT
    # print "\n\n=================================================================\n\n";
}

sub collate_cell()
{
    $lines =~ s/^\s+// ;  # remove whitespace at beginning of line
    $lines =~ s/\s+$// ;  # remove whitespace at end of line
    unless ( $lines )
    {
        return;
    }
    $lines =~ s/\s+/ / ;  # replace runs of whitespace with a single space
    $profile{$cell_map[$cell]} = $lines;
    # print "\tNew cell $lines - >>>" . $cell_map[$cell] . "<<<\n\n\n";
    $lines = "";
}

while (<>)
{
    if ( $found_table || m/\\header\s+\\o\s+SXE Profile/ )
    {
        $found_table = 1;
    }
    next unless $found_table;

    if ( $first_time_thru )
    {
        warn "Found table at row $.\n";
        undef $first_time_thru;
        print <<END_CODE;
const char *domainStrings[] = {
END_CODE
        next;
    }

    if ( m/\\row|\\endtable/ ) # end of previous row
    {
        # print "matched \\row - endtable\n";
        collate_cell();
        my $cell_count = keys %profile;
        if ( $cell_count == 4 )
        {
            output_code();
        }
        else
        {
            warn "Found $cell_count cells in row $row!\n" unless ( $row == 0 );
        }
        %profile = ();
        $lines = "";
        $cell = 0;
        $row++;
    }
    elsif ( m/\\o (.*?)$/ ) # beginning of a new cell
    {
        my $profile = $1;
        if ( $lines )
        {
            collate_cell();
            $lines = "";
            $cell++;
        }
        $lines = $profile;
        # print "matched \\o --- >>>$1<<<\n";
    }
    else  # continuation of a cell
    {
        $lines .= $_;
    }
}

print <<END_FILE;
    0,
    0
};
END_FILE
