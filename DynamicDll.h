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
//! \brief DynamicDllClass handles runtime loading of .dlls or .sos

#ifndef _DYNAMICDLL_H
#define _DYNAMICDLL_H

#include <string>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#else
#define HINSTANCE void *
#endif

//! DynamicDllClass handles runtime loading of .dlls or .sos

//! DynamicDllClass handles runtime loading of .dlls or .sos
//! It's a baseclass; you'll need to derive from it for specific
//! dlls
class DynamicDllClass
{
public:
   HINSTANCE hdlDllLibrary;   //!< Holds handle to dll (or .so)

   DynamicDllClass(){ hdlDllLibrary = 0; }
   void LoadDll( string sDllName );  //!< Loads dll sDllName; do this before using functions
   void UnloadDll();  //!< Unload dll; do this at application shutdown (or if yo uwant to change dll)

   virtual void LoadFunctions();  //!< instantiate this in derived classes to load individual functions
protected:
   void *GetFunctionAddress( char *FunctionName );  //!< obtains the function address of the function FunctionName
};

#endif // _DYNAMICDLL_H
