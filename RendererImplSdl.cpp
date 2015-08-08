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
//! \brief This module carries out OpenGL initialization and manages drawing of the world on demand

// Documentation in header file

#ifdef _WIN32
#include <windows.h>
#endif

#include <string>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <stdio.h>
using namespace std;

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "SDL.h"
#include "SDL_syswm.h"
#include "tinyxml.h"

#include "TickCount.h"
#include "Diag.h"
#include "Constants.h"
#include "Selection.h"
#include "WorldStorage.h"
#include "Animation.h"
#include "PlayerMovement.h"
#include "Graphics.h"
//#include "keyandmouse.h"
#include "Camera.h"
#include "Editing3D.h"

#include "RendererImpl.h"

#include "MetaverseClient.h"

namespace MetaverseClient
{
    //! used for object selection (used here so we know what objects to draw highlighted)
    //! we also add OpenGL names for the objects we draw, so that Selection in mvSelection
    //! can detect which object the user clicks on
    extern mvSelection Selector;

    extern ConfigClass mvConfig;   //!< Loads configuration from config.xml, and makes it available as properties
    extern mvWorldStorage World;   //!< contains world (which we draw)

    //! contains information on current avatar position; arguably this could be in World
    //! or elsewhere, but for now it's in animator
    extern Animation animator;

    extern TextureInfoCache textureinfocache;
    extern PlayerMovementClass PlayerMovement;

    extern int iMyReference;   //!< iReference for Avatar object
    extern float fHeight;         //!< height of hard-coded platform, which we draw
    extern char SkyboxReference[33];

    extern mvGraphicsClass mvGraphics;
    extern mvCameraClass Camera;

    extern Editing3DClass Editing3D;
}
using namespace MetaverseClient;

namespace RendererImpl
{
    const SDL_VideoInfo* VideoInfo = NULL;
    SDL_SysWMinfo WMInfo;

    const char WindowName[] = "The OpenSource Metaverse Project";
    int iWindowWidth;
    int iWindowHeight;

    void (*pMainLoop)( void );

    //! \brief Contains attributes of current third party viewpoint, used by camera
    namespace ThirdPartyView
    {
        float fZoom = -10;
        float fRotate = 0;
    }

    int iViewPoint = VIEWPOINT_MOUSELOOK;
    //   float fZoom = -10.0;
    //   float fRotate = 0.0;

    void ToggleViewPoint()
    {
        DEBUG(  "toggling viewpoint..." ); // DEBUG
        iViewPoint++;
        if( iViewPoint > 2 )
        {
            iViewPoint = 0;
        }
    }

    static void ProcessEvents( void )
    {
        bool bReceivedMouseMove = false;
        SDL_Event event;
        while( SDL_PollEvent( &event ) )
        {
            switch( event.type )
            {
                case SDL_KEYDOWN:
                // DEBUG(  "key down" ); // DEBUG
                pCallbackToPython->KeyDown( event.key.keysym.sym );
                //mvKeyboardAndMouse::keydown( event.key );
                break;
                case SDL_KEYUP:
                /* Handle key presses. */
                pCallbackToPython->KeyUp( event.key.keysym.sym );
                //mvKeyboardAndMouse::keyup( event.key );
                break;

                case SDL_MOUSEMOTION:
                if( !bReceivedMouseMove )
                {
                    pCallbackToPython->MouseMove( event.motion.x, event.motion.y );
                    // mvKeyboardAndMouse::mousemove( event.motion );
                    bReceivedMouseMove = true;
                }
                break;

                case SDL_MOUSEBUTTONDOWN:
                pCallbackToPython->MouseDown( event.button.button == SDL_BUTTON_LEFT,
                                              event.button.button == SDL_BUTTON_RIGHT, event.motion.x, event.motion.y );
                //mvKeyboardAndMouse::mousedown( event.button );
                break;

                case SDL_MOUSEBUTTONUP:
                pCallbackToPython->MouseUp( event.button.button == SDL_BUTTON_LEFT,
                                            event.button.button == SDL_BUTTON_RIGHT, event.motion.x, event.motion.y );
                //mvKeyboardAndMouse::mouseup( event.button );
                break;

                case SDL_QUIT:
                pCallbackToPython->Quit();
                exit(0);
                break;
            }
        }
    }

    int GetScreenWidth()
    {
        return glutGet( GLUT_SCREEN_WIDTH );

        // could possibly do this with wxwidgets instead:
        //       int x,y,width,height;
        //       wxClientDisplayRect(&x, &y, &width, &height);
        //       return width;
    }

    int GetScreenHeight()
    {
        return glutGet( GLUT_SCREEN_HEIGHT );
        //       int x,y,width,height;
        //       wxClientDisplayRect(&x, &y, &width, &height);
        //       return width;
    }

    int GetWindowWidth()
    {
#ifdef _WIN32
        RECT Rect;
        GetWindowRect( WMInfo.window,  &Rect );
        DEBUG(  "WindowRect left, right, right-left: " << Rect.left << " " << Rect.right << " " << Rect.right - Rect.left ); // DEBUG
        return Rect.right - Rect.left;
#else

        return iWindowWidth;
#endif

    }

    int GetWindowHeight()
    {
        DEBUG( "GetWindowHeight()" );
#ifdef _WIN32

        RECT Rect;
        GetWindowRect( WMInfo.window,  &Rect );
        DEBUG(  "WindowRect top,bottom, top-bottom: " << Rect.top << " " << Rect.bottom << " " << Rect.top - Rect.bottom ); // DEBUG
        return Rect.bottom - Rect.top;
#else

        return iWindowHeight;
#endif

    }

    int GetWindowXPos()
    {
#ifdef _WIN32
        RECT Rect;
        GetWindowRect( WMInfo.window,  &Rect );
        DEBUG(  "WindowRect left, right, right-left: " << Rect.left << " " << Rect.right << " " << Rect.right - Rect.left ); // DEBUG
        return Rect.left;
#else

        return 0;
#endif

    }

    int GetWindowYPos()
    {
#ifdef _WIN32
        RECT Rect;
        GetWindowRect( WMInfo.window,  &Rect );
        DEBUG(  "WindowRect top,bottom, top-bottom: " << Rect.top << " " << Rect.bottom << " " << Rect.top - Rect.bottom ); // DEBUG
        return Rect.top;
#else

        return 0;
#endif

    }

    //! Functions to register callbacks:
    void RendererRegisterMainLoop(void (*pNewMainLoop)( void ))
    {
        pMainLoop = pNewMainLoop;
    }

    void RendererRegisterVisibilityCallback( void (*pVisibility)( int ) )
    {
        // glutVisibilityFunc( pVisibility );
    }

    //! Draws hardcoded platern that we start over
    //! This is a temporary function since land will be migrated to the databse
    void DrawLandscape()
    {
        int i, iy;

        float lcolor[] =
            {
                0.0f, 0.7f, 0.2f, 1.0f
            };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lcolor);

        int iLandCoord = 0;
        for( iy = -10; iy <=10; iy++ )
        {
            for( i = -10; i <= 10; i++ )
            {
                glLoadName( Selector.iTargetReference );
                Selector.LandCoords[ iLandCoord ].x = -10.0F + i * 1.0F;
                Selector.LandCoords[ iLandCoord ].y = -10.0F + iy * 1.0F;

                glPushMatrix();

                glTranslatef( -10.0F + i * 1.0F, -10.0F + iy * 1.0F, fHeight - 0.5 );
                //glutSolidCube( 1.0 );
                mvGraphics.DoCube();

                glPopMatrix();
                Selector.HitTargets[ Selector.iTargetReference ].TargetType = HITTARGETTYPE_LANDCOORD;
                Selector.HitTargets[ Selector.iTargetReference ].iForeignReference = iLandCoord;
                Selector.iTargetReference++;
                iLandCoord++;
            }
        }
    }


    void DrawSkybox(Rot CameraRot)
    {
        int iTextureID = 0;
        if( strcmp( SkyboxReference, "" ) != 0 )
        {
            iTextureID = textureinfocache.TextureReferenceToTextureId( SkyboxReference );
            strcpy(SkyboxReference, "");
        }
        else
        {
            iTextureID = textureinfocache.TextureReferenceToTextureId( World.GetSkyboxChecksum() );
        }

        if (iTextureID > -1)
        {
            mvGraphics.SetColor(1, 1, 1);

            mvGraphics.PushMatrix();

            mvGraphics.Scalef( -2, -2, -2 );
            mvGraphics.RotateToRot(CameraRot);
            mvGraphics.Bind2DTexture(iTextureID );
            glDepthMask( false);
            mvGraphics.DoSphere();
            glDepthMask( true);
            mvGraphics.Bind2DTexture(0);

            mvGraphics.PopMatrix();
        }
    }

    void SetupAxes( Object *p_Object )
    {
        if( p_Object != NULL )
        {
            if( p_Object->iParentReference != 0 )
            {
                Object *p_ParentObject = World.GetObjectByReference( p_Object->iParentReference );
                SetupAxes( p_ParentObject );
            }
            glTranslatef( p_Object->pos.x, p_Object->pos.y, p_Object->pos.z);
            mvGraphics.RotateToRot(  p_Object->rot );
        }
    }

    //! Draws highlighting on selected objects into openGL's 3d world
    void DrawSelectedObjects()
    {
        int iSelectedObjectWorldArrayNum;

        SelectionIteratorTypedef iterator;
        for ( iterator = Selector.SelectedObjects.begin( ) ; iterator != Selector.SelectedObjects.end( ); iterator++ )
        {
            //DEBUG(  " Drawing selected ref " << iterator->second.iReference ); // DEBUG
            Object *p_Object = World.GetObjectByReference( iterator->second.iReference );
            if( p_Object->iParentReference == 0 )
            {
                p_Object->DrawSelected();
            }
            else
            {
                glPushMatrix();

                SetupAxes( World.GetObjectByReference ( p_Object->iParentReference ) );

                //glTranslatef( p_Object->pos.x, p_Object->pos.y, p_Object->pos.z);
                //RotateToRot(  p_Object->rot );
                p_Object->DrawSelected();
                glPopMatrix();
            }
        }
    }

    void CallbackAddName( int iReference )
    {
        glLoadName( Selector.iTargetReference );
        Selector.HitTargets[ Selector.iTargetReference ].TargetType = HITTARGETTYPE_OBJECT;
        Selector.HitTargets[ Selector.iTargetReference ].iForeignReference = iReference;
        Selector.iTargetReference++;
    }

    //! Draws all non-hardcoded objects in world - including avatars - into the 3D world of OpenGL
    void DrawObjects()
    {
        float mcolor[] =
            {
                1.0f, 0.0f, 0.5f, 1.0f
            };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mcolor);

        for( int i = 0; i < World.iNumObjects; i++ )
        {
            // dont draw submembers directly, dont draw default objects
            if( World.GetObject(i)->iParentReference == 0 && World.GetObject(i)->iReference != 0 )
            {
                // dont draw own avatar in mouselook mode
                if( World.GetObject(i)->iReference != MetaverseClient::iMyReference )
                {

                    glRasterPos3f(World.GetObject(i)->pos.x, World.GetObject(i)->pos.y, World.GetObject(i)->pos.z);
                    glLoadName( Selector.iTargetReference );
                    //DEBUG(  "calling draw " << i << " type " << World.GetObject(i)->sDeepObjectType); // DEBUG
                    World.GetObject(i)->Draw();

                    //   DEBUG(  "draw done" ); // DEBUG
                    Selector.HitTargets[ Selector.iTargetReference ].TargetType = HITTARGETTYPE_OBJECT;
                    Selector.HitTargets[ Selector.iTargetReference ].iForeignReference = World.GetObject(i)->iReference;
                    Selector.iTargetReference++;

                    // maybe draw selected here, by migrating the selected tag into the object structure?
                }
                //      else if( RendererImpl::iViewPoint != RendererImpl::VIEWPOINT_MOUSELOOK )
                else
                {
                    glRasterPos3f(World.GetObject(i)->pos.x, World.GetObject(i)->pos.y, World.GetObject(i)->pos.z);
                    glLoadName( Selector.iTargetReference );
                    //   DEBUG(  "calling draw" ); // DEBUG

                    World.GetObject(i)->Draw();
                    //   DEBUG(  "draw done" ); // DEBUG
                    Selector.HitTargets[ Selector.iTargetReference ].TargetType = HITTARGETTYPE_OBJECT;
                    Selector.HitTargets[ Selector.iTargetReference ].iForeignReference = World.GetObject(i)->iReference;
                    Selector.iTargetReference++;

                    // maybe draw selected here, by migrating the selected tag into the object structure?
                }
            }
        }

    }

    //! Draws everything in world into the 3D world of OpenGL
    void DrawWorld()
    {
        //DEBUG(  "DrawWorld" ); // DEBUG

        int iLastCount = MVGetTickCount();

        Selector.ResetNames();


        //   DEBUG(  "drawobjects" ); // DEBUG
        DrawObjects();

        int iDrawObjectsTime = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();

        //   DEBUG(  "drawselectedobjects" ); // DEBUG
        DrawSelectedObjects();
        int iSelectedObjectsTime = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();

        //   DEBUG(  "drawlandscape" ); // DEBUG
        //DrawLandscape();
        //int iLandscapeTime = MVGetTickCount() - iLastCount; iLastCount = MVGetTickCount();



        //   DEBUG(  "DrawEditBars" ); // DEBUG
        Editing3D.DrawEditBarsToOpenGL();
        int iEditBarsTime = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();
        //  DEBUG(  "world drawn" ); // DEBUG

        TIMING( "DrawWorld timings: " << iDrawObjectsTime << " " << iSelectedObjectsTime << " " << iEditBarsTime );
    }

    //! called to draw what we see in window, once a frame.
    //! calls drawworld
    void disp()
    {
        int iLastCount = MVGetTickCount();

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glLoadIdentity();

        // rotate so z axis is up, and x axis is forward
        glRotatef( 90, 0.0, 0.0, 1.0 );
        glRotatef( 90, 0.0, 1.0, 0.0 );



        int idispstageone = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();

        if( RendererImpl::iViewPoint == RendererImpl::VIEWPOINT_MOUSELOOK )
        {
            if( Camera.bCameraEnabled )
            {
                Rot myRot;
                InverseRot(myRot, Camera.CurrentCameraRot);
                DrawSkybox(myRot);
                Camera.ApplyCameraToOpenGL();

            }
            else
            {
                glRotatef( -PlayerMovement.avataryrot, 0, 1, 0 );
                glRotatef( -PlayerMovement.avatarzrot, 0, 0, 1 );
                Rot myRot;
                DrawSkybox(myRot);
                glTranslatef( -PlayerMovement.avatarxpos, -PlayerMovement.avatarypos, -PlayerMovement.avatarzpos );
            }

        }
        else if( RendererImpl::iViewPoint == RendererImpl::VIEWPOINT_BEHINDPLAYER )
        {
            if( Camera.bCameraEnabled )
            {
                Rot myRot;
                InverseRot(myRot, Camera.CurrentCameraRot);
                DrawSkybox(myRot);
                Camera.ApplyCameraToOpenGL();
            }
            else
            {
                glRotatef( -18, 0, 1, 0 );

                Rot myRot;
                Vector3 V;
                V.x = 0;
                V.y = PlayerMovement.avataryrot * piover180;
                V.z = PlayerMovement.avatarzrot * piover180;
                AxisAngles2Rot(myRot, V);

                Rot myRot2;
                InverseRot(myRot2, myRot);

                DrawSkybox(myRot2);

                glTranslatef( 3.0, 0.0, -1.0 );

                glRotatef( -PlayerMovement.avataryrot, 0, 1, 0 );
                glRotatef( -PlayerMovement.avatarzrot, 0, 0, 1 );

                glTranslatef( -PlayerMovement.avatarxpos, -PlayerMovement.avatarypos, -PlayerMovement.avatarzpos );

            }
        }
        else if( RendererImpl::iViewPoint == RendererImpl::VIEWPOINT_THIRDPARTY )
        {
            if( Camera.bCameraEnabled )
            {
                Rot myRot;
                InverseRot(myRot, Camera.CurrentCameraRot);
                DrawSkybox(myRot);
                Camera.ApplyCameraToOpenGL();
            }
            else
            {
                glRotatef( -18, 0, 1, 0 );
                glRotatef( -90, 0, 0, 1 );

                Rot myRot;
                Vector3 V;
                V.x = 0;
                V.y = 0;
                V.z = -PlayerMovement.fThirdPartyViewRotate * piover180;
                AxisAngles2Rot(myRot, V);
                DrawSkybox(myRot);

                glTranslatef( 0.0, -PlayerMovement.fThirdPartyViewZoom, PlayerMovement.fThirdPartyViewZoom / 3.0 );

                glRotatef( -PlayerMovement.fThirdPartyViewRotate, 0, 0, 1 );

                glTranslatef( -PlayerMovement.avatarxpos, -PlayerMovement.avatarypos, -PlayerMovement.avatarzpos );
            }
        }

        int idispstagetwo = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();
        DrawWorld();

        int idispstagethree = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();
        GLfloat position[] =
            {
                -100.5f, 100.0f, 100.0f, 1.0f
            };
        glLightfv(GL_LIGHT0, GL_POSITION, position);

        //     DEBUG(  "...disp done" ); // DEBUG
        SDL_GL_SwapBuffers( );
        int idispstagefour = MVGetTickCount() - iLastCount;
        iLastCount = MVGetTickCount();

        TIMING( "disp timings " << idispstageone << " " << idispstagetwo << " " << idispstagethree << " " << idispstagefour );
    }

    void RendererStartMainLoop()
    {
        while( 1 )
        {
            int iTimeTicks = MVGetTickCount();
            int iLastCount = MVGetTickCount();

            ProcessEvents();
            int iEventsTime = MVGetTickCount() - iLastCount;
            iLastCount = MVGetTickCount();

            pMainLoop();
            int iMainLoopTime = MVGetTickCount() - iLastCount;
            iLastCount = MVGetTickCount();

            disp();
            int iDispTime = MVGetTickCount() - iLastCount;
            iLastCount = MVGetTickCount();

            TIMING( "RendererMainloop timings: " << iEventsTime << " " << iMainLoopTime << " " << iDispTime );
        }
    }

    //! Initializes OpenGL, sets up viewport,lighting, and the OpenGL options we will use (lighting etc)
    void RendererInit(int argc, char *argv[])
    {
        if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        {
            DEBUG(  "Video initialization failed: " << SDL_GetError() ); // DEBUG
            exit(1);
        }
        glutInit(&argc, argv);
        VideoInfo = SDL_GetVideoInfo();
        if( !VideoInfo )
        {
            DEBUG(  "Video info query failed: " << SDL_GetError() ); // DEBUG
            exit(1);
        }

        int bpp = VideoInfo->vfmt->BitsPerPixel;

        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

        iWindowWidth = GetScreenWidth();
        iWindowHeight = GetScreenHeight();

        // set optional window size
        ConfigClass::StringMap::iterator itw = mvConfig.ClientConfig.find( "clientWindow.width" );
        ConfigClass::StringMap::iterator ith = mvConfig.ClientConfig.find( "clientWindow.height" );
        if( itw != mvConfig.ClientConfig.end() && ith != mvConfig.ClientConfig.end() )
        {
            iWindowWidth = atoi( itw->second.c_str() );
            iWindowHeight = atoi( ith->second.c_str() );
        }
        DEBUG(  "requested window width/height: " << iWindowWidth << " " << iWindowHeight ); // DEBUG

        SDL_WM_SetIcon(SDL_LoadBMP("osmpico32.bmp"), NULL);
        if( SDL_SetVideoMode( iWindowWidth, iWindowHeight, bpp, SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF ) == 0 )
        {
            DEBUG(  "Video mode set failed: " << SDL_GetError() ); // DEBUG
            exit( 1 );
        }

        SDL_VERSION(&WMInfo.version);
        SDL_GetWMInfo(&WMInfo);

#ifdef _WIN32

        DEBUG(  "Windows handle?: " << WMInfo.window ); // DEBUG
        SetWindowPos(
            WMInfo.window,
            HWND_TOP,
            0,
            0,
            0,
            0,
            SWP_NOSIZE | SWP_NOZORDER
        );
#endif

        //DEBUG(  "actual window width/height: " << GetWindowWidth() << " " << GetWindowHeight() ); // DEBUG

#ifdef _WIN32

        RECT ClientRect;
        GetClientRect( WMInfo.window,  &ClientRect );
        DEBUG(  "clientrect: " << ClientRect.bottom << " " << ClientRect.right ); // DEBUG
        iWindowWidth = ClientRect.right;
        iWindowHeight = ClientRect.bottom;
#endif

        SDL_WM_SetCaption( WindowName, "" );

        glClearColor(0.0,0.0,0.0,0.0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable (GL_CULL_FACE);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        GLfloat ambientLight[] =
            {
                0.4f, 0.4f, 0.4f, 1.0f
            };
        GLfloat diffuseLight[] =
            {
                0.6f, 0.6f, 0.6f, 1.0f
            };
        GLfloat specularLight[] =
            {
                0.2f, 0.2f, 0.2f, 1.0f
            };
        GLfloat position[] =
            {
                -1.5f, 1.0f, -4.0f, 1.0f
            };

        glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
        glLightfv(GL_LIGHT0, GL_POSITION, position);

        glLoadIdentity();

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        GLfloat aspect = (GLfloat) iWindowWidth / iWindowHeight;
        gluPerspective( 45.0, aspect, 0.5, 100.0 );

        glMatrixMode( GL_MODELVIEW );
        glViewport (0, 0, iWindowWidth, iWindowHeight);

        Object::SetAddNameCallback( CallbackAddName );
    }

}

