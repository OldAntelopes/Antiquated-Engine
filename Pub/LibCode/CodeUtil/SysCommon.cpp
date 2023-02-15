
#include <stdarg.h>
#include <StandardDef.h>
#include <System.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <CodeUtil.h>
/*
#ifdef CONSOLE
#ifndef tm
typedef struct {
   int tm_hour;   // hour (0 - 23)
   int tm_isdst;  // daylight saving time enabled/disabled 
   int tm_mday;   // day of month (1 - 31) 
   int tm_min;    // minutes (0 - 59) 
   int tm_mon;    // month (0 - 11 : 0 = January) 
   int tm_sec;    // seconds (0 - 59) 
   int tm_wday;   // Day of week (0 - 6 : 0 = Sunday) 
   int tm_yday;   // Day of year (0 - 365) 
   int tm_year;   // Year less 1900 
} tm;
#endif
#endif
*/
#ifndef _SYSTEMTIME
typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif

fnUserPrintFunction		mfnSysUserPrintFunction = NULL;


void		SysRegisterUserPrintHandler( fnUserPrintFunction pUserPrintHandler )
{
	mfnSysUserPrintFunction = pUserPrintHandler;
}



void		SysUserPrint( int mode, const char* text, ... )
{
char		acString[1024];
va_list		marker;
ulong*		pArgs;
int			nLen;

	pArgs = (ulong*)( &text ) + 1;

	va_start( marker, text );     
	vsprintf( acString, text, marker );

	nLen = strlen( acString );
	if ( nLen > 0)
	{
		if ( mfnSysUserPrintFunction )
		{
			mfnSysUserPrintFunction( mode, acString );
		}
		else
		{
		SYS_LOCALTIME	xTime;

			SysGetLocalTime( &xTime );
			printf( "[%d-%d]%02d:%02d:%02d ", xTime.wDay, xTime.wMonth, xTime.wHour, xTime.wMinute, xTime.wSecond );
			printf( acString );
			printf( "\n" );
		}
	}
}


int		SysStricmp( char* szOut, const char* szIn )
{
	return( stricmp( szOut, szIn ) );
}

/*

int		SysGetFileSize( void* pFile )
{
	if ( pFile != NULL )
	{
	int		nFileSize;
		fseek( (FILE*)pFile, 0, SEEK_END );
		nFileSize = ftell((FILE*)pFile);
		rewind( (FILE*)pFile );
		return( nFileSize );
	}
	return( 0 );
}
*/

#ifdef WIN32
ulong	SysGetTimeLong(void )
{
ulong	ulCurrentTime = 0;
__time32_t	xtimet;
//int		nRet;

	_time32( &xtimet );
	ulCurrentTime = *( (ulong*)(&xtimet) );

	return( ulCurrentTime );
}
#endif



u64		SysStringToU64( const char* szString )
{
#ifdef __GNUC__
	return( strtoull( szString, NULL, 10 ) );
#else
	return( _atoi64( szString ) );
#endif
}

//#define	MEM_BOUNDS_CHECK

// -- Wrapper for malloc or whateva equivalent mem alloc function
void*	SystemMalloc( int nMemsize )
{
#ifdef MEM_BOUNDS_CHECK
byte*	pbMem;

	nMemsize += 8;
	pbMem = (byte*)malloc(nMemsize);
	pbMem[0] = 0xEE;
	pbMem[1] = 0x01;
	pbMem[2] = 0xCC;
	pbMem[3] = 0x44;
	pbMem[nMemsize-4] = 0xEE;
	pbMem[nMemsize-3] = 0x01;
	pbMem[nMemsize-2] = 0xCC;
	pbMem[nMemsize-1] = 0x44;
	return( (void*)(pbMem + 4) );
#endif

	return( malloc( nMemsize ) );
}


// -- Wrapper for free or whateva equivalent mem alloc function
void		SystemFree( void* pMem )
{
#ifdef MEM_BOUNDS_CHECK
byte*	pbMem;
	pbMem = (byte*)(pMem);
	pbMem -= 4;
	pMem = (void*)(pbMem);
	if ( ( pbMem[0] != 0xEE ) ||
		 ( pbMem[1] != 0x01 ) ||
		 ( pbMem[2] != 0xCC ) ||
		 ( pbMem[3] != 0x44 ) )
	{
		// TODO - raise a windows type assertion here to let us jump to debug
		PANIC_IF(TRUE,"pre-mem bounds check error");
	}
#endif

	free( pMem );
}

// Erm, you know, malloc() and free() are kinda universal... --Guy
// yeh but using em means you cant easily slot in your own memchecking stuff..(e.g. a check for certain sized allocations in the systemmalloc)
//   more possible debug functionality than a direct system call - M


#define	SALT	0x00254813

unsigned int	BasicHash(const char* szString )
{
int		val;

    unsigned int h = SALT;
    while ( *szString != 0 )
	{
		val = *szString++;
        h = h * 101 + val;
	}
    return h;
}

void	MakeSubDirectory( const char* pcFilename, const char* pcRootPath )
{
const char*	pcRunner = pcFilename + strlen( pcFilename );
const char*	pcSecondRunner;
char	acFolderName[256];
char*	pcFolder = acFolderName;
char	acFullPath[256];

	while ( ( pcRunner > pcFilename ) &&
		    ( *pcRunner != '\\' ) &&
			( *pcRunner != '/' ) )
	{
		pcRunner--;
	}
	if ( pcRunner > pcFilename )
	{
		pcSecondRunner = pcRunner;
		while ( ( pcSecondRunner > pcFilename ) &&
				( *pcSecondRunner != '\\' ) &&
				( *pcSecondRunner != '/' ) )
		{
			pcSecondRunner--;
		}
		if ( pcSecondRunner > pcFilename )
		{
			pcSecondRunner = pcFilename;
			while ( pcSecondRunner != pcRunner )
			{
				*pcFolder++ = *pcSecondRunner++;
			}
			*pcFolder = 0;
			strcpy( acFullPath, pcRootPath );
			strcat( acFullPath, "\\" );
			strcat( acFullPath, acFolderName );

			if ( SysMkDir( acFullPath ) != 0 )
			{
			char	acString[512];
				sprintf( acString, "\\%s", acFullPath );
				if ( SysMkDir( acString ) != 0 )
				{
					sprintf( acString, "Error creating directory - %s\n", acFullPath );
					SysDebugPrint( acString );
				}
			}
		}
	}
	
}

// ---- Given a filename like "Data\512x\image.jpg   and a root "Data\Textures\Land\Auto"
// This function will create the required subfolders in order to contain the file
// i.e. Data/Textures/Land/Auto/Data/
//		Data/Textures/Land/Auto/Data/512x/
// (Can also be used with "" for path and a full path filename)
//-----------------
void	SysMakeSubDirectories( const char* pcFilename, const char* pcRootPath )
{
const char*	pcRunner = pcFilename;
//const char*	pcSecondRunner;
const char* pcFilenameEnd = pcFilename + strlen( pcFilename );
char	acFolderName[256];
char*	pcFolder = acFolderName;
char	acFullPath[256];

	while ( pcRunner < pcFilenameEnd ) 
	{
		while ( ( pcRunner < pcFilenameEnd ) &&
			    ( *pcRunner != '\\' ) &&
				( *pcRunner != '/' ) )
		{
			*pcFolder++ = *pcRunner++;
		}

		*pcFolder = 0;
		if ( pcRunner < pcFilenameEnd )
		{
			strcpy( acFullPath, pcRootPath );
			strcat( acFullPath, "\\" );
			strcat( acFullPath, acFolderName );

			if ( SysMkDir( acFullPath ) != 0 )
			{
			char	acString[512];

				sprintf( acString, "Error creating directory - %s\n", acFullPath );
				SysDebugPrint( acString );
			}
			pcRunner++;
		}
		*pcFolder++ = '\\';
	}
	
}

unsigned int		GetBlendedCol( unsigned int ulCol1, unsigned int ulCol2, float fCol1BlendStrength )
{
float	fR1, fG1, fB1, fA1;
float	fR2, fG2, fB2, fA2;
ulong	ulA, ulR, ulG, ulB;
ulong	ulOut;

	fA1 = (float)( ulCol1 >> 24) / 0xFF;
	fR1 = (float)( (ulCol1 >> 16) & 0xFF) / 0xFF;
	fG1 = (float)( (ulCol1 >> 8) & 0xFF) / 0xFF;
	fB1 = (float)( ulCol1 & 0xFF ) / 0xFF;

	fA2 = (float)( ulCol2 >> 24) / 0xFF;
	fR2 = (float)( (ulCol2 >> 16) & 0xFF) / 0xFF;
	fG2 = (float)( (ulCol2 >> 8) & 0xFF) / 0xFF;
	fB2 = (float)( ulCol2 & 0xFF ) / 0xFF;

	fA1 = (fA1 * fCol1BlendStrength) + (fA2 * (1.0f-fCol1BlendStrength) );
	fR1 = (fR1 * fCol1BlendStrength) + (fR2 * (1.0f-fCol1BlendStrength) );
	fG1 = (fG1 * fCol1BlendStrength) + (fG2 * (1.0f-fCol1BlendStrength) );
	fB1 = (fB1 * fCol1BlendStrength) + (fB2 * (1.0f-fCol1BlendStrength) );

	ulA = (ulong)( fA1 * 0xFF );
	ulR = (ulong)( fR1 * 0xFF );
	ulG = (ulong)( fG1 * 0xFF );
	ulB = (ulong)( fB1 * 0xFF );

	ulOut = (ulA << 24) | ( ulR << 16) | (ulG << 8) | (ulB);
	return( ulOut );
}

unsigned int	GetColWithModifiedAlpha( unsigned int ulCol, float fAlpha )
{
ulong	ulAlpha = ulCol >> 24;
ulong	ulResult;

	ulAlpha = (ulong)( (float)ulAlpha * fAlpha );
	if ( ulAlpha > 0xff ) ulAlpha = 0xFF;
	ulResult = (ulCol & 0xFFFFFF) | (ulAlpha << 24);
	return( ulResult );
}


int	 RandInt( int nMin, int nMax )		// returns a random int between min and max inclusive
{

	if ( nMax <= nMin )
	{
		return( nMin );
	}
	else
	{
	int	val;
		val = (rand() % ((nMax-nMin)+1)) + nMin;
		return( val );
	}
}

float	FRand( float fLow, float fHigh )
{
int		nRand;
float	fGap = fHigh - fLow;
float	fRand;

	nRand = rand() & 32767;
	fRand = ((float)(nRand) * fGap) / 32768;
	fRand += fLow;
	return( fRand );
}