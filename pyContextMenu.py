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

# This module is responsible for showing a context menu
# main public method is DisplayContextMenu( parentmodule, dom ), which takes in 
# a PyGUIApp object and a parsed XML incoming IPC, containing screenx, screeny attributes
# and a <selection/> subtree with the ireference of each currently selected object
# screenx and screeny are used to position the contextmenu on the screen

import wx

import random
import time
import struct
import string

import os
import sys

import xml.dom.minidom
import traceback

def GetObjectNameFromUser( sOldName ):
   dlg = wx.TextEntryDialog(
      None, 'Please enter new object name:',
      'Set Object Name', sOldName )

   ObjectName = sOldName
    
   if dlg.ShowModal() == wx.ID_OK:
      ObjectName = dlg.GetValue()

   dlg.Destroy()
   
   return ObjectName

def DoTextureThing():
   dlg = wx.FileDialog(
      None, message="Choose a file", defaultDir="%userprofile%\my documents\OSMP Inventory\Textures", 
      defaultFile="", wildcard="*.tga;*.pcx", style=wx.OPEN | wx.CHANGE_DIR
      )
        
   path = ""
   # Show the dialog and retrieve the user response. If it is the OK response, 
   # process the data.
   if dlg.ShowModal() == wx.ID_OK:
      # This returns a Python list of files that were selected.
      paths = dlg.GetPaths()

      for path in paths:
         pass

   dlg.Destroy()
   return path

def GetScriptPath():
   dlg = wx.FileDialog(
      None, message="Choose a file", defaultDir="%userprofile%\my documents\OSMP Inventory\Scripts", 
      defaultFile="", wildcard="*.lua", style=wx.OPEN | wx.CHANGE_DIR
      )
        
   path = ""
   # Show the dialog and retrieve the user response. If it is the OK response, 
   # process the data.
   if dlg.ShowModal() == wx.ID_OK:
      # This returns a Python list of files that were selected.
      paths = dlg.GetPaths()

      for path in paths:
         pass

   dlg.Destroy()
   return path

def GetTerrainPath():
   dlg = wx.FileDialog(
      None, message="Choose a 128x128 8-bit .raw terrain file:", defaultDir="%userprofile%\my documents\OSMP Inventory\Terrains", 
      defaultFile="", wildcard="*.raw", style=wx.OPEN | wx.CHANGE_DIR
      )
        
   path = ""
   # Show the dialog and retrieve the user response. If it is the OK response, 
   # process the data.
   if dlg.ShowModal() == wx.ID_OK:
      # This returns a Python list of files that were selected.
      paths = dlg.GetPaths()

      for path in paths:
         pass

   dlg.Destroy()
   return path

def GetMeshPath():
   dlg = wx.FileDialog(
      None, message="Choose a mesh file:", defaultDir="%userprofile%\my documents\OSMP Inventory\Meshes", 
      defaultFile="", wildcard="*.md2", style=wx.OPEN | wx.CHANGE_DIR
      )
        
   path = ""
   # Show the dialog and retrieve the user response. If it is the OK response, 
   # process the data.
   if dlg.ShowModal() == wx.ID_OK:
      # This returns a Python list of files that were selected.
      paths = dlg.GetPaths()

      for path in paths:
         pass

   dlg.Destroy()
   return path

def GetExportFilePath():
   dlg = wx.FileDialog(
      None, message="Please specify a filename", defaultDir="%userprofile%\my documents\OSMP Inventory\Objects", 
      defaultFile="", wildcard="*.xml", style=wx.SAVE | wx.CHANGE_DIR
      )
        
   path = ""
   if dlg.ShowModal() == wx.ID_OK:
      paths = dlg.GetPaths()

      for path in paths:
         pass

   dlg.Destroy()
   return path

def GetImportFilePath():
   dlg = wx.FileDialog(
      None, message="Please specify a filename", defaultDir="%userprofile%\my documents\OSMP Inventory\Objects", 
      defaultFile="", wildcard="*.xml", style=wx.OPEN | wx.CHANGE_DIR
      )
        
   path = ""
   if dlg.ShowModal() == wx.ID_OK:
      paths = dlg.GetPaths()
      path = paths[0]

   dlg.Destroy()
   
   return path

def DoColorDialog(parent):
    dlg = wx.ColourDialog(parent)
    dlg.GetColourData().SetChooseFull(True)

    color = ""
    if dlg.ShowModal() == wx.ID_OK:
        data = dlg.GetColourData()
        color = data.GetColour().Get()

    dlg.Destroy()
    return color

#def SelectedToXML( selected ):
 #  xmltext = ""
  # for iReference in selected.keys():
   #   xmltext += "<selected ireference=\"" + str( iReference ) + "\" objectname=\"" + selected[ iReference ].get('objectname') + "\"/>"
  # # print xmltext
  # return xmltext

class FrameLinking(wx.Frame):
    def __init__(self, parent, x, y, selected, parentmodule ):
        wx.Frame.__init__(self, parent, -1, "Grouping:",
                          pos=(x, y), size=(130, 200), style=wx.STAY_ON_TOP | wx.FRAME_TOOL_WINDOW  )

        self.parentmodule = parentmodule
        
        self.screenx = x
        self.screeny = y
        self.selected = selected

        panel = wx.Panel(self, size=(130,200))

        self.Bind(wx.EVT_KILL_FOCUS, self.KillFocus )

        sizer = wx.BoxSizer(wx.VERTICAL)

        if( len( selected ) > 1 ):
           button = wx.Button(panel, -1, "Link")
           self.Bind(wx.EVT_BUTTON, self.Link, button)
           sizer.Add(button, 1, wx.EXPAND)
           
        if( len( selected ) >= 1 ):
           button = wx.Button(panel, -1, "Link to avatar")
           self.Bind(wx.EVT_BUTTON, self.LinkToAvatar, button)
           sizer.Add(button, 1, wx.EXPAND)
           
        if( len( selected ) >= 1 ):
           button = wx.Button(panel, -1, "Remove from linked set")
           self.Bind(wx.EVT_BUTTON, self.RemoveFromLinkedSet, button)
           sizer.Add(button, 1, wx.EXPAND)
                           
        if( len( selected ) == 1 ):
           button = wx.Button(panel, -1, "Unlink")
           self.Bind(wx.EVT_BUTTON, self.Unlink, button)
           sizer.Add(button, 1, wx.EXPAND)
                           
        cancelbutton = wx.Button(panel, -1, "Cancel")
        self.Bind(wx.EVT_BUTTON, self.Cancel, cancelbutton)        
        sizer.Add(cancelbutton, 1, wx.EXPAND)        
                
        panel.SetSizer(sizer)
        panel.Layout()
               
    def Link( self, evt ):
       self.parentmodule.LinkObjects( self.screenx, self.screeny, self.selected )
       self.Destroy()
       
    def LinkToAvatar( self, evt ):
       self.parentmodule.LinkObjectsToAvatar( self.screenx, self.screeny, self.selected )
       self.Destroy()
       
    def RemoveFromLinkedSet( self, evt ):
       ireflist = []
       for iref in self.selected.keys():
          ireflist.append( iref )
       self.parentmodule.RemoveFromLinkedSet( self.screenx, self.screeny, ireflist )
       self.Destroy()
       
    def Unlink( self, evt ):
       self.parentmodule.UnlinkObjects( self.screenx, self.screeny, self.selected )
       self.Destroy()
       
    def Cancel( self, evt ):
       self.Destroy()
       
    def KillFocus( self, evt ):
       print "killfocus"
       self.Destroy()
       
class FrameScripting(wx.Frame):
    def __init__(self, parent, x, y, selected, parentmodule ):
        wx.Frame.__init__(self, parent, -1, "Scripting:",
                          pos=(x, y), size=(130, 200), style=wx.STAY_ON_TOP | wx.FRAME_TOOL_WINDOW  )

        self.parentmodule = parentmodule
        
        self.screenx = x
        self.screeny = y
        self.selected = selected

        panel = wx.Panel(self, size=(130,200))

        self.Bind(wx.EVT_KILL_FOCUS, self.KillFocus )

        sizer = wx.BoxSizer(wx.VERTICAL)

        if( len( selected ) > 0 ):
           button = wx.Button(panel, -1, "Assign Script...")
           self.Bind(wx.EVT_BUTTON, self.AssignScript, button)
           sizer.Add(button, 1, wx.EXPAND)
           
        if( len( selected ) == 1 ):
           button = wx.Button(panel, -1, "Edit Script...")
           self.Bind(wx.EVT_BUTTON, self.EditScript, button)
           sizer.Add(button, 1, wx.EXPAND)
                           
        if( len( selected ) == 1 ):
           button = wx.Button(panel, -1, "Remove Script")
           self.Bind(wx.EVT_BUTTON, self.RemoveScript, button)
           sizer.Add(button, 1, wx.EXPAND)
                           
        cancelbutton = wx.Button(panel, -1, "Cancel")
        self.Bind(wx.EVT_BUTTON, self.Cancel, cancelbutton)        
        sizer.Add(cancelbutton, 1, wx.EXPAND)        
                
        panel.SetSizer(sizer)
        panel.Layout()         

    def AssignScript( self, evt ):
       path = GetScriptPath()
       self.Destroy()
       self.parentmodule.AssignScriptToSelection( path )
       
    def EditScript( self, evt ):
       self.Destroy()
       iReference = self.selected.keys()[0]
       self.parentmodule.EditScript( iReference )
      
    def RemoveScript( self, evt ):
       self.Destroy()
       self.parentmodule.RemoveScriptFromSelection()
      
    def Cancel( self, evt ):
       self.Destroy()
       
    def KillFocus( self, evt ):
       print "killfocus"
       self.Destroy()
       
class FrameCreatemenu(wx.Frame):
    def __init__(self, parent, x, y, selected, parentmodule ):
        wx.Frame.__init__(self, parent, -1, "Create:",
                          pos=(x, y), size=(100, 200), style=wx.STAY_ON_TOP | wx.FRAME_TOOL_WINDOW  )

        self.parentmodule = parentmodule
        
        self.screenx = x
        self.screeny = y
        self.selected = selected

        # print "__init__"
        # Now create the Panel to put the other controls on.
        panel = wx.Panel(self, size=(100,200))

        self.Bind(wx.EVT_KILL_FOCUS, self.KillFocus )

        sizer = wx.BoxSizer(wx.VERTICAL)

        # print len( selected )

        #if( len( selected ) == 0 ):
        button = wx.Button(panel, -1, "Create Cube")
        self.Bind(wx.EVT_BUTTON, self.CreateCube, button)
        sizer.Add(button, 1, wx.EXPAND)
           
        button = wx.Button(panel, -1, "Create Sphere")
        self.Bind(wx.EVT_BUTTON, self.CreateSphere, button)
        sizer.Add(button, 1, wx.EXPAND)
           
        button = wx.Button(panel, -1, "Create Cylinder")
        self.Bind(wx.EVT_BUTTON, self.CreateCylinder, button)
        sizer.Add(button, 1, wx.EXPAND)
           
        button = wx.Button(panel, -1, "Create Cone")
        self.Bind(wx.EVT_BUTTON, self.CreateCone, button)
        sizer.Add(button, 1, wx.EXPAND)
           
        button = wx.Button(panel, -1, "Create Mesh...")
        self.Bind(wx.EVT_BUTTON, self.CreateMesh, button)
        sizer.Add(button, 1, wx.EXPAND)          

        button = wx.Button(panel, -1, "Create Terrain...")
        self.Bind(wx.EVT_BUTTON, self.CreateTerrain, button)
        sizer.Add(button, 1, wx.EXPAND)          

        cancelbutton = wx.Button(panel, -1, "Cancel")
        self.Bind(wx.EVT_BUTTON, self.Cancel, cancelbutton)        
        sizer.Add(cancelbutton, 1, wx.EXPAND)        
                
        panel.SetSizer(sizer)
        panel.Layout()
        
    def CreateCube( self, evt ):
       self.Destroy()
       self.parentmodule.CreateObject( "CUBE", self.screenx, self.screeny, None )
       
    def CreateSphere( self, evt ):
       self.Destroy()
       self.parentmodule.CreateObject( "SPHERE", self.screenx, self.screeny, None )
       
    def CreateCylinder( self, evt ):
       self.Destroy()
       self.parentmodule.CreateObject( "CYLINDER", self.screenx, self.screeny, None )
       
    def CreateCone( self, evt ):
       self.Destroy()
       self.parentmodule.CreateObject( "CONE", self.screenx, self.screeny, None )
       
    def CreateMesh( self, evt ):
       path = GetMeshPath()
       self.Destroy()      
       self.parentmodule.CreateObject( "MD2MESH", self.screenx, self.screeny, path )
       
    def CreateTerrain( self, evt ):
       path = GetTerrainPath()
       self.Destroy()      
       self.parentmodule.CreateObject( "TERRAIN", self.screenx, self.screeny, path )
       
    def Cancel( self, evt ):
       self.Destroy()
       
    def KillFocus( self, evt ):
       print "killfocus"
       self.Destroy()
       
       
class FrameSimAdminmenu(wx.Frame):
    def __init__(self, parent, x, y, selected, parentmodule ):
        wx.Frame.__init__(self, parent, -1, "SimAdmin:",
                          pos=(x, y), size=(100, 200), style=wx.STAY_ON_TOP | wx.FRAME_TOOL_WINDOW  )

        self.parentmodule = parentmodule
        
        self.screenx = x
        self.screeny = y
        self.selected = selected

        # print "__init__"
        # Now create the Panel to put the other controls on.
        panel = wx.Panel(self, size=(100,200))

        self.Bind(wx.EVT_KILL_FOCUS, self.KillFocus )

        sizer = wx.BoxSizer(wx.VERTICAL)
        
        button = wx.Button(panel, -1, "Set Skybox...")
        self.Bind(wx.EVT_BUTTON, self.SetSkybox, button)
        sizer.Add(button, 1, wx.EXPAND)
        
        cancelbutton = wx.Button(panel, -1, "Cancel")
        self.Bind(wx.EVT_BUTTON, self.Cancel, cancelbutton)        
        sizer.Add(cancelbutton, 1, wx.EXPAND)        
                
        panel.SetSizer(sizer)
        panel.Layout()
    
    def SetSkybox( self, evt ):
       path = DoTextureThing()       
       self.Destroy()
       self.parentmodule.SetSkyboxTexture( path )
       
    def Cancel( self, evt ):
       self.Destroy()
       
    def KillFocus( self, evt ):
       print "killfocus"
       self.Destroy()
       
class FrameContextMenu(wx.Frame):
    def __init__(self, parent, title, x, y, selected, parentmodule ):
        wx.Frame.__init__(self, parent, -1, title,
                          pos=(x, y), size=(100, 300), style=wx.STAY_ON_TOP | wx.FRAME_TOOL_WINDOW  )

        self.parentmodule = parentmodule
        
        self.screenx = x
        self.screeny = y
        self.selected = selected        

        # print "__init__"
        # Now create the Panel to put the other controls on.
        panel = wx.Panel(self, size=(100,300))

        self.Bind(wx.EVT_KILL_FOCUS, self.KillFocus )
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)

        sizer = wx.BoxSizer(wx.VERTICAL)

        # print len( selected )
        isTerrain = False
        for i,v in selected.iteritems():
           if (v['deepobjecttype'] == "TERRAIN"):
              isTerrain = True

        if( len( selected ) == 1 ):
           text = wx.StaticText(panel, -1, "Object: " + selected.itervalues().next()['objectname'] )
           sizer.Add(text, 1, wx.EXPAND)

           button = wx.Button(panel, -1, "Change Name...")
           self.Bind(wx.EVT_BUTTON, self.ChangeName, button)
           sizer.Add(button, 1, wx.EXPAND)
           
        if( len( selected ) > 0 ):
           exportbutton = wx.Button(panel, -1, "Export")
           self.Bind(wx.EVT_BUTTON, self.Export, exportbutton)
           sizer.Add(exportbutton, 1, wx.EXPAND)
           
        if True:
           #if( len( selected ) > 0 ):
           importbutton = wx.Button(panel, -1, "Import")
           self.Bind(wx.EVT_BUTTON, self.Import, importbutton)
           sizer.Add(importbutton, 1, wx.EXPAND)
           
        button = wx.Button(panel, -1, "Create...")
        self.Bind(wx.EVT_BUTTON, self.Create, button)
        sizer.Add(button, 1, wx.EXPAND)
           
        if( len( selected ) > 0 ):
           button = wx.Button(panel, -1, "Scripting...")
           self.Bind(wx.EVT_BUTTON, self.Scripting, button)
           sizer.Add(button, 1, wx.EXPAND)
           
        if( len( selected ) > 0 ):
           texturebutton = wx.Button(panel, -1, "Assign Texture...")
           self.Bind(wx.EVT_BUTTON, self.Texture, texturebutton)
           sizer.Add(texturebutton, 1, wx.EXPAND)
           
        if( isTerrain ):
           setskyboxbutton = wx.Button(panel, -1, "Set Skybox...")
           self.Bind(wx.EVT_BUTTON, self.SetTSkybox, setskyboxbutton)
           sizer.Add(setskyboxbutton, 1, wx.EXPAND)
           
        if( len( selected ) == 1 ):
           edittexturebutton = wx.Button(panel, -1, "Edit Texture...")
           self.Bind(wx.EVT_BUTTON, self.EditTexture, edittexturebutton)
           sizer.Add(edittexturebutton, 1, wx.EXPAND)
           
        if( len( selected ) > 0 ):
           colorbutton = wx.Button(panel, -1, "Color...")
           self.Bind(wx.EVT_BUTTON, self.Color, colorbutton)
           sizer.Add(colorbutton, 1, wx.EXPAND)
           
        if( len( selected ) >= 1 ):
           linkbutton = wx.Button(panel, -1, "Linking...")
           self.Bind(wx.EVT_BUTTON, self.Linking, linkbutton)
           sizer.Add(linkbutton, 1, wx.EXPAND)
           
        if( len( selected ) > 0 ):
           unselectallbutton = wx.Button(panel, -1, "Unselect All")
           self.Bind(wx.EVT_BUTTON, self.UnselectAll, unselectallbutton)
           sizer.Add(unselectallbutton, 1, wx.EXPAND)
           
        if( len( selected ) > 0 ):
           deletebutton = wx.Button(panel, -1, "Delete")
           self.Bind(wx.EVT_BUTTON, self.Delete, deletebutton)
           sizer.Add(deletebutton, 1, wx.EXPAND)           
           
        button = wx.Button(panel, -1, "Config")
        self.Bind(wx.EVT_BUTTON, self.Config, button)
        sizer.Add(button, 1, wx.EXPAND)
        
        button = wx.Button(panel, -1, "Sim Admin")
        self.Bind(wx.EVT_BUTTON, self.SimAdmin, button)
        sizer.Add(button, 1, wx.EXPAND)

        button = wx.Button(panel, -1, "Quit")
        self.Bind(wx.EVT_BUTTON, self.Quit, button)
        sizer.Add(button, 1, wx.EXPAND)

        cancelbutton = wx.Button(panel, -1, "Cancel")
        self.Bind(wx.EVT_BUTTON, self.Cancel, cancelbutton)        
        sizer.Add(cancelbutton, 1, wx.EXPAND)        
                
        panel.SetSizer(sizer)
        panel.Layout()
        
    def SetSkybox( self, evt ):
       path = DoTextureThing()       
       self.Destroy()
       self.parentmodule.SetSkyboxTexture( path )
       
    def SetTSkybox( self, evt ):
       path = DoTextureThing()       
       self.Destroy()
       self.parentmodule.SetSkyboxTerrain( path )
       
    def ChangeName( self, evt ):
       self.Destroy()
       iReference = self.selected.iterkeys().next()
       sOldName = self.selected[iReference]["objectname"]
       sNewObjectName = GetObjectNameFromUser( sOldName )
       if( sNewObjectName != sOldName ):
          self.selected[ iReference ]["objectname"] = sNewObjectName
          self.parentmodule.SetName( iReference, sNewObjectName )
       
    def Create( self, evt ):
       self.Destroy()
       self.ShowCreateMenu()
 
    def Config( self, evt ): 
       self.Destroy()
       self.parentmodule.ShowConfigDialog()
       
    def SimAdmin( self, evt ):
       self.Destroy()
       simadminframe = FrameSimAdminmenu( None, self.screenx, self.screeny, self.selected, self.parentmodule )
       simadminframe.Show(True)
       
    def Texture( self, evt ):
       path = DoTextureThing()
       self.Destroy()
       self.parentmodule.ApplyTextureOnSelection( path )
       
    def EditTexture( self, evt ):
       self.Destroy()
       iReference = self.selected.keys()[0]
       self.parentmodule.EditTexture( iReference )
       
    def Color( self, evt ):
       # self.Destroy()
       color = DoColorDialog(self)
       self.Destroy()
       Color = {}
       Color["r"] = float(color[0])/256
       Color["g"] = float(color[1])/256
       Color["b"] = float(color[2])/256
       self.parentmodule.SetColorOnSelection( Color )
       
    def Export( self, evt ):
       self.Destroy()
       path = GetExportFilePath()
       self.parentmodule.Export( path )
       
    def Import( self, evt ):
       self.Destroy()
       path = GetImportFilePath()
       if path != "":
          self.parentmodule.Import( path )
       
    def Scripting( self, evt ):
       self.ShowScriptingMenu()
       self.Destroy()
       
    def Linking( self, evt ):
       self.ShowLinkingMenu()
       self.Destroy()
       
    def Delete( self, evt ):
       self.Destroy()
       self.parentmodule.DeleteSelection()
       
    def UnselectAll( self, evt ):
       self.Destroy()
       self.parentmodule.UnselectAll()
       
    def Cancel( self, evt ):
       self.Destroy()
       
    def KillFocus( self, evt ):
       print "killfocus"
       self.Destroy()
       
    def ShowCreateMenu( self ):
       createframe = FrameCreatemenu( None, self.screenx, self.screeny, self.selected, self.parentmodule )
       createframe.Show(True)
       
    def ShowLinkingMenu( self ):
       linkframe = FrameLinking( None, self.screenx, self.screeny, self.selected, self.parentmodule )
       linkframe.Show(True)
       
    def ShowScriptingMenu( self ):
       scriptingframe = FrameScripting( None, self.screenx, self.screeny, self.selected, self.parentmodule )
       scriptingframe.Show(True)
       
    def AskYesNoQuestion( self, message ):
       dialog = wx.MessageDialog( None, message, "Question", wx.YES_NO | wx.ICON_INFORMATION )
       result = dialog.ShowModal()
       dialog.Destroy()
       return result == wx.ID_YES
       
    def Quit( self, evt ):
       result = self.AskYesNoQuestion( "Quit.  Are you sure?" )
       if result:
          self.parentmodule.Quit()
       
    def OnKeyDown(self, evt):
       print "onkeydown"
       
       if( evt.GetKeyCode() == wx.WXK_ESCAPE ):
          self.Destroy()
    
    def Die( self ):
       self.Destroy()

currentcontextmenu = None
       
def DisplayContextMenu( parentmodule, iScreenX, iScreenY, Selected ):
   global currentcontextmenu

   # print "displaycontextmenu"
     
   displayrect = wx.ClientDisplayRect()
   screenheight = displayrect[3]
  
   if( iScreenY + 200 > screenheight ):
      iScreenY = screenheight - 200
           
   #print selected   
   
   if( currentcontextmenu != None ):
      try:
         currentcontextmenu.Die()
      except:
         pass
      currentcontextmenu = None
   
   currentcontextmenu = FrameContextMenu(None, "Pick an option:", iScreenX, iScreenY, Selected, parentmodule )
   currentcontextmenu.Show(True)

