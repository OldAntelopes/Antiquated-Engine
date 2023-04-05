
#include <curl/curl.h>

#include "StandardDef.h"
#include "HTTP.h"
#include "HTTPFetch.h"

CURL*					mpxCurl;
HTTPLoggingFunction		mfnVerboseLogOutputFunction = NULL;


void		HTTPSetVerboseLoggingFunction( HTTPLoggingFunction fnLogOutputFunction )
{
	mfnVerboseLogOutputFunction = fnLogOutputFunction;
}


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


static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size,   void *userp)
{
//  struct data *config = (struct data *)userp;
 const char *text;
 
  (void)handle; /* prevent compiler warning */
 
  switch(type) 
  {
  case CURLINFO_TEXT:
	  {
	  char		acBuff[512];
		strcpy( acBuff, "== Info: " );
		strcat( acBuff, data );
		mfnVerboseLogOutputFunction( acBuff );
	  }
    /* FALLTHROUGH */
  default: /* in case a new one is introduced to shock us */
    return 0;
 
  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }
 
	mfnVerboseLogOutputFunction( text );
 return 0;
}

int HTTPPerformFetch( char *acFullURL, HTTPDownloadDetails* pxDetails )
{
CURL*		pxCurl = mpxCurl;
CURLcode	nRes;
char		acErrorMsg[CURL_ERROR_SIZE];
double		dTotalBytes = 0;
double		dTransferedBytes = 0;
long		lRetCode = 0;
int			nNumRetries = 0;
bool		bRetry = false;

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

	if ( mfnVerboseLogOutputFunction )
	{
		curl_easy_setopt(pxCurl, CURLOPT_DEBUGFUNCTION, my_trace);
		curl_easy_setopt(pxCurl, CURLOPT_VERBOSE, 1);
	}

	curl_easy_setopt( pxCurl, CURLOPT_ERRORBUFFER, &acErrorMsg );
	curl_easy_setopt( pxCurl, CURLOPT_NOPROGRESS, 1);
	//	curl_easy_setopt( pxCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1 );
	//	curl_easy_setopt( pxCurl, CURLOPT_MUTE, 1);
	curl_easy_setopt( pxCurl, CURLOPT_URL, acFullURL );
	curl_easy_setopt( pxCurl, CURLOPT_CONNECTTIMEOUT, 15 );
	curl_easy_setopt( pxCurl, CURLOPT_WRITEFUNCTION, HTTPDataReceive );
	curl_easy_setopt( pxCurl, CURLOPT_WRITEDATA, pxDetails );
	curl_easy_setopt( pxCurl, CURLOPT_TIMEOUT, 30 );
	curl_easy_setopt( pxCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
	curl_easy_setopt( pxCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	struct curl_slist *headers=NULL;
	//	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
	// CHanging the above to the below prevented 406 errors on comms out from EC2 instances. This needs investigating further and not just random guessing at :) Bet Adrian will know ;)
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Accept: */*");
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

 	curl_easy_setopt(pxCurl, CURLOPT_SSL_VERIFYPEER , 0);
	curl_easy_setopt(pxCurl, CURLOPT_SSL_VERIFYHOST , 1);
		//   Provide CA Certs from http://curl.haxx.se/docs/caextract.html
	//    curl_easy_setopt(pxCurl, CURLOPT_CAINFO, "cacert.pem");
	// Switched to using the windows ones:
    curl_easy_setopt(pxCurl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);

	do
	{
		bRetry = false;
		nRes = curl_easy_perform( pxCurl );
	
		switch( nRes )
		{
		case CURLE_URL_MALFORMAT:
			lRetCode = -1;
			break;
		case CURLE_SSL_CACERT:
			lRetCode = -3;
			break;
		case CURLE_SEND_ERROR:
			lRetCode = nRes;			
			bRetry = true;
			break;
		case 0:
			curl_easy_getinfo( pxCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dTotalBytes );
			curl_easy_getinfo( pxCurl, CURLINFO_SIZE_DOWNLOAD, &dTransferedBytes );
			curl_easy_getinfo( pxCurl, CURLINFO_RESPONSE_CODE, &lRetCode );
			pxDetails->nTotalBytes = (int)dTotalBytes;
			break;
		default:
			lRetCode = nRes;
			break;
		}
		
		if ( bRetry )
		{
			SysSleep(10);
			if ( mfnVerboseLogOutputFunction )
			{
			char	acBuff[256];
				sprintf( acBuff, "*** Retrying (%d attempt)", nNumRetries );
				mfnVerboseLogOutputFunction( acBuff );
			}
			nNumRetries++;
		}

	} while( ( bRetry ) &&
		     ( nNumRetries < 5 ) );

	curl_slist_free_all(headers);

	if ( pxDetails->pLocalFile )
	{
		fclose( (FILE*)pxDetails->pLocalFile );
	}
	return( lRetCode );

}


void 	HTTPFetchSystemInit( void )
{
	mpxCurl = curl_easy_init();
}


void	HTTPFetchSystemShutdown( void )
{
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
