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
// ======================================================================================
//

//! \file
//! \brief mvbasictypes contains the definition of Rot, Vector3, Vector3 and Color

//! mvbasictypes contains the definition of Rot, Vector3, Vector3 and Color
//! and functions for manipulating them: converting to/from XML, printing to a stream

//
// ======================================================================================

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifndef _MVBASIC_TYPES_H
#define _MVBASIC_TYPES_H

#include <iostream>
using namespace std;

#include "tinyxml.h"
#include "Math.h"

typedef class Vector3 Vector3;
typedef class Vector3 Vector3;

//! Color contains a color, in r,g,b format; plus helper functions
class Color
{
private:
   char XMLAttributes[64];
	
public:
   float r;  //!< red value
   float g;  //!< green value
   float b;  //!< blue value
   
   Color(){r = g = b = 0; }
   //! constructs from passed in x,y,z
   Color( const float r, const float g, const float b )
   {
   	  this->r = r;
   	  this->g = g;
   	  this->b = b;
   }
   
   //! initializes from passed in string array, eg ["0.6", "0.2", "0.3"] in order r,g,b
   Color( const char *array[] )
   {
      r = atof( array[0] ); 
      g = atof( array[1] ); 
      b = atof( array[2] );
   }
   
  //! Copies to passed in color
   Color &operator=( const Color &targetcolor )
   {
	   r = targetcolor.r;
	   g = targetcolor.g;
	   b = targetcolor.b;
	   return *this;
  }
  
  //! Writes values to passed-in xml element, <someelement r="..." g="..." b="..."/>
  void WriteToXMLElement( TiXmlElement *p_Element );
  
  //! Reads values from passed in XML element in format <someelement r="..." g="..." b="..."/>
   Color( const TiXmlElement *p_Element );
   
   //! Writes values to xml string
  char *ToXMLAttributes();
  
  //! writes out to ostream
  friend ostream& operator<< ( ostream& os, const Color &color );
};

#endif // _MVBASIC_TYPES_H
