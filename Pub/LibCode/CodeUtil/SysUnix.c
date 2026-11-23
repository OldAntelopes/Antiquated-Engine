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
#include <fnmatch.h>
#include <string.h>

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


void SysCopyFolderContents( const char* szSrcFolder, const char* szDestFolder )
{
DIR* dirp;
struct dirent* dp = NULL;
char	acString[256];
char	acString2[256];
BOOL boFinished = FALSE; 

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
		if ( ( dp->d_name[0] != '.' ) &&
			 ( dp->d_type != DT_DIR ) )
		{
			sprintf( acString, "%s/%s", szSrcFolder,  dp->d_name );
			sprintf( acString2, "%s/%s", szDestFolder, dp->d_name );
			SysCopyFile( acString, acString2, FALSE );
		}
	 
	    dp = readdir(dirp);
	    if (dp == NULL)
		{
		    boFinished = TRUE; 
		}
	}
	closedir(dirp);

}


/***************************************************************************
 * Function    : SysGetAllFilesInFolder
 * Params      : szFilenameSearch - wildcard patterns separated by ';' (e.g. "*.mit;*.txt")
 * Description : As the Windows version: each subfolder (when bIncludeSubFolders, not recursed) with bIsDirectory
 *				 TRUE, then each file matching a pattern, as "<folder>/<name>"
 ***************************************************************************/
void		SysGetAllFilesInFolder( const char* szSrcFolder, const char* szFilenameSearch, BOOL bIncludeSubFolders, fnDirListingCallback callback )
{
DIR*			dirp;
struct dirent*	dp;
struct stat		xStat;
char			acString[512];
char			acSearchCopy[256];
char*			pcToken;
char*			pcSave = NULL;
BOOL			bIsDir;

	dirp = opendir( szSrcFolder );
	if ( dirp == NULL )
	{
		return;
	}

	while ( ( dp = readdir( dirp ) ) != NULL )
	{
		if ( ( strcmp( dp->d_name, "." ) == 0 ) || ( strcmp( dp->d_name, ".." ) == 0 ) )
		{
			continue;
		}
		snprintf( acString, sizeof( acString ), "%s/%s", szSrcFolder, dp->d_name );
		if ( dp->d_type == DT_UNKNOWN )
		{
			bIsDir = ( ( stat( acString, &xStat ) == 0 ) && S_ISDIR( xStat.st_mode ) ) ? TRUE : FALSE;
		}
		else
		{
			bIsDir = ( dp->d_type == DT_DIR ) ? TRUE : FALSE;
		}

		if ( bIsDir )
		{
			if ( bIncludeSubFolders )
			{
				callback( acString, TRUE );
			}
			continue;
		}

		strncpy( acSearchCopy, szFilenameSearch ? szFilenameSearch : "*", sizeof( acSearchCopy ) - 1 );
		acSearchCopy[ sizeof( acSearchCopy ) - 1 ] = 0;
		for ( pcToken = strtok_r( acSearchCopy, ";", &pcSave ); pcToken != NULL; pcToken = strtok_r( NULL, ";", &pcSave ) )
		{
			if ( fnmatch( pcToken, dp->d_name, FNM_CASEFOLD ) == 0 )
			{
				callback( acString, FALSE );
				break;
			}
		}
	}
	closedir( dirp );
}

/***************************************************************************
 * Function    : SysUnixFillLocalTime
 * Description : SYS_LOCALTIME as Windows fills it (SYSTEMTIME): the full year (2026), the month 1-12, wDay the day of
 *				 the month and wDayOfWeek 0-6 from Sunday. Unix's struct tm counts years from 1900 and months from 0.
 ***************************************************************************/
static void		SysUnixFillLocalTime( const struct tm* pxTm, SYS_LOCALTIME* pTime )
{
	pTime->wYear = (short)( pxTm->tm_year + 1900 );
	pTime->wMonth = (short)( pxTm->tm_mon + 1 );
	pTime->wDay = (short)pxTm->tm_mday;
	pTime->wHour = (short)pxTm->tm_hour;
	pTime->wMinute = (short)pxTm->tm_min;
	pTime->wSecond = (short)pxTm->tm_sec;
	pTime->wDayOfWeek = (short)pxTm->tm_wday;
}

/***************************************************************************
 * Function    : SysGetUnixTimestampFromLocalTime
 * Description : The inverse of SysGetLocalTime: takes the full year and a 1-12 month (as SysGetLocalTime gives them)
 ***************************************************************************/
uint32				SysGetUnixTimestampFromLocalTime( SYS_LOCALTIME* pTime )
{
time_t	epoch;
struct tm		xTime;

	memset( &xTime, 0, sizeof( xTime ) );
	xTime.tm_hour = pTime->wHour;
	xTime.tm_mday = pTime->wDay;
	xTime.tm_min = pTime->wMinute;
	xTime.tm_mon = pTime->wMonth - 1;
	xTime.tm_sec = pTime->wSecond;
	xTime.tm_year = pTime->wYear - 1900;
	xTime.tm_isdst = -1;		// let mktime decide whether summer time applies
	epoch = mktime( &xTime );

	return( (uint32)epoch );
}

/***************************************************************************
 * Function    : SysGetLocalTimeFromUnixTimestamp
 ***************************************************************************/
void		SysGetLocalTimeFromUnixTimestamp( uint32 ulUnixTime, SYS_LOCALTIME* pTime )
{
time_t		xTime = (time_t)ulUnixTime;
struct tm	xTm;

	if ( localtime_r( &xTime, &xTm ) != NULL )
	{
		SysUnixFillLocalTime( &xTm, pTime );
	}
	else
	{
		memset( pTime, 0, sizeof( SYS_LOCALTIME ) );
	}
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

	ret = pthread_create( &threadHandle, NULL, (void*(*)(void*))pfnThreadFunction, pThreadPointerParam);

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

/***************************************************************************
 * Function    : SysGetUTCTime
 * Description : As on Windows (SysUnixFillLocalTime). Before 2026-11-23 the Unix build gave years since 1900, a 0-11
 *				 month and the weekday as wDay
 ***************************************************************************/
void            SysGetUTCTime( SYS_LOCALTIME* pTime )
{
time_t		xTime = time( NULL );
struct tm	xTm;

	gmtime_r( &xTime, &xTm );
	SysUnixFillLocalTime( &xTm, pTime );
}

/***************************************************************************
 * Function    : SysGetLocalTime
 * Description : As on Windows (SysUnixFillLocalTime)
 ***************************************************************************/
void            SysGetLocalTime( SYS_LOCALTIME* pTime )
{
time_t		xTime = time( NULL );
struct tm	xTm;

	localtime_r( &xTime, &xTm );
	SysUnixFillLocalTime( &xTm, pTime );
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
