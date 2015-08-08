#include <stdio.h>
#ifdef _WIN32
#include "winsock2.h"
#define CLOSESOCKET closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKADDR sockaddr
#define LPVOID void *
#endif

void InstanceThread(LPVOID);

#define BUFSIZE 512

void mvCloseSocket( SOCKET socket )
{
#ifdef _WIN32
    closesocket( socket );
#else

    shutdown( socket, SHUT_RDWR );
#endif
}

int main(int argc, char *argv[])
{
    long iPortNumber;
    SOCKET AcceptSocket;
    SOCKET m_socket;
    sockaddr_in service;

    //  printf( "hello world" );
    iPortNumber = atoi( argv[1] );

    printf( "%i\n", iPortNumber );

#ifdef _WIN32
    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
    if ( iResult != NO_ERROR )
    {
        printf("Error at WSAStartup()\n");
        return -1;
    }
#endif

    // Create a socket.
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( m_socket == INVALID_SOCKET )
    {
        printf( "Error at socket()\n" );
#ifdef _WIN32

        printf( "%ld\n", WSAGetLastError() );
        WSACleanup();
#endif

        return -1;
    }

    // Bind the socket.

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    service.sin_port = htons( iPortNumber );

    if ( bind( m_socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR )
    {
        printf( "bind() failed.\n" );
        mvCloseSocket(m_socket);
        return -1;
    }

    if ( listen( m_socket, SOMAXCONN ) == SOCKET_ERROR )
    {
        printf( "Error listening on socket.\n");
#ifdef _WIN32

        printf( "%i\n", WSAGetLastError() );
#endif

        return -1;
    }

    while(1)
    {

        // Accept connections.
        AcceptSocket = SOCKET_ERROR;
        AcceptSocket = accept( m_socket, NULL, NULL );
        if(  AcceptSocket == SOCKET_ERROR )
        {
            printf( "Error accepting socket\n" );
#ifdef _WIN32

            printf( "%i\n", WSAGetLastError() );
#endif

            return -1;
        }

        // Print data received
        int bytesRecv = 0;
        char recvbuf[BUFSIZE] = "";
        bytesRecv = recv( AcceptSocket, recvbuf, BUFSIZE, 0 );
        printf( "%s\n", recvbuf );
    }

    return 0;
}

