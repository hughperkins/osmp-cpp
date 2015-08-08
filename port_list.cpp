// Copyright Mark Wagner 2005
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURVector3E. See the GNU General Public License for
//  more details.
//
// You should have received a copy of the GNU General Public License along
// with this program in the file licence.txt; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-
// 1307 USA
// You can find the licence also on the web at:
// http://www.opensource.org/licenses/gpl-license.php
//

//! \file
//! \brief This module provides port definitions used by the OSMP software

// Consts removed, Hugh Perkins, May 2005.

#include "port_list.h"

// Server-side ports
int iPortDBInterfaceListen = 22170;
int iPortServerConsoleListen = 22171;
int iPortFileServerAgentListen = 22140;

// Auth server
int iAuthServerPortForDBInterface = 25001;  //!< Port to which the Auth DB interface connects

// Client-side ports
int iPortRendererIPC = 22166;
int iPortGUIIPC = 22167;
int iPortLoginDialog = 22168;
int iPortFileTransfer = 22169;    // The port for communication between metaverseclient and clientfiletransferagent
int iPortFileTransferServerPort = 22174;

// Network ports
int iPortAuthServerForClients = 25101;  //!< Port to which Metaverse clients connect; needs to be published to Internet
int iPortAuthServerForSims = 25100;     //!< Port to which Metaverse servers connect; needs to be published to Internet
int iPortMetaverseServer = 22165;     //!< Port to which Metaverse Clients connect;  this is the port that needs to be open on firewall/proxy

