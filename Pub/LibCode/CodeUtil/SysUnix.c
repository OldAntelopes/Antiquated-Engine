//---------------------------------------------------------------
// SysUnix.c
//
//  This file is the Unix implementations of the (system related) functions defined in System.h
//---------------------------------------------------------------


#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <pthread.h>

//#include <Compat.h>
#include "System.h"

#ifdef SERVER

#else
//#include "Globals.h"
#endif

void				SysExitThread( int nRetVal )
{
	pthread_exit(NULL);
}


void		SysPanicIf( int condition, const char* text, ... )
{
	if ( condition )
	{
	char		acString[1024];
	va_list		marker;
	uint32*		pArgs;
	int			nLen;
		pArgs = (uint32*)( &text ) + 1;

		va_start( marker, text );     
		vsprintf( acString, text, marker );

		nLen = strlen( acString );
		if ( nLen > 0)
		{
			printf( "**PANIC** " );
			printf( acString );
			printf( "\n");
		}
	}
}


unsigned int		SysCreateThread( fnThreadFunction pfnThreadFunction, void* pThreadPointerParam, uint32 ulThreadParam, int nPriority )
{
int	ret;
pthread_t	threadHandle;
//pthread_attr_t		threadAttributes;

//	pthread_attr_init( &threadAttributes );

	// todo - Priority setting of threads in linux?
//	if ( nPriority < 0 )
//	{
//		SetThreadPriority( hThread, THREAD_PRIORITY_BELOW_NORMAL );
//	}

	ret = pthread_create( &threadHandle, NULL, pfnThreadFunction, pThreadPointerParam);

	return( (unsigned int)( threadHandle ) );
}


const char*		SysNetworkGetIPAddressText( uint32 ulIP )
{
	return( inet_ntoa( *((struct in_addr*)(&ulIP))) );
}

uint32			SysNetworkGetIPAddress( const char* szIPAddressString )
{
	return( (uint32)(inet_addr( szIPAddressString )) );
}


void		SysSleep( int millisecs )
{
	usleep(millisecs * 1000);
}

void            SysGetLocalTime( SYS_LOCALTIME* pTime )
{
time_t xTime;
struct tm* systime;

	time( &xTime );
	systime = localtime( &xTime );
     

        pTime->wDay = systime->tm_wday;
        pTime->wHour = systime->tm_hour;
        pTime->wMinute = systime->tm_min;
        pTime->wMonth = systime->tm_mon;
        pTime->wSecond = systime->tm_sec;
        pTime->wYear = systime->tm_year;
}

void SysGetCurrentDir( int nStrLen, char* szBuffer )
{
	getcwd( szBuffer, nStrLen );
}

BOOL SysSetCurrentDir( const char* szDir )
{
	return( (chdir( szDir ) != 0) ? FALSE : TRUE );
}

//-------------------------------------------------------
// Function : SysGetTick			Unix Implementation
//   Returns the number of milliseconds since the puter was turned on
//		(Or when the application started.. doesnt matter which as long as it goes up regularly..)
//--------------------------------------------------------
uint32	SysGetTick( void )
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return ( (uint32)( (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull) ) );

#ifdef OLD_IMPLEMENTATION
struct timeval xCurrentTime ;

    gettimeofday(&xCurrentTime, 0) ;
    return( (time(0)*1000000 + xCurrentTime.tv_usec) / 1000 ) ;
#endif
}


//-------------------------------------------------------
// Function : SysMkDir				Unix Implementation
//   
//--------------------------------------------------------
BOOL	SysMkDir( const char* szDirname )
{
	return( mkdir( szDirname, S_IRWXU ) );
}

//-------------------------------------------------------
// Function : SysDeleteFile				Unix Implementation
//   Deletes the specified file from the puter
//--------------------------------------------------------
BOOL	SysDeleteFile( const char* szFilename )
{
	return( unlink( szFilename ) );
}
