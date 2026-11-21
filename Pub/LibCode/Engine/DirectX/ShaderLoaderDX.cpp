
#include <stdio.h>
#include "EngineDX.h"

#include <StandardDef.h>
#include <Engine.h>
#include <Interface.h>

#include "ShaderLoaderDX.h"

#ifdef TUD9
LPDIRECT3DVERTEXDECLARATION9	mpStandardVertexDeclaration = NULL; //VertexDeclaration (NEW)
LPDIRECT3DVERTEXDECLARATION9	mpLandscapeVertexDeclaration = NULL; //VertexDeclaration (NEW)
#endif

void	EngineShadersStandardVertexDeclaration( int mode )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShadersStandardVertexDeclaration TBI" );
#else
	switch( mode )
	{
	case 0:
	default:
		mpEngineDevice->SetVertexDeclaration(mpStandardVertexDeclaration);
		break;
	case 1:
		mpEngineDevice->SetVertexDeclaration(mpLandscapeVertexDeclaration);
		break;
	}
#endif
}

//-------------------------------------------------------------------------------------------------------------------------
// Compiled shader cache (opt-in, EngineSetShaderCache). Off by default: every shader compiles from source as before.
//
// The June 2010 D3DX compiler is slow on big shaders (minutes for some), so an app can keep the bytecode on disk. Each entry
// is keyed by a hash of the source, every file it #includes (recursively), the defines, entry point, profile, flags and the
// D3DX version, so editing any of them compiles afresh. Files are written to a temporary name and renamed into place, so
// several processes and threads can share one folder.
//-------------------------------------------------------------------------------------------------------------------------
#ifndef TUD11

#define	SHADER_CACHE_MAGIC			0x31435341		// 'ASC1'
#define	SHADER_CACHE_MAX_INCLUDE_DEPTH	8
#define	SHADER_CACHE_MAX_FILE_SIZE		( 4 * 1024 * 1024 )

static char						msacShaderCacheFolder[MAX_PATH] = "";
static fnEngineShaderCacheLog	mspfnShaderCacheLog = NULL;

typedef struct
{
	unsigned int	ulMagic;
	unsigned int	ulCodeSize;
	unsigned int	ulHashLow;
	unsigned int	ulHashHigh;

} SHADER_CACHE_HEADER;

/***************************************************************************
 * Function    : EngineSetShaderCache
 * Description : Turns on the compiled shader cache in szFolder (created if missing; NULL or "" turns it off). pfnLog, optional,
 *				 gets a line for each shader the engine loads (hit or compile, and how long), on the calling thread
 ***************************************************************************/
void	EngineSetShaderCache( const char* szFolder, fnEngineShaderCacheLog pfnLog )
{
	mspfnShaderCacheLog = pfnLog;
	if ( ( szFolder == NULL ) || ( szFolder[0] == 0 ) )
	{
		msacShaderCacheFolder[0] = 0;
		return;
	}
	strncpy( msacShaderCacheFolder, szFolder, sizeof( msacShaderCacheFolder ) - 1 );
	msacShaderCacheFolder[sizeof( msacShaderCacheFolder ) - 1] = 0;
	CreateDirectoryA( msacShaderCacheFolder, NULL );
}

static unsigned __int64	ShaderCacheHashBytes( unsigned __int64 ullHash, const void* pvData, int nBytes )
{
const unsigned char*	pbData = (const unsigned char*)pvData;
int		nLoop;

	for ( nLoop = 0; nLoop < nBytes; nLoop++ )
	{
		ullHash ^= pbData[nLoop];
		ullHash *= 0x100000001B3ULL;		// FNV-1a 64
	}
	return( ullHash );
}

static unsigned __int64	ShaderCacheHashString( unsigned __int64 ullHash, const char* szText )
{
	if ( szText == NULL ) szText = "";
	// The terminator goes in too, so "ab"+"c" differs from "a"+"bc"
	return( ShaderCacheHashBytes( ullHash, szText, (int)strlen( szText ) + 1 ) );
}

static unsigned char*	ShaderCacheReadFile( const char* szFilename, int* pnSize )
{
FILE*			pFile = fopen( szFilename, "rb" );
unsigned char*	pbData;
long			lSize;

	*pnSize = 0;
	if ( pFile == NULL ) return( NULL );
	fseek( pFile, 0, SEEK_END );
	lSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );
	if ( ( lSize < 0 ) || ( lSize > SHADER_CACHE_MAX_FILE_SIZE ) )
	{
		fclose( pFile );
		return( NULL );
	}
	pbData = (unsigned char*)malloc( lSize + 1 );
	if ( pbData == NULL )
	{
		fclose( pFile );
		return( NULL );
	}
	if ( fread( pbData, 1, lSize, pFile ) != (size_t)lSize )
	{
		free( pbData );
		fclose( pFile );
		return( NULL );
	}
	fclose( pFile );
	pbData[lSize] = 0;
	*pnSize = (int)lSize;
	return( pbData );
}

/***************************************************************************
 * Function    : ShaderCacheHashFile
 * Description : Adds a source file and, in order, every file it #includes (relative to its own folder, as D3DX's default
 *				 include handler finds them) to the hash. Returns FALSE if the top file can't be read.
 ***************************************************************************/
static BOOL	ShaderCacheHashFile( unsigned __int64* pullHash, const char* szFilename, int nDepth )
{
unsigned char*	pbData;
int				nSize;
char			acDir[MAX_PATH];
char			acInclude[MAX_PATH];
const char*		pcSlash;
const char*		pcLine;

	*pullHash = ShaderCacheHashString( *pullHash, szFilename );
	pbData = ShaderCacheReadFile( szFilename, &nSize );
	if ( pbData == NULL )
	{
		*pullHash = ShaderCacheHashString( *pullHash, "<missing>" );
		return( FALSE );
	}
	*pullHash = ShaderCacheHashBytes( *pullHash, &nSize, sizeof( nSize ) );
	*pullHash = ShaderCacheHashBytes( *pullHash, pbData, nSize );

	acDir[0] = 0;
	pcSlash = strrchr( szFilename, '\\' );
	if ( ( pcSlash == NULL ) || ( strrchr( szFilename, '/' ) > pcSlash ) ) pcSlash = strrchr( szFilename, '/' );
	if ( pcSlash )
	{
	int		nDirLen = (int)( pcSlash - szFilename ) + 1;
		if ( nDirLen >= (int)sizeof( acDir ) ) nDirLen = sizeof( acDir ) - 1;
		memcpy( acDir, szFilename, nDirLen );
		acDir[nDirLen] = 0;
	}

	// Every line that starts '#include', commented out or not: an extra file in the hash only costs a read
	pcLine = (const char*)pbData;
	while ( pcLine && *pcLine )
	{
	const char*		pcScan = pcLine;

		while ( ( *pcScan == ' ' ) || ( *pcScan == '\t' ) ) pcScan++;
		if ( strncmp( pcScan, "#include", 8 ) == 0 )
		{
		const char*		pcOpen;
		const char*		pcClose = NULL;

			pcScan += 8;
			while ( ( *pcScan == ' ' ) || ( *pcScan == '\t' ) ) pcScan++;
			pcOpen = pcScan;
			if ( *pcOpen == '"' ) pcClose = strchr( pcOpen + 1, '"' );
			else if ( *pcOpen == '<' ) pcClose = strchr( pcOpen + 1, '>' );
			if ( ( pcClose ) && ( pcClose > pcOpen + 1 ) && ( nDepth < SHADER_CACHE_MAX_INCLUDE_DEPTH ) )
			{
			int		nNameLen = (int)( pcClose - pcOpen ) - 1;

				if ( strlen( acDir ) + nNameLen < sizeof( acInclude ) )
				{
					snprintf( acInclude, sizeof( acInclude ), "%s%.*s", acDir, nNameLen, pcOpen + 1 );
					ShaderCacheHashFile( pullHash, acInclude, nDepth + 1 );
				}
			}
		}
		pcLine = strchr( pcLine, '\n' );
		if ( pcLine ) pcLine++;
	}
	free( pbData );
	return( TRUE );
}

/***************************************************************************
 * Function    : ShaderCacheMakePath
 * Description : The cache file for a shader: <folder>\<file stem>[_<defines>]_<profile>_<hash>.cso. pcPrefixLen gets the length
 *				 of the part before the hash, so older entries for the same shader can be found and removed.
 ***************************************************************************/
static void	ShaderCacheMakePath( char* pcOut, int nOutSize, const char* szFilename, const D3DXMACRO* pxMacros, const char* szProfile, unsigned __int64 ullHash, int* pnPrefixLen )
{
char			acStem[96];
const char*		pcName = szFilename;
const char*		pcDot;
int				nLen;
int				nLoop;

	if ( strrchr( pcName, '\\' ) ) pcName = strrchr( pcName, '\\' ) + 1;
	if ( strrchr( pcName, '/' ) ) pcName = strrchr( pcName, '/' ) + 1;
	pcDot = strrchr( pcName, '.' );
	nLen = pcDot ? (int)( pcDot - pcName ) : (int)strlen( pcName );
	if ( nLen > 40 ) nLen = 40;
	snprintf( acStem, sizeof( acStem ), "%.*s", nLen, pcName );
	// Each define's name, so the variants of one source keep their own entries
	for ( nLoop = 0; ( pxMacros ) && ( pxMacros[nLoop].Name ) && ( nLoop < 4 ); nLoop++ )
	{
	int		nUsed = (int)strlen( acStem );
		snprintf( acStem + nUsed, sizeof( acStem ) - nUsed, "_%.20s", pxMacros[nLoop].Name );
	}
	for ( nLoop = 0; acStem[nLoop]; nLoop++ )
	{
	char	c = acStem[nLoop];
		if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || ( c == '_' ) || ( c == '-' ) ) ) acStem[nLoop] = '_';
	}
	snprintf( pcOut, nOutSize, "%s\\%s_%s_", msacShaderCacheFolder, acStem, szProfile );
	*pnPrefixLen = (int)strlen( pcOut );
	nLen = (int)strlen( pcOut );
	snprintf( pcOut + nLen, nOutSize - nLen, "%08x%08x.cso", (unsigned int)( ullHash >> 32 ), (unsigned int)ullHash );
}

static BOOL	ShaderCacheLoad( const char* szCachePath, unsigned __int64 ullHash, LPD3DXBUFFER* ppCode )
{
unsigned char*			pbData;
int						nSize;
SHADER_CACHE_HEADER*	pxHeader;
BOOL					bOK = FALSE;

	pbData = ShaderCacheReadFile( szCachePath, &nSize );
	if ( pbData == NULL ) return( FALSE );
	pxHeader = (SHADER_CACHE_HEADER*)pbData;
	if ( ( nSize > (int)sizeof( SHADER_CACHE_HEADER ) ) &&
		 ( pxHeader->ulMagic == SHADER_CACHE_MAGIC ) &&
		 ( pxHeader->ulCodeSize == (unsigned int)( nSize - sizeof( SHADER_CACHE_HEADER ) ) ) &&
		 ( pxHeader->ulHashLow == (unsigned int)ullHash ) &&
		 ( pxHeader->ulHashHigh == (unsigned int)( ullHash >> 32 ) ) &&
		 ( D3DXGetShaderVersion( (const DWORD*)( pbData + sizeof( SHADER_CACHE_HEADER ) ) ) != 0 ) )
	{
		if ( SUCCEEDED( D3DXCreateBuffer( pxHeader->ulCodeSize, ppCode ) ) )
		{
			memcpy( (*ppCode)->GetBufferPointer(), pbData + sizeof( SHADER_CACHE_HEADER ), pxHeader->ulCodeSize );
			bOK = TRUE;
		}
	}
	free( pbData );
	return( bOK );
}

static void	ShaderCacheSave( const char* szCachePath, int nPrefixLen, unsigned __int64 ullHash, LPD3DXBUFFER pCode )
{
char					acTemp[MAX_PATH + 32];
char					acPattern[MAX_PATH];
FILE*					pFile;
SHADER_CACHE_HEADER		xHeader;
BOOL					bWritten = FALSE;
WIN32_FIND_DATAA		xFind;
HANDLE					hFind;

	xHeader.ulMagic = SHADER_CACHE_MAGIC;
	xHeader.ulCodeSize = (unsigned int)pCode->GetBufferSize();
	xHeader.ulHashLow = (unsigned int)ullHash;
	xHeader.ulHashHigh = (unsigned int)( ullHash >> 32 );

	snprintf( acTemp, sizeof( acTemp ), "%s.%u_%u.tmp", szCachePath, (unsigned int)GetCurrentProcessId(), (unsigned int)GetCurrentThreadId() );
	pFile = fopen( acTemp, "wb" );
	if ( pFile == NULL ) return;
	if ( ( fwrite( &xHeader, sizeof( xHeader ), 1, pFile ) == 1 ) &&
		 ( fwrite( pCode->GetBufferPointer(), xHeader.ulCodeSize, 1, pFile ) == 1 ) )
	{
		bWritten = TRUE;
	}
	if ( fclose( pFile ) != 0 ) bWritten = FALSE;
	if ( ( bWritten == FALSE ) || ( MoveFileExA( acTemp, szCachePath, MOVEFILE_REPLACE_EXISTING ) == FALSE ) )
	{
		DeleteFileA( acTemp );
		return;
	}

	// Remove older entries for the same shader (a source edit leaves the old one behind otherwise). One in use by
	// another process just stays until next time.
	if ( nPrefixLen <= 0 || nPrefixLen >= MAX_PATH - 8 ) return;
	memcpy( acPattern, szCachePath, nPrefixLen );
	strcpy( acPattern + nPrefixLen, "*.cso" );
	hFind = FindFirstFileA( acPattern, &xFind );
	if ( hFind != INVALID_HANDLE_VALUE )
	{
	const char*		pcOwnName = szCachePath + strlen( msacShaderCacheFolder ) + 1;
	int				nStemLen = nPrefixLen - ( (int)strlen( msacShaderCacheFolder ) + 1 );

		do
		{
			// Exactly a 16-digit hash after the prefix, so another shader whose stem starts the same is left alone
			if ( ( _stricmp( xFind.cFileName, pcOwnName ) != 0 ) &&
				 ( strlen( xFind.cFileName ) == (size_t)( nStemLen + 16 + 4 ) ) )
			{
			char	acOld[MAX_PATH + 8];
				snprintf( acOld, sizeof( acOld ), "%s\\%s", msacShaderCacheFolder, xFind.cFileName );
				DeleteFileA( acOld );
			}
		} while ( FindNextFileA( hFind, &xFind ) );
		FindClose( hFind );
	}
}

#endif // TUD11

/***************************************************************************
 * Function    : EngineCompileShaderFileCached
 * Description : As D3DXCompileShaderFromFile (no include handler), through the compiled shader cache when EngineSetShaderCache
 *				 has turned it on. Safe to call from a worker thread. On a cache hit *ppErrors is NULL and the constant
 *				 table comes from the bytecode. pbFromCache (optional) says which happened.
 ***************************************************************************/
HRESULT		EngineCompileShaderFileCached( const char* szFilename, const D3DXMACRO* pxMacros, const char* szEntry, const char* szProfile, DWORD ulFlags,
										   LPD3DXBUFFER* ppCode, LPD3DXBUFFER* ppErrors, LPD3DXCONSTANTTABLE* ppTable, BOOL* pbFromCache )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineCompileShaderFileCached TBI" );
	return( E_FAIL );
#else
HRESULT				ret;
unsigned __int64	ullHash = 0xCBF29CE484222325ULL;
char				acCachePath[MAX_PATH + 64];
int					nPrefixLen = 0;
unsigned int		ulVersion = D3DX_SDK_VERSION;
int					nLoop;

	if ( pbFromCache ) *pbFromCache = FALSE;
	if ( ppErrors ) *ppErrors = NULL;
	if ( msacShaderCacheFolder[0] == 0 )
	{
		return( D3DXCompileShaderFromFile( szFilename, pxMacros, NULL, szEntry, szProfile, ulFlags, ppCode, ppErrors, ppTable ) );
	}

	ullHash = ShaderCacheHashString( ullHash, "AntiquatedShaderCache1" );
	ullHash = ShaderCacheHashBytes( ullHash, &ulVersion, sizeof( ulVersion ) );
	ullHash = ShaderCacheHashBytes( ullHash, &ulFlags, sizeof( ulFlags ) );
	ullHash = ShaderCacheHashString( ullHash, szEntry );
	ullHash = ShaderCacheHashString( ullHash, szProfile );
	for ( nLoop = 0; ( pxMacros ) && ( pxMacros[nLoop].Name ); nLoop++ )
	{
		ullHash = ShaderCacheHashString( ullHash, pxMacros[nLoop].Name );
		ullHash = ShaderCacheHashString( ullHash, pxMacros[nLoop].Definition );
	}
	if ( ShaderCacheHashFile( &ullHash, szFilename, 0 ) == FALSE )
	{
		// Can't read the source: let D3DX report it
		return( D3DXCompileShaderFromFile( szFilename, pxMacros, NULL, szEntry, szProfile, ulFlags, ppCode, ppErrors, ppTable ) );
	}

	ShaderCacheMakePath( acCachePath, sizeof( acCachePath ), szFilename, pxMacros, szProfile, ullHash, &nPrefixLen );
	if ( ShaderCacheLoad( acCachePath, ullHash, ppCode ) )
	{
		if ( ppTable )
		{
			if ( FAILED( D3DXGetShaderConstantTable( (const DWORD*)(*ppCode)->GetBufferPointer(), ppTable ) ) )
			{
				// Shouldn't happen (the compiler writes the table into the bytecode); compile instead
				SAFE_RELEASE( *ppCode );
				*ppTable = NULL;
				DeleteFileA( acCachePath );
				goto compile;
			}
		}
		if ( pbFromCache ) *pbFromCache = TRUE;
		return( S_OK );
	}

compile:
	ret = D3DXCompileShaderFromFile( szFilename, pxMacros, NULL, szEntry, szProfile, ulFlags, ppCode, ppErrors, ppTable );
	if ( SUCCEEDED( ret ) && ( *ppCode ) )
	{
		ShaderCacheSave( acCachePath, nPrefixLen, ullHash, *ppCode );
	}
	return( ret );
#endif
}

#ifndef TUD11
static void	ShaderCacheLogLoad( const char* szFilename, const char* szDefine, BOOL bFromCache, BOOL bOK, unsigned int ulStartTick )
{
char	acLine[384];

	if ( ( mspfnShaderCacheLog == NULL ) || ( msacShaderCacheFolder[0] == 0 ) ) return;
	snprintf( acLine, sizeof( acLine ), "Shader cache: %s%s%s %s in %u ms", szFilename, szDefine ? " " : "", szDefine ? szDefine : "",
			  bOK ? ( bFromCache ? "loaded from cache" : "compiled" ) : "FAILED to compile", (unsigned int)( GetTickCount() - ulStartTick ) );
	mspfnShaderCacheLog( acLine );
}
#endif

LPGRAPHICSVERTEXSHADER		EngineLoadVertexShader( const char* szShaderName, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable, int flags )
{
LPGRAPHICSVERTEXSHADER			pVertexShader = NULL;

#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineLoadVertexShader TBI" );
#else
HRESULT		ret;
LPD3DXBUFFER		pErrorBuffer = NULL;
LPD3DXBUFFER		pShaderCode = NULL; //Temporary buffer (NEW)

char		acShaderFilename[256];

	sprintf( acShaderFilename, "Data\\Shaders\\%s.vsh", szShaderName );

	if ( mpStandardVertexDeclaration == NULL )
	{
	// Standard ENGINEVERTEX with two UVS
	D3DVERTEXELEMENT9 declStandard[] = { { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
							 { 0,12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
							 { 0,24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
						     { 0,28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
						     { 0,36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
								 D3DDECL_END()  };
	D3DVERTEXELEMENT9 declLandscape[] = { { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
							 { 0,12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
							 { 0,24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
						     { 0,28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
						     { 0,36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
							 { 0,44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
								 D3DDECL_END()  };
		mpEngineDevice->CreateVertexDeclaration( declStandard, &mpStandardVertexDeclaration );
		mpEngineDevice->CreateVertexDeclaration( declLandscape, &mpLandscapeVertexDeclaration );
	}

	{
	BOOL			bFromCache = FALSE;
	unsigned int	ulStart = GetTickCount();

		// D3DXCompileShaderFromFile, unless the opt-in shader cache (EngineSetShaderCache) has it already
		ret = EngineCompileShaderFileCached( acShaderFilename, NULL, "vs_main", "vs_3_0", 0, &pShaderCode, &pErrorBuffer, ppxConstantTable, &bFromCache );
		ShaderCacheLogLoad( acShaderFilename, NULL, bFromCache, SUCCEEDED( ret ), ulStart );
	}
	if(FAILED(ret))
	{
		if ( pErrorBuffer )
		{
		char*		pcErrors = (char*)pErrorBuffer->GetBufferPointer();
#ifdef TOOL
			printf( pcErrors );
#else
#ifdef _DEBUG
			MessageBox(NULL, pcErrors, "Error", MB_OK);
#endif
#endif
		}
		else
		{
		FILE*		pFile = fopen( acShaderFilename, "rb" );

			if ( pFile == NULL )
			{
				printf( "File not found\n" );
			}
			else
			{
				fclose( pFile );
			}
		}
//		PrintConsoleCR( "*Warning*: Vertex shader load failed - Some graphical effects may be missing", 0 );
		return( NULL );
	}

	mpEngineDevice->CreateVertexShader((DWORD*)pShaderCode->GetBufferPointer(), &pVertexShader );
	pShaderCode->Release();
#endif

	return( pVertexShader );

}




// EngineLoadPixelShader's body, with optional preprocessor macros (pxMacros: a D3DXMACRO list; NULL = none, as before)
static LPGRAPHICSPIXELSHADER		EngineLoadPixelShaderMacros( const char* szShaderName, const void* pxMacros, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable )
{
HRESULT		ret;
LPGRAPHICSPIXELSHADER			pPixelShader = NULL;

#ifdef TUD11
	PANIC_IF( TRUE, "EngineLoadPixelShader DX11 TBI" );
	
#else
LPD3DXBUFFER		pErrorBuffer = NULL;
LPD3DXBUFFER		pShaderCode = NULL; //Temporary buffer (NEW)
char		acShaderFilename[256];
FILE*		pFile;

	sprintf( acShaderFilename, "Data\\Shaders\\%s.psh", szShaderName );

	pFile = fopen( acShaderFilename, "rb" );
	if ( pFile )
	{
		fclose( pFile );
	}
	else
	{
	char	acBuff[512];
		sprintf(acBuff, "Shader file ('%s') not found. Is working folder set to correct Exe dir?", acShaderFilename);
		PANIC_IF( TRUE, acBuff );
	}
	//------------ Init Pixel Shader
	{
	BOOL			bFromCache = FALSE;
	unsigned int	ulStart = GetTickCount();
	const D3DXMACRO*	pxMacroList = (const D3DXMACRO*)pxMacros;

		// D3DXCompileShaderFromFile, unless the opt-in shader cache (EngineSetShaderCache) has it already
		ret = EngineCompileShaderFileCached( acShaderFilename, pxMacroList, "ps_main", "ps_3_0", 0, &pShaderCode, &pErrorBuffer, ppxConstantTable, &bFromCache );
		ShaderCacheLogLoad( acShaderFilename, pxMacroList ? pxMacroList[0].Name : NULL, bFromCache, SUCCEEDED( ret ), ulStart );
	}
	if(FAILED(ret))
	{
		if ( pErrorBuffer )
		{
		char*		pcErrors = (char*)pErrorBuffer->GetBufferPointer();
			printf( pcErrors );
#ifdef TOOL
			MessageBox(NULL, pcErrors, "Error", MB_OK);
#else
#ifdef _DEBUG
			MessageBox(NULL, pcErrors, "Error", MB_OK);
#endif
#endif
		}
//		PrintConsoleCR( "*Warning*: Pixel shader load failed - Some graphical effects may be missing", 0 );
		return( NULL );
	}
		
	mpEngineDevice->CreatePixelShader((DWORD*)pShaderCode->GetBufferPointer(), &pPixelShader);
	pShaderCode->Release();
#endif

	return( pPixelShader );
}

LPGRAPHICSPIXELSHADER		EngineLoadPixelShader( const char* szShaderName, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable, int flags )
{
	return( EngineLoadPixelShaderMacros( szShaderName, NULL, ppxConstantTable ) );
}

/***************************************************************************
 * Function    : EngineLoadPixelShaderWithDefine
 * Description : As EngineLoadPixelShader, compiled with one preprocessor define (szDefine, set to 1; NULL = none), so one
 *				 .psh can build more than one shader (ModelMaterialsAddPixelShaderVariant)
 ***************************************************************************/
LPGRAPHICSPIXELSHADER		EngineLoadPixelShaderWithDefine( const char* szShaderName, const char* szDefine, LPGRAPHICSCONSTANTBUFFER* ppxConstantTable )
{
#ifdef TUD11
	return( EngineLoadPixelShaderMacros( szShaderName, NULL, ppxConstantTable ) );
#else
D3DXMACRO	axDefines[2];

	if ( szDefine == NULL )
	{
		return( EngineLoadPixelShaderMacros( szShaderName, NULL, ppxConstantTable ) );
	}
	axDefines[0].Name = szDefine;
	axDefines[0].Definition = "1";
	axDefines[1].Name = NULL;
	axDefines[1].Definition = NULL;
	return( EngineLoadPixelShaderMacros( szShaderName, axDefines, ppxConstantTable ) );
#endif
}



void		EngineReleaseConstantBuffer( LPGRAPHICSCONSTANTBUFFER* ppConstantBuffer )
{
#ifdef TUD11
	SAFE_FREE( *ppConstantBuffer );
#else
	SAFE_RELEASE( *ppConstantBuffer );
#endif

}

void				EngineShaderConstantsSetFloat( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, float fValue )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShaderConstantsSetFloat TBI" );
#else
	D3DXHANDLE handle; 

	handle = pConstantBuffer->GetConstantByName(NULL, szConstantName);
	if ( handle )
	{
		pConstantBuffer->SetFloat( mpEngineDevice, handle, fValue );
	}
#endif

}

void				EngineShaderConstantsSetVect( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, const VECT* pxVect )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShaderConstantsSetVect TBI" );
#else
	D3DXHANDLE handle; 

	handle = pConstantBuffer->GetConstantByName(NULL, szConstantName);
	if ( handle )
	{
		pConstantBuffer->SetFloatArray( mpEngineDevice, handle, (const float*)pxVect, 3 );
	}
#endif
}

void				EngineShaderConstantsSetMatrix( LPGRAPHICSCONSTANTBUFFER pConstantBuffer, const char* szConstantName, const ENGINEMATRIX* pxMatrix )
{
#ifdef TUD11
	PANIC_IF( TRUE, "DX11 EngineShaderConstantsSetMatrix TBI" );
#else
	D3DXHANDLE handle; 

	handle = pConstantBuffer->GetConstantByName(NULL, szConstantName);
	if ( handle )
	{
		pConstantBuffer->SetMatrix( mpEngineDevice, handle, (D3DXMATRIX*)pxMatrix );
	}
#endif

}
