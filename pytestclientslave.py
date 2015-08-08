import traceback
import sys
import time
import wx

import pylogin2
import osmpclient

app = None
loginwindow = None

class OurCallbacks( osmpclient.CallbackToPythonClass ):
   #def __init__(self):
   #   osmpclient.CallbackToPythonClass.__init__( self )
   def DoEvents( self ):
      print "DoEvents"
      # app.Update()
      print app.Pending()
      #while app.Pending():
      #   app.Dispatch()
      # app.Yield()
      loginwindow.Refresh()
      loginwindow.Update()

class PyGUIApp( wx.App ):
   def OnInit(self):
      global app, loginwindow
      app = self
      window = pylogin2.ShowLoginFrame( self )
      self.OurCallbacks = OurCallbacks()
      osmpclient.InitMetaverseClient( self.OurCallbacks );
      osmpclient.MetaverseclientStartRenderer();
            
      return True

try:
   app = PyGUIApp(True)
   app.MainLoop()
   print "done"
except:
   traceback.print_exc(file=sys.stdout)
   while 1:
      time.sleep(1.0)
print "done"

