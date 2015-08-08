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
//!
//! This module carries out OpenGL initialization and manages drawing of the world on demand
//! In addition, it's responsible for registering callbacks for keyboard, mouse, screen resize and so on

#ifndef _RENDERERIMPL_H
#define _RENDERERIMPL_H

#include "Object.h"
#include "Prim.h"
#include "ObjectGrouping.h"

//! These functions carry out OpenGL initialization and manage drawing of the world on demand
//!
//! These functions carry out OpenGL initialization and manage drawing of the world on demand
//! In addition, they're responsible for registering callbacks for keyboard, mouse, screen resize and so on
namespace RendererImpl
{
    const int VIEWPOINT_MOUSELOOK = 0;
    const int VIEWPOINT_BEHINDPLAYER = 1;
    const int VIEWPOINT_THIRDPARTY = 2;

    void ToggleViewPoint();                                                 //!< toggles viewpoint, ie mouselook vs third party view vs behind player

    int GetScreenWidth();  //!< get width of screen
    int GetScreenHeight();  //!< get screen height

    int GetWindowWidth();  //!< get the window height; I think this is the internal window height, not including dragbar etc, but I dont remember :-/
    int GetWindowHeight();  //!< get the window width; I think this is the internal window width, not including dragbar etc, but I dont remember :-/
    int GetWindowXPos();  //!< get position of window on screen, x coordinate
    int GetWindowYPos();  //!< get position of window on screen, y coordinate

    void RendererInit(int argc, char *argv[]);                               //!< Call this before anything else; pass in argc and argv from main()
    void RendererRegisterVisibilityCallback( void (*pVisibility)( int ) );   //!< Use to register visibiltiy callback (minimized/maximize)
    void RendererRegisterMainLoop(void (*pMainLoop)( void ));                //!< Use to register application MainLoop
    void RendererStartMainLoop();                                            //!< Starts the ball rolling.  Mainloop will be called each frame now
    void DrawLandscape();                                                    //!< draws hardcoded terrain; temporary function until terrain ispart of db
    void DrawSelectedObjects();                                              //!< draws selected object highlighting
    void DrawObjects();                                                      //!< draws objects in world, including other avatars and so on
    void DrawWorld();                                                        //!< draws worldstorage objects and hardcoded terrain
    void disp();                                                             //!< function registered with OpenGL which draws everything you see in the Window (calls DrawWorld)

    void SetupAxes( Object *p_Object );  //!< applies the appropriate transformations so a child object renders at the right place and orientation; this is used for example when someone selects a single primitive from within a linked set
}

#endif // _RENDERERIMPL_H
