
#ifndef SERVER

#include <stdio.h>
#include <string.h>

#include <StandardDef.h>
#include <System.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>
#include <CodeUtil.h>
	
#include "SceneMap.h"

#include "../LibCode/Engine/ModelArchive.h"


//-----------------------------------------------------------------------------------------

fnAddItemCallback		m_fnAddItemCallback = NULL; 

char		mszLocalDownloadPath[256] = "Data/Scenes/";
const char*		mszLocalDownloadScriptFile = "Data/Scenes/download.uim";
char		mszElementDownloadPath[256] = "Data/Scenes/";
char		mszSceneMapLoadRootPath[256] = "";

BOOL		msbLoadWarningAbsolutePaths = FALSE;

SceneMap*	mpxCurrentScene = NULL;


//-------------------------------------------------------------
// SceneMap
//

SceneMapElement* SceneMap::AddNewElement( SceneMapElement* pParent, SceneMapElement::ELEMENT_TYPES eType )
{
SceneMapElement*		pNewElement;

	switch( eType )
	{
	case SceneMapElement::REGION:
		pNewElement = new SceneRegionElement;
		break;	
	case SceneMapElement::FURNITURE:
		pNewElement = new SceneFurnitureElement;
		break;
	case SceneMapElement::ROOM:
		pNewElement = new SceneRoomElement;
		break;
	case SceneMapElement::LIGHT:
		pNewElement = new SceneLightElement;
		break;
	default:
		// This element type has not been implemented yet
		return( NULL );
		break;
	}

	if ( !pParent->mpChildElement )
	{
		pParent->mpChildElement = pNewElement;
	}
	else
	{
	SceneMapElement*		pBrotherElement = pParent->mpChildElement;
		while( pBrotherElement->mpBrotherElement != NULL )
		{
			pBrotherElement = pBrotherElement->mpBrotherElement;
		}
		pBrotherElement->mpBrotherElement = pNewElement;
	}
	return( pNewElement );
}

VECT SceneMap::ParseGetVector3( char* pcParam )
{
char*	pcVal1 = pcParam;
char*	pcVal2;
char*	pcVal3;
VECT	vecRet;
	vecRet.x = vecRet.y = vecRet.z = 0.0f;
	pcVal2 = SplitStringSep( pcVal1, ',' );
	if ( pcVal2[0] != 0 )
	{
		pcVal3 = SplitStringSep( pcVal2, ',' );
		if ( pcVal3[0] != 0 )
		{
			vecRet.x = (float)(strtod( pcVal1, NULL ));	
			vecRet.y = (float)(strtod( pcVal2, NULL ));	
			vecRet.z = (float)(strtod( pcVal3, NULL ));	
		}
	}
	return( vecRet );
}

VECT SceneMap::ParseDegreesAngleVector3( char* pcParam )
{
VECT	vecRet;
	vecRet = ParseGetVector3( pcParam );
	vecRet.x = (vecRet.x * TwoPi) / 360.0f;
	vecRet.y = (vecRet.y * TwoPi) / 360.0f;
	vecRet.z = (vecRet.z * TwoPi) / 360.0f;
	return( vecRet );
}


void SceneMap::ParseDownloadedElement( const char* szName )
{
	switch ( m_DownloadType )
	{
	case ELEMENT_DOWNLOAD_ROOM_MODEL:
		mpCurrentParseElement->SetModel(szName);
		break;
	case ELEMENT_DOWNLOAD_ROOM_TEXTURE:
		mpCurrentParseElement->SetTexture(szName);
		break;
	}
	//Continue parsing the uim file..
	ParseLoadedScript();
}

void SceneMap::ElementHTTPDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName )
{
	if ( nRet == FETCHFILE_SUCCESS )
	{
		if ( mpxCurrentScene )
		{
			mpxCurrentScene->ParseDownloadedElement( szName );
		}
	}
	else if ( nRet == FETCHFILE_NOT_FOUND )
	{
	EngineGameInterface*		pGameInterface = EngineGetGameInterface();

		if ( pGameInterface )
		{
		char	acString[512];
			sprintf( acString, "Interior Scene file not found: %s", szName );
			pGameInterface->DebugPrint( 0, acString );
		}

		if ( mpxCurrentScene )
		{
			mpxCurrentScene->ParseDownloadedElement( szName );
		}
	}
}


void SceneMap::ElementServerDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName )
{
	if ( nRet == FETCHFILE_SUCCESS )
	{
		if ( mpxCurrentScene )
		{
			mpxCurrentScene->ParseDownloadedElement( szName );
		}
	}
}


BOOL	SceneMap::HasFullyLoaded( void )
{
	if ( mboIsDownloading )
	{
		return( FALSE );
	}
	return( TRUE );
}




//-----------------------------------
// FetchElement
//  Returns TRUE if the element download is in progress
///  (FALSE if the element was loaded immediately)
BOOL	SceneMap::FetchElement( char* szName, eELEMENT_DOWNLOAD_TYPE eType, bool bUsingHTTP )
{
SceneMapGameInterface*		pSceneMapGameInterface = EngineGetSceneMapGameInterface();
	m_DownloadType = eType;

	if ( pSceneMapGameInterface )
	{
		if ( bUsingHTTP )
		{
			pSceneMapGameInterface->FetchElementFile( szName, mszLocalDownloadPath, m_szHTTPRoot, ElementHTTPDownloadCallback );
		}
		else
		{
			pSceneMapGameInterface->FetchElementFile( szName, mszLocalDownloadPath, NULL, ElementServerDownloadCallback );
		}
	}
	else
	{
		// TODO - Default fopen implementation
	}
	return( TRUE );
}


//-----------------------------------
// LoaderParseNextLine
//  Returns TRUE if the line parsed requires a download
BOOL SceneMap::LoaderParseNextLine( char* pcLine )
{
char*	pcVal;
//int		nVal;
SceneMapElement*		pNewElement;

	pcVal = SplitStringSep( pcLine, '=' );
	StripWhiteSpaceRight( pcVal );
	if ( stricmp( pcLine, "room" ) == 0 )
	{
		if ( mpCurrentParseElement )
		{
			mpCurrentParseElement->OnParseComplete();
		}
		mnNumberOfRoomsParsed++;
		pNewElement = AddNewElement( &mFirstElement, SceneMapElement::ROOM );
		mpCurrentParseElement = pNewElement;
		mpCurrentParseRoom = pNewElement;
		//if we got the script from a website, thats where we get the rooms n stuff too
		if ( mboIsDownloading )
		{
			return( FetchElement( pcVal, ELEMENT_DOWNLOAD_ROOM_MODEL,mboIsHTTPDownload ) );
		}
		else	// local load - just get the model name directly
		{
		FILE*	pFile;
		char	acFullPath[256];

			sprintf( acFullPath, "%s\\%s", mszSceneMapLoadRootPath, pcVal );
			pFile = fopen( acFullPath, "rb" );

			if ( !pFile )
			{
				SysDebugPrint( 0, "Missing file- %s", pcVal );
				mboIsDownloading = true;
				return( FetchElement( pcVal, ELEMENT_DOWNLOAD_ROOM_MODEL,mboIsHTTPDownload ) );		
			}
			else
			{
				fclose( pFile );
			}
			// If the filename reference in the UIM contains a : its an absolute path
			if ( strstr( pcVal, ":" ) )
			{
				pNewElement->SetModel( pcVal );
				msbLoadWarningAbsolutePaths = TRUE;
			}
			else
			{
			char	acString[256];

				sprintf( acString, "%s\\%s", mszSceneMapLoadRootPath, pcVal );
				pNewElement->SetModel( acString );
			}
		}
	}
	else if ( mpCurrentParseElement )
	{
		if ( stricmp( pcLine, "furn" ) == 0 )
		{
			pNewElement = AddNewElement( mpCurrentParseRoom, SceneMapElement::FURNITURE );
			mpCurrentParseElement = pNewElement;
			if ( mboIsDownloading )
			{
				return( FetchElement( pcVal, ELEMENT_DOWNLOAD_ROOM_MODEL,mboIsHTTPDownload ) );
			}
			else	// local load - just get the model name directly
			{
			FILE*	pFile;
			char	acFullPath[256];

				sprintf( acFullPath, "%s\\%s", mszSceneMapLoadRootPath, pcVal );
				pFile = fopen( acFullPath, "rb" );
				if ( !pFile )
				{
					mboIsDownloading = true;
					return( FetchElement( pcVal, ELEMENT_DOWNLOAD_ROOM_MODEL,mboIsHTTPDownload ) );		
				}
				else
				{
					fclose( pFile );
				}
				// If the filename reference in the UIM contains a : its an absolute path
				if ( strstr( pcVal, ":" ) )
				{
					pNewElement->SetModel( pcVal );
					msbLoadWarningAbsolutePaths = TRUE;
				}
				else
				{
				char	acString[256];
					sprintf( acString, "%s\\%s", mszSceneMapLoadRootPath, pcVal );
					pNewElement->SetModel( acString );
				}
			}
		}
		else if ( stricmp( pcLine, "region" ) == 0 )
		{
		SceneRegionElement*		pRegion;
			pNewElement = AddNewElement( &mFirstElement, SceneMapElement::REGION );
			mpCurrentParseElement = pNewElement;

			pRegion = (SceneRegionElement*)( pNewElement );
			pRegion->SetRegionType( pcVal );

		}
		else if ( stricmp( pcLine, "param" ) == 0 )
		{
			// if region, need to set SetRegionParam
			if ( stricmp( mpCurrentParseElement->GetTypeName(), "Region" ) == 0 )
			{
			SceneRegionElement*		pRegion = (SceneRegionElement*)mpCurrentParseElement;
				pRegion->SetRegionParam( pcVal );
			}
		}
		else if ( stricmp( pcLine, "scale" ) == 0 )
		{
			// if region, need to set Setscale
			if ( stricmp( mpCurrentParseElement->GetTypeName(), "Region" ) == 0 )
			{
			SceneRegionElement*		pRegion = (SceneRegionElement*)mpCurrentParseElement;
			float		fScale = (float)atof(pcVal) * 0.0686367f;
				pRegion->SetScale( fScale );
			}
		}
		else if ( stricmp( pcLine, "tex" ) == 0 )
		{
			//if we got the script from a website, thats where we get the rooms n stuff too
			if ( mboIsDownloading )
			{
				if ( ( pcVal ) &&
					 ( pcVal[0] != 0 ) )
				{
					return( FetchElement( pcVal, ELEMENT_DOWNLOAD_ROOM_TEXTURE,mboIsHTTPDownload ) );
				}
			}
			else	// local load - just get the model name directly
			{
				if ( ( pcVal ) &&
					 ( pcVal[0] != 0 ) )
				{
				FILE*	pFile;
				char	acFullPath[256];

					sprintf( acFullPath, "%s\\%s", mszSceneMapLoadRootPath, pcVal );
					pFile = fopen( acFullPath, "rb" );
					if ( !pFile )
					{
						mboIsDownloading = true;
						return( FetchElement( pcVal, ELEMENT_DOWNLOAD_ROOM_TEXTURE,mboIsHTTPDownload ) );		
					}
					else
					{
						fclose( pFile );
					}
					// If the filename reference in the UIM contains a : its an absolute path
					if ( strstr( pcVal, ":" ) )
					{
						mpCurrentParseElement->SetTexture( pcVal );
						msbLoadWarningAbsolutePaths = TRUE;
					}
					else
					{
					char	acString[256];

						sprintf( acString, "%s\\%s", mszSceneMapLoadRootPath, pcVal );
						mpCurrentParseElement->SetTexture( acString );
					}
				}
				else
				{
					mpCurrentParseElement->SetTexture( NULL );
				}
			}
		}
		else if ( stricmp( pcLine, "pos" ) == 0 )
		{
			mpCurrentParseElement->mPos = ParseGetVector3( pcVal );
		}
		else if ( stricmp( pcLine, "rot" ) == 0 )
		{
			mpCurrentParseElement->mRot = ParseDegreesAngleVector3( pcVal );
		}
	}
	return( FALSE );
}


SceneMapElement* 	SceneMap::FindParentOf( SceneMapElement* pChild )
{
	if ( mFirstElement.mpChildElement )
	{
		return( mFirstElement.mpChildElement->FindParentOf( pChild ) );
	}
	return( NULL );
}

//-----------------------------------
// Update
void SceneMap::Update( const VECT* pxPlayerPos )
{
	if ( mFirstElement.mpChildElement )
	{
		mFirstElement.mpChildElement->Update( pxPlayerPos );
	}
}

#ifdef TOOL
//-----------------------------------
// Save
BOOL SceneMap::Save( char* szFilename )
{
FILE*	pFile;
char*	pcExtension = szFilename + ( strlen( szFilename ) - 3 );
char	acString[256];

	if ( stricmp( pcExtension, "uim" ) != 0 )
	{
		sprintf( acString, "%s.uim", szFilename );
	}
	else
	{
		strcpy( acString, szFilename );
	}

	pFile = fopen( acString, "wb" );
	if ( pFile != NULL )
	{
		mFirstElement.mpChildElement->Save(pFile);
	
		fclose( pFile );
		return( TRUE );
	}
	return( FALSE );
}


class	FileEntry
{
public:
	FileEntry( const char* szFilename, int nFilesize )
	{
		mszFilename = (char*)malloc( strlen( szFilename ) + 1 );
		strcpy( mszFilename, szFilename );
		mnFilesize = nFilesize;
	}
	~FileEntry()
	{
		free( mszFilename );
	}
	const char*	GetFileName() { return( mszFilename ); }
	int			GetFileSize() { return( mnFilesize ); }

	FileEntry*	GetNext() { return( mpNext ); }
	void		SetNext( FileEntry* pFileEntry ) { mpNext = pFileEntry; }
private:
	char*	mszFilename;
	FileEntry*	mpNext;
	int		mnFilesize;
};


class TableOfContents
{
public:
	TableOfContents()
	{
		mpFirst = NULL;
		mpLast = NULL;
		mnNumFiles = 0;
	}

	void	Reset( void )
	{
	FileEntry*	pFileEntry = mpFirst;
	FileEntry*	pNextFileEntry;	

		while( pFileEntry )
		{
			pNextFileEntry = pFileEntry->GetNext();
			delete pFileEntry;
			pFileEntry = pNextFileEntry;
		}
		mpFirst = NULL;
		mpLast = NULL;
		mnNumFiles = 0;
	}

	void	Add( const char* szFilename, int nFileSize )
	{
	FileEntry*	pFileEntry = mpFirst;
		while ( pFileEntry )
		{
			if ( stricmp( pFileEntry->GetFileName(), szFilename ) == 0 )
			{
				return;
			}
			pFileEntry = pFileEntry->GetNext();
		}

		FileEntry*		pNewEntry = new FileEntry( szFilename, nFileSize );

		if ( mpLast )
		{
			mpLast->SetNext( pNewEntry );	
		}
		else 
		{
			mpFirst = pNewEntry;
			mpLast = pNewEntry;
		}
		pNewEntry->SetNext( NULL );
		mpLast = pNewEntry;
		mnNumFiles++;
	}

	void	Save( FILE* pFile )
	{
	FileEntry*	pFileEntry = mpFirst;
	const char*		pcRunner;
	const char*		pcFilename;
	BYTE	bLen;
	int		nFileSize;
	int		nOffset = 0;

		while( pFileEntry )
		{
			pcFilename = pFileEntry->GetFileName();
			nFileSize = pFileEntry->GetFileSize();
			pcRunner = pcFilename + strlen( pcFilename );
			while ( ( pcRunner != pcFilename ) &&
					( *pcRunner != '\\' ) &&
					( *pcRunner != '/' ) )
			{
				pcRunner--;
			}
			if ( ( *pcRunner == '\\' ) ||
				 ( *pcRunner == '/' ) )
			{
				pcRunner++;
			}

			pcFilename = pcRunner;
			bLen = (BYTE)( strlen( pcFilename ) );
			fwrite( &nOffset, sizeof(nOffset), 1, pFile );
			fwrite( &nFileSize, sizeof(nFileSize), 1, pFile );
			fwrite( &bLen, sizeof( BYTE ), 1, pFile );
			fwrite( pcFilename, bLen, 1, pFile );

			nOffset += nFileSize;

			pFileEntry = pFileEntry->GetNext();
		}
	}

	void	SaveFiles( FILE* pFile )
	{
	FileEntry*	pFileEntry = mpFirst;
	const char*		pcFilename;
	void*		pMem;
	FILE*	pLoadFile;
	
		while( pFileEntry )
		{
			pcFilename = pFileEntry->GetFileName();
			pLoadFile = fopen( pcFilename, "rb" );
			if ( pLoadFile )
			{
				pMem = malloc( pFileEntry->GetFileSize() );
				fread( pMem, pFileEntry->GetFileSize(), 1, pLoadFile );
				fclose( pLoadFile );
				fwrite( pMem,pFileEntry->GetFileSize(), 1, pFile );
				free( pMem );
			}
			pFileEntry = pFileEntry->GetNext();
		}
	}

	int		GetNumberOfFiles( void ) { return( mnNumFiles ); }
private:
	FileEntry*		mpFirst;
	FileEntry*		mpLast;
	int		mnNumFiles;
};

TableOfContents		m_TableOfContents;

void	TableOfContentsAddFileReference( const char* szFilename, int nFilesize )
{
	m_TableOfContents.Add( szFilename, nFilesize );
	
}

BOOL SceneMap::SaveArchive( char* szFilename )
{
FILE*	pFile;
char*	pcExtension = szFilename + ( strlen( szFilename ) - 3 );
char	acString[256];

	if ( stricmp( pcExtension, "uma" ) != 0 )
	{
		sprintf( acString, "%s.uma", szFilename );
	}
	else
	{
		strcpy( acString, szFilename );
	}
	pFile = fopen( acString, "wb" );
	if ( pFile != NULL )
	{
	MODEL_ARCHIVE_FILE_HEADER	xFileHeader;
	int		nNumFiles;
	FILE*	pUIMFile = fopen("uim.tmp", "wb" );
	int		nUIMSize;

		if ( pUIMFile )
		{
			m_TableOfContents.Reset();
			// Add the uim to the table of contents
			mFirstElement.mpChildElement->Save(pUIMFile);
			fclose( pUIMFile );
			pUIMFile = fopen( "uim.tmp", "rb" );
			nUIMSize = SysGetFileSize( pUIMFile );
			m_TableOfContents.Add( "00001.uim", nUIMSize );
			fclose( pUIMFile );

			// Get the TOC
			mFirstElement.mpChildElement->GetFileReferences(TableOfContentsAddFileReference);

			nNumFiles = m_TableOfContents.GetNumberOfFiles();

			// Write archive header
			EngineModelArchiveFillHeader( &xFileHeader, nNumFiles );
			fwrite( &xFileHeader, sizeof( xFileHeader ), 1, pFile );

			m_TableOfContents.Save( pFile );
			// Save out UIM to archive
			mFirstElement.mpChildElement->Save(pFile);
			// TODO - Save out each model and texture reference from the TOC
			m_TableOfContents.SaveFiles( pFile );
		}
		fclose( pFile );
		m_TableOfContents.Reset();
		return( TRUE );
	}
	return( FALSE );
}


#endif

void	SceneMap::SetCacheSubFolder( const char* szFolderName )
{ 
	strcpy( m_szCacheSubfolder, szFolderName );

}

void SceneMap::ScriptHTTPDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName )
{
	if ( nRet == FETCHFILE_SUCCESS )
	{
		if ( mpxCurrentScene )
		{
			if ( mpxCurrentScene->Load( mszLocalDownloadScriptFile ) != TRUE )
			{
				SysDebugPrint(0, "Failed to load map" );
			}
		}
	}
	else
	{
		SysDebugPrint( 0, "ERROR: Failed to download meshlandscape" );			
	}
}

void SceneMap::ScriptServerDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName )
{
	if ( nRet == FETCHFILE_SUCCESS )
	{
		if ( mpxCurrentScene )
		{
			mpxCurrentScene->Load( szName );
		}
	}
}


BOOL SceneMap::LoadFromServer( const char* szFilename )
{
//int		Len;
char*	pcRunner;

	mboIsDownloading = true;
	mboIsHTTPDownload = false;

	strcpy( mszLocalDownloadPath, szFilename );
	pcRunner = mszLocalDownloadPath + strlen( mszLocalDownloadPath );
	while ( ( pcRunner > mszLocalDownloadPath ) &&
			( *pcRunner != '/' ) &&
			( *pcRunner != '\\' ) )
	{
		pcRunner--;
	}

	if ( pcRunner > mszLocalDownloadPath )
	{
		*pcRunner = 0;
	}

	SceneMapGameInterface*		pSceneMapGameInterface = EngineGetSceneMapGameInterface();
	if ( pSceneMapGameInterface )
	{
		pSceneMapGameInterface->FetchSceneFile( szFilename,mszLocalDownloadScriptFile , NULL, ScriptServerDownloadCallback );
	}
	return( TRUE );
}

BOOL SceneMap::LoadFromWebsite( const char* szFilename )
{
int		Len;

#ifdef TOOL
	sprintf( mszLocalDownloadPath, "DataCache\\" );
	SysMkDir( mszLocalDownloadPath );
#else
	sprintf( mszLocalDownloadPath, "%s\\Data\\Scenes\\%s", SysGetWritableDataFolderPath( NULL ), m_szCacheSubfolder );
	SysMkDir( mszLocalDownloadPath );
	sprintf( mszLocalDownloadPath, "Data\\Scenes\\%s", m_szCacheSubfolder );
#endif

	mboIsDownloading = true;
	mboIsHTTPDownload = true;
	strcpy( m_szHTTPRoot, szFilename );
	Len = strlen(m_szHTTPRoot);
	while ( ( Len > 0 ) &&
			( m_szHTTPRoot[Len] != '/' ) &&
			( m_szHTTPRoot[Len] != '\\' ) )
	{
		Len--;
	}
	m_szHTTPRoot[Len] = 0;

	SceneMapGameInterface*		pSceneMapGameInterface = EngineGetSceneMapGameInterface();
	if ( pSceneMapGameInterface )
	{
		pSceneMapGameInterface->FetchSceneFile( szFilename, mszLocalDownloadScriptFile, m_szHTTPRoot, ScriptHTTPDownloadCallback );
	}
	return( TRUE );
}


void	SceneMap::LoaderPreParseNextLine( char* pcLine )
{
char*	pcVal;

	pcVal = SplitStringSep( pcLine, '=' );
	StripWhiteSpaceRight( pcVal );
	if ( stricmp( pcLine, "room" ) == 0 )
	{
		mnNumberOfRooms++;
	}
	else if ( stricmp( pcLine, "tex" ) == 0 )
	{
	}
}

BOOL SceneMap::PreParseLoadedScript( void )
{
char	acNextLine[512];
	while ( mpcScriptPos < mpcScriptEnd ) 
	{
		acNextLine[0] = 0;
		// Get the next line if we can
		mpcScriptPos = ScriptGetNextLine( mpcScriptPos, acNextLine, mpcScriptEnd );
		if ( acNextLine[0] != 0 )
		{
			if ( acNextLine[0] == '#' )
			{
				// ignore comment lines					
			}
			else
			{
				LoaderPreParseNextLine( acNextLine );
			}
		}
	}
	mpcScriptPos = mpcScriptMem;
	return( FALSE );
}

BOOL SceneMap::ParseLoadedScript( void )
{
BOOL	bWaitingForDownload = false;
char	acNextLine[512];

	msbLoadWarningAbsolutePaths = FALSE;

	while ( ( mpcScriptPos < mpcScriptEnd ) &&
			( bWaitingForDownload == false ) )
	{
		acNextLine[0] = 0;
		// Get the next line if we can
		mpcScriptPos = ScriptGetNextLine( mpcScriptPos, acNextLine, mpcScriptEnd );
		if ( acNextLine[0] != 0 )
		{
			if ( acNextLine[0] == '#' )
			{
				// ignore comment lines					
			}
			else
			{
				bWaitingForDownload = LoaderParseNextLine( acNextLine );
			}
		}
	}
#ifdef TOOL
	if ( msbLoadWarningAbsolutePaths )
	{
		SysDebugPrint( 0, "Warning : Absolute file paths in .uim file" );
	}
#endif
	if ( !bWaitingForDownload )
	{
		// Finish 
		if ( mpCurrentParseElement )
		{
			mpCurrentParseElement->OnParseComplete();
		}

		SystemFree( mpcScriptMem );	
		mboIsDownloading = false;

		if ( mpfnLoadCompleteCallback )
		{
			mpfnLoadCompleteCallback( 0 );
		}
		mpcScriptMem = NULL;
		mpcScriptPos = NULL;
		mpcScriptEnd = NULL;
		return( TRUE );
	}
	return( FALSE );
}



BOOL SceneMap::Load( const char* szFilename )
{
FILE*	pFile;
int		nFileSize;
char	acRepathed[256];

#ifdef TOOL
	strcpy( acRepathed, szFilename );
#else
	// check to see if its a full path with drive name for local testing
	if ( szFilename[1] == ':' )
	{
		strcpy( acRepathed, szFilename );
	}
	else
	{
		sprintf( acRepathed, "%s\\%s", SysGetWritableDataFolderPath( NULL ), szFilename );
	}
#endif
	pFile = fopen( acRepathed, "rb" );
	if ( pFile != NULL )
	{
	int		nLoop;
	char	acFilePath[256];

		nLoop = strlen( acRepathed );
		while ( ( nLoop > 0 ) &&
				( acRepathed[nLoop] != '\\' ) &&
				( acRepathed[nLoop] != '/' ) )
		{
			nLoop--;
		}
		strcpy( acFilePath, acRepathed );
		acFilePath[nLoop] = 0;
		strcpy( mszSceneMapLoadRootPath, acFilePath );
		nFileSize = SysGetFileSize( pFile );
		mpcScriptMem = (char*)( SystemMalloc( nFileSize + 1 ) );
		ZeroMemory( mpcScriptMem, nFileSize + 1 );
		fread( mpcScriptMem, nFileSize, 1, pFile );
		fclose( pFile );
		
		mpcScriptEnd = mpcScriptMem + nFileSize;

		mpcScriptPos = mpcScriptMem;
		PreParseLoadedScript();
		ParseLoadedScript();
		return( TRUE );
	}
	return( FALSE );
}

SceneMap::~SceneMap()
{
SceneMapElement*		pNextElement;
SceneMapElement*		pThisElement;
	
	pNextElement = mFirstElement.mpChildElement;
	
	while ( pNextElement )
	{
		pThisElement = pNextElement;
		pNextElement = pNextElement->mpBrotherElement;
		delete pThisElement;
	}
}

BOOL	SceneMap::Raycast( VECT* pxPos1, VECT* pxPos2, VECT* pxHit, VECT* pxNormal )
{
SceneMapElement*		pNextElement;

	// TODO - optimise which elements we check against	
	pNextElement = mFirstElement.mpChildElement;
	
	while ( pNextElement )
	{
		if ( pNextElement->Raycast(pxPos1, pxPos2, pxHit, pxNormal ) == TRUE )
		{
			return( TRUE );
		}
		pNextElement = pNextElement->mpBrotherElement;
	}
	return( FALSE );
}



SceneRegionElement*		SceneMap::FindRegion( const char* szRegionType, const char* szRegionParam )
{
	if ( !mboIsDownloading )
	{
		if ( mFirstElement.mpChildElement )
		{
		SceneRegionElement*		pFoundElement;
			pFoundElement = (SceneRegionElement*)mFirstElement.mpChildElement->FindRegion( szRegionType, szRegionParam );
			return( pFoundElement );
		}
	}
	return( NULL );
}

void SceneMap::Display( const VECT* pxPos, const VECT* pxRot, int nFlags )
{
	if ( mboIsDownloading )
	{
	char	acString[256];
	int		Y = InterfaceGetHeight() - 190;
//	int		nHeightOfBox;
//	int		nBarWidth;
	int		nLoop = 0;
	int		nHeightUsed = 0;
	int		nBytesInFile = 0;
	int		nBytesTransferred = 0;
	float	fTransferTime = 0.0f;

		InterfaceSetFontFlags(FONT_FLAG_DROP_SHADOW );
		if ( mnNumberOfRooms == 0 )
		{
			sprintf( acString, "Downloading Map Data" );
		}
		else
		{
			sprintf( acString, "Downloading Graphics %d of %d", mnNumberOfRoomsParsed, mnNumberOfRooms  );
		}
		InterfaceTextCenter( 2, 0, InterfaceGetWidth(), Y, acString, COL_F2_COMMS_SPECIAL, 1 );

#ifndef STANDALONE

// TODO - UniversalPub version needs this re-implementing through SceneMapGameInterface
	/*
		char	acFilename[256];
		// TODO - Display download progress
		ClientToWorldGetFileReceiveProgress( 0, &nBytesTransferred, &nBytesInFile, &fTransferTime, acFilename, 256 );
		if ( nBytesInFile != 0 )
		{
			sprintf( acString, "Downloading '%s'",acFilename );		
			InterfaceTextCenter( 2, 0, InterfaceGetWidth(), Y+30, acString, COL_SYS_SPECIAL, 0 );

			sprintf( acString, "%dk of %dk", nBytesTransferred/1024, nBytesInFile/1024 );		
			InterfaceTextCenter( 2, 0, InterfaceGetWidth(), Y+46, acString, COL_SYS_SPECIAL, 0 );
		}

  */
#endif
		InterfaceSetFontFlags(0 );
	}
	else
	{
		EngineDefaultState();
		if ( mFirstElement.mpChildElement )
		{
			mFirstElement.mpChildElement->Display( pxPos, pxRot, nFlags );
		}
	}
}

void		SceneMap::TriggerRegionEvent( eREGION_CALLBACK_TYPE nCallback, SceneRegionElement* pElement )
{
	if ( mpfnRegionCallback )
	{
		mpfnRegionCallback( nCallback, pElement );
	}
}


//------------------------------------------------------------
// C-Stylee access functions
//
void	SceneMapReset( void )
{
	if ( mpxCurrentScene )
	{
		// TODO - sort out destructor...
		delete mpxCurrentScene;
		mpxCurrentScene = NULL;
	}
}

void		SceneMapRegisterSceneExitCallback( fnSceneExitCallback pfnCallback )
{
	if ( mpxCurrentScene )
	{
		mpxCurrentScene->RegisterSceneExitCallback( pfnCallback );
	}
}


void		SceneMapRegisterLoadCompleteCallback( fnLoadCompleteCallback pfnCallback )
{
	if ( mpxCurrentScene )
	{
		mpxCurrentScene->RegisterLoadCompleteCallback( pfnCallback );
	}
}

void		SceneMapRegisterRegionCallback( fnRegionCallback pfnCallback )
{
	if ( mpxCurrentScene )
	{
		mpxCurrentScene->RegisterRegionCallback( pfnCallback );
	}
}


void	SceneMapInitNew( void )
{
	if ( !mpxCurrentScene )
	{
		mpxCurrentScene = new SceneMap;
	}
}


BOOL	SceneMapLoad( char* szFilename, fnAddItemCallback pAddCallback, const char* szCacheSubFolder )
{
#ifdef TOOL
	char	acString[512];
	sprintf( acString, "Loading Scene file: %s", szFilename );
	SysDebugPrint( 0, acString );
#endif

	m_fnAddItemCallback = pAddCallback;

	if ( strnicmp( szFilename, "http", 4 ) == 0 )
	{
		if ( !mpxCurrentScene )
		{
			mpxCurrentScene = new SceneMap;
		}
		mpxCurrentScene->SetCacheSubFolder( szCacheSubFolder );
		mpxCurrentScene->LoadFromWebsite( szFilename );
	}
	else
	{
	char*	pcExtension = szFilename + (strlen(szFilename) - 3);

		if ( stricmp( pcExtension, "uma" ) == 0 )
		{
			SceneMapInitNew();
			// Temp - this bit will need to extract the uim and models etc from the uma so it can
			// be edited.
			SceneMapAddElement( SceneMapElement::ROOM, szFilename, NULL, NULL );
		}
		else
		{
			if ( !mpxCurrentScene )
			{
				mpxCurrentScene = new SceneMap;
			}

			mpxCurrentScene->SetCacheSubFolder( szCacheSubFolder );
			if ( mpxCurrentScene->Load( szFilename ) == FALSE )
			{
#ifdef TOOL
				SysDebugPrint( 0, "** Load failed" );
#else
				mpxCurrentScene->LoadFromServer( szFilename );
#endif
				return( FALSE );
			}
		}
	}
	return( TRUE );
}

#ifdef TOOL
BOOL		SceneMapSaveArchive( char* szFilename )
{
	if ( mpxCurrentScene )
	{
		mpxCurrentScene->SaveArchive( szFilename );
		return( TRUE );
	}
	return( FALSE );
}


BOOL		SceneMapSave( char* szFilename )
{
	if ( mpxCurrentScene )
	{
		mpxCurrentScene->Save( szFilename );
		return( TRUE );
	}
	return( FALSE );
}
#endif

void	SceneMapDisplay( const VECT* pxPos, const VECT* pxRot, int nFlags )
{
	if ( mpxCurrentScene )
	{
		mpxCurrentScene->Display( pxPos, pxRot, nFlags );
	} 
}

void	SceneMapUpdate( const VECT* pxPlayerPos )
{
	if ( mpxCurrentScene )
	{
		mpxCurrentScene->Update(pxPlayerPos);
	}
}

BOOL		SceneMapHasFullyLoaded( void )
{
	if ( mpxCurrentScene )
	{
		return( mpxCurrentScene->HasFullyLoaded() );
	}
	return( TRUE );
}


SceneMapElement*		SceneMapGetBaseElement( void )
{
	if ( mpxCurrentScene )
	{
		return( mpxCurrentScene->GetFirstElement() );
	}
	return( NULL );
}

SceneMap*			SceneMapGet()
{
	return( mpxCurrentScene );
}

BOOL		SceneMapRaycast( VECT* pxPos1, VECT* pxPos2, VECT* pxHit, VECT* pxNormal )
{
	if ( mpxCurrentScene )
	{
		return( mpxCurrentScene->Raycast( pxPos1, pxPos2, pxHit, pxNormal ) );
	}
	return( FALSE );
}


SceneMapElement*		SceneMapAddElement( SceneMapElement::ELEMENT_TYPES eType, char* szModelName, char* szTextureName, SceneMapElement* pParent )
{
SceneMapElement*		pNewElement = NULL;

	if ( mpxCurrentScene )
	{
		if ( pParent )
		{
			pNewElement = mpxCurrentScene->AddNewElement( pParent, eType );
		}
		else
		{
			pNewElement = mpxCurrentScene->AddNewElement( mpxCurrentScene->GetRoot(), eType );
		}
		if ( pNewElement )
		{
			if ( szModelName )
			{
				pNewElement->SetModel( szModelName );
			}
			if ( szTextureName )
			{
				pNewElement->SetTexture( szTextureName );
			}
		}
	}
	return( pNewElement );
}

#endif // ndef SERVER