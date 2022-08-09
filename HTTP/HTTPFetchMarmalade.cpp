
#include "StandardDef.h"

#include "HTTP.h"
#include "HTTPFetch.h"

#include "IwHTTP.h"

enum HTTPStatus
{
    kNone,
    kDownloading,
    kOK,
    kError,
	kCancelled,
};

HTTPStatus		msHTTPStatus = kNone;
CIwHTTP*		mspHTTPObject = NULL;
BOOL			msbHTTPHeadResult = FALSE;


static int32 GotData(void* pSystemData , void* pUserData)
{
    // This is the callback indicating that a ReadDataAsync call has
    // completed.  Either we've finished, or a bigger buffer is
    // needed.  If the correct ammount of data was supplied initially,
    // then this will only be called once. However, it may well be
    // called several times when using chunked encoding.
    // Firstly see if there's an error condition.
    if (mspHTTPObject->GetStatus() == S3E_RESULT_ERROR)
    {
        // Something has gone wrong
        msHTTPStatus = kError;
    }
    else if (mspHTTPObject->ContentReceived() != mspHTTPObject->ContentLength())
    {
		// TODO ....!!
       msHTTPStatus = kError;

    }
    else
    {
	HTTPDownloadDetails*		pxDetails = (HTTPDownloadDetails*)pUserData;
//	int			nBytesRead;

		pxDetails->nTotalBytes = (int)mspHTTPObject->ContentReceived();
		pxDetails->nResponseCode = mspHTTPObject->GetResponseCode();
		msHTTPStatus = kOK;
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Called when the response headers have been received
//-----------------------------------------------------------------------------
static int32 HTTPPostCallback(void* pSystemData, void* pUserData)
{
    if (mspHTTPObject->GetStatus() == S3E_RESULT_ERROR)
    {
        // Something has gone wrong
        msHTTPStatus = kError;
    }
    else
    {
	HTTPDownloadDetails*		pxDetails = (HTTPDownloadDetails*)pUserData;
	uint32		lenExpected =  mspHTTPObject->ContentExpected();

		// pxDetails->nResponseBufferSize 
		mspHTTPObject->ReadDataAsync((char*)pxDetails->pbResponseBuffer, lenExpected, 0, GotData, pUserData);

	}
    return 0;
}

//-----------------------------------------------------------------------------
// Called when the response headers have been received
//-----------------------------------------------------------------------------
static int32 HTTPGetCallback(void* pSystemData, void* pUserData)
{
    if (mspHTTPObject->GetStatus() == S3E_RESULT_ERROR)
    {
        // Something has gone wrong
        msHTTPStatus = kError;
    }
    else
    {
	HTTPDownloadDetails*		pxDetails = (HTTPDownloadDetails*)pUserData;
	int			nBytesRead;

		nBytesRead = mspHTTPObject->ReadData( (char*)pxDetails->pbResponseBuffer, pxDetails->nResponseBufferSize );

		if ( nBytesRead >= pxDetails->nResponseBufferSize )
		{
			SysPanicIf( TRUE, "Full http recv buffer?" );
		}

		pxDetails->nTotalBytes = nBytesRead;
		pxDetails->nResponseCode = mspHTTPObject->GetResponseCode();
		msHTTPStatus = kOK;
	}
    return 0;
}


//----------------------------------------------------------------------------------
// HTTPFetchGlobalInit
// 
// Called from the main thread when the HTTP system is first initialised (HTTPInitialise)
//----------------------------------------------------------------------------------
void		HTTPFetchGlobalInit( void )
{
	// Nothing to do here atm
}

//----------------------------------------------------------------------------------
// HTTPFetchSystemInit
//
// This is called from the HTTP thread when it is initialised. It creates a CIwHTTP object
// which is used for all subsequent HTTP requests that are triggered on the thread
//----------------------------------------------------------------------------------
void 	HTTPFetchSystemInit( void )
{
	mspHTTPObject = new CIwHTTP;
}

//----------------------------------------------------------------------------------
// HTTPPerformFetch
//
// Called from the HTTP thread to perform an individual HTTP request
//----------------------------------------------------------------------------------
int		HTTPPerformFetch( char *acFullURL, HTTPDownloadDetails* pxDetails )
{
	pxDetails->nResponseCode = 0;

	if ( pxDetails->pcPostFields )
	{
	char	acBuff[256];
	s3eResult	result;

		msHTTPStatus = kDownloading;
		mspHTTPObject->SetRequestHeader( "Content-Type", "application/octet-stream" );
		sprintf( acBuff, "%d", pxDetails->nPostBodyLen );
		mspHTTPObject->SetRequestHeader( "Content-length", acBuff );
		result = mspHTTPObject->Post( acFullURL, pxDetails->pcPostFields, pxDetails->nPostBodyLen, HTTPPostCallback, pxDetails );

		while( msHTTPStatus == kDownloading )
		{
		s3eResult	result;

			if ( HTTPThreadShutdownRequested() == TRUE )
			{
				mspHTTPObject->Cancel();
				msHTTPStatus = kCancelled;
			}
			else
			{
				result = mspHTTPObject->GetStatus();

				SysSleep( 1 );
			}
		}

		if ( msHTTPStatus != kCancelled )
		{
			pxDetails->nResponseCode = mspHTTPObject->GetResponseCode();
		}
	}
	else
	{
		msHTTPStatus = kDownloading;
		mspHTTPObject->Get( acFullURL, HTTPGetCallback, pxDetails );

		while( msHTTPStatus == kDownloading )
		{
			if ( HTTPThreadShutdownRequested() == TRUE )
			{
				mspHTTPObject->Cancel();
				msHTTPStatus = kCancelled;
			}
			else
			{
				SysSleep( 1 );
			}
		}

	}
		
	return( pxDetails->nResponseCode );		
}

void		HTTPFetchSystemShutdown( void )
{
	SAFE_DELETE( mspHTTPObject );
}


void		HTTPFetchGlobalShutdown( void )
{
	// Nothing to do here
}

