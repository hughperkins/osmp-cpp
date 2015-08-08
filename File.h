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
//!
//! This module contains file functions that may be non-portable
//! They're grouped here to make it easier to port them as and when
//!
//! using wxWidgets to facilitate portability

#ifndef _MVFILE_H
#define _MVFILE_H

#include <time.h>

#ifdef _WIN32
#define STAT _stat
#else
#define STAT stat
#endif

void GetTempName( char *TempName, const char *Prefix );          //!< returns a temp path (used by ClientEditing)
//string mvGetTempPath();                                         //!< get the path of the temp directory (not currently used)
time_t GetLastFileModifyTime( const char *sFilePath );         //!< as title says; used by ClientEditing to determine when a filew has changed
int GetFileSize( const char *sFilePath );                      //!< as title, size in bytes
bool FileExists( const char *sFilePath );                     //!< returns true/false if file exists or not

#endif // _MVFILE_H
