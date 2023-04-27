
#include <stdio.h>

#include <curl/curl.h>
#include <StandardDef.h>
#include <Interface.h>
#include <System.h>

#include "HTTP.h"
#include "HTTPDownload.h"


#define		MAX_HTTP_DOWNLOADS		16

enum
{
	HTTP_DOWNLOAD_STATE_NONE = 0,
	HTTP_DOWNLOAD_STATE_IN_PROGRESS,
	HTTP_DOWNLOAD_COMPLETED,
};

typedef struct
{
	fnFetchFileCallback		fnCompleteCallback;
	HTTPDownloadUpdateCallback			fnUpdateCallback;
	char		acLocalFilename[256];
	char		acFullURL[256];
	long		lCBParam;
	int			nType;
	int			nState;
	FETCHFILE_RETURN_VAL			nRetVal;
	int			nBytesReceived;
	int			nTotalBytes;
	int			nPathAttempt;
	BOOL		bCancelRequested;

	int			nPartialDownloadStart;
	int			nPartialDownloadEnd;
	BOOL		bIgnoreCache;

} HTTP_DOWNLOAD_DETAILS;

HTTP_DOWNLOAD_DETAILS		maHTTPDownloadDetails[MAX_HTTP_DOWNLOADS];
BOOL	mboShowHTTPRequestMessages = FALSE;

void	HTTPValidateString( char* szString )
{
int		nLoop = 0;

	while ( szString[nLoop] != 0 )
	{
		if ( szString[nLoop] == '\\' )
		{
			szString[nLoop] = '/';
		}
		nLoop++;
	}
}



BOOL HTTPDownloadCallback( long lBytesTransferred, long lTotalBytes , long lUserParam )
{
HTTP_DOWNLOAD_DETAILS*  pxDetails;
		
	pxDetails = (HTTP_DOWNLOAD_DETAILS*)( lUserParam );
	pxDetails->nTotalBytes = lTotalBytes;
	pxDetails->nBytesReceived = lBytesTransferred;
	//No longer needed with new method ?
	/*
	if ( unRecvd > 0 )
	{
	FILE*	pFile;
	
		pFile = fopen(pxDetails->acLocalFilename, "ab" );
		fwrite( pbData, unRecvd, 1, pFile );
		fclose( pFile );
	}
	
	*/
	// TODO - if requested, cancel download by returning false
	if ( pxDetails->bCancelRequested == TRUE )
	{
		return( FALSE );
	}
	return( TRUE );
}


void	HTTPDownloadFileTransferRequest( HTTP_DOWNLOAD_DETAILS*  pxDetails )
{
int		nRet;
#ifndef TOOL
char	acString[256];
#endif

#ifndef TOOL
	if ( mboShowHTTPRequestMessages )
	{ 
		sprintf( acString, "Requesting %s", pxDetails->acFullURL );
		NetworkingUserDebugPrint( 1, acString );
	}
#endif
	
	//new method , using curl (note that, despite of the name, it's not restricted to http !)
	nRet = HTTPGetFile( pxDetails->acFullURL, pxDetails->acLocalFilename, HTTPDownloadCallback, (long)pxDetails, pxDetails->bIgnoreCache );
	switch ( nRet )
	{
	case CURLE_OK:	//0
		pxDetails->nRetVal = FETCHFILE_SUCCESS;
#ifndef TOOL
		if ( mboShowHTTPRequestMessages )
		{
			sprintf( acString, "%s completed", pxDetails->acFullURL );
			NetworkingUserDebugPrint( 1, acString );
		}
#endif
		break;
	case CURLE_HTTP_NOT_FOUND:
		pxDetails->nRetVal = FETCHFILE_NOT_FOUND;
#ifndef TOOL
		if ( mboShowHTTPRequestMessages )
		{
			sprintf( acString, "%s not found", pxDetails->acFullURL );
			NetworkingUserDebugPrint( 0, acString );
		}
#endif
		break;
	default:
		pxDetails->nRetVal = FETCHFILE_FAILED;
#ifndef TOOL
		if ( mboShowHTTPRequestMessages )
		{
			sprintf( acString, "%s errored", pxDetails->acFullURL );
			NetworkingUserDebugPrint( 0, acString );
		}
#endif
		break;	
	}

	pxDetails->nState = HTTP_DOWNLOAD_COMPLETED;
}


long WINAPI HTTPDownloadNewThread(long lParam)
{ 
HTTP_DOWNLOAD_DETAILS*	pxDetails;

	pxDetails = (HTTP_DOWNLOAD_DETAILS*)( lParam );
	HTTPDownloadFileTransferRequest( pxDetails );
	return( 0 );
}


HTTP_DOWNLOAD_DETAILS*	HTTPDownloadGetNewDetails( void )
{
int		nLoop;
	for ( nLoop = 0; nLoop < MAX_HTTP_DOWNLOADS; nLoop++ )
	{
		if ( maHTTPDownloadDetails[nLoop].nState == HTTP_DOWNLOAD_STATE_NONE )
		{
			return( maHTTPDownloadDetails + nLoop );
		}
	}
	return( NULL );
}

void	HTTPDownloadsCallbackAndFinish( HTTP_DOWNLOAD_DETAILS* pxDetails )
{
	if ( pxDetails->fnCompleteCallback )
	{
		pxDetails->fnCompleteCallback( pxDetails->nRetVal, pxDetails->lCBParam, pxDetails->acLocalFilename );
	}
	ZeroMemory( pxDetails, sizeof( HTTP_DOWNLOAD_DETAILS ) );
	pxDetails->nState = HTTP_DOWNLOAD_STATE_NONE;
}

void	HTTPReplaceSpaces( char* pcIn, char* pcOut )
{
	while ( *pcIn != 0 )
	{
		if ( *pcIn == ' ' )
		{
			*pcOut++ = '%';
			*pcOut++ = '2';
			*pcOut++ = '0';
			pcIn++;
		}
		else
		{
			*pcOut++ = *pcIn++;
		}
	}
	*pcOut = 0;
}

char*	mszAssetsURL = "http://gameislands.net/gamecontent/modellib/Assets/";

char	maszContentPathRoots[8][256] = 
{
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	""
};

void	HTTPDownloadSetContentPath( int nIndex, const char* szContentPathRoot )
{
	strcpy( maszContentPathRoots[nIndex], szContentPathRoot );
}


void	HTTPDownloadTryNextPath( HTTP_DOWNLOAD_DETAILS* pxDetails )
{
#ifndef TOOL
uint32 iID;
HANDLE hThread;
char*	pcDownloadURLRoot = NULL;
char	acFullURL[256];
BOOL	bIsAssetFile = FALSE;

	pxDetails->nPathAttempt++;

	switch( pxDetails->nPathAttempt )
	{
	case 1:
		if ( strnicmp( pxDetails->acLocalFilename, "Data\\Assets", 11 ) == 0 )
		{
			pcDownloadURLRoot = mszAssetsURL;
			bIsAssetFile = TRUE;
		}
		else
		{
			pcDownloadURLRoot = maszContentPathRoots[0];
		}
		break;
	case 2:
		pcDownloadURLRoot = maszContentPathRoots[1];
		break;
	case 3:
		pcDownloadURLRoot = maszContentPathRoots[2];
		break;
	case 4:
		pcDownloadURLRoot = maszContentPathRoots[3];
		break;
	}

	if ( ( pcDownloadURLRoot != NULL ) &&
		 ( *pcDownloadURLRoot != 0 ) )
	{
	char	acString[256];
	
		if ( bIsAssetFile )
		{
			// Asset filename is Data\\Assets\\04\etc.atm
			// Need to remove the Data\\Assets bit when generating the url..
			sprintf( acString, "%s/%s", pcDownloadURLRoot, pxDetails->acLocalFilename + 12 );
		}
		else
		{
			sprintf( acString, "%s/%s", pcDownloadURLRoot, pxDetails->acLocalFilename );
		}

		HTTPReplaceSpaces( acString, acFullURL );
		strcpy( pxDetails->acFullURL, acFullURL );
		pxDetails->nState = HTTP_DOWNLOAD_STATE_IN_PROGRESS;
		HTTPValidateString( pxDetails->acFullURL );
		hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)HTTPDownloadNewThread,(LPVOID)(pxDetails),0,&iID);
	}
	else	// No more paths available.. give up..
	{	
		HTTPDownloadsCallbackAndFinish( pxDetails );
	}
#endif // ndef TOOL
}


//-------------------------------------------------------------------------------------------------

void	HTTPDownloadGetFile( int nType, const char* szLocalFile, fnFetchFileCallback fnCompleteCallback, long lCBParam )
{
HTTP_DOWNLOAD_DETAILS*	pxNewDetails;

	pxNewDetails = HTTPDownloadGetNewDetails();

	if ( pxNewDetails )
	{
		strcpy( pxNewDetails->acLocalFilename, szLocalFile );
		pxNewDetails->nType = nType;
		pxNewDetails->nState = HTTP_DOWNLOAD_STATE_IN_PROGRESS;
		pxNewDetails->fnCompleteCallback = fnCompleteCallback;
		pxNewDetails->lCBParam = lCBParam;
		pxNewDetails->nPathAttempt = 0;
		pxNewDetails->nPartialDownloadStart = 0;
		pxNewDetails->nPartialDownloadEnd = 0;
		pxNewDetails->bIgnoreCache = FALSE;
		HTTPDownloadTryNextPath( pxNewDetails );
	}
}


void	HTTPDownloadInitiate( int nType, const char* szFullURL, const char* szLocalFilename, fnFetchFileCallback fnCompleteCallback,HTTPDownloadUpdateCallback fnUpdateCallback, long lCBParam, BOOL bIgnoreCache )
{
uint32 iID;
HANDLE hThread;
HTTP_DOWNLOAD_DETAILS*	pxNewDetails;

	pxNewDetails = HTTPDownloadGetNewDetails();

	if ( pxNewDetails )
	{
		strcpy( pxNewDetails->acFullURL, szFullURL );
		strcpy( pxNewDetails->acLocalFilename, szLocalFilename );

		pxNewDetails->nType = nType;
		pxNewDetails->nState = HTTP_DOWNLOAD_STATE_IN_PROGRESS;
		pxNewDetails->fnCompleteCallback = fnCompleteCallback;
		pxNewDetails->fnUpdateCallback = fnUpdateCallback;
		pxNewDetails->lCBParam = lCBParam;
		pxNewDetails->nPathAttempt = 0;
		pxNewDetails->nPartialDownloadStart = 0;
		pxNewDetails->nPartialDownloadEnd = 0;
		pxNewDetails->bIgnoreCache = bIgnoreCache;

		HTTPValidateString( pxNewDetails->acFullURL );
		hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)HTTPDownloadNewThread,(LPVOID)(pxNewDetails),0,&iID);
	}
	else
	{
		// TODO Will need to fail and callback elegantly here..
		// (Tho this shouldnt really ever happen anyway..)
	}
}

void	HTTPDownloadInitiatePartial( int nType, char* szFullURL, char* szLocalFilename, fnFetchFileCallback fnCompleteCallback,HTTPDownloadUpdateCallback fnUpdateCallback, long lCBParam, int nBytesFromStart, int nBytesToRead )
{
uint32 iID;
HANDLE hThread;
HTTP_DOWNLOAD_DETAILS*	pxNewDetails;

	// TODO --- This is yet to be implemented..
	//   (but can be done using the CURLOPT_RANGE to allow us to start radio downloads in the middle of 
	//	  a broadcast, for instance)
	pxNewDetails = HTTPDownloadGetNewDetails();

	if ( pxNewDetails )
	{
		strcpy( pxNewDetails->acFullURL, szFullURL );
		strcpy( pxNewDetails->acLocalFilename, szLocalFilename );
		pxNewDetails->nType = nType;
		pxNewDetails->nState = HTTP_DOWNLOAD_STATE_IN_PROGRESS;
		pxNewDetails->fnCompleteCallback = fnCompleteCallback;
		pxNewDetails->fnUpdateCallback = fnUpdateCallback;
		pxNewDetails->lCBParam = lCBParam;
		pxNewDetails->nPathAttempt = 0;
		pxNewDetails->nPartialDownloadStart = nBytesFromStart;
		pxNewDetails->nPartialDownloadEnd = nBytesFromStart + nBytesToRead;
		pxNewDetails->bIgnoreCache = TRUE;

		HTTPValidateString( pxNewDetails->acFullURL );
		hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)HTTPDownloadNewThread,(LPVOID)(pxNewDetails),0,&iID);
	}
	else
	{
		// TODO Will need to fail and callback elegantly here..
		// (Tho this shouldnt really ever happen anyway..)
	}
}

//---------------------------------------------------------------------------------------------------------


char* HTTPDownloadsGetActiveDetails( int* pnEntry, int* pnBytesRecvd, int* pnTotalBytes )
{
HTTP_DOWNLOAD_DETAILS*	pxDetails;
int		nLoop;

	for ( nLoop = *pnEntry; nLoop < MAX_HTTP_DOWNLOADS; nLoop++ )
	{
		pxDetails = &maHTTPDownloadDetails[ nLoop ];
		if ( pxDetails->nState != HTTP_DOWNLOAD_STATE_NONE )
		{
			*pnBytesRecvd = pxDetails->nBytesReceived;
			*pnTotalBytes = pxDetails->nTotalBytes;
			*pnEntry = nLoop + 1;
			return( pxDetails->acFullURL );
		}
	}
	return( NULL );
}

void HTTPDownloadsInit( void )
{
	ZeroMemory( maHTTPDownloadDetails, sizeof( HTTP_DOWNLOAD_DETAILS ) * MAX_HTTP_DOWNLOADS );
}

int		mnLastTick = 0;

void	HTTPDownloadsUpdate( void )
{
int		nLoop;
HTTP_DOWNLOAD_DETAILS*	pxDetails = maHTTPDownloadDetails;
BOOL	bShouldContinue;
int		sysTick = SysGetTick();

	mnLastTick = sysTick;

	for ( nLoop = 0; nLoop < MAX_HTTP_DOWNLOADS; nLoop++ )
	{
		if ( pxDetails->nState != HTTP_DOWNLOAD_STATE_NONE )
		{
			if ( pxDetails->nState == HTTP_DOWNLOAD_COMPLETED )
			{
				// If the download failed and we're using the method where
				// various URL paths are checked
				if ( ( pxDetails->nRetVal != FETCHFILE_SUCCESS ) &&
					 ( pxDetails->nPathAttempt != 0 ) )
				{
					HTTPDownloadTryNextPath( pxDetails );
				}
				else
				{
					HTTPDownloadsCallbackAndFinish( pxDetails );
				}
			}
			else if ( pxDetails->fnUpdateCallback != NULL )
			{
				bShouldContinue = pxDetails->fnUpdateCallback( pxDetails->nBytesReceived, pxDetails->nTotalBytes, 0, pxDetails->lCBParam, pxDetails->acLocalFilename );
				pxDetails->bCancelRequested = !bShouldContinue;
			}
		}
		pxDetails++;
	}
}
