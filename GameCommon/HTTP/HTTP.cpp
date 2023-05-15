
#include <stdio.h>
#include "StandardDef.h"

#include "HTTP.h"
#include "HTTPFetch.h"

#define		MAX_HTTP_DOWNLOADS		16

enum
{
	HTTP_DOWNLOAD_STATE_NONE = 0,
	HTTP_DOWNLOAD_STATE_QUEUED,
	HTTP_DOWNLOAD_STATE_IN_PROGRESS,
	HTTP_DOWNLOAD_COMPLETED,
};

//#define HTTP_POST_LOGGING

HTTPDownloadDetails		maHTTPDownloadDetails[MAX_HTTP_DOWNLOADS];		// Ahh, the old days
unsigned int			mhHTTPDownloadThread = NOTFOUND;
volatile BOOL			mbHTTPDownloadJobPending = FALSE;
HTTPDownloadDetails*	mpxHTTPDownloadNextJob = NULL;
BOOL					mbHTTPDownloadKillThread = FALSE;
BOOL					mbHTTPInitialised = FALSE;
int						mnHTTPNumDownloadsInProgress = 0;

BOOL		HTTPThreadShutdownRequested( void )
{
	return( mbHTTPDownloadKillThread );
}

HTTPDownloadDetails*	HTTPDownloadGetNewDetails( void )
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

#ifdef HTTP_POST_LOGGING
void	HTTPLog( const char* szURL, const char* szBody, int nBodyLen )
{
FILE*		pFile = fopen( "HTTPPost.log", "ab" );
char		acBuff[4096];

	if ( pFile )
	{
		sprintf( acBuff, "URL: %s\r\n", szURL );
		fwrite( acBuff, strlen( acBuff ), 1, pFile );
		if ( szBody )
		{
		int		nLoop;
		const char*	pcRunner;
		char*	pcOutRunner;

			sprintf( acBuff, "Body: " );
			pcOutRunner = acBuff + 6;
			pcRunner = szBody;
			for ( nLoop = 0; nLoop < nBodyLen; nLoop++ )
			{
				sprintf( pcOutRunner, "%02x,", (BYTE)*pcRunner );
				pcRunner++;
				pcOutRunner += 3;
			}

		}
		strcat( acBuff, "\r\n-----------------------------------------------------------\r\n" );
		fwrite( acBuff, strlen( acBuff ), 1, pFile );

		fclose( pFile );
	}

}
#endif


long WINAPI HTTPCommonDownloadNewThread(long lParam)
{ 
HTTPDownloadDetails*	pxDetails;
int			nRet;

	HTTPFetchSystemInit();

	while( mbHTTPDownloadKillThread == FALSE )
	{
		if ( mbHTTPDownloadJobPending == TRUE )
		{
			pxDetails = mpxHTTPDownloadNextJob;
			pxDetails->nState = HTTP_DOWNLOAD_STATE_IN_PROGRESS;
			mpxHTTPDownloadNextJob = NULL;

#ifdef HTTP_POST_LOGGING
			HTTPLog( pxDetails->pcFullURL, pxDetails->pcPostFields, pxDetails->nPostBodyLen );
#endif
			mbHTTPDownloadJobPending = FALSE;
			nRet = HTTPPerformFetch( pxDetails->pcFullURL, pxDetails );
			pxDetails->nResponseCode = nRet;
			pxDetails->nState = HTTP_DOWNLOAD_COMPLETED;
		}
		else
		{
			SysSleep(2);
		}
	}

	HTTPFetchSystemShutdown();
	SysExitThread( 0 );
	return( 0 );
}


int		HTTPPostEx( const char *acFullURL, BYTE* pbBody, int nBodyLen, HTTPResponseHandler fnResponseHandler, void* vpParam, int nTimeoutSecs )
{
	if ( mbHTTPInitialised )
	{
	HTTPDownloadDetails*	pxNewDetails;
	
		pxNewDetails = HTTPDownloadGetNewDetails();

		if ( pxNewDetails )
		{
			pxNewDetails->pcFullURL = (char*)SystemMalloc( strlen( acFullURL ) + 1 );
			strcpy( pxNewDetails->pcFullURL, acFullURL );
			pxNewDetails->nState = HTTP_DOWNLOAD_STATE_QUEUED;
			mnHTTPNumDownloadsInProgress++;
			pxNewDetails->fnResponseCallback = fnResponseHandler;
	//		pxNewDetails->fnUpdateCallback = fnUpdateCallback;
			pxNewDetails->vpCBParam = vpParam;
			pxNewDetails->nPartialDownloadStart = 0;
			pxNewDetails->nPartialDownloadEnd = 0;
			pxNewDetails->pbResponseBuffer = (BYTE*)( SystemMalloc( 16384 ) );
			pxNewDetails->nResponseBufferSize = 16384;
			pxNewDetails->acLocalFilename[0] = 0;
			pxNewDetails->nPostBodyLen = nBodyLen;

			pxNewDetails->pcPostFields = (char*)( SystemMalloc( nBodyLen + 1 ) );
			memcpy( pxNewDetails->pcPostFields, pbBody, nBodyLen );
			pxNewDetails->pcPostFields[nBodyLen] = 0;
		}
		else
		{
			// TODO Will need to fail and callback elegantly here..?
			// (Tho this shouldnt really ever happen anyway..)
			SysUserPrint( 0, "ERROR: HTTP Post failed - couldn't get new details" );
		}
	}
	else
	{
		SysUserPrint( 0, "ERROR: HTTP Post failed - system not initialised" );
	}
	return( 1 );

}


int		HTTPPost( const char *acFullURL, BYTE* pbBody, int nBodyLen, HTTPResponseHandler fnResponseHandler, void* vpParam )
{
	return( HTTPPostEx( acFullURL, pbBody, nBodyLen, fnResponseHandler, vpParam, 0 ) ); 
}

int		HTTPGetFile( const char *acFullURL, const char* szLocalFilename, HTTPResponseHandler fnResponseHandler, void* vpParam )
{
	if ( mbHTTPInitialised )
	{
	HTTPDownloadDetails*	pxNewDetails;

		pxNewDetails = HTTPDownloadGetNewDetails();

		if ( pxNewDetails )
		{
			pxNewDetails->pcFullURL = (char*)SystemMalloc( strlen( acFullURL ) + 1 );
			strcpy( pxNewDetails->pcFullURL, acFullURL );
			pxNewDetails->nState = HTTP_DOWNLOAD_STATE_QUEUED;
			mnHTTPNumDownloadsInProgress++;
			pxNewDetails->fnResponseCallback = fnResponseHandler;
	//		pxNewDetails->fnUpdateCallback = fnUpdateCallback;
			pxNewDetails->vpCBParam = vpParam;
			pxNewDetails->nPartialDownloadStart = 0;
			pxNewDetails->nPartialDownloadEnd = 0;
			pxNewDetails->pbResponseBuffer = NULL;
			pxNewDetails->nResponseBufferSize = 0;
			strcpy( pxNewDetails->acLocalFilename, szLocalFilename );
			pxNewDetails->pcPostFields = NULL;	
		}
		else
		{
			// TODO Will need to fail and callback elegantly here.. ?
			// (Tho this shouldnt really ever happen anyway..)
			SysUserPrint( 0, "ERROR: HTTP GET failed - couldn't get new details" );
		}
	}
	else
	{
		SysUserPrint( 0, "ERROR: HTTP GET failed - system not initialised" );
	}

	return( 1 );

}

int		HTTPGet( const char *acFullURL, HTTPResponseHandler fnResponseHandler, void* vpParam )
{
	if ( mbHTTPInitialised )
	{
	HTTPDownloadDetails*	pxNewDetails;

		pxNewDetails = HTTPDownloadGetNewDetails();

		if ( pxNewDetails )
		{
			pxNewDetails->pcFullURL = (char*)SystemMalloc( strlen( acFullURL ) + 1 );
			strcpy( pxNewDetails->pcFullURL, acFullURL );
	//		strcpy( pxNewDetails->acLocalFilename, szLocalFilename );

			pxNewDetails->nState = HTTP_DOWNLOAD_STATE_QUEUED;
			mnHTTPNumDownloadsInProgress++;
			pxNewDetails->fnResponseCallback = fnResponseHandler;
	//		pxNewDetails->fnUpdateCallback = fnUpdateCallback;
			pxNewDetails->vpCBParam = vpParam;
			pxNewDetails->nPartialDownloadStart = 0;
			pxNewDetails->nPartialDownloadEnd = 0;
			pxNewDetails->pbResponseBuffer = (BYTE*)( SystemMalloc( 16384 ) );
			pxNewDetails->nResponseBufferSize = 16384;
			pxNewDetails->acLocalFilename[0] = 0;
			pxNewDetails->pcPostFields = NULL;
		
	//		pxNewDetails->bIgnoreCache = bIgnoreCache;

	//		HTTPValidateString( pxNewDetails->acFullURL );
	//		hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)HTTPDownloadNewThread,(LPVOID)(pxNewDetails),0,&iID);
		}
		else
		{
			// TODO Will need to fail and callback elegantly here..
			// (Tho this shouldnt really ever happen anyway..)
			SysUserPrint( 0, "ERROR: HTTP GET failed - couldn't get new details" );
		}
	}
	else
	{
		SysUserPrint( 0, "ERROR: HTTP GET failed - system not initialised" );
	}

	return( 1 );

}


void	HTTPResetDownloadDetails( HTTPDownloadDetails*	pxDownloadDetails )
{
	SAFE_FREE( pxDownloadDetails->pbResponseBuffer );
	SAFE_FREE( pxDownloadDetails->pcFullURL );
	SAFE_FREE( pxDownloadDetails->pcPostFields );

	memset( pxDownloadDetails, 0, sizeof( HTTPDownloadDetails ) );
	pxDownloadDetails->nState = HTTP_DOWNLOAD_STATE_NONE;

}

//------------------------------------------------------------------------------------
//-  HTTPUpdate ---------- Must be called regularly to receive callbacks
//   (When a download gets to HTTP_DOWNLOAD_COMPLETED state, this frees up allocated data within the details
//  and set back to HTTP_DOWNLOAD_STATE_NONE
//----------------------------------------------------------------
void		HTTPUpdate( float fDelta )
{
	if ( mnHTTPNumDownloadsInProgress > 0 )
	{
	HTTPDownloadDetails*	pxDownloadDetails;
	int		nLoop;

		for( nLoop = 0; nLoop < MAX_HTTP_DOWNLOADS; nLoop++ )
		{
			pxDownloadDetails = maHTTPDownloadDetails + nLoop;

			switch( pxDownloadDetails->nState )
			{
			case HTTP_DOWNLOAD_STATE_QUEUED:
				if ( mbHTTPDownloadJobPending == FALSE )
				{
					mpxHTTPDownloadNextJob = pxDownloadDetails;
					mbHTTPDownloadJobPending = TRUE;
				}
				break;
			case HTTP_DOWNLOAD_COMPLETED:
				if ( pxDownloadDetails->fnResponseCallback )
				{
					pxDownloadDetails->fnResponseCallback( pxDownloadDetails->nResponseCode, pxDownloadDetails->pbResponseBuffer, pxDownloadDetails->nTotalBytes, pxDownloadDetails->vpCBParam );
				}
			
				HTTPResetDownloadDetails( pxDownloadDetails );
				mnHTTPNumDownloadsInProgress--;
				break;
			default:
				break;
			}
		}
	}
}

//------------------------------------------------------------------------------------
void		HTTPInitialise( void )
{
	if ( mbHTTPInitialised == FALSE )
	{
	int		nLoop;

		for ( nLoop = 0; nLoop < MAX_HTTP_DOWNLOADS; nLoop++ )
		{
			memset( &maHTTPDownloadDetails[nLoop], 0, sizeof( HTTPDownloadDetails ) );
		}

		HTTPFetchGlobalInit();

		mhHTTPDownloadThread = SysCreateThread( (fnThreadFunction)HTTPCommonDownloadNewThread, NULL, 0, 0 );
		mbHTTPInitialised = TRUE;
	}
}



void		HTTPShutdown( void )
{
int		nLoop;

	mbHTTPDownloadKillThread = TRUE;
	SysSleep(5);

	HTTPFetchGlobalShutdown();

	for ( nLoop = 0; nLoop < MAX_HTTP_DOWNLOADS; nLoop++ )
	{
		HTTPResetDownloadDetails( &maHTTPDownloadDetails[nLoop] );
	}
	mbHTTPInitialised = FALSE;
}


