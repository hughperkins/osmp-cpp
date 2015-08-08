// Copyright Hugh Perkins 2004
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURVector3E. See the GNU General Public License for
//  more details.
//
// You should have received a copy of the GNU General Public License along
// with this program in the file licence.txt; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-
// 1307 USA
// You can find the licence also on the web at:
// http://www.opensource.org/licenses/gpl-license.php
//

//! /file
//! /brief MetaverseClient.h header file for metaverseclientconsolidated.cpp

// see renderer .cpp file for documentation

#include <string>
using namespace std;

#include "tinyxml.h"

#include "CallbackToPythonClass.h"

#include "Config.h"
#include "WorldStorage.h"
#include "Selection.h"
#include "Animation.h"
#include "ClientEditing.h"
#include "TerrainInfoCache.h"
#include "MeshInfoCache.h"
#include "RendererTexturing.h"
#include "ClientTerrainFunctions.h"
#include "ClientMeshFileMgmt.h"
#include "ClientLinking.h"
#include "PlayerMovement.h"
#include "Editing3D.h"
#include "ObjectImportExport.h"
#include "OdePhysicsEngine.h"
#include "Graphics.h"
#include "ScriptMgmt.h"
#include "Camera.h"

void SetCallbackToPython( CallbackToPythonClass &callbackobject );
void InitMetaverseClient( CallbackToPythonClass &callbackobject );
void SetSimIPAddress( const char *SimIPAddress, const int SimPort );

void SpawnFileAgent();

void MetaverseclientStartRenderer();
void MetaverseclientShutdown();

void SetAvatariReference( int AvatariReference );

ConfigClass &GetmvConfig();

mvWorldStorage &GetWorld();
mvSelection &GetSelector();
Animation &GetAnimator();

TextureInfoCache &GetTextureInfoCache();
TerrainCacheClass &GetTerrainCache();
MeshInfoCacheClass &GetMeshInfoCache();
ClientsideEditingClass &GetEditorMgr();
ClientLinkingClass &GetClientLinking();
ObjectImportExportClass &GetObjectImportExport();

RendererTexturingClass &GetRendererTexturing();
ClientTerrainFunctionsClass &GetClientTerrainFunctions();
ClientMeshFileMgmtClass &GetMeshFileMgmt();
ScriptMgmtClass &GetScriptMgmt();

CollisionAndPhysicsEngineClass &GetCollisionAndPhysicsEngine();
mvGraphicsClass &GetmvGraphics();

PlayerMovementClass &GetPlayerMovement();
Editing3DClass &GetEditing3D();

mvCameraClass &GetCamera();

//void SetWorldStorage( mvWorldStorage &World );

namespace MetaverseClient
{
	void SendToServer( const char *sMessage );
	//void DisplayContextMenu( const int mousex, const int mousey );
	//void SendClickEventToServer( const int mousex, const int mousey );
  //void DisplayHelp();
  //void DeleteCurrentSelection();
  //void ActivateChat();
  	void ObjectImportMessageCallback( string message );
	void SendXMLDocToFileAgent( TiXmlDocument &IPC );
	extern CallbackToPythonClass *pCallbackToPython;
	void SendXMLStringToFileAgent( const char *Message );
}
