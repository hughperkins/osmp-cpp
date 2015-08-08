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

# This module is responsible for showing a config dialog
# main public method is DisplayConfigDialog( parentmodule, dom ), which takes in 
# a PyGUIApp object and a parsed XML incoming IPC, containing screenx, screeny attributes

import wx

import random
import time
import struct
import string

import os
import sys

import xml.dom.minidom
import traceback

from configDialogClass import *
#from wxPython.wx import *

configdialog = None
      
def DisplayConfigDialog( parentmodule, startuppath ):   
   global configdialog   
     
   if (configdialog == None ):      
      configdialog = configDialogFrame(startuppath, parentmodule, None, -1, "")
      configdialog.Show(True)
      parentmodule.SetTopWindow(configdialog)
      #parentmodule.frame = configdialog

   else:            
      if (configdialog.IsShown() == False):         
         configdialog.Show(True)
      configdialog.Raise()
      configdialog.loadConfig()
