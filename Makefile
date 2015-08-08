# Copyright Hugh Perkins 2004, 2005
#
#################################################
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
#################################################

#################################################
#
# This makefile is divided into sections as follows:
# - #defines for each platform/compiler combo, eg LINUXSERVERONLY, MSVC
# - target lists
# - object lists
# - linking instructions
# - compilation instructions
# - Python interface creation
# - clean
#
#################################################

include config

#################################################
# MINGW
#################################################

ifeq ($(MINGW),1)
C++FLAGS = -I/c/dev/osmpdevkit-mingw/tinyxml/newversion \
 -I/c/dev/osmpdevkit-mingw/mysql/include \
 -I/c/dev/osmpdevkit-mingw/logging/include \
 -I/c/dev/osmpdevkit-mingw/glut-3.7.6-bin \
 -I/c/dev/osmpdevkit-mingw/sdl/include \
 -I/c/dev/wxWidgets-2.5.2/include \
 -I/c/dev/CS/include \
 -I/c/dev/osmpdevkit-mingw/lua/include \
 -I/c/dev/osmpdevkit-mingw/pthreads-win32/include \
 -I/c/dev/osmpdevkit-mingw/ode/include \
 -DTIXML_USE_STL \
 -g
CCFLAGS = $(C++FLAGS)

Cc = gcc $(CCFLAGS) -c
C++ = g++ $(C++FLAGS) -c

OUT = -o
COMPILEOUT = -o
OBJSUFFIX = .o

LINKFLAGS = -L/c/dev/osmpdevkit-mingw/logging/lib \
   -L/c/dev/osmpdevkit-mingw/tinyxml/newversion \
   -L/c/dev/osmpdevkit-mingw/mysql/lib/opt \
   -L/c/dev/osmpdevkit-mingw/glut-3.7.6-bin \
   -L/c/dev/osmpdevkit-mingw/sdl/libmw \
   -L/c/dev/osmpdevkit-mingw/lua/lib \
   -L/c/dev/osmpdevkit-mingw/pthreads-win32/lib \
   -L/c/dev/wxWidgets-2.5.2/lib \
   -Xlinker --heap -Xlinker 8192 -Xlinker --stack -Xlinker 8192
LINKLIBS = -lws2_32 -lxmlmingw -lmysqlmingw -lglut32 -lopengl32 -lglu32 --lwx_mswd_core-2.5 -lwx_based-2.5 -lwxtiffd -lwxjpegd -lwxpngd -lwxzlibd -lwxregexd -lwxexpatd -lluamw -llualibmw -lpthreadVC -lrpcrt4 -loleaut32 -lole32 -luuid -lwinspool -lwinmm -lshell32 -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lgdi32 -lSDL
LINKER = g++ $(LINKFLAGS)

MAKEDLL = dllwrap

DLLNAME = -o
DEF = --def

OUTDIR = mingw/
EXESUFFIX = .exe

endif

ifeq ($(LINUXALL),1)
LINUX=1
endif

ifeq ($(LINUXSERVERONLY),1)
LINUX=1
endif

#################################################
# LINUX, pre-specific
#################################################

ifeq ($(LINUX),1)

DEVDIRPATH = ..
INITIATE = initiate-linux
INSTALLPHYSICSENGINE = installphysicsengine-linux

C++FLAGS = \
 $(ADDITIONALINCLUDEPATHS) \
 -I$(LOCALPREFIX)/include \
 -I$(LOCALPREFIX)/include/mysql \
 -I$(LOCALPREFIX)/include/python2.4 \
 -I/usr/include \
 -I/usr/include/mysql \
 -I/usr/include/python24 \
 -DTIXML_USE_STL \
 -DLOG_TIMESTAMP \
 -g -fPIC $(shell wx-config --cxxflags)
 
SWIGCOMMAND = swig
SWIGDEFINES =
OUT = -o
COMPILEOUT = -o
OBJSUFFIX = .o

LINKFLAGS = \
   $(ADDITIONALLIBPATHS) \
	-L$(LOCALPREFIX)/lib \
	-L/usr/lib \
   -L/usr/lib/SDL \
 $(shell wx-config --ldflags)
   
WXLIBS = -lwx_base-2.5  -lwx_base_net-2.5 -lwx_base_xml-2.5
LINKLIBS = -llaminarchaos -llogging -lmysqlclient -ltinyxml \
   -llua -llualib -lpthread -lode -ldl -lGLU -Lgl $(shell wx-config --libs)
   
DEFINE = -D
DLLNAME = -o
DEF = --def

OUTDIR = linux/
EXESUFFIX =
DSOLIBSUFFIX = .so
DLLPREFIX = lib
DLLSUFFIX = .so

RMCOMMAND = rm
CHDIRCOMMAND = cd

endif

#################################################
# LINUXSERVERONLY
#################################################

ifeq ($(LINUXSERVERONLY),1)
   
endif

#################################################
# LINUXALL
#################################################

ifeq ($(LINUXALL),1)

LINKLIBS = -ltartan -lglut -llaminarchaos -llogging -lmysqlclient -ltinyxml \
   -llua -llualib -lpthread -lode -ldl -lGLU -Lgl -lXmu $(shell wx-config --libs)
   
endif

#################################################
# LINUX, post-specific
#################################################

ifeq ($(LINUX),1)

CCFLAGS = $(C++FLAGS)
Cc = gcc $(CCFLAGS) -c
C++ = g++ $(C++FLAGS) -c

LINKER = g++ $(LINKFLAGS)
MAKEDLL = gcc -shared $(LINKFLAGS)

# $(LINKLIBS) is used directly in the target commands below,
# since it has to go at the end of the line

endif

#################################################
# CYGWIN, not really supported at this time
#################################################

ifeq ($(CYGWIN),1)
DRIVEPATH = /cygdrive/c

C++FLAGS = -I$(DRIVEPATH)/dev/osmpdevkit-cygwin/tinyxml \
 -I$(DRIVEPATH)/dev/osmpdevkit-cygwin/mysql/include \
 -I$(DRIVEPATH)/dev/osmpdevkit-cygwin/logging/include \
 -I$(DRIVEPATH)/dev/osmpdevkit-cygwin/sdl/include \
 -I$(DRIVEPATH)/dev/wxWidgets-2.5.2/include \
 -I$(DRIVEPATH)/dev/CS/include \
 -I$(DRIVEPATH)/dev/osmpdevkit-cygwin/lua/include \
 -I$(DRIVEPATH)/dev/osmpdevkit-cygwin/pthreads-win32/include \
 -I$(DRIVEPATH)/dev/osmpdevkit-cygwin/ode/include \
 -DTIXML_USE_STL \
 -g
CCFLAGS = $(C++FLAGS)
blah =  -mno-align-double

Cc = gcc $(CCFLAGS) -c
C++ = g++ $(C++FLAGS) -c

OUT = -o
COMPILEOUT = -o
OBJSUFFIX = .o

LINKFLAGS = -L/c/dev/osmpdevkit-cygwin/logging/lib \
   -L$(DRIVEPATH)/dev/osmpdevkit-cygwin/tinyxml \
   -L$(DRIVEPATH)/dev/osmpdevkit-cygwin/logging/lib \
   -L$(DRIVEPATH)/dev/osmpdevkit-cygwin/mysql/lib/opt \
   -L$(DRIVEPATH)/dev/osmpdevkit-cygwin/glut-3.7.6-bin \
   -L$(DRIVEPATH)/dev/osmpdevkit-cygwin/sdl/lib \
   -L$(DRIVEPATH)/dev/osmpdevkit-cygwin/lua/lib \
   -L$(DRIVEPATH)/dev/wxWidgets-2.5.2/lib \
   -Xlinker --heap -Xlinker 8192 -Xlinker --stack -Xlinker 8192
WXLIBS = -lwx_msw_core-2.5 -lwx_base-2.5 -lwxtiff -lwxjpeg -lwxpng -lwxzlib -lwxregex -lwxexpat
WIN32LIBS = -lrpcrt4 -luuid -lwinspool -lwinmm -lshell32 -lcomctl32 -lcomdlg32 -lctl3d32 -ladvapi32 -lwsock32 -lgdi32 \
   -loleaut32 -lole32
LINKLIBS = -lmysqlclient -lws2_32 -lxmlcyg -lglut32cyg -lopengl32 -lglu32 -lSDLmain -lSDL $(WXLIBS) \
   -llua -llualib $(WIN32LIBS)
LINKER = g++ $(LINKFLAGS)

MAKEDLL = dllwrap

DLLNAME = -o
DEF = --def

OUTDIR = cygwin/
EXESUFFIX = .exe

RMCOMMAND = rm

endif

#################################################
# Microsoft Visual C++ .Net 2003
#################################################

ifeq ($(MSVC),1)
INITIATE = initiate-windows
C++FLAGS = \
 /I"$(PLATFORMSDKINCLUDE)" \
 /I"$(STANDARDLIBRARYINCLUDE)" \
 /I"$(TINYXMLINCLUDE)" \
 /I"$(MYSQLINCLUDE)" \
 /I"$(LOGGINGINCLUDE)" \
 /I"$(ODEINCLUDE)" \
 /I"$(LAMINARCHAOSINCLUDE)" \
 /I"$(LUAINCLUDE)" \
 /I"$(GLUTINCLUDE)" \
 /I"$(SDLINCLUDE)" \
 /I"$(PYTHONINCLUDE)" \
 /I"$(WXWIDGETSINCLUDE)" \
 /I"$(TARTANINCLUDE)" \
 /I"$(PTHREADSINCLUDE)" \
 /DTIXML_USE_STL \
 /MD \
 /D_WIN32_WINNT=0x0500 /D "WIN32" /D "_WINDOWS"  \
 /nologo \
 /Zi /Od /EHsc /GR
CCFLAGS = $(C++FLAGS)

Cc = cl $(CCFLAGS) /c
C++ = cl $(C++FLAGS) /c

LINKFLAGS = \
  /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcp.lib /NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:msvcprtd.lib \
 /LIBPATH:"$(PLATFORMSDKLIB)" \
 /LIBPATH:"$(STANDARDLIBRARYLIB)" \
 /LIBPATH:"$(TINYXMLLIB)" \
 /LIBPATH:"$(LOGGINGLIB)" \
 /LIBPATH:"$(ODELIB)" \
 /LIBPATH:"$(MYSQLLIB)" \
 /LIBPATH:"$(LUALIB)" \
 /LIBPATH:"$(GLUTLIB)" \
 /LIBPATH:"$(LAMINARCHAOSLIB)" \
 /LIBPATH:"$(SDLLIB)" \
 /LIBPATH:"$(WXWIDGETSLIB)" \
 /LIBPATH:"$(TARTANLIB)" \
 /LIBPATH:"$(PYTHONLIB)" \
 /LIBPATH:"$(PTHREADSLIB)" \
 wsock32.lib gdi32.lib gdiplus.lib kernel32.lib shell32.lib user32.lib oleaut32.lib ole32.lib advapi32.lib version.lib \
 glu32.lib opengl32.lib \
 tinyxmlstl.lib \
 ode.lib \
 libmysql.lib \
 glut32.lib \
 SDL.lib \
 liblua.lib libaux.lib \
 pthreadvc.lib \
 libtartan.lib \
 liblaminarchaos.lib \
 liblogging.lib \
 wxmsw25_core.lib wxbase25.lib

# LINKER = link $(LINKFLAGS) /DEBUG 
LINKER = msvc\_link.bat /DEBUG 

SWIGCOMMAND = $(SWIGPATH)/swig
SWIGDEFINES = -D_WIN32
SWIGINCLUDES = -I"$(PLATFORMSDKINCLUDE)" -I"$(CPPRUNTIMEINCLUDE)" 

OUT = /OUT:
COMPILEOUT = /Fo
OBJSUFFIX = .obj

# MAKEDLL = link $(LINKFLAGS) /dll
MAKEDLL = msvc\_link.bat /dll

DEFINE = /D
LIBSUFFIX = .lib
DLLNAME = /out:
DEF = /def:
DSOLIBSUFFIX = .lib
DLLPREFIX =
DLLSUFFIX = .dll

OUTDIR = msvc/
EXESUFFIX = .exe

endif

###################################################################################3
# Target lists
##########################################################################

ALLSERVER = config serverfileagent AuthServerDatabaseManager metaverseserver databasemanager scriptingenginelua authserver OdePhysicsEngine

ALLOUTSTD = config metaverseserver databasemanager _osmpclient scriptingenginecppexample serverfileagent \
   clientfileagent scriptingenginelua OdePhysicsEngine authserver AuthServerDatabaseManager

ALLOUTWIN = setfocustowindow
ifeq ($(MSVC),1)
SDLMAIN = $(OUTDIR)SDL_win32_main$(OBJSUFFIX)
ALLOUT = $(ALLOUTSTD) $(ALLOUTWIN) OdePhysicsEngine
endif

ifeq ($(MINGW),1)
SDLMAIN = $(OUTDIR)SDL_win32_main$(OBJSUFFIX)
ALLOUT = $(ALLOUTSTD) $(ALLOUTWIN)
endif

ifeq ($(CYGWIN),1)
SDLMAIN =
ALLOUT = $(ALLOUTSTD) $(ALLOUTWIN)
endif

ifeq ($(LINUXSERVERONLY),1)
SDLMAIN =
ALLOUT = $(ALLSERVER)
endif

ifeq ($(LINUXALL),1)
SDLMAIN =
ALLOUT = $(ALLOUTSTD)
endif

all: $(INITIATE) $(ALLOUT) 
	

initiate-linux:	
	install -d linux

initiate-windows:	
	echo set CL=$(C++FLAGS) > $(OUTDIR)_cl.bat
	echo cl %%* >> $(OUTDIR)_cl.bat
	echo set LINK=$(LINKFLAGS) > $(OUTDIR)_link.bat
	echo link %%* >> $(OUTDIR)_link.bat

installphysicsengine-linux:
	install -d $(LOCALPREFIX)/lib
	install libodephysicsengine.so $(LOCALPREFIX)/lib


###############################################################################
# object lists
#############################################################################

MVWORLDSTORAGEOBJS = $(OUTDIR)Sphere$(OBJSUFFIX) $(OUTDIR)Cylinder$(OBJSUFFIX) $(OUTDIR)Cone$(OBJSUFFIX) \
  $(OUTDIR)Avatar$(OBJSUFFIX) $(OUTDIR)Cube$(OBJSUFFIX) \
  $(OUTDIR)Prim$(OBJSUFFIX) \
	$(OUTDIR)ObjectGrouping$(OBJSUFFIX) $(OUTDIR)Object$(OBJSUFFIX) $(OUTDIR)WorldStorage$(OBJSUFFIX) \
	$(OUTDIR)Terrain$(OBJSUFFIX) $(OUTDIR)Math$(OBJSUFFIX) $(OUTDIR)BasicTypes$(OBJSUFFIX) \
	$(OUTDIR)TextureInfoCache$(OBJSUFFIX) $(OUTDIR)TerrainInfoCache$(OBJSUFFIX) $(OUTDIR)MeshInfoCache$(OBJSUFFIX) \
	$(OUTDIR)FileInfoCache$(OBJSUFFIX) $(OUTDIR)Constants$(OBJSUFFIX) $(OUTDIR)XmlHelper$(OBJSUFFIX) \
	$(OUTDIR)Mesh$(OBJSUFFIX) $(OUTDIR)mvMd2Mesh$(OBJSUFFIX)

SCRIPTINGENGINEOBJS = $(OUTDIR)SocketsClass$(OBJSUFFIX) $(MVWORLDSTORAGEOBJS) \
	$(OUTDIR)TickCount$(OBJSUFFIX) $(OUTDIR)Checksum$(OBJSUFFIX) $(OUTDIR)File$(OBJSUFFIX) \
	$(OUTDIR)Animation$(OBJSUFFIX) $(OUTDIR)ScriptInfoCache$(OBJSUFFIX) $(OUTDIR)DiagConsole$(OBJSUFFIX)

SCRIPTINGENGINELUAOBJS = $(SCRIPTINGENGINEOBJS) $(OUTDIR)LuaScriptingAPI$(OBJSUFFIX) \
  $(OUTDIR)LuaScriptingAPIHelper$(OBJSUFFIX) $(OUTDIR)threadwrapper$(OBJSUFFIX) \
	$(OUTDIR)LuaScriptingAPIGod$(OBJSUFFIX) $(OUTDIR)LuaScriptingAPIGetObjectPropertiesStandard$(OBJSUFFIX) \
	$(OUTDIR)LuaScriptingAPISetObjectPropertiesStandard$(OBJSUFFIX) \
  $(OUTDIR)LuaScriptingStandardRPC$(OBJSUFFIX) $(OUTDIR)LuaScriptingPhysics$(OBJSUFFIX) \
  $(OUTDIR)LuaDBAccess$(OBJSUFFIX) $(OUTDIR)LuaMath$(OBJSUFFIX) $(OUTDIR)LuaScriptingAPITimerProperties$(OBJSUFFIX) \
  $(OUTDIR)LuaKeyboard$(OBJSUFFIX) $(OUTDIR)port_list$(OBJSUFFIX)

METAVERSECLIENTOBJS = $(OUTDIR)SocketsClass$(OBJSUFFIX) $(MVWORLDSTORAGEOBJS) \
  $(OUTDIR)Animation$(OBJSUFFIX) \
  $(OUTDIR)Editing3D$(OBJSUFFIX) $(OUTDIR)Checksum$(OBJSUFFIX) $(OUTDIR)ObjectImportExport$(OBJSUFFIX) \
  $(OUTDIR)RendererImplSdl$(OBJSUFFIX) $(OUTDIR)RendererTexturing$(OBJSUFFIX)\
  $(OUTDIR)ClientEditing$(OBJSUFFIX) $(OUTDIR)Selection$(OBJSUFFIX) \
  $(OUTDIR)File$(OBJSUFFIX) $(OUTDIR)Config$(OBJSUFFIX) $(OUTDIR)ScriptMgmt$(OBJSUFFIX) \
  $(OUTDIR)ClientTerrainFunctions$(OBJSUFFIX) $(OUTDIR)ClientLinking$(OBJSUFFIX) $(OUTDIR)PlayerMovement$(OBJSUFFIX) \
  $(OUTDIR)ClientFileMgmtFunctions$(OBJSUFFIX) $(OUTDIR)ClientMeshFileMgmt$(OBJSUFFIX) $(OUTDIR)Graphics$(OBJSUFFIX) \
  $(OUTDIR)TickCount$(OBJSUFFIX) $(OUTDIR)Camera$(OBJSUFFIX) \
  $(OUTDIR)Editing3DPos$(OBJSUFFIX) $(OUTDIR)Editing3DScale$(OBJSUFFIX) $(OUTDIR)Editing3DRot$(OBJSUFFIX) \
  $(OUTDIR)System$(OBJSUFFIX) $(OUTDIR)port_list$(OBJSUFFIX) \
  $(OUTDIR)DiagConsole$(OBJSUFFIX) \
  $(OUTDIR)osmpclient_wrap$(OBJSUFFIX)
#  $(OUTDIR)CollisionAndPhysicsDllLoader$(OBJSUFFIX) $(OUTDIR)DynamicDll$(OBJSUFFIX) \

DATABASEMANAGEROBJS = $(OUTDIR)SocketsClass$(OBJSUFFIX) $(MVWORLDSTORAGEOBJS) \
  $(OUTDIR)TickCount$(OBJSUFFIX) $(OUTDIR)Parse$(OBJSUFFIX)  \
  $(OUTDIR)ScriptInfoCache$(OBJSUFFIX) $(OUTDIR)MySQLDBInterface$(OBJSUFFIX) \
  $(OUTDIR)Config$(OBJSUFFIX)  $(OUTDIR)System$(OBJSUFFIX) $(OUTDIR)port_list$(OBJSUFFIX) \
  $(OUTDIR)DiagConsole$(OBJSUFFIX)

METAVERSESERVEROBJS = $(OUTDIR)SocketsClass$(OBJSUFFIX) $(MVWORLDSTORAGEOBJS) \
  $(OUTDIR)TickCount$(OBJSUFFIX) $(OUTDIR)Animation$(OBJSUFFIX) \
	$(OUTDIR)SocketsConnectionManager$(OBJSUFFIX) $(OUTDIR)Checksum$(OBJSUFFIX) $(OUTDIR)ScriptInfoCache$(OBJSUFFIX) \
	$(OUTDIR)Config$(OBJSUFFIX) $(OUTDIR)System$(OBJSUFFIX) $(OUTDIR)SpawnWrap$(OBJSUFFIX) $(OUTDIR)port_list$(OBJSUFFIX) \
	$(OUTDIR)DiagConsole$(OBJSUFFIX)
#	$(OUTDIR)CollisionAndPhysicsDllLoader$(OBJSUFFIX) $(OUTDIR)DynamicDll$(OBJSUFFIX) \

CLIENTFILEAGENTOBJS = $(OUTDIR)clientfileagent$(OBJSUFFIX) $(OUTDIR)DiagConsole$(OBJSUFFIX) \
   $(OUTDIR)SocketsClass$(OBJSUFFIX) $(OUTDIR)FileTrans$(OBJSUFFIX) $(OUTDIR)File$(OBJSUFFIX) \
   $(OUTDIR)port_list$(OBJSUFFIX) $(OUTDIR)Config$(OBJSUFFIX) $(OUTDIR)TickCount$(OBJSUFFIX)

AUTHSERVEROBJS = $(OUTDIR)Config$(OBJSUFFIX) $(OUTDIR)SocketsClass$(OBJSUFFIX) \
  $(OUTDIR)SocketsConnectionManager$(OBJSUFFIX) $(OUTDIR)DiagConsole$(OBJSUFFIX) \
  $(OUTDIR)SpawnWrap$(OBJSUFFIX) $(OUTDIR)System$(OBJSUFFIX) $(OUTDIR)port_list$(OBJSUFFIX) \
  $(OUTDIR)TickCount$(OBJSUFFIX)


ODEPHYSICSENGINEOBJS = $(OUTDIR)OdePhysicsEngine$(OBJSUFFIX) $(OUTDIR)Config$(OBJSUFFIX) \
   $(MVWORLDSTORAGEOBJS) 


OdePhysicsEngine:	libodephysicsengine$(DLLSUFFIX) _odephysicsengine$(DLLSUFFIX)
	

collisionandphysicsdllprot:	collisionandphysicsdllprot.dll
	
	
physicsdllprot:	physicsdllprot.dll
	
	
collisiondllprot:	collisiondllprot.dll
	
	
clientfileagent:	$(OUTDIR)clientfileagent$(EXESUFFIX)
	
	
scriptingenginecppexample:	$(OUTDIR)scriptingenginecppexample$(EXESUFFIX)
	

scriptingenginelua:	$(OUTDIR)scriptingenginelua$(EXESUFFIX)
	

metaverseserver:	$(OUTDIR)metaverseserver$(EXESUFFIX)
	

serverfileagent:	$(OUTDIR)serverfileagent$(EXESUFFIX)
	

AuthServerDatabaseManager:	$(OUTDIR)AuthServerDatabaseManager$(EXESUFFIX)
	

_osmpclient:	_osmpclient$(DLLSUFFIX)
	

renderermain:	$(OUTDIR)renderermain$(EXESUFFIX)
	

databasemanager:	$(OUTDIR)databasemanager$(EXESUFFIX)
	

authserver:	$(OUTDIR)authserver$(EXESUFFIX)
	

setfocustowindow:	$(OUTDIR)setfocustowindow$(EXESUFFIX)
	
mvsocketdriver:	$(OUTDIR)mvsocketdriver$(EXESUFFIX)

##############################################################################
# Linking instructions, for both executables and dsos/dlls
##############################################################################

$(OUTDIR)metaverseserver$(EXESUFFIX):	$(OUTDIR)metaverseserver$(OBJSUFFIX) \
      $(METAVERSESERVEROBJS) libodephysicsengine$(DLLSUFFIX)
	$(LINKER) $(OUTDIR)metaverseserver$(OBJSUFFIX) $(METAVERSESERVEROBJS)	\
	   $(OUT)$(OUTDIR)metaverseserver$(EXESUFFIX) libodephysicsengine$(DSOLIBSUFFIX) \
	      $(LINKLIBS)

_osmpclient$(DLLSUFFIX): $(OUTDIR)MetaverseClientPySlave$(OBJSUFFIX) $(METAVERSECLIENTOBJS) \
      libodephysicsengine$(DLLSUFFIX)
	$(MAKEDLL) $(OUT)_osmpclient$(DLLSUFFIX) $(OUTDIR)MetaverseClientPySlave$(OBJSUFFIX) \
	   $(METAVERSECLIENTOBJS) libodephysicsengine$(DSOLIBSUFFIX) $(LINKLIBS)

_odephysicsengine$(DLLSUFFIX): $(OUTDIR)OdePhysicsEngine_wrap$(OBJSUFFIX) \
      libodephysicsengine$(DLLSUFFIX) $(INSTALLPHYSICSENGINE)
	$(MAKEDLL) $(OUT)_odephysicsengine$(DLLSUFFIX) $(OUTDIR)OdePhysicsEngine_wrap$(OBJSUFFIX) \
	   libodephysicsengine$(DSOLIBSUFFIX) $(LINKLIBS)

ifeq ($(MSVC),1)
libodephysicsengine$(DLLSUFFIX): $(ODEPHYSICSENGINEOBJS) $(OUTDIR)DiagConsole$(OBJSUFFIX) 
	$(MAKEDLL) $(OUT)libodephysicsengine$(DLLSUFFIX) $(ODEPHYSICSENGINEOBJS) $(OUTDIR)DiagConsole$(OBJSUFFIX) $(LINKLIBS)
endif
ifeq ($(LINUX),1)
libodephysicsengine$(DLLSUFFIX): $(ODEPHYSICSENGINEOBJS)
	$(MAKEDLL) $(OUT)libodephysicsengine$(DLLSUFFIX) $(ODEPHYSICSENGINEOBJS) $(LINKLIBS)
endif

$(OUTDIR)testmvclientslave.exe: $(OUTDIR)testmvclientslave$(OBJSUFFIX) $(OUTDIR)MetaverseClientPySlave$(OBJSUFFIX) $(METAVERSECLIENTOBJS)
	$(LINKER) /out:$(OUTDIR)testmvclientslave.exe $(OUTDIR)testmvclientslave$(OBJSUFFIX) $(OUTDIR)MetaverseClientPySlave$(OBJSUFFIX) $(METAVERSECLIENTOBJS) $(LINKLIBS)

$(OUTDIR)databasemanager$(EXESUFFIX):	$(OUTDIR)DatabaseManager$(OBJSUFFIX) $(DATABASEMANAGEROBJS)
	$(LINKER) $(OUTDIR)DatabaseManager$(OBJSUFFIX) $(DATABASEMANAGEROBJS) $(OUT)$(OUTDIR)databasemanager$(EXESUFFIX) \
	    $(LINKLIBS)

$(OUTDIR)AuthServerDatabaseManager$(EXESUFFIX):	$(OUTDIR)AuthServerDatabaseManager$(OBJSUFFIX) $(OUTDIR)SocketsClass$(OBJSUFFIX) \
     $(OUTDIR)MySQLDBInterface$(OBJSUFFIX) $(OUTDIR)Config$(OBJSUFFIX) $(OUTDIR)DiagConsole$(OBJSUFFIX) $(OUTDIR)System$(OBJSUFFIX)
	$(LINKER) $(OUTDIR)AuthServerDatabaseManager$(OBJSUFFIX) $(OUTDIR)SocketsClass$(OBJSUFFIX) \
	   $(OUTDIR)MySQLDBInterface$(OBJSUFFIX) $(OUTDIR)Config$(OBJSUFFIX) $(OUTDIR)DiagConsole$(OBJSUFFIX) $(OUTDIR)TickCount$(OBJSUFFIX) $(OUTDIR)System$(OBJSUFFIX) \
	   $(OUT)$(OUTDIR)AuthServerDatabaseManager$(EXESUFFIX)  $(LINKLIBS)

$(OUTDIR)scriptingenginecppexample$(EXESUFFIX):	$(OUTDIR)scriptingenginecppexample$(OBJSUFFIX) $(SCRIPTINGENGINEOBJS) $(OUTDIR)port_list$(OBJSUFFIX)
	$(LINKER) $(OUTDIR)scriptingenginecppexample$(OBJSUFFIX) $(OUTDIR)port_list$(OBJSUFFIX) $(SCRIPTINGENGINEOBJS) $(OUT)$(OUTDIR)scriptingenginecppexample$(EXESUFFIX)  $(LINKLIBS)

$(OUTDIR)setfocustowindow$(EXESUFFIX):	$(OUTDIR)SetFocusToWindow$(OBJSUFFIX)
	$(LINKER) $(OUTDIR)SetFocusToWindow$(OBJSUFFIX) $(OUT)$(OUTDIR)setfocustowindow$(EXESUFFIX) $(LINKLIBS)

$(OUTDIR)serverfileagent$(EXESUFFIX):	$(OUTDIR)serverfileagent$(OBJSUFFIX) $(OUTDIR)DiagConsole$(OBJSUFFIX) \
     $(OUTDIR)SocketsClass$(OBJSUFFIX) $(OUTDIR)Checksum$(OBJSUFFIX) $(OUTDIR)System$(OBJSUFFIX)
	$(LINKER) $(OUTDIR)serverfileagent$(OBJSUFFIX) $(OUTDIR)DiagConsole$(OBJSUFFIX) $(OUTDIR)TickCount$(OBJSUFFIX) $(OUTDIR)SocketsClass$(OBJSUFFIX) $(OUTDIR)System$(OBJSUFFIX) \
	   $(OUTDIR)Checksum$(OBJSUFFIX) $(OUT)$(OUTDIR)serverfileagent$(EXESUFFIX) \
	    $(LINKLIBS)

$(OUTDIR)clientfileagent$(EXESUFFIX):  $(CLIENTFILEAGENTOBJS)
	$(LINKER) $(OUT)$(OUTDIR)clientfileagent$(EXESUFFIX) $(CLIENTFILEAGENTOBJS) $(LINKLIBS)

$(OUTDIR)scriptingenginelua$(EXESUFFIX):	$(OUTDIR)scriptingenginelua$(OBJSUFFIX) $(SCRIPTINGENGINELUAOBJS)
	$(LINKER) $(OUTDIR)scriptingenginelua$(OBJSUFFIX)  $(SCRIPTINGENGINELUAOBJS)	\
	   $(OUT)$(OUTDIR)scriptingenginelua$(EXESUFFIX) $(LINKLIBS)

$(OUTDIR)testobjectimport$(EXESUFFIX):	$(OUTDIR)testobjectimport$(OBJSUFFIX)  $(OUTDIR)DiagConsole$(OBJSUFFIX) \
     $(OUTDIR)SocketsClass$(OBJSUFFIX) \
     $(OUTDIR)MySQLDBInterface$(OBJSUFFIX) $(OUTDIR)Graphics$(OBJSUFFIX) $(OUTDIR)Avatar$(OBJSUFFIX) \
     $(OUTDIR)Cube$(OBJSUFFIX) $(OUTDIR)Prim$(OBJSUFFIX) $(OUTDIR)ObjectGrouping$(OBJSUFFIX) \
     $(OUTDIR)Object$(OBJSUFFIX) $(OUTDIR)WorldStorage$(OBJSUFFIX) $(OUTDIR)Constants$(OBJSUFFIX) \
     $(OUTDIR)BasicTypes$(OBJSUFFIX) $(OUTDIR)Math$(OBJSUFFIX) $(OUTDIR)TextureInfoCache$(OBJSUFFIX) \
     $(OUTDIR)Checksum$(OBJSUFFIX) $(OUTDIR)XmlHelper$(OBJSUFFIX) $(OUTDIR)ObjectImportExport$(OBJSUFFIX)
	$(LINKER) $(OUTDIR)testobjectimport$(OBJSUFFIX) $(OUTDIR)DiagConsole$(OBJSUFFIX) $(OUTDIR)TickCount$(OBJSUFFIX) $(OUTDIR)SocketsClass$(OBJSUFFIX) \
	   $(OUTDIR)dbabstractionlayer$(OBJSUFFIX) \
	   $(OUTDIR)Graphics$(OBJSUFFIX) $(OUTDIR)Avatar$(OBJSUFFIX) $(OUTDIR)Cube$(OBJSUFFIX) $(OUTDIR)Prim$(OBJSUFFIX) \
	   $(OUTDIR)ObjectGrouping$(OBJSUFFIX) $(OUTDIR)Object$(OBJSUFFIX) WorldStorage$(OBJSUFFIX) \
	   $(OUTDIR)Constants$(OBJSUFFIX) $(OUTDIR)BasicTypes$(OBJSUFFIX) $(OUTDIR)Math$(OBJSUFFIX) \
	   $(OUTDIR)TextureInfoCache$(OBJSUFFIX) $(OUTDIR)Checksum$(OBJSUFFIX) $(OUTDIR)XmlHelper$(OBJSUFFIX) \
	   $(OUTDIR)ObjectImportExport$(OBJSUFFIX) $(OUT)$(OUTDIR)testobjectimport$(EXESUFFIX) /STACK:4096 /HEAP:8192

$(OUTDIR)authserver$(EXESUFFIX):	$(OUTDIR)authserver$(OBJSUFFIX) $(AUTHSERVEROBJS)
	$(LINKER) $(OUT)$(OUTDIR)authserver$(EXESUFFIX) $(OUTDIR)authserver$(OBJSUFFIX) $(AUTHSERVEROBJS) $(LINKLIBS)

$(OUTDIR)CallCollisionAndPhysicsDllProt$(EXESUFFIX):	CallCollisionAndPhysicsDllProt$(OBJSUFFIX) DynamicDll$(OBJSUFFIX) CollisionAndPhysicsDllLoader$(OBJSUFFIX) $(MVWORLDSTORAGEOBJS)
	$(LINKER) $(OUT)CallCollisionAndPhysicsDllProt$(EXESUFFIX) CallCollisionAndPhysicsDllProt$(OBJSUFFIX) DynamicDll$(OBJSUFFIX) CollisionAndPhysicsDllLoader$(OBJSUFFIX) $(MVWORLDSTORAGEOBJS) \
	$(LINKLIBS)
	
$(OUTDIR)mvsocketdriver$(EXESUFFIX): $(OUTDIR)mvsocketdriver$(OBJSUFFIX) $(OUTDIR)SocketsClass$(OBJSUFFIX)
	$(LINKER) $(OUT)$(OUTDIR)mvsocketdriver$(EXESUFFIX) $(OUTDIR)mvsocketdriver$(OBJSUFFIX) $(OUTDIR)SocketsClass$(OBJSUFFIX) $(LINKLIBS)

##############################################################################
# Compilation instructions
##############################################################################

$(OUTDIR)OdePhysicsEngine$(OBJSUFFIX):  OdePhysicsEngine.cpp OdePhysicsEngine.h
	$(C++) OdePhysicsEngine.cpp $(DEFINE)BUILDING_PHYSICSENGINE $(COMPILEOUT)$@

$(OUTDIR)CallCollisionAndPhysicsDllProt$(OBJSUFFIX):	CallCollisionAndPhysicsDllProt.cpp
	$(C++) CallCollisionAndPhysicsDllProt.cpp $(COMPILEOUT)$@

$(OUTDIR)SetFocusToWindow$(OBJSUFFIX):	SetFocusToWindow.cpp
	$(C++) SetFocusToWindow.cpp $(COMPILEOUT)$@

$(OUTDIR)DynamicDll$(OBJSUFFIX):	DynamicDll.cpp DynamicDll.h
	$(C++) DynamicDll.cpp $(COMPILEOUT)$@

$(OUTDIR)CollisionAndPhysicsDllLoader$(OBJSUFFIX):	CollisionAndPhysicsDllLoader.cpp CollisionAndPhysicsDllLoader.h
	$(C++) CollisionAndPhysicsDllLoader.cpp $(COMPILEOUT)$@

$(OUTDIR)CollisionDllLoader$(OBJSUFFIX):	CollisionDllLoader.cpp CollisionDllLoader.h
	$(C++) CollisionDllLoader.cpp $(COMPILEOUT)$@

$(OUTDIR)PhysicsDllLoader$(OBJSUFFIX):	PhysicsDllLoader.cpp PhysicsDllLoader.h
	$(C++) PhysicsDllLoader.cpp $(COMPILEOUT)$@

$(OUTDIR)scriptingenginecppexample$(OBJSUFFIX):      scriptingenginecppexample.cpp Diag.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h WorldStorage.h SocketsClass.h IDBInterface.h TickCount.h port_list.h
	$(C++) scriptingenginecppexample.cpp $(COMPILEOUT)$@
	
$(OUTDIR)scriptingenginelua$(OBJSUFFIX):   scriptingenginelua.cpp Diag.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h WorldStorage.h SocketsClass.h IDBInterface.h TickCount.h
	$(C++) scriptingenginelua.cpp $(COMPILEOUT)$@
	
$(OUTDIR)ObjectImportExport$(OBJSUFFIX):	ObjectImportExport.cpp ObjectImportExport.h
	$(C++) ObjectImportExport.cpp $(COMPILEOUT)$@

$(OUTDIR)ClientEditing$(OBJSUFFIX):	ClientEditing.cpp ClientEditing.h
	$(C++) ClientEditing.cpp $(COMPILEOUT)$@

$(OUTDIR)RendererImpl$(OBJSUFFIX):	RendererImpl.cpp RendererImpl.h
	$(C++) RendererImpl.cpp $(COMPILEOUT)$@

$(OUTDIR)RendererImplSdl$(OBJSUFFIX):	RendererImplSdl.cpp RendererImpl.h
	$(C++) RendererImplSdl.cpp $(COMPILEOUT)$@

$(OUTDIR)FileInfoCache$(OBJSUFFIX):	FileInfoCache.cpp FileInfoCache.h
	$(C++) FileInfoCache.cpp $(COMPILEOUT)$@

$(OUTDIR)MeshInfoCache$(OBJSUFFIX):	MeshInfoCache.cpp MeshInfoCache.h
	$(C++) MeshInfoCache.cpp $(COMPILEOUT)$@

$(OUTDIR)ClientFileMgmtFunctions$(OBJSUFFIX):	ClientFileMgmtFunctions.cpp ClientFileMgmtFunctions.h
	$(C++) ClientFileMgmtFunctions.cpp $(COMPILEOUT)$@

$(OUTDIR)ClientMeshFileMgmt$(OBJSUFFIX):	ClientMeshFileMgmt.cpp ClientMeshFileMgmt.h
	$(C++) ClientMeshFileMgmt.cpp $(COMPILEOUT)$@

$(OUTDIR)RendererTexturing$(OBJSUFFIX):	RendererTexturing.cpp RendererTexturing.h
	$(C++) RendererTexturing.cpp $(COMPILEOUT)$@

$(OUTDIR)ClientLinking$(OBJSUFFIX):	ClientLinking.cpp ClientLinking.h
	$(C++) ClientLinking.cpp $(COMPILEOUT)$@

$(OUTDIR)ClientTerrainFunctions$(OBJSUFFIX):	ClientTerrainFunctions.cpp clientterrainfunctions.h
	$(C++) ClientTerrainFunctions.cpp $(COMPILEOUT)$@

$(OUTDIR)clientlogin$(OBJSUFFIX):	clientlogin.cpp clientlogin.h port_list.h
	$(C++) clientlogin.cpp $(COMPILEOUT)$@

$(OUTDIR)testobjectimport$(OBJSUFFIX):	testobjectimport.cpp
	$(C++) testobjectimport.cpp $(COMPILEOUT)$@

$(OUTDIR)authserver$(OBJSUFFIX):	authserver.cpp port_list.h
	$(C++) authserver.cpp $(COMPILEOUT)$@

$(OUTDIR)ScriptMgmt$(OBJSUFFIX):	ScriptMgmt.cpp
	$(C++) ScriptMgmt.cpp $(COMPILEOUT)$@

$(OUTDIR)PlayerMovement$(OBJSUFFIX):	PlayerMovement.cpp PlayerMovement.h
	$(C++) PlayerMovement.cpp $(COMPILEOUT)$@

$(OUTDIR)Camera$(OBJSUFFIX):	Camera.cpp Camera.h
	$(C++) Camera.cpp $(COMPILEOUT)$@

$(OUTDIR)threadwrapper$(OBJSUFFIX):	threadwrapper.cpp threadwrapper.h
	$(C++) threadwrapper.cpp $(COMPILEOUT)$@

$(OUTDIR)ScriptInfoCache$(OBJSUFFIX):	ScriptInfoCache.cpp
	$(C++) ScriptInfoCache.cpp $(COMPILEOUT)$@

$(OUTDIR)File$(OBJSUFFIX):	File.cpp File.h
	$(C++) File.cpp $(COMPILEOUT)$@
	
$(OUTDIR)Config$(OBJSUFFIX):	Config.cpp Config.h
	$(C++) Config.cpp $(COMPILEOUT)$@

$(OUTDIR)SDL_win32_main$(OBJSUFFIX):	SDL_win32_main.c
	$(C++) SDL_win32_main.c $(COMPILEOUT)$@

$(OUTDIR)serverfileagent$(OBJSUFFIX):	serverfileagent.cpp Diag.h SocketsClass.h
	$(C++) serverfileagent.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaScriptingAPI$(OBJSUFFIX):	LuaScriptingAPI.cpp LuaScriptingAPI.h
	$(C++) LuaScriptingAPI.cpp $(COMPILEOUT)$@

$(OUTDIR)SpawnWrap$(OBJSUFFIX):	SpawnWrap.cpp SpawnWrap.h
	$(C++) SpawnWrap.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaScriptingAPIGod$(OBJSUFFIX):	LuaScriptingAPIGod.cpp LuaScriptingAPIGod.h
	$(C++) LuaScriptingAPIGod.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaScriptingStandardRPC$(OBJSUFFIX):	LuaScriptingStandardRPC.cpp LuaScriptingStandardRPC.h
	$(C++) LuaScriptingStandardRPC.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaScriptingPhysics$(OBJSUFFIX):	LuaScriptingPhysics.cpp LuaScriptingPhysics.h
	$(C++) LuaScriptingPhysics.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaScriptingAPITimerProperties$(OBJSUFFIX):	LuaScriptingAPITimerProperties.cpp LuaScriptingAPITimerProperties.h
	$(C++) LuaScriptingAPITimerProperties.cpp $(COMPILEOUT)$@
	
$(OUTDIR)LuaKeyboard$(OBJSUFFIX):	LuaKeyboard.cpp LuaKeyboard.h
	$(C++) LuaKeyboard.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaDBAccess$(OBJSUFFIX):	LuaDBAccess.cpp LuaDBAccess.h
	$(C++) LuaDBAccess.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaMath$(OBJSUFFIX):	LuaMath.cpp LuaMath.h
	$(C++) LuaMath.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaScriptingAPIGetObjectPropertiesStandard$(OBJSUFFIX):	LuaScriptingAPIGetObjectPropertiesStandard.cpp LuaScriptingAPIGetObjectPropertiesStandard.h
	$(C++) LuaScriptingAPIGetObjectPropertiesStandard.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaScriptingAPISetObjectPropertiesStandard$(OBJSUFFIX):	LuaScriptingAPISetObjectPropertiesStandard.cpp LuaScriptingAPISetObjectPropertiesStandard.h
	$(C++) LuaScriptingAPISetObjectPropertiesStandard.cpp $(COMPILEOUT)$@

$(OUTDIR)LuaScriptingAPIHelper$(OBJSUFFIX):	LuaScriptingAPIHelper.cpp LuaScriptingAPIHelper.h
	$(C++) LuaScriptingAPIHelper.cpp $(COMPILEOUT)$@

$(OUTDIR)clientfileagent$(OBJSUFFIX):	clientfileagent.cpp Diag.h SocketsClass.h port_list.h
	$(C++) clientfileagent.cpp $(COMPILEOUT)$@

$(OUTDIR)FileTrans$(OBJSUFFIX):	FileTrans.cpp FileTrans.h SocketsClass.h
	$(C++) FileTrans.cpp $(COMPILEOUT)$@

$(OUTDIR)Checksum$(OBJSUFFIX):	Checksum.cpp Checksum.h
	$(C++) Checksum.cpp $(COMPILEOUT)$@

$(OUTDIR)XmlHelper$(OBJSUFFIX):	XmlHelper.cpp XmlHelper.h
	$(C++) XmlHelper.cpp $(COMPILEOUT)$@

$(OUTDIR)diagwindows$(OBJSUFFIX):	diagwindows.cpp Diag.h
	$(C++) diagwindows.cpp $(COMPILEOUT)$@

$(OUTDIR)TextureInfoCache$(OBJSUFFIX):	TextureInfoCache.cpp TextureInfoCache.h
	$(C++) TextureInfoCache.cpp $(COMPILEOUT)$@

$(OUTDIR)TerrainInfoCache$(OBJSUFFIX):	TerrainInfoCache.cpp TerrainInfoCache.h
	$(C++) TerrainInfoCache.cpp $(COMPILEOUT)$@

$(OUTDIR)Editing3D$(OBJSUFFIX):   Editing3D.cpp Editing3D.h WorldStorage.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h Animation.h Selection.h
	$(C++) Editing3D.cpp $(COMPILEOUT)$@
	
$(OUTDIR)Editing3DPos$(OBJSUFFIX):	Editing3DPos.cpp Editing3DPos.h WorldStorage.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h Animation.h Selection.h
	$(C++) Editing3DPos.cpp $(COMPILEOUT)$@
	
$(OUTDIR)Editing3DScale$(OBJSUFFIX):	Editing3DScale.cpp Editing3DScale.h WorldStorage.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h Animation.h Selection.h 
	$(C++) Editing3DScale.cpp $(COMPILEOUT)$@
	
$(OUTDIR)Editing3DRot$(OBJSUFFIX):	Editing3DRot.cpp Editing3DRot.h WorldStorage.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h Animation.h Selection.h
	$(C++) Editing3DRot.cpp $(COMPILEOUT)$@
	
$(OUTDIR)SocketsConnectionManager$(OBJSUFFIX):	SocketsConnectionManager.h SocketsConnectionManager.cpp SocketsClass.h Diag.h
	$(C++) SocketsConnectionManager.cpp $(COMPILEOUT)$@

$(OUTDIR)DatabaseManager$(OBJSUFFIX):	DatabaseManager.cpp Diag.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h WorldStorage.h SocketsClass.h GraphicsInterface.h MySQLDBInterface.h TickCount.h TextureInfoCache.h Parse.h
	$(C++) DatabaseManager.cpp $(COMPILEOUT)$@

$(OUTDIR)AuthServerDatabaseManager$(OBJSUFFIX):	AuthServerDatabaseManager.cpp Diag.h SocketsClass.h MySQLDBInterface.h
	$(C++) AuthServerDatabaseManager.cpp $(COMPILEOUT)$@

$(OUTDIR)metaverseserver$(OBJSUFFIX):	MetaverseServer.cpp Diag.h Object.h ObjectGrouping.h Avatar.h Prim.h  WorldStorage.h SocketsClass.h GraphicsInterface.h TickCount.h TextureInfoCache.h port_list.h
	$(C++) MetaverseServer.cpp $(COMPILEOUT)$@

$(OUTDIR)metaverseclient$(OBJSUFFIX):	MetaverseClient.cpp Diag.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h WorldStorage.h SocketsClass.h GraphicsInterface.h IDBInterface.h TickCount.h port_list.h
	$(C++) MetaverseClient.cpp $(COMPILEOUT)$@

$(OUTDIR)renderermain$(OBJSUFFIX):	renderermain.cpp RendererGlut.h Animation.h KeyAndMouse.h Selection.h WorldStorage.h SocketsClass.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h
	$(C++) renderermain.cpp $(COMPILEOUT)$@

$(OUTDIR)metaverseclientconsolidated$(OBJSUFFIX):	metaverseclientconsolidated.cpp RendererGlut.h Animation.h KeyAndMouse.h Selection.h WorldStorage.h SocketsClass.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h
	$(C++) metaverseclientconsolidated.cpp $(COMPILEOUT)$@

$(OUTDIR)MetaverseClientPySlave$(OBJSUFFIX):	MetaverseClientPySlave.cpp RendererGlut.h \
      Animation.h MetaverseClient.h \
      Selection.h WorldStorage.h SocketsClass.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h
	$(C++) MetaverseClientPySlave.cpp $(COMPILEOUT)$@

$(OUTDIR)DiagConsole$(OBJSUFFIX):	DiagConsole.cpp Diag.h
	$(C++) DiagConsole.cpp $(COMPILEOUT)$@

$(OUTDIR)System$(OBJSUFFIX):	System.cpp System.h
	$(C++) System.cpp $(COMPILEOUT)$@

$(OUTDIR)SocketsClass$(OBJSUFFIX):	SocketsClass.cpp SocketsClass.h
	$(C++) SocketsClass.cpp $(COMPILEOUT)$@

$(OUTDIR)Selection$(OBJSUFFIX):	Selection.cpp Selection.h RendererGlut.h Animation.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h WorldStorage.h
	$(C++) Selection.cpp $(COMPILEOUT)$@

$(OUTDIR)Parse$(OBJSUFFIX):	Parse.cpp Parse.h Diag.h
	$(C++) Parse.cpp $(COMPILEOUT)$@

$(OUTDIR)WorldStorage$(OBJSUFFIX):	WorldStorage.cpp WorldStorage.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h
	$(C++) WorldStorage.cpp $(COMPILEOUT)$@

$(OUTDIR)MySQLDBInterface$(OBJSUFFIX):	MySQLDBInterface.cpp MySQLDBInterface.h IDBInterface.h SocketsClass.h Diag.h
	$(C++) MySQLDBInterface.cpp $(COMPILEOUT)$@

$(OUTDIR)Graphics$(OBJSUFFIX):	Graphics.cpp Graphics.h GraphicsInterface.h Diag.h
	$(C++) Graphics.cpp $(COMPILEOUT)$@

$(OUTDIR)TickCount$(OBJSUFFIX):	TickCount.cpp TickCount.h
	$(C++) TickCount.cpp $(COMPILEOUT)$@

$(OUTDIR)Animation$(OBJSUFFIX):	Animation.cpp Animation.h Selection.h WorldStorage.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h Diag.h
	$(C++) Animation.cpp $(COMPILEOUT)$@

$(OUTDIR)KeyAndMouse$(OBJSUFFIX):	KeyAndMouse.cpp KeyAndMouse.h Animation.h Selection.h RendererGlut.h Math.h
	$(C++) KeyAndMouse.cpp $(COMPILEOUT)$@

$(OUTDIR)KeyAndMousesdl$(OBJSUFFIX):	KeyAndMousesdl.cpp KeyAndMouse.h Animation.h Selection.h RendererGlut.h Math.h
	$(C++) KeyAndMousesdl.cpp $(COMPILEOUT)$@

$(OUTDIR)Avatar$(OBJSUFFIX):	Avatar.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h Avatar.h Prim.h 
	$(C++) Avatar.cpp $(COMPILEOUT)$@

$(OUTDIR)Cube$(OBJSUFFIX):	Cube.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h Cube.h Prim.h 
	$(C++) Cube.cpp $(COMPILEOUT)$@

$(OUTDIR)Sphere$(OBJSUFFIX):	Sphere.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h Prim.h Sphere.h
	$(C++) Sphere.cpp $(COMPILEOUT)$@

$(OUTDIR)Terrain$(OBJSUFFIX):	Terrain.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h ObjectGrouping.h Prim.h 
	$(C++) Terrain.cpp $(COMPILEOUT)$@

$(OUTDIR)Cylinder$(OBJSUFFIX):	Cylinder.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h Prim.h Cylinder.h
	$(C++) Cylinder.cpp $(COMPILEOUT)$@

$(OUTDIR)Cone$(OBJSUFFIX):	Cone.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h Prim.h Cone.h 
	$(C++) Cone.cpp $(COMPILEOUT)$@

$(OUTDIR)Prim$(OBJSUFFIX):	Prim.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h Prim.h 
	$(C++) Prim.cpp $(COMPILEOUT)$@

$(OUTDIR)Mesh$(OBJSUFFIX):	Mesh.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h Prim.h 
	$(C++) Mesh.cpp $(COMPILEOUT)$@

$(OUTDIR)mvMd2Mesh$(OBJSUFFIX):	mvMd2Mesh.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h Prim.h 
	$(C++) mvMd2Mesh.cpp $(COMPILEOUT)$@

$(OUTDIR)ObjectGrouping$(OBJSUFFIX):	ObjectGrouping.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h ObjectGrouping.h 
	$(C++) ObjectGrouping.cpp $(COMPILEOUT)$@

$(OUTDIR)Object$(OBJSUFFIX):	Object.cpp IDBInterface.h SocketsClass.h GraphicsInterface.h TickCount.h Object.h 
	$(C++) Object.cpp $(COMPILEOUT)$@

$(OUTDIR)Constants$(OBJSUFFIX):	Constants.cpp Constants.h
	$(C++) Constants.cpp $(COMPILEOUT)$@
	
$(OUTDIR)Math$(OBJSUFFIX):	Math.cpp Math.h BasicTypes.cpp BasicTypes.h
	$(C++) Math.cpp $(COMPILEOUT)$@
	
$(OUTDIR)BasicTypes$(OBJSUFFIX): BasicTypes.cpp BasicTypes.h
	$(C++) BasicTypes.cpp $(COMPILEOUT)$@

$(OUTDIR)testmvclientslave$(OBJSUFFIX): testmvclientslave.cpp MetaverseClient.h
	$(C++) testmvclientslave.cpp $(COMPILEOUT)$@

$(OUTDIR)port_list$(OBJSUFFIX): port_list.cpp port_list.h
	$(C++) port_list.cpp $(COMPILEOUT)$@

$(OUTDIR)mvsocketdriver$(OBJSUFFIX): mvsocketdriver.cpp SocketsClass.h
	$(C++) mvsocketdriver.cpp $(COMPILEOUT)$@

#############################################################################
#
# EVERYTHING Python-interface based for the osmpclient module is in the following 
# make target (osmpclient_wrap.cxx)
# To add a new interface file:
# - add the corresponding .h file to the top of this target, with the other .h files
# - add the .i file to the bottom of this target, with the other .i files
# Please try to keep the layout of the .h files and .i files identical to 
# facilitate maintenance
# - add the .i file into osmpclient.i file
#
# There are lots of wrap files specified below.  
# The only wrap file that we are using is osmpclient_wrap.cxx
#
# The other _wrap files you see below are unimportant at this time.
# You can just ignore the other _wrap files; they are not used at all by the client
# at this time
#
####################################################################################

# run swig on the interface files:
osmpclient_wrap.cxx: OsmpClient.i Config.i \
      MetaverseClient.h mvtypemaps.i CallbackToPythonClass.h \
      \
      Selection.h RendererTexturing.h \
      BasicTypes.h WorldStorage.h \
      Selection.h Object.h ObjectGrouping.h Avatar.h Cube.h Prim.h Cone.h Sphere.h Cylinder.h Animation.h TextureInfoCache.h TerrainInfoCache.h \
      MeshInfoCache.h ClientEditing.h clientterrainfunctions.h ClientMeshFileMgmt.h \
      ClientFileMgmtFunctions.h ClientLinking.h ObjectImportExport.h \
      Math.h \
      RendererImpl.h Graphics.h GraphicsInterface.h ScriptMgmt.h PlayerMovement.h \
      Camera.h \
      Editing3D.h \
      \
      Selection.i RendererTexturing.i \
      BasicTypes.i WorldStorage.i \
      Selection.i ObjectStorage.i Animation.i TextureInfoCache.i TerrainInfoCache.i \
      MeshInfoCache.i ClientEditing.i clientterrainfunctions.i ClientMeshFileMgmt.i \
      ClientFileMgmtFunctions.i ClientLinking.i ObjectImportExport.i \
      Math.i
	$(SWIGCOMMAND) -python -c++ -w503,450 $(SWIGINCLUDES) $(SWIGDEFINES) -DBUILDING_PYTHONINTERFACES osmpclient.i

# compile the resulting omspclient_wrap.cxx file:
$(OUTDIR)osmpclient_wrap$(OBJSUFFIX): osmpclient_wrap.cxx
	$(C++) osmpclient_wrap.cxx $(COMPILEOUT)$@

OdePhysicsEngine_wrap.cxx: mvtypemaps.i \
      OdePhysicsEngine.h \
      \
      OdePhysicsEngine.i
	$(SWIGCOMMAND) -python -c++ $(SWIGINCLUDES) $(SWIGDEFINES) -DBUILDING_PYTHONINTERFACES OdePhysicsEngine.i

$(OUTDIR)OdePhysicsEngine_wrap$(OBJSUFFIX): OdePhysicsEngine_wrap.cxx
	$(C++) OdePhysicsEngine_wrap.cxx $(COMPILEOUT)$@

# Following wrap file build instructsion are not used at this time (just for testing
# sometimes)

BasicTypes_wrap.cxx: BasicTypes.i BasicTypes.h mvtypemaps.i
	$(SWIGCOMMAND) -python -c++ BasicTypes.i

WorldStorage_wrap.cxx: WorldStorage.i WorldStorage.h BasicTypes_wrap.cxx mvtypemaps.i
	$(SWIGCOMMAND) -python -c++ WorldStorage.i

$(OUTDIR)Config_wrap$(OBJSUFFIX): Config_wrap.cxx
	$(C++) Config_wrap.cxx $(COMPILEOUT)$@

Config_wrap.cxx: Config.i Config.h
	$(SWIGCOMMAND) -python -c++ Config.i

$(OUTDIR)BasicTypes_wrap$(OBJSUFFIX): BasicTypes_wrap.cxx
	$(C++) BasicTypes_wrap.cxx $(COMPILEOUT)$@

$(OUTDIR)WorldStorage_wrap$(OBJSUFFIX): WorldStorage_wrap.cxx
	$(C++) WorldStorage_wrap.cxx $(COMPILEOUT)$@

$(OUTDIR)CollisionAndPhysicsDllLoader_wrap$(OBJSUFFIX): CollisionAndPhysicsDllLoader_wrap.cxx
	$(C++) CollisionAndPhysicsDllLoader_wrap.cxx $(COMPILEOUT)$@

##############################################################################
# Clean.  Only works on Linux at the moment
##############################################################################

clean:	cleanobj cleanexe
		

cleanobj:	
	$(RMCOMMAND) $(OUTDIR)*$(OBJSUFFIX)

ifeq ($(LINUXALL),1)
cleanexe:
	$(CHDIRCOMMAND) $(OUTDIR) && $(RMCOMMAND) $(ALLOUT)
else
cleanexe:
	$(RMCOMMAND) $(OUTDIR)*$(EXESUFFIX)
endif

##############################################################################
# Config target.  So we can separate our own customization from what is in CVS
##############################################################################

config:  
	@echo *****************************************************
	@echo *****************************************************
	@echo **  
	@echo ** Please copy config.sample to config
	@echo ** and customize it for your build and machine
	@echo ** then rerun make
	@echo ** 
	@echo *****************************************************
	@echo *****************************************************
	@quit
