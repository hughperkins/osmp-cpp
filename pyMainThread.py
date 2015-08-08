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

import mvEvents

import wx

import pyThreadComms

import pyDevices

# SimSocket = None

import osmpclient
from osmpclient import *
import odephysicsengine

def PrintException():
   print "exception"
   traceback.print_exc(file=sys.stdout)
   while 1:
      time.sleep(1.0)

class CallbacksFromCPPClass( osmpclient.CallbackToPythonClass ):
   def __init__(self):
      self.SimSocket = None
      self.bSpawnNewWindows = False
      self.WorldInitiated = False
      self.MessageFragment = ""
      self.GUIApp = None
      self.pyDevices = pyDevices.pyDevices()
      osmpclient.CallbackToPythonClass.__init__( self )

   def SetSpawnNewWindows( self, Value ):
      self.bSpawnNewWindows = Value

   def SetGUIApp( self, guiapp ):
      print "Setting guiapp: " + str( guiapp )
      self.GUIApp = guiapp
      wx.PostEvent( guiapp, mvEvents.OnTest( blah = "blah" ) )   

   def SetSimSocket( self, socket, IPAddress, Port ):
      print "Setting socket: " + str( socket )
      self.SimSocket = socket
      self.SimSocket.setblocking(0)
      self.SimIPAddress = IPAddress
      self.SimPort = Port

   def SetWorld( self, World ):
      print "Got world: " + str( World )
      self.World = World

   def SetiMyReference( self, iReference ):
      print "Setting iReference: " + str( iReference )
      self.iMyReference = iReference
      osmpclient.SetAvatariReference( self.iMyReference )

   def SendToServer( self, messagestring ):
      if self.SimSocket != None:
         try:
            print "Received SendToServer [" + messagestring + "]"
            self.SimSocket.send( messagestring )
         except:
            PrintException();
            
   def SendClickEventToServer( self, x, y ):
       iReference = self.Selector.GetClickedPrimReference( x, y )
       if( iReference > 0 ):
          print  "click event"
          self.SendToServer( '<event type="clickdown" ireference="' + str( iReference ) + \
             '"/>\n' )
             
   def DisplayContextMenu( self, mousex, mousey ):
      try:
         print "Displycontextmenu " + str( mousex ) + " " + str( mousey )
         
         selected = {}
         selectedobjectreferences = self.Selector.GetSelectedObjectReferences()
         for iReference in selectedobjectreferences:
            selected[ iReference ] = {}
            thisobject = self.World.GetObjectByReference( iReference )
            selected[ iReference ]["objectname"] = thisobject.GetObjectName()
            selected[ iReference ]["deepobjecttype"] = thisobject.GetDeepObjectType()
         
         evt = mvEvents.OnMainThreadCommandForGUI( Command = "DisplayContextMenu",
            iScreenX = mousex, iScreenY = mousey, Selected = selected )
         wx.PostEvent( self.GUIApp, evt )
      except:
         PrintException();
      
   def ActivateChat( self ):
      try:
         print "ActivateChat() cmd"    
      except:
         PrintException();
   
   def Hide( self ):
      print "hide() cmd"    
   
   def Reappear( self ):
      print "reappearcmd"    

   def KeyDown( self, keycode ):
      print "keydown: " + str( keycode )
      self.pyDevices.KeyDown( keycode )
      
   def DisplayHelp( self ):
       print  "Displaying helpfile " + self.mvConfig.F1HelpFile + " ..."
       filename = self.mvConfig.F1HelpFile
       
       if sys.platform == "win32":
          os.system( 'launchhelp.bat "' + filename + '"' )
       else:
          os.system( 'lynx "' + filename + '"' )
      
   def KeyUp( self, keycode ):
      pass
      self.pyDevices.KeyUp( keycode )
   
   def MouseMove( self, x, y ):
      self.pyDevices.MouseMove( x, y )
   
   def MouseDown( self, bLeftDown, bRightDown, x, y ):
      self.pyDevices.MouseDown( bLeftDown, bRightDown, x, y )
   
   def MouseUp( self, bLeftUp, bRightUp, x, y ):
      self.pyDevices.MouseUp(  bLeftUp, bRightUp, x, y )

   def SendMessageToGUI( self, messagestring ):
      try:
         print "received message for us: " + messagestring
         raise "Error: received message in SendMessageToGUI.  This should not happen any more"
         #evt = mvEvents.OnServerCommandForGUI( Message = messagestring )
         #wx.PostEvent( self.GUIApp, evt )
      except:
         PrintException()
      
   def GetNextMessagesFromSocket( self, socket ):
      try:
         ReceiveFragment = socket.recv( 4096 )
      except:
         return []
      
      self.MessageFragment = self.MessageFragment + ReceiveFragment
      messages = string.split( self.MessageFragment, "\n" )      
      receivedmessages = messages[0:len(messages) - 1]
      self.MessageFragment = messages[ len( messages ) - 1 ]      
      return receivedmessages

   def ResetAll( self ):
      self.World.Clear() 
      self.TextureManager.Clear()
      self.TerrainCache.Clear()
      self.MeshInfoCache.Clear()
      self.EditorMgr.Clear()
      self.Selector.Clear()

      self.SendToServer( "<requestworldstate />\n" )

   def CreateNewObject( self, sType, sFilename, x, y, z ):
      print  "creating object at coords " + str( x ) + " " + str( y ) + " " + str( z )
      
      if sType == "TERRAIN":
         if sFilename != "":
            self.ClientTerrainFunctions.CreateTerrain( sFilename.encode('UTF-8'), x, y, z )
           
      elif sType == "MD2MESH":
         self.MeshFileMgmt.CreateFile( sFilename.encode('UTF-8'), x, y, z )

      else:
         message = '<objectcreate type="' + sType + '" iparentreference="0">' + \
            '<geometry><pos x="' + str(x) + '" y="' + str(y) + '" z="' + str(z) + '"/></geometry>' + \
            '</objectcreate>\n'
         self.SendToServer( message )

   def DeleteCurrentSelection( self ):
      print "Delete current selection"
      selectedobjectreferences = self.Selector.GetSelectedObjectReferences()
      for iReference in selectedobjectreferences:
         print "deleting object reference " + str( iReference )
         message = '<objectdelete ireference="' + str( iReference ) + '"/>\n'
         self.SendToServer( message )

   def ProcessMessagesFromServer( self ):
      message = None
      #print "processing sockets messages:"
      messages = self.GetNextMessagesFromSocket( self.SimSocket )
      for message in messages:
          #print "received sockets message [" + message + "]"
          
          dom = xml.dom.minidom.parseString( message )
          Command = dom.documentElement.tagName
          
          if( Command == "comm" ):
             print "Posting to GUI" + message
             wx.PostEvent( self.GUIApp, mvEvents.OnMainThreadCommandForGUI( 
                Command = "ChatMessage", 
                sType = dom.documentElement.getAttribute("type"),
                sSourceName = dom.documentElement.getAttribute("objectname"),
                sMessage =  dom.documentElement.getAttribute("message") ) )
          
          elif( Command == "objectrefreshdata" ):
             NewObject = self.World.StoreObjectXMLString( message )
             self.CollisionAndPhysicsEngine.ObjectCreate( NewObject )
             
          elif( Command == "objectcreate" ):
             NewObject = self.World.StoreObjectXMLString( message )
             self.CollisionAndPhysicsEngine.ObjectCreate( NewObject )
             
          elif( Command == "objectupdate" ):
             TargetObject = self.World.UpdateObjectXMLString( message )
             self.CollisionAndPhysicsEngine.ObjectModify( TargetObject )
             
          elif( Command == "objectdelete" ):
             iReference = int( dom.documentElement.getAttribute("ireference") )
             if iReference != 0:
                self.World.DeleteObjectByObjectReference( iReference )
                self.Animator.RemoveFromMovingObjects( iReference )
                self.Selector.RemoveFromSelectedObjects( iReference )
                self.CollisionAndPhysicsEngine.ObjectDestroy( iReference )
             
          elif( Command == "reset" ):
             self.ResetAll()
             
          elif( Command == "skyboxupdate" ):
             print " ** GOT SKYBOX UPDATE FROM CLIENT"
             newskyboxchecksum = dom.documentElement.getAttribute( "stexturereference" )
             self.World.SetSkyboxChecksum( newskyboxchecksum.encode('utf-8') )
             print " ** NEW  CLIENT SKYBOX " + str( self.World.GetSkyboxChecksum() )

          elif( Command == "texture" ):
             print " ** CALL RegTextFromXML val: texture in ProcessXML"
             self.RendererTexturing.RegisterTextureFromXMLString( message )
      
          elif( Command == "terrain" ):
             print " ** Call RegisterTerrainFromXML()"
             self.ClientTerrainFunctions.RegisterTerrainFromXMLString( message )
               
          elif( Command == "meshfile" ):
             self.MeshFileMgmt.RegisterFileFromXMLString( message );
             
          elif( Command == "objectmove" ):
             iReference = int( dom.documentElement.getAttribute("ireference") )
             if iReference != 0:
                if not self.Selector.IsSelected( iReference ) and iReference != self.iMyReference:
                   self.Animator.MoveObjectFromXMLString( message );
                self.World.UpdateObjectXMLString( message )
          
          elif( Command == "inforesponse" ):
             if dom.documentElement.getAttribute("clienteditreference") != None:
                self.EditorMgr.ProcessInfoResponseFromXMLString( message )
                
          elif( Command == "hyperlinkagent" ):
             sType = dom.documentElement.getAttribute("type")
             iOwner = int( dom.documentElement.getAttribute("iowner") )
             sServerIP = dom.documentElement.getAttribute("serverip")
             iServerPort = int( dom.documentElement.getAttribute("serverport") )
             
             evt = mvEvents.OnCommandForGUI( Command = "RequestPermissions",
                sType = sType, iOwner = iScriptOwner,
                sServerIP = sServerIP, iServerPort = iServerPort )             
             wx.PostEvent( self.GUIApp, evt )
          
          elif( Command == "capture" ):
             what = dom.documentElement.getAttribute("what")
             iReference = int( dom.documentElement.getAttribute("ireference") )
             
             if what == "wholekeyboard":
                osmpclient.mvKeyboardAndMouse_StartCapture( iReference )
                
             elif what == "wholekeyboardoff":
                osmpclient.mvKeyboardAndMouse_StopCapture( iReference )
             
          else:
             print "WARNING: unhandled server message: " + message
      
   def ProcessMessagesFromOtherThreads( self ):
      message = None
      message = pyThreadComms.GetNextMessageForWorkerThread()
      #print "checking for thread messages:"
      while not message is None:
         print "Got message: " + str( message )
         
         if message.Command == "SendToCPP":
            raise "SendToCPP called, in pyMainThread " + message.Message
            # osmpclient.HandleGUICommand( message.Message.encode('utf-8') )
            
         elif message.Command == "SendToServer":
            print "Sending to server, " + message.Message 
            self.SendToServer( message.Message )
            
         elif message.Command == "Link":
            print "Link " + str( message.SelectedReferenceList )
            if self.Selector.GetNumSelected() > 0:
               self.ClientLinking.LinkSelectedObjects()
               
         elif message.Command == "LinkToAvatar":
            if self.Selector.GetNumSelected() > 0:
                self.ClientLinking.LinkSelectedObjectsToAvatar()
                
         elif message.Command == "RemoveFromLinkedSet":
            self.ClientLinking.RemoveFromLinkedSetRefList( message.SelectedReferenceList )

         elif message.Command == "UnlinkObjects":
            self.ClientLinking.UnlinkSelectedObject()
            
         elif message.Command == "SetName":
            iReference = message.iReference
            sNewName = message.sNewName
            self.SendToServer( '<objectupdate ireference="' + str( iReference ) + '" objectname="' + sNewName + '">\n' )
            
         elif message.Command == "UnselectAll":
            self.Selector.Clear()

         elif message.Command == "Export":
            sFilename = message.sFilename
            print sFilename
            selectedobjectreferences = self.Selector.GetSelectedObjectReferences()
            for iReference in selectedobjectreferences:
               TargetObject = self.World.GetObjectByReference( iReference )
               self.ObjectImportExport.ExportObject( sFilename.encode('UTF-8'), TargetObject )

         elif message.Command == "Import":
            sFilename = message.sFilename
            print sFilename
            self.ObjectImportExport.ImportObject( sFilename.encode('UTF-8') )

         elif message.Command == "SetColor":
            try:
               print "SetColor"
               NewColor = osmpclient.COLOR( message.Color["r"], message.Color["g"], message.Color["b"] )
               print NewColor
               selectedobjectreferences = self.Selector.GetSelectedObjectReferences()
               for iReference in selectedobjectreferences:
                  print iReference
                  TargetObject = self.World.GetObjectByReference( iReference )
                  print str( TargetObject )
                  if not TargetObject is None:
                     print TargetObject.GetObjectType()
                     print "PRIM" == str( TargetObject.GetObjectType() )
                     if TargetObject.GetObjectType() == "PRIM":
                        message = '<objectupdate ireference="' + str( iReference ) + \
                           '"><faces><face num="0"><color ' + NewColor.ToXMLAttributes() + \
                           '/></face></faces></objectupdate>\n'
                        print message
                        self.SendToServer( message )
            except:
               PrintException()

         elif message.Command == "ApplyTexture":
            self.RendererTexturing.ApplyTextureFromFilename( message.sFilename.encode('UTF-8') )
            
         elif message.Command == "ReloadConfig":
            self.mvConfig.ReadConfig()

         elif message.Command == "CreateObject":
            print "create command received from gui"
            Avatar = self.World.GetObjectByReference( self.iMyReference )
            if Avatar is None:
               continue
            
            iWindowX = message.iScreenX - osmpclient.GetWindowXPos()
            iWindowY = message.iScreenY - osmpclient.GetWindowYPos()
            
            HitTarget = self.Selector.GetClickedHitTarget( iWindowX, iWindowY )
            
            sFilename = message.sFilename
  
            OurPos = Avatar.pos
            OurRot = Avatar.rot
            OurLookVector = self.mvGraphics.GetMouseVector( OurPos, OurRot, iWindowX, iWindowY )
            osmpclient.VectorNormalize( OurLookVector )
  
            if not HitTarget is None:
               if HitTarget.TargetType == osmpclient.HITTARGETTYPE_OBJECT:
                  print "collided with object"
                  MouseHitObject = self.World.GetObjectByReference( HitTarget.iForeignReference )
                  bCollided, RezPos = self.CollisionAndPhysicsEngine.CollideSphereAsRayWithObject( OurPos, OurLookVector, 0.2, MouseHitObject )
                  if bCollided:
                     self.CreateNewObject( message.sType, sFilename, RezPos.x, RezPos.y, RezPos.z )
                  else:
                     PosOnRay = OurPos + OurLookVector * 3.0
                     self.CreateNewObject( message.sType, sFilename, PosOnRay.x, PosOnRay.y, PosOnRay.z )
                     
            else:
               print str( OurPos )
               print str( OurLookVector )
               PosOnRay = OurPos + OurLookVector * 3.0
               self.CreateNewObject( message.sType, sFilename, PosOnRay.x, PosOnRay.y, PosOnRay.z )

         elif message.Command == "DeleteSelection":
            self.DeleteCurrentSelection()
               
         elif message.Command == "SetSkyboxTexture":
            self.RendererTexturing.ApplySkyboxTextureFromFilename( message.sFilename.encode('UTF-8') )

         elif message.Command == "SetSkyboxTerrain":
            self.RendererTexturing.ApplySkyboxTerrainFromFilename( message.sFilename.encode('UTF-8') )

         elif message.Command == "AssignScript":
            print  "got assign script command " + message.sFilename
            self.ScriptMgmt.AssignScriptFromFilename( message.sFilename.encode('UTF-8') )
         
         elif message.Command == "RemoveScript":
            self.ScriptMgmt.RemoveScriptFromSelectedObjects()
         
         elif message.Command == "EditTexture":
            self.EditorMgr.EditTexture( message.iReference )
         
         elif message.Command == "EditScript":
            self.EditorMgr.EditScriptInitiate( message.iReference )
 
         elif message.Command == "PermissionsResponse":
            if message.Result:
               if message.sType == "hyperlink":
                  self.SimSocket.close()
                  self.SimSocket = None
                  self.ResetAll()
                  evt = mvEvents.OnMainThreadCommandForGUI( Command = "ConnectToServer",
                     sServerIP = message.sServerIP, iServerPort = message.iServerPort )
                  wx.PostEvent( self.GUIApp, evt )
                  
         elif message.Command == "Quit":
            print "quit received"
            os._exit(0)
                  
         else:
            raise "ERROR: unknown message command: " + message.Command
                        
         message = pyThreadComms.GetNextMessageForWorkerThread()
      
   # Note that this function will not return until the renderer shuts down,
   # but the renderer will call back to us regularly, eg via DoEvents (once a frame)
   def InitiateWorld( self ):
      try:
         self.mvConfig = osmpclient.GetmvConfig()
         
         print "getting world..."
         self.World = osmpclient.GetWorld()
         print "getting animator..."
         self.Animator = osmpclient.GetAnimator()
         print "getting selector..."
         self.Selector = osmpclient.GetSelector()
         print "got all objects, spawning clientfileagent..."
         self.PlayerMovement = osmpclient.GetPlayerMovement()
         self.Editing3D = osmpclient.GetEditing3D()
         
         self.TextureManager = osmpclient.GetTextureInfoCache()
         self.TerrainCache = osmpclient.GetTerrainCache()
         self.MeshInfoCache = osmpclient.GetMeshInfoCache()
         self.EditorMgr = osmpclient.GetEditorMgr()
         self.ClientLinking = osmpclient.GetClientLinking()
         self.ObjectImportExport = osmpclient.GetObjectImportExport()
         
         self.RendererTexturing = osmpclient.GetRendererTexturing()
         self.ClientTerrainFunctions = osmpclient.GetClientTerrainFunctions()
         self.MeshFileMgmt = osmpclient.GetMeshFileMgmt();
         self.ScriptMgmt = osmpclient.GetScriptMgmt();
         
         self.Camera = osmpclient.GetCamera();
         
         osmpclient.SetSimIPAddress( self.SimIPAddress.encode('utf-8'), self.SimPort )
         self.SpawnClientFileAgent()
         
         self.CollisionAndPhysicsEngine = osmpclient.GetCollisionAndPhysicsEngine()
         self.mvGraphics = osmpclient.GetmvGraphics()
            
         self.pyDevices.SetPlayerMovement( self.PlayerMovement )
         self.pyDevices.SetSendToServer( self.SendToServer )
         self.pyDevices.SetEditing3D( self.Editing3D )
         self.pyDevices.SetmvConfig( self.mvConfig )
         self.pyDevices.SetCallingObject( self )
            
         print "starting renderer..."
            
         osmpclient.MetaverseclientStartRenderer()
      except:
         PrintException()
      
   def DoEvents( self ):
      try:
         if not self.WorldInitiated:
            self.SendToServer("<requestworldstate />\015\012")
            self.WorldInitiated = True
      
         # print ">>>Doevents"
         
         self.ProcessMessagesFromOtherThreads()                     
         self.ProcessMessagesFromServer()
         
         #print "<<<Doevents"
      except:
         PrintException()

   # starts clientfileagent
   def SpawnClientFileAgent( self ):         
      osmpclient.SpawnFileAgent()
      
     # if sys.platform == "win32":
     #    if not self.bSpawnNewWindows:
     #       os.system( "msvc\\clientfileagent.exe")
     #       time.sleep(10)
     #    else:
     #       os.system( "spawnclientfileagent.bat")
     # else:
     #    os.system( "linux/clientfileagent")
            
def StartCPPClient():
   CallbacksFromCPP = CallbacksFromCPPClass()
   osmpclient.InitMetaverseClient( CallbacksFromCPP )

   bClientFinished = False
   
   while not bClientFinished:
      nextmessage = pyThreadComms.GetNextMessageForWorkerThread()
      
      if not nextmessage is None:
         print "Got message: " + str( nextmessage )
         sys.__stdout__.write("is this console?" )
         print "Command: " + nextmessage.Command
         
         if nextmessage.Command == "GiveSimSocketOwnership":
            CallbacksFromCPP.SetSimSocket( nextmessage.SimSocket, nextmessage.SimIPAddress.encode('utf-8'), nextmessage.SimPort )
            
         elif nextmessage.Command == "SetiReference":
            CallbacksFromCPP.SetiMyReference( nextmessage.iReference )
            
         elif nextmessage.Command == "SetGUIApp":
            CallbacksFromCPP.SetGUIApp( nextmessage.GUIApp )
            
         elif nextmessage.Command == "SetSpawnNewWindows":
            CallbacksFromCPP.SetSpawnNewWindows( nextmessage.Value )
                        
         elif nextmessage.Command == "InitiateWorld":
            CallbacksFromCPP.InitiateWorld()
            bClientFinished = True
            
      time.sleep(0.01)
      
   print "Exited StartCPPClient()"
