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
//! \brief This module is responsible for moving one's own avatar around
//!
//! This module is responsible for moving one's own avatar around
//! it is called by renderermain

#ifndef _PLAYERMOVEMENT_H
#define _PLAYERMOVEMENT_H

//! \brief PlayerMovementClass is responsible for moving one's own avatar around
//!
//! PlayerMovementClass is responsible for moving one's own avatar around
//! it is called by renderermain
class PlayerMovementClass
{
public:
   bool bAvatarMoved;  //! If avatar has moved (so it should be synchronized to server)

   float fAvatarTurnSpeed;  //! turnspeed of avatar (constant)
   float fAvatarMoveSpeed;   //! movespeed of avatar (constant)
   
   bool kMovingLeft;    //!< set by keyboardandmouse, and used by playermovement to set current player object velocity
   bool kMovingRight;//!< set by keyboardandmouse, and used by playermovement to set current player object velocity
   bool kMovingForwards;//!< set by keyboardandmouse, and used by playermovement to set current player object velocity
   bool kMovingBackwards;//!< set by keyboardandmouse, and used by playermovement to set current player object velocity
   bool kMovingUpZAxis;//!< set by keyboardandmouse, and used by playermovement to set current player object velocity
   bool kMovingDownZAxis;//!< set by keyboardandmouse, and used by playermovement to set current player object velocity
   
   bool bJumping;//!< set by keyboardandmouse, and used by playermovement to set current player object velocity
   
   float avatarzrot,avataryrot;    //!< avatar z and y rot, used to get avatar rotation
   float avatarxpos, avatarypos, avatarzpos;   //!< avatar x,y,z pos, no longer used since we set avatar object velocity now and let physics engine move avatar

   float fThirdPartyViewZoom;  //!< used in third party view (f9 twice).  Distance from avatar
   float fThirdPartyViewRotate;   //!< used in third party view (f9 twice).  Rotation around avatar

   PlayerMovementClass()
   {
      bAvatarMoved = false;
      
      fAvatarTurnSpeed = 60.0;   // constant that sets how fast an avatar turns
      fAvatarMoveSpeed = 4.0;    // constant that sets how fast an avatar moves

      avatarxpos = 0.0; 
      avatarypos = 0.0;    
      avatarzpos = 0.5;  

      kMovingLeft = false;
      kMovingRight = false;
      kMovingUpZAxis = false;
      kMovingDownZAxis = false;
      kMovingForwards = false;
      kMovingForwards = false;
      
      bJumping = false;
      
      avatarzrot = 0;
      avataryrot = 0;
   
      fThirdPartyViewZoom = -10;
      fThirdPartyViewRotate = 0;
   }
   void MovePlayer();             //!< call this to set current avatar object velocities according to current keyboard state
   void UpdateAvatarObjectRotAndPos();  //!< call this to synchronize avatarposx, avatarposy etc into the actual underlying avartar object pos and rot in mvWorldStorage
   void MouseLook( int iDeltaMouseX, int iDeltaMouseY );
   //void ToggleFly();
};

#endif // _PLAYERMOVEMENT_H
