
#include <s3eFile.h>
#include <stdarg.h>
#include <stdio.h>
#include <s3e.h>

#include <time.h>
#include <StandardDef.h>
#include <System.h>

fnDebugPrintHandler		mpfnDebugPrintHandler = NULL;
BOOL		mbIgnoreFuturePanics = FALSE;

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

		nLen = strlen(acString);
		if ( nLen > 0)
		{
			s3eDebugTracePrintf( "*** PANIC ***" );
			s3eDebugTracePrintf( acString );
		}

		if ( mbIgnoreFuturePanics == FALSE )
		{
		s3eErrorShowResult		result;
			result = s3eDebugAssertShow( S3E_MESSAGE_CONTINUE_STOP, acString );
			if ( result == S3E_ERROR_SHOW_IGNORE )
			{
				mbIgnoreFuturePanics = TRUE;
			}
			else if ( result == S3E_ERROR_SHOW_STOP )
			{
				// breakpoint here..
			}
		}
	}
}

BOOL	SysCheckKeyState( int nKeyCode )
{
	// Well.. we don't have a keyboard most likely...
	return( FALSE );
}

BOOL	SysOSYield( void )
{

	return( FALSE );
}

void		SysGetMACaddress( unsigned char* szOutBuffer )
{
	// todo

	szOutBuffer[0] = 0;

}

uint32	SysGetTimeLong(void )
{
uint32	ulCurrentTime = (uint32)( s3eTimerGetUTC() / 1000 );
	return( ulCurrentTime );
}

BOOL	SysIsFinite( float fVal )
{
	// todo - implement this
	return( TRUE );
}

void				SysExitThread( int nRetval )
{
	s3eThreadExit( (void*)nRetval );
}

unsigned int		SysCreateThread( fnThreadFunction threadFunction, void* pThreadPointerParam, uint32 ulThreadParam, int nPriority )
{
s3eThread*	pThread;

	// MARMALADE TODO - Note we're not passing the ulThreadParam at all... may need to work a way of doing that
	// similarly with thread priority
	pThread = s3eThreadCreate( (s3eThreadStartFunc) threadFunction, pThreadPointerParam, NULL );
	return( (uint32)pThread );
}

uint32				SysGetUnixTimestampFromLocalTime( SYS_LOCALTIME* pTime )
{
time_t	epoch;
tm		xTime;

	xTime.tm_hour = pTime->wHour;
	xTime.tm_mday = pTime->wDay;
	xTime.tm_min = pTime->wMinute;
	xTime.tm_mon = pTime->wMonth;
	xTime.tm_sec = pTime->wSecond;
	xTime.tm_year = pTime->wYear;

	// todo 
	epoch = mktime( &xTime );

	return( *( (uint32*)( &epoch ) ) );
}

void		SysGetLocalTimeFromUnixTimestamp( uint32 ulUnixTime, SYS_LOCALTIME* pTime )
{
struct tm *dcp;

	dcp = localtime((const time_t*)&ulUnixTime);

	if ( dcp != NULL )
	{
		pTime->wDay = dcp->tm_mday;
		pTime->wHour = dcp->tm_hour;
		pTime->wMinute = dcp->tm_min;
		pTime->wMonth = dcp->tm_mon;
		pTime->wSecond = dcp->tm_sec;
		pTime->wYear = dcp->tm_year;
	}
	else
	{
		memset( pTime, 0, sizeof( SYS_LOCALTIME ) );
	}
}


void				SysGetLocalTime( SYS_LOCALTIME* pMem )
{
time_t now = time(NULL);
struct tm* local_tm = localtime(&now);
	
	pMem->wDay = local_tm->tm_wday;
	pMem->wHour = local_tm->tm_hour;
	pMem->wMinute = local_tm->tm_min;
	pMem->wMonth = local_tm->tm_mon;
	pMem->wSecond = local_tm->tm_sec;
	pMem->wYear = local_tm->tm_year;

}


void	SysSleep( int nMillisecs )
{
	s3eDeviceYield(nMillisecs);
}

uint32	SysGetTick( void )
{
	return( (uint32) s3eTimerGetMs() );
}

u64		SysGetMicrosecondTick( void )
{
	return( (u64) s3eTimerGetMs() * 1000 );
}

void*	SysFileOpen( const char* szFilename, const char* szOpenMode )
{
s3eFile*	pFile = s3eFileOpen( szFilename, szOpenMode );

	return( pFile );
}

int		SysFileRead( unsigned char* pcOutMem, int nReadSize, int nNumReads, void* pFile )
{
	return( s3eFileRead( pcOutMem, nReadSize, nNumReads, (s3eFile*)pFile ) );

}

void	SysFileClose( void* pFile )
{
	s3eFileClose( (s3eFile*)pFile );

}

int		SysGetFileSize( void* pFile )
{
	return( s3eFileGetSize( (s3eFile*) pFile ) );
}

int		SysFileWrite( unsigned char* pOutMem, int nWriteSize, int nNumWrites, void* pFile )
{
	return( s3eFileWrite( pOutMem, nWriteSize, nNumWrites, (s3eFile*)pFile ) );
	
}

int		SysFileSeek( void* pFile, int offset, int mode )
{
	s3eFileSeek( (s3eFile*)pFile, offset, S3E_FILESEEK_CUR );
	return( 1 );
}


void		SysDebugPrint( const char* text, ... )
{
char		acString[1024];
va_list		marker;
uint32*		pArgs;
int			nLen;

	pArgs = (uint32*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );

	nLen = strlen(acString) ;
	if ( nLen > 0)
	{
		s3eDebugTracePrintf( acString );
	}

	if ( mpfnDebugPrintHandler )
	{
	char            acString[512]; 
	va_list         marker; 

	    va_start( marker, text );    
		vsprintf( acString, text, marker ); 
		mpfnDebugPrintHandler( acString );
	}

}

void		SysRegisterDebugPrintHandler( fnDebugPrintHandler pDebugPrintHandler )
{
	mpfnDebugPrintHandler = pDebugPrintHandler;
}


BOOL		SysDeleteFile( const char* szFilename )
{
	return( FALSE );
}


BOOL		SysMkDir( const char* szDirname )
{
	return( FALSE );
}

void		SysGetCurrentDir( int nStrLen, char* szBuffer )
{
	szBuffer[0] = 0;
}

char	mszWritableFolderPath[32] = "";

const char*	SysGetWritableDataFolderPath( const char* szGameName )
{
	return( mszWritableFolderPath );
}

BOOL		SysSetCurrentDir( const char* szDir )
{
	return( FALSE );
}
