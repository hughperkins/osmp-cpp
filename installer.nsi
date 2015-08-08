; Copyright Hugh Perkins 2004
;
;NSIS Modern User Interface version 1.70
;OSMP Installer Script

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  Name "The OpenSource Metaverse Project"
  OutFile "metaversesetup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\OSMP"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\OSMP" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "Licence.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "OSMP" osmp

  CreateDirectory "$DOCUMENTS\OSMP Inventory\Scripts"
  CreateDirectory "$DOCUMENTS\OSMP Inventory\Textures"
  CreateDirectory "$DOCUMENTS\OSMP Inventory\Meshes"
  CreateDirectory "$DOCUMENTS\OSMP Inventory\Objects"
  CreateDirectory "$DOCUMENTS\OSMP Inventory\Notecards"
  CreateDirectory "$DOCUMENTS\OSMP Inventory\Terrains"
  CreateDirectory "$DOCUMENTS\OSMP Inventory\Sims"
  
  CreateDirectory "$INSTDIR\Metaverse\clientdata\cache\textures"
  CreateDirectory "$INSTDIR\Metaverse\clientdata\cache\terrains"
  CreateDirectory "$INSTDIR\Metaverse\clientdata\cache\meshes"
  CreateDirectory "$INSTDIR\Metaverse\clientdata\cache\scripts"
  CreateDirectory "$INSTDIR\Metaverse\serverdata\textures"
  CreateDirectory "$INSTDIR\Metaverse\serverdata\scripts"
  CreateDirectory "$INSTDIR\Metaverse\serverdata\terrains"
  CreateDirectory "$INSTDIR\Metaverse\serverdata\meshfiles"

  SetOutPath "$INSTDIR\Metaverse\msvc"
  
  ;OSMP Files
  File msvc\authserver.exe
  File msvc\authserverdbinterface.exe

  File msvc\SetFocusToWindow.exe
  File msvc\metaverseserver.exe
  File msvc\dbinterface.exe
  File msvc\scriptingenginecppexample.exe
  File msvc\scriptingenginelua.exe
  File msvc\serverfileagent.exe
  File msvc\clientfileagent.exe
  
  SetOutPath "$INSTDIR\Metaverse"
  
  File libodephysicsengine.dll

  File _odephysicsengine.dll
  File _osmpclient.dll
  
  File config.xml.template
  File Licence.txt
  File *.py
  File *.bat
  File *.ico
  File osmpico32.bmp

  SetOutPath "$INSTDIR\DatabaseStuff"
  ; databasesetup stuff
  File ..\databasestuff\*.*

  ; sample lua script
  ; These are in cvs module "luascripts"
  SetOutPath "$DOCUMENTs\osmp inventory\scripts"
  File "..\luascripts\*.lua"
  
  SetOutPath "$DOCUMENTs\osmp inventory\meshes"
  File "*.md2"
  
  SetOutPath "$DOCUMENTs\osmp inventory\textures"
  File "*.pcx"
  File "*.tga"
  
  SetOutPath "$DOCUMENTs\osmp inventory\terrains"
  File "*.raw"
  
  SetOutPath "$INSTDIR"

  ;SetOutPath "$INSTDIR\dist"
  ;File dist\*.* commented out; we'll supply actual raw Python
  
  SetOutPath "$INSTDIR\Metaverse"

  ; runtime dlls for pthread, mysql and glut
  File ..\redist-mingandmsvc\*.*

  CreateDirectory "$SMPROGRAMS\OpenSource Metaverse Project"
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Authentication Server with MySQL (requires MySQL).lnk" "$INSTDIR\Metaverse\msvc\authserver.exe" "" "$INSTDIR\Metaverse\osmpico32.ico" 0
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Authentication Server without Database.lnk" "$INSTDIR\Metaverse\msvc\authserver.exe" --nodb "$INSTDIR\Metaverse\osmpico32.ico" 0
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Metaverse Server without Database (Needs AuthServer First).lnk" "$INSTDIR\Metaverse\msvc\metaverseserver.exe" --nodb "$INSTDIR\Metaverse\osmpico32.ico" 0
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Metaverse Server with MySQL (requires MySQL) (Needs Authserver first).lnk" "$INSTDIR\Metaverse\msvc\metaverseserver.exe" "" "$INSTDIR\Metaverse\osmpico32.ico" 0
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Start Lua Server-side Scripting Engine (start server first).lnk" "$INSTDIR\Metaverse\msvc\scriptingenginelua.exe" "" "$INSTDIR\Metaverse\osmpico32.ico" 0
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\MetaverseClient (needs server first).lnk" "$INSTDIR\Metaverse\startclient.bat" "" "$INSTDIR\Metaverse\osmpico32.ico" 0
  ;CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Scripting Engine Demo (start server and client first).lnk" "$INSTDIR\Metaverse\msvc\scriptingenginecppexample.exe" "" "$INSTDIR\Metaverse\osmpico32.ico" 0
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Open local inventory.lnk" "$DOCUMENTs\osmp inventory"
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\User instructions.lnk" "http://manageddreams.com/osmpwiki/index.php?title=User_Instructions"
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Open config.xml file.lnk" "notepad" "$INSTDIR\Metaverse\config.xml"
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Uninstall.lnk" "$INSTDIR\uninstall.exe"

  CreateDirectory "$SMPROGRAMS\OpenSource Metaverse Project\Sim admin"
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Sim admin\Snapshot Sim.lnk" "$INSTDIR\DatabaseStuff\RunSimSnapshot.bat" "" "$INSTDIR\Metaverse\osmpico32.ico" 0
  CreateShortCut "$SMPROGRAMS\OpenSource Metaverse Project\Sim admin\Restore Sim.lnk" "$INSTDIR\DatabaseStuff\RunSimSnapshot.bat" --restore "$INSTDIR\Metaverse\osmpico32.ico" 0

  Call SetupMetaverseDB
  
  ;Store installation folder
  WriteRegStr HKLM "Software\OSMP" "" $INSTDIR
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OSMP" "DisplayName" "The OpenSource Metaverse Project"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OSMP" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OSMP" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OSMP" "NoRepair" 1

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
   Exec "$INSTDIR\Metaverse\launchhelp.bat"

SectionEnd

Function SetupMetaverseDB
  ExecWait "$INSTDIR\Metaverse\configuremysqldatabases.bat"
FunctionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_osmp ${LANG_ENGLISH} "OSMP - Metaverse Client and Server components"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${osmp} $(DESC_osmp)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  ;Delete "$INSTDIR\Uninstall.exe"

  Delete "$INSTDIR\Metaverse\clientdata\cache\textures\*.*"
  RMDir "$INSTDIR\Metaverse\clientdata\cache\textures"
  
  Delete "$INSTDIR\Metaverse\clientdata\cache\terrains\*.*"
  RMDir "$INSTDIR\Metaverse\clientdata\cache\terrains"
  
  Delete "$INSTDIR\Metaverse\clientdata\cache\meshfiles\*.*"
  RMDir "$INSTDIR\Metaverse\clientdata\cache\meshfiles"
  
  Delete "$INSTDIR\Metaverse\clientdata\cache\scripts\*.*"
  RMDir "$INSTDIR\Metaverse\clientdata\cache\scripts"
  
  RMDir "$INSTDIR\Metaverse\clientdata\cache"
  RMDir "$INSTDIR\Metaverse\clientdata"

  Delete "$INSTDIR\Metaverse\serverdata\textures\*.*"
  RMDir "$INSTDIR\Metaverse\serverdata\textures"
  
  Delete "$INSTDIR\Metaverse\serverdata\terrains\*.*"
  RMDir "$INSTDIR\Metaverse\serverdata\terrains"
  
  Delete "$INSTDIR\Metaverse\serverdata\meshfiles\*.*"
  RMDir "$INSTDIR\Metaverse\serverdata\meshfiles"
  
  Delete "$INSTDIR\Metaverse\serverdata\scripts\*.*"
  RMDir "$INSTDIR\Metaverse\serverdata\scripts"
  RMDir "$INSTDIR\Metaverse\serverdata"

  Delete "$INSTDIR\Metaverse\*.*"
  RMDir "$INSTDIR\Metaverse"

  Delete "$INSTDIR\DatabaseStuff\*.*"
  RMDir "$INSTDIR\DatabaseStuff"

  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"

  Delete "$SMPROGRAMS\OpenSource Metaverse Project\*.*"
  RMDir "$SMPROGRAMS\OpenSource Metaverse Project"

  DeleteRegKey /ifempty HKLM "Software\OSMP"

SectionEnd