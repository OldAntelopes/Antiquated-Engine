
#include <math.h>
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
fnDebugPrintHandler		mpfnDebugPrintHandler = NULL;

float mfSysFrameDelta = 0.01f;

void		SysRegisterUserPrintHandler( fnUserPrintFunction pUserPrintHandler )
{
	mfnSysUserPrintFunction = pUserPrintHandler;
}



void		SysUserPrint( int mode, const char* text, ... )
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
	return( tinstricmp( szOut, szIn ) );
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

float			SysGetFrameDelta()
{
	return( mfSysFrameDelta );
}

void				SysSetFrameDelta( float fDelta )
{
	mfSysFrameDelta = fDelta;
}

uint32	SysGetTimeLong(void )
{
uint32	ulCurrentTime = 0;
#ifdef WIN32
__time32_t	xtimet;
//int		nRet;

	_time32( &xtimet );
	ulCurrentTime = *( (uint32*)(&xtimet) );
#else
time_t xTime;

	time( &xTime );
	ulCurrentTime = *( (uint32*)(&xTime) );
#endif
	return( ulCurrentTime );
}



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

// Blend between two colours... When fCol1BlendStrength = 1.0f  its fully col1, when its 0.0g its fully colour 2
uint32		GetBlendedCol( uint32 ulCol1, uint32 ulCol2, float fCol1BlendStrength )
{
float	fR1, fG1, fB1, fA1;
float	fR2, fG2, fB2, fA2;
uint32	ulA, ulR, ulG, ulB;
uint32	ulOut;

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

	ulA = (uint32)( fA1 * 0xFF );
	ulR = (uint32)( fR1 * 0xFF );
	ulG = (uint32)( fG1 * 0xFF );
	ulB = (uint32)( fB1 * 0xFF );

	ulOut = (ulA << 24) | ( ulR << 16) | (ulG << 8) | (ulB);
	return( ulOut );
}

int		ClampInt( int nVal, int nMin, int nMax )
{
	if ( nVal < nMin )
	{
		nVal = nMin;
	}
	else if ( nVal > nMax )
	{
		nVal = nMax;
	}
	return( nVal );
}

float	FSlerp( float fValFrom, float fValTo, float fPhase )
{
float	fSlerpPhase;

	fSlerpPhase = ( fPhase * A180 ) - A90;
	fSlerpPhase = ((sinf( fSlerpPhase )) + 1.0f) * 0.5f;
	return( ( fValFrom * (1.0f-fSlerpPhase) ) + ( fValTo * fSlerpPhase ) );
}


float	FClamp( float fVal, float fMin, float fMax )
{
	if ( fVal < fMin )
	{
		fVal = fMin;
	}
	else if ( fVal > fMax )
	{
		fVal = fMax;
	}
	return( fVal );
}

uint32		GetColValue( COLOUR xCol )
{
ushort		uwRed;
ushort		uwGreen;
ushort		uwBlue;
ushort		uwAlpha;
uint32		ulCol;

	xCol.r = FClamp( xCol.r, 0.0f, 1.0f );
	xCol.g = FClamp( xCol.g, 0.0f, 1.0f );
	xCol.b = FClamp( xCol.b, 0.0f, 1.0f );
	xCol.a = FClamp( xCol.a, 0.0f, 1.0f );

	uwRed = ((ushort)( xCol.r * 255.0f )) ;
	uwGreen = ((ushort)( xCol.g * 255.0f )) ;
	uwBlue = ((ushort)( xCol.b * 255.0f )) ;
	uwAlpha = ((ushort)( xCol.a * 255.0f )) ;

	ulCol = (uwAlpha << 24) | (uwRed << 16) | (uwGreen << 8) | uwBlue;
	return( ulCol );	
}

COLOUR			GetColFloats( uint32 ulCol )
{
COLOUR	xCol;

	xCol.r = ((ulCol >> 16) & 0xFF) / 255.0f;
	xCol.g = ((ulCol >> 8) & 0xFF) / 255.0f;
	xCol.b = (ulCol & 0xFF) / 255.0f;
	xCol.a = ((ulCol >> 24) & 0xFF) / 255.0f;

	return( xCol );
}

uint32	GetColWithModifiedAlpha( uint32 ulCol, float fAlpha )
{
uint32	ulAlpha = ulCol >> 24;
uint32	ulResult;

	ulAlpha = (uint32)( (float)ulAlpha * fAlpha );
	if ( ulAlpha > 0xff ) ulAlpha = 0xFF;
	ulResult = (ulCol & 0xFFFFFF) | (ulAlpha << 24);
	return( (unsigned long)ulResult );
}


int	 RandInt( int nMin, int nMax )		// returns a random int between min and max-1
{

	if ( nMax <= nMin )
	{
		return( nMin );
	}
	else
	{
	int	val;
		val = (rand() % (nMax-nMin)) + nMin;
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

int		mnSystemNumFilesOpen = 0;	

int		SysFileGetNumOpenHandles( void )
{
	return( mnSystemNumFilesOpen );
}

FILE*	SysFileOpen( const char* szFilename, const char* szOpenMode )
{
FILE* pFile = fopen( szFilename, szOpenMode );

	if ( pFile )
	{
		mnSystemNumFilesOpen++;
	}
	return( pFile );
}

int		SysFileRead( unsigned char* pcOutMem, int nReadSize, int nNumReads, FILE* pFile )
{
	return( fread( pcOutMem, nReadSize, nNumReads, (FILE*)pFile ) );

}


void	SysFileCloseAll( void )
{
	mnSystemNumFilesOpen = 0;
	fcloseall();
}

void	SysFileClose( FILE* pFile )
{
	if ( ( pFile ) &&
		 ( mnSystemNumFilesOpen > 0 ) )
	{
		mnSystemNumFilesOpen--;
	}
	fclose( (FILE*)pFile );
}


int		SysGetFileSize( FILE* pFile )
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

int		SysFileWrite( unsigned char* pOutMem, int nWriteSize, int nNumWrites, FILE* pFile )
{
	return( fwrite( pOutMem, nWriteSize, nNumWrites, (FILE*)pFile ) );
	
}

int		SysFileSeek( FILE* pFile, int offset, int mode )
{
	fseek( (FILE*)pFile, offset, SEEK_CUR );
	return( 1 );
}


//-----------------------------------------------------------------
void		SysDebugPrint( const char* format,  ... )
{
#ifdef _DEBUG
char            acString[512]; 
va_list         marker; 

    va_start( marker, format );    
	vsprintf( acString, format, marker ); 
	if ( mpfnDebugPrintHandler )
	{
		mpfnDebugPrintHandler( acString );
	}
	else
	{
	SYS_LOCALTIME	xTime;

		SysGetLocalTime( &xTime );
		printf( "[%d-%d]%02d:%02d:%02d [DBG] ", xTime.wDay, xTime.wMonth, xTime.wHour, xTime.wMinute, xTime.wSecond );
		printf( acString );
		printf( "\n" );
	}
#else
	if ( mpfnDebugPrintHandler )
	{
	char            acString[512]; 
	va_list         marker; 

	    va_start( marker, format );    
		vsprintf( acString, format, marker ); 
		mpfnDebugPrintHandler( acString );
	}
#endif
}
void		SysRegisterDebugPrintHandler( fnDebugPrintHandler pDebugPrintHandler )
{
	mpfnDebugPrintHandler = pDebugPrintHandler;
}

