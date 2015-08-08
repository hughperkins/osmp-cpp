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
// see header file for documentation

#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
using namespace std;

#include "Selection.h"
#include "WorldStorage.h"
#include "MetaverseClient.h"

#include "Avatar.h"

#include "ClientLinking.h"

namespace MetaverseClient
{
    extern int iMyReference;
    extern mvWorldStorage World;
    extern mvSelection Selector;
}
using namespace MetaverseClient;

//extern void SendClientMessage( const char *message );

void ClientLinkingClass::LinkSelectedObjectsToAvatar()
{
    if( Selector.GetNumSelected() > 0 )
    {
        int iMemberReference;
        Avatar *p_AvatarObject;
        p_AvatarObject = dynamic_cast< Avatar * >(World.GetObjectByReference( MetaverseClient::iMyReference ));

        set <int>
        Members;
        Members.clear();

        for( int i = 0; i < p_AvatarObject->iNumSubObjects; i++ )
        {
            iMemberReference = p_AvatarObject->SubObjectReferences[i]->iReference;
            Members.insert( iMemberReference );
            //          messagestream << "<member ireference=\"" << iMemberReference << "\"/>";
        }

        for ( Selector.SelectedObjectIterator = Selector.SelectedObjects.begin(); Selector.SelectedObjectIterator != Selector.SelectedObjects.end( ); Selector.SelectedObjectIterator++ )
        {
            iMemberReference = Selector.SelectedObjectIterator->second.iReference;
            if( iMemberReference != MetaverseClient::iMyReference )
            {
                Members.insert( iMemberReference );
                //messagestream << "<member ireference=\"" << iMemberReference << "\"/>";
            }
        }

        ostringstream messagestream;
        messagestream << "<objectupdate ireference=\"" << MetaverseClient::iMyReference << "\" type=\"Avatar\">"
        "<members>";

        for( set < int >::iterator membersiterator = Members.begin(); membersiterator != Members.end(); membersiterator++ )
        {
            messagestream << "<member ireference=\"" << *membersiterator << "\"/>";
        }

        messagestream << "</members></objectupdate>" << endl;
        string message = messagestream.str();
        DEBUG(  "Sending to server: [" << message ); // DEBUG
        MetaverseClient::SendToServer( message.c_str() );
        Selector.Clear();
    }
}

void ClientLinkingClass::LinkSelectedObjects()
{
    if( Selector.GetNumSelected() > 0 )
    {
        int iLinkNum;

        Selector.SelectedObjectIterator = Selector.SelectedObjects.begin();
        int iReference = Selector.SelectedObjectIterator->second.iReference;
        if( iReference != -1 )
        {
            DEBUG(  "making message..." ); // DEBUG
            int iArrayNum = World.GetArrayNumForObjectReference( iReference );
            if( iArrayNum != -1 )
            {
                Object *p_Root = World.GetObject( iArrayNum );
                ostringstream messagestream;
                messagestream << "<objectcreate type=\"ObjectGrouping\"><geometry>"
                << p_Root->pos << "</geometry>"
                "<members>";

                int iMemberReference;
                for (  ; Selector.SelectedObjectIterator != Selector.SelectedObjects.end( ); Selector.SelectedObjectIterator++ )
                {
                    iMemberReference = Selector.SelectedObjectIterator->second.iReference;
                    messagestream << "<member ireference=\"" << iMemberReference << "\"/>";
                }

                messagestream << "</members></objectcreate>" << endl;
                string message = messagestream.str();
                DEBUG(  "Sending to server: [" << message ); // DEBUG
                MetaverseClient::SendToServer( message.c_str() );

                Selector.Clear();
            }
        }
    }
}

//void ClientLinkingClass::RemoveFromLinkedSetGUIXML( TiXmlElement *pElement )
void ClientLinkingClass::RemoveFromLinkedSetRefList( vector<int> &RefList )
{
    int iFirstSelectedReference = RefList.front();
    Object *p_FirstTargetObject = World.GetObjectByReference( iFirstSelectedReference );
    if( p_FirstTargetObject == NULL )
    {
        DEBUG(  "RemoveFromLinkedSetGUIXML() Warning: firstselectereference " << iFirstSelectedReference << " doesnt exist" ); // DEBUG
    }

    int iGroupReference = p_FirstTargetObject->iParentReference;
    ObjectGrouping *p_Group = dynamic_cast< ObjectGrouping *>( World.GetObjectByReference( iGroupReference ) );
    if( p_Group == NULL )
    {
        DEBUG(  "RemoveFromLinkedSetGUIXML() Warning: iGroupReference " << iGroupReference << " doesnt exist" ); // DEBUG
    }

    set < int >
    iReferencesToDelete;
    iReferencesToDelete.clear();

    int iSelectedReference;
    vector<int>::iterator it;
    for( it = RefList.begin(); it != RefList.end(); it++ )
    {
        iSelectedReference = *it;
        DEBUG(  "RemoveFromLinkedSetGUIXML ref " << iSelectedReference << " is to be deleted" ); // DEBUG
        iReferencesToDelete.insert( iSelectedReference );
    }

    ostringstream messagestream;
    messagestream << "<objectupdate ireference=\"" << iGroupReference << "\">"
    "<members>";

    bool bAtLeastOnePrimRemaining = false;

    for( int i = 0; i < p_Group->iNumSubObjects; i++ )
    {
        int iChildReference = p_Group->SubObjectReferences[ i ]->iReference;
        if( iReferencesToDelete.find( iChildReference ) == iReferencesToDelete.end() )
        {
            messagestream << "<member ireference=\"" << iChildReference << "\"/>";
            bAtLeastOnePrimRemaining = true;
        }
    }

    int iMemberReference;
    messagestream << "</members></objectupdate>" << endl;

    if( bAtLeastOnePrimRemaining )
    {
        string message = messagestream.str();
        DEBUG(  "Sending to server: [" << message ); // DEBUG
        MetaverseClient::SendToServer( message.c_str() );
    }
}

void ClientLinkingClass::UnlinkSelectedObject()
{
    if( Selector.GetNumSelected() == 1 )
    {
        Selector.SelectedObjectIterator = Selector.SelectedObjects.begin();
        int iSelectedReference = Selector.SelectedObjectIterator->second.iReference;
        int iSelectedArrayNum = World.GetArrayNumForObjectReference( iSelectedReference );
        if( iSelectedArrayNum != -1 )
        {
            Object *p_Object = World.GetObject( iSelectedArrayNum );
            if( strcmp( p_Object->sDeepObjectType, "OBJECTGROUPING" ) == 0 )
            {
                DEBUG(  "deleting objectgrouping ref " << iSelectedReference ); // DEBUG
                ostringstream messagestream;
                messagestream << "<objectdelete ireference=\"" << iSelectedReference << "\" type=\"ObjectGrouping\" recursivedelete=\"false\"/>" << endl;
                DEBUG(  "sending to server [" << messagestream.str() << "]" ); // DEBUG
                MetaverseClient::SendToServer( messagestream.str().c_str() );
                Selector.Clear();
            }
            else
            {
                DEBUG(  "object is not an objectgrouping: " << p_Object->sDeepObjectType ); // DEBUG
            }
        }
        else
        {
            DEBUG(  "object doesnt actually exist" ); // DEBUG
        }
    }
    else
    {
        DEBUG(  Selector.GetNumSelected() << " object selected, ignoring unlink request" ); // DEBUG
    }
}
