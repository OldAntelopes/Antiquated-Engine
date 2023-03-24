
#include <stdio.h>
#include "StandardDef.h"

#include "BasicWebServer.h"
#include "WebServerTCPConnections.h"
#include "WebServerConnectionManager.h"
#include "../Util/cJSON.h"

DefaultRequestBuffer*		mspBufferedRequests = NULL;

RegisteredRequestHandlerList*	mspRegisteredRequestHandlerList = NULL;

void	DefaultPageRequestHandlerDeleteDefaultRequestBuffer( DefaultRequestBuffer* pDefaultRequestBufferToDelete )
{
DefaultRequestBuffer*		pDefaultRequestBuffers = mspBufferedRequests;
DefaultRequestBuffer*		pLast = NULL;
DefaultRequestBuffer*		pNext;

	while( pDefaultRequestBuffers )
	{
		pNext = pDefaultRequestBuffers->mpNext;
		if ( pDefaultRequestBuffers == pDefaultRequestBufferToDelete )
		{
			if ( pLast )
			{
				pLast->mpNext = pNext;
			}
			else
			{
				mspBufferedRequests = pNext;
			}
			delete pDefaultRequestBufferToDelete;
			return;
		}
		else
		{
			pLast = pDefaultRequestBuffers;
		}
		pDefaultRequestBuffers = pNext;
	}
}


BOOL	RegisteredRequestHandlerList::Register( const char* szRegisteredRequestName, RequestHandlerNewFunction fnNewRequest )
{
RegisteredRequestHandlerList*		pNewRequestRegistration = new RegisteredRequestHandlerList;

	pNewRequestRegistration->mszRegisteredRequestName = (char*)( malloc( strlen( szRegisteredRequestName ) + 1 ) );
	strcpy( pNewRequestRegistration->mszRegisteredRequestName, szRegisteredRequestName );
	pNewRequestRegistration->mfnRequestNew = fnNewRequest;
	pNewRequestRegistration->mpRequestHandlerSingleton = NULL;

	pNewRequestRegistration->mpNext = mspRegisteredRequestHandlerList;
	mspRegisteredRequestHandlerList = pNewRequestRegistration;

	return( TRUE );
}

void		DefaultPageRequestHandlerSendResponse( DefaultRequestBuffer* pOriginalRequest, int nResponseCode, const char* pbBody, int nBodyLen )
{
	TcpConnectionSendHTTPResponse( pOriginalRequest->mpClientConnection->GetTcpSocket(), nResponseCode, pbBody, nBodyLen );
}

BOOL		DefaultPageRequestHandlerCompareWithoutExtension( const char* szHandler, const char* szRequest )
{
const char*		pszHandlerRunner = szHandler;
const char*		pszRequestRunner = szRequest;

	while( ( *pszRequestRunner != 0 ) &&
		   ( *pszHandlerRunner != 0 ) )
	{
		if ( *pszRequestRunner == '.' )
		{
			break;
		}
		else if ( *pszRequestRunner != *pszHandlerRunner )
		{
			return( FALSE );
		}
		pszRequestRunner++;
		pszHandlerRunner++;
	}

	if ( ( *pszRequestRunner == '.' ) &&
		 ( *pszHandlerRunner == 0 ) )
	{
		return( TRUE );
	}
	return( FALSE );
}

//---------------------------------------------------------------------------------------
// DefaultPageRequestHandlerParseRequest
// 
// This is called when we've received a http request.
//  Return TRUE if the request was fully handled here so the request buffer can be deleted
//  Return FALSE if we've started off some async process that needs the DefaultRequestBuffer to stay around.
//---------------------------------------------------------------------------------------
BOOL		DefaultPageRequestHandlerParseRequest( DefaultRequestBuffer*	pDefaultRequestBuffer )
{
RegisteredRequestHandlerList*		pRequestHandlers = mspRegisteredRequestHandlerList;
const char*			pszRequest = pDefaultRequestBuffer->mszRequest + 1;
RegisteredRequestHandlerList*		pDefaultHandler = NULL;
RegisteredRequestHandlerList*		pDefaultExtensionlessHandler = NULL;

	// If its an empty / index request
	if ( pszRequest[0] == 0 )
	{
		pszRequest = "index.html";
	}

	// Check all the registered request handlers..
	while( pRequestHandlers )
	{
		if ( pRequestHandlers->mszRegisteredRequestName[0] == 0 )
		{
			pDefaultHandler = pRequestHandlers;
		}

		// TODO - Check for wildcards here (e.g. mszRegisteredRequestName could be index.* and handle index.html, index.htm, index etc)
		if ( tinstricmp( pRequestHandlers->mszRegisteredRequestName, pszRequest ) == 0 )
		{
			return( pRequestHandlers->mpRequestHandlerSingleton->HandleRequest( pDefaultRequestBuffer ) );
		}
		else if ( DefaultPageRequestHandlerCompareWithoutExtension( pRequestHandlers->mszRegisteredRequestName, pszRequest ) == TRUE )
		{
			pDefaultExtensionlessHandler = pRequestHandlers;
		}

		pRequestHandlers = pRequestHandlers->mpNext;
	}

	if ( pDefaultExtensionlessHandler )
	{
		return( pDefaultExtensionlessHandler->mpRequestHandlerSingleton->HandleRequest( pDefaultRequestBuffer ) );
	}
	else if ( pDefaultHandler )
	{
		return( pDefaultHandler->mpRequestHandlerSingleton->HandleRequest( pDefaultRequestBuffer ) );
	}

	 // Unknown send 404 response
	SysUserPrint( 0, "Received unknown request: %s", pDefaultRequestBuffer->mszRequest );
	TcpConnectionSendHTTPResponse( pDefaultRequestBuffer->mpClientConnection->GetTcpSocket(), 404, NULL, 0 );
	return( TRUE );
}

//------------------------------------------------------

int		DefaultPageRequestHandler( ClientConnection* pConnection, const char* szRequest, RequestParamsList* pRequestParams )
{
DefaultRequestBuffer*		pDefaultRequestBuffer;
int			nBodyLen = 0;
char	szRecvBuffer[4096];
//const char*			pcAdminKey;

	// We copy all the info into a buffer so that its kept around if we want to do things like
	//  validate login details before we send a response.
	// For basic default behaviour this isnt really needed
	pDefaultRequestBuffer = new DefaultRequestBuffer;
	pDefaultRequestBuffer->mpNext = mspBufferedRequests;
	mspBufferedRequests = pDefaultRequestBuffer;

	nBodyLen = pConnection->GetHTTPRequestBody( szRecvBuffer, 4096 );

	if ( nBodyLen > 0 )
	{
		pDefaultRequestBuffer->mszResponseBody = (char*)malloc( nBodyLen + 1 );
		memcpy( pDefaultRequestBuffer->mszResponseBody, szRecvBuffer, nBodyLen );
		pDefaultRequestBuffer->mszResponseBody[nBodyLen] = 0;
	}

	pDefaultRequestBuffer->mszRequest = (char*)( malloc( strlen(szRequest)+1 ) );
	strcpy( pDefaultRequestBuffer->mszRequest, szRequest );
	pDefaultRequestBuffer->mpClientConnection = pConnection;
	pRequestParams->Copy( &pDefaultRequestBuffer->mRequestParams );

	if ( DefaultPageRequestHandlerParseRequest( pDefaultRequestBuffer ) == TRUE )
	{
		DefaultPageRequestHandlerDeleteDefaultRequestBuffer( pDefaultRequestBuffer );
	}

	// Return 0 if not handled
	return( 1 );
}



void		DefaultPageRequestHandlerInit( void )
{
	BasicWebServerRegisterGETHandler( DefaultPageRequestHandler );
	BasicWebServerRegisterPOSTHandler( DefaultPageRequestHandler );
//	HostLoginServiceRegisterSessionResponse( PageRequestSessionValidated );

	// Create all request handler singletons
	RegisteredRequestHandlerList*		pRequestHandlers = mspRegisteredRequestHandlerList;

	// Check all the registered request handlers..
	while( pRequestHandlers )
	{
		pRequestHandlers->mpRequestHandlerSingleton = pRequestHandlers->mfnRequestNew();
		pRequestHandlers = pRequestHandlers->mpNext;
	}
}

void	DefaultPageRequestHandlerShutdown( void )
{
RegisteredRequestHandlerList*		pRequestHandlers = mspRegisteredRequestHandlerList;
RegisteredRequestHandlerList*		pNext;

	while( pRequestHandlers )
	{
		pNext = pRequestHandlers->mpNext;
		delete pRequestHandlers;
		pRequestHandlers = pNext;
	}
}

