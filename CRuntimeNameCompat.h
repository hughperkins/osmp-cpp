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

//! \file
//! \brief This file contains #defines to map linux C runtime library function names to Windows c runtime function names
//!
//! This file contains #defines to map linux C runtime library function names to Windows c runtime function names
//! Typically, this means prefixing an _.

#ifndef _CRUNTIMENAMECOMPAT_H
#define _CRUNTIMENAMECOMPAT_H

#ifdef _WIN32 // Windows
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#endif

#endif // _CRUNTIMENAMECOMPAT_H
