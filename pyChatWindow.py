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

# This module is the Metaverse Chat window
# Its main public method are:
# - DisplayChatWindow( parentmodule ) which takes an object of type PyGUIApp, with a callable method SendToClient
#   and creates a ChatWindow, returning a reference to the caller
# - Die, called on a ChatWindow object, which causes the ChatWindow to die
#
# The caller can send messages to ChatWindow by calling the function DisplayMessage,
# passing in an approrpiate XML IPC dom

import wx

import random
import time
import struct
import string

import os
import sys

import xml.dom.minidom
import traceback

class FrameChatWindow(wx.Frame):
   def __init__(self, parent, title, parentmodule ):
       wx.Frame.__init__(self, parent, -1, title,
                         pos=(100, 640), size=(600, 130), style=wx.STAY_ON_TOP | wx.DEFAULT_FRAME_STYLE )

       self.parentmodule = parentmodule

       # Now create the Panel to put the other controls on.
       panel = wx.Panel(self)

       # and a few controls  wxEVENT_TYPE_TEXT_ENTER_COMMAND  
       chathistory = wx.TextCtrl(panel, -1, "", style = wx.TE_MULTILINE | wx.TE_READONLY )
       chathistory.SetFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL))
       chathistory.SetSize(chathistory.GetBestSize())
       chathistory.SetEditable( False );
       self.chathistory = chathistory
       self.FirstChatLineAdded = False

       chatentry = wx.TextCtrl(panel, -1, "", style=wx.TE_PROCESS_ENTER)
       chatentry.SetFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL))
       chatentry.SetSize(chatentry.GetBestSize())
       self.chatentry = chatentry
       self.chatentry.Clear()
       self.chatentry.SetFocus();

       self.chatentry.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
       self.Bind( wx.EVT_SET_FOCUS, self.OnSetFocus)

       #button = wx.Button(panel, -1, "Close")
       #self.Bind(wx.EVT_BUTTON, self.OnTimeToClose, button)
       
       sizer = wx.BoxSizer(wx.VERTICAL)
       sizer.Add(chathistory, 1, wx.EXPAND)
       sizer.Add(chatentry, 0, wx.EXPAND)
       #sizer.Add( button, 0, wx.EXPAND )
       panel.SetSizer(sizer)
       panel.Layout()
       
   def Die( self ):
      self.Destroy()
       
   def OnKeyDown(self, evt):
       #print "onkeydown"
       
       if( evt.GetKeyCode() == wx.WXK_RETURN ):
       
          if( self.chatentry.GetLineText(0) == "" ):
             self.parentmodule.GiveFocusToRenderer()
             
          else:          
             self.SendComms( self.chatentry.GetLineText(0) )
             self.chatentry.Clear()
       evt.Skip()
       
   def OnSetFocus( self, evt ):
       # print "onsetfocus"
       pass

   def SendComms( self, Message ):
      bOOC = False
      if( string.lower( Message[:5] ) == "/ooc " ):
         bOOC = True
         Message = Message[5:]
      Message = string.replace( Message, "<","&lt;" )
      Message = string.replace( Message, ">","&gt;" )
      Message = string.replace( Message, "&","&amp;" )
      Message = string.replace( Message, "\"","&quot;" )
      Message = string.replace( Message, "'","&apos;" )
      Message = string.replace( Message, "/","" )
      Message = string.replace( Message, "\\","" )
      if( bOOC ):
         self.parentmodule.SendToServer( "<comm message=\"" + Message + "\" type=\"ooc\"/>\n" )
      else:
         self.parentmodule.SendToServer( "<comm message=\"" + Message + "\" type=\"say\"/>\n" )   

   def DisplayMessage( self, sSourceName, sType, sMessage ):
      # print "chatwindow.displaymessage()"
      
      if self.FirstChatLineAdded:
         self.chathistory.AppendText( "\n" )
         
      self.FirstChatLineAdded = True
      
      if( sType == "ooc" ):
         self.chathistory.AppendText( sSourceName + " ooc: " + sMessage )
            
      elif( sType == "say" ):
         self.chathistory.AppendText( sSourceName + ": " + sMessage )

def DisplayChatWindow( parentmodule ):
   ParentModule = parentmodule
   frame = FrameChatWindow( None, "Metaverse Chat:", parentmodule )
   frame.Show(True)
   parentmodule.SetTopWindow(frame)
   return frame
