#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "TickCount.h"

#define _IMPLEMENT_MSG_GROUPS
#include "Diag.h"

// 2005-06-25: willkn: added function below for optional prepending of time in DEBUG() logging macro
char* sztime()
{
    // get time string
    time_t tt;
    time( &tt );
    char *sz = ctime( &tt );
    char szCTime[256];
    strcpy( szCTime, sz );

    // chop newline
    szCTime[strlen(szCTime)-1]=0;

    // put year at beginning
    static char szTime[256];
    strncpy( szTime, szCTime + strlen( szCTime ) - 4, 4 );

    // append rest of time
    szCTime[strlen(szCTime)-5]=0;
    szTime[4] = ' ';
    szTime[5] = 0;
    strcat( szTime, szCTime );

    // add tick count for sub-second resolution
    unsigned int uiTick = (unsigned int) MVGetTickCount();
    char szTick[128];
    sprintf( szTick, " (%012u)", uiTick );
    strcat( szTime, szTick );

    return szTime;
}// sztime


//! \file
//! \brief These functions are deprecated and should not be used


void DebugInit()
{}

//! This function is deprecated and should not be used
void Debug( char *sMessage, ... )
{
    va_list args;
    va_start( args, sMessage );
    vprintf( sMessage, args );
}

//! This function is deprecated and should not be used
void SignalCriticalError( char *sMessage, ... )
{
    va_list args;
    va_start( args, sMessage );
    vprintf( sMessage, args );
}
