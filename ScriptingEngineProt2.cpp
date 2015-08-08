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

// This file demonstrates use of XML Sockets API as interface for scripting engines into metaverseserver
// This compiles into an executable taht will download the current world state then make all cubes in the
// world chnages smoothly in position, size, rotation and color using the XML call <objectmove/>

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include <stdio.h>
#include <math.h>

#include "tinyxml.h"

#include "port_list.h"

#include "WorldStorage.h"
#include "mvobjectstorage.h"
#include "SocketsClass.h"
#include "TickCount.h"
#include "Math.h"
#include "texturemanager.h"
#include "TerrainInfoCache.h"
#include "MeshInfoCache.h"

//TextureInfoCache textureinfocache;
//TerrainCacheClass TerrainCache;
// MeshInfoCacheClass MeshInfoCache;

mvsocket SocketMetaverseServer;

char sMetaverseServerIP[64];
char sAvatarName[64] = "Guest";
char sAvatarPassword[64] = "";

#define BUFSIZE 2047
char SendBuffer[ BUFSIZE + 1 ];
char ReadBuffer[ 4097 ];

mvWorldStorage World;

int iMyReference = 0;

int StartTickCount = 0;
const int iTicksPerFrame = 200;

int LastTickCount = 0;

int iLastScriptingFrameTickCount;

float fAngle = 0;
float fRadius = 5.0;

void SendClientMessage( const char *message )
{
  SocketMetaverseServer.Send( message );
}

void SocketsBlockTillNextFrame()
{
	vector<const mvsocket *> readsocks;
	int iTicksSinceLastTime = MVGetTickCount() - LastTickCount;
	LastTickCount = MVGetTickCount();

	int iTicksTillNextFrame = iTicksPerFrame - iTicksSinceLastTime;
	if( iTicksTillNextFrame < 0 )
	{
		iTicksTillNextFrame = 0;
	}

	//	Debug( "ticks till next frame: %i\n", iTicksTillNextFrame );
	timeval TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = iTicksTillNextFrame;

//	fd_set TargetSet;
//	FD_ZERO( &TargetSet );
//	FD_SET( SocketMetaverseServer.socket, &TargetSet);
//	select( SocketMetaverseServer.socket + 1, &TargetSet, NULL, NULL, &TimeOut );
	readsocks.push_back(&SocketMetaverseServer);
	SocketsReadBlock(iTicksTillNextFrame, readsocks);
}

void HandleServerInput( char *ReadBuffer )
{
   	   if( ReadBuffer[0] == '<' )
   	   {
   	   	   Debug( "XML IPC received from server [%s]\n", ReadBuffer );
   	   	   
           TiXmlDocument IPC;
           IPC.Parse( ReadBuffer );
           TiXmlElement *pElement = IPC.RootElement();
           if( strcmp( pElement->Value(), "objectrefreshdata" ) == 0 )
           {
               World.StoreObjectXML( IPC.RootElement() );           	   
           }
           else if( strcmp( pElement->Value(), "objectcreate" ) == 0 ) 
           {
               World.StoreObjectXML( IPC.RootElement() );       	   
           }
           else if( strcmp( pElement->Value(), "objectupdate" ) == 0 )
           {
           	  World.UpdateObjectXML( IPC.RootElement() );
           }
           else if( strcmp( pElement->Value(), "objectdelete" ) == 0 ) 
           {
               World.DeleteObjectXML( IPC.RootElement() );       	   
           }
           else if( strcmp( pElement->Value(), "objectmove" ) == 0 ) 
           {
               World.MoveObjectXML( IPC.RootElement() );       	   
           }
           if( strcmp( pElement->Value(), "loginaccept" ) == 0 )
           {
           	  iMyReference = atoi( pElement->Attribute("ireference" ) );
           }
   	  }
   	  else
   	  {
   	   	   Debug( "Legacy IPC received from server [%s]\n", ReadBuffer );
       }	 
}

void MoveObjectsInCircle()
{
	int iCurrentTickCount = MVGetTickCount();
	if( iCurrentTickCount - StartTickCount > 5000 )
	{
		  StartTickCount = iCurrentTickCount;
	//  Debug( "MoveObjectsInCircle()\n" );
	  int iReference;
	  for( int i = 0; i < World.iNumObjects; i++ )
	  {
//	  	  if( World.GetObject( i )->iParentReference == 0
	  	  if( strcmp( World.GetObject( i )->sDeepObjectType, "AVATAR" ) != 0
	  	     && World.GetObject( i )->iReference != 0 )
	  	  {
	  	  	 iReference = World.GetObject( i )->iReference;
	  	  	 Debug( "processing cube ref %i...\n", iReference );

           if( ( i / 2 ) * 2 == i )	  	  	 
           {
           	Rot RandomRot;
           	Vector3 RandomVector;
           	RandomVector.x = rand() * 2.0 - 1.0;
           	RandomVector.z = rand() * 2.0 - 1.0;
           	RandomVector.y = rand() * 2.0 - 1.0;
           	AxisAngle2Rot( RandomRot, RandomVector, rand() * 6.1 );
	  	  	 sprintf( SendBuffer, "<objectmove ireference=\"%i\"><faces><face num=\"0\"><color r=\"%f\" g=\"%f\" b=\"%f\"/></face></faces>"
	  	  	     "<geometry><scale x=\"%f\" y=\"%f\" z=\"%f\"/><pos x=\"%f\" y=\"%f\" z=\"%f\"/>"
	  	  	     "<rot x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\"/>"
	  	  	     "</geometry>"
	  	  	     "<dynamics><duration milliseconds=\"6000\"/></dynamics></objectmove>\n",
	  	  	       iReference, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX,
	  	  	       (float)rand() / RAND_MAX * 5.0, (float)rand() / RAND_MAX * 3.0, (float)rand() / RAND_MAX * 5.0,
	  	  	       (float)rand() / RAND_MAX * 7.0 + World.GetObject( i )->pos.x, (float)rand() / RAND_MAX * 7.0 + World.GetObject( i )->pos.y, (float)rand() / RAND_MAX * 3.0,
	  	  	       RandomRot.x, RandomRot.y, RandomRot.z, RandomRot.s
	  	  	        );
	  	  	 }
	  	  	 else
	  	  	 {
	  	  	 sprintf( SendBuffer, "<objectmove ireference=\"%i\"><faces><face num=\"0\"><color r=\"%f\" g=\"%f\" b=\"%f\"/></face></faces>"
	  	  	     "<geometry><scale x=\"%f\" y=\"%f\" z=\"%f\"/></geometry>"
	  	  	     "<dynamics><duration milliseconds=\"6000\"/></dynamics></objectmove>\n",
	  	  	       iReference, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX,
	  	  	       (float)rand() / RAND_MAX * 5.0, (float)rand() / RAND_MAX * 3.0, (float)rand() / RAND_MAX * 5.0
	  	  	        );
	  	  	}
	  	  	 printf( "Sending to server [%s]\n", SendBuffer );    
           SocketMetaverseServer.Send( SendBuffer );
	  	  }
	  }
	}
//	  Debug( "MoveObjectsInCircle() done\n" );
}

void MainLoop()
{
    while(1)
    {
//    	  Debug( "Blocking till next frame...\n" );
    	  SocketsBlockTillNextFrame();
  //  	  Debug( "Next frame now\n" );

   	   sprintf( ReadBuffer, "" );
       int ReadResult = SocketMetaverseServer.ReceiveLineIfAvailable( ReadBuffer );
       if( ReadResult == SOCKETS_READ_OK )
       {
      	   Debug( "Read buffer from server: [%s]\n", ReadBuffer );
          HandleServerInput( ReadBuffer );          
       }
       else if( ReadResult == SOCKETS_READ_SOCKETGONE )
       {
          printf( "server disconnected\n" );
          exit(1);
       }       

       if( MVGetTickCount() - iLastScriptingFrameTickCount > iTicksPerFrame )
       {
          MoveObjectsInCircle();
          iLastScriptingFrameTickCount = MVGetTickCount();
        }
    }
}  

int main(int argc, char *argv[]) {
   int argnum = 0;

   sprintf( sMetaverseServerIP, "127.0.0.1" );
   sprintf( sAvatarName, "Guest" );
   sprintf( sAvatarPassword, "" );

   for( argnum = 0; argnum < argc - 1; argnum++ )
   {
      if( strcmp( argv[ argnum ], "-s" ) == 0 )
      {
         sprintf( sMetaverseServerIP, argv[ argnum + 1] );       
         argnum++;
      }
      else if( strcmp( argv[ argnum ], "-u" ) == 0 )
      {
         sprintf( sAvatarName, argv[ argnum + 1] );       
         argnum++;
      }
      else if( strcmp( argv[ argnum ], "-p" ) == 0 )
      {
         sprintf( sAvatarPassword, argv[ argnum + 1] );       
         argnum++;
      }
      else if( strcmp( argv[ argnum ], "-?" ) == 0 || strcmp( argv[ argnum ], "-h" ) == 0  || strcmp( argv[ argnum ], "/h" ) == 0  || strcmp( argv[ argnum ], "/?" ) == 0 )
      {
         printf( "Usage: %s [ -s server IP ] -u user [ -p password ]\n", argv[0] );
         exit(1);
      }
   }

   printf( "Connecting to server at IP %s port %i\n", sMetaverseServerIP, iPortMetaverseServer );

    mvsocket::InitSocketSystem();

    printf( "Connecting to server...\n" );
    SocketMetaverseServer.Init();
    SocketMetaverseServer.ConnectToServer( inet_addr( sMetaverseServerIP ), iPortMetaverseServer );

    printf( "Attempting to login as %s...\n", sAvatarName );
    sprintf( SendBuffer, "<login name=\"%s\" password=\"%s\"/>\n", sAvatarName, sAvatarPassword );
    SocketMetaverseServer.Send( SendBuffer );

    bool bAuthenticated = false;
    int ReadResult = SOCKETS_READ_OK;
    while( ReadResult == SOCKETS_READ_OK && bAuthenticated == false )
    {
    	Debug( "reading from server...\n" );
    ReadResult = SocketMetaverseServer.ReceiveLineBlocking( ReadBuffer );
    	Debug( "read [%s]\n", ReadBuffer );
    
    if( ReadResult == SOCKETS_READ_OK )
    {
       TiXmlDocument IPC;
       IPC.Parse( ReadBuffer );
    	 if( strcmp( IPC.RootElement()->Value(), "loginaccept" ) == 0 )
    	 {
    	 	  printf( "Successfully logged in as %s\n", sAvatarName );
    	 	  iMyReference = atoi( IPC.RootElement()->Attribute( "ireference" ) );
    	 	  printf( "av reference = %i\n", iMyReference );
    	 	  bAuthenticated = true;
    	 }
    	 else if( strncmp( ReadBuffer, "<loginreject", strlen( "<loginreject" ) ) == 0 )
    	 {
    	 	  printf( "Login failed.  Please try again.\n", sAvatarName );
    	 	  exit(1);
    	 }
    }
    else
    {
    	  printf( "Failed to connect to server, or login failed. Please try again.\n" );
    	  exit(1);
    }
    }
    
    if( bAuthenticated == false )
    {
    	  printf( "Failed to connect to server, or login failed. Please try again.\n" );
    	  exit(1);
    }

    printf( "Initialization complete\n" );
    
    printf( "Requesting world state...\n" );
    
    SocketMetaverseServer.Send( "<requestworldstate />\n" );
    
    MainLoop();
    
    return 0;
}

