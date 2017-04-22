#!/usr/bin/perl

#
# Copyright 2014, Cypress Semiconductor
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Cypress Semiconductor;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Cypress Semiconductor.
#

# Syntax:
# ./print_file.pl <file>
if ( $#ARGV != 0 )
{
    print "Incomplete arguments";
    exit;
}

my $file = $ARGV[0];

# Open message file
open INFILE, "<:raw", $file or die "cant open " . $file;

while ($line = <INFILE>)
{
    $line =~ s{\r|\n}{}g;
    print "$line\n";
}

close INFILE;

# Exit successfully
exit(0);
