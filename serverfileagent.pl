#!/usr/bin/perl

# Copyright (c) 2005 Mark Wagner
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
#  more details.
#
# You should have received a copy of the GNU General Public License along
# with this program in the file licence.txt; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-
# 1307 USA
# You can find the licence also on the web at:
# http://www.opensource.org/licenses/gpl-license.php
#

#
# This module implements the serverfileagent in Perl
#

# History: 20050414 Mark Wagner - Created, based loosely on the example in perlipc

# TODO: Rate limiting, either global or per-connection
# TODO: Read config.xml for configuration parameters
# TODO: Sanity-check on server parameters
# TODO: Use a proper XML library for parsing messages?
# TODO: Error handling
# TODO: Protection against DoS attacks such as too many uploads

use warnings;
use strict;
use IO::Socket::INET;
use Getopt::Long;
use POSIX ":sys_wait_h";
use Digest::MD5;

my $gMetaverseServerAddx = '127.0.0.1';
my $gMetaverseServerPort = 22140;
my $gClientListenPort = 22174;

my $ClientListen;
my $ServerListen;

my %Kid_Status;

my $verbose = 0;

my $EOL = "\015\012";

########## Signal handler for child processes ##########
sub REAPER {
	my $child;
	# If a second child dies while in the signal handler caused by the
	# first death, we won't get another signal. So must loop here else
	# we will leave the unreaped child as a zombie. And the next time
	# two children die we get another zombie. And so on.
	while (($child = waitpid(-1,WNOHANG)) > 0) {
		$Kid_Status{$child} = $?;
	}
	$SIG{CHLD} = \&REAPER;
}
$SIG{CHLD} = \&REAPER;

########## SIGTERM handler: Do a clean shutdown ##########
sub TERM {
	$ClientListen->shutdown(2) if($ClientListen);
	$ServerListen->shutdown(2) if($ServerListen);
}
$SIG{TERM} = \&TERM;

sub min
{
	my $x = shift;
	my $y = shift;
	if($x > $y)
	{
		return $y;
	}
	else
	{
		return $x;
	}
}

########## Debugging output functions ##########
sub Debug
{
	my $output = shift;
	print STDERR "DEBUG: $output\n";
}

sub Info
{
	my $output = shift;
	print STDERR "INFO: $output\n";
}

sub Warning
{
	my $output = shift;
	print STDERR "WARNING: $output\n";
}

sub Error
{
	my $output = shift;
	print STDERR "ERROR: $output\n";
}

sub GetLocalFilename
{
	my $Filename = shift;
	my $Filetype = shift;
	# TODO: Sanity-check the filename
#	return undef if($Filename =~ /../);	# Relative path in name
#	return undef if($Filename =~ /\//); # Unix path separator
#	return undef if($Filename =~ /\\/); # DOS path separator
#	return undef if($Filename =~ /:/);  # Old-style Mac path separator
	
	# Determine the location for the file
	if($Filetype eq 'TEXTURE')
	{
		return "./ServerData/Textures/$Filename";
	}
	elsif($Filetype eq 'SCRIPT')
	{
		return "./ServerData/Scripts/$Filename";
	}
	elsif($Filetype eq 'TERRAIN')
	{
		return "./ServerData/Terrains/$Filename";
	}
	elsif($Filetype eq 'MESHFILE')
	{
		return "./ServerData/MeshFiles/$Filename";
	}
	else
	{
		Warning("Unknown file type $Filetype");
		return undef;
	}
}

########## Process a client upload ##########
sub ProcessIncomingFile
{
	my ($Socket, $Filename, $Filetype, $Filesize, $Checksum) = @_;
	my $Buffer;
	my $BytesRecv = 0;
	my $LocalFilename;
	my $MD5 = Digest::MD5->new();
	
	Debug("Incoming file: $Filename type: $Filetype size: $Filesize checksum: $Checksum");
	# Verify the filename is safe and get the local path
	$LocalFilename = GetLocalFilename($Filename, $Filetype);	
	
	# Open the local file
	Debug("Output file: $LocalFilename");
	open OUTFILE, ">", $LocalFilename;
	
	# Recieve the data
	while($BytesRecv < $Filesize and $Socket->connected())
	{
		my $BytesToGet = min(($Filesize - $BytesRecv), 4096);
		$Socket->recv($Buffer, $BytesToGet, MSG_DONTWAIT);
		print OUTFILE $Buffer;
		$MD5->add($Buffer);
		$BytesRecv += length($Buffer);
		Debug("$BytesRecv got");
	}
	close OUTFILE;
	
	Debug("File got");
	
	# Verify the MD5 sum
	my $Digest = $MD5->hexdigest();
	Debug("Digest $Digest expected $Checksum");
	if($Digest ne $Checksum)
	{
		# Failed checksum.  Report
		$Socket->print("<fileuploadfail type=\"$Filetype\" sourcefilename=\"$Filename\" checksum=\"$Checksum\" />$EOL");
		Warning("Checksum failed for $LocalFilename.  Expected $Checksum, was $Digest");
	}
	else
	{
		# Checksum verified.  Reply
		$Socket->print("<fileuploadsuccess type=\"$Filetype\" sourcefilename=\"$Filename\" checksum=\"$Checksum\" />$EOL");
		# Report to the server
		$ServerListen->print("<newfileupload type=\"$Filetype\" sourcefilename=\"$Filename\" serverfilename=\"$LocalFilename\" checksum=\"$Checksum\"/>$EOL");
		Info("File $LocalFilename uploaded");
	}
}

########## Process a client download ##########
sub ProcessOutgoingFile
{
	my ($Socket, $Filename, $Filetype, $Checksum) = @_;
	Debug("Outgoing file: $Filename type: $Filetype checksum: $Checksum");
	# Open the local file
	my $LocalFilename = GetLocalFilename($Filename, $Filetype);
	my $Buffer;
	# Shovel data through the connection
	open INFILE, "<", $LocalFilename;
	while(!eof(INFILE) and $Socket->connected())
	{
		read(INFILE, $Buffer, 4096);
		$Socket->print($Buffer);
	}
}

########## Check for new connections ##########
sub AcceptNewClient
{
	my $Socket = shift;
	my $Command = '';
	my $ChildPid;
	my $Filename;
	my $Filetype;
	my $Filesize;
	my $Checksum;
	# If there's a new connection, accept
	my $NewSocket = $Socket->accept();
	
	# Fork off a child process to handle it
	if(($ChildPid = fork()) == 0)
	{
		# We're the new process
		
		# Read the command from the client
		# TODO: Handle all line-end variants
		my $Char = '';
		while($Char ne "\012")
		{
			$NewSocket->recv($Char, 1, 0);
			$Command .= $Char;
		}
		Debug("Command: $Command");
		
		# Process appropriately
		if($Command =~ /<loadergetfile/) 
		{
			($Filename) = $Command =~ /serverfilename="([^"]+)"/;
			($Filetype) = $Command =~ /type="([A-Za-z]+)"/;
			($Checksum) = $Command =~ /checksum="([0-9a-fA-F]*)"/;
			ProcessOutgoingFile($NewSocket, $Filename, $Filetype, $Checksum);
		}
		elsif($Command =~ /<loadersendfile/)
		{
			($Filename) = $Command =~ /sourcefilename="([^"]+)"/;
			($Filetype) = $Command =~ /type="([A-Za-z]+)"/;
			($Filesize) = $Command =~ /filesize="(\d+)"/;
			($Checksum) = $Command =~ /checksum="([0-9a-fA-F]*)"/;
			ProcessIncomingFile($NewSocket, $Filename, $Filetype, $Filesize, $Checksum);
		}
		$NewSocket->shutdown(2);
		exit;
	}
	else
	{
		$Kid_Status{$ChildPid} = 1;
	}
}


########## Program entry point ##########

# Parse the command line for options
GetOptions(	'verbose+' => \$verbose, 
            'serverip' => \$gMetaverseServerAddx,
			'serverport' => \$gMetaverseServerPort,
			'listenport' => \$gClientListenPort );

# Create the client transfer listener
if(!($ClientListen = IO::Socket::INET->new(LocalPort => $gClientListenPort, Listen => 4, Proto => 'tcp')))
{
	print "ERROR: Failed to open client listener\n";
}
else
{
	# Connect to the metaverse server
	if(!($ServerListen = IO::Socket::INET->new(PeerAddr => $gMetaverseServerAddx, PeerPort => $gMetaverseServerPort, Proto => 'tcp')))
	{
		print "ERROR: Failed to open server connection\n";
	}
	else
	{
		while(1)
		{
			# Block until something happens on a socket
			my $sendvec = '';
			vec($sendvec, $ClientListen->fileno(), 1) = 1;
			vec($sendvec, $ServerListen->fileno(), 1) = 1;
			select($sendvec, undef, undef, undef);
			# TODO: Verify the server's still alive

			# Check for new client connections
			if(vec($sendvec, $ClientListen->fileno(), 1))
			{
				AcceptNewClient($ClientListen);
			}
			# Check for server instructions
			if(vec($sendvec, $ServerListen->fileno(), 1))
			{
				# TODO: Parse server commands
			}

		}
	}
}
########## Shutdown ##########
$ClientListen->shutdown(2) if($ClientListen);
$ServerListen->shutdown(2) if($ServerListen);
# Wait for all child processes to terminate
1 while(wait() != -1);
exit;
