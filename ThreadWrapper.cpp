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
//! \brief Wraps platform-dependent threads functions

#ifndef _WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <pthread.h>

#include "ThreadWrapper.h"

pthread_cond_t pausethreadcond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t pausethreadmut = PTHREAD_MUTEX_INITIALIZER;

void PauseThreadMilliseconds( int iMilliseconds )
{
#ifndef _WIN32
    pthread_mutex_lock(&pausethreadmut);
    struct timeval now;
    gettimeofday(&now, 0 );

    struct timespec timeout;
    timeout.tv_sec = now.tv_sec;
    timeout.tv_nsec = now.tv_usec * 1000;
    timeout.tv_nsec = timeout.tv_nsec + iMilliseconds * 1000 * 1000;
    if( timeout.tv_nsec > 1000 * 1000 * 1000 )
    {
        timeout.tv_sec += 1;
        timeout.tv_nsec -= 1000 * 1000 * 1000;
    }
    pthread_cond_timedwait(&pausethreadcond, &pausethreadmut, &timeout );
    pthread_mutex_unlock(&pausethreadmut);
#else

    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = iMilliseconds * 1000 * 1000;
    pthread_delay_np( &timeout );
#endif
}

