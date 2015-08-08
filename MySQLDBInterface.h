// Copyright Hugh Perkins 2004, 2005, 2006
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
//! \brief Implementation class for IDBInterface, for MySQL databases.
//!
//! TMySQLDBInterface abstracts MySQL (or other RDBMS if we change) functions to generic functions
//! so that if we migrate to another RDBMS, theres only this module to chagne, in theory
//! it also helps out a bit, for example MySQL makes it hard to retrieve fields by name
//! in a single command, so we create that command here
//!
//! This class implements the IDBInterface interface class

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifndef _MYSQLDBINTERFACE_H
#define _MYSQLDBINTERFACE_H

#include "mysql.h"

#include "IDBInterface.h"

//! Implementation class for IDBInterface, for MySQL databases.

//! MySQLDBInterface abstracts MySQL (or other RDBMS if we change) functions to
//! generic functions
//! so that if we migrate to another RDBMS, theres only this module to chagne, in theory
//! it also helps out a bit, for example MySQL makes it hard to retrieve fields by name
//! in a single command, so we create that command here
//!
//! This class implements the IDBInterface interface class
//!
//! For all the query functions, including ExecuteSQL, you can pass in a printf-style string
//! followed by a list of arguments, ie you can use %s etc in the string, and add the insertion string as an additional argument
//! into the function
//!
//! To use this class:
//!
//! First connect to the database:
//!  - call DBConnect, passing in the hostname, database name, password and username
//!
//! At the end you will call disconnect:
//!  - call DisconnectDB();
//!
//! If you want to execute a one off SQL statement:
//!   - call ExecuteSQL, passing in your SQL string
//! 
//! If you want to execute an SQL query, and receive one row:
//!   - call RunOneRowQuery, passing in your SQL query
//!   - you can call RowAvailable() to check for received data
//!   - you can use GetFieldValueByName to receive each value in the row, passing in the field name as a string
//!
//! If you want to execute an SQL query, and receive many rows:
//!   - call RunMultiRowQuery, passing in your SQL query
//!   - you can call RowAvailable() to check for received data
//!   - you can use GetFieldValueByName to receive each value in the row, passing in the field name as a string
//!   - you can call NextRow to get the next row (then GetFieldValueByName)
//!   - you can call EndMultiRowQuery at the end, to free resources
class MySQLDBInterface : public IDBInterface
{
public:
		virtual void ExecuteSQL( const char *SQL, ... );                      //!< just execute some SQL directly, we dont want any results
		
		virtual void RunOneRowQuery( const char *SQL, ... );                  //!< execute some SQL, we only want the first row
		
		virtual void RunMultiRowQuery( const char *SQL, ... );                 //!< we want to get many rows of results
		virtual void NextRow();                                                //!< next row...
		virtual void EndMultiRowQuery();                                       //!< clean up once all rows fetched
		
		virtual const char *GetFieldValueByName( const char *sName );    //!< gets the field value (as a string) for the field with specified name
		virtual bool RowAvailable();                          //!< returns true/false if row is available or not
		
		virtual void DBConnect( const char *Host, const char *DBName, const char *username = "root", const char *password = "" );                   //!< connect to db. Pass in name of database
		virtual void DisconnectDB();  //!< Disconnect from database
		
		virtual char *GetQuerySQL();  //!< Returns query SQL
protected:
	MYSQL metaversedb;
	
	char query[1024];  //!< holds SQL query
	MYSQL_RES *result; //!< To be used to fetch information into
	MYSQL_ROW row;  //!< MYSQL row from last query
	unsigned int iNumFields;  //!< Number of fields in last query row
	//MYSQL_FIELD *p_Fields;
	char FieldNames[30][33]; //!< List of field names for last query returned row
	
	virtual void GetFieldNames();  //!< Populates FieldNames array with field names for last query row
};

#endif //_MYSQLDBINTERFACE_H

