#!/usr/bin/perl
#
# Lists members of all users, all groups, or optionally just the group
# specified on the command line.
# Invokes add_users_and_groups with extracted user or group as command line arguments.


# Copyright © 2010-2013 by Zed Pobre (zed@debian.org or zed@resonant.org)
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#

use strict; use warnings;

$ENV{"PATH"} = "/usr/bin:/bin";

my $hostid = shift;

my %groupmembers;
my $usertext = `getent passwd`;

my @users = $usertext =~ /^([a-zA-Z0-9_-]+):/gm;

foreach my $userid (@users)
{
    my $usergrouptext = `id -G $userid`;
    my $useridtoadd = `id -u $userid`;
    my $uidcmd = "./add_users_and_groups 0 $hostid $useridtoadd";
    `$uidcmd`;
    my @grouplist = split(' ',$usergrouptext);

    foreach my $group (@grouplist)
    {
        $groupmembers{$group}->{$useridtoadd} = 1;
    }
}

foreach my $group (sort keys %groupmembers)
{
    my $grpcmd = "./add_users_and_groups 1 $hostid $group ";
    foreach my $member (sort keys %{$groupmembers{$group}})
    {
        $member =~ s/^ *//;
        $member =~ s/ *$//;
        $member =~ s/\n*$//;
        $grpcmd .= "$member ";
    }
    $grpcmd =~ s/ *$//;
    $grpcmd .= "\n";
    `$grpcmd`;
}
