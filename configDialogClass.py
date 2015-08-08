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
import os
import string
import xml.dom.minidom
from wxPython.wx import *

class configDialogFrame(wx.Frame):
    def __init__(self, startuppath, parent, *args, **kwds):
        # begin wxGlade: MyFrame4.__init__
        kwds["style"] = wx.CAPTION|wx.MINIMIZE_BOX| wx.CLOSE_BOX|wx.MAXIMIZE_BOX|wx.SYSTEM_MENU|wx.RESIZE_BORDER|wx.FRAME_NO_TASKBAR|wx.TAB_TRAVERSAL 
        wx.Frame.__init__(self, *args, **kwds)
        self.notebook_1 = wx.Notebook(self, -1, style=0)
        self.notebook_1_pane_3 = wx.Panel(self.notebook_1, -1)
        self.notebook_1_pane_2 = wx.Panel(self.notebook_1, -1)
        self.notebook_1_pane_1 = wx.Panel(self.notebook_1, -1)
        self.label_4 = wx.StaticText(self.notebook_1_pane_1, -1, "Text Editor", style=wx.ALIGN_RIGHT)
        self.label_2 = wx.StaticText(self.notebook_1_pane_1, -1, "Script Editor", style=wx.ALIGN_RIGHT)
        self.label_5 = wx.StaticText(self.notebook_1_pane_1, -1, "Texture Editor", style=wx.ALIGN_RIGHT)
        self.textEditor = wx.TextCtrl(self.notebook_1_pane_1, -1, "")
        self.scriptEditor = wx.TextCtrl(self.notebook_1_pane_1, -1, "")
        self.textureEditor = wx.TextCtrl(self.notebook_1_pane_1, -1, "")
        self.textEditorSelect = wx.Button(self.notebook_1_pane_1, -1, "Select...")
        self.scriptEditorSelect = wx.Button(self.notebook_1_pane_1, -1, "Select...")
        self.textureEditorSelect = wx.Button(self.notebook_1_pane_1, -1, "Select...")
        self.label_6 = wx.StaticText(self.notebook_1_pane_2, -1, "Temp Files")
        self.tempFiles = wx.TextCtrl(self.notebook_1_pane_2, -1, "")
        self.tempFilesSelect = wx.Button(self.notebook_1_pane_2, -1, "Select...")
        self.label_7 = wx.StaticText(self.notebook_1_pane_3, -1, "Command")		
        self.commandList = wx.ListBox(self.notebook_1_pane_3, -1, choices=[])
        self.static_line_1 = wx.StaticLine(self.notebook_1_pane_3, -1)
        self.add = wx.Button(self.notebook_1_pane_3, -1, "Add")
        self.remove = wx.Button(self.notebook_1_pane_3, -1, "Remove")
        self.update = wx.Button(self.notebook_1_pane_3, -1, "Update")
        self.conflict = wx.StaticText(self.notebook_1_pane_3, -1, "conflict detected")
        self.label_8 = wx.StaticText(self.notebook_1_pane_3, -1, "Keycode")
        self.keycode = wx.TextCtrl(self.notebook_1_pane_3, -1, "", style=wx.TE_READONLY)             					  
        self.okay = wx.Button(self, -1, "&Okay")
        self.apply = wx.Button(self, -1, "&Apply")        
        self.cancel = wx.Button(self, -1, "&Cancel")

        self.__set_properties()
        self.__do_layout()
        # end wxGlade
        self.parentmodule = parent
        EVT_CLOSE(self, self.OnCancel)
        self.Bind(EVT_BUTTON, self.OnAdd, self.add)
        self.Bind(EVT_BUTTON, self.OnRemove, self.remove)
        self.Bind(EVT_LISTBOX, self.OnClick, self.commandList)
        self.Bind(EVT_BUTTON, self.OnOkay, self.okay)
        self.Bind(EVT_BUTTON, self.OnApply, self.apply)
        self.Bind(EVT_BUTTON, self.OnUpdate, self.update)
        self.Bind(EVT_BUTTON, self.OnCancel, self.cancel)
        self.Bind(EVT_BUTTON, self.OnTextEditorSelect, self.textEditorSelect)
        self.Bind(EVT_BUTTON, self.OnScriptEditorSelect, self.scriptEditorSelect)
        self.Bind(EVT_BUTTON, self.OnTextureEditorSelect, self.textureEditorSelect)
        self.Bind(EVT_BUTTON, self.OnTempFilesSelect, self.tempFilesSelect)
        self.Bind(EVT_KEY_DOWN,  self.OnChar, self)
        self.Bind(EVT_TEXT, self.OnChange, self.textEditor)
        self.Bind(EVT_TEXT, self.OnChange, self.scriptEditor)
        self.Bind(EVT_TEXT, self.OnChange, self.textureEditor)
        self.Bind(EVT_TEXT, self.OnChange, self.tempFiles)    
        EVT_SET_FOCUS(self.keycode, self.DoFocus)            
        self.Centre()
        str_list = []        
        str_list.append(startuppath)
        str_list.append(os.sep)
        str_list.append("config.xml")
        self.configfile = ''.join(str_list)

        self.loadConfig()        
        self.drawkeylist()
        self.apply.Enable(False)
        self.update.Enable(False)
        self.remove.Enable(False)
        self.checkConflict()
        
    def DoFocus(self, event):
        self.SetFocus()

    def loadConfig(self):
        doc = xml.dom.minidom.parse(self.configfile,"")
        e = doc.getElementsByTagName("scripteditor")
        try:
            self.scriptEditor.SetValue(e.item(0).getAttribute("path"))
        except:
            pass        
        e = doc.getElementsByTagName("texteditor")
        try:
            self.textEditor.SetValue(e.item(0).getAttribute("path"))
        except:
            pass        
        e = doc.getElementsByTagName("textureeditor")
        try:
            self.textureEditor.SetValue(e.item(0).getAttribute("path"))
        except:
            pass        
        e = doc.getElementsByTagName("temp")
        try:
            self.tempFiles.SetValue(e.item(0).getAttribute("path"))
        except:
            pass        
        e = doc.getElementsByTagName("key") 
        self.keylistk = []
        self.keylistv = []
        for f in e:
           try:
               self.keylistk.append(f.getAttribute("command"))
               self.keylistv.append(f.getAttribute("keycode"))
           except:
               pass
        
        self.configdata = doc

    def saveConfig(self):
        doc = self.configdata
        e = doc.getElementsByTagName("scripteditor")        
        try:
            e.item(0).setAttribute("path", self.scriptEditor.GetValue())
        except:
            pass
        e = doc.getElementsByTagName("texteditor")
        try:
            e.item(0).setAttribute("path", self.textEditor.GetValue())
        except:
            pass
        e = doc.getElementsByTagName("textureeditor")
        try:
            e.item(0).setAttribute("path", self.textureEditor.GetValue())
        except:
            pass
        e = doc.getElementsByTagName("temp")
        try:
            e.item(0).setAttribute("path", self.tempFiles.GetValue())
        except:
            pass
        m = doc.getElementsByTagName("keymappings")
        e = doc.getElementsByTagName("key")  
        try:      
            for k in e:
                k.unlink()
                m.item(0).removeChild(k)            

            for i,k in enumerate(self.keylistk):
                t = doc.createElement("key") 
                t.setAttribute("command", k)
                t.setAttribute("keycode", self.keylistv[i])
                m.item(0).appendChild(t)
        except:
            pass

        out_file = open(self.configfile, "w")
        out_file.write(doc.toxml())
        out_file.close()
        
    def checkConflict(self):
		tmplistk = []
		tmplistv = []
		self.conflict.Hide()
		m = "Conflicts:"
		found = ""
		for i,j in enumerate(self.keylistk):    	    
			m = self.keylistv[i]			
			for k,l in enumerate(tmplistk):
				n = tmplistv[k]
				if (m == n):					
					found = found + "\n" + j + " and " + l + " both using : " + m
    		
			if (found == ""):
				tmplistk.append(self.keylistk[i])
				tmplistv.append(self.keylistv[i])
			else:
				self.conflict.SetLabel(found)
				self.conflict.Show()

    def setKeylist(self, k, v):
        self.keylistk.append(k)
        self.keylistv.append(v)

    def drawkeylist(self):
        self.commandList.Clear()
        self.keycode.SetValue(" ")
        for k in self.keylistk:
            self.commandList.Append(k)

    def OnAdd(self, event):        
        addDlg = ConfigDialogDlg(self, None, -1, "")
        addDlg.Show(True)

    def OnRemove(self, event):
        try: 
           x = self.keylistk.index(self.commandList.GetStringSelection())
           del self.keylistk[x]
           del self.keylistv[x]
        except:
           pass

        self.apply.Enable(True)
        self.drawkeylist()
        
    def OnUpdate(self, event):
        self.keylistv[self.commandList.GetSelection()] = self.keycode.GetValue().lower()
        self.checkConflict()
        self.update.Enable(False)
        self.apply.Enable(True)

    def OnClick(self, event):
        if (self.commandList.GetStringSelection() == ""):
           self.remove.Enable(False)
           self.keycode.SetValue(" ")           
        else:
           self.remove.Enable(True)
           self.keycode.SetValue(self.keylistv[self.commandList.GetSelection()].upper())

    def OnChange(self, event):
       self.apply.Enable(True)

    def OnTextEditorSelect(self, event):
       dlg = wx.FileDialog(None, message="Choose a text editor", wildcard="Applications (*.exe)|*.exe|All files|*.*", style=wx.OPEN | wx.CHANGE_DIR|wxHIDE_READONLY)
        
       path = ""
       if dlg.ShowModal() == wx.ID_OK:      
          path = dlg.GetPath()
          self.textEditor.SetValue(path)

       return path

    def OnScriptEditorSelect(self, event):
       dlg = wx.FileDialog(None, message="Choose a script editor", wildcard="Applications (*.exe)|*.exe|All files|*.*", style=wx.OPEN | wx.CHANGE_DIR|wxHIDE_READONLY)
        
       path = ""
       if dlg.ShowModal() == wx.ID_OK:      
          path = dlg.GetPath()
          self.scriptEditor.SetValue(path)

       return path


    def OnTextureEditorSelect(self, event):
       dlg = wx.FileDialog(None, message="Choose a texture editor", wildcard="Applications (*.exe)|*.exe|All files|*.*", style=wx.OPEN | wx.CHANGE_DIR|wxHIDE_READONLY)
        
       path = ""
       if dlg.ShowModal() == wx.ID_OK:      
          path = dlg.GetPath()
          self.textureEditor.SetValue(path)

       return path

    def OnTempFilesSelect(self, event):
       dlg = wxDirDialog(None, "Choose a directory", "", wxDD_NEW_DIR_BUTTON)
        
       path = ""
       if dlg.ShowModal() == wx.ID_OK:      
          path = dlg.GetPath()
          self.tempFiles.SetValue(path)

       return path

    def OnApply( self, event):
        self.saveConfig()
        self.parentmodule.ReloadConfig()
        self.apply.Disable()

    def OnOkay(self,event):
        self.saveConfig()
        self.parentmodule.ReloadConfig()
        self.Show(False)

    def OnCancel(self,event):
        self.Show(False)
        
    def OnChar(self, event):   
        keymap = {WXK_F1:'F1', WXK_F2:'F2', WXK_F3:'F3', WXK_F3:'F3', WXK_F4:'F4', WXK_F5:'F5', WXK_F6:'F6', WXK_F7:'F7', WXK_F8:'F8', WXK_F9:'F9', WXK_UP:'UPARROW', WXK_DOWN:'DOWNARROW', WXK_LEFT:'LEFTARROW', WXK_RIGHT:'RIGHTARROW', WXK_SPACE  :'SPACEBAR', WXK_SHIFT : 'SHIFT', WXK_MENU :'ALT', WXK_CONTROL:'CTRL', WXK_DELETE:'DELETE', WXK_RETURN:'ENTER'}
        
        keycode = event.GetKeyCode()
        keystr = ""
        try:
            keystr = keymap[keycode]
        except:
            try:
                if (chr(keycode).isalnum()):
                    keystr2 = chr(keycode)
                    keystr = keystr2.upper()
            except:
                pass

        if (keystr == ""):
           self.update.Enable(False)
           self.keycode.SetValue(" ")
        else:
           self.update.Enable(True)
           self.keycode.SetValue(keystr)

    def __set_properties(self):
        # begin wxGlade: MyFrame4.__set_properties
        self.SetTitle("Configure metaverse client")
        self.SetBackgroundColour(wx.Colour(212, 208, 200))
        self.SetSize((442, 314))
        if (self.commandList.GetCount() > 0) :
           self.commandList.SetSelection(0)
        # end wxGlade

    def __do_layout(self):
        # begin wxGlade: MyFrame4.__do_layout
        sizer_3 = wx.BoxSizer(wx.VERTICAL)
        sizer_14 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_15 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_17 = wx.BoxSizer(wx.VERTICAL)
        sizer_16 = wx.BoxSizer(wx.VERTICAL)
        sizer_18 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_7 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_6 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_13 = wx.BoxSizer(wx.VERTICAL)
        sizer_12 = wx.BoxSizer(wx.VERTICAL)
        sizer_8 = wx.BoxSizer(wx.VERTICAL)
        sizer_8.Add(self.label_4, 0, wx.TOP|wx.BOTTOM|wx.ALIGN_RIGHT, 3)
        sizer_8.Add(self.label_2, 0, wx.TOP|wx.ALIGN_RIGHT, 5)
        sizer_8.Add(self.label_5, 0, wx.TOP|wx.ALIGN_RIGHT, 8)
        sizer_6.Add(sizer_8, 0, wx.ALIGN_CENTER_VERTICAL, 0)
        sizer_12.Add(self.textEditor, 0, wx.LEFT|wx.EXPAND, 3)
        sizer_12.Add(self.scriptEditor, 0, wx.LEFT|wx.EXPAND, 3)
        sizer_12.Add(self.textureEditor, 0, wx.LEFT|wx.EXPAND, 3)
        sizer_6.Add(sizer_12, 3, wx.ALIGN_CENTER_VERTICAL, 0)
        sizer_13.Add(self.textEditorSelect, 0, wx.ALL|wx.ALIGN_CENTER_HORIZONTAL, 0)
        sizer_13.Add(self.scriptEditorSelect, 0, wx.ALL|wx.ALIGN_CENTER_HORIZONTAL, 0)
        sizer_13.Add(self.textureEditorSelect, 0, wx.ALL|wx.ALIGN_CENTER_HORIZONTAL, 0)
        sizer_6.Add(sizer_13, 1, wx.ALIGN_CENTER_VERTICAL, 0)
        self.notebook_1_pane_1.SetAutoLayout(1)
        self.notebook_1_pane_1.SetSizer(sizer_6)
        sizer_6.Fit(self.notebook_1_pane_1)
        sizer_6.SetSizeHints(self.notebook_1_pane_1)
        sizer_7.Add(self.label_6, 0, wx.LEFT|wx.RIGHT|wx.ALIGN_CENTER_VERTICAL, 4)
        sizer_7.Add(self.tempFiles, 3, wx.RIGHT|wx.ALIGN_CENTER_VERTICAL, 3)
        sizer_7.Add(self.tempFilesSelect, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 1)
        self.notebook_1_pane_2.SetAutoLayout(1)
        self.notebook_1_pane_2.SetSizer(sizer_7)
        sizer_7.Fit(self.notebook_1_pane_2)
        sizer_7.SetSizeHints(self.notebook_1_pane_2)
        sizer_16.Add(self.label_7, 0, wx.LEFT, 60)
        sizer_16.Add(self.commandList, 0, wx.LEFT, 60)
        sizer_16.Add(self.static_line_1, 0, wx.ALL|wx.EXPAND, 4)
        sizer_18.Add(self.add, 0, 0, 0)
        sizer_18.Add(self.remove, 0, 0, 0)
        sizer_18.Add(self.update, 0, 0, 0)        
        sizer_16.Add(sizer_18, 1, wx.LEFT, 40)
        sizer_15.Add(sizer_16, 1, wx.ALIGN_CENTER_VERTICAL, 0)
        sizer_17.Add(self.label_8, 0, wx.LEFT, 30)
        sizer_17.Add(self.keycode, 0, wx.LEFT|wx.BOTTOM, 30)
        sizer_15.Add(sizer_17, 1, wx.ALIGN_CENTER_VERTICAL, 0)
        self.notebook_1_pane_3.SetAutoLayout(1)
        self.notebook_1_pane_3.SetSizer(sizer_15)
        sizer_15.Fit(self.notebook_1_pane_3)
        sizer_15.SetSizeHints(self.notebook_1_pane_3)
        self.notebook_1.AddPage(self.notebook_1_pane_1, "Editors")
        self.notebook_1.AddPage(self.notebook_1_pane_2, "Directories")
        self.notebook_1.AddPage(self.notebook_1_pane_3, "Keyboard")
        sizer_3.Add(wx.NotebookSizer(self.notebook_1), 3, wx.ALL|wx.EXPAND, 3)
        sizer_14.Add(self.okay, 0, wx.ALL|wx.ALIGN_BOTTOM|wx.ALIGN_CENTER_VERTICAL, 3)
        sizer_14.Add(self.apply, 0, wx.ALL|wx.ALIGN_BOTTOM, 3)
        sizer_14.Add(self.cancel, 0, wx.ALL|wx.ALIGN_BOTTOM, 3)
        sizer_3.Add(sizer_14, 0, wx.ALIGN_RIGHT, 0)
        self.SetAutoLayout(1)
        self.SetSizer(sizer_3)
        self.Layout()
        # end wxGlade

class ConfigDialogDlg(wx.Frame):
    def __init__(self, prt, *args, **kwds):
        # begin wxGlade: MyDialog1.__init__
        kwds["style"] = wxWANTS_CHARS|wxSTAY_ON_TOP|wxCAPTION|wxTAB_TRAVERSAL |wxFRAME_NO_TASKBAR
        wx.Frame.__init__(self, *args, **kwds)
        self.label_9 = wx.StaticText(self, -1, "Command")
        self.selectCommand = wx.ComboBox(self, -1, choices=["mouselook", "moveleft", "moveright", "moveforwards", "movebackwards", "moveup", "movedown", "jump", "cameramode", "cameraorbit", "camerapan", "editmode", "editrotation", "editscale", "editalternativeaxes", "selectobject", "selectindividual", "help", "toggleviewpoint"], style=wxCB_READONLY|wxCB_SORT)
        self.label_10 = wx.StaticText(self, -1, "Keycode")
        #self.selectKeycode = wx.StaticText(self, -1, " ", size=(100,19), style=wx.ST_NO_AUTORESIZE )
        self.selectKeycode = wx.TextCtrl(self, -1, "",style=wx.TE_READONLY)
        self.static_line_2 = wx.StaticLine(self, -1)
        self.addokay = wx.Button(self, -1, "Okay")
        self.addcancel = wx.Button(self, -1, "Cancel")

        self.__set_properties()
        self.__do_layout()
        # end wxGlade
        self.prt = prt
        self.addokay.Enable(False)
        self.Bind(EVT_BUTTON, self.OnAddOkay, self.addokay)
        self.Bind(EVT_BUTTON, self.OnAddCancel, self.addcancel)
        self.Bind(EVT_KEY_DOWN,  self.OnChar, self)
        EVT_SET_FOCUS(self.selectKeycode, self.DoFocus)
        self.MakeModal(True)
        self.Centre()

    def DoFocus(self, event):
        self.SetFocus()

    def OnAddOkay(self, event):
        item = self.selectCommand.GetStringSelection()
        self.prt.setKeylist(item, self.selectKeycode.GetValue())
        self.prt.checkConflict()
        self.prt.drawkeylist()
        self.prt.apply.Enable(True)
        self.MakeModal(False)
        self.Destroy()

    def OnAddCancel(self, event):
        self.MakeModal(False)
        self.Destroy()
    
    def OnChar(self, event):   
        keymap = {WXK_F1:'F1', WXK_F2:'F2', WXK_F3:'F3', WXK_F3:'F3', WXK_F4:'F4', WXK_F5:'F5', WXK_F6:'F6', WXK_F7:'F7', WXK_F8:'F8', WXK_F9:'F9', WXK_UP:'UPARROW', WXK_DOWN:'DOWNARROW', WXK_LEFT:'LEFTARROW', WXK_RIGHT:'RIGHTARROW', WXK_SPACE  :'SPACEBAR', WXK_SHIFT : 'SHIFT', WXK_MENU :'ALT', WXK_CONTROL:'CTRL', WXK_DELETE:'DELETE', WXK_RETURN:'ENTER'}
        
        keycode = event.GetKeyCode()
        keystr = ""
        try:
            keystr = keymap[keycode]
        except:
            try:
                if (chr(keycode).isalnum()):
                    keystr2 = chr(keycode)
                    keystr = keystr2.upper()
            except:
                pass

        if (keystr == ""):
           self.addokay.Enable(False)
           self.selectKeycode.SetValue(" ")
        else:
           self.addokay.Enable(True)
           self.selectKeycode.SetValue(keystr)

    def __set_properties(self):
        # begin wxGlade: MyDialog1.__set_properties
        self.SetTitle("Add Command")
        self.SetBackgroundColour(wx.Colour(212, 208, 200))
        self.selectCommand.SetSelection(0)
        self.label_9.SetBackgroundColour(wx.Colour(212, 208, 200))
        self.label_10.SetBackgroundColour(wx.Colour(212, 208, 200))
        self.selectKeycode.SetBackgroundColour(wx.Colour(247, 255, 217))
        self.selectKeycode.SetFont(wx.Font(12, wx.MODERN, wx.NORMAL, wx.NORMAL, 0, ""))
        # end wxGlade

    def __do_layout(self):
        # begin wxGlade: MyDialog1.__do_layout
        sizer_19 = wx.BoxSizer(wx.VERTICAL)
        sizer_20 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_21 = wx.BoxSizer(wx.HORIZONTAL)
        sizer_23 = wx.BoxSizer(wx.VERTICAL)
        sizer_22 = wx.BoxSizer(wx.VERTICAL)
        sizer_22.Add(self.label_9, 0, wx.ALL, 2)
        sizer_22.Add(self.selectCommand, 0, wx.ALL|wx.EXPAND, 2)
        sizer_21.Add(sizer_22, 1, 0, 0)
        sizer_21.Add((20, 20), 0, 0, 0)
        sizer_23.Add(self.label_10, 0, wx.ALL, 2)
        sizer_23.Add(self.selectKeycode, 0, wx.TOP|wx.ALIGN_BOTTOM, 3)
        sizer_21.Add(sizer_23, 1, 0, 0)
        sizer_19.Add(sizer_21, 0, wx.EXPAND, 0)
        sizer_19.Add(self.static_line_2, 0, wx.ALL|wx.EXPAND, 8)
        sizer_20.Add(self.addokay, 0, wx.ALL, 2)
        sizer_20.Add(self.addcancel, 0, wx.ALL, 2)
        sizer_19.Add(sizer_20, 0, wx.ALIGN_CENTER_HORIZONTAL, 0)
        self.SetAutoLayout(1)
        self.SetSizer(sizer_19)
        sizer_19.Fit(self)
        sizer_19.SetSizeHints(self)
        self.Layout()
        # end wxGlade
