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

# This module is a login dialog used by metaverseclient to obtain login credentials
# and the IP address of the target Metaverse server

import wx
from wxPython.wx import *
 
import random
import time
import thread
import socket
import struct
import string

import os
import sys

import xml.dom.minidom
import traceback

import wx.lib.newevent

import  wx.lib.mixins.listctrl as listmix

iDefaultAuthServerPort = 25101

sMetaverseServerIP = "127.0.0.1"
sMessage = ""

import mvEvents

class SimListCtrl(wx.ListCtrl, listmix.ListCtrlAutoWidthMixin):
    def __init__(self, parent, ID, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0):
        wx.ListCtrl.__init__(self, parent, ID, pos, size, style)
        listmix.ListCtrlAutoWidthMixin.__init__(self)

def Connect( sSimName, sSimIP, sSimPort ):
   s.send( "<mvserverselect mvserverip=\"" + sSimIP + "\" mvserverport=\"" + sSimPort + "\"/>\n" );

class ChooseSimServerFrame(wx.Frame):
    def __init__(self, parent, parentmodule, AvailableServers ):
        wx.Frame.__init__(self, parent, -1, "Please choose a sim:",
                          pos=(100, 100), size=(600, 600), style=wx.DEFAULT_FRAME_STYLE )

        self.ParentModule = parentmodule
        panel = wx.Panel(self)

        sizer = wx.BoxSizer(wx.VERTICAL)
 #       gs = self.gs = wx.GridBagSizer(2, 1)
        self.list = SimListCtrl(panel, -1,
                                 style=wx.LC_REPORT 
                                 #| wx.BORDER_SUNKEN
                                 | wx.BORDER_NONE
                                 | wx.LC_EDIT_LABELS
                                 #| wxLC_NO_HEADER
                                 #| wxLC_VRULES | wxLC_HRULES
                                 | wx.LC_SINGLE_SEL 
                                 )
                                 
        self.list.InsertColumn(0, "Sim Name")
        self.list.InsertColumn(1, "Sim IP")
        self.list.InsertColumn(2, "Sim Port")
        
        sizer.Add( self.list, 1, wx.EXPAND )

        font = wx.Font(12, wx.SWISS, wx.NORMAL, wx.BOLD)
        
        button = wx.Button(panel, -1, "Connect")
        sizer.Add( button, 0, wx.EXPAND )
#        gs.Add( button,
#              (1,0), (1,1), wx.ALIGN_CENTER | wx.ALL, 1)
        self.Bind(wx.EVT_BUTTON, self.ConnectButton, button)
        self.Bind(wx.EVT_LIST_ITEM_ACTIVATED, self.ConnectButton, self.list);

        panel.SetSizer(sizer)        
        panel.Layout()
        
        for AvailableServer in AvailableServers:
           iNextIndex = self.list.GetItemCount()
           self.list.InsertStringItem( iNextIndex, AvailableServer["Name"] )
           self.list.SetStringItem(iNextIndex, 1, AvailableServer["IPAddress"] )
           self.list.SetStringItem(iNextIndex, 2, str(AvailableServer["Port"]) )
        
    def DieNow( self, evt ):
       self.Destroy()
        
    def ConnectButton( self,evt):
       print "ConnectButton"
       print self.list.GetSelectedItemCount()
       if self.list.GetSelectedItemCount() == 1:
          item = self.list.GetNextItem(-1, wx.LIST_NEXT_ALL, wx.LIST_STATE_SELECTED)
          #print item
          if ( item != -1 ):
             simname = self.list.GetItem( 0 ).GetText()
             simip = self.list.GetItem( 0, 1 ).GetText()
             simport = int( self.list.GetItem( 0, 2 ).GetText() )
             evt = mvEvents.OnUserSelectsSimServer( Name=simname, IPAddress = simip, Port = int(simport))
             wx.PostEvent( self.ParentModule, evt )
             
             #self.ParentModule.OnUserSelectsSimServer( simname, simip, simport )
             self.Destroy()
        
    def OnSetFocus( self, evt ):
       pass
       print "onsetfocus"
        
def ShowChooseSimServerFrame( self, AvailableServers ):
   self.SimServerFrame = ChooseSimServerFrame(None, self, AvailableServers )
   self.SimServerFrame.Show(True)
   self.SetTopWindow(self.SimServerFrame)
   return self.SimServerFrame

class LoginFrame(wx.Frame):
    def __init__(self, parent, MainApp, message ):
        wx.Frame.__init__(self, parent, -1, "Login to Auth Server",
                          pos=(300, 300), size=(430, 300), style=wx.DEFAULT_FRAME_STYLE )
#                          pos=(300, 300), size=(430, 300), style=wx.STAY_ON_TOP | wx.DEFAULT_FRAME_STYLE )

        self.ParentModule = MainApp
        # print str( self.parentmodule )
        
        self.Message = message

        # Now create the Panel to put the other controls on.
        panel = wx.Panel(self)
        # gs = wx.FlexGridSizer(5, 2, 2, 2)  # rows, cols, hgap, vgap
        gs = self.gs = wx.GridBagSizer(5, 2)

        font = wx.Font(12, wx.SWISS, wx.NORMAL, wx.BOLD)

        text = wx.StaticText(panel, -1, "Please fill in your avatar name and server IP address.\nYou can choose your avatar name." )
        text.SetFont(font)
        gs.Add( text,
              (0,0), (1,2), wx.ALIGN_CENTER | wx.ALL, 5)

        font = wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL)

        text = wx.StaticText(panel, -1, "Avatar name:" )
        text.SetFont(font)
        gs.Add( text,
              (1,0), (1,1), wx.ALIGN_RIGHT |wx.ALIGN_CENTER_VERTICAL | wx.ALL, 5)
        loginnamebox =  wx.TextCtrl(panel, -1, "", style=wx.TE_PROCESS_ENTER )
        loginnamebox.SetFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL))
        loginnamebox.SetSize(loginnamebox.GetBestSize())
        self.loginnamebox = loginnamebox
        self.loginnamebox.Clear()
        self.loginnamebox.SetFocus();
        gs.Add( loginnamebox,
              (1,1), (1,1), wx.ALIGN_CENTER | wx.ALL, 5)
        
        text = wx.StaticText(panel, -1, "Password (could be blank):" )
        text.SetFont(font)
        gs.Add( text,
              (2,0), (1,1), wx.ALIGN_RIGHT |wx.ALIGN_CENTER_VERTICAL | wx.ALL, 5)
        loginpasswordbox =  wx.TextCtrl(panel, -1, "", style=wx.TE_PROCESS_ENTER )
        loginpasswordbox.SetFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL))
        loginpasswordbox.SetSize(loginpasswordbox.GetBestSize())
        self.loginpasswordbox = loginpasswordbox
        self.loginpasswordbox.Clear()
        gs.Add( loginpasswordbox,
              (2,1), (1,1), wx.ALIGN_CENTER | wx.ALL, 5)
        
        text = wx.StaticText(panel, -1, "Login Server:" )
        text.SetFont(font)
        gs.Add( text,
              (3,0), (1,1), wx.ALIGN_RIGHT |wx.ALIGN_CENTER_VERTICAL| wx.ALL, 5)
        targetipbox =  wx.TextCtrl(panel, -1, "", style=wx.TE_PROCESS_ENTER )
        targetipbox.SetFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL))
        targetipbox.SetSize(targetipbox.GetBestSize())
        self.targetipbox = targetipbox
        self.targetipbox.Clear()
        self.targetipbox.AppendText(sMetaverseServerIP)
        gs.Add( targetipbox,
              (3,1), (1,1), wx.ALIGN_CENTER | wx.ALL, 5)

        # Message text at bottom of dialog, just above buttons,
        # eg for "couldnt connect to server", etc
        font = wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL)
        text = wx.StaticText(panel, -1, self.Message )
        text.SetFont(font)
        gs.Add( text,
              (4,0), (1,2), wx.ALIGN_CENTER | wx.ALL, 5)
              
        loginbutton = wx.Button(panel, -1, "Login")
        gs.Add( loginbutton,
              (5,0), (1,2), wx.ALIGN_CENTER | wx.ALL, 5)
        self.Bind(wx.EVT_BUTTON, self.LoginButton, loginbutton)

        self.loginnamebox.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.loginpasswordbox.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.targetipbox.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind( wx.EVT_SET_FOCUS, self.OnSetFocus)

        panel.SetSizer(gs)        
        panel.Layout()
        
        #if( bStandalone == False ):
        #   self.socketreadthread = SocketReadThread(self, s)
        #   self.socketreadthread.Start()
        
    def DieNow( self, evt ):
       # print "die event received"
       # self.socketreadthread.Stop()
       self.Destroy()
       
    def Login( self, name, password, server ):
       global iDefaultAuthServerPort
       print "posting login event..."
       evt = mvEvents.OnUserSelectsAuthServer( name=name,password=password,server=server,port = iDefaultAuthServerPort)
       print str( evt )
       wx.PostEvent( self.ParentModule, evt )
       # self.ParentModule.OnUserChoosesAuthServer( name, password, server, iDefaultAuthServerPort )
       self.Destroy()
       
    def LoginButton( self,evt):
       #print str( self )
       self.Login( self.loginnamebox.GetLineText(0), self.loginpasswordbox.GetLineText(0), self.targetipbox.GetLineText(0) )
       #self.Destroy()
        
    def OnKeyDown(self, evt):
        #print "onkeydown"
        if( evt.GetKeyCode() == wx.WXK_RETURN ):
           #print str( self )
           self.Login( self.loginnamebox.GetLineText(0), self.loginpasswordbox.GetLineText(0), self.targetipbox.GetLineText(0) )
           # self.parentmodule.DieNow()
           #evt = mvDieEvent()
           #wx.PostEvent(self.parentmodule, evt)
           #self.Destroy()
        evt.Skip()
        
    def OnSetFocus( self, evt ):
       pass
        #print "onsetfocus"
        
def ShowLoginFrame( MainApp, Message = "" ):
   evt = mvEvents.OnTest()
   wx.PostEvent( MainApp, evt )
   frame = LoginFrame(None, MainApp, Message )
   frame.Show(True)
   MainApp.SetTopWindow(frame)
   return frame
