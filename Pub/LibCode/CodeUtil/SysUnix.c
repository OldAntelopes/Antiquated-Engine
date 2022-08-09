//---------------------------------------------------------------
// SysUnix.c
//
//  This file is the Unix implementations of the (system related) functions defined in System.h
//---------------------------------------------------------------


#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include <Compat.h>
#include "System.h"

#ifdef SERVER

#else
//#include "Globals.h"
#endif


int             SysGetFileSize( void* pFile )
{
        if ( pFile != NULL )
        {
        int             nFileSize;
                fseek( pFile, 0, SEEK_END );
                nFileSize = ftell(pFile);
                rewind( pFile );
                return( nFileSize );
        }
        return( 0 );
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

BOOL SysSetCurrentDir( char* szDir )
{
	return( (chdir( szDir ) != 0) ? FALSE : TRUE );
}

//-------------------------------------------------------
// Function : SysGetTick			Unix Implementation
//   Returns the number of milliseconds since the puter was turned on
//		(Or when the application started.. doesnt matter which as long as it goes up regularly..)
//--------------------------------------------------------
ulong	SysGetTick( void )
{
struct timeval xCurrentTime ;

    gettimeofday(&xCurrentTime, 0) ;
    return( (time(0)*1000000 + xCurrentTime.tv_usec) / 1000 ) ;
}

// -- Wrapper for malloc or whateva equivalent mem alloc function
void*	SystemMalloc( int nMemsize )
{
	return( malloc( nMemsize ) );
}

// -- Wrapper for free or whateva equivalent mem alloc function
void		SystemFree( void* pMem )
{
	free( pMem );
}

//-------------------------------------------------------
// Function : SysMkDir				Unix Implementation
//   
//--------------------------------------------------------
BOOL	SysMkDir( char* szDirname )
{
	return( mkdir( szDirname, S_IRWXU ) );
}

//-------------------------------------------------------
// Function : SysDeleteFile				Unix Implementation
//   Deletes the specified file from the puter
//--------------------------------------------------------
BOOL	SysDeleteFile( char* szFilename )
{
	return( unlink( szFilename ) );
}
