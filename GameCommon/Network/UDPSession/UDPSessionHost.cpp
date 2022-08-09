
#include <UnivSocket.h>
#include "StandardDef.h"

#include "UDPSessionConnection.h"
#include "UDPSessionConnectionMessages.h"
#include "UDPSessionHostConnectionManager.h"
#include "UDPSessionHost.h"

#define		MAX_UDP_PACKET_SIZE		8192
//#define		SIMULATE_PACKETLOSS

int						m_hUDPSessionHostSocket = -1;
byte					m_abMsgBuffer[MAX_UDP_PACKET_SIZE];
struct	sockaddr_in		mxUDPSessionHostLastMessageSourceAddress;

#ifdef WIN32
WSADATA mUDPSessionHostWSAData;
#endif

UDPSessionConnectionMessageReceiveHandler		mfnUDPSessionHostMessageHandler = NULL;
void*											mpUDPSessionUserData = NULL;

void	UDPSessionHostMessageSend( char* pcMsg, int nMsgLen, ulong ulToIP, ushort uwToPort, int nFlags )
{
struct sockaddr_in xDestAddress;
int nRet = 0;
int nAddrLength;
struct sockaddr_in*		pxSendAddress = (struct sockaddr_in*)( &xDestAddress );

	ZeroMemory(&xDestAddress, sizeof( xDestAddress) );
	xDestAddress.sin_family = AF_INET;
	xDestAddress.sin_port = htons( uwToPort );
	xDestAddress.sin_addr = *( (struct in_addr*)( &ulToIP ) );
	
	if ( ( m_hUDPSessionHostSocket != -1 ) &&
		 ( nMsgLen < 8000 ) )
	{
		nAddrLength = sizeof( struct sockaddr_in );

#ifdef SIMULATE_PACKETLOSS
		if ( (rand() % 2) == 0 )
		{
			return;
		}
#endif
		nRet = sendto( m_hUDPSessionHostSocket, (const char*)pcMsg, nMsgLen, 0, (struct sockaddr *)pxSendAddress, nAddrLength );
	}
}


BOOL	UDPSessionHostCreateSocket( ushort uwHostPort )
{
struct sockaddr_in client_addr;
struct hostent *hp = NULL;
int nRet;
ulong	ulBlockingOff;
ulong	ulTick = SysGetTick();

	ZeroMemory( m_abMsgBuffer, MAX_UDP_PACKET_SIZE );

	nRet = WSAStartup(0x0101,&mUDPSessionHostWSAData);
                
	m_hUDPSessionHostSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if ( m_hUDPSessionHostSocket == -1)
    {
		return( FALSE );
    }

	/** Bind the socket to receive and send on any available port and address **/	
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(uwHostPort);
	client_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_hUDPSessionHostSocket, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1)	
	{
		closesocket(m_hUDPSessionHostSocket);
		return( FALSE );
	}

	SysDebugPrint( "UDP Socket opened on %d\n", uwHostPort );

	ulBlockingOff = 1;
	// Set socket to non-blocking 
	nRet = ioctlsocket ( m_hUDPSessionHostSocket, FIONBIO, &ulBlockingOff );

	return( TRUE );
}


void	UDPSessionHostUpdateConnection( float fDeltaTime )
{
int nRet;
uchar*	pcIncoming;
int		nRead = 0;
int		nAddrLen;
int		nLastError;
UDPSessionConnection*		pFromConnection;
ulong	ulFromIP;
ushort	uwFromPort;

	if ( m_hUDPSessionHostSocket == -1 )
	{
		return;
	}
	nAddrLen = sizeof( mxUDPSessionHostLastMessageSourceAddress );

	while ( 1 )
	{
		// Read the next message
		nRet = recvfrom(m_hUDPSessionHostSocket,(char*)m_abMsgBuffer,MAX_UDP_PACKET_SIZE,0,(struct sockaddr *)&mxUDPSessionHostLastMessageSourceAddress,&nAddrLen);

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
				SysUserPrint( 0, "Socket error %d", nLastError );
				return;
			}
		}

		if ( nRet == MAX_UDP_PACKET_SIZE )
		{
			SysUserPrint( 0, "ERROR: Full comm buffer" );
		}

		pcIncoming = m_abMsgBuffer;
	
		while ( *(pcIncoming) != 0 ) 
		{
			ulFromIP = *( (ulong*)(&mxUDPSessionHostLastMessageSourceAddress.sin_addr) );
			uwFromPort = ntohs( mxUDPSessionHostLastMessageSourceAddress.sin_port );

			// First check to see if  mxUDPSessionHostLastMessageSourceAddress  is from a known connection
			pFromConnection = UDPSessionHostConnectionManagerGetConnection( ulFromIP, uwFromPort );

			//  (If not, only start-connection type messages should be accepted, everything else ignored)
			if ( pFromConnection == NULL )
			{
				if ( nRet == sizeof( UDPSESSION_CONNECT_MESSAGE ) )
				{
				UDPSESSION_CONNECT_MESSAGE*		pxConnectMsg = (UDPSESSION_CONNECT_MESSAGE*)( m_abMsgBuffer );

					if ( pxConnectMsg->xMsgHeader.uwFixedSystemIdentifier == UDPSESSION_CONNECT_FIXED_SYSTEM_IDENTIFIER )
					{
						// TODO - Check protocol version, parse sessionkey using encrypted key set etc

						SysUserPrint( 0, "Creating new connection for %s:%d (UID: %ld)", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort, pxConnectMsg->ullUserUID );
						pFromConnection = UDPSessionHostConnectionManagerCreateNew( ulFromIP, uwFromPort, pxConnectMsg->acSessionKey, pxConnectMsg->ullUserUID );

						pFromConnection->RegisterMessageHandler( mfnUDPSessionHostMessageHandler, mpUDPSessionUserData );
					}
					else
					{
						SysUserPrint( 0, "Invalid sys identifier from unregistered source %s:%d", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
					}
					nRead = sizeof( UDPSESSION_CONNECT_MESSAGE );
				}
				else
				{
				UDPSESSION_MSG_HEADER*		pxMsgHeader = (UDPSESSION_MSG_HEADER*)( m_abMsgBuffer );
					
					if ( pxMsgHeader->uwFixedSystemIdentifier == UDPSESSION_TERMINATE_FIXED_SYSTEM_IDENTIFIER )
					{
						// Unknown source has asked to terminate.. thats fine.. we've probably already done so..
					}
					else
					{
						SysUserPrint( 0, "Ignored invalid message from unregistered source %s:%d", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
					}
				}
			}
			else if ( pFromConnection->HasDisconnected() == TRUE )
			{
			UDPSESSION_MSG_HEADER*		pxMsgHeader = (UDPSESSION_MSG_HEADER*)( m_abMsgBuffer );

				if ( pxMsgHeader->uwFixedSystemIdentifier == UDPSESSION_TERMINATE_FIXED_SYSTEM_IDENTIFIER )
				{
					// They've terminated too, good
					SysUserPrint( 0, "Termination confirmation from source %s:%d", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
				}
				else
				{
					SysUserPrint( 0, "Ignored message from disconnected %s:%d", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
					pFromConnection->ResendTermination();
				}
			}
			else if ( pFromConnection->IsPending() == TRUE )
			{
			UDPSESSION_MSG_HEADER*		pxMsgHeader = (UDPSESSION_MSG_HEADER*)( m_abMsgBuffer );

				if ( pxMsgHeader->uwFixedSystemIdentifier == UDPSESSION_TERMINATE_FIXED_SYSTEM_IDENTIFIER )
				{
					SysUserPrint( 0, "Pending connection %s:%d terminated from source", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
					pFromConnection->Disconnect();
				}
				else
				{
					SysUserPrint( 0, "Ignored message from %s:%d as connection pending", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
				}
			}
			else if ( pFromConnection->IsVerified() == TRUE )
			{
			UDPSESSION_MSG_HEADER*		pxMsgHeader = (UDPSESSION_MSG_HEADER*)( m_abMsgBuffer );
			
				// Check here to see if we've got a valid packet
				switch( pxMsgHeader->uwFixedSystemIdentifier )
				{
				case UDPSESSION_TERMINATE_FIXED_SYSTEM_IDENTIFIER:
					SysUserPrint( 0, "Terminated by source (%s:%d)", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
					pFromConnection->Disconnect();
					break;
				case UDPSESSION_CONNECT_FIXED_SYSTEM_IDENTIFIER:
					SysUserPrint( 0, "Ignored duplicate connect message from %s:%d on valid connection", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
			
					// TODO - Should send em another connect accept response in case they lost the first one

					nRead = sizeof( UDPSESSION_CONNECT_MESSAGE );
					break;
				case UDPSESSION_CLIENTGUARREQUEST_FIXED_SYSTEM_IDENTIFIER:
					pFromConnection->OnClientRequestGuarUpdate( (const char*)m_abMsgBuffer );
					nRead = pxMsgHeader->uwMsgLen;
					break;
				case UDPSESSION_GUARANTEED_FIXED_SYSTEM_IDENTIFIER:
					pFromConnection->OnReceiveGuaranteedMessage( (const char*)m_abMsgBuffer );
					nRead = pxMsgHeader->uwMsgLen;
					break;
				case UDPSESSION_NONGUARANTEED_FIXED_SYSTEM_IDENTIFIER:
					pFromConnection->OnReceiveNonGuaranteedMessage( (const char*)m_abMsgBuffer );
					nRead = pxMsgHeader->uwMsgLen;
					break;
				default:
					SysUserPrint( 0, "Invalid sys identifier from valid source %s:%d", inet_ntoa( *(struct in_addr*)( &ulFromIP ) ), uwFromPort );
					break;
				}			
			}

			if ( nRead > 0 )
			{
				memset( pcIncoming, 0, nRead );
			}
			else
			{
				memset( m_abMsgBuffer, 0, MAX_UDP_PACKET_SIZE );
				return;
			}
			pcIncoming += nRead;
		} 
	}

}


//------------------------------------------------------------------------------------

int		UDPSessionHostInitialise( ushort uwHostPort, const char* szLoginServiceURL  )
{

	UDPSessionRegisterMessageSendFunction( UDPSessionHostMessageSend );

	UDPSessionHostConnectionManagerInitialise( szLoginServiceURL );

	UDPSessionHostCreateSocket( uwHostPort );

	return( 1 );
}


void		UDPSessionHostUpdate( float delta )
{
	UDPSessionHostUpdateConnection( delta );

	UDPSessionHostConnectionManagerUpdate( delta );
}


void		UDPSessionHostRegisterMessageHandler( UDPSessionConnectionMessageReceiveHandler fnMessageHandler )
{
	mfnUDPSessionHostMessageHandler = fnMessageHandler;

}
