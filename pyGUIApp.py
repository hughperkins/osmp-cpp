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

# This module is a Python GUI for the Metaverse Project
# it communicates with metaverseclient via XML Sockets IPC
# the client passes messages back and forth between the Renderer (rendererglutsplit.exe)
# and this script
# the renderer is in charge for now and will send messages to pygui.py to put up contextmenus
# by default, this scirpt shows a chat window
# into which are pasted chat messages received (from metaverseserver, via metaverseclient)
# the chat window can be used to send messages back to the server, and thence to other players
#
# This module is responsible for connecting to metaverseclient over XML sockets IPC
# It uses PyChatWindow.py and PyContextMenu.py to display the actual windows
# This module provides the function SendToClient to the daughter windows, which will send
# a sockets message to metaverseclient

# Other functions availble to daughter windows:
# - SetFocusToRenderer -> passes Window focus to renderer, by calling outside application SetFocusToWindow

import wx

import random
import time
import socket
import struct
import string
import os
import sys
# import threading
import xml.dom.minidom
import traceback

import wx.lib.newevent

# import osmpclient

import mvEvents

import pySockets
import pyThreadComms

import pyContextMenu
import pyChatWindow
import pyConfigDialog
import pylogin2

startuppath = os.getcwd();

def formatExceptionInfo(maxTBlevel=5):
         cla, exc, trbk = sys.exc_info()
         excName = cla.__name__
         try:
             excArgs = exc.__dict__["args"]
         except KeyError:
             excArgs = "<no args>"
         excTb = traceback.format_tb(trbk, maxTBlevel)
         return (excName, excArgs, excTb)

class PyGUIApp(wx.App):
   def __init__(self, bSomething, threadobject, options ):
      self.ThreadObject = threadobject
      self.options = options
      wx.App.__init__( self, bSomething )
      self.SetExitOnFrameDelete( False )

   def OnInit(self ):
      # self.ThreadObject = threadobject

      # self.ShowLoginDialog()
      # global options
      if self.options.Username == "":
         evt = mvEvents.OnShowLoginToAuthServer()
         wx.PostEvent( self, evt )
      else:
         self.ConnectToAuthServer( self.options.AuthServerIP, int(self.options.AuthServerPort), self.options.Username, self.options.Password )
            
      self.Bind( mvEvents.EVT_DIE, self.DieNow )
      
      self.Bind( mvEvents.EVT_ONUSERSELECTSAUTHSERVER, self.OnUserSelectsAuthServer )
      self.Bind( mvEvents.EVT_ONUSERSELECTSSIMSERVER, self.OnUserSelectsSimServer )

      self.Bind( mvEvents.EVT_SHOWLOGINTOAUTHSERVER, self.OnShowLoginToAuthServer )
      self.Bind( mvEvents.EVT_TEST, self.OnTest )
      
      # self.Bind( mvEvents.EVT_ONSERVERCOMMANDFORGUI, self.OnServerCommandForGUI )
      self.Bind( mvEvents.EVT_ONMAINTHREADCOMMANDFORGUI, self.OnMainThreadCommandForGUI )
      
      print "End of OnInit()"
      
      return True

   def OnTest( self, evt ):
      print "OnTest"
      
   def OnShowLoginToAuthServer( self, evt ):
      pylogin2.ShowLoginFrame( self )
      evt = mvEvents.OnTest()
      wx.PostEvent( self, evt )
      
   def OnUserSelectsAuthServer( self, evt ):
      print "parentmodule.logintoserver " + evt.name + " " + evt.server + " " + str(evt.port )
      self.ConnectToAuthServer( evt.server, int(evt.port), evt.name, evt.password  )
      
   def ConnectToAuthServer( self, authserverip, authserverport, username, password ):
      self.Username = username
      self.Password = password
      
      self.bAuthenticated = False
      
      # Try to connect to authserver
      print "trying to connect to " + str( authserverip ) + ":" + str(authserverport )
      self.AuthServerSocket = pySockets.ConnectToServerSocket( authserverip, authserverport )
      
      if self.AuthServerSocket is None:
         # Failed to connect, show login dialog to user again
         pylogin2.ShowLoginFrame( self, "Couldnt connect to authentication server" )
         return
         
      else:
         # Connected Ok!  Send our username and password; try to authenticate
         self.bAuthenticated, Message = self.AuthenticateToAuthServer()
         
      if not self.bAuthenticated:
         # not authenticated, so get credentials from user again
         pylogin2.ShowLoginFrame( self, Message )
         return
      
      if len( self.AvailableServers ) == 0:
         # no available sims, so ask user for another auth server
         pylogin2.ShowLoginFrame( self, "No available sims on this auth server. Maybe try another auth server?" )         
         return
      
      # auth ok, got simlist.  Show user list of available sims
      # global options
      if self.options.SimServerIP == "":
         pylogin2.ShowChooseSimServerFrame( self, self.AvailableServers )
      else:
         self.SimIP = self.options.SimServerIP
         self.SimPort = int( self.options.SimServerPort )
         self.ConnectToSimServer()
   
   def AuthenticateToAuthServer( self ):
      self.AuthServerSocket.send( '<login name="' + self.Username + '" password="' + self.Password + '"/>\015\012' )
      try:
         Received = self.AuthServerSocket.recv( 4096 )
         print "socketread: " + Received
         dom = xml.dom.minidom.parseString( Received )
         print str(dom)
         if( dom.documentElement.tagName == "loginaccept" ):
            print "Logged in to Auth Server ok"
            self.AvailableServers = []
            for simelement in dom.documentElement.getElementsByTagName("sims")[0].getElementsByTagName("sim"):
               AvailableServer = {}
               AvailableServer["Name"] = simelement.getAttribute("servername")
               AvailableServer["IPAddress"] = simelement.getAttribute("serverip")
               AvailableServer["Port"] = int( simelement.getAttribute("serverport") )
               self.AvailableServers.append( AvailableServer )
            print "Available servers: " + str( self.AvailableServers )
            return True, ""
         elif( dom.documentElement.tagName == "loginreject" ):
            print "Error logging into server"
            return False, "Error logging into server"
         else:
            print "AuthenticateToAuthServer() unrecognised message tag:" + dom.documentElement.tagName
            return False, "unrecognised message tag:" + dom.documentElement.tagName
      except:
         print "Error reading authentication server socket"
         return False, "Error reading server socket"
        
   def OnUserSelectsSimServer( self, evt ):
      print "OnUserChoosesSimServer", evt.Name, evt.IPAddress, evt.Port
      self.SimName = evt.Name
      self.SimIP = evt.IPAddress
      self.SimPort = evt.Port
      self.ConnectToSimServer()
      
   def ConnectToSimServer( self ):
      # Connect to sim server
      print "Connecting to server: " + self.SimIP + " " + str( self.SimPort )
      self.SimSocket = pySockets.ConnectToServerSocket( self.SimIP, self.SimPort )
      
      if self.SimSocket is None:
         # couldnt connect to sim server
         # pylogin2.ShowLoginFrame( self, "Couldnt connect to sim server." )         
         pylogin2.ShowChooseSimServerFrame( self, self.AvailableServers )
         return
      
      self.AuthenticateToSimServer()
      
   def AuthenticateToSimServer( self ):
      self.bAuthenticatedToSim = False
      self.SimSocket.send( '<login name="' + self.Username + '" password="' + self.Password + '"/>\015\012' )
      try:
         while not self.bAuthenticatedToSim:
            Received = self.SimSocket.recv( 4096 )
            print "socketread: " + Received
            dom = xml.dom.minidom.parseString( Received )
            print str(dom)
            if( dom.documentElement.tagName == "loginaccept" ):
               print "Logged in to Sim Server ok"
               self.bAuthenticatedToSim = True
               self.iReference = int( dom.documentElement.getAttribute("ireference") )
               
            elif( dom.documentElement.tagName == "loginreject" ):
               print "Error logging into server"
               evt = mvEvents.OnShowLoginToAuthServer(message="Error logging in to sim server")
               wx.PostEvent( self, evt )
               self.bAuthenticatedToSim = False
            else:
               print "AuthenticateToSimServer() unrecognised message tag:" + dom.documentElement.tagName
      except:
         print "Error reading sim server socket"
         evt = mvEvents.OnShowLoginToAuthServer(message="Connection to sim server failed")
         wx.PostEvent( self, evt )
         
      if self.bAuthenticatedToSim:
         print "Authenticated to sim ok"
         self.InitiateWorld()

   def InitiateWorld( self ):
      print "Initiating world..."
      # sys.__stdout__.write("options.SpawnNewWindows=" + options.SpawnNewWindows )
      # sys.exit(1)
      if self.options.SpawnNewWindows != "":
         pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
            Command = "SetSpawnNewWindows", Value = True ) )      
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "SetGUIApp", GUIApp = self ) )      
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "GiveSimSocketOwnership", SimSocket = self.SimSocket, SimIPAddress = self.SimIP, SimPort = self.SimPort ) )
      self.SimSocket = None
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "SetiReference", iReference = self.iReference ) )      
      evt = mvEvents.CommandEvent( Command = "InitiateWorld" )
      pyThreadComms.SendMessageToWorkerThread( evt )
      
      self.ChatWindow = pyChatWindow.DisplayChatWindow( self )
      
   def RequestPermissions( self, sType = None, iOwner = None, sServerIP = None, iServerPort = None ):
      if( type == "hyperlink" ):       
          sMessage = 'Would you like to hyperlink to server ' + sServerIP + ':' + str( iServerPort ) + '?'
          dlg = wx.MessageDialog(None, sMessage, 'Request Permissions', wx.YES_NO | wx.ICON_QUESTION )
          result = dlg.ShowModal()
          dlg.Destroy()
          
          if( result == wx.ID_YES ):
             evt = mvEvents.CommandEvent( Command = "PermissionsResponse",
                 sType = "hyperlink", bResult = True, iOwner = iOwner,
                 sServerIP = sServerIP, iServerPort = iServerPort )
          else:
             evt = mvEvents.CommandEvent( Command = "PermissionsResponse",
                 sType = "hyperlink", bResult = False, iOwner = iOwner,
                 sServerIP = sServerIP, iServerPort = iServerPort )
                 
          pyThreadComms.SendMessageToWorkerThread( evt )

   def DieNow( self, evt ):
      print "die event received"
      # self.socketreadthread.Stop()
      # self.chatwindow.Die()
      # self.Destroy()
      # sys.exit(0)
      
   def OnMainThreadCommandForGUI( self, evt ):
      Command = evt.Command
      
      if Command == "RequestPermissions":
         self.RequestPermissions( sType = evt.sType, iOwner = evt.iOwner, 
            sServerIP = evt.sServerIP, iServerPort = evt.iServerPort )
            
      elif Command == "ConnectToServer":
         self.SimSocket = None
         self.SimIP = evt.sServerIP
         self.SimPort = evt.iServerPort
         self.ConnectToSimServer()
         
      elif Command == "DisplayContextMenu":         
            print "contextmenu event"
            pyContextMenu.DisplayContextMenu( self, evt.iScreenX, evt.iScreenY, evt.Selected )
            
      elif Command == "ChatMessage":
         self.ChatWindow.DisplayMessage(
            sType = evt.sType, 
            sSourceName = evt.sSourceName,
            sMessage = evt.sMessage )
      
      else:
         print "ERROR: unknown command: " + Command
      
   def LinkObjects( self, screenx, screeny, selected ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "Link", iScreenX = screenx, iScreenY = screeny,
         SelectedReferenceList = selected ) )
      
   def LinkObjectsToAvatar( self, screenx, screeny, selected ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "LinkToAvatar", iScreenX = screenx, iScreenY = screeny,
         SelectedReferenceList = selected ) )
      
   def RemoveFromLinkedSet( self, screenx, screeny, selected ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "RemoveFromLinkedSet", iScreenX = screenx, iScreenY = screeny,
         SelectedReferenceList = selected ) )
         
   def UnlinkObjects( self, screenx, screeny, selected ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "UnlinkObjects", iScreenX = screenx, iScreenY = screeny,
         SelectedReferenceList = selected ) )
         
   def SetName( self, iReference, sNewObjectName ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "SetName", iReference = iReference, sNewName = sNewObjectName ) )
         
   def UnselectAll( self ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "UnselectAll" ) )
         
   def Export( self, sFilename ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "Export", sFilename = sFilename ) )
         
   def Import( self, sFilename ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "Import", sFilename = sFilename ) )
   
   def DeleteSelection( self ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "DeleteSelection" ) )
         
   def CreateObject( self, type, screenx, screeny, sFilename ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "CreateObject", sType = type, 
         iScreenX = screenx, iScreenY = screeny, sFilename = sFilename ) )
               
   def ApplyTextureOnSelection( self, sFilename ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "ApplyTexture", sFilename = sFilename ) )
         
   def SetColorOnSelection( self, color ):
      print "SetColorOnSelection"
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "SetColor", Color = color ) )
         
   def ReloadConfig( self ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "ReloadConfig" ) )
         
   def SetSkyboxTexture( self, path ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "SetSkyboxTexture", sFilename = path ) )
      
   def SetSkyboxTerrain( self, path ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "SetSkyboxTerrain", sFilename = path ) )
      
   def RemoveScriptFromSelection( self ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "RemoveScript" ) )
      
   def EditScript( self, iReference ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "EditScript", iReference = iReference ) )
     
   def EditTexture( self, iReference ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "EditTexture", iReference = iReference ) )
            
   def AssignScriptToSelection( self, path ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "AssignScript", sFilename = path ) )
      
   def SendToClient( self, message ):
      print "Sending to pymainthread: " + message
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "SendToCPP", Message = message ) )

   def SendToServer( self, message ):
      print "Sending to server: " + message
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "SendToServer", Message = message ) )

   def ShowConfigDialog( self ):
      global startuppath
      pyConfigDialog.DisplayConfigDialog( self, startuppath )
         
   def Quit( self ):
      pyThreadComms.SendMessageToWorkerThread( mvEvents.CommandEvent(
         Command = "Quit" ) )
      # self.Destroy()
      
