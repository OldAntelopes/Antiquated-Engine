//---------------------------------------------------------------
// SysUnix.c
//
//  This file is the Unix implementations of the (system related) functions defined in System.h
//---------------------------------------------------------------


#include <unistd.h>
#include <dirent.h>
#include <errno.h>
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

BOOL		SysFinite( float fVal )
{
	if ( __builtin_isfinite( fVal ) )
	{
		return( TRUE );
	}
	return( FALSE );
}

void		SysGetAllFilesInFolder( const char* szSrcFolder, fnDirListingCallback callback )
{
DIR* dirp;
struct dirent* dp = NULL;
char	acString[256];
BOOL boFinished = FALSE; 

//	sprintf( acString, "%s/*", szDirectoryName );	 
	dirp = opendir(szSrcFolder);
	if (dirp != NULL)
	{
		dp = readdir(dirp);
	}
	
	if (dp == NULL)
	{
		return;
	}

	while ( !boFinished )
	{ 
		if ( dp->d_name[0] != '.' )
		{
			if (dp->d_type != DT_DIR)
			{
				sprintf( acString, "%s/%s", szSrcFolder, dp->d_name );
				callback( acString );
			}
		}

	    dp = readdir(dirp);
	    if (dp == NULL)
		{
		    boFinished = TRUE; 
		}
	}
	closedir(dirp);
}

uint32				SysGetUnixTimestampFromLocalTime( SYS_LOCALTIME* pTime )
{
time_t	epoch;
struct tm		xTime;

	xTime.tm_hour = pTime->wHour;
	xTime.tm_mday = pTime->wDay;
	xTime.tm_min = pTime->wMinute;
	xTime.tm_mon = pTime->wMonth;
	xTime.tm_sec = pTime->wSecond;
	xTime.tm_year = pTime->wYear;
	epoch = mktime( &xTime );

	return( *( (uint32*)( &epoch ) ) );
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
	pTime->wDayOfWeek = systime->tm_wday;
}

void SysGetCurrentDir( int nStrLen, char* szBuffer )
{
	getcwd( szBuffer, nStrLen );
}

BOOL SysSetCurrentDir( const char* szDir )
{
	return( (chdir( szDir ) != 0) ? FALSE : TRUE );
}

u64		SysGetMicrosecondTick( void )
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return ( (uint32)( (uint64_t)(ts.tv_nsec / 1000) + ((uint64_t)ts.tv_sec * 1000000ull) ) );
}

BOOL SysDoesDirExist( const char* szPath)
{
DIR* dir = opendir(szPath);
	if (dir) 
	{   
		closedir(dir);
		return( TRUE );
	}
	return( FALSE );
}
const char*	SysGetWritableDataFolderPath( const char* szGameName )
{
	return ("./");
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
