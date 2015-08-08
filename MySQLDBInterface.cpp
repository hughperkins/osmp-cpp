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
//! \brief The dbabstractionlayer abstracts MySQL (or other RDBMS if we change) functions to generic functions

// see header file dbabstractionlayer.h for documentation

// Modified Hugh Perkins 20050416 to replace mysql_real_query by mysql_query

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include <stdlib.h>
#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "Diag.h"
#include "MySQLDBInterface.h"

void MySQLDBInterface::DisconnectDB()
{
  mysql_close(&metaversedb);
}

void MySQLDBInterface::DBConnect( const char *Host, const char *DBName, const char *username, const char *password )
{
  mysql_init( &metaversedb );

 if(!mysql_real_connect(&metaversedb, Host, username, password, DBName, 0, NULL, 0 )){
 //if(!mysql_connect(&metaversedb, "", "root", "" )){
    printf(mysql_error(&metaversedb));
    exit(1);
}

if(mysql_select_db(&metaversedb, DBName)){
    printf(mysql_error(&metaversedb));
    exit(1);
}
printf( "Database connected\n" );
}

char *MySQLDBInterface::GetQuerySQL()
{
	 return query;
}
/*
MYSQL_ROW *GetCurrentRow()
{
	 return &row;
}
*/
const char *MySQLDBInterface::GetFieldValueByName( const char *sName )
{
	DEBUG( "GetFieldValueByName " << sName );
	DEBUG( iNumFields );

	for( int i = 0; i < iNumFields; i++ )
	{
		 if( strcmp( FieldNames[ i ], sName ) == 0 )
		 {
		 	  DEBUG( "field num is " << i );
		 	  //DEBUG( row );
		 	  //DEBUG( row[ i ][0] );
		 	  //DEBUG( row[ i ][1] );
		 	  //DEBUG( row[ i ][2] );
		 	  //DEBUG( row[ i ][3] );
		 	  //DEBUG( row[ i ][4] );
		 	  //DEBUG( row[ i ] );
		 	  return row[ i ];
		 }
	}
  DEBUG( "WARNING: field " << sName << " not found in query " << query );
  exit(1);
	return "";
}

void MySQLDBInterface::GetFieldNames()
{
	MYSQL_FIELD *pNextField;
	iNumFields = 0;
	pNextField = mysql_fetch_field( result );
	while( pNextField != NULL )
	{
		sprintf( FieldNames[ iNumFields ], pNextField->name );
		 printf( "field %i [%s]\n", iNumFields, pNextField->name );
   	iNumFields++;
	pNextField = mysql_fetch_field( result );
	}
}

void MySQLDBInterface::ExecuteSQL( const char *SQL, ... )
{
      va_list args;
   va_start( args, SQL );
  vsprintf(query, SQL, args );
  DEBUG( "Executing sql [%s]..." << query );
  if(mysql_query(&metaversedb, query )){ /* Make query */
     printf(mysql_error(&metaversedb));
     exit(1);
  }
}

void MySQLDBInterface::EndMultiRowQuery()
{
  mysql_free_result(result);
}

void MySQLDBInterface::RunMultiRowQuery( const char *SQL, ... )
{
  DEBUG( "RunMultiRowQuery() >>>" );

      va_list args;
   va_start( args, SQL );
  vsprintf(query, SQL, args );
   DEBUG( "running query [" << query << "]" );
  if(mysql_query(&metaversedb, query )){ /* Make query */
  	 DEBUG( "Error running mysql real query" );
     printf(mysql_error(&metaversedb));
     exit(1);
  }
   DEBUG( "getting result..." );
  result=mysql_store_result(&metaversedb); /* Download result from server */
   DEBUG( "fetching row..." );
  row=mysql_fetch_row(result); /* Get a row from the results */
  GetFieldNames();

  DEBUG( "RunMultiRowQuery() <<<" );

  //iNumFields = mysql_num_fields(result);
  //debug( "number of fields retrieved: %i\n", iNumFields );
  //p_Fields = mysql_fetch_fields(result);
}

void MySQLDBInterface::RunOneRowQuery( const char *SQL, ... )
{
      va_list args;
   va_start( args, SQL );
  vsprintf(query, SQL, args );
  RunMultiRowQuery( query );
  EndMultiRowQuery();
}

bool MySQLDBInterface::RowAvailable()
{
	if( row != NULL )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MySQLDBInterface::NextRow()
{
  row=mysql_fetch_row(result); /* Get a row from the results */
}

