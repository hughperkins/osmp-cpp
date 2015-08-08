#!/usr/bin/env python
# Copyright Hugh Perkins 2004
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

import random
import time
import struct
import string

import os
import sys

import xml.dom.minidom
import traceback

bRestore = False

if( len( sys.argv ) > 1 ):
   if( sys.argv[1] == "--restore" ):
      print "Option --restore selected"
      bRestore = True

def SnapshotDB():	
	   dlg = wx.FileDialog(
	      None, message="Save sim to what filename?", defaultDir="%userprofile%\my documents\OSMP Inventory\Sims", 
	      defaultFile="", wildcard="*.sim", style=wx.SAVE | wx.CHANGE_DIR
	      )
	        
	   path = ""
	   if dlg.ShowModal() == wx.ID_OK:
	      paths = dlg.GetPaths()
	
	      for path in paths:
	         pass
	
	   dlg.Destroy()
	   
	   if( path != "" ):
	      # L = [ "snapshot.db", "\"" + path + "\"" ]
	      os.system( "snapshotdb.bat \"" + path + "\"" )

def RestoreDB():	
	   dlg = wx.FileDialog(
	      None, message="Please select a sim file:", defaultDir="%userprofile%\my documents\OSMP Inventory\Sims", 
	      defaultFile="", wildcard="*.sim", style=wx.OPEN | wx.CHANGE_DIR
	      )
	        
	   path = ""
	   if dlg.ShowModal() == wx.ID_OK:
	      paths = dlg.GetPaths()
	
	      for path in paths:
	         pass
	
	   dlg.Destroy()
	   
	   if( path != "" ):
	      os.system( "restoredb.bat \"" + path + "\"" )

class DummyApp(wx.App):
   def OnInit(self):
      return True            
         
def StartApp():
   app = DummyApp(True)
   if bRestore:
      RestoreDB()
   else:
      SnapshotDB()

if __name__ == '__main__':
   try:
      StartApp()   
   except:
      traceback.print_exc(file=sys.stdout)
      while 1:
         time.sleep(1.0)
