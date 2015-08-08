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
// See header file for documentation

#include <string>
#include <sstream>
#include <iostream>
using namespace std;

#ifdef _WIN32
#include "process.h"
#endif

#include "time.h"

#include "tinyxml.h"

#include "ClientEditing.h"
#include "WorldStorage.h"
#include "RendererTexturing.h"
#include "TextureInfoCache.h"
#include "Config.h"
#include "File.h"
#include "Terrain.h"
#include "ScriptMgmt.h"
#include "MetaverseClient.h"

namespace MetaverseClient
{
    extern RendererTexturingClass RendererTexturing;
    extern ScriptMgmtClass ScriptMgmt;
}
using namespace MetaverseClient;

// extern mvWorldStorage World;
// extern void SendClientMessage( const char *message );

void ClientsideEditingClass::DumpEditsInProgress()
{
    EditsInProgressIterator iterator;
    for( iterator = EditsInProgress.begin(); iterator != EditsInProgress.end(); iterator++ )
    {
        DEBUG(  "EditInProgress:" << iterator->second.iEditingInfoReference << " " << iterator->second.sFilePath
                << " " << iterator->second.iObjectReference ); // DEBUG
    }
}

int ClientsideEditingClass::RegisterWithEditingCache( int iObjectReference, string sType, string sFilePath, string sEditorCommand )
{
    DEBUG(  "registerwitheditingcache" ); // DEBUG
    DumpEditsInProgress();

    EditingInfoClass NewEditingInfo;
    NewEditingInfo.iEditingInfoReference = iNextEditingInfoReference;
    NewEditingInfo.iObjectReference = iObjectReference;
    NewEditingInfo.sType = sType;
    NewEditingInfo.sFilePath = sFilePath;
    NewEditingInfo.sEditorCommand = sEditorCommand;
    EditsInProgress.insert( EditInProgressPair( iNextEditingInfoReference, NewEditingInfo ) );

    DumpEditsInProgress();

    iNextEditingInfoReference++;

    return ( iNextEditingInfoReference - 1 );
}

string ClientsideEditingClass::GenerateTempPath( string sSourceFilename, string Postfix )
{
    char TempPath[256];
    char TempName[64];
    GetTempName( TempName, sSourceFilename.c_str() );
    sprintf( TempPath, "%s%s", TempName, Postfix.c_str() );

    string stringTempName = TempPath;
    return stringTempName;
}

string ClientsideEditingClass::GenerateTempDir( string sTempDir)
{
    string stringTempDir = sTempDir;
    return stringTempDir;
}


void ClientsideEditingClass::DownloadTextureToTemp( TEXTUREINFO &rTextureInfo, int iEditingRegNumber, string sTempPath )
{

    ostringstream messagestream;
    messagestream << "<loadergetfile type=\"TEXTURE\" checksum=\"" << rTextureInfo.sChecksum
    << "\" oureditreference=\"" << iEditingRegNumber << "\" serverfilename=\"" << rTextureInfo.sServerFilename << "\""
    << " sourcefilename=\"" << rTextureInfo.sSourceFilename << "\" "
    "ourlocalfilepath=\"" << sTempPath << "\" />" << endl;
    DEBUG(  "sending to clientfileagent: " << messagestream.str() ); // DEBUG

    TiXmlDocument IPC;
    IPC.Parse( messagestream.str().c_str() );
    //MetaverseClientSocket.Send( message.c_str() );
    MetaverseClient::SendXMLDocToFileAgent( IPC );

    //RendererMain::SendToClient( messagestream.str().c_str() );
}

void ClientsideEditingClass::DownloadScriptToTemp( EditingInfoClass &rEditInfo, int iEditingRegNumber, string sTempPath )
{

    ostringstream messagestream;
    messagestream << "<loadergetfile type=\"SCRIPT\" checksum=\"" << rEditInfo.sChecksumOfItemToEdit
    << "\" oureditreference=\"" << iEditingRegNumber << "\" serverfilename=\"" << rEditInfo.sServerFilename << "\""
    << " sourcefilename=\"" << rEditInfo.sSourceFilename << "\" "
    "ourlocalfilepath=\"" << sTempPath << "\" />" << endl;
    DEBUG(  "sending to client: " << messagestream.str() ); // DEBUG
    //RendererMain::SendClientMessage( messagestream.str().c_str() );
    TiXmlDocument IPC;
    IPC.Parse( messagestream.str().c_str() );
    //MetaverseClientSocket.Send( message.c_str() );
    MetaverseClient::SendXMLDocToFileAgent( IPC );
}

void ClientsideEditingClass::LaunchEditorOnFile( string sEditorCommand, string sTempPath )
{
#ifdef _WIN32
    spawnlp( _P_NOWAIT, sEditorCommand.c_str(), sEditorCommand.c_str(), sTempPath.c_str(), NULL );
#else

    system(sEditorCommand.c_str());
#endif
}

void ClientsideEditingClass::AddFileToChangeDetection( EditingInfoClass &EditingInfo )
{
    EditingInfo.LastModifiedTime = GetLastFileModifyTime( EditingInfo.sFilePath.c_str() );
}

//! - Gets texture reference for iObjectReference, from World
//! - Generates a temporary file name for the texture
//! - triggers download of file
//! - we will receive a new XML message/function call from metaverseclient when download is complete
void ClientsideEditingClass::EditTexture( int iObjectReference )
{
    int iArrayNum = World.GetArrayNumForObjectReference( iObjectReference );
    if( iArrayNum != -1 )
    {
        string sTextureReference;
        if( strcmp( World.GetObject( iArrayNum )->ObjectType, "PRIM" ) == 0 )
        {
            Prim *p_Prim = dynamic_cast< Prim * >( World.GetObject( iArrayNum ) );
            sTextureReference = p_Prim->GetTexture( 0 );

            TEXTUREINFO &rTextureInfo = textureinfocache.Textures.find( sTextureReference )->second;

            DEBUG(  "edittexture" ); // DEBUG
            string sTempPath = GenerateTempPath( rTextureInfo.sSourceFilename, rTextureInfo.sSourceFilename.substr( rTextureInfo.sSourceFilename.length() - 4, 4 ) );
            DEBUG(  "temp path: " << sTempPath ); // DEBUG
            int iEditingRegNumber = RegisterWithEditingCache( iObjectReference, "TEXTURE", sTempPath, mvConfig.TextureEditor );
            // DEBUG(  "registered with editingcache" ); // DEBUG
            DownloadTextureToTemp( rTextureInfo, iEditingRegNumber, sTempPath );
            DEBUG(  "command sent to fileagent" ); // DEBUG
        }
        else
        {
            DEBUG(  "warning: attempting to assign texture to non-prim " << World.GetObject( iArrayNum )->ObjectType ); // DEBUG
        }
    }
    else
    {
        DEBUG(  "Warning: Object " << iObjectReference << " not found" ); // DEBUG
    }
}

void ClientsideEditingClass::EditObject( int iObjectReference )
{
    int iArrayNum = World.GetArrayNumForObjectReference( iObjectReference );
    if( iArrayNum != -1 )
    {
        DEBUG(  "edit objectinventory " ); // DEBUG

        string sTempDir = GenerateTempDir( mvConfig.TempDirectory );

        if( strcmp( World.GetObject( iArrayNum )->ObjectType, "PRIM" ) == 0 )
        {
            if( strcmp( World.GetObject( iArrayNum )->sDeepObjectType, "TERRAIN" ) == 0 )
            {
                Terrain *p_Terrain = dynamic_cast< Terrain * >( World.GetObject( iArrayNum ) );
            }

            Prim *p_Prim = dynamic_cast< Prim * >( World.GetObject( iArrayNum ) );
            string sTextureReference = p_Prim->GetTexture( 0 );
            TEXTUREINFO &rTextureInfo = textureinfocache.Textures.find( sTextureReference )->second;

            string sTempPath = GenerateTempPath( rTextureInfo.sSourceFilename, rTextureInfo.sSourceFilename.substr( rTextureInfo.sSourceFilename.length() - 4, 4 ) );
            DEBUG(  "temp path: " << sTempPath ); // DEBUG
            int iEditingRegNumber = RegisterWithEditingCache( iObjectReference, "TEXTURE", sTempDir, mvConfig.TextureEditor );

            DownloadTextureToTemp( rTextureInfo, iEditingRegNumber, sTempDir );
            DEBUG(  "command sent to fileagent" ); // DEBUG
        }
    }
    else
    {
        DEBUG(  "EditObject Warning: Object " << iObjectReference << " not found" ); // DEBUG
    }
}

//! - creates an entry in the editingcache for this script edit
//! - sends a message to metaverseclient asking for information about this object's script(s)
//! - the client will forward the message to the server and forward the server's reply back to us
void ClientsideEditingClass::EditScriptInitiate( int iObjectReference )
{
    int iArrayNum = World.GetArrayNumForObjectReference( iObjectReference );
    if( iArrayNum != -1 )
    {
        Object *p_Object = World.GetObject( iArrayNum );

        //string sScriptReference = p_Object->sScriptReference;
        //TEXTUREINFO &rTextureInfo = textureinfocache.Textures.find( sTextureReference )->second;

        DEBUG(  "editscript" ); // DEBUG

        int iEditingRegNumber = RegisterWithEditingCache( iObjectReference, "SCRIPT", "", mvConfig.ScriptEditor );

        ostringstream messagestream;
        messagestream << "<requestinfo type=\"SCRIPT\" ireference=\"" << iObjectReference << "\" "
        "clienteditreference=\"" << iEditingRegNumber << "\"/>" << endl;
        DEBUG(  "sending to server " << messagestream.str() ); // DEBUG
        MetaverseClient::SendToServer( messagestream.str().c_str() );
    }
    else
    {
        DEBUG(  "Warning: Object " << iObjectReference << " not found" ); // DEBUG
    }
}

void ClientsideEditingClass::ProcessInfoResponseFromXMLString( const char *XMLString )
{
    TiXmlDocument IPC;
    IPC.Parse( XMLString );
    ProcessInfoResponse( IPC.RootElement() );
}

//! Accepts XML element:
//! <inforesponse clienteditreference="..." checksum="..." serverfilename="..." sourcefilename="..."/>
//!
//! This inforesponse could contain information about a script - or other file -
//! that we want to edit
//!
//! If clienteditreference is found in EditsInProgress:
//!   - populates the EditInProgress structure with the data, that is:
//!     - file checksum
//!     - serverfilename
//!     - sourcefilename
//!   - generates a temporary filepath
//!   - downloads the script to temp, via metaverseclient and clientfileagent
//!   - once the download has completed, client will send you XML message:
//!     <pre><loaderfiledone oureditreference="..."/></pre>
//!   - you'll then call OpenDownloadedFileForEditingFromXML to open the editor on the downloaded file
void ClientsideEditingClass::ProcessInfoResponse( TiXmlElement *pElement )
{
    DEBUG(  "continueeditingsequence" ); // DEBUG
    DumpEditsInProgress();
    int iEditingRegNumber = atoi( pElement->Attribute("clienteditreference") );
    if( EditsInProgress.find( iEditingRegNumber ) != EditsInProgress.end() )
    {
        DEBUG(  "Received info for editing from server for edit " << iEditingRegNumber ); // DEBUG
        EditingInfoClass &rEditInProgress = EditsInProgress.find( iEditingRegNumber )->second;

        rEditInProgress.sChecksumOfItemToEdit = pElement->Attribute("checksum");
        rEditInProgress.sServerFilename = pElement->Attribute("serverfilename");
        rEditInProgress.sSourceFilename = pElement->Attribute("sourcefilename");

        if( rEditInProgress.sType == "SCRIPT" )
        {
            DEBUG(  "script file" ); // DEBUG
            string sTempPath = GenerateTempPath( rEditInProgress.sSourceFilename, ".lua" );
            DEBUG(  "temp path: " << sTempPath << " generated from " << rEditInProgress.sSourceFilename ); // DEBUG
            rEditInProgress.sFilePath = sTempPath;

            DownloadScriptToTemp( rEditInProgress, iEditingRegNumber, sTempPath );
        }
    }
    DumpEditsInProgress();
}

//! Processes XML message <someelement oureditreference="..."/>
//! - finds the editing info about this file, using oureditreference
//! - adds the file to change detection,
//! - and launches the editor on that file
void ClientsideEditingClass::OpenDownloadedFileForEditingFromXML( TiXmlElement *pElement )
{
    int iEditReference = atoi( pElement->Attribute("oureditreference" ) );
    EditingInfoClass &EditingInfo = EditsInProgress.find( iEditReference )->second;
    AddFileToChangeDetection( EditingInfo );
    LaunchEditorOnFile( EditingInfo.sEditorCommand, EditingInfo.sFilePath );
}

void ClientsideEditingClass::WriteChangedFileToServer( EditingInfoClass &rEditingInfo )
{
    DEBUG(  "writing changes back to server..." ); // DEBUG
    if( rEditingInfo.sType == "TEXTURE" )
    {
        DEBUG(  "assigning texture to object " << rEditingInfo.sFilePath << " " << rEditingInfo.iObjectReference ); // DEBUG
        RendererTexturing.ApplyTextureToOneObject( rEditingInfo.iObjectReference, rEditingInfo.sFilePath );
    }
    else if( rEditingInfo.sType == "SCRIPT" )
    {
        DEBUG(  "assigning script to object " << rEditingInfo.sFilePath << " " << rEditingInfo.iObjectReference ); // DEBUG
        ScriptMgmt.AssignScriptToOneObject( rEditingInfo.iObjectReference, rEditingInfo.sFilePath );
    }
}

void ClientsideEditingClass::Clear()
{
    EditsInProgress.clear();
}

void ClientsideEditingClass::UploadChangedFiles()
{
    time_t now;
    time( &now );
    // just check once a second, to save resources
    //  DumpEditsInProgress();
    if( difftime( now, LastChangeCheck ) >= fScanInterval )
    {
        EditsInProgressIterator iterator;
        for( iterator = EditsInProgress.begin(); iterator != EditsInProgress.end(); iterator++ )
        {
            if( iterator->second.LastModifiedTime != 0 )
            {
                EditingInfoClass &rEditingInfo = iterator->second;
                DEBUG(  "Scanning file " << rEditingInfo.sFilePath << "  for reference " << rEditingInfo.iObjectReference ); // DEBUG
                time_t FileLastModifiedTime = GetLastFileModifyTime( rEditingInfo.sFilePath.c_str() );
                if( FileLastModifiedTime != 0 )
                {
                    if( difftime( FileLastModifiedTime, rEditingInfo.LastModifiedTime ) >= 3.0 )
                    {
                        DEBUG(  "UploadChangedFiles() writing file " << rEditingInfo.iObjectReference << " to server..." ); // DEBUG
                        WriteChangedFileToServer( rEditingInfo );
                        rEditingInfo.LastModifiedTime = FileLastModifiedTime;
                    }
                }
            }
        }
        LastChangeCheck = now;
    }
}
