// Copyright Hugh Perkins 2005
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

#include <iostream>
using namespace std;

class CallbackToPythonClass
{
public:
    virtual void SendToServer( const char *CALLBACKINPUT ) = 0;  //!< send message to metaverse server
    virtual void DisplayContextMenu( const int mousex, const int mousey ){}  //!< Display context menu at x,y
    virtual void DisplayConfigDialog(){}  //!< Display configuration dialog box
    virtual void ActivateChat(){}  //!< Set focus to chat
    virtual void Hide(){}  //!< renderer window was minimized, so gui should minimize too
    virtual void Reappear(){}  //!< renderer window reappeared, so gui should reappear too
    virtual void DoEvents(){}
    virtual void DeleteCurrentSelection(){ cout << "DeleteCurrentSelection hasnt been implemented" << endl; }
    
    virtual void KeyDown( int keycode ){}
    virtual void KeyUp( int keycode ){}
    virtual void MouseMove( int x, int y ){}
    virtual void MouseDown( bool bLeftDown, bool bRightDown, int x, int y ){}
    virtual void MouseUp( bool bLeftUp, bool bRightUp, int x, int y ){}
    
    virtual void Quit(){}
    
    virtual void SendMessageToGUI( const char *CALLBACKINPUT ) = 0; //!< Generic function for migration purposes
};
   
