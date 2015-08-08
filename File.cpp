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
//! \brief This module contains file functions that may be non-portable

// For documentation see header file

#ifdef _WIN32
#include <windows.h>
#endif

//#ifdef USINGBOOST  // Boost
//#include <boost/filesystem/operations.hpp>
//#include <boost/filesystem/exception.hpp>
//namespace fs = boost::filesystem;
//
//#else  // wxWidgets
//#include "wx/wx.h"
//#include "wx/filename.h"
//#include "wx/file.h"
//
//#endif

#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#endif

#include <iostream>
using namespace std;

#include "Diag.h"
#include "File.h"

void GetTempName( char *TempName, const char *Prefix )
{
    //#ifdef USINGBOOST
    //  sprintf( TempName, "%sXXXXXX", Prefix );
    //  _mktemp( TempName );
    //#else // wxwindows
    // wxString wxTempName = wxFileName::CreateTempFileName( Prefix, NULL );
    //  sprintf( TempName, wxTempName.c_str() );
    //#endif

#ifdef _WIN32
    //    char template[ 512 + 1 ];
    GetTempPath( 512, TempName );
    sprintf( TempName, "%s%sXXXXXX", TempName, Prefix );
    _mktemp( TempName );
#else

    sprintf( TempName, "%s%sXXXXXX", P_tmpdir, Prefix );
    mktemp( TempName );
#endif
}

//string mvGetTempPath( char *sTempPath )
//{
//#ifdef USINGBOOST
//  fs::path TempPath = fs::complete( fs::initial_path(), "temp" );
//  return TempPath.string();
//#else // wxWidgets
//  return "temp";
//#endif
//}

time_t GetLastFileModifyTime( const char *sFilePath )
{
    //#ifdef USINGBOOST
    //  fs::path BoostFilePath( sFilePath, fs::native );
    //  DEBUG(  BoostFilePath.string() ); // DEBUG//
    //
    // try
    // {
    //    std::time_t ft = fs::last_write_time( BoostFilePath );
    //    return ft;
    // }
    // catch( ... )
    // {
    //   DEBUG(  "ERROR: Problem getting file date " << sFilePath ); // DEBUG
    //   return 0;
    // }
    //#else
    // wxFileName myfile( sFilePath );
    //DEBUG(  "checking " << sFilePath ); // DEBUG
    //wxDateTime Lastmodificationtime = myfile.GetModificationTime();
    //return Lastmodificationtime.GetTicks();
    //#endif

    DEBUG(  "checking " << sFilePath ); // DEBUG
    int statreturn = 0;
    struct STAT buffer;
    statreturn = STAT( sFilePath, &buffer );

    if( statreturn != 0 )
    {
        DEBUG(  "failed to open file " << sFilePath ); // DEBUG
        return -1;
    }
    else
    {
        return buffer.st_mtime;
    }
}

int GetFileSize( const char *sFilePath )
{
    //  wxFile TargetFile( sFilePath );
    //  if( !TargetFile.IsOpened() )
    //  {
    //    DEBUG(  "failed to open file " << sFilePath ); // DEBUG
    //    return -1;
    //  }
    //  else
    //  {
    //     return TargetFile.Length();
    //  }

    int statreturn = 0;
    struct STAT buffer;
    statreturn = STAT( sFilePath, &buffer );

    if( statreturn != 0 )
    {
        DEBUG(  "failed to open file " << sFilePath ); // DEBUG
        return -1;
    }
    else
    {
        return buffer.st_size;
    }
}

bool FileExists( const char *sFilePath )
{
    // return ::wxFileExists( sFilePath );
    struct STAT buffer;
    return( STAT( sFilePath, &buffer ) == 0 );
}
