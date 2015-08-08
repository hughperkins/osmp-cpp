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
// see headerfile playermovement.h for documentation

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

//#include "keyandmouse.h"
#include "PlayerMovement.h"
#include "RendererImpl.h"
#include "Constants.h"
#include "Animation.h"
//#include "keyandmouse.h"
#include "WorldStorage.h"
#include "MetaverseClient.h"

namespace MetaverseClient
{
    extern int iMyReference;
    extern mvWorldStorage World;
    extern Animation animator;
}
using namespace MetaverseClient;

namespace RendererImpl
{
    extern int iViewPoint;
}

namespace PlayerMovement
{
    //Vector3 ZAXIS = { 0.0, 0.0, 1.0 };
    //Vector3 YAXIS = { 0.0, 1.0, 0.0 };

    float fAvatarAcceleration = 60.0;
}

using namespace PlayerMovement;

void PlayerMovementClass::MouseLook( int iMouseMoveX, int iMouseMoveY )
{
    avatarzrot -= iMouseMoveX;
    avataryrot += iMouseMoveY;
    UpdateAvatarObjectRotAndPos();
}

void PlayerMovementClass::UpdateAvatarObjectRotAndPos()
{
    Object *p_Avatar = World.GetObjectByReference( MetaverseClient::iMyReference );

    if( p_Avatar != NULL )
    {
        p_Avatar->pos.x = avatarxpos;
        p_Avatar->pos.y = avatarypos;
        p_Avatar->pos.z = avatarzpos;

        Rot rTwist, rPitch;
        Rot rAvatar;
        //AxisAngle2Rot( rTwist, ZAXIS, ((float)avatarzrot * mvConstants::piover180 ) );
        //AxisAngle2Rot( rPitch, YAXIS, ((float)avataryrot * mvConstants::piover180) );
        //RotMultiply( rAvatar, rTwist, rPitch );

        AxisAngle2Rot( rAvatar, ZAXIS, ((float)avatarzrot * mvConstants::piover180 ) );

        p_Avatar->rot = rAvatar;
    }
}

void PlayerMovementClass::MovePlayer()
{
    float fRight = 0.0;
    float fUp = 0.0;
    float fForward = 0.0;

    if( kMovingLeft )
    {
        //    DEBUG(  "MovePlayer() key left pressed" <<endl;
        fRight -= 1.0;
        bAvatarMoved = true;
    }
    if( kMovingRight )
    {
        //   DEBUG(  "MovePlayer() key right pressed" <<endl;
        fRight += 1.0;
        bAvatarMoved = true;
    }
    if( kMovingForwards )
    {
        //   DEBUG(  "MovePlayer() key forward pressed" <<endl;
        bAvatarMoved = true;
        fForward += 1.0;
    }
    if( kMovingBackwards )
    {
        //   DEBUG(  "MovePlayer() key backwards pressed" <<endl;
        fForward -= 1.0;
        bAvatarMoved = true;
    }
    if( kMovingUpZAxis )
    {
        //   DEBUG(  "MovePlayer() key up pressed" <<endl;
        fUp += 1.0;
        bAvatarMoved = true;
    }
    if( kMovingDownZAxis )
    {
        //   DEBUG(  "MovePlayer() key down pressed" <<endl;
        fUp -= 1.0;
        bAvatarMoved = true;
    }

    //   DEBUG(  "fup " << fUp << " fforward " << fForward << " fRight " << fRight ); // DEBUG

    if( bAvatarMoved )
    {
        float fTimeSlotMultiplier = (float)animator.GetThisTimeInterval() / 1000.0;

        Object *p_Avatar = NULL;
        switch( RendererImpl::iViewPoint )
        {
            case RendererImpl::VIEWPOINT_MOUSELOOK:
            case RendererImpl::VIEWPOINT_BEHINDPLAYER:

            p_Avatar = World.GetObjectByReference( MetaverseClient::iMyReference );
            if( p_Avatar != NULL )
            {
                p_Avatar->vVelocity.x = 2 * fAvatarMoveSpeed * fForward * (float)cos(avatarzrot*mvConstants::piover180);
                p_Avatar->vVelocity.y = 2 * fAvatarMoveSpeed * fForward * (float)sin(avatarzrot*mvConstants::piover180);

                p_Avatar->vVelocity.x += 2 * fAvatarMoveSpeed * fRight * (float)sin(avatarzrot*mvConstants::piover180);
                p_Avatar->vVelocity.y -= 2 * fAvatarMoveSpeed * fRight * (float)cos(avatarzrot*mvConstants::piover180);

                p_Avatar->vVelocity.z = fAvatarMoveSpeed * fUp;
            }

            /*

               avatarxpos += fForward * (float)cos(avatarzrot*mvConstants::piover180) * fAvatarMoveSpeed * fTimeSlotMultiplier;
               avatarypos += fForward * (float)sin(avatarzrot*mvConstants::piover180) * fAvatarMoveSpeed * fTimeSlotMultiplier;

               avatarxpos += fRight * (float)sin(avatarzrot*mvConstants::piover180) * fAvatarMoveSpeed * fTimeSlotMultiplier;
               avatarypos -= fRight * (float)cos(avatarzrot*mvConstants::piover180) * fAvatarMoveSpeed * fTimeSlotMultiplier;

               avatarzpos += fUp * fAvatarMoveSpeed * fTimeSlotMultiplier;
               */
            break;

            case RendererImpl::VIEWPOINT_THIRDPARTY:
            fThirdPartyViewZoom += fForward;
            fThirdPartyViewRotate += fRight * 3.0;
            break;
        }
        UpdateAvatarObjectRotAndPos();
    }
}
