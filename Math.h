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
//! \brief Contains quaternion and matrix maths
//!
//! Contains quaternion and matrix maths
//!
//! Quaternion maths: multiple, inverse, multiply on a vector, etc
//!
//! Quaternions are easy to invert (cf matrices), easy to interpolate (cf Euler, axis/angle)
//! lots of docs available on http://gamedev.net in section "articles"
//!
//! Reminder: sin, cos are in radians.  glRotatef is in degrees.  angle in AxisAngle here is in radians.

#ifndef _MVMATH_H
#define _MVMATH_H

// #include "BasicTypes.h"

#include <iostream>
using namespace std;

#include "tinyxml.h"

//! Rot contains a rotation, in x,y,z,s quaternion format; plus helper functions
class Rot
{
private:
   char XMLAttributes[64];
	
public:
   float x; //!< quaternion x
   float y; //!< quaternion y
   float z; //!< quaternion z
   float s; //!< quaternion s ("scalar")
   //! initializes from passed in string array, eg ["0","0","0","1"]
   Rot( const char *array[] )
   { 
      x = atof( array[0] ); 
      y = atof( array[1] ); 
      z = atof( array[2] ); 
      s = atof( array[3] ); 
   }
   
   //! default constructor, initialies to [0,0,0,1]
   Rot() { x = y = z = 0; s = 1; }
   
   //! constructs from passed-in x,y,z,s
   Rot( const float x, const float y, const float z, const float s )
   {
   	  this->x = x;
   	  this->y = y;
   	  this->z = z;
   	  this->s = s;
   }
  //! initializes from passed-in xml element, in format <someelement x="..." y="..." z="..." s="..."/>
   Rot( const TiXmlElement *p_Element );
   
  Rot &operator=( const Rot &rot )
   {
 	   x = rot.x;
  	   y = rot.y;
  	   z = rot.z;
  	   s = rot.s;
  	   return *this;
  }
   
   //! writes out to xml string in format <rot x="..." y="..." z="..." s="..."/>
  char *ToXMLAttributes();
  
   //! writes out to passed-in xml element in format <someelement x="..." y="..." z="..." s="..."/>
  void WriteToXMLElement( TiXmlElement *p_Element );
  
  // writes out to ostream 
   friend ostream& operator<< ( ostream& os, const Rot& rot );
};

//! Vector class
class Vector3
{
public:
	float x; //!< x
	float y; //!< y
	float z; //!< z
  friend ostream& operator<< ( ostream& os, const Vector3& vector );
  
  Vector3()
  {
  	x = 0;
  	y = 0;
  	z = 0;
  }
  Vector3 ( const TiXmlElement *p_Element ) //!< Initializes Vector3 from passed in XML element in format <something x="..." y="..." z="..."/>
  {
	  x = atof( p_Element->Attribute( "x" ) );
	  y = atof( p_Element->Attribute( "y" ) );
	  z = atof( p_Element->Attribute( "z" ) );
  }

 // Vector3( const class Vector3 &Pos );   //!< creates vector from pos
 // Vector3( const class Vector3 &Scale );  //!< creates vector from scale
  Vector3( const float x, const float y, const float z )  //!< creates vector from x,y,z
  {
  	this->x = x; this->y = y; this->z = z;
  }
  inline Vector3 operator- ( const Vector3 &V2 ) const;  //!< subtraction operator
  inline Vector3 operator+ ( const Vector3 &V2 ) const;  //!< addition operator
  inline Vector3 operator* ( const float f ) const;  
  inline Vector3 operator/ ( const float f ) const;  
  friend Vector3 operator* ( const float fMultiplier, const Vector3 &V );  
  
  inline Vector3 operator* ( const Rot &Rot ) const;
  
  //inline Vector3 &operator=( const Vector3 &V2 )
  //{
  //    *this = V2;
  //    return *this;
  //}
  
  void WriteToXMLElement( TiXmlElement *p_Element );  //!< writes out to passed-in xml element
};

inline Vector3 Vector3::operator- ( const Vector3 &V2 ) const
{
	 Vector3 result;
	 result.x = x - V2.x;
	 result.y = y - V2.y;
	 result.z = z - V2.z;
	 return result;
}

inline Vector3 Vector3::operator+ ( const Vector3 &V2 ) const
{
	 Vector3 result;
	 result.x = x + V2.x;
	 result.y = y + V2.y;
	 result.z = z + V2.z;
	 return result;
}

inline Vector3 operator* ( const float fMultiplier, const Vector3 &V )
{
	 Vector3 result;
	 result.x = V.x * fMultiplier;
	 result.y = V.y * fMultiplier;
	 result.z = V.z * fMultiplier;
	 return result;
}

inline Vector3 Vector3::operator* ( const float f ) const
{
	 Vector3 result;
	 result.x = x * f;
	 result.y = y * f;
	 result.z = z * f;
	 return result;
}

inline Vector3 Vector3::operator/ ( const float f ) const
{
	 Vector3 result;
	 result.x = x / f;
	 result.y = y / f;
	 result.z = z / f;
	 return result;
}

#ifndef BUILDING_PYTHONINTERFACES
const Vector3 XAXIS( 1.0, 0.0, 0.0 );  //!< unit vector along x axis
const Vector3 YAXIS( 0.0, 1.0, 0.0 );  //!< unit vector along y axis
const Vector3 ZAXIS( 0.0, 0.0, 1.0 );  //!< unit vector along z axis

const float piover180 = 0.0174532925f;
const float PI = 3.1415926535f;
const float TwoPI = 2.0*3.1415926535f;
#endif

void RotMultiply( class Rot &Qr, const class Rot &Q1, const class Rot &Q2 );             //!< multiplies two rots
float VectorMag( const Vector3 &V ); //!< returns magnitude of vector V
void VectorCross( Vector3 &vR, const Vector3 &v1, const Vector3 &v2 );  //!< vR is crossproduct of v1 and v2
float VectorDot( const Vector3 &v1, const Vector3 &v2 );  //!< returns dot producdt of v1 and v2
void RotBetween( class Rot &rResult, const Vector3 &Vector1, const Vector3 &Vector2 );  // returns rResult as rotation between vectors Vector1 and Vector2
void VectorNormalize( Vector3 &V );                          //!< normalizes V; V is both input and output
void AxisAngle2Rot( class Rot &Qr, const Vector3 &V, const float Theta );     //!< converts from axis/angle notation to quaternion rotation
void Rot2AxisAngle(Vector3 &Vr, float &Thetar, const class Rot &R );      //!< converst from quaternion rotation to axis/angle rotation
void AxisAngles2Rot(Rot &Qr, const Vector3 &V);
void InverseRot( class Rot &RInverted, const class Rot &R );                   //!< inverts a quaternion rotatoin
//void MultiplyVectorByRot( Vector3 &vResult, const class Rot &Rot, const Vector3 &vector  ); //!< Multiplies Vector by Rot, returning in VectorResult
//void MultiplyPosByRot( Vector3 &PosResult, const class Rot &Rot, const Vector3 &Pos  );  //!< Multiplies Pos by Rot, returning in PosResult
//void MultiplyScaleByRot( Vector3 &ScaleResult, const class Rot &Rot, const Vector3 &Scale );  //!< Multiplies Scale by Rot, and returns in ScaleResult
float SquareVectorDistance( const Vector3 &p1, const Vector3 &p2 );                //!< returns square of distance between Vector31 and Vector32 (using square saves execution time)
void SetZeroRot( class Rot &rot );                 //!< assigns ZERO_RotATIOn to Rot rot

inline Vector3 Vector3::operator* ( const Rot &rot ) const
{
   Vector3 vResult;

	 Rot InverseInRot;
	 InverseRot( InverseInRot, rot );
	 Rot VectorRot;
	 VectorRot.x = x;
	 VectorRot.y = y;
	 VectorRot.z = z;
	 VectorRot.s = 0;
	 Rot IntRot;
	 RotMultiply( IntRot, VectorRot, rot );
	 Rot ResultRot;
	 RotMultiply( ResultRot, InverseInRot, IntRot );
	 vResult.x = ResultRot.x;
	 vResult.y = ResultRot.y;
	 vResult.z = ResultRot.z;

   return vResult;  	
}

#endif // _MVMATH_H
