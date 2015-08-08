#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#include "tinyxml.h"

//#include "sockets.h"

//! \file
//! \brief These functions are deprecated and should not be used

const int iPortDebugConsole = 22155;

SOCKET DiagConsoleSocket;

char SendBuffer1[2048];
char SendBuffer2[2048];

bool bInitialized = false;
bool bAttemptedToConnectToDebugConsole = false;

SOCKET DiagSocketsConnectToServer( unsigned long IPAddress, int iPort )
{
    SOCKET m_socket = SOCKET_ERROR;

    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if ( m_socket == INVALID_SOCKET )
    {
        //    debug( "Error at socket(): %ld\n", WSAGetLastError() );
        //  WSACleanup();
        return SOCKET_ERROR;
        //  exit(1);
    }

    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = IPAddress;
    clientService.sin_port = htons( iPort );
    if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR)
    {
        //debug( "Failed to connect.\n" );
        //   WSACleanup();
        //   exit(1);
        return SOCKET_ERROR;
    }

    return m_socket;
}

void DebugInit()
{
    DiagConsoleSocket = DiagSocketsConnectToServer( inet_addr( "127.0.0.1" ), iPortDebugConsole );
    if( DiagConsoleSocket != SOCKET_ERROR )
    {
        bInitialized = true;
    }
    bAttemptedToConnectToDebugConsole = true;
}

//! This function is deprecated and should not be used

// This should probably
// dump to a file or something in the future.  (or maybe send out sockets messages
// to a console)
void debug( char *sMessage, ... )
{
    //    MessageBox(NULL, "debug function start", "Debug message",MB_OK | MB_ICONINFORMATION);
    if( !bInitialized && !bAttemptedToConnectToDebugConsole )
    {
        //    MessageBox(NULL, "init", "Debug message",MB_OK | MB_ICONINFORMATION);
        DebugInit();
    }

    if( bInitialized )
    {
        va_list args;
        va_start( args, sMessage );
        //   MessageBox(NULL, "vsprintf", "Debug message",MB_OK | MB_ICONINFORMATION);
        vsprintf( SendBuffer1, sMessage, args );

        //    MessageBox(NULL, "sprintf", "Debug message",MB_OK | MB_ICONINFORMATION);
        sprintf( SendBuffer2, "<debug message=\"%.2047s\"/>\n", SendBuffer1 );

        //    MessageBox(NULL, "send", "Debug message",MB_OK | MB_ICONINFORMATION);
        int bytessent = send( DiagConsoleSocket, SendBuffer2, strlen( SendBuffer2 ), 0 );
        if( bytessent == SOCKET_ERROR )
        {
            bInitialized = false;
        }

        //    MessageBox(NULL, "debug function done", "Debug message",MB_OK | MB_ICONINFORMATION);
    }
}

//! This function is deprecated and should not be used
void SignalCriticalError( char *sMessage, ... )
{
    MessageBox(NULL, "crit function start", "Debug message",MB_OK | MB_ICONINFORMATION);
    va_list args;
    va_start( args, sMessage );
    vsprintf( SendBuffer1, sMessage, args );
    MessageBox(NULL, SendBuffer1, "Critical error",MB_OK | MB_ICONINFORMATION);
    MessageBox(NULL, "crit function done", "Debug message",MB_OK | MB_ICONINFORMATION);
}

