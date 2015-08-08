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

import xml.dom.minidom

bWasChanged = False

def GetNodesOfName( element, nodeName ):
   nodes = []
   # print element.nodeName + " " + nodeName
   for childelement in element.childNodes:
      if childelement.nodeType == element.ELEMENT_NODE:
         # print "GetNodesOfName(): " + childelement.nodeName
         if childelement.nodeName == nodeName:
            nodes.append( childelement )
   # print "GetNodesOfName(): " + element.nodeName + " " + nodeName + " " + str( nodes )
   return nodes
      
def OverwriteXML( targetelement, sourceelement ):
   # print targetelement.nodeName + " " + sourceelement.nodeName

   for childelement in sourceelement.childNodes:
      if childelement.nodeType == childelement.ELEMENT_NODE:
         # print "OverwriteXML(): " + childelement.nodeName
         if len( GetNodesOfName( targetelement, childelement.nodeName ) ) == 0:
            print "Cloning node " + childelement.nodeName
            global bWasChanged
            bWasChanged = True
            print str( childelement )
            print str( targetelement )
            clonednode = childelement.cloneNode( True )
            targetelement.appendChild( clonednode )
            print str( targetelement )
         
         if childelement.hasChildNodes():
            OverwriteXML( GetNodesOfName( targetelement, childelement.nodeName )[0], childelement )

def CopyFile( targetname, sourcename ):
   srchandle = file( sourcename, "r" )
   srclines = string.join( srchandle.readlines(), "" )
   srchandle.close()
   
   dsthandle = file( targetname, "w")
   dsthandle.write( srclines )
   dsthandle.close()

def UpdateConfigXMLFile():
   import string
   
   global bWasChanged
   bWasChanged = False

   startuppath = os.getcwd()
   print "Startup path: " + startuppath
   
   if not os.path.exists( startuppath + "/config.xml" ):
      CopyFile( startuppath + "/config.xml", startuppath + "/config.xml.template" )
      return
   
   TemplateFileHandle = file( startuppath + "/config.xml.template", "r" )
   TemplateFileContents = string.join( TemplateFileHandle.readlines(), "" )
   TemplateFileHandle.close()
   
   TemplateDom = xml.dom.minidom.parseString( TemplateFileContents )

   ConfigFileHandle = file( startuppath + "/config.xml", "r" )
   ConfigFileContents = string.join( ConfigFileHandle.readlines(), "" )
   ConfigFileHandle.close()

   try:
      ConfigDom = xml.dom.minidom.parseString( ConfigFileContents )
   except:
      print "Config.xml updated.  Writing out changes..."
      CopyFile( startuppath + "/config.xml", startuppath + "/config.xml.template" )
      return      

   OverwriteXML( ConfigDom.documentElement, TemplateDom.documentElement )

   if bWasChanged:
      print "Config.xml updated.  Writing out changes..."
      newtext = ConfigDom.toprettyxml( "  ", "\n", 'utf-8') 
      
      splittext = string.split( newtext, "\n" )
      newtext = ""
      for line in splittext:
         if line.strip() != "":
            newtext = newtext + line + "\n"      

      ConfigFileHandle = file( startuppath + "/config.xml", "w" )
      ConfigFileHandle.write( newtext )
      ConfigFileHandle.close()

   # sys.exit(1)
