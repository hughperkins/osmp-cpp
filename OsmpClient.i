// Copyright Hugh Perkins 2005
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

%module(directors="1") osmpclient
%{
#include "MetaverseClient.h"
#include "RendererImpl.h"
#include "PlayerMovement.h"
#include "Editing3D.h"
#include "Camera.h"
#include "SDL_keysym.h"
%}

%import "typemaps.i"
%import "mvtypemaps.i"

//! /file
//! /brief metaverseclient.i swig interface file for metaverseclientpyslave.cpp

%feature("director") CallbackToPythonClass;

%include "CallbackToPythonClass.h"

%include "Config.i"

typedef Vector3 Vector3;
typedef Vector3 Vector3;

%include "BasicTypes.i"
%include "ObjectStorage.i"
%include "WorldStorage.i"

//%include "CollisionAndPhysicsDllLoader.i"
//%include "odephysicsengine.i"

%include "Selection.i"
%include "Animation.i"
%include "ClientEditing.i"
%include "ClientLinking.i"
%include "ObjectImportExport.i"

%include "ClientFileMgmtFunctions.i"
%include "ClientMeshFileMgmt.i"
%include "clientterrainfunctions.i"
%include "RendererTexturing.i"
%include "ScriptMgmt.h"

%include "RendererImpl.h"

%include "Math.i"

%include "Camera.h"

%include "SDL_keysym.h"
%include "PlayerMovement.h"
%include "Editing3D.h"

argoutfn( Vector3 )
%include "GraphicsInterface.h"
%include "Graphics.h"

%include "MetaverseClient.h"
