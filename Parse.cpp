// Copyright Hugh Perkins 2004
//
// This client will listen to connect from the renderer and GUI.
// It will attempt to connect to the Metaverse server at the IP address specified in
// the commandline
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

#include <stdio.h>
#include <string.h>

int Parse( char args[10][512], char *target, char *separator )
{
    int iArgNum = 0;
    int bFinished = 0;
    char *pStartSubstring;
    char *pEndSubstring;
    pStartSubstring = target;
    while( bFinished == 0 && iArgNum <= 9 )
    {
        pEndSubstring = strstr( pStartSubstring, separator );
        if( pEndSubstring != NULL )
        {
            sprintf( args[ iArgNum ], "%.*s", pEndSubstring - pStartSubstring, pStartSubstring );
        }
        else
        {
            sprintf( args[ iArgNum ], "%s", pStartSubstring );
            bFinished = 1;
        }
        iArgNum++;
        pStartSubstring = pEndSubstring + strlen( separator );
    }
    return iArgNum;
}

int ParseManyArgs( char args[40][64], char *target, char *separator )
{
    int iArgNum = 0;
    int bFinished = 0;
    char *pStartSubstring;
    char *pEndSubstring;
    pStartSubstring = target;
    while( bFinished == 0 && iArgNum <= 9 )
    {
        pEndSubstring = strstr( pStartSubstring, separator );
        if( pEndSubstring != NULL )
        {
            sprintf( args[ iArgNum ], "%.*s", pEndSubstring - pStartSubstring, pStartSubstring );
        }
        else
        {
            sprintf( args[ iArgNum ], "%s", pStartSubstring );
            bFinished = 1;
        }
        iArgNum++;
        pStartSubstring = pEndSubstring + strlen( separator );
    }
    return iArgNum;
}
