
#include <stdio.h>
#include <UnivSocket.h>
#include <StandardDef.h>
#include <CodeUtil.h>

#ifdef MARMALADE
#include <s3eSocket.h>
#else
#include "TCP4u/TCP4u.h"
#endif

#include "Networking.h"
#include "NetworkConnection.h"

#ifndef NO_PEER_CONN
#include "PeerConnection\ConnectionManager.h"
#include "PeerConnection\PeerConnection.h"
#endif

#ifndef SERVER
#include "UDPSession\UDPSessionConnection.h"
#include "UDPSession\UDPSessionClient.h"
#endif

#define	UDP_PEER_CONNECTION_GUARANTEED	0xB4
#define	UDP_PEER_CONNECTION_SYS			0xB6

#define	MAX_REROUTES	16
#define MESSAGE_SCRAMBLING
//#define	SEND_MSG_LOG

#ifdef WIN32
WSADATA ws;
#endif

int		mnNumReroutes = 0;
#ifdef MARMALADE

s3eInetAddress		mxLastMessageSourceAddress;
#else
struct sockaddr_in	maxReRouteAddrSrc[MAX_REROUTES];
struct sockaddr_in	maxReRouteAddrDest[MAX_REROUTES];

struct	sockaddr_in			mxLastMessageSourceAddress;
#endif

BOOL	mboUDPConnected = FALSE;
int		mnConnectionMode = 0;

BOOL	mbNetworkConnectionPacketLoggingEnabled = FALSE;
FILE*	mpNetworkConnectionPacketLogFile = NULL;
ulong	mulNewNetstatsLastSampleTick = 0;

extern "C"
{
// Horrible hackery to avoid replacing lots of TU references to this function (which i should really just do..)
//    backward compatability of encryption keys is da issue (should i find this comment some years later and not remember what i was thinking about)
//       (p.s. the comment above won't help me to remember)
extern void GenericSendMessage( char* pcMsg, int nMsgLength, struct sockaddr_in* pxSendAddress );	

}

NetworkConnection		m_sNetworkConnectionSingleton;

int		mnScrambleKeyLength = 16;
byte	mabScrambleKeyBuffer[256] = { 17, 115, 83, 94, 32, 150, 77, 250, 33, 15, 88, 139, 201,60,234, 171, 0 };

NETWORK_STATISTICS		m_sNetworkStatistics = { 0 };

NEW_NETWORK_STATISTICS		m_sNetstatsCurrent;
NEW_NETWORK_STATISTICS		m_sNetstatsRecent;
NEW_NETWORK_STATISTICS		m_sNetstatsTotal;
NEW_NETWORK_STATISTICS		m_sNetstatsPeak;

ulong		mulNewNetatsLastSampleTick = 0;

void			NetworkingStatisticsActivatePacketLog( BOOL bFlag )
{
	if ( ( bFlag ) &&
		 ( !mbNetworkConnectionPacketLoggingEnabled ) )
	{
		mpNetworkConnectionPacketLogFile = fopen( "packet.log", "wb" );
	}
	else if ( (!bFlag) &&
		      ( mpNetworkConnectionPacketLogFile != NULL ) )
	{
		fclose( mpNetworkConnectionPacketLogFile );
		mpNetworkConnectionPacketLogFile = NULL;
	}

	mbNetworkConnectionPacketLoggingEnabled = bFlag;
		
}

void	NetworkConnectionPacketLogAddLine( const char* szText )
{
	if ( mpNetworkConnectionPacketLogFile )
	{
	SYS_LOCALTIME	xSystemTime;
	char	acString[256];
	//ulong	ulSysTick = SysGetTick() % 1000;

		SysGetLocalTime( &xSystemTime );
		sprintf( acString, "[%02d:%02d:%02d] ", xSystemTime.wHour, xSystemTime.wMinute, xSystemTime.wSecond );
		fwrite( acString, strlen( acString ), 1, mpNetworkConnectionPacketLogFile );
		fwrite( szText, strlen( szText ), 1, mpNetworkConnectionPacketLogFile );
		sprintf( acString, "\r\n" );
		fwrite( acString, strlen( acString ), 1, mpNetworkConnectionPacketLogFile );
	}
}

void	NetworkingStatsUpdate( NEW_NETWORK_STATISTICS* pxStats )
{
int		nSampleTime = SysGetTick() - pxStats->mulStatsStartTick;

	if ( nSampleTime > 0 )
	{
		pxStats->mnSentBytesPerSecond = (pxStats->mnBytesSent * 1000) / nSampleTime; 
		pxStats->mnRecvdBytesPerSecond = (pxStats->mnBytesRecvd * 1000) / nSampleTime;
	}
}

NEW_NETWORK_STATISTICS*		NetworkingStatisticsGetRecent( void )
{
	return( &m_sNetstatsRecent );
}

NEW_NETWORK_STATISTICS*		NetworkingStatisticsGetTotals( void )
{
	return( &m_sNetstatsTotal );
}


void	NetworkingStatsAddReceive( byte* pbMsg, int nBytesReceived, int nType )
{
	if ( mbNetworkConnectionPacketLoggingEnabled )
	{
	char	acLogText[400];
	NetworkStatisticsLogAnnotationCallback		fnAnnotationCallback = NetworkConnection::Get().GetAnnotationCallback();

		sprintf( acLogText, "-RECV- Size:%03d | %02x %02x %02x %02x %02x %02x %02x %02x | ", nBytesReceived, pbMsg[0], pbMsg[1], pbMsg[2], pbMsg[3], pbMsg[4], pbMsg[5], pbMsg[6], pbMsg[7] );
		if ( fnAnnotationCallback )
		{
		char	acAnnotation[256];
			acAnnotation[0] = 0;
			fnAnnotationCallback( pbMsg, nBytesReceived, acAnnotation );
			strcat( acLogText, acAnnotation );
		}
		NetworkConnectionPacketLogAddLine( acLogText );
	}

	m_sNetworkStatistics.mnStatsBytesRecvd += nBytesReceived;
	m_sNetstatsCurrent.mnBytesRecvd += nBytesReceived;
	m_sNetstatsTotal.mnBytesRecvd += nBytesReceived;
	
	// TODO - track acknowledgements

	m_sNetstatsCurrent.mnNumberOfMessagesReceived++;
	m_sNetstatsTotal.mnNumberOfMessagesReceived++;

}


void	NetworkingStatsAddSend( byte* pbMsg, int nBytesSent, BOOL bIsResend )
{
	if ( mbNetworkConnectionPacketLoggingEnabled )
	{
	char	acLogText[400];
	NetworkStatisticsLogAnnotationCallback		fnAnnotationCallback = NetworkConnection::Get().GetAnnotationCallback();

		sprintf( acLogText, "-SEND- Size:%03d | %02x %02x %02x %02x %02x %02x %02x %02x | ", nBytesSent, pbMsg[0], pbMsg[1], pbMsg[2], pbMsg[3], pbMsg[4], pbMsg[5], pbMsg[6], pbMsg[7] );
		if ( fnAnnotationCallback )
		{
		char	acAnnotation[256];
			acAnnotation[0] = 0;
			fnAnnotationCallback( pbMsg, nBytesSent, acAnnotation );
			strcat( acLogText, acAnnotation );
		}
		NetworkConnectionPacketLogAddLine( acLogText );
	}

	m_sNetworkStatistics.mnStatsBytesSent += nBytesSent;

	m_sNetstatsCurrent.mnBytesSent += nBytesSent;
	m_sNetstatsTotal.mnBytesSent += nBytesSent;
	if ( bIsResend == TRUE )
	{
		m_sNetstatsTotal.mnNumberOfResends++;
		m_sNetstatsCurrent.mnNumberOfResends++;
	}
	else
	{
		m_sNetstatsTotal.mnNumberOfMessagesSent++;
		m_sNetstatsCurrent.mnNumberOfMessagesSent++;
	}
}

NETWORK_STATISTICS*		NetworkingGetStatistics( void )
{
ulong	ulTick = SysGetTick();

	if ( (ulTick - mulNewNetstatsLastSampleTick) > 3000 )
	{
		m_sNetstatsRecent = m_sNetstatsCurrent;
		memset( &m_sNetstatsCurrent, 0, sizeof( m_sNetstatsCurrent ) );
		m_sNetstatsCurrent.mulStatsStartTick = ulTick;

		NetworkingStatsUpdate( &m_sNetstatsRecent );
		NetworkingStatsUpdate( &m_sNetstatsTotal );
		mulNewNetstatsLastSampleTick = ulTick;
	}

	if ( (ulTick - m_sNetworkStatistics.mulLastStatsTick) > 2000 )
	{
		m_sNetworkStatistics.mnStatsTotalBytesRecvd += m_sNetworkStatistics.mnStatsBytesRecvd;
		m_sNetworkStatistics.mnStatsTotalBytesSent += m_sNetworkStatistics.mnStatsBytesSent;
		m_sNetworkStatistics.mnLastStatsBytesRecvd = m_sNetworkStatistics.mnStatsBytesRecvd;
		m_sNetworkStatistics.mnLastStatsBytesSent = m_sNetworkStatistics.mnStatsBytesSent;
		m_sNetworkStatistics.mulLastStatsTick = SysGetTick();
		m_sNetworkStatistics.mnStatsBytesSent = 0;
		m_sNetworkStatistics.mnStatsBytesRecvd = 0;
	}
	return( &m_sNetworkStatistics );
}


const char*		NetworkGetIPAddressText( ulong ulIP )
{
#ifdef MARMALADE
static	char	acIPAddressBuffer[64] = "";

	s3eInetNtoa( (s3eInetIPAddress)ulIP, acIPAddressBuffer, 63 );
	return( acIPAddressBuffer );
#else
	return( inet_ntoa( *((struct in_addr*)(&ulIP))) );
#endif
}

ulong			NetworkGetIPAddress( const char* szIPAddressString )
{
#ifdef MARMALADE
char	acNumBuffer[32];
const char*		pcRunner = szIPAddressString;
char*			pcOutRunner = acNumBuffer;
ulong	ulIPAddr = 0;
int		nShift = 0;

	while( *pcRunner != 0 ) 
	{
		if ( *pcRunner == '.' )
		{
			*pcOutRunner = 0;
			ulIPAddr |= ( strtoul( acNumBuffer, NULL, 10 ) << nShift );
			pcOutRunner = acNumBuffer;
			nShift += 8;
			pcRunner++;
		}
		else
		{
			*pcOutRunner++ = *pcRunner++;
		}
	}
	*pcOutRunner = 0;
	ulIPAddr |= ( strtoul( acNumBuffer, NULL, 10 ) << nShift );
	return( ulIPAddr );
#else
	return( (ulong)(inet_addr( szIPAddressString )) );
#endif

}


void NetworkSetMessageScrambleKey( int nKeyLength, byte* pbBuffer )
{
	mnScrambleKeyLength = nKeyLength;
	memcpy( mabScrambleKeyBuffer, pbBuffer, nKeyLength );
}


int NetworkConnectionGenericSendMessage( const char* pcMsg, int nMsgLength, ulong ulIP, ushort uwPort, BOOL bIsResend )
{
	return( NetworkConnection::Get().Send( pcMsg, nMsgLength, ulIP, uwPort, bIsResend, TRUE ) );
}

int NetworkConnectionGenericSendMessageNoScramble( const char* pcMsg, int nMsgLength, unsigned long ulIP, unsigned short uwPort, BOOL bIsResend )
{
	return( NetworkConnection::Get().Send( pcMsg, nMsgLength, ulIP, uwPort, bIsResend, FALSE ) );
}


unsigned long	GetLastGuestIP( void )
{
#ifdef MARMALADE
	return( (ulong)mxLastMessageSourceAddress.m_IPAddress );
#else
	return( *( (ulong*)(&mxLastMessageSourceAddress.sin_addr) ) );
#endif
}

unsigned short	GetLastGuestPort( void )
{
#ifdef MARMALADE
	return( s3eInetNtohs( mxLastMessageSourceAddress.m_Port ) );
#else
	return( ntohs( mxLastMessageSourceAddress.sin_port ) );
#endif
}

#ifndef MARMALADE
struct sockaddr_in*	GetLastGuestAddress( void )
{
	return( &mxLastMessageSourceAddress );
}

void	SetLastGuestAddress( struct sockaddr_in* pAddress )
{
	mxLastMessageSourceAddress = *pAddress;
}

int	NetworkingGetSocket( void )
{
	return( NetworkConnection::Get().GetSocket() );
}

#endif

/***************************************************************************
 * Function    : NetworkConnectionScrambleMessage
 * Params      :
 * Returns     : 
 ***************************************************************************/
void	NetworkConnectionScrambleMessage( byte* pbMsg, int nLen )
{
#ifdef MESSAGE_SCRAMBLING
int		nLoop;
	for ( nLoop = 0; nLoop < nLen; nLoop++ )
	{
		pbMsg[ nLoop ] += mabScrambleKeyBuffer[ (nLoop % mnScrambleKeyLength) ];
	}
#endif
}

void NetworkConnectionUnscrambleMessage( byte* pbMsg, int nLen )
{
#ifdef MESSAGE_SCRAMBLING
int		nLoop;
	for ( nLoop = 0; nLoop < nLen; nLoop++ )
	{
		pbMsg[ nLoop ] -= mabScrambleKeyBuffer[ (nLoop % mnScrambleKeyLength) ];
	}
#endif
}


void	ParseRerouteLine( int nLineNum, char* pcBuff )
{
#ifndef MARMALADE
char*	pcRouteIP1;
char*	pcRouteIP2;
char*	pcRoutePort1;
char*	pcRoutePort2;
char*	pcRouteEnd;
ulong	ulIP;
ushort	wPort;

	pcRouteIP1 = pcBuff;
	pcRoutePort1 = pcRouteIP1;

	while ( ( *(pcRoutePort1) != 0 ) &&
		    ( *(pcRoutePort1) != ':' ) )
	{
		pcRoutePort1++;
	}
	if ( *(pcRoutePort1) == ':' )
	{
		*(pcRoutePort1) = 0;
		pcRoutePort1++;
		pcRouteIP2 = pcRoutePort1;
		while ( ( *(pcRouteIP2) != 0 ) &&
			    ( *(pcRouteIP2) != '=' ) )
		{
			pcRouteIP2++;
		}
		if ( *(pcRouteIP2) == '=' )
		{
			*(pcRouteIP2) = 0;
			pcRouteIP2++;
			pcRoutePort2 = pcRouteIP2;
			while ( ( *(pcRoutePort2) != 0 ) &&
				    ( *(pcRoutePort2) != ':' ) )
			{
				pcRoutePort2++;
			}
			if ( *(pcRoutePort2) == ':' )
			{
				*(pcRoutePort2) = 0;
				pcRoutePort2++;
				pcRouteEnd = pcRoutePort2;
				while ( ( *(pcRouteEnd) != 0 ) &&
					    ( *(pcRouteEnd) != '\r' ) &&
					    ( *(pcRouteEnd) != '\n' ) )
				{
					pcRouteEnd++;
				}
		
				*(pcRouteEnd) = 0;

				if ( mnNumReroutes < MAX_REROUTES )
				{
					ZeroMemory( &maxReRouteAddrSrc[mnNumReroutes], sizeof(maxReRouteAddrSrc[0]) );
					ZeroMemory( &maxReRouteAddrDest[mnNumReroutes], sizeof(maxReRouteAddrDest[0]) );
					ulIP = NetworkGetIPAddress( pcRouteIP1 );
					wPort = (ushort)( strtol( pcRoutePort1, NULL, 10 ) );
					maxReRouteAddrSrc[mnNumReroutes].sin_addr = *( (struct in_addr*)( &ulIP ) );
					maxReRouteAddrSrc[mnNumReroutes].sin_family = AF_INET;
					maxReRouteAddrSrc[mnNumReroutes].sin_port = htons( wPort );

					ulIP = NetworkGetIPAddress( pcRouteIP2 );
					wPort = (ushort)( strtol( pcRoutePort2, NULL, 10 ) );
					maxReRouteAddrDest[mnNumReroutes].sin_addr = *( (struct in_addr*)( &ulIP ) );
					maxReRouteAddrDest[mnNumReroutes].sin_family = AF_INET;
					maxReRouteAddrDest[mnNumReroutes].sin_port = htons( wPort );
					mnNumReroutes++;
				}
			}

		}
	}
#endif
}


void	LoadRerouteText( void )
{
FILE*	pFile;
int		nFileSize;
char	acBuff[256];
int		nBytesRead = 0;
char*	pcReadBuff = acBuff;
int		nLineNum = 0;
int		nBytesInLine = 0;
char*	pcScript;
char*		pcScriptEnd;
char*		pcScriptPos;

	pFile = fopen( "reroute.txt", "rb" );

	if ( pFile != NULL )
	{	
		nFileSize = SysGetFileSize(pFile);

		pcScript = (char*)( SystemMalloc( nFileSize ) );
		fread( pcScript, nFileSize, 1, pFile );
		fclose( pFile );
		pcScriptEnd = pcScript + nFileSize;

		pcScriptPos = pcScript;
		while( pcScriptPos < pcScriptEnd )
		{
			pcScriptPos = ScriptGetNextLine( pcScriptPos, acBuff, pcScriptEnd );
			if ( acBuff[0] != 0 )
			{
				if ( acBuff[0] == '#' )
				{
					// ignore comment lines					
				}
				else
				{
					ParseRerouteLine( nLineNum, acBuff );
				}
			}
			nLineNum++;
		}
		SystemFree( pcScript );		
	}

}


NetworkConnection::NetworkConnection()
{
#ifdef MARMALADE
	m_pUDPSocket = NULL;
#else
	m_hSocket = -1;
#endif
	m_fnIncomingMessageHandler = NULL;
	m_fnLogAnnotationHandler = NULL;
}

NetworkConnection::~NetworkConnection()
{
	ShutdownUDP();

}

NetworkConnection&		NetworkConnection::Get()
{
	return( m_sNetworkConnectionSingleton );
}

SOCKET		m_ListenSocket;
HANDLE		m_hConnectionListenThread;
volatile BOOL		m_bNetworkThreadShutdown = FALSE;
volatile BOOL		m_bNewTCPConnectionsPending = FALSE;


SOCKET		m_pendingNewConnection = 0;		// TODO - will need more of these..


long WINAPI NetworkTCPListenThread(long lParam)
{ 
#ifndef MARMALADE
SOCKET		xNewConnection;
int		ret;
#endif
BOOL	bHasErrored = FALSE;

	do
	{
		// Only process one connection at a time
		if ( m_bNewTCPConnectionsPending == FALSE )
		{
#ifdef MARMALADE
			// TODO - MARMALADE TODO

#else
			ret = TcpAccept(&xNewConnection, m_ListenSocket, 1, NULL);

			// If we've got a new TCP connection comin in
			if ( ret == TCP4U_SUCCESS )
			{
		//		xNewConnection
				m_pendingNewConnection = xNewConnection;
				m_bNewTCPConnectionsPending = TRUE;
//				printf("TCP connection created!\n");
			}

			if ( ( ret != TCP4U_SUCCESS ) &&
				 ( ret != TCP4U_TIMEOUT ) )
			{
				bHasErrored = TRUE;
			}
#endif
		}
		SysSleep( 10 );
		
	} while( bHasErrored == FALSE );

	return( 0 );
}


void	NetworkConnection::UpdateNewTCPConnections( void )
{
#ifndef NO_PEER_CONN
PeerConnection*		pNewConnection;
#endif
ulong	ulIPAddress = 0;

#ifdef MARMALADE
	// MARMALADE TODO	

#else
	char StationName[64];

	TcpGetRemoteID (m_pendingNewConnection, StationName, sizeof StationName, &ulIPAddress);
#endif


#ifndef NO_PEER_CONN
	pNewConnection = ConnectionManager::Get().DecodeNewTCPConnection(ulIPAddress, 0);

	if ( pNewConnection )
	{
		pNewConnection->OnRemoteTCPConnected( m_pendingNewConnection );
	}
	else
	{
		// TODO - 
		// Error getting a PeerConnection class... we should terminte the pendingNewConnection SOCKET here

	}
#else
		// TODO - 
		// we should terminate the pendingNewConnection SOCKET here

#endif
	m_bNewTCPConnectionsPending = FALSE;
	m_pendingNewConnection = 0;

}

void	NetworkConnection::ShutdownTCP( void )
{
#ifdef MARMALADE
	// MARMALADE TODO
#else
	TcpAbort();

	m_bNetworkThreadShutdown = TRUE;
	Sleep(10);

	TcpClose ( &m_ListenSocket ); 
	Tcp4uCleanup();
#endif
}

void	NetworkConnection::InitialiseTCPListener( ushort uwPort, NetworkConnectionMessageReceiveHandler messageHandler, int nMode )
{
#ifdef MARMALADE
		// MARMALADE TODO

#else
int		ret;
ulong	iID;

	Tcp4uInit();

	ret = TcpGetListenSocket (&m_ListenSocket, NULL, &uwPort, 4);
	if ( ret == TCP4U_SUCCESS )
	{
		printf("[NetworkConnection] TCP listen socket open on %d\n", uwPort );
		m_hConnectionListenThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)NetworkTCPListenThread,(LPVOID)(NULL),0,&iID);
	}
#endif
}


BOOL	NetworkConnection::InitialiseUDP( ushort uwPort, NetworkConnectionMessageReceiveHandler messageHandler, int nMode )
{
#ifndef MARMALADE
struct sockaddr_in client_addr;
struct hostent *hp = NULL;
int nRet;
int	nSize;
int	nVal;
ulong	ulBlockingOff;
#endif
ulong	ulTick = SysGetTick();

	mnConnectionMode = nMode;

	m_fnIncomingMessageHandler = messageHandler;

	memset( &m_sNetworkStatistics, 0, sizeof( m_sNetworkStatistics ) );

	memset( &m_sNetstatsTotal, 0, sizeof( m_sNetstatsTotal ) );
	memset( &m_sNetstatsCurrent, 0, sizeof( m_sNetstatsCurrent ) );
	memset( &m_sNetstatsRecent, 0, sizeof( m_sNetstatsRecent ) );
	memset( &m_sNetstatsPeak, 0, sizeof( m_sNetstatsPeak ) );
	m_sNetworkStatistics.mulLastStatsTick = ulTick;
	m_sNetworkStatistics.mulStatsStartTick = ulTick;
	m_sNetstatsTotal.mulStatsStartTick = ulTick;
	m_sNetstatsCurrent.mulStatsStartTick = ulTick;

	ZeroMemory( m_abMsgBuffer, MAXBUFLEN );

#ifdef MARMALADE

	// MARMALADE TODO
	m_pUDPSocket = s3eSocketCreate( S3E_SOCKET_UDP, 0 );

	if ( uwPort == 0 )
	{
		s3eSocketBind( m_pUDPSocket, NULL, 1 );
	}
	else
	{
		// TODO - Bind to a particular port..
	}

#else
	nRet = WSAStartup(0x0101,&ws);
                
	m_hSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if ( m_hSocket == -1)
    {
//		PANIC_IF( TRUE, "UDP Initialisation error : Couldnt bind port" );
		return( FALSE );
    }

	/** Bind the socket to receive and send on any available port and address **/	
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(uwPort);
	client_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_hSocket, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)	
	{
//		PANIC_IF( TRUE, "UDP Initialisation error : Couldnt bind port" );
		closesocket(m_hSocket);
		return( FALSE );
	}

	printf("UDP Socket opened on %d\n", uwPort );

	ulBlockingOff = 1;
	/** Set socket to non-blocking **/
	nRet = ioctlsocket ( m_hSocket, FIONBIO, &ulBlockingOff );

	nSize = sizeof( int );
	getsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)(&nVal), &nSize );

	if ( nVal < 65536 )
	{
		nVal = 65536;
		setsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)(&nVal), nSize );
		getsockopt( m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)(&nVal), &nSize );
	}

	getsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)(&nVal), &nSize );

	if ( nVal < 65536 )
	{
		nVal = 65536;
		setsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)(&nVal), nSize );
		getsockopt( m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)(&nVal), &nSize );
	}

#ifdef NETWORK_UPNP
	// Find out which port we're using
	int nLen = sizeof( client_addr );	
	getsockname ( m_hSocket, (struct sockaddr *)&client_addr, &nLen );
	UPNPSetMyInternalPort( ntohs(client_addr.sin_port) );
#endif
#endif

	LoadRerouteText();

#ifdef SEND_MSG_LOG
	{
	FILE*	pFile;
	char	acString[256];
		pFile = fopen( "Logs\\network.log", "ab" );
		sprintf( acString,"RestartUDP end\r\n" ); 
		fwrite( acString, strlen( acString ), 1, pFile );
		fclose( pFile );
	}
#endif
	return( TRUE );
}


void	NetworkConnection::GetLastMessageSource( ulong* pulIP, ushort* puwPort )
{
	*pulIP = GetLastGuestIP();
	*puwPort = GetLastGuestPort();
}


int		NetworkConnection::HandleSystemMessages( unsigned char* pcMsg )
{
int		nMsgLen = 0;

#ifndef NO_PEER_CONN

	switch ( *(pcMsg) )
	{
	case UDP_PEER_CONNECTION_SYS:
		nMsgLen = NetworkingPeerConnectionReceiveSysMessage( pcMsg );
		ZeroMemory( pcMsg, nMsgLen );
		pcMsg += nMsgLen;	
		break;	
	case UDP_PEER_CONNECTION_GUARANTEED:
		nMsgLen = NetworkingPeerConnectionReceiveGuaranteedMessage( pcMsg );
		ZeroMemory( pcMsg, nMsgLen );
		pcMsg += nMsgLen;	
		break;	
	default:
		break;
	}
#endif

	return( nMsgLen );
}


void	NetworkConnection::Update( float fDeltaTime )
{
int nRet;
#ifndef MARMALADE
int	nAddrLen;
int		nLastError;
#endif
uchar*	pcIncoming;
int		nRead = 0;

#ifdef MARMALADE
	if ( m_pUDPSocket == NULL )
	{
		return;
	}
#else
	if ( m_hSocket == -1 )
	{
		return;
	}
	nAddrLen = sizeof( mxLastMessageSourceAddress );
#endif

	if ( m_bNewTCPConnectionsPending )
	{
		UpdateNewTCPConnections();
	}

	while ( 1 )
	{
#ifdef MARMALADE
		// MARMALADE TODO
		nRet = s3eSocketRecvFrom( m_pUDPSocket, (char*)m_abMsgBuffer, MAXBUFLEN, 0, &mxLastMessageSourceAddress);
		if ( nRet == -1 )
		{
			return;
		}
#else
		// Read the next message
		nRet = recvfrom(m_hSocket,(char*)m_abMsgBuffer,MAXBUFLEN,0,(struct sockaddr *)&mxLastMessageSourceAddress,&nAddrLen);

		// If error 
		if ( nRet == SOCKET_ERROR )
		{
			nLastError = WSAGetLastError();
			// If error is recv buffer empty .. 
			if ( nLastError == WSAEWOULDBLOCK )
			{
				return;
			}
			else
			{
				// TODO - Network disconnect error - socket error
				return;
			}
		}
#endif

		if ( nRet == MAXBUFLEN )
		{
//			PrintConsoleCR( "Full comm buffer warning!", COL_WARNING );
		}

		pcIncoming = m_abMsgBuffer;

#ifndef NO_UDPSESSIONCLIENT
#ifndef SERVER
		// Check for stuff coming through for UDPSessions
		while( *pcIncoming != 0 )
		{
		UDPSESSION_MSG_HEADER*		pxUDPSessionMsgHeader = (UDPSESSION_MSG_HEADER*)( pcIncoming );
		ulong		ulFromIP =  GetLastGuestIP();
		ushort		uwFromPort = GetLastGuestPort();

			nRead = 0;

			switch( pxUDPSessionMsgHeader->uwFixedSystemIdentifier )
			{
			case UDPSESSION_CONNECT_FIXED_SYSTEM_IDENTIFIER:
			case UDPSESSION_GUARANTEED_FIXED_SYSTEM_IDENTIFIER:
			case UDPSESSION_NONGUARANTEED_FIXED_SYSTEM_IDENTIFIER:
			case UDPSESSION_TERMINATE_FIXED_SYSTEM_IDENTIFIER:
				nRead = UDPSessionClientOnUDPMsgReceive( (char*)pcIncoming, ulFromIP, uwFromPort );
				break;
			default:
				nRead = 0;
			}

			if ( nRead != 0 )
			{
				memset( pcIncoming, 0, nRead );
				pcIncoming += nRead;

				if ( *pcIncoming == 0 )
				{
					return;
				}
			}
			else
			{
				break;
			}
		}
#endif
#endif

		NetworkConnectionUnscrambleMessage( pcIncoming, nRet );
		NetworkingStatsAddReceive( (byte*)pcIncoming, nRet + 8, 0 );
		
		while ( *(pcIncoming) != 0 ) 
		{
//			if ( mxGuestAddr.sin_port != ntohs(GetServerStats()->uwPort) )
//			{
//				PrintConsoleCR("Warning - Message received from incorrect port number", COL_WARNING );
//			}

//			mulLastMessageReceived = SysGetTick();
			nRead = HandleSystemMessages( (unsigned char*)pcIncoming );

			if ( ( nRead == 0 ) &&
				 ( m_fnIncomingMessageHandler ) )
			{
				nRead = m_fnIncomingMessageHandler( (unsigned char*)pcIncoming );
			}

			if ( nRead <= 0 )
			{
				memset( m_abMsgBuffer, 0, MAXBUFLEN );
				return;
			}
			else
			{	
/*
				if ( nRead > MAXBUFLEN )
				{
				int		nError = 0;
					nError++;
					printf( "Error %d", nError );
				}
*/
				memset( pcIncoming, 0, nRead );
			}
			pcIncoming += nRead;
		} 

	}


}


void	NetworkConnection::ShutdownUDP( void )
{
#ifdef MARMALADE


#else
	if ( m_hSocket != -1 )
	{
		closesocket(m_hSocket);
		m_hSocket = -1;

		WSACleanup();
	}
#endif

	if ( mpNetworkConnectionPacketLogFile != NULL )
	{
		fclose( mpNetworkConnectionPacketLogFile );
		mpNetworkConnectionPacketLogFile = NULL;
	}
}

#ifdef MARMALADE
int		NetworkConnection::Send( const char* pMsg, int nMsgLength, ulong ulIP, ushort uwPort, BOOL bIsResend, BOOL bScramble )
{
s3eInetAddress		xDestinationAddress;
int		nBytesSent;
char	szBuffer[8192];

	memcpy( szBuffer, pMsg, nMsgLength );
	memset( &xDestinationAddress, 0, sizeof( xDestinationAddress ) );
	xDestinationAddress.m_IPAddress = ulIP;
	xDestinationAddress.m_Port = s3eInetHtons( uwPort );
	
		// MARMALADE TODO 
	NetworkingStatsAddSend( (byte*)(szBuffer), (nMsgLength + 8), FALSE );
	if ( bScramble )
	{
		NetworkConnectionScrambleMessage( (byte*)szBuffer, nMsgLength );
	}

	nBytesSent = s3eSocketSendTo( m_pUDPSocket, (const char*)szBuffer, nMsgLength, 0, &xDestinationAddress );

	if ( bScramble )
	{
		NetworkConnectionUnscrambleMessage( (byte*)szBuffer, nMsgLength );
	}

	return( nBytesSent );
}
#else

int		NetworkConnection::Send( const char* pMsg, int nMsgLength, ulong ulIP, ushort uwPort, BOOL bIsResend, BOOL bScramble )
{
struct sockaddr_in xDestAddress;
int nRet = 0;
int nAddrLength;
//char	acString[256];
struct sockaddr_in		xNewSendAddress;
struct sockaddr_in*		pxSendAddress = (struct sockaddr_in*)( &xDestAddress );

	ZeroMemory(&xDestAddress, sizeof( xDestAddress) );
	xDestAddress.sin_family = AF_INET;
	xDestAddress.sin_port = htons( uwPort );
	xDestAddress.sin_addr = *( (struct in_addr*)( &ulIP ) );
	
	if ( ( m_hSocket != -1 ) &&
		 ( nMsgLength < 8000 ) )
	{
	char	szBuffer[8192];
		memcpy( szBuffer, pMsg, nMsgLength );
		nAddrLength = sizeof( struct sockaddr_in );
		if ( mnNumReroutes > 0 )
		{
			if ( pxSendAddress != NULL )
			{
			int		nLoop;

				for ( nLoop = 0; nLoop < mnNumReroutes; nLoop++ )
				{

					if ( *((ulong*)(&pxSendAddress->sin_addr)) == *((ulong*)(&maxReRouteAddrSrc[nLoop].sin_addr)) )
					{
						if ( ( maxReRouteAddrSrc[nLoop].sin_port == 0 ) ||
							 ( (pxSendAddress->sin_port == maxReRouteAddrSrc[nLoop].sin_port ) || (0 == maxReRouteAddrSrc[nLoop].sin_port ) ) )
						{
							xNewSendAddress = maxReRouteAddrDest[nLoop];

							if ( ( maxReRouteAddrSrc[nLoop].sin_port == 0 ) &&
								 ( maxReRouteAddrDest[nLoop].sin_port == 0 ) )
							{
								xNewSendAddress.sin_port = pxSendAddress->sin_port;
							}
							pxSendAddress = &xNewSendAddress;
						}
					}
				}
			}
		}

		NetworkingStatsAddSend( (byte*)(szBuffer), (nMsgLength + 8), FALSE );
		if ( bScramble )
		{
			NetworkConnectionScrambleMessage( (byte*)szBuffer, nMsgLength );
		}
		if ( pxSendAddress != NULL )
		{
			nRet = sendto( m_hSocket, (const char*)szBuffer, nMsgLength, 0, (struct sockaddr *)pxSendAddress, nAddrLength );

#ifdef SEND_MSG_LOG
			{
			FILE*	pFile;
			char	acString[256];
			char	acBuff[32];
			char	acCharBuff[256];
			int		nLoop;

				pFile = fopen( "Logs\\network.log", "ab" );
				memcpy( acCharBuff, szBuffer, nMsgLength );
				acCharBuff[nMsgLength] = 0;
				sprintf( acString,"sendto using sock %d, len %d [%s]\r\n", m_hSocket, nMsgLength, acCharBuff ); 
				fwrite( acString, strlen( acString ), 1, pFile );
				acString[0] = 0;
				for( nLoop = 0; nLoop < nMsgLength; nLoop++ )
				{
					sprintf( acBuff, "%02x,", (unsigned char)( szBuffer[nLoop] ) );
					strcat( acString, acBuff );
				}
				strcat( acString, "\r\n" );
				fwrite( acString, strlen( acString ), 1, pFile );
				fclose( pFile );
			}
#endif
		}
		else
		{
			nRet = sendto( m_hSocket, (const char*)szBuffer, nMsgLength, 0, (struct sockaddr *)&mxLastMessageSourceAddress, nAddrLength );
		}

		if ( bScramble )
		{
			NetworkConnectionUnscrambleMessage( (byte*)szBuffer, nMsgLength );
		}
	}
	else
	{
#ifdef SEND_MSG_LOG
		{
		FILE*	pFile;
		char	acString[256];
			pFile = fopen( "Logs\\network.log", "ab" );
			sprintf( acString,"sendto using sock %d (not initialised)\r\n", m_hSocket ); 
			fwrite( acString, strlen( acString ), 1, pFile );
			fclose( pFile );
		}
#endif

	}
	return( nRet );
}

#endif		// win32 version


