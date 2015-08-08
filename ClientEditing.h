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
//! \brief This module handles in-place editing of textures and scripts on the client

#ifndef _CLIENTEDITING_H
#define _CLIENTEDITING_H

#include <string>
#include <map>
using namespace std;

#include "tinyxml.h"
#include "TextureInfoCache.h"
#include "WorldStorage.h"
#include "Config.h"

//! Holds information on one edit-in-progress
class EditingInfoClass
{
public:
    int iEditingInfoReference;   //!< sequential reference number for edit; unique within the client
    int iObjectReference;   //!< iReference for object being edited; unique within sim/database
    string sType;     //!< filetype (ie "SCRIPT" or "TEXTURE", for now)
    string sFilePath;   //!< path of file being edited (once downloaded etc)
    string sEditorCommand;  //!< command to use to edit file after download (retrieved from config.xml)
    time_t LastModifiedTime;  //!< last modified time of file; used to check for changes

    // added for script editing:
    string sChecksumOfItemToEdit;  //!< md5 checksum of file being edited (I think?)
    string sServerFilename;     //!< Filename of file on the server
    string sSourceFilename;   //!< filename of original file

    EditingInfoClass()
    {
        iEditingInfoReference = 0;
        iObjectReference = 0;
        sType = "";
        sFilePath = "";
        sEditorCommand = "";
        LastModifiedTime = 0;

        sChecksumOfItemToEdit = "";
        sServerFilename = "";
        sSourceFilename = "";
    }
};

//! The ClientsideEditingClass handles in-place editing of textures and scripts on the client

//! The ClientsideEditingClass handles in-place editing of textures and scripts on the client
//! This means that you can see a texture in the world, open it in the editor of your choice,
//! and, when you save, the changes are automatically replicated to the server!
//!
//! When you tell OSMP you want to edit the texture, script etc, it downloads it if
//! necessary, then opens it in the editor configured in config.xml
//!
//! The file will be scanned for changes, and uploaded to the server as they
//! occur
//!
//! To use this class:
//! - Construct, passing in mvWorldStorage and mvConfig to constructor
//! - call Settextureinfocache, passing in reference to textureinfocache
//!
//! - Usage depends on type of file:
//!   - files whose reference etc is already known on the client (eg textures)
//!   - files whose reference is not normally known on the client (eg scripts)
//!
//! - for files whose reference etc is already known on the client (eg textures)
//!   - call EditTexture, or similar, passing in the object reference
//!   - the file reference for the object will be determined, using the textureinfocache (or equivalent)
//!   - a temporary filename will be generated,
//!   - download will be triggered, via metaverseclient and thence clientfileagent
//!   - after the file is downloaded, metaverseclient will send you XML message:
//!      <pre><loaderfiledone oureditreference="..."/></pre>
//!   - send this message to OpenDownloadedFileForEditingFromXML, passing in the XML message element
//!     This function will open the file in the editor defined in config.xml
//!
//! - files whose reference is not normally known on the client (eg scripts)
//!   - call EditScriptInitiate, or equivalent, passing in the object reference
//!   - since we dont know the reference for this object's script (most likely),
//!     - a message will be sent to the metaverseclient asking for more information about the script
//!     - metaverseclient will forward the request to the server, and the reply back to us
//!   - after the file is downloaded, metaverseclient will send you XML message:
//!      <pre><loaderfiledone oureditreference="..."/></pre>
//!   - send this message to OpenDownloadedFileForEditingFromXML, passing in the XML message element
//!     This function will open the file in the editor defined in config.xml
//!
//! - Call UploadChangedFiles() regularly to check for changed files and upload them.
//!   Note that UploadChangedFiles() will only scan if previous scan was at least fScanInterval seconds ago
//!
//! - You can reinitialize the cache by calling Clear; eg after hyperlinking to a new server
//!
class ClientsideEditingClass
{
public:
    ClientsideEditingClass( mvWorldStorage &InWorld, ConfigClass &NewmvConfig, TextureInfoCache &Newtextureinfocache ) :
            World(InWorld ),
            mvConfig( NewmvConfig ),
            fScanInterval( 0.5 ),
            textureinfocache( Newtextureinfocache )
    {
        iNextEditingInfoReference = 1;
        EditsInProgress.clear();
        LastChangeCheck = 0;
    }

    void EditTexture( int iObjectReference );                        //!< Launches in-place editing on the texture on the object iObjectReference
    void EditScriptInitiate( int iObjectReference );            //!< Launches in-place editing on the script on the object iObjectReference
    void EditObject( int iObjectReference );                        //!< Launches object inventory window on the object iObjectReference

    void ProcessInfoResponseFromXMLString( const char *XMLString );
    void ProcessInfoResponse( TiXmlElement *pElement );      //!< Handles the <inforesponse/> message received from the MetaverseServer after the EditScriptInitiate call above
    void OpenDownloadedFileForEditingFromXML( TiXmlElement *pElement );   //!< pass in the XML received from the client file agent after a successful download to launch the appropriate editor on the file

    void UploadChangedFiles(); //!< if hasnt scanned for fScanInterval seconds, scans all files that are on the changedetection list, and uploads all that have changed

    void Clear();                //!< reinitializes all caches

protected:
    const float fScanInterval;

    mvWorldStorage &World;   //!< Stores objects in world
    ConfigClass &mvConfig;

    TextureInfoCache &textureinfocache;

    int iNextEditingInfoReference;        //!< reference number to assign to next EditingInfoClass in EditsInProgress
    map < int, EditingInfoClass > EditsInProgress;   //!< stores an EditingInfoClass for each edit in progress

    time_t LastChangeCheck;  //!< tickcount when files being edited were last scanned for changes

    int RegisterWithEditingCache( int iObjectReference, string sType, string sFilePath, string sEditorCommand );  //!< registers a new edit with the editing cache
    string GenerateTempPath( string sSourceFilename, string Postfix );                                             //!< generates a temporary path for a file that will be downloaded and edited
    string GenerateTempDir( string sTempDir );                                             //!< generates a temporary directory for object inventory

    void DownloadTextureToTemp( TEXTUREINFO &rTextureInfo, int iEditingRegNumber, string sTempPath );           //!< downloads the texture specified in TextureInfo to the path sTempPath.  Need to have got an edit ref number by calling RegisterWithEditingCache first
    void DownloadScriptToTemp( EditingInfoClass &rEditInfo, int iEditingRegNumber, string sTempPath );           //!< downloads the script specified in rEditInfo to the path sTempPath.  Need to have got an edit ref number by calling RegisterWithEditingCache first

    void WriteChangedFileToServer( EditingInfoClass &rEditingInfo );                          //!< writes the file specified by rEditingInfo back to the Metaverse Server
    void AddFileToChangeDetection( EditingInfoClass &EditingInfo );                           //!< adds a downloaded file to the list of files to be monitored for changes

    void LaunchEditorOnFile( string sEditorCommand, string sTempPath );                    //!< launches the editor specified in sEditorCommand on the file sTempPath

    void DumpEditsInProgress();                  //!< diagnostic tool: dumps the information on all edits in progress to console
};

typedef map < int, EditingInfoClass >::iterator EditsInProgressIterator;
typedef pair < int, EditingInfoClass > EditInProgressPair;

#endif // _CLIENTEDITING_H
