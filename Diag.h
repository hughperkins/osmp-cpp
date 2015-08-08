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

// Modified Hugh Perkins 20050416 to remove ERROR definition conflict

//! \file
//! \brief Contains debugging functions, to write debugging info to stdout in a configurable way

#ifndef _DIAG_H
#define _DIAG_H

#include <iostream>
using namespace std;

#define _LOG // nb: define this to enable compilation of logging macros
#include "log/log.hpp"
//#define _NOLOGGINGLIB

//#define DEFINE_MSG_GROUP(a,b,c)
#ifdef LOG_TIMESTAMP
extern char* sztime();	// defined in diagconsole.cpp
#define DEBUG(a) _DEBUG( "Debug: " << sztime() << ": " << a );
#define TIMING(a) _TIMING( "Timing: "  << sztime() << ": " << a );
#else
#define DEBUG(a) _DEBUG( "Debug: " << a );
#define TIMING(a) _TIMING( "Timing: " << a );
#endif
#define INFO(a) _INFO( a );
#define WARNING(a) _WARNING( "Warning: " );
#define ERRORMSG(a) _ERRORMSG( "Error: " );

// *******************************************************************************************

DEFINE_MSG_GROUP( CDebugMsgGroup , "DEBUG" , "DEBUG message group." ) 

DEFINE_MSG_GROUP( CInfoMsgGroup , "INFO" , "INFO message group." ) 
DEFINE_MSG_GROUP( CWarningMsgGroup , "WARNING" , "WARNING message group." ) 
DEFINE_MSG_GROUP( CErrorMsgGroup , "ERROR" , "ERROR message group." ) 
DEFINE_MSG_GROUP( CTimingMsgGroup , "TIMING" , "TIMING message group." ) 

// Following functions should be used for debug in OSMP:
#define _DEBUG( msg )     LOG_GMSG( CDebugMsgGroup , msg )
#define _TIMING( msg )     LOG_GMSG( CTimingMsgGroup , msg )

#define _INFO( msg )     LOG_GMSG( CInfoMsgGroup , msg )
#define _WARNING( msg )     LOG_GMSG( CWarningMsgGroup , msg )
#define _ERRORMSG( msg )     LOG_GMSG( CErrorMsgGroup , msg )

// *******************************************************************************************



// *******************************************************************************************

// Following functions are DEPRECATED and should not be used (dangerous because not type-safe)

void DebugInit();  //!< Deprecated, dont use
void Debug( char *sMessage, ... );  //!< Deprecated, dont use
void SignalCriticalError( char *sMessage, ... );  //!< Deprecated, dont use

// *******************************************************************************************


#endif // _DIAG_H
