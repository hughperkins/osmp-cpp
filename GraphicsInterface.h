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
//! \brief mvGraphicsInterface is a pure virtual interface class for mvGraphics

#ifndef _MVGRAPHICSINTERFACE_H
#define _MVGRAPHICSINTERFACE_H

#include <string.h>

#include "BasicTypes.h"

//! mvGraphicsInterface is a pure virtual interface class for mvGraphics

//! mgraphics contains certain wrapper routines around OpenGL/GLUT
//! for the moment, it contains routines to display text on the screen
//! and to rotate to a passed in quaternion (OpenGL expects axis/angle)
//!
//! This is the abstract pure virtual interface class, to reduce link dependencies
//!
//! See mvGraphics (mvGraphics.h) for documentation
class mvGraphicsInterface
{
public:
   virtual void printtext( char * string) = 0;
   virtual void screenprinttext(int x, int y, char * string) = 0;
   virtual float GetScalingFrom3DToScreen( float fDepth ) = 0;
   virtual void RotateToRot( Rot &rot ) = 0;
   virtual void SetColor( float r, float g, float b ) = 0;
   
   virtual void DrawWireframeBox( int iNumSlices ) = 0;
	 virtual void DoCone() = 0;
	 virtual void DoCube() = 0;
     virtual void DoSphere() = 0;
 virtual void DoCylinder() = 0;

virtual void DoWireSphere() = 0;

   virtual void RenderHeightMap(  unsigned char *g_HeightMap, int iMapSize ) = 0;
   virtual void RenderTerrain(  unsigned char *g_HeightMap, int iMapSize ) = 0;

   virtual void DrawSquareXYPlane() = 0;
   virtual void DrawParallelSquares( int iNumSlices ) = 0;
	virtual void Translatef( float x, float y, float z ) = 0;
   virtual void Rotatef( float fAngleDegrees, float fX, float fY, float fZ) = 0;
    virtual void Scalef( float x, float y, float z ) = 0;
	virtual void Bind2DTexture( int iTextureID ) = 0;
	virtual void PopMatrix() = 0;
	virtual void PushMatrix() = 0;
    virtual void SetMaterialColor(float *mcolor) = 0;
    virtual void RasterPos3f(float x, float y, float z ) = 0;
};

#endif // _MVGRAPHICSINTERFACE_H
