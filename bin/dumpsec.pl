#!/usr/bin/perl
use strict;
use warnings;

use Cwd;
use Config;
use File::Basename;
use lib ( dirname($0)."/../src/build/bin" );
use Qtopia::Paths;
use Qtopia::Vars;
use Qtopia::Opt;

Qtopia::Paths::get_paths();
Qtopia::Opt::read_config_cache();

use Data::Dumper;

use constant QSXE_KEY_LEN => 16;
use constant DUMP_DEBUG => 0;

# note that the use of the union in the AuthRecord data type causes
# the struct to be padded out with an extra 2 nul's before the time_t
# - at least it does on my platform...
use constant AUTH_RECORD_SIZE => 24;


use constant INSTALL_REC_HEAD_SIZE => $Config{'shortsize'} + 1;

sub usage
{
    warn "usage: $0 [path-to-installed-image] [program-name]\n";
    exit;
}

# if we find the SXE files there assume its an image, not a program name
my @install_root_possibles = @ARGV;
my @verbose_report_progs = @ARGV;

push @install_root_possibles, $QPEDIR;
push @install_root_possibles, opt("prefix");
push @install_root_possibles, opt("prefix", "default");

my $install_root = ".";
foreach my $d ( @install_root_possibles )
{
    if ( -f "$d/etc/keyfile" )
    {
        $install_root = $d;
        last;
    }
}

# if the first argument was an install root, shift it off
if ( (@verbose_report_progs > 0) && ($install_root eq $verbose_report_progs[0]) )
{
    shift @verbose_report_progs;
}

unless ( -d $install_root && -f "$install_root/etc/keyfile" )
{
    warn "Could not find Qt Extended install dir. Did you run \"qbuild image\"?\n";
    exit;
}

print "Reporting on Safe Execution for image: $install_root\n";

my %program_ids = ();
my %bin_names = ();
my %install_ids = ();

my $data;

my $keyfilename = "$install_root/etc/keyfile";

open KEYSEQFILE, "<$keyfilename.sequence" or die "open $keyfilename.sequence : $!\n";
binmode KEYSEQFILE;
my $pack_format = "a" . QSXE_KEY_LEN . " x C x x L";
while ( read( KEYSEQFILE, $data, AUTH_RECORD_SIZE ))
{
    my ( $key, $progId, $change_time ) = unpack $pack_format, $data;
    $program_ids{ $progId } = { KEY => $key, CHG => $change_time };
}
close KEYSEQFILE;

if ( DUMP_DEBUG )
{
    print "********* keyfile seq *********\n";
    print Dumper( \%program_ids );
}

=perldoc

struct usr_key_entry
{
    char key[QSXE_KEY_LEN];
    ino_t ino;   // on x86 4 bytes
    dev_t dev;   // on x86 8 bytes
};

=cut

my %devices;

open KEYFILE, "<$keyfilename" or die "open $keyfilename : $!\n";
binmode KEYFILE;
$pack_format = "a" . QSXE_KEY_LEN . " L L L";
while ( read( KEYFILE, $data, 28 ))
{
    my ( $key, $inode, $device_lo, $device_hi ) = unpack $pack_format, $data;
    if ( $device_lo ) # little endian
    {
        die "Oops, dev too big for 32 bit perl" if ( $device_hi );
        $devices{$device_lo}->{$inode} = $key;
    }
    else              # big endian
    {
        die "Oops, dev too big for 32 bit perl" if ( $device_lo );
        $devices{$device_hi}->{$inode} = $key;
    }
}
close KEYFILE;

if ( DUMP_DEBUG )
{
    print "********* devices *********\n";
    print Dumper( \%devices );
}

my $installs_file_name = "$install_root/etc/installs";
open INSTALLS, $installs_file_name or die "open $installs_file_name: $!\n";
binmode INSTALLS;
while ( read( INSTALLS, $data, INSTALL_REC_HEAD_SIZE ))
{
    my ( $install_id ) = unpack "S x", $data;
    # print "install id:", $install_id;
    my $full_path = <INSTALLS>;
    # print "\t$full_path";
    chomp $full_path;

    my ( $bin_short_name ) = $full_path =~ m/([^\/]+)$/ ;
    unless ( $bin_short_name )
    {
        warn "couldn't understand path $full_path\n";
        next;
    }
    if ( $bin_short_name =~ /^lib(\S+?)\.so/ )
    {
        $bin_short_name = $1;
    }
    # print "\t$bin_short_name\n";
    $bin_names{ $bin_short_name } = { ID => $install_id, PATH => $full_path };
    $install_ids{ $install_id } = { BIN => $bin_short_name };
}
close INSTALLS;

if ( DUMP_DEBUG )
{
    print "********* installs *********\n";
    print Dumper( \%bin_names );
}

=pod

struct IdBlock
{
    quint64 inode;
    quint64 device;
    unsigned char pad;
    unsigned char progId;
    unsigned short installId;
    unsigned int keyOffset;
    qint64 install_time;
};

=cut

use constant MANIFEST_REC_SIZE => 32;

my $time_now = time;

my $manifest_file_name = "$install_root/etc/manifest";
open MANIFEST, $manifest_file_name or die "open $manifest_file_name : $!\n";
binmode MANIFEST;
while ( read( MANIFEST, $data, MANIFEST_REC_SIZE ))
{
    my ( $inode_lo, $inode_hi, $device_lo, $device_hi, $progId, $installId, $key_offset, $install_time_lo, $install_time_hi ) =
        unpack "I I I I x C S I i i", $data;
    # print "DEBUG: inode:", $inode_lo, "  device:", $device_lo, "  prog id:", $progId, "  install id:", $installId, 
    #     "  offset: ", $key_offset, "  install time:", $install_time_lo, "\n";

    if ( !exists( $install_ids{ $installId } ))
    {
        warn "Install $installId ( in $install_root/etc/manifest ) not listed in $install_root/etc/installs\n";
        $install_ids{ $installId } = {};
    }
    my $inst_hashref = $install_ids{ $installId };
    if ( $inode_lo ) # litle endian
    {
        $inst_hashref->{'INODE'} = $inode_lo;
        $inst_hashref->{'DEV'} = $device_lo;
        $inst_hashref->{'PROG'} = $progId;
        $inst_hashref->{'TIME'} = $install_time_lo;
        $inst_hashref->{'OFFS'} = $key_offset;
    }
    else
    {
        $inst_hashref->{'INODE'} = $inode_hi;
        $inst_hashref->{'DEV'} = $device_hi;
        $inst_hashref->{'PROG'} = $progId;
        $inst_hashref->{'TIME'} = $install_time_hi;
        $inst_hashref->{'OFFS'} = $key_offset;
    }
}
close MANIFEST;

if ( DUMP_DEBUG )
{
    print "********* manifest *********\n";
    print Dumper( \%install_ids );
}

my %profiles = ();
my $current_profile;
my $profiles_file_name = "$install_root/etc/sxe.profiles";
open PROFILES, $profiles_file_name or die "open $profiles_file_name : $!\n";
while ( <PROFILES> )
{
    chomp;
    if ( m/\[(\w+)\]/ )
    {
        $current_profile = $1;
        $profiles{ $current_profile } = [];
    }
    else
    {
        push @{$profiles{ $current_profile }}, $_;
    }
}
close PROFILES;

my %policies = ();
my $current_policy;
my $policies_file_name = "$install_root/etc/sxe.policy";
open POLICIES, $policies_file_name or die "open $policies_file_name : $!\n";
while ( <POLICIES> )
{
    chomp;
    if ( m/\[(\w+)\]/ )
    {
        $current_policy = $1;
        $policies{ $current_policy } = [];
    }
    else
    {
        push @{$policies{ $current_policy }}, $_;
    }
}
close POLICIES;

if ( DUMP_DEBUG )
{
    print "********* policies *********\n";
    print Dumper( \%policies );
}

my $verbose = undef;

sub scan_path
{
    my $the_path = shift;
    my $the_key = shift;

    # might be an installed package on the desktop, located in $the_path already
    unless ( -x $the_path )
    {
        unless ( $the_path =~ m/^$install_root/ )  # is the binary in the install root?
        {
            # no, its probably set with -prefix=/opt/Qtopia or similar
            my ( $prefix, $rel_path ) = $the_path =~ m/^(.*?)\/(bin\/.*?)$/ ;
            unless ( $rel_path )
            {
                ( $prefix, $rel_path ) = $the_path =~ /^(.*?)\/(plugins\/application\/.*?)$/ ;
            }
            $the_path = "$install_root/$rel_path";
            print "\t\tRun-time prefix $prefix assumed - looking for binary at $the_path\n";
        }
    }
    open BIN, $the_path or warn "\t\t******** open $the_path : $!\n" and return;
    binmode( BIN );
    local $/ = undef;  # slurp mode
    my $the_data = <BIN>;
    close BIN;
    if ( $the_data =~ m/\Q$the_key\E(..)/s )
    {
        my ( $stuff, $progid ) = unpack "C2", $1;
        if ( $verbose )
        {
            print "\t\tChecked $the_path: found PROG ID: $progid - KEY matched as above\n";
        }
    }
    else
    {
        warn "\t\t********* key is not found in $the_path\nRe-run \"qbuild image\"...?\n";
    }
}

sub report_by_short_name
{
    my $p = shift;
    print "---> $p\n";
    my $was_prog_id;
    if ( !exists $bin_names{ $p } )
    {
        foreach my $inst_id ( keys %install_ids )
        {
            if ( $install_ids{$inst_id}->{'PROG'} eq $p )  # ok, it was a prog id
            {
                $was_prog_id = "yes";
                report_by_short_name( $install_ids{$inst_id}->{'BIN'} );
            }
        }
        unless ( $was_prog_id )
        {
            print "\tNOT FOUND\n";
        }
        return;
    }
    my $install_id = $bin_names{$p}->{'ID'};
    my $program_id = $install_ids{ $install_id }->{'PROG'};
    my $prog_path = $bin_names{$p}->{'PATH'};

    my $key;
    print "\tProgram id: $program_id\n" if ( defined $program_id );
    if ( $verbose && defined $program_id )
    {
        if ( exists $program_ids{$program_id} )
        {
            my $dev = $install_ids{$install_id}->{'DEV'};
            my $ino = $install_ids{$install_id}->{'INODE'};
            $key = $devices{$dev}->{$ino};
            my $key_change_time = $time_now - $program_ids{$program_id}->{'CHG'};
            my $byte;
            print "\tkey:";
            foreach $byte ( unpack "C16", $key )
            {
                printf "%x", $byte;
            }
            # currently not used
            # print " - changed $key_change_time seconds ago\n";
            print "\n";
            scan_path( $prog_path, $key );
        }
        else
        {
            print "ERROR: No keyfile entry for $program_id\n";
        }
    }
    else
    {
        if ( $key )
        {
            scan_path( $prog_path, $key );
        }
    }
        
    print "\tInstalled:  $prog_path\n";
    if ( $verbose )
    {
        print "\t\t" . ( $time_now - $install_ids{$install_id}->{'TIME'} ) . " seconds ago\n";
        print "\t\tinode:" . $install_ids{$install_id}->{'INODE'};
        print "\tdevice:" . $install_ids{$install_id}->{'DEV'};
        print "\n";
        if ( exists $policies{$program_id} )
        {
            print "\tSXE Profiles:\n";
            my @profile_names = @{$policies{$program_id}};
            my $needs_eol = undef;
            foreach my $prof ( @profile_names )
            {
                if ( $prof =~ /(\w+)\{/ )  # doc mimetypes are eg "docs{image/*}"
                {
                    $prof = $1;
                }
                print "\t\t$prof\n";
                if ( !exists $profiles{$prof} && $prof ne "none" )
                {
                    print "\t\t\tERROR: $prof is not a profile listed in $profiles_file_name\n";
                }
                foreach my $line ( @{$profiles{$prof}} )
                {
                    print "\t\t\t$line\n";
                }
            }
        }
        else
        {
            print "\tNo SXE profiles found, this program will likely not run under SXE!\n";
        }
    }
}

if ( @verbose_report_progs )
{
    $verbose = "yes";
    foreach my $p ( @verbose_report_progs )
    {
        report_by_short_name( $p );
    }
}
else
{
    foreach my $p ( keys %bin_names )
    {
        report_by_short_name( $p );
    }
}
