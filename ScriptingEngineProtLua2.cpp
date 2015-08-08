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
//! \brief This module is the Lua scripting engine main module.
//!
//! This module is the Lua scripting engine main module.
//! It is derived from the original module "scriptingengineprotlua.cpp"
//! Compared to scriptingengineprotlua.cpp, this module introduces threads to
//! ensure that no script can block another, and is basically largley more robust
//! compilation errors are passed up to scripter via a "Say" from object containing the script
//!
//! A mutex called EngineMutex is created in this module and used to prevent race conditions throughout
//! the lua scripting engine and lua scripting api modules
//! Basically, you should lock EngineMutex before doing *anything* and unlock it afterwards
//! Ideally we want the engine to run as single-threaded as possible, to prevent it exploding server resources
//! This module runs quite happily using the single-threaded MS runtime library (compile option /ML) because
//! the mutexes take care of preventing race conditions.
//!
//! lua can handle multithreading jsut fine.  Make sure that only one function is running at a time in each VM
//! We do this by queuing events up until the VM is free then calling the function correspdongin to the event at that time
//!
//! Note that for the moment the scriptingengine doesnt bind with odephysicsengine.dll, but it probably should do,
//! and it will do in the future.

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "pthread.h"

#include <stdio.h>
#include <math.h>
#include <map>

extern "C"
{
   #include <lua.h>
   #include "lauxlib.h"
   #include "lualib.h"
}

#include "tinyxml.h"

#include "port_list.h"

#include "WorldStorage.h"
#include "mvobjectstorage.h"
#include "SocketsClass.h"
#include "TickCount.h"
#include "Math.h"
#include "texturemanager.h"
#include "Animation.h"
#include "ScriptInfoCache.h"
#include "TerrainInfoCache.h"
#include "MeshInfoCache.h"
#include "ThreadWrapper.h"

#include "scriptingenginelua.h"
#include "LuaScriptingAPI.h"
#include "LuaScriptingAPIGod.h"
#include "LuaScriptingAPIHelper.h"
#include "LuaEventClass.h"

lua_State* luaVM;

mvWorldStorage World;  //!< stores current world state

//TextureInfoCache textureinfocache;
Animation animator( World );   //!< handles non-physical interpolated movement
ScriptInfoCacheClass ScriptInfoCache;    //!< stores scripts received
//TerrainCacheClass TerrainCache;
// MeshInfoCacheClass MeshInfoCache;

mvsocket SocketMetaverseServer;

char sMetaverseServerIP[64];
char sAvatarName[64] = "Guest";
char sAvatarPassword[64] = "";

//#define BUFSIZE 2047
char SendBuffer[ 2048 ];
char ReadBuffer[ 4097 ];

int iMyReference = 0;

int StartTickCount = 0;
const int iTicksPerFrame = 200;  //!< used to regulate resource utilisation by scripting engine

int LastTickCount = 0;

int iLastScriptingFrameTickCount;

int totalTimers = 0;
	
struct timerData
{
	int duration;
	int repeats;
	int rcount;
	int tickcount; 
	string function;	
	bool running;
	int iRef;
	bool active;
} timerList[2048];

int totalCaptures = 0;
	
struct captureData
{
	int iRef;
	int iowner;
	bool active;
} captureList[2048];

// These are probably deprecated and can be removed
float fAngle = 0;
float fRadius = 5.0;

pthread_mutex_t EngineMutex;    //!< The mutex to encourage this process to run as single-threaded as possible

map < int, ObjectVMInfoClass > ObjectVMs;   //!< stores all the VMs

map < int, EventInfo * > Events;   //!< This should probably be migrated to be a set, contains all queued events/function calls
int iNextEventNum = 1;

//! sends a message to the Metaverse server.  Function name should be changed really
void SendClientMessage( const char *message )
{	
	SocketMetaverseServer.Send( message );
}

//! waits till something arrives in the server socket or till next frame (15 milliseconds or so)
//! reduces resource wastage by scripting engine
void SocketsBlockTillNextFrame()
{
	vector<const mvsocket *> sockets;
	int iTicksSinceLastTime = MVGetTickCount() - LastTickCount;
	LastTickCount = MVGetTickCount();

	int iTicksTillNextFrame = iTicksPerFrame - iTicksSinceLastTime;
	if( iTicksTillNextFrame < 0 )
	{
		iTicksTillNextFrame = 0;
	}

	//	Debug( "ticks till next frame: %i\n", iTicksTillNextFrame );
//	timeval TimeOut;
//	TimeOut.tv_sec = 0;
//	TimeOut.tv_usec = iTicksTillNextFrame;

//	fd_set TargetSet;
//	FD_ZERO( &TargetSet );
//	FD_SET( SocketMetaverseServer.socket, &TargetSet);
//	select( SocketMetaverseServer.socket + 1, &TargetSet, NULL, NULL, &TimeOut );
	sockets.push_back(&SocketMetaverseServer);
	SocketsReadBlock(iTicksTillNextFrame, sockets);
}

//! Creates a new VM for file sScriptPath and object iObjectReference
//! iObjectReference is used to send a Say from the object with the results of the compilation: success or the error message 
lua_State *CreateVMFromScript( string sScriptPath, int iObjectReference )
{
   lua_State *pluaVM = lua_open();

   if( NULL == pluaVM )
   {
      printf("Error Initializing lua\n" );
      exit(1);
   }
  luaopen_math( pluaVM );
  luaopen_base( pluaVM);

  RegisterLuaStandardFunctions( pluaVM );
  
 // char sScriptPath[256];
 // sprintf( sScriptPath, "serverdata\\scripts\\%s", pElement->Attribute("serverfilename" ) );

	 DEBUG(  "loading file " << sScriptPath << " ..." ); // DEBUG
	 int loadresult = luaL_loadfile( pluaVM, sScriptPath.c_str() );	
	 if( loadresult == 0 )
	 {
   	  DEBUG(  " file loaded ok" ); // DEBUG
   	  LuaScriptingAPIHelper::SayFromObject( iObjectReference, "Script loaded ok" );
   	  lua_dofile( pluaVM, sScriptPath.c_str() );	  		  
	  }
	  else
	  {
		  string errmsg = lua_tostring( pluaVM, -1 );
	     DEBUG(  "error message: " << errmsg << " loadresult: " << loadresult ); 	
	    
		  char sErrorCode[33];				
		  sprintf( sErrorCode, "%i", loadresult );
	     LuaScriptingAPIHelper::SayFromObject( iObjectReference, "error message: " + errmsg + " loadresult: " + sErrorCode );
	  }
  
    return pluaVM;  
}

//! Creates a new VM from the script referenced in ScriptInfo (a filename) for the object iObjectReference
lua_State *CreateVMFromScriptInfo( SCRIPTINFO &rScriptInfo, int iObjectReference )
{
	 return CreateVMFromScript( "serverdata\\scripts\\" + rScriptInfo.sServerFilename, iObjectReference );
}

//! Updates VM associated with object referenced by XML pElement if script has been added/changed/deleted
void UpdateScriptsForObject( TiXmlElement *pElement )
{
	 int iObjectReference = atoi( pElement->Attribute("ireference") );
	 int iArrayNum = World.GetArrayNumForObjectReference( iObjectReference );

   if( iArrayNum != -1 )
   {
	  TiXmlHandle docHandle( pElement );

	 if( !docHandle.FirstChild("scripts").FirstChild("script").Element() || iArrayNum == -1 )
   {
   	  if( ObjectVMs.find( iObjectReference ) != ObjectVMs.end() )
   	  {
   	  		  // when sending updates without script this was disabling the vm's
           	  // wasn't sure if the updates for things like phantom should also
           	  // send script data again so for now I just comment out the calll
           	  // to .erase here.  objectrefreshdata and objectupdatedata both call this.
           	  
   	  	  //ObjectVMs.erase( iObjectReference );
   	  	  DEBUG(  "would removing vm for object " << iObjectReference ); // DEBUG
   	  }
   }
   else
   {
   	  string sNewScriptReference = pElement->FirstChildElement("scripts")->FirstChildElement("script")->Attribute("sscriptreference");
   	  
   	  if (sNewScriptReference != "")
   	  {
	   	  ObjectVMInfoClass *pObjectVMInfo = NULL;
	   	  
	   	  if( ObjectVMs.find( iObjectReference ) == ObjectVMs.end() )
	   	  {
	   	  	     DEBUG(  "creating new objectvminfo object..." ); // DEBUG
	   	     	  ObjectVMInfoClass NewVMInfo;
	   	     	  DEBUG(  "inserting into set..." ); // DEBUG
	   	     	  ObjectVMs.insert( pair< int, ObjectVMInfoClass > ( iObjectReference, NewVMInfo ) );
	   	     	  DEBUG(  "retrieving ref..." ); // DEBUG
	   	        pObjectVMInfo = &( ObjectVMs.find( iObjectReference )->second );
	   	     	  DEBUG(  "populating " << iObjectReference); // DEBUG
	   	        pObjectVMInfo->iObjectReference = iObjectReference;
	   	        pObjectVMInfo->pVM = NULL;
	   	        pObjectVMInfo->bVMInitialized = false;
	   	     	  DEBUG(  "inserting done" ); // DEBUG
	   	  }
	   	  else
	   	  {
	   	  		DEBUG(  "populating2 " << iObjectReference); // DEBUG
	      	   pObjectVMInfo = &(ObjectVMs.find( iObjectReference )->second );
	   	  }
	   	  
	   	  if( sNewScriptReference != pObjectVMInfo->sScriptReference )
	   	  {
	   	     DEBUG(  "updating vm for object " << iObjectReference ); // DEBUG
	   	     // ObjectVMs.erase( iObjectReference );
	   	     
	 	     	  if( pObjectVMInfo->bVMInitialized )
	 	     	  {
	      	  	  DEBUG(  "unloading old vm..." ); // DEBUG
	   	           lua_close( pObjectVMInfo->pVM );
	   	           pObjectVMInfo->pVM = NULL;
	  	           pObjectVMInfo->bVMInitialized = false;
	 	        }
	   	    
	   	     pObjectVMInfo->sScriptReference = sNewScriptReference;
	   	     if( ScriptInfoCache.Scripts.find( sNewScriptReference ) != ScriptInfoCache.Scripts.end() )
	   	     {
	      	  	 DEBUG(  "loading script into new vm..." ); // DEBUG
		          pObjectVMInfo->pVM = CreateVMFromScriptInfo( ScriptInfoCache.Scripts.find( sNewScriptReference )->second, iObjectReference );
	             LuaScriptingAPIHelper::AddObjectReferenceToVMRegistry( iObjectReference, pObjectVMInfo->pVM );
	
	             //pthread_mutex_unlock( &EngineMutex );
	             DEBUG("Queue Init event for VM # " << iObjectReference);             
	             EventInit *pEvent = new EventInit;                          
					 
					 pEvent->sEventName = "Init";
					 pEvent->sEventClass = "EventInit";
					 pEvent->iVMNum = iObjectReference; 
					
					 QueueEvent( pEvent );
					 
		          pObjectVMInfo->bVMInitialized = true;
		        }
	   	  }
   		}
   		else
   		{
   			ObjectVMs.erase( iObjectReference );
   	  	   DEBUG(  "** did remove vm for object " << iObjectReference ); // DEBUG
   	  	}
   	}
   }
}

//! deletes any VM associated with a deleted object
void PurgeScriptForDeletedObject( TiXmlElement *pElement )
{
	 int iObjectReference = atoi( pElement->Attribute("ireference") );
	 DEBUG(  "purging vm for deleted object ref " << iObjectReference ); // DEBUG
   ObjectVMs.erase( iObjectReference );	 
}

//! stores script file info received via XML in the script info cache
void CacheScriptInfoFromXML( TiXmlElement *pElement )
{
	 DEBUG(  "registering script info from xml" ); // DEBUG
	 SCRIPTINFO &ScriptInfo = ScriptInfoCache.AddInfoFromXML( pElement );
	 
	 for( ObjectVMIterator iterator = ObjectVMs.begin(); iterator != ObjectVMs.end(); iterator++ )
	 {
	 	  DEBUG(  "checking against unitilized vms: " << iterator->second.sScriptReference.c_str() << " " << pElement->Attribute("checksum") ); // DEBUG
	 	  if( !iterator->second.bVMInitialized 
	 	     && strcmp( iterator->second.sScriptReference.c_str(), pElement->Attribute("checksum") ) == 0 )
	 	  {
      	  	 DEBUG(  "loading script into vm for object ..." << iterator->first ); // DEBUG
	          iterator->second.pVM = CreateVMFromScriptInfo( ScriptInfo, iterator->first );
			    LuaScriptingAPIHelper::AddObjectReferenceToVMRegistry( iterator->first, iterator->second.pVM );			  
			  
			    DEBUG("Queue Scriptinit 2 for VM # " << iterator->first);
			    
             EventInit *pEvent = new EventInit;
				 
				 pEvent->sEventName = "Init";
				 pEvent->sEventClass = "EventInit";
				 pEvent->iVMNum = iterator->first; 
				
				 QueueEvent( pEvent );             
             
	          iterator->second.bVMInitialized = true;     	   
	 	  }
	 }
}

//! Runs as a separate thread each call.  Called by StartFunction.  Launches a single function/event on teh VM referenced
//! by the passed in iVMNum (which is the iReference of the object containing the script)
//! locks mutex as normal, then unlocks just prior to function call, to allow rest of program/scripts to run
//! on return, locks the mutex again, then unlocks it at end of function.
void *ChildThreadFunction( void *ptr )
{
	pthread_mutex_lock( &EngineMutex );

   int iVMNum = (int)ptr;
   ObjectVMInfoClass &ObjectVMInfo = ObjectVMs.find( iVMNum )->second;
   
   const EventInfo *pEvent = ObjectVMInfo.pEvent;

/*
   if( ObjectVMInfo.CurrentEvent.sFunctionName == "AsyncRPC" ) // temp fudge for proof of concept
   {
   	 lua_pushstring( ObjectVMInfo.pVM, ObjectVMInfo.CurrentEvent.sData.c_str() );

    	pthread_mutex_unlock( &EngineMutex );
     lua_pcall( ObjectVMInfo.pVM, 1, 0, 0 ); 
    	pthread_mutex_lock( &EngineMutex );
   }
   else
   {

   	 pthread_mutex_unlock( &EngineMutex );
     lua_pcall( ObjectVMInfo.pVM, 0, 0, 0 ); 
    	pthread_mutex_lock( &EngineMutex );
   }
   */
   
   string sEventName = ObjectVMInfo.pEvent->sEventName;
   
   if( ObjectVMInfo.pEvent->sEventName == "AsyncRPC" )
   {
       const EventInfoMulticastRPC *pRPCEvent = dynamic_cast< const EventInfoMulticastRPC *>( pEvent );
       
       lua_getglobal( ObjectVMInfo.pVM, pRPCEvent->sEventName.c_str() );
       
       if( LuaScriptingAPIHelper::GetTypename( ObjectVMInfo.pVM, -1 ) == "function" )
       {
		   	   lua_pushstring( ObjectVMInfo.pVM, pRPCEvent->sMessage.c_str() );
		   	   lua_pushnumber( ObjectVMInfo.pVM, pRPCEvent->iSenderReference );
					
					pthread_mutex_unlock( &EngineMutex );				   
					LuaScriptingAPIHelper::DoPCall(ObjectVMInfo.pVM, 2, 0, 0);
					
					pthread_mutex_lock( &EngineMutex );
		}
   }
   else if( ObjectVMInfo.pEvent->sEventName == "Click" )
   {
       const EventClick *pClickEvent = dynamic_cast< const EventClick *>( pEvent );
       
       lua_getglobal( ObjectVMInfo.pVM, pClickEvent->sEventName.c_str() );
       DEBUG("Calling click event in script");
       if( LuaScriptingAPIHelper::GetTypename( ObjectVMInfo.pVM, -1 ) == "function" )
       {
			string errmsg;
	   	    lua_pushnumber( ObjectVMInfo.pVM, pClickEvent->iClickerReference );
	   	       
			   pthread_mutex_unlock( &EngineMutex );	
				LuaScriptingAPIHelper::DoPCall(ObjectVMInfo.pVM, 1, 0, 0);
			   pthread_mutex_lock( &EngineMutex );
		}
   }
   else if( ObjectVMInfo.pEvent->sEventName == "KeyUp" )
   {
       const EventKeyUp *pKeyUpEvent = dynamic_cast< const EventKeyUp *>( pEvent );
       
       lua_getglobal( ObjectVMInfo.pVM, pKeyUpEvent->sEventName.c_str() );
       DEBUG("Calling KeyUp event in script");
       if( LuaScriptingAPIHelper::GetTypename( ObjectVMInfo.pVM, -1 ) == "function" )
       {
				string errmsg;
	   	   lua_pushstring( ObjectVMInfo.pVM, pKeyUpEvent->sValue.c_str() );
	   	   lua_pushnumber( ObjectVMInfo.pVM, pKeyUpEvent->iReference );
	   	       
			   pthread_mutex_unlock( &EngineMutex );	
				LuaScriptingAPIHelper::DoPCall(ObjectVMInfo.pVM, 2, 0, 0);
			   pthread_mutex_lock( &EngineMutex );
		}
   }
   else if( ObjectVMInfo.pEvent->sEventName == "KeyDown" )
   {
       const EventKeyDown *pKeyDownEvent = dynamic_cast< const EventKeyDown *>( pEvent );
       
       lua_getglobal( ObjectVMInfo.pVM, pKeyDownEvent->sEventName.c_str() );
       DEBUG("Calling KeyDown event in script");
       if( LuaScriptingAPIHelper::GetTypename( ObjectVMInfo.pVM, -1 ) == "function" )
       {
				string errmsg;
	   	   lua_pushstring( ObjectVMInfo.pVM, pKeyDownEvent->sValue.c_str() );
	   	   lua_pushnumber( ObjectVMInfo.pVM, pKeyDownEvent->iReference );
	   	       
			   pthread_mutex_unlock( &EngineMutex );	
				LuaScriptingAPIHelper::DoPCall(ObjectVMInfo.pVM, 2, 0, 0);
			   pthread_mutex_lock( &EngineMutex );
		}
   }
   else if( ObjectVMInfo.pEvent->sEventName == "CollisionEnd" )
   {
       const EventCollisionEnd *pCollisionEndEvent = dynamic_cast< const EventCollisionEnd *>( pEvent );
       
       lua_getglobal( ObjectVMInfo.pVM, pCollisionEndEvent->sEventName.c_str() );
       DEBUG("Calling CollisonEnd event in script");
       if( LuaScriptingAPIHelper::GetTypename( ObjectVMInfo.pVM, -1 ) == "function" )
       {				
	   	   lua_pushnumber( ObjectVMInfo.pVM, pCollisionEndEvent->iReference );
	   	       
			   pthread_mutex_unlock( &EngineMutex );	
				LuaScriptingAPIHelper::DoPCall(ObjectVMInfo.pVM, 1, 0, 0);
			   pthread_mutex_lock( &EngineMutex );
		}
   }
   else if( ObjectVMInfo.pEvent->sEventName == "CollisionStart" )
   {
       const EventCollisionStart *pCollisionStartEvent = dynamic_cast< const EventCollisionStart *>( pEvent );
       
       lua_getglobal( ObjectVMInfo.pVM, pCollisionStartEvent->sEventName.c_str() );
       DEBUG("Calling CollisionStart event in script");
       if( LuaScriptingAPIHelper::GetTypename( ObjectVMInfo.pVM, -1 ) == "function" )
       {				
	   	   lua_pushnumber( ObjectVMInfo.pVM, pCollisionStartEvent->iReference );
	   	       
			   pthread_mutex_unlock( &EngineMutex );	
				LuaScriptingAPIHelper::DoPCall(ObjectVMInfo.pVM, 1, 0, 0);
			   pthread_mutex_lock( &EngineMutex );
		}
   }
   else if( ObjectVMInfo.pEvent->sEventName == "UserData" )
   {
   	   const EventInfoUserData *pUserDataEvent = dynamic_cast< const EventInfoUserData * >( pEvent );
   	   
       lua_getglobal( ObjectVMInfo.pVM, pUserDataEvent->sEventName.c_str() );
       
       if( LuaScriptingAPIHelper::GetTypename( ObjectVMInfo.pVM, -1 ) == "function" )
       {
	   	   lua_pushstring( ObjectVMInfo.pVM, pUserDataEvent->sClientSideReference.c_str() );
	   	   lua_pushnumber( ObjectVMInfo.pVM, pUserDataEvent->iOwner );
	   	   lua_pushstring( ObjectVMInfo.pVM, pUserDataEvent->sStore.c_str() );
	   	   lua_pushstring( ObjectVMInfo.pVM, pUserDataEvent->sKey.c_str() );
	   	   lua_pushstring( ObjectVMInfo.pVM, pUserDataEvent->sData.c_str() );
	   	   
			   pthread_mutex_unlock( &EngineMutex );	
				LuaScriptingAPIHelper::DoPCall(ObjectVMInfo.pVM, 5, 0, 0);
			   pthread_mutex_lock( &EngineMutex );   	  
		}
   }
   else
   {
       lua_getglobal( ObjectVMInfo.pVM, pEvent->sEventName.c_str() );
   	   
       if( LuaScriptingAPIHelper::GetTypename( ObjectVMInfo.pVM, -1 ) == "function" )
       {
			   pthread_mutex_unlock( &EngineMutex );			   
				LuaScriptingAPIHelper::DoPCall(ObjectVMInfo.pVM, 0, 0, 0);
			   pthread_mutex_lock( &EngineMutex );   	   
		}
   }

   DEBUG(  "returned from lua function, VM " << sEventName << " " << iVMNum ); // DEBUG
	 ObjectVMInfo.bVMIsRunning = false;
	 
   delete pEvent;
   pEvent = 0;
   ObjectVMInfo.pEvent = 0;
	 
	pthread_mutex_unlock( &EngineMutex );

  return 0;
}

// starts a function/event on a VM.  The VM should already have been checked to ensure its not running prior to calling this
// this will start a new thread for this VM function call
// DEPRICATED in favor of StartEvent
/*
void _StartFunction( int iVMNum, string sFunctionName, string sData = "" )
{

  if( ObjectVMs.find( iVMNum ) != ObjectVMs.end() )
  {   
	    //pthread_mutex_lock( &EngineMutex );
	
	    ObjectVMInfoClass &ObjectVMInfo = ObjectVMs.find( iVMNum )->second;
			ObjectVMInfo.bVMIsRunning = true;

			ObjectVMInfo.CurrentEvent.iVMNum = iVMNum;
			ObjectVMInfo.CurrentEvent.sFunctionName = sFunctionName;
			ObjectVMInfo.CurrentEvent.sData = sData;
			
			DEBUG(  "StartFunction() starting function " << sFunctionName << " on vm " << iVMNum ); // DEBUG
		  lua_getglobal( ObjectVMInfo.pVM, sFunctionName.c_str() );
		  pthread_create(&(ObjectVMInfo.threadobject), 0, ChildThreadFunction, (void *)iVMNum );	
			DEBUG(  "StartFunction() ...done" ); // DEBUG
		
			//pthread_mutex_unlock( &EngineMutex );
  }
}

// queues an event to a VM.  If the VM isnt currently executing, launches the event/function directly
// DEPRICATED in favor of QueueEvent
void _QueueFunction( int iVMNum, string sFunctionName, string sData )
{
   ObjectVMInfoClass &ObjectVMInfo = ObjectVMs.find( iVMNum )->second;

	 if( ObjectVMInfo.bVMIsRunning == false )
	 {
	 	  StartFunction( iVMNum, sFunctionName, sData );
	 }
	 else
	 {
	 	  EventInfo *pEvent = new EventInfo;
	 	  pEvent->iVMNum = iVMNum;
	 	  pEvent->sFunctionName = sFunctionName;
	 	  pEvent->sData = sData;
	 	  DEBUG(  "QueueFunction() " << iVMNum << " " << sFunctionName << " [" << sData << "]" ); // DEBUG
	 	  Events.insert( pair< int, EventInfo * >( iNextEventNum, pEvent ) );
	 	  iNextEventNum++;
	 }
}
*/

//! Start running an event on its targeted VM
//! assuming that VM is currently available (not running another event)
void StartEvent( const EventInfo *pEvent )
{

	if( ObjectVMs.find( pEvent->iVMNum ) != ObjectVMs.end() )
	{     			
		ObjectVMInfoClass &ObjectVMInfo = ObjectVMs.find( pEvent->iVMNum )->second;	    
		ObjectVMInfo.bVMIsRunning = true;		 
		ObjectVMInfo.pEvent = pEvent;		
		DEBUG(  "StartEvent() starting thread for event name " << pEvent->sEventName << " on vm " << pEvent->iVMNum ); // DEBUG		
		pthread_create(&(ObjectVMInfo.threadobject), 0, ChildThreadFunction, (void *)pEvent->iVMNum );	
		DEBUG(  "StartEvent() ...done" ); // DEBUG
		pthread_mutex_unlock( &EngineMutex );
	}
	else
	{
  		DEBUG(  "could not find vm for StartEvent() " << pEvent->sEventName << " on vm " << pEvent->iVMNum ); // DEBUG
	}
}

//! Adds an event to the global event queue
//! If the targeted VM is currently available, start running event straight
//! away
void QueueEvent( EventInfo *pEvent )
{
	ObjectVMInfoClass &ObjectVMInfo = ObjectVMs.find( pEvent->iVMNum )->second;

	if( ObjectVMInfo.bVMIsRunning == false )
	{
		DEBUG(  "StartEvent() " << pEvent->iVMNum << " " << pEvent->sEventName ); // DEBUG
	 	StartEvent( pEvent );
	}
	else
	{
		//EventInfo *pEvent = new EventInfo;
	 	//pEvent->iVMNum = iVMNum;
	 	//pEvent->sFunctionName = sFunctionName;
	 	//pEvent->sData = sData;
	 	DEBUG(  "QueueEvent() " << pEvent->iVMNum << " " << pEvent->sEventName ); // DEBUG
	 	Events.insert( pair< int, EventInfo * >( iNextEventNum, pEvent ) );
	 	iNextEventNum++;
	}
}

//! Starts queued events on each VM, as long as the VM isnt running currently
//! otherwise leaves the event in the queue for that VM
//! (only one function should be running in a VM at a time)
void StartQueuedFunctions()
{
//	DEBUG(  "StartQueuedFunctions() iterating over queued functions..." ); // DEBUG
	for( map< int, EventInfo * >::iterator iterator = Events.begin(); iterator != Events.end(); iterator++ )
	{
		 if( ObjectVMs.find( iterator->second->iVMNum ) != ObjectVMs.end() )
		 {
	     ObjectVMInfoClass &ObjectVMInfo = ObjectVMs.find( iterator->second->iVMNum )->second;
			 if( !ObjectVMInfo.bVMIsRunning )
			 {
			 	  DEBUG(  "StartQueuedFunctions() starting function " << iterator->second->sEventName << " on vm " << iterator->second->iVMNum ); // DEBUG
			 	  StartEvent( iterator->second );
			 	  DEBUG(  "StartQueuedFunctions() ...function started, removing from queue..." ); // DEBUG
			 	  // delete iterator->second;
			 	  map< int, EventInfo * >::iterator nextiterator = iterator;
			 	  nextiterator++;
			 	  Events.erase( iterator );
			 	  iterator = nextiterator;
			 	  DEBUG(  "StartQueuedFunctions() ...removed from queue ok." ); // DEBUG
			 } 
		}
		else
		{
			 DEBUG(  "StartQueuedFunctions() dropping event for non-existant vm " <<  iterator->second->sEventName << " on vm " << iterator->second->iVMNum );
			 delete iterator->second;
		 	  map< int, EventInfo * >::iterator nextiterator = iterator;
		 	  nextiterator++;
		 	  Events.erase( iterator );
		 	  iterator = nextiterator;
			 DEBUG(  "dropped" ); // DEBUG
		}
	}
//	DEBUG(  "StartQueuedFunctions() done" ); // DEBUG
}

//! Calls the function "Timer" on each active VM
void RunTimerEvents()
{	
	int iCurrentTickCount = MVGetTickCount();
	
	for (int i = 0; i < totalTimers; i++)
	{		
		if (timerList[i].active && timerList[i].running)
		{			
			if (iCurrentTickCount - timerList[i].tickcount > timerList[i].duration)
			{
				timerList[i].tickcount = iCurrentTickCount;
				//ObjectVMInfoClass *pObjectVMInfo = NULL;
				if (ObjectVMs.find(  timerList[i].iRef )->second.bVMInitialized )
				{
					if (timerList[i].rcount > 0)
					{
						timerList[i].rcount--;
						if (timerList[i].rcount == 0)
						{
							timerList[i].running = false;
						}
					}
					
					EventTimer *pEvent = new EventTimer;
				 
					pEvent->sEventName = timerList[i].function;
					pEvent->sEventClass = "EventTimer";
					pEvent->iVMNum = timerList[i].iRef; 
					
					QueueEvent( pEvent );
					
					DEBUG(  "RunTimerEvents() " << timerList[i].function << " in " << timerList[i].iRef << " ...done" );
				}
				else
				{
					timerList[i].active = false;
					//DEBUG("Timer " << timerList[i].iRef << " not initialized " << i);
				}
			}
		}
	}			  
}

//! Passes on collision start event from passed-in XML message
//! to appropriate VM (if any)
void SendCollisionStart ( TiXmlElement *pElement )
{
	Debug("Inside CollisionStart event");
	int ireference = atoi(pElement->Attribute("ireference"));	
	int itarget = atoi(pElement->Attribute("itarget"));	
	if (ObjectVMs.find(itarget)->second.bVMInitialized )
	{					
		EventCollisionStart *pEvent = new EventCollisionStart;
	 
		pEvent->sEventName = "CollisionStart";
		pEvent->sEventClass = "EventCollisionStart";		
		pEvent->iReference = ireference;
		pEvent->iVMNum = itarget;
				
		QueueEvent( pEvent );		
	}
}

//! Passes on collision start event from passed-in XML message
//! to appropriate VM (if any)
void SendCollisionEnd ( TiXmlElement *pElement )
{
	Debug("Inside CollisionEnd event");
	int ireference = atoi(pElement->Attribute("ireference"));	
	int itarget = atoi(pElement->Attribute("itarget"));	
	if (ObjectVMs.find(itarget)->second.bVMInitialized )
	{					
		EventCollisionEnd *pEvent = new EventCollisionEnd;
	 
		pEvent->sEventName = "CollisionEnd";
		pEvent->sEventClass = "EventCollisionEnd";		
		pEvent->iReference = ireference;
		pEvent->iVMNum = itarget;
				
		QueueEvent( pEvent );		
	}
}

//! Passes on keydown event from passed-in XML message
//! to appropriate VM (if any)
void SendKeyDown ( TiXmlElement *pElement )
{
	Debug("Inside KeyDown event");
	string sValue = pElement->Attribute("value");
	int iowner = atoi(pElement->Attribute("iowner"));	
	for (int i = 0; i < totalCaptures; i++)
	{		
		if ((captureList[i].iowner == iowner) && (captureList[i].active))
		{	
			if (ObjectVMs.find(captureList[i].iRef)->second.bVMInitialized )
			{					
				EventKeyDown *pEvent = new EventKeyDown;
			 
				pEvent->sEventName = "KeyDown";
				pEvent->sEventClass = "EventKeyDown";
				pEvent->sValue = sValue;
				pEvent->iReference = iowner;
				pEvent->iVMNum = captureList[i].iRef; 
				
				QueueEvent( pEvent );
				
				DEBUG(  "KeyDownEvent() in " << captureList[i].iRef << " sval : " << sValue << " iowner " << iowner << " ...done" );
				
			}
		}
	}			  
}

//! Passes on keyup event from passed-in XML message
//! to appropriate VM (if any)
void SendKeyUp ( TiXmlElement *pElement )
{	
	string sValue = pElement->Attribute("value");
	int iowner = atoi(pElement->Attribute("iowner"));
	for (int i = 0; i < totalCaptures; i++)
	{		
		if ((captureList[i].iowner == iowner) && (captureList[i].active))
		{	
			if (ObjectVMs.find(captureList[i].iRef)->second.bVMInitialized )
			{					
				EventKeyUp *pEvent = new EventKeyUp;
			 
				pEvent->sEventName = "KeyUp";
				pEvent->sEventClass = "EventKeyUp";
				pEvent->sValue = sValue;
				pEvent->iReference = iowner;				
				pEvent->iVMNum = captureList[i].iRef; 
				
				QueueEvent( pEvent );
				
				DEBUG(  "KeyUpEvent() in " << captureList[i].iRef << " sval : " << sValue << " iowner " << iowner << " ...done" );
				
			}
		}
	}			  
}

//! handles events recevied from server
//! currently, this handles the clickdown event, which occurs when an avatar clicks on an object
//! if there is a script associated with that object, and it has a Click event, that event is queued for the associated VM
void HandleEvent( TiXmlElement *pElement )
{
	Debug("Inside SCRIPT ENGINE handle event");
	 string sType = pElement->Attribute("type" );
	 int iTargetReference = atoi( pElement->Attribute("ireference") );
	 int iClickerReference = atoi( pElement->Attribute("iowner") );
	 if( sType == "clickdown" )
	 {
	 	 DEBUG(  "clickdown event received" ); // DEBUG
	 	 
	 	 EventClick *pEvent = new EventClick;
	 	 
	    pEvent->sEventName = "Click";
	    pEvent->sEventClass = "EventClick";
	    pEvent->iVMNum = iTargetReference;
	    pEvent->iClickerReference = iClickerReference;

      QueueEvent( pEvent );
	 	 
	 	 /*
     for( map< int, ObjectVMInfoClass >::iterator iterator = ObjectVMs.begin(); iterator != ObjectVMs.end(); iterator++ )
     {
     	 if( iterator->second.bVMInitialized )
     	 {
   	 	  if( iterator->second.iObjectReference == iTargetReference )
   	 	  {
   	 	  	 DEBUG(  "attempting to luanch Click functin on VM " << iterator->first ); // DEBUG
		     	 lua_State *pluaVM = iterator->second.pVM;
		      lua_getglobal( pluaVM, "Click" );
		      DEBUG(  "typename of top of stack: " << LuaScriptingAPIHelper::GetTypename( pluaVM, -1 ) ); // DEBUG
		      if( LuaScriptingAPIHelper::GetTypename( pluaVM, -1 ) == "function" )
		      {
  		     	 DEBUG(  "running click event on VM " << iterator->first ); // DEBUG
		         lua_pushnumber( pluaVM, (float)iClickerReference );
		         lua_pcall( pluaVM, 1, 0, 0 );  
		         DEBUG(  "end run" ); // DEBUG
		      }
        }
      }
     }	 	  
      */
	 }
}

//! Processes inforesponse message (from database usually),
//! generating a new Event if necessary
void HandleInfoResponse( TiXmlElement *pElement )
{
	 string sType = pElement->Attribute("type" );
	 if( sType == "DBUSERDATA" )
	 {
	 	   int iClientReferenceNum = atoi( pElement->Attribute( "ireference" ) );

			 string sClientSideReference = pElement->Attribute("clientsidereference" );
			 int iOwner = atoi( pElement->Attribute("idataowner" ) );
			 string sStore = pElement->Attribute("store" );
			 string sKey = pElement->Attribute("valuename" );
			 string sData = pElement->Attribute("value" );
			 
			 EventInfoUserData *pEvent = new EventInfoUserData;
			 
			 pEvent->sEventName = "UserData";
			 pEvent->sEventClass = "EventInfoUserData";
			 pEvent->iVMNum = iClientReferenceNum;
			 pEvent->sClientSideReference = sClientSideReference;
			 pEvent->iOwner = iOwner;
			 pEvent->sStore = sStore;
			 pEvent->sData = sData;

	     QueueEvent( pEvent );
	 }
}

//! handles XML input from server, such as object updates, news of new scripts and so on
void HandleServerInput( char *ReadBuffer )
{
   	   if( ReadBuffer[0] == '<' )
   	   {
   	   	   DEBUG( "XML IPC received from server " << ReadBuffer );
   	   	   
           TiXmlDocument IPC;
           IPC.Parse( ReadBuffer );
           TiXmlElement *pElement = IPC.RootElement();
           if( strcmp( pElement->Value(), "objectrefreshdata" ) == 0 )
           {
               World.StoreObjectXML( IPC.RootElement() );
           	   UpdateScriptsForObject( IPC.RootElement() );
           }
           else if( strcmp( pElement->Value(), "objectcreate" ) == 0 ) 
           {
               World.StoreObjectXML( IPC.RootElement() );       	   
           }
           else if( strcmp( pElement->Value(), "event" ) == 0 ) 
           {
               HandleEvent( IPC.RootElement() );       	   
           }
           else if( strcmp( pElement->Value(), "inforesponse" ) == 0 ) 
           {
           	   HandleInfoResponse( IPC.RootElement() );       	   
           }
           else if( strcmp( pElement->Value(), "objectupdate" ) == 0 )
           {             	  
           	  UpdateScriptsForObject( IPC.RootElement() );         	  
           	  World.UpdateObjectXML( IPC.RootElement() );           	  
           }
           else if( strcmp( pElement->Value(), "objectdelete" ) == 0 ) 
           {
               World.DeleteObjectXML( IPC.RootElement() );       	   
            	  PurgeScriptForDeletedObject( IPC.RootElement() );
           }
           else if( strcmp( pElement->Value(), "objectmove" ) == 0 ) 
           {
               animator.MoveObject( pElement );       	   
               //World.MoveObjectXML( IPC.RootElement() );       	   
           }
           else if( strcmp( pElement->Value(), "script" ) == 0 )
           {
           	   CacheScriptInfoFromXML( pElement );
           }
           else if( strcmp( pElement->Value(), "loginaccept" ) == 0 )
           {
           	  iMyReference = atoi( pElement->Attribute("ireference" ) );
           }
           else if( strcmp( pElement->Value(), "keyup" ) == 0 )
           {           	  
           	  SendKeyUp(IPC.RootElement() );
           }
           else if( strcmp( pElement->Value(), "keydown" ) == 0 )
           {           	  
           	  SendKeyDown(IPC.RootElement() );
           }
           else if( strcmp( pElement->Value(), "collisionstart" ) == 0 )
           {
           		SendCollisionStart(IPC.RootElement() );
           }
           else if( strcmp( pElement->Value(), "collisionend" ) == 0 )
           {
           		SendCollisionEnd(IPC.RootElement() );
           }
   	  }
   	  else
   	  {
   	   	   Debug( "Legacy IPC received from server [%s]\n", ReadBuffer );
       }	 
}

//! mainloop
void MainLoop()
{
	   pthread_mutex_init(&EngineMutex, 0);       

    while(1)
    {
//    	  Debug( "Blocking till next frame...\n" );
    	  SocketsBlockTillNextFrame();
  //  	  Debug( "Next frame now\n" );

       pthread_mutex_lock( &EngineMutex );
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
          RunTimerEvents();
          iLastScriptingFrameTickCount = MVGetTickCount();
       }
       StartQueuedFunctions();
      	pthread_mutex_unlock( &EngineMutex );

         // Not sure if this is strictly necessary but doesnt do any harm for now
         PauseThreadMilliseconds( 100 );
			  //struct timespec delay;
			  //   delay.tv_sec = 0;
			  //   delay.tv_nsec = 100 * 1000 * 1000;
			  //   pthread_delay_np( &delay );
    }
}  

//! initialization
int main(int argc, char *argv[]) {
   int argnum = 0;
   
  // SetupLua();

   ScriptInfoCache.Scripts.clear();
   ObjectVMs.clear();

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

