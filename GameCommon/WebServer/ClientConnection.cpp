
#ifdef WIN32
#include "stdwininclude.h"
#endif

#include "UnivSocket.h"
#include "StandardDef.h"

#include "ClientConnection.h"

#include "BasicWebServer.h"
#include "BasicWebServerInternal.h"
#include "WebServerConnectionManager.h"


#include "WebServerTCPConnections.h"
#include "RequestParams.h"



const char* SERVER_STRING =	"Server: Mit Webserver/0.0.1\r\n";

//#define DEBUG_TCP_RECEIVE

void		DebugTCPReceive( const char* szText )
{
#ifdef DEBUG_TCP_RECEIVE
	printf( szText );
	printf( "\n" );
#endif
}

void		DecodeHeaderLine( const char* szLine, char* szWordOut, char* szValueOut )
{
const char*	pcLineRunner = szLine;
char*	pcOutRunner = szWordOut;

	while( ( *pcLineRunner != ':' ) &&
		   ( *pcLineRunner != 0 ) )
	{
		*pcOutRunner = *pcLineRunner;
		pcOutRunner++;
		pcLineRunner++;
	}

	*pcOutRunner = 0;

	if ( *pcLineRunner == ':' )
	{
		pcLineRunner++;
		if ( *pcLineRunner == ' ' )
		{
			pcOutRunner = szValueOut;
			while( ( *pcLineRunner != '\r' ) &&
				   ( *pcLineRunner != 0 ) )
			{
				*pcOutRunner = *pcLineRunner;
				pcOutRunner++;
				pcLineRunner++;
			}		
		}
		*pcOutRunner = 0;
	}

}


BOOL	CurrentReceive::Update( int hTCPSocket, ulong ulFromIP )
{
char	szRecvBuffer[1024];
int		ret;

	szRecvBuffer[0] = 0;
	szRecvBuffer[1] = 0;

	if ( mbHasRequest == FALSE )
	{
		ret = TCPSocketGetRequestLine( hTCPSocket, szRecvBuffer, 1024 );

		if ( ret > 0 )
		{
		int		nRequestLen = strlen( szRecvBuffer );

			mulLastReceiveTime = SysGetTimeLong();

			// If keepalive, ignore..
			if ( ( nRequestLen == 1 ) &&
				 ( szRecvBuffer[0] == 10 ) )
			{
//				DebugTCPReceive( "Keepalive");
				return( TRUE );
			}
			else
			{
				mszRequest = (char*)malloc( nRequestLen + 1 );
				memcpy( mszRequest, szRecvBuffer, nRequestLen );
				mszRequest[nRequestLen] = 0;
				mbHasRequest = TRUE;
				DebugTCPReceive( "Request:");
				DebugTCPReceive( mszRequest );

				mszRequestHeaders = (char*)( malloc( 4096 ) );
				mszRequestHeaders[0] = 0;
				mpcRequestHeadersRunner = mszRequestHeaders;
			}
		}
		else if ( ret == TCPSOCKET_ERROR_DISCONNECT )
		{
			return( FALSE );
		}
		else if ( ret == 0 )
		{
		ulong		ulSecsSinceReceive = SysGetTimeLong() - mulLastReceiveTime;
			// Timeout all connections if we haven't received anything from them for over a minute..
			if ( ulSecsSinceReceive > 60 )
			{
				// return FALSE to signal disconnect
				return( FALSE );
			}
		}
	}
	else if ( mbHasHeaders == FALSE )
	{
		do 
		{
			ret = TCPSocketGetRequestLine( hTCPSocket, szRecvBuffer, 1024);

			if ( ret == 2 )		// if empty line ( CRLF ) its the end of the headers.. body is next
			{
				mulLastReceiveTime = SysGetTimeLong();
				if ( ( strlen( szRecvBuffer ) == 1 ) &&
					 ( szRecvBuffer[0] == 10 ) )
				{
					// ignore keepalive
					DebugTCPReceive( "Keepalive?" );
				}

				mbHasHeaders = TRUE;
				if ( mnContentLength > 0 )
				{
				char	acString[256];

					mszBody = (char*)( malloc( mnContentLength + 1 ) );
					mszBody[0] = 0;
					mpcBodyRunner = mszBody;

					sprintf( acString, "Headers complete: content length %d", mnContentLength );
					DebugTCPReceive( acString );
				}
				else
				{
					mbHasBody = TRUE;
					DebugTCPReceive( "Headers complete: no content" );
				}
				ret = 0;
			}
			else if ( ret == 1 )
			{
				mulLastReceiveTime = SysGetTimeLong();
				DebugTCPReceive( "Ignore 1 byte keepalive" );
			}
			else if ( ret > 1000 )
			{
				// Over 1000 is probably someone trying to 'attack' the server.. 
				// Ignore and close the conn...
#if 0 
				SysDebugPrint( "WARN: Oversized request, ignored from %s", inet_ntoa( *((struct in_addr*)(&ulFromIP)) ) );
#endif
				// return FALSE to signal disconnect
				return( FALSE );
			}
			else if ( ret > 0 )
			{
			char	acHeaderName[256];
			char	acHeaderValue[1024];

				mulLastReceiveTime = SysGetTimeLong();
				strcpy( mpcRequestHeadersRunner, szRecvBuffer );
				mpcRequestHeadersRunner += ret;
				if ( mpcRequestHeadersRunner >= mszRequestHeaders + 4095 )
				{
					SysPanicIf( TRUE, "Request headers overrun" );
					mpcRequestHeadersRunner = mszRequestHeaders;
				}

				DebugTCPReceive( "Header:" );
				DebugTCPReceive( szRecvBuffer );
				DecodeHeaderLine( szRecvBuffer, acHeaderName, acHeaderValue );

				if ( tinstricmp( acHeaderName, "Content-length" ) == 0 )
				{
					mnContentLength = strtol( acHeaderValue, NULL, 10 );
				}
			}
			else if ( ret == TCPSOCKET_ERROR_DISCONNECT )
			{
				return( FALSE );
			}
			else if ( ret == 0 )
			{
			ulong		ulSecsSinceReceive = SysGetTimeLong() - mulLastReceiveTime;
				// Timeout all connections if we haven't received anything from them for over a minute..
				if ( ulSecsSinceReceive > 60 )
				{
					// return FALSE to signal disconnect
					return( FALSE );
				}
			}
	
		} while( ret > 0 );
	}
	else if ( mbHasBody == FALSE )
	{
		ret = recv( hTCPSocket, mpcBodyRunner, mnContentLength - mnContentReceived, 0 );
		if ( ret > 0 )
		{
			mulLastReceiveTime = SysGetTimeLong();
			if ( ( ret == 2 ) &&
				 ( mpcBodyRunner[0] == 10 ) )
			{
				// ignore keepalive
				DebugTCPReceive( "Ignored body keepalive" );
			}
			else
			{
				mnContentReceived += ret;
				mpcBodyRunner += ret;
			}

			if ( mnContentReceived >= mnContentLength )
			{
				SysPanicIf( mnContentReceived > mnContentLength, "Content length overrun" );
				DebugTCPReceive( "Body complete" );
				mbHasBody = TRUE;
			}
		}
		else if ( ret < 0 )
		{
#ifdef WIN32
			// WSAEWOULDBLOCK just means we have no data yet.. anything else is a proper error
			if ( WSAGetLastError() != WSAEWOULDBLOCK )
			{
				DebugTCPReceive( "Reset - body error" );
				Reset();
				return( FALSE );
			}
#else

#endif
		}
		else if ( ret == 0 )
		{
		ulong		ulSecsSinceReceive = SysGetTimeLong() - mulLastReceiveTime;
			// Timeout all connections if we haven't received anything from them for over a minute..
			if ( ulSecsSinceReceive > 60 )
			{
				// return FALSE to signal disconnect
				return( FALSE );
			}
		}
	}
	return( TRUE );
}


int		ClientConnection::GetHTTPRequestBody( char* pcOutBuffer, int nOutBufferSize )
{
int		nBodyLen = mCurrentReceive.GetBodyLength();

	if ( nBodyLen > 0 )
	{
		if ( nBodyLen > nOutBufferSize )
		{
			SysDebugPrint( "ERROR: Body size overflow" );
			nBodyLen = -1;
		}
		else
		{
			memcpy( pcOutBuffer, mCurrentReceive.GetBody(), nBodyLen );
		}
	}
	else
	{
		pcOutBuffer[0] = 0;
	}
	return( nBodyLen );
}


//---------------------------------------------------------------------------
// SendStandardHeaderResponse
//---------------------------------------------------------------------------
void	ClientConnection::SendStandardHeaderResponse( const char* szContentType, int nContentLength )
{
char buf[512];

	 sprintf(buf, SERVER_STRING);
	 send(m_hTcpSocket, buf, strlen(buf), 0);
	 
	 if ( szContentType )
	 {
	

	 }
	 else
	 {
		sprintf(buf, "Content-Type: text/html\r\n");
	 }
	 send( m_hTcpSocket, buf, strlen(buf), 0);

	 sprintf(buf, "Content-Length: %d\r\n", nContentLength );
	 send( m_hTcpSocket, buf, strlen(buf), 0);

	 sprintf(buf, "\r\n");
	 send( m_hTcpSocket, buf, strlen(buf), 0);
}

void	ClientConnection::SendBody( const char* szContentBody, int nLen )
{
	send( m_hTcpSocket, szContentBody, nLen, 0);
}



char*	ClientConnection::WriteStandardHTMLHeader( char* szBodyBuffer, const char* szTitle )
{
char buf[512];
int		nLen;

	sprintf(buf, "<HTML><HEAD><TITLE>%s\r\n", szTitle );
	nLen = strlen( buf );
	memcpy( szBodyBuffer, buf, nLen );
	szBodyBuffer += nLen;

	sprintf(buf, "</TITLE></HEAD>\r\n");
	nLen = strlen( buf );
	memcpy( szBodyBuffer, buf, nLen );
	szBodyBuffer += nLen;

	return( szBodyBuffer );
}

char*	ClientConnection::WriteStandardHTMLBodyFooter( char* szBodyBuffer )
{
char buf[512];
int		nLen;

	sprintf(buf, "</TD></TR></TABLE></BODY></HTML>\r\n");
	nLen = strlen( buf );
	memcpy( szBodyBuffer, buf, nLen );
	szBodyBuffer += nLen;

	return( szBodyBuffer );
}

char*	ClientConnection::WriteStandardHTMLBodyHeader( char* szBodyOut )
{
char buf[512];
int		nLen;
	
	sprintf(buf, "<BODY><CENTER><TABLE WIDTH=800><TR HEIGHT=100 BGCOLOR=#203070><TD ALIGN=CENTER><FONT FACE=ARIAL SIZE=6 COLOR=#FFFFFF><b>Old Man HTTP Server</b></font></TD></TR>\r\n");
	nLen = strlen( buf );
	memcpy( szBodyOut, buf, nLen );
	szBodyOut += nLen;

	sprintf(buf, "<TR HEIGHT=20><TD></TD></TR><TR HEIGHT=100%%><TD>\r\n");
	nLen = strlen( buf );
	memcpy( szBodyOut, buf, nLen );
	szBodyOut += nLen;

	return( szBodyOut );
}


//---------------------------------------------------------------------------
// SendNotImplementedResponse
//---------------------------------------------------------------------------
void ClientConnection::SendNotImplementedResponse( void )
{
/*
char buf[1024];

	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send( m_hTcpSocket, buf, strlen(buf), 0);

	SendStandardHeaderResponse( NULL );

	SendStandardHTMLHeader( "Method Not Implemented");

 	SendStandardHTMLBodyHeader();

	sprintf(buf, "<font size=4><B>HTTP request method not supported.</B></font>\r\n");
	send(m_hTcpSocket, buf, strlen(buf), 0);

	SendStandardHTMLBodyFooter();
*/
}

//---------------------------------------------------------------------------
// SendNotFoundResponse
//---------------------------------------------------------------------------
void ClientConnection::SendNotFoundResponse( void )
{
char buf[1024];
char	szBody[4096];
char*	pcBodyRunner = szBody;
int		nLen;

	sprintf(buf, "HTTP/1.0 404 Not Found\r\n");
	send(m_hTcpSocket, buf, strlen(buf), 0);
	
	pcBodyRunner = WriteStandardHTMLHeader( pcBodyRunner, "Page not found");
	pcBodyRunner = WriteStandardHTMLBodyHeader( pcBodyRunner );

	sprintf(buf, "<font size=4><B>404: Page not found</B></font>\r\n");
	nLen = strlen( buf );
	memcpy( pcBodyRunner, buf, nLen );
	pcBodyRunner += nLen;

	pcBodyRunner = WriteStandardHTMLBodyFooter( pcBodyRunner );

	nLen = (int)( pcBodyRunner - szBody );

	// ---------- Send response
	SendStandardHeaderResponse( NULL, nLen );

	SendBody( szBody, nLen );

}

void	ClientConnection::SendHTTPResponse( const char* szResponseBody, int nBodyLen )
{
char buf[1024];

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	send(m_hTcpSocket, buf, strlen(buf), 0);
	
	// ---------- Send response
	SendStandardHeaderResponse( NULL, nBodyLen );

	SendBody( szResponseBody, nBodyLen );
}


//------------------------------------------------------------------
				



const char*		ClientConnection::GetNextWord( const char* szLine, char* szWordOut, int nWordOutBufferSize )
{
const char*		pcInRunner = szLine;
char*			pcOutRunner = szWordOut;

	while( ( *pcInRunner != 0 ) &&
		   ( *pcInRunner != ' ' ) )
	{
		*pcOutRunner++ = *pcInRunner++;
	}

	*pcOutRunner = 0;
	if ( *pcInRunner == ' ' )
	{
		pcInRunner++;
	}
	return( pcInRunner );
}



char*		ClientConnection::StripParamsFromPath( char* szFullPath )
{
char*		pcRunner = szFullPath;

	while( ( *pcRunner != 0 ) &&
		   ( *pcRunner != '?' ) )
	{
		pcRunner++;
	}

	if ( *pcRunner == '?' )
	{
		*pcRunner = 0;
		pcRunner++;
		return( pcRunner );
	}
	return( NULL );
}


void	ClientConnection::ParseRequest( const char* szRecvBuffer )
{
const char*		pcRunner = szRecvBuffer;
const char*		pcNextRunner;
char			szMethod[256];
char			szPath[1024];
char			szProtocol[128];
char*			pcParams;
int				nPathLen;

	mRequestParamsList.Clear();

	pcRunner = GetNextWord( pcRunner, szMethod, 255 );
	pcNextRunner = GetNextWord( pcRunner, szPath, 1023 );
	nPathLen = (int)( pcNextRunner - pcRunner );
	pcRunner = GetNextWord( pcNextRunner, szProtocol, 127 );

	if ( nPathLen > 800 )
	{
		SendNotImplementedResponse();
		mRequestParamsList.Clear();
		return;
	}

	pcParams = StripParamsFromPath( szPath );
	if ( pcParams )
	{
		mRequestParamsList.ParseParams( pcParams );
	}

//	char szString[512];
//	sprintf( szString, "Request: %s", szPath );
//	DisplayAddText( szString );

	if ( tinstricmp( szMethod, "GET" ) == 0 )
	{
	BasicWebServerRequestHandler		fnGETHandler = BasicWebServerGetGETHandler();
	int		nHandled = 0;

		if ( fnGETHandler )
		{
			nHandled = fnGETHandler( this, szPath, &mRequestParamsList );
		}

		if ( nHandled == 0 )
		{
			SendNotFoundResponse( );
			mRequestParamsList.Clear();
		}
	}
	else if ( strcmp( szMethod, "POST" ) == 0 )
	{
	BasicWebServerRequestHandler		fnPOSTHandler = BasicWebServerGetPOSTHandler();
	int		nHandled = 0;

		if ( fnPOSTHandler )
		{
			nHandled = fnPOSTHandler( this, szPath, &mRequestParamsList );
		}

		if ( nHandled == 0 )
		{
			SendNotFoundResponse( );
			mRequestParamsList.Clear();
		}
	}
	else
	{
		SendNotImplementedResponse();
		mRequestParamsList.Clear();
	}
}

void	ClientConnection::CloseConnection( void )
{
	mRequestParamsList.Clear();

#ifdef WIN32
	closesocket(m_hTcpSocket);
#else
	shutdown(m_hTcpSocket, SHUT_RDWR);
#endif
	m_hTcpSocket = 0;
}

void	ClientConnection::DeleteConnection( void )
{
	CloseConnection();
	// This marks this connection as for deletion, and is deleted in subsequent update
	WebServerConnectionManager::Get().DestroyConnection( this );
}


void	ClientConnection::UpdateConnection( float fDelta )
{
	if ( m_hTcpSocket > 0 )
	{
		if ( mCurrentReceive.Update( m_hTcpSocket, m_ulIP ) == FALSE )
		{
		BasicWebServerCloseConnectionHandler	fnCloseConnectionHandler = BasicWebServerGetCloseConnectionHandler();

			if ( fnCloseConnectionHandler )
			{
				fnCloseConnectionHandler( this );
			}
			
			DeleteConnection();
		}
		else
		{
			if ( mCurrentReceive.HasReceivedRequest() )
			{
				ParseRequest( mCurrentReceive.GetRequest() );			
				DebugTCPReceive( "Reset : Parse request" );
				mCurrentReceive.Reset();
			}
		}
	}
	else
	{
	BasicWebServerCloseConnectionHandler	fnCloseConnectionHandler = BasicWebServerGetCloseConnectionHandler();

		if ( fnCloseConnectionHandler )
		{
			fnCloseConnectionHandler( this );
		}
		DeleteConnection();
	}
}
