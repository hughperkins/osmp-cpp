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

import random
import time
import socket
import struct
import string
import os
import sys
import thread
import traceback

import pyConfigFileUpdating

import pyGUIApp
import pyMainThread

options = None

class SecondThreadClass:
    def __init__( self ):
        print "thread init"

    def Start(self):
        print "thread start"
        self.keepGoing = self.running = True
        thread.start_new_thread(self.Run, ())

    def Stop(self):
        self.keepGoing = False

    def IsRunning(self):
        return self.running

    def Run(self):
        try:
           print "thread run:"
           while self.keepGoing:
               print "Running..."
               global options
               app = pyGUIApp.PyGUIApp(True, self, options )
               app.MainLoop()
               #StartCPPClient()   
        except:
            traceback.print_exc(file=sys.stdout)
            traceback.print_exc(file=sys.__stderr__)
            while 1:
               time.sleep(1.0)

        self.running = False

def StartApp():
   SecondThread = SecondThreadClass()
   SecondThread.Start()
   time.sleep( 0.1)
   try:
      pyMainThread.StartCPPClient()   
   except:
      traceback.print_exc(file=sys.stdout)
      traceback.print_exc(file=sys.__stderr__)
      while 1:
         time.sleep(1.0)

def ParseOptions():
   global options
   from optparse import OptionParser
   parser = OptionParser()
   parser.add_option("-u", "--user", dest="Username", default="",
                  help="Specify username" )
   parser.add_option("-p", "--password", dest="Password", default="",
                  help="Specify password" )
   parser.add_option("-a", "--authserver", dest="AuthServerIP", default="127.0.0.1",
                  help="Specify authentication server" )
   parser.add_option("--authserverport", dest="AuthServerPort", default="25101",
                  help="Specify authentication server port" )
   parser.add_option("-s", "--simserver", dest="SimServerIP", default="",
                  help="Specify sim server" )
   parser.add_option("--simserverport", dest="SimServerPort", default="22165",
                  help="Specify sim server" )
   parser.add_option("--spawnnewwindows", dest="SpawnNewWindows", default="",
                  help="Tech option: spawns fileagent in new window (via batch file)" )
   # parser.add_option("--standalone", dest = "Standalone", help = "Dont connect to server" )

   (options, args) = parser.parse_args()
   
if __name__ == '__main__':
   ParseOptions()
   pyConfigFileUpdating.UpdateConfigXMLFile()
   StartApp()


  



