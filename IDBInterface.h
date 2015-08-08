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

//! \file 
//! \brief interface class for any dbinterface implementation, such as mysqldbinterface.

//! public interface class for dbabstractionlayer
//! created in order to reduce link dependencies
//! see dbabstractionlayer.h for documentation

#ifndef _IDBABSTRACTIONLAYER_H
#define _IDBABSTRACTIONLAYER_H

//! IDBInterface is the interface class for mysqldbinterface

//! IDBInterface is the interface class for any dbinterface implementation, such as mysqldbinterface.
class IDBInterface
{
public:
		virtual void ExecuteSQL( const char *SQL, ... ) = 0;                      //!< just execute some SQL directly, we dont want any results
		
		virtual void RunOneRowQuery( const char *SQL, ... ) = 0;                  //!< execute some SQL, we only want the first row
		
		virtual void RunMultiRowQuery( const char *SQL, ... ) = 0;                 //!< we want to get many rows of results
		virtual void NextRow() = 0;                                                //!< next row...
		virtual void EndMultiRowQuery() = 0;                                       //!< clean up once all rows fetched
		
		virtual const char *GetFieldValueByName( const char *sName ) = 0;  //!< returns the value of one field/column in a row, by name
		virtual bool RowAvailable() = 0;   //!< returns true if another row of data available
		
		virtual void DBConnect( const char *Host, const char *DBName, const char *username = "root", const char *password = "" ) = 0;                   //!< connect to db. Pass in name of database
		virtual void DisconnectDB() = 0;  //!< Disconnect from DB
		
		virtual char *GetQuerySQL() = 0;
};

#endif // _IDBABSTRACTIONLAYER_H
