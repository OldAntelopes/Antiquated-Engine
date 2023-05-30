#include <stdio.h>
#include <UnivSocket.h>

#include "../tcp4u/tcp4u.h"

#include <StandardDef.h>


#include "WebServerConnectionManager.h"
#include "WebServerTCPConnections.h"


SOCKET		m_WebServerListenSocket;
unsigned int		m_hWebServerConnectionListenThread;
volatile BOOL		m_bWebServerNetworkThreadShutdown = FALSE;
volatile BOOL		m_bWebServerNewTCPConnectionsPending = FALSE;


SOCKET		m_WebServerPendingNewConnection = 0;		// TODO - will need more of these..


long	WebServerTCPListenThread(long lParam)
{ 
SOCKET		xNewConnection;
int		ret;
BOOL	bHasErrored = FALSE;

	do
	{
		// Only process one connection at a time
		if ( m_bWebServerNewTCPConnectionsPending == FALSE )
		{
			ret = TcpAccept(&xNewConnection, m_WebServerListenSocket, 1, NULL);

			// If we've got a new TCP connection comin in
			if ( ret == TCP4U_SUCCESS )
			{
		//		xNewConnection
				m_WebServerPendingNewConnection = xNewConnection;
				m_bWebServerNewTCPConnectionsPending = TRUE;
//				printf("TCP connection created!\n");
			}

			if ( ( ret != TCP4U_SUCCESS ) &&
				 ( ret != TCP4U_TIMEOUT ) )
			{
				bHasErrored = TRUE;
			}
		}
		SysSleep( 10 );
		
	} while( bHasErrored == FALSE );

	return( 0 );
}


void	TCPConnections::UpdateNewTCPConnections( void )
{
ClientConnection*		pNewConnection;
char StationName[64];
DWORD	dwAddress;
uint32 ulIPAddress;

	TcpGetRemoteID (m_WebServerPendingNewConnection, StationName, sizeof StationName, &dwAddress);
	ulIPAddress = *((uint32*)&dwAddress);
	pNewConnection = WebServerConnectionManager::Get().DecodeNewTCPConnection(ulIPAddress);

	if ( pNewConnection )
	{
		pNewConnection->SetTcpSocket( m_WebServerPendingNewConnection );
	}
	else
	{
		// TODO - 
		// Error getting a ClientConnection class... we should terminate the pendingNewConnection SOCKET here

	}
	m_bWebServerNewTCPConnectionsPending = FALSE;
	m_WebServerPendingNewConnection = 0;

}

void	TCPConnections::ShutdownTCP( void )
{
	TcpAbort();

	m_bWebServerNetworkThreadShutdown = TRUE;
	SysSleep(10);

	TcpClose ( &m_WebServerListenSocket ); 
	Tcp4uCleanup();
}

int	TCPConnections::InitialiseTCPListener( ushort uwPort )
{
int		ret;

	Tcp4uInit();

	ret = TcpGetListenSocket (&m_WebServerListenSocket, NULL, &uwPort, 4);
	if ( ret == TCP4U_SUCCESS )
	{
		printf("BasicWebServer TCP listen socket open on %d\n", uwPort );
		m_hWebServerConnectionListenThread = SysCreateThread( WebServerTCPListenThread, NULL, 0, 0 );
		return( 1 );
	}
	else
	{
		printf("ERROR: TCP listen socket bind failed on %d\n", uwPort );
		return( -1 );
	}
}


void	TCPConnections::Update( void )
{
	if ( m_bWebServerNewTCPConnectionsPending )
	{
		UpdateNewTCPConnections();
	}

}

//------------------------------------------------------------------

int		TCPSocketGetRequestLine( SOCKET hSocket, char* recvBuff, int size)
{
char	c = 0;
int		ret;
char*	pcOutRunner = recvBuff;
char*	pcBuffEnd = recvBuff + size - 1;
int		nBytesReceived = 0;
int		error;

#ifdef WIN32
    ret = recv( hSocket, &c, 1, MSG_PEEK );
#else
    ret = recv( hSocket, &c, 1, MSG_PEEK | MSG_DONTWAIT );
#endif

	// Messages have been returned
	if ( ret > 0 )
	{
		if ( c == '\n' )
		{
			ret = recv( hSocket, &c, 1, 0 );
			nBytesReceived++;
		}
		
		while ( ( pcOutRunner < pcBuffEnd ) && 
				( c != '\n') )
		{
			ret = recv( hSocket, &c, 1, 0 );
			nBytesReceived++;

			if ( nBytesReceived == size )
			{
				printf( "ERROR: Recv buffer not big enough!!!\n" );
				return( 0 );
			}

			if ( ret > 0 )
			{
				if ( c == '\r')
				{
					ret = recv( hSocket, &c, 1, MSG_PEEK );
					if ( (ret > 0) && 
						 (c == '\n') )
					{
						ret = recv( hSocket, &c, 1, 0 );
					}
					else
					{
						 c = '\n';
					}
				}		
			}
			else
			{
				c = '\n';
			}
			*pcOutRunner = c;
			pcOutRunner++;
		}	// end while

		*pcOutRunner = 0;
		pcOutRunner++;

		return( pcOutRunner - recvBuff );
	}

#ifdef WIN32
	error = WSAGetLastError();
	if ( error == WSAEWOULDBLOCK )
	{
		return( 0 );
	}
#else
	if ( ( errno == EWOULDBLOCK ) ||
		 ( errno == EAGAIN ) )
	{
		return( 0 );
	}
#endif
	return( TCPSOCKET_ERROR_DISCONNECT );
}


void	TcpConnectionSendHTTPResponse( SOCKET hSocket, int nResponseCode, const char* szBody, int nBodyLen, eHeaderFlags flags  )
{
char	buf[256];

	switch( nResponseCode )
	{
	case 200:
		sprintf(buf, "HTTP/1.1 200 OK\r\n");
		break;
	case 201:
		sprintf(buf, "HTTP/1.1 201 Created\r\n");
		break;
	case 202:
		sprintf(buf, "HTTP/1.1 202 Accepted\r\n");
		break;
	case 204:
		sprintf(buf, "HTTP/1.1 204 No Content\r\n");
		break;
	case 400:
		sprintf(buf, "HTTP/1.1 400 Bad Request\r\n");
		break;
	case 401:
		sprintf(buf, "HTTP/1.1 401 Unauthorized\r\n");
		break;
	case 402:
		sprintf(buf, "HTTP/1.1 402 Payment Required\r\n");
		break;
	case 403:
		sprintf(buf, "HTTP/1.1 403 Forbidden\r\n");
		break;
	case 404:
		sprintf(buf, "HTTP/1.1 404 Not Found\r\n");
		break;
	case 405:
		sprintf(buf, "HTTP/1.1 405 Error\r\n");
		break;
	case 408:
		sprintf(buf, "HTTP/1.1 408 Request Timeout\r\n");
		break;
	case 500:
		sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\n");
		break;
	case 501:
		sprintf(buf, "HTTP/1.1 501 Not Implemented\r\n");
		break;
	case 503:
		sprintf(buf, "HTTP/1.1 503 Service Unavailable\r\n");
		break;
	default:
		sprintf(buf, "HTTP/1.1 %d\r\n", nResponseCode );
		break;
	}
	
	send( hSocket, buf, strlen(buf), 0);

	sprintf(buf, "Content-Length: %d\r\n", nBodyLen);
	send( hSocket, buf, strlen(buf), 0);

	if ( flags & CONTENT_TYPE_TEXT )
	{
		sprintf(buf, "Content-Type: text/html\r\n" );
		send( hSocket, buf, strlen(buf), 0);
	}
	
	if ( ( flags & CLOSE_CONNECTION ) ||
		 ( nResponseCode != 200 ) )
	{
		sprintf(buf, "Connection: close\r\n" );
		send( hSocket, buf, strlen(buf), 0);
	}
	else
	{
		 sprintf(buf, "Connection: keep-alive\r\n" );
		 send( hSocket, buf, strlen(buf), 0);
	}
	// end headers
	sprintf(buf, "\r\n");
	send( hSocket, buf, strlen(buf), 0);

	if ( nBodyLen > 0 )
	{
		send( hSocket, szBody, nBodyLen, 0);
	}
}
