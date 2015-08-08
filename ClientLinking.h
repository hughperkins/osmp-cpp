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
//! \brief This module handles linking/unlinking of prims/objects for rendererglutsplit
//!
//! Things you can do with this file:\
//!
//! Call LinkSelectedObjectsToAvatar:
//!   - mvSelection should contain prims / objects you want to link to your avatar
//!   - after this call, they will now be linked to your avatar
//!
//! Call LinkSelectedObjects:
//!   - mvSelection should contain prims / objects you want to link together
//!   - after this call, they will be linked together
//!
//! Call UnlinkSelectedObject:
//!   - mvSelecdtion should contain an object you want to unlink
//!   - after this call, the object will be unlinked into its children
//!
//! Call RemoveFromLinkedSetGUIXML
//!   - you'll need to pass in XML element as follows:
//!     <someelement>
//!     <selection>
//!     <selected ireference="..."/>  (as many of these as you like
//!     </selection>
//!     </someelement>
//!   - After this call, all the prims/objects in the xml will be unlinked from their parent (?)

#ifndef _CLIENTLINKING_H
#define _CLIENTLINKING_H

//! \brief This namespace handles linking/unlinking of prims/objects for rendererglutsplit
//!
//! This namespace handles linking/unlinking of prims/objects for rendererglutsplit
//!
//! Things you can do with functions in this namespace:\
//!
//! Call LinkSelectedObjectsToAvatar:
//!   - mvSelection should contain prims / objects you want to link to your avatar
//!   - after this call, they will now be linked to your avatar
//!
//! Call LinkSelectedObjects:
//!   - mvSelection should contain prims / objects you want to link together
//!   - after this call, they will be linked together
//!
//! Call UnlinkSelectedObject:
//!   - mvSelecdtion should contain an object you want to unlink
//!   - after this call, the object will be unlinked into its children
//!
//! Call RemoveFromLinkedSetGUIXML
//!   - you'll need to pass in XML element as follows:
//!     <pre>
//!     <someelement>
//!     <selection>
//!     <selected ireference="..."/>  (as many of these as you like
//!     </selection>
//!     </someelement>
//!     </pre>
//!   - After this call, all the prims/objects in the xml will be unlinked from their parent (?)

#include <vector>
using namespace std;

class ClientLinkingClass
{
public:
   void LinkSelectedObjectsToAvatar();            //!< Links selected objects to avatar
   void LinkSelectedObjects();                     //!<  adds selected objects into a new ObjectGrouping group
   void RemoveFromLinkedSetRefList( vector<int> &RefList );         //!<  removes an object/prim from a linked set.  XML passed in is the XML received from GUI
//   void RemoveFromLinkedSetGUIXML( TiXmlElement *pElement );         //!<  removes an object/prim from a linked set.  XML passed in is the XML received from GUI
   void UnlinkSelectedObject();                      //!<  if the selected object is an ObjectGrouping, deletes it, leaving the child objects unlinked
protected:
};

#endif // _CLIENTLINKING_H
