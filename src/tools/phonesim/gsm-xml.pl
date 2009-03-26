#!/usr/bin/perl

use strict;
use warnings;

#
# Program to convert a GSM Specification into an xml data file
#

use Cwd;
use File::Find;

#
# Iterate over the directory tree, looking for files containing test cases.
#

=head1 SYNOPSIS

gen-report

=head1 DESCRIPTION

Generate reports of tests from html files
=cut

my $title_regex = qr/^[0-9.]+[0-9]+\s+[\w\s]+\+\w/ ;
my $command_regex = qr/^[0-9.]+[0-9]+\s+[\w\s]+\+(.+)/ ;
my $profile_regex = qr/^[0-9.]+[0-9]+\s+([\w\s]+)\+\w/ ;

my $format_regex = qr/Table.+parameter command syntax/ ;
my $responses_regex = qr/Possible responses/ ;
my $description_regex = qr/Description\n+.*Implementation/ ;

# total count of specs parsed
my $spec_count = 0;
my $inFile = "GSMSpecification.txt";
my $outFile = "GSMSpecification.xml";

open ( INFILE, $inFile ) or die "Couldn't open $inFile : $!";

my $command;
my $profile;
my $format;
my $response;
my $description;
my @commandlist;

my $line;

open(OUTFILE, ">$outFile") || die "Can't open $outFile : $!";
print OUTFILE "<gsm>\n";

while( defined ($line = <INFILE>) ){

    $command = "";
    $profile = "";
    $format = "";
    $response = "";
    $description = "";
    @commandlist = ();

    if( $line =~ m/$title_regex/ ){

        $line =~ /^[0-9.]+[0-9]+\s+([\w\s]+)\+(.*)[\s\n]*?/;
        $command = $2;
        $profile = $1;
        chomp($command);
        $command =~ s/\s//;
        chomp($profile);

        while( $line !~ /^Possible response\(s\)/ ){
            $line = <INFILE>;
        }

        while( $line !~ /^Description/ ){
            $line = <INFILE>;
            if( $line !~ /:/ ){
                if($line =~ /\+($command.*)/){
                    my $commandString = $1;
                    if($commandString =~ /\?/){
                        $commandString =~ s/\?.*/?/;
                    }elsif($commandString =~ /=/){
                        $commandString =~ s/=.*/=/;
                    }

                    push( @commandlist, $commandString );
                }
            }
        }

        while( $line !~ /^Description/ ){
            $line = <INFILE>;
        }

        if( $line =~ /^Description/ ){
            $line = <INFILE>;
            while( $line !~ /^Implementation/ ){
                $description = $description.$line;
                $line = <INFILE>;
            }
            $description =~ s/</[/g;
            $description =~ s/>/]/g;
            $description =~ s/"/'/g;
            $description =~ s/&/:/g;

            foreach my $commandItem(@commandlist){
                print OUTFILE "<spec>\n";
                print OUTFILE "    <command>AT+$commandItem</command>\n";
                if( $commandItem =~ /=\?/ ){
                    print OUTFILE "    <profile>$profile(Query all)</profile>\n";
                }elsif($commandItem =~ /\?/){
                    print OUTFILE "    <profile>$profile(Query)</profile>\n";
                }elsif($commandItem =~ /=/){
                    print OUTFILE "    <profile>$profile(Assign)</profile>\n";
                }else{
                    print OUTFILE "    <profile>$profile</profile>\n";
                }
                print OUTFILE "    <description>$description</description>\n";
                print OUTFILE "</spec>\n";
            }
        }
    }


}



print OUTFILE "</gsm>";


close INFILE;
close OUTFILE;

print "Done\n\n\n\n";

