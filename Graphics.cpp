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
// See header for documentation

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <string.h>
#include <math.h>
#include <stdio.h>

#include "Graphics.h"
#include "BasicTypes.h"
#include "Math.h"
#include "Constants.h"
#include "RendererImpl.h"

#define LISTCube 1
#define LISTSphere 2
#define LISTCylinder 3
#define LISTCone 4

bool bCubeDefined = false;
bool bConeDefined = false;
bool bCylinderDefined = false;
bool bSphereDefined = false;

#define WIDTH 800
#define HEIGHT 600

//! 2d position, used in mvgraphics.cpp
struct Pos2D
{
    float x;
    float y;
};

//! Feedback point buffer for OpenGL feedback, used by mvgraphics.cpp
struct FeedbackPointBufferStruct
{
    float type;
    float x;
    float y;
};

//! Feedback line buffer for OpenGL feedback, used by mvgraphics.cpp
struct FeedbackLineBufferStruct
{
    float type;
    Pos2D Vertices[2];
};

void mvGraphicsClass::GetMouseVector( Vector3 &rvMouseVector, const Vector3 OurPos, const Rot rOurRot, const int imousex, int imousey )
{

    //float FeedbackBuffer[9];
    FeedbackPointBufferStruct FeedbackBuffer[2];
    glFeedbackBuffer( 2 * sizeof( FeedbackBuffer ), GL_2D, (GLfloat *)&FeedbackBuffer );

    GLint viewport[4];
    // This Sets The Array <viewport> To The Size And Location Of The Screen Relative To The Window
    glGetIntegerv(GL_VIEWPORT, viewport);

    glPushMatrix();

    glRenderMode( GL_FEEDBACK );

    glLoadIdentity();

    // rotate so z axis is up, and x axis is forward
    glRotatef( 90, 0.0, 0.0, 1.0 );
    glRotatef( 90, 0.0, 1.0, 0.0 );

    glTranslatef( 10.0f, 0.0, 0.0 );

    glBegin( GL_POINTS );
    glVertex3f( 0.0, 0.0, 0.0 );
    glVertex3f( 0.0, -1.0, 0.0 );
    glEnd();

    DEBUG(  "number of points: " << glRenderMode( GL_RENDER ) ); // DEBUG

    DEBUG(  FeedbackBuffer[0].type ); // DEBUG

    glPopMatrix();

    DEBUG(  "Screencoords of input vertex: " << FeedbackBuffer[0].x << " " << FeedbackBuffer[0].y ); // DEBUG
    DEBUG(  "Screencoords of input vertex: " << FeedbackBuffer[1].x << " " << FeedbackBuffer[1].y ); // DEBUG

    Vector3 ScreenPosPointOne;
    ScreenPosPointOne.x = 0;
    ScreenPosPointOne.y = RendererImpl::GetWindowWidth() - FeedbackBuffer[0].x;
    ScreenPosPointOne.z = FeedbackBuffer[0].y;

    Vector3 ScreenPosPointTwo;
    ScreenPosPointTwo.x = 0;
    ScreenPosPointTwo.y = RendererImpl::GetWindowWidth() - FeedbackBuffer[1].x;
    ScreenPosPointTwo.z = FeedbackBuffer[1].y;

    Vector3 ScreenMousePos( 0, RendererImpl::GetWindowWidth() - imousex, RendererImpl::GetWindowHeight() - imousey );
    DEBUG(  "posone " << ScreenPosPointOne << " postwo " << ScreenPosPointTwo ); // DEBUG

    Vector3 MouseVectorOnScreenScreenScreenCoords( ScreenMousePos - ScreenPosPointOne );
    DEBUG(  " MouseVectorOnScreenScreenScreenCoords " << MouseVectorOnScreenScreenScreenCoords ); // DEBUG

    float fScalingFromWorldToScreen = ( (float)ScreenPosPointOne.y -  (float)ScreenPosPointTwo.y );
    DEBUG(  "fScalingFromWorldToScreen " << fScalingFromWorldToScreen ); // DEBUG
    DEBUG(  " GetScalingFrom3DToScreen " << GetScalingFrom3DToScreen( 10.0f ) ); // DEBUG

    Vector3 MouseVectorRayWorldCoordsAvatarAxes;
    MouseVectorRayWorldCoordsAvatarAxes.y = MouseVectorOnScreenScreenScreenCoords.y * ( 1.0f / fScalingFromWorldToScreen );
    MouseVectorRayWorldCoordsAvatarAxes.z = MouseVectorOnScreenScreenScreenCoords.z * ( 1.0f / fScalingFromWorldToScreen );
    MouseVectorRayWorldCoordsAvatarAxes.x = 10.0f;
    DEBUG(  "MouseVectorRayWorldCoordsAvatarAxes " << MouseVectorRayWorldCoordsAvatarAxes  ); // DEBUG

    Rot rInverseOurRot;
    InverseRot( rInverseOurRot, rOurRot );

    Vector3 MouseVectorRayWorldCoordsWorldAxes;
    MouseVectorRayWorldCoordsWorldAxes = MouseVectorRayWorldCoordsAvatarAxes * rInverseOurRot;
    // MultiplyVectorByRot( MouseVectorRayWorldCoordsWorldAxes, rInverseOurRot, MouseVectorRayWorldCoordsAvatarAxes );
    DEBUG(  " MouseVectorRayWorldCoordsWorldAxes " << MouseVectorRayWorldCoordsWorldAxes ); // DEBUG

    rvMouseVector = MouseVectorRayWorldCoordsWorldAxes;
}

float mvGraphicsClass::GetScalingFrom3DToScreen( float fDepth )
{
    FeedbackLineBufferStruct FeedbackBuffer;

    GLint viewport[4];
    // This Sets The Array <viewport> To The Size And Location Of The Screen Relative To The Window
    glGetIntegerv(GL_VIEWPORT, viewport);

    glLoadIdentity();

    // rotate so z axis is up, and x axis is forward
    glRotatef( 90, 0.0, 0.0, 1.0 );
    glRotatef( 90, 0.0, 1.0, 0.0 );

    glFeedbackBuffer( sizeof( FeedbackLineBufferStruct ), GL_2D, (GLfloat *)&FeedbackBuffer );
    glRenderMode( GL_FEEDBACK );
    glPushMatrix();
    glTranslatef( fDepth, 0, 0 );

    glBegin( GL_LINES );
    glVertex3f( 0, -0.5, 0 );
    glVertex3f( 0, 0.5, 0 );
    glEnd();

    glPopMatrix();
    int iNumValues = glRenderMode( GL_RENDER );

    //DEBUG(  "Screencoords of test vertices: " << FeedbackBuffer[0].x << " " << FeedbackBuffer[0].y << " " << FeedbackBuffer[1].x << " " << FeedbackBuffer[1].y ); // DEBUG

    int DeltaX = (int)FeedbackBuffer.Vertices[1].x - (int)FeedbackBuffer.Vertices[0].x;
    int DeltaY = (int)FeedbackBuffer.Vertices[1].y - (int)FeedbackBuffer.Vertices[0].y;
    return sqrt( (float)DeltaX * (float)DeltaX + (float)DeltaY * (float)DeltaY );
}

void mvGraphicsClass::SetColor( float r, float g, float b )
{
    float mcolor[4];

    mcolor[0] = r;
    mcolor[1] = g;
    mcolor[2] = b;
    mcolor[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mcolor);
}

void mvGraphicsClass::printtext( char * string)
{
    for (char *p = string; *p; p++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *p);
    }
}

void mvGraphicsClass::screenprinttext(int x, int y, char * string)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glRasterPos2i(x,y);
    printtext( string );
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void mvGraphicsClass::RotateToRot( Rot &rot )
{
    float fRotAngle;
    Vector3 vAxis;
    Rot2AxisAngle( vAxis, fRotAngle, rot );
    glRotatef( fRotAngle / mvConstants::piover180, vAxis.x, vAxis.y, vAxis.z );
}

void mvGraphicsClass::DrawSquareXYPlane()
{
    // DEBUG(  "DrawSquareXYPlanes" ); // DEBUG
    glBegin( GL_LINE_LOOP );
    glVertex3f( -0.5, -0.5, 0);
    glVertex3f(-0.5, 0.5,0);
    glVertex3f( 0.5,0.5, 0);
    glVertex3f( 0.5,-0.5, 0);
    glEnd();
}

void mvGraphicsClass::DrawParallelSquares( int iNumSlices )
{
    // DEBUG(  "DrawParallelSquares" ); // DEBUG
    glPushMatrix();
    glTranslatef( 0,0, -0.5 );
    float fSpacing = 1.0 / (float)iNumSlices;
    for( int i = 0; i <= iNumSlices; i++ )
    {
        DrawSquareXYPlane();
        glTranslatef( 0, 0, fSpacing );
    }
    glPopMatrix();
}

void mvGraphicsClass::DrawWireframeBox( int iNumSlices )
{
    // DEBUG(  "DrawWireframeBox" ); // DEBUG
    glPushMatrix();
    DrawParallelSquares( iNumSlices );
    glPushMatrix();
    glRotatef( 90, 1,0,0);
    DrawParallelSquares( iNumSlices );

    glPopMatrix();

    glRotatef( 90, 0,1,0);
    DrawParallelSquares( iNumSlices );


    glPopMatrix();
}

void mvGraphicsClass::DoCone()
{
    if( !bConeDefined )
    {
        glNewList(LISTCone, GL_COMPILE);

        glPushMatrix();
        GLUquadricObj *quadratic=gluNewQuadric();   // Create A Pointer To The Quadric Object
        gluQuadricNormals(quadratic, GLU_SMOOTH); // Create Smooth Normals
        gluQuadricTexture(quadratic, GL_TRUE);  // Create Texture Coords
        glTranslatef(0.0f,0.0f,-0.5f);   // Center The Cone
        gluQuadricOrientation( quadratic, GLU_OUTSIDE );
        gluCylinder(quadratic,0.5f,0.0f,1.0f,32,32);
        //glTranslatef(0.0f,0.0f,-0.5f);
        glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
        gluDisk(quadratic,0.0f,0.5f,32,32);
        //glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
        //glTranslatef(0.0f,0.0f,1.0f);
        //gluDisk(quadratic,0.0f,1.0f,32,32);
        glPopMatrix();

        glEndList();
        bConeDefined = true;
    }

    glCallList(LISTCone);
}

void mvGraphicsClass::DoCube()
{

    if( !bCubeDefined )
    {
        glNewList(LISTCube, GL_COMPILE);

        glBegin(GL_QUADS);
        glNormal3f( 0.0F, 0.0F, 1.0F);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f( 1, 1, 1);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0, 1,1);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 0,0, 1);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 1,0, 1);

        glNormal3f( 0.0F, 0.0F,-1.0F);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 1, 1, 0);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f( 1,0, 0);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f( 0,0, 0);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(0, 1, 0);

        glNormal3f( 0.0F, 1.0F, 0.0F);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f( 1, 1, 1);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f( 1, 1,0);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(0, 1,0);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(0, 1, 1);

        glNormal3f( 0.0F,-1.0F, 0.0F);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0,0,0);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 1,0,0);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 1,0, 1);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(0,0, 1);

        glNormal3f( 1.0F, 0.0F, 0.0F);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 1, 1, 1);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f( 1,0, 1);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f( 1,0,0);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 1, 1,0);

        glNormal3f(-1.0F, 0.0F, 0.0F);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(0,0,0);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(0,0, 1);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(0, 1, 1);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0, 1,0);
        glEnd();
        glEndList();

        //    DEBUG(  "cube list stored" ); // DEBUG
        bCubeDefined = true;
    }

    glPushMatrix();
    glTranslatef( -0.5 , -0.5, -0.5 );

    // if( sTextureChecksum != "" )
    // {
    // }

    glCallList(LISTCube);
    glPopMatrix();
}

void mvGraphicsClass::DoSphere()
{
    if( !bSphereDefined )
    {
        glNewList(LISTSphere, GL_COMPILE);

        GLUquadricObj *quadratic=gluNewQuadric();   // Create A Pointer To The Quadric Object
        gluQuadricNormals(quadratic, GLU_SMOOTH); // Create Smooth Normals
        gluQuadricTexture(quadratic, GL_TRUE);  // Create Texture Coords
        gluSphere(quadratic,0.5f,32,32);
        glEndList();
        bSphereDefined = true;
    }

    glCallList(LISTSphere);
}

void mvGraphicsClass::DoCylinder()
{
    if( !bCylinderDefined )
    {
        glNewList(LISTCylinder, GL_COMPILE);

        glPushMatrix();
        GLUquadricObj *quadratic=gluNewQuadric();   // Create A Pointer To The Quadric Object
        gluQuadricNormals(quadratic, GLU_SMOOTH); // Create Smooth Normals
        gluQuadricTexture(quadratic, GL_TRUE);  // Create Texture Coords
        glTranslatef(0.0f,0.0f,-0.5f);   // Center The Cylinder
        gluQuadricOrientation( quadratic, GLU_OUTSIDE );
        gluCylinder(quadratic,0.5f,0.5f,1.0f,32,32);
        //glTranslatef(0.0f,0.0f,-0.5f);
        glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
        gluDisk(quadratic,0.0f,0.5f,32,32);
        glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
        glTranslatef(0.0f,0.0f,1.0f);
        gluDisk(quadratic,0.0f,0.5f,32,32);
        glPopMatrix();

        glEndList();
        bCylinderDefined = true;
    }

    glCallList(LISTCylinder);
}

// based on http://nehe.gamedev.net
int Height( unsigned char *g_HeightMap, int iMapSize, const int X, const int Y)    // This Returns The Height From A Height Map Index
{
    int x = X % iMapSize;        // Error Check Our x Value
    int y = Y % iMapSize;        // Error Check Our y Value

    if(!g_HeightMap)
        return 0;       // Make Sure Our Data Is Valid

    return g_HeightMap[x + (y * iMapSize)];    // Index Into Our Height Array And Return The Height
}

// based on http://nehe.gamedev.net
void mvGraphicsClass::RenderHeightMap(  unsigned char *g_HeightMap, int iMapSize )     // This Renders The Height Map As Quads
{
    int X = 0, Y = 0;
    int x, y, z;

    if( !g_HeightMap )
    {
        DEBUG(  "Error: no height map data available" ); // DEBUG
        return;
    }

    glBegin( GL_QUADS );

    // DEBUG(  "drawing quads..." ); // DEBUG
    for ( X = 2; X < (iMapSize - 3); X += 1 )
    {
        // DEBUG(  "X " << X ); // DEBUG
        for ( Y = 2; Y < (iMapSize - 3); Y += 1 )
        {
            Vector3 Normal; // = VectorNormals[ X + 128 * Y ];
            Normal.z = 1;
            Normal.x = ( Height( g_HeightMap, iMapSize, X + 10, Y ) - Height( g_HeightMap, iMapSize, X, Y ) ) / 10.0f;
            Normal.y = ( Height( g_HeightMap, iMapSize, X, Y + 10 ) - Height( g_HeightMap, iMapSize, X, Y ) ) / 10.0f;

            x = X;
            y = Y;
            z = Height( g_HeightMap, iMapSize, X, Y );
            glNormal3f( Normal.x, Normal.y, Normal.z);
            glTexCoord2f((float)X / (float)iMapSize, (float)Y / (float)iMapSize);
            glVertex3i(x, y, z);

            x = X + 1;
            y = Y;
            z = Height( g_HeightMap, iMapSize, X + 1, Y );
            glNormal3f( Normal.x, Normal.y, Normal.z);
            glTexCoord2f((float)(X + 1) / (float)iMapSize, (float)(Y ) / (float)iMapSize);
            glVertex3i(x, y, z);

            x = X + 1;
            y = Y + 1 ;
            z = Height( g_HeightMap, iMapSize, X + 1, Y + 1 );
            glNormal3f( Normal.x, Normal.y, Normal.z);
            glTexCoord2f((float)(X + 1) / (float)iMapSize, (float)(Y + 1 ) / (float)iMapSize);
            glVertex3i(x, y, z);

            x = X;
            y = Y + 1 ;
            z = Height( g_HeightMap, iMapSize, X, Y + 1 );
            glNormal3f( Normal.x, Normal.y, Normal.z);
            glTexCoord2f((float)(X) / (float)iMapSize, (float)(Y + 1 ) / (float)iMapSize);
            glVertex3i(x, y, z);
        }
    }
    //DEBUG(  "Quads done" << X ); // DEBUG
    glEnd();
}

//int mvGraphicsClass::iNextTerrainListNumber = 1001;

void mvGraphicsClass::RenderTerrain(  unsigned char *g_HeightMap, int iMapSize )
{
    // DEBUG(  "RenderTerrain() start..." ); // DEBUG
    glPushMatrix();
    glTranslatef( -0.5,-0.5,-0.5 );
    glScalef( 1 / (float)iMapSize, 1 / (float)iMapSize, 1.0f / 256.0f );

    //if( !bTerrainListInitialized )
    // {
    //  iOurTerrainListNumber = iNextTerrainListNumber;
    // iNextTerrainListNumber++;

    //   glNewList(iOurTerrainListNumber, GL_COMPILE);
    // DEBUG(  "rendering height map..." ); // DEBUG

    RenderHeightMap( g_HeightMap, iMapSize );
    // DEBUG(  "rendering done" ); // DEBUG
    //   glEndList();

    //  bTerrainListInitialized = true;
    //}

    //glCallList(iOurTerrainListNumber);

    glPopMatrix();

    //  DEBUG(  "RenderTerrain() ... done" ); // DEBUG
}

void mvGraphicsClass::Translatef( float x, float y, float z )
{
    ::glTranslatef( x, y, z );
}

void mvGraphicsClass::Scalef( float x, float y, float z )
{
    ::glScalef( x, y, z );
}

void mvGraphicsClass::Bind2DTexture(int iTextureID )
{
    ::glBindTexture( GL_TEXTURE_2D, iTextureID );
}

void mvGraphicsClass::PopMatrix()
{
    ::glPopMatrix();
}

void mvGraphicsClass::PushMatrix()
{
    ::glPushMatrix();
}

void mvGraphicsClass::SetMaterialColor(float *mcolor)
{
    glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mcolor );
}

void mvGraphicsClass::DoWireSphere()
{
    glutWireSphere(0.5, 32, 32 );
}

void mvGraphicsClass::RasterPos3f(float x, float y, float z )
{
    ::glRasterPos3f(x, y, z );
}

void mvGraphicsClass::Rotatef( float fAngleDegrees, float fX, float fY, float fZ)
{
    glRotatef( fAngleDegrees, fX, fY, fZ );
}

