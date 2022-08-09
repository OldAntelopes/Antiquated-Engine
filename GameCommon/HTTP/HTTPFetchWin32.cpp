
#include <curl/curl.h>

#include "StandardDef.h"

#include "../Console/Console.h"

#include "HTTP.h"
#include "HTTPFetch.h"

CURL*					mpxCurl;

//#define ENABLE_CURL_LOG	

int		HTTPDataReceive( const void* pMem, size_t size, size_t num, void* pData )
{
HTTPDownloadDetails*	pxDetails = (HTTPDownloadDetails*)( pData );
BYTE*		pbDataOut;

	if ( pxDetails->pLocalFile )
	{
		fwrite( pMem, size, num, (FILE*)pxDetails->pLocalFile );
	}
	else
	{
		// todo - check for buffer overrun...
		if ( pxDetails->nBytesReceived + (int)(size * num) > pxDetails->nResponseBufferSize )
		{
			// PANIC!??!?!?!
			return( 0 );
		}

		pbDataOut = pxDetails->pbResponseBuffer + pxDetails->nBytesReceived;
		memcpy( pbDataOut, pMem, size * num );
		pxDetails->nBytesReceived += size * num;
	
		pbDataOut = pxDetails->pbResponseBuffer + pxDetails->nBytesReceived;
		*pbDataOut = 0;
	}

	return( size * num );
}

FILE*		mpCurlOutputFile = NULL;

int progress_callback(void *clientp,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow)
{
//	ConsolePrint( 0, "dltotal: %d dlnow: %d\r\n", dltotal, dlnow );
	return( 0 );
}

int HTTPPerformFetch( char *acFullURL, HTTPDownloadDetails* pxDetails )
{
CURL*		pxCurl = mpxCurl;
CURLcode	nRes;
char		acErrorMsg[CURL_ERROR_SIZE];
double		dTotalBytes = 0;
double		dTransferedBytes = 0;
long		lRetCode = 0;

	acErrorMsg[0] = 0;
	pxDetails->nBytesReceived = 0;
	if ( pxDetails->pbResponseBuffer )
	{
		pxDetails->pbResponseBuffer[0] = 0;
	}
	else if ( pxDetails->acLocalFilename[0] != 0 )
	{
		pxDetails->pLocalFile = fopen( pxDetails->acLocalFilename, "wb" );
		if ( pxDetails->pLocalFile == NULL )
		{
			return( -2 );
		}
	}
	else
	{
		return( -1 );
	}
	curl_easy_setopt( pxCurl, CURLOPT_ERRORBUFFER, &acErrorMsg );
//	curl_easy_setopt( pxCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1 );
#ifdef ENABLE_CURL_LOG	
	curl_easy_setopt( pxCurl, CURLOPT_NOPROGRESS, 1);
	
	curl_easy_setopt( pxCurl, CURLOPT_XFERINFOFUNCTION, progress_callback );
//	curl_easy_setopt( pxCurl, CURLOPT_DEBUGFUNCTION
#else
	curl_easy_setopt( pxCurl, CURLOPT_NOPROGRESS, 1);
#endif
//	curl_easy_setopt( pxCurl, CURLOPT_VERBOSE, 1);
//	curl_easy_setopt( pxCurl, CURLOPT_MUTE, 1);
	curl_easy_setopt( pxCurl, CURLOPT_URL, acFullURL );
	curl_easy_setopt( pxCurl, CURLOPT_CONNECTTIMEOUT, 15 );
	curl_easy_setopt( pxCurl, CURLOPT_WRITEFUNCTION, HTTPDataReceive );
	curl_easy_setopt( pxCurl, CURLOPT_WRITEDATA, pxDetails );
	curl_easy_setopt( pxCurl, CURLOPT_TIMEOUT, 30 );

	struct curl_slist *headers=NULL;
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
	 /* pass our list of custom made headers */
	 curl_easy_setopt(pxCurl, CURLOPT_HTTPHEADER, headers);

	if ( pxDetails->pcPostFields )
	{
		curl_easy_setopt( pxCurl, CURLOPT_POST, 1);
		curl_easy_setopt( pxCurl, CURLOPT_POSTFIELDS, pxDetails->pcPostFields );
		curl_easy_setopt( pxCurl, CURLOPT_POSTFIELDSIZE, pxDetails->nPostBodyLen );
	}
	else
	{
		curl_easy_setopt( pxCurl, CURLOPT_POST, 0);
		curl_easy_setopt( pxCurl, CURLOPT_POSTFIELDS, NULL );
		curl_easy_setopt( pxCurl, CURLOPT_HTTPGET, 1);
	}

/*
	mfnDownloadCallbackFunc = fnDownloadCallbackFunc;
	if ( mfnDownloadCallbackFunc )
	{
		curl_easy_setopt( pxCurl, CURLOPT_PROGRESSFUNCTION, CurlProgressCallback );
		curl_easy_setopt( pxCurl, CURLOPT_PROGRESSDATA, lParam );
	}
*/
 
	curl_easy_setopt(pxCurl, CURLOPT_SSL_VERIFYPEER , 0);
	curl_easy_setopt(pxCurl, CURLOPT_SSL_VERIFYHOST , 1);
    /* Provide CA Certs from http://curl.haxx.se/docs/caextract.html */
    curl_easy_setopt(pxCurl, CURLOPT_CAINFO, "cacert.pem");

	nRes = curl_easy_perform( pxCurl );
	
	curl_slist_free_all(headers);

	if ( nRes == CURLE_URL_MALFORMAT )
	{
		return( -4 );
	}

	if ( pxDetails->pLocalFile )
	{
		fclose( (FILE*)pxDetails->pLocalFile );
	}

	if ( nRes == CURLE_SSL_CACERT )
	{
		return( -3 );
	}

	curl_easy_getinfo( pxCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dTotalBytes );
	curl_easy_getinfo( pxCurl, CURLINFO_SIZE_DOWNLOAD, &dTransferedBytes );
	curl_easy_getinfo( pxCurl, CURLINFO_RESPONSE_CODE, &lRetCode );

		pxDetails->nTotalBytes = (int)dTotalBytes;
	return( lRetCode );
}


void 	HTTPFetchSystemInit( void )
{
	mpxCurl = curl_easy_init();

#ifdef ENABLE_CURL_LOG	
	if ( mpCurlOutputFile == NULL )
	{
		mpCurlOutputFile = fopen( "curllog.txt", "w+" );
		curl_easy_setopt(mpxCurl, CURLOPT_STDERR, mpCurlOutputFile ); 
	}
#endif
}


void	HTTPFetchSystemShutdown( void )
{
#ifdef ENABLE_CURL_LOG	
	if ( mpCurlOutputFile != NULL )
	{
		curl_easy_setopt(mpxCurl, CURLOPT_STDERR, NULL ); 
		fclose( mpCurlOutputFile );
		mpCurlOutputFile = NULL;
	}
#endif

	curl_easy_cleanup(mpxCurl);
	mpxCurl = NULL;
}

void	HTTPFetchGlobalInit( void )
{
	// This will init the winsock stuff if needed
	curl_global_init( CURL_GLOBAL_ALL );
}

void	HTTPFetchGlobalShutdown( void ) 
{
	curl_global_cleanup();
}
