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
//! \brief This header file provides variable declarations for all ports used by the OSMP software


#ifndef __PORT_LIST_H
#define __PORT_LIST_H

extern int iPortRendererIPC;
extern int iPortGUIIPC;
extern int iPortLoginDialog;
extern int iPortFileTransfer;
extern int iPortFileTransferServerPort;

extern int iAuthServerPortForDBInterface;
extern int iPortAuthServerForClients;
extern int iPortAuthServerForSims;

extern int iPortMetaverseServer;     //!< Port to which Metaverse Clients connect;  this is the port that needs to be open on firewall/proxy
extern int iPortDBInterfaceListen;
extern int iPortServerConsoleListen;
extern int iPortFileServerAgentListen;

#endif //__PORT_LIST_H
