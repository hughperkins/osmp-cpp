#!/usr/bin/env python
# Copyright Hugh Perkins 2005
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

import wx

# (SocketReadEvent, EVT_SOCKET_READ) = wx.lib.newevent.NewEvent()
(DieEvent, EVT_DIE) = wx.lib.newevent.NewEvent()

(OnShowLoginToAuthServer, EVT_SHOWLOGINTOAUTHSERVER ) = wx.lib.newevent.NewEvent()
(OnUserSelectsSimServer, EVT_ONUSERSELECTSSIMSERVER ) = wx.lib.newevent.NewEvent()
(OnUserSelectsAuthServer, EVT_ONUSERSELECTSAUTHSERVER ) = wx.lib.newevent.NewEvent()

(OnServerCommandForGUI, EVT_ONSERVERCOMMANDFORGUI ) = wx.lib.newevent.NewEvent()
(OnMainThreadCommandForGUI, EVT_ONMAINTHREADCOMMANDFORGUI ) = wx.lib.newevent.NewEvent()

(CommandEvent, EVT_TEST ) = wx.lib.newevent.NewEvent()

(OnTest, EVT_TEST ) = wx.lib.newevent.NewEvent()
