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
import traceback

import os
import sys

from osmpclient import *

class pyDevices:
   PlayerMovement = None
   bCapture = False
   Editing3D = None
   SendToServer = None
   
   Keys = {}
   
   bLeftMouseDown = False
   bRightMouseDown = False   
      
   def SetPlayerMovement( self, playermovement ):
      self.PlayerMovement = playermovement
   
   def SetEditing3D( self, editing3d ):
      self.Editing3D = editing3d
   
   def SetCapture( self, newcapture ):
      self.bCapture = newcapture
   
   def SetSendToServer( self, sendtoserver ):
      self.SendToServer = sendtoserver
   
   def SetmvConfig( self, mvConfig ):
      self.mvConfig = mvConfig
      
   def SetCallingObject( self, Callingobject ):
      self.CallingObject = Callingobject
      
   def GetKey( self, keyarrayindex ):
      if self.Keys.has_key( keyarrayindex ):
         return self.Keys[ keyarrayindex ]
      else:
         return False

   def ReadModifiers( self ):
      self.kShift = False
      self.kCtrl = False
      self.kAlt = False  
      
      if self.GetKey( SDLK_RSHIFT ) or self.GetKey( SDLK_LSHIFT ):
         self.kShift = True
      
      if self.GetKey( SDLK_RALT ) or self.GetKey( SDLK_LALT ):
         self.kAlt = True
      
      if self.GetKey( SDLK_RCTRL ) or self.GetKey( SDLK_LCTRL ):
         self.kCtrl = True

   def CommandKeyPressed( self, sCommand ):
      self.ReadModifiers()
      
      sKeyCode = self.mvConfig.GetKeycodeForCommand( sCommand )
      sKeyCode = sKeyCode.lower()
      
      if( sKeyCode == "alt" ):
         return self.kAlt
         
      elif( sKeyCode == "ctrl" ):
         return self.kCtrl
      
      elif( sKeyCode == "shift" ):
         return self.kShift
      
      elif( sKeyCode == "f1" ):
         return self.kF1
      
      elif( sKeyCode == "f9" ):
         return self.kF9
      
      elif( sKeyCode == "leftarrow" ):
         return self.GetKey( SDLK_LEFT )
      
      elif( sKeyCode == "rightarrow" ):
         return self.GetKey( SDLK_RIGHT )
      
      elif( sKeyCode == "uparrow" ):
         return self.GetKey( SDLK_UP )
      
      elif( sKeyCode == "downarrow" ):
         return self.GetKey( SDLK_DOWN )
      
      elif( sKeyCode == "spacebar" ):
         return self.GetKey(' ')
      
      elif( sKeyCode == "delete" ):
         return self.GetKey(SDLK_DELETE)
      
      elif( sKeyCode == "enter" ):
         return self.GetKey(SDLK_RETURN)
      
      else:
          return self.GetKey( ord( sKeyCode ) )

   def KeyCodeIntToKeyName( self, iKeyCode ):
      if iKeyCode == SDLK_LSHIFT or iKeyCode == SDLK_RSHIFT:
        sKeyCode = "shift"
           
      elif iKeyCode == SDLK_LALT or iKeyCode == SDLK_RALT:
        sKeyCode = "alt"
           
      elif iKeyCode == SDLK_LCTRL or iKeyCode == SDLK_RCTRL:
        sKeyCode = "ctrl"
          
      elif iKeyCode == SDLK_F1:
        sKeyCode = "f1"
          
      elif iKeyCode == SDLK_F9:
        sKeyCode = "f9"
          
      elif iKeyCode == SDLK_LEFT:
        sKeyCode = "leftarrow"
          
      elif iKeyCode == SDLK_RIGHT:
        sKeyCode = "rightarrow"
          
      elif iKeyCode == SDLK_UP:
        sKeyCode = "uparrow"
          
      elif iKeyCode == SDLK_DOWN:
        sKeyCode = "downarrow"
          
      elif iKeyCode == SDLK_DELETE:
        sKeyCode = "delete"
          
      elif iKeyCode == SDLK_RETURN:
        sKeyCode = "enter"
      
      else:
          sKeyCode = chr( iKeyCode )

      return sKeyCode
   
   def KeyCodeIntToCommand( self, iKeyCode ):
       sKeyCode = self.KeyCodeIntToKeyName(iKeyCode)
       return self.mvConfig.GetCommandForKeycode( sKeyCode )

   def KeyDown( self, keycode ):
      try:
         print "keydown: " + str( keycode )
         self.Keys[ keycode ] = True
          
         sCommandString = self.KeyCodeIntToCommand( keycode )
         print "sCommandString = " + sCommandString
         if self.bCapture:
            self.SendToServer( '<keydown value="' + self.KeyCodeIntToKeyName( keycode ) << '"/>\n' )
      
         if( sCommandString == "moveleft" ):
            self.PlayerMovement.kMovingLeft = True
      
         elif( sCommandString == "moveright" ):
            self.PlayerMovement.kMovingRight = True
      
         elif( sCommandString == "moveforwards" ):
            self.PlayerMovement.kMovingForwards = True
      
         elif( sCommandString == "movebackwards" ):
            self.PlayerMovement.kMovingBackwards = True
      
         elif( sCommandString == "moveup" ):
            self.PlayerMovement.kMovingUpZAxis = True
      
         elif( sCommandString == "movedown" ):
            self.PlayerMovement.kMovingDownZAxis = True
      
         elif( sCommandString == "jump" ):
             self.PlayerMovement.bJumping = true
             
         elif( sCommandString == "activatechat" ):
              self.CallingObject.ActivateChat()
              
         elif( sCommandString == "delete" ):
            self.CallingObject.DeleteCurrentSelection()
            
         elif( sCommandString == "help" ):
            self.CallingObject.DisplayHelp()
            
         elif( sCommandString == "toggleviewpoint" ):
            print "toggleviewpoint"
            try:
               ToggleViewPoint()
            except:
               traceback.print_exc(file=sys.stdout)
               while 1:
                  time.sleep(1.0)
   
         elif( sCommandString == "editmode" or sCommandString == "editrotation" or \
               sCommandString == "editscale"  ):
            if( self.CommandKeyPressed( "editmode" ) ):
               if( self.CommandKeyPressed( "editscale" ) ):
                  self.Editing3D.ShowEditScaleBars()
               elif( self.CommandKeyPressed( "editrotation" ) ):
                  self.Editing3D.ShowEditRotBars()
               else:
                  print "showing editposbars..."
                  self.Editing3D.ShowEditPosBars()
      except:
         traceback.print_exc( sys.__stderr__ )

   def KeyUp( self, keycode ):
      try:
         self.Keys[ keycode ] = False
         self.ReadModifiers()
         
         sCommandString = self.KeyCodeIntToCommand( keycode )
         if (self.bCapture ):
            self.SendToServer( '<keyup value="' + self.KeyCodeIntToKeyName( keycode ) << '"/>\n' )
   
         if( sCommandString == "moveleft" ):
            self.PlayerMovement.kMovingLeft = False
      
         elif( sCommandString == "moveright" ):
            self.PlayerMovement.kMovingRight = False
      
         elif( sCommandString == "moveforwards" ):
            self.PlayerMovement.kMovingForwards = False
      
         elif( sCommandString == "movebackwards" ):
            self.PlayerMovement.kMovingBackwards = False
      
         elif( sCommandString == "moveup" ):
            self.PlayerMovement.kMovingUpZAxis = False
      
         elif( sCommandString == "movedown" ):
            self.PlayerMovement.kMovingDownZAxis = False
      
         elif( sCommandString == "editmode" or sCommandString == "editrotation" \
               or sCommandString == "editscale" ):
               
            if( self.CommandKeyPressed( "editmode" ) ):
            
               if( self.CommandKeyPressed( "editscale" ) ):
                  self.Editing3D.ShowEditScaleBars();
   
               elif( self.CommandKeyPressed( "editrotation" ) ):
                  self.Editing3D.ShowEditRotBars()
   
               else:
                  self.Editing3D.ShowEditPosBars()
                  
            else:
               self.Editing3D.HideEditBars()
   
      except:
         traceback.print_exc(file=sys.stdout)
         while 1:
            time.sleep(1.0)
            
   def MouseMove( self, x, y ):
      try:
         if( self.bLeftMouseDown ):
            if( self.CommandKeyPressed( "editmode" ) ):
               self.EditMove( x, y )
               
            elif( self.CommandKeyPressed( "cameramode" ) ):
               # print "moving camera..."
               self.CameraMove( x, y )
               
            else:
                self.CallingObject.Camera.CancelCamera()
                self.PlayerMovement.MouseLook( x - self.mousex, y - self.mousey )
                
         self.mousex = x
         self.mousey = y
      except:
         traceback.print_exc(file=sys.stdout)
         while 1:
            time.sleep(1.0)
   
   def MouseDown( self, bLeftDown, bRightDown, x, y ):
      try:
         if bLeftDown:
            self.bLeftMouseDown = True
            
            if self.CommandKeyPressed( "cameramode" ):
               print "camera mode selected, initiating camera move..."
               self.InitiateCameraMove( x, y )
            
            elif( self.CommandKeyPressed( "editmode" ) ):
               print "edit mode selected, initiating edit move..."
               self.InitiateEditMove( x, y )
   
            elif( self.CommandKeyPressed( "selectobject" ) ):
               self.CallingObject.Selector.ToggleClickedInSelection( True, x, y )
   
            elif( self.CommandKeyPressed( "selectindividual" ) ):
               self.CallingObject.Selector.ToggleClickedInSelection( False, x, y )
   
            else:
               self.CallingObject.SendClickEventToServer( x, y )
   
         elif bRightDown:
            self.CallingObject.DisplayContextMenu( x, y )
            self.bRightMouseDown = True
      except:
         traceback.print_exc(file=sys.stdout)
         while 1:
            time.sleep(1.0)
   
   def MouseUp( self, bLeftUp, bRightUp, x, y ):
      try:
         if bLeftUp:
            self.bLeftMouseDown = False
            self.bDragging = False
            self.Editing3D.EditDone()
            self.CallingObject.Camera.CameraMoveDone()
         
         elif bRightUp:      	      
            self.bRightMouseDown = False
         
         self.mousex = x
         self.mousey = y
      except:
         traceback.print_exc(file=sys.stdout)
         while 1:
            time.sleep(1.0)
   
   def CameraMove( self, x, y ):
       #print "camera move..."
       if( self.CommandKeyPressed( "camerapan" ) ):
          #print "updating pan camera..."
          self.CallingObject.Camera.UpdatePanCamera( x, y )

       elif( self.CommandKeyPressed( "cameraorbit" ) ):
          #print "updating orbit camera..."
          self.CallingObject.Camera.UpdateOrbitCamera( x, y )

       else:
          #print "updating alt zoom camera..."
          self.CallingObject.Camera.UpdateAltZoomCamera( x, y )
       # print "camera updated"
   
   def InitiateCameraMove( self, x, y ):
       #print "initiate camera move"
       if( self.CommandKeyPressed( "camerapan" ) ):
          self.CallingObject.Camera.InitiatePanCamera( x, y )

       elif( self.CommandKeyPressed( "cameraorbit" ) ):
          self.CallingObject.Camera.InitiateOrbitCamera( x, y )

       else:
          self.CallingObject.Camera.InitiateAltZoomCamera( x, y )
       #print "done"
   
   def EditMove( self, x, y ):
       bAltAxesPressed = self.CommandKeyPressed( "editalternativeaxes" )
       if( self.CommandKeyPressed( "editscale" ) ):
           self.CallingObject.Editing3D.UpdateScaleEdit( bAltAxesPressed, x, y )

       elif( self.CommandKeyPressed( "editrotation" ) ):
           self.CallingObject.Editing3D.UpdateRotateEdit( bAltAxesPressed, x, y )

       else:
           self.CallingObject.Editing3D.UpdateTranslateEdit( bAltAxesPressed, x, y )
   
   def InitiateEditMove( self, x, y ):
       if( self.CommandKeyPressed( "editscale" ) ):
            self.CallingObject.Editing3D.InitiateScaleEdit( x, y )

       elif( self.CommandKeyPressed( "editrotation" ) ):
            self.CallingObject.Editing3D.InitiateRotateEdit( x, y )

       else:
            self.CallingObject.Editing3D.InitiateTranslateEdit( x, y )

