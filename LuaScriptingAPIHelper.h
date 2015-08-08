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

//! \file
//! \brief This module contains helper functions used by the Lua Scripting API functions
//!
//! This module contains helper functions used by the Lua Scripting API functions
//! Lua Scripting API Helper functions are never called directly from a Lua script and should not
//! handle mutexes as a rule.
//! argument validation and sanity checking should be handled by the actual Lua Scripting API functions -
//! called from the Lua scripts - and should not normally be essential within this LuaScriptingAPIHelper module
//!
//! Note: namespace LuaScriptingAPIHelper created for namespace encapsulation (useful to work out where a function is
//! that is being called).  All new functions should be added into this namespace.  We will migrate the others inside
//! at some point.
//!
//! Programming standards:
//! - None of these functions should be directly callable by Lua scripts
//! - None of these functions should lock or unlock mutexes

#include <string>
using namespace std;

#include "BasicTypes.h"
#include "Math.h"

// Note: old functions not in namespace; but be sure to add new ones into it
void TableToPos( lua_State *L, Vector3 &NewPos );            //!< converts passed in table with "x","y","z" keys into a Vector3
void TableToScale( lua_State *L, Vector3 &NewScale );   //!< converts passed in table with "x","y","z" keys into a Vector3
void TableToColor( lua_State *L, Color &NewColor );   //!< converts passed in table with "r","g","b" keys into a Color
void TableToRot( lua_State *L, Rot &NewRot );           //!< converts passed in table with "x","y","z","s" keys into a Rot
void TableToVector( lua_State *L, Vector3 &NewVector );   //!< converts passed in table with "x","y","z" keys into a Vector3
int GetReferenceFromVMRegistry( lua_State *L );   //!< Retrieves iReference of object, which we previously stored in VM's local registry
lua_State *GetVMForReference( int iReference );     //!< Retrieves a pointer to the VM correspondign to object iReference

//! \brief Functions available to the functions to Lua script-callable functions
namespace LuaScriptingAPIHelper  // Note: Add new functions to this namespace
{
    string GetTypename( lua_State *L, int iStackPos );                         //!< returns the typename of the value at stack position stackpos
    int SwapParams(lua_State *L, lua_State *pluaVM);  // swaps stacks for two VM's
    void SayFromObject( int iObjectReference, string sMessage );               //!< sends a Say in the sim from the object iObjectReference
    void AddObjectReferenceToVMRegistry( int iObjectReference, lua_State *pluaVM );        //!<  stores the value iObjectReference in the VM registry of the passed in VM, for later retrieval by GetReferenceFromVMRegistry within each Lua-called function call
    void DumpVMRegistry( lua_State *pluaVM );      //!< diagnostic tool: dumps the contents of the VM registry of the passed-in VM
    bool DoPCall(lua_State *L, int nargs, int nresults, int errfunc); //!< error handled pcall

    void PushRotAsTable( lua_State *L, const Rot &rot );  //!< Pushes a rot onto the stack of VM L, as a table
    void PushVectorAsTable( lua_State *L, const Vector3 &vector );  //!< Pushes a vector onto the stack of VM L, as a table
}
