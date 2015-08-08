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
//! \brief mgraphics contains certain wrapper routines around OpenGL/GLUT
//! 
//! mgraphics contains certain wrapper routines around OpenGL/GLUT
//! for the moment, it contains routines to display text on the screen
//! and to rotate to a passed in quaternion (OpenGL expects axis/angle)
//!
//! Note that there is a public interface class for this: mvgraphicsinterface, which allows
//! link and include dependencies to be significantly reduced, eg with OpenGL.

#ifndef _MVGRAPHICS_H
#define _MVGRAPHICS_H

#ifdef _WIN32 // msvc and ming
#include <windows.h> 
#endif

#include <GL/gl.h> 
#include <GL/glu.h> 

#include <string.h>
#include <math.h>
#include <stdio.h>

#include "GraphicsInterface.h"
#include "BasicTypes.h"

//! mvGraphicsClass contains certain wrapper routines around OpenGL functions

//! mvGraphicsClass contains certain wrapper routines around OpenGL functions
//! for the moment, it contains routines to display text on the screen
//! and to rotate to a passed in quaternion (OpenGL expects axis/angle)
//!
//! Note that there is a public interface class for this: mvgraphicsinterface, which allows
//! link and include dependencies to be significantly reduced, eg with OpenGL.
class mvGraphicsClass : public mvGraphicsInterface
{
protected:
	int iNextTerrainListNumber;

public:
	virtual void printtext( char * string);                          //!< prints text within opengl 3d world at current translation
	virtual void screenprinttext(int x, int y, char * string);        //!< prints text on opengl screen at screenpos specified (in ortho mode)
  	virtual float GetScalingFrom3DToScreen( float fDepth );  //!< Determines the scaling between coordinates on the screen and distances in the world
  	       //!< at depth fDepth from screen
	virtual void RotateToRot( Rot &rot );                       //!< does a glRotatef according to passed in rot
  	virtual void SetColor( float r, float g, float b );  

	virtual void DrawWireframeBox( int iNumSlices );               //!< draws a wireframebox.  Different from glutWireBox, because draws a grid of lines on each
	                                                         //!< face for visibility.  Dimension of grid is iNumSlices                                                         
	virtual void DoCone();   //!< Draws a solid cone, in current texture
  	virtual void DoCube();  //!< Draws a solid cube, in current texture
 	virtual void DoCylinder();  //!< Draws a solid cylinder, in current texture
  	virtual void DoSphere();  //!< Draws a solid sphere, in current texture

   virtual void DoWireSphere();  //!< Draws a wire sphere

	virtual void RenderHeightMap(  unsigned char *g_HeightMap, int iMapSize );  //!< renders the terrain in g_HeightMap; which is of size iMapSize x iMapSize (each height is one byte)
   virtual void RenderTerrain(  unsigned char *g_HeightMap, int iMapSize );  //!< renders the terrain in g_HeightMap; which is of size iMapSize x iMapSize (each height is one byte)

  	virtual void DrawSquareXYPlane();                   //!< draws a 1x1 square in current x-y plane, centered on 0,0,0
  	virtual void DrawParallelSquares( int iNumSlices );   //!< draws parallel squares in current x-y plane, along the z-axis, from -0.5 to +0.5

  	virtual void GetMouseVector( Vector3 &rvMouseVector, const Vector3 OurPos, const Rot rOurRot, const int imousex, int imousey );  //!< Returns the vector corresponding to a mouse drag, in global world coordinates
	virtual void Translatef( float x, float y, float z );  //!< wraps glTranslatef
   virtual void Rotatef( float fAngleDegrees, float fX, float fY, float fZ);  //!< wraps glRotatef
	virtual void Scalef( float x, float y, float z );  //!< wraps glScalef
	virtual void Bind2DTexture( int iTextureID );	
	virtual void PopMatrix(); //!< wraps glPopMatrix
	virtual void PushMatrix();  //!< wraps glPushMatrix
   virtual void SetMaterialColor(float *mcolor);
   virtual void RasterPos3f(float x, float y, float z );
};

#endif // _MVGRAPHICS_H
