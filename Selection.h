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
//! \brief mvSelection handles object selection within the 3d renderer environment
//! 
//! mvSelection handles object selection within the 3d renderer environment
//! It uses OpenGL's Selection mode to determine which objects you click on
//! The current selection is stored in the map SelectedObjects
//!
//! Functions are available to help manipulate and search this map

// documentaiton on maps available in http://msdn.microsoft.com/library

#ifndef _MVSELECTION_H
#define _MVSELECTION_H

#include <map>
#include <vector>
using namespace std;

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

//! stores coordinates of a hardcoded bit of green plateau
struct LANDCOORD
{
    float x;
    float y;
};

//! stores the reference of a single selected object/prim
struct SELECTION
{
	 int iReference;  //!< iReference number of object (globally unique number within prim/server/database)
	// int iObjectArrayNum;
};

//! Used by HITTARGET structure to define what type of hittarget this is
enum enumHitTargetType
{
	 HITTARGETTYPE_LANDCOORD,  // land coordinate, (bit of hardcoded green platform)
	 HITTARGETTYPE_OBJECT,  // object, ie prim, avatar etc
	 HITTARGETTYPE_EDITHANDLE,  // editing handle; doesnt really exist in world, just appears whilst editing
	 HITTARGETTYPE_TERRAIN   // terrain object
};

//! Holds one hittarget, ie something that can be clicked, eg an object reference
struct HITTARGET
{
	 //bool bIsObject;
	 enumHitTargetType TargetType;  //!< target type, ie HITTARGETTYPE_LANDCOORD, HITTARGETTYPE_OBJECT etc
	 int iForeignReference;  //!< reference number according to type.  For HITTARGETTYPE_OBJECT, this would be iReference of the object
};

//! mvSelection handles object selection within the 3d renderer environment

//! mvSelection handles object selection within the 3d renderer environment
//! It uses OpenGL's Selection mode to determine which objects you click on
//! The current selection is stored in the map SelectedObjects
//!
//! Functions are available to help manipulate and search this map
class mvSelection
{
protected:
   void PurgeThingsThatShouldntBeHere();  //!< remove objects/prims from selection if they no longer exist

public:
   LANDCOORD LandCoords[1000];  //!< coordinates of hardcoded green plateau (?)
   HITTARGET HitTargets[ 1000 ];  //!< array of potential hittargets

   map <int, SELECTION, less<int> > SelectedObjects;             //!< currently selected objects/prims
   map <int, SELECTION, less<int> >::iterator SelectedObjectIterator;
   //SELECTION SelectedObjects[10];
   //int iNumSelected;

   bool bSendObjectMoveForSelection;   //!< Used by 3d editing to notify selection that the current selection has moved and should be replicated to server
   int iTargetReference;

   mvSelection()
   {
   	   //iNumSelected = 0;
   	   iTargetReference = 0;
   	   bSendObjectMoveForSelection = false;
   	   SelectedObjects.clear();
   }

   int GetNumSelected();                              //!< get number of objects selected
   vector<int> GetSelectedObjectReferences();
   bool IsSelected( int iReference );                  //!< is anything selected?
   void RemoveFromSelectedObjects( int iReference );    //!< removes object with iREference=iREference from selection
   // int FindSelectionForReference( int iReference );      //!< gets the key to the selected object with iReference = iReference
   
   int GetClickedTopLevelObjectReference( int iMouseX, int iMouseY );   //!< gets iReference of object clicked on.  Mouse x/y coordinates passed in.
   int GetClickedPrimReference( int iMouseX, int iMouseY );   //!< gets iReference of object clicked on.  Mouse x/y coordinates passed in.

   void ToggleObjectInSelection( int iReference, bool bSelectParentObject );     //!< toggles object in selection.  If bSelectParentObject, goes up object hierarchy to find toplevel parent and select/deselect that
   void ToggleClickedInSelection( bool bSelectParentObject, int iMouseX, int iMouseY );    //!< toggle a clicked object selected/not selected; mouse x/y coordinates passed in
   HITTARGET *GetClickedHitTarget( int iMouseX, int iMouseY );    //!< HITTARGET gives information on a clicked object

   bool IsPartOfAvatar( int iReference );           //!< retursn true/false if passed in object reference is part of an avatar
   void Clear();              //!< clears the cache (unselects everything)
   void AddHitTarget( enumHitTargetType TargetType, int iForeignReference );
   void ResetNames() { iTargetReference = 0; }
};

typedef map <int, SELECTION, less<int> >::iterator SelectionIteratorTypedef;    //!< use this typedef to create iterator objects for the SelectedObjects map

#endif // _MVSELECTION_H
