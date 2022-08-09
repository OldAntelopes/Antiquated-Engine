

#include "StandardDef.h"
#include "Networking.h"

#include "ClientConnectionToChatService.h"

#define UDP_CHATSERVER_CONNECT	0xAF

int		mnStandardNetClientScrambleKeyLength = 16;
//byte	mabStandardAtracScrambleKey[16] = { 128, 190, 110, 94, 32, 150, 77, 250, 33, 15, 88, 139, 201,60,234, 171 };
byte	mabStandardNetClientScrambleKey[16] = { 63, 117, 43, 155, 11, 94, 172, 213, 35, 241, 78, 124, 203,163,57, 141 };

ulong	mulNetClientLastChatServerConnectTime = 0;
ulong	mulNetClientLastChatServerConnectRestartTime = 0;


//----------------------------------------------------------------------------
//  ClientGetNewPeerConnection
//
//  Called when the Network library initiates a new 'PeerConnection'
//  This function determines what to do with it...
//----------------------------------------------------------------------------
void*	ClientGetNewPeerConnection( BYTE bConnectionCode, ulong ulIP, ushort uwPort,BYTE bParamCode )
{
	if ( bConnectionCode == PEERCONNECTION_BASIC )
	{
	ClientConnectionToChatService*	pClientConnectionToChatServer = ClientConnectionToChatService::Get();
//	ClientConnectionToAuthServer*		pClientConnectionToAuthServer = ClientConnectionToAuthServer::Get();
	
//		if ( pClientConnectionToAuthServer->IsAuthServerAddress( ulIP, uwPort ) == TRUE )
//		{
//			MainDisplaySetHeadlineText( "Auth connecting" );
//			return( pClientConnectionToAuthServer );
//		}
//		else
//		{
//			MainDisplaySetHeadlineText( "NCS connecting");
			return( pClientConnectionToChatServer );
//		}
	}
	return( NULL );
}


int	DecodeUDPMessagePacket( byte* pcIncoming )
{
int		nLen = 0;

	// All non-guaranteed messages are handled by the Networking system, so we just null the message and ignore it..
	ZeroMemory( pcIncoming, 256 );

	return( nLen );
}



//--------------------------------------------------------------------------------------------
void	NetClientUpdate( float fDelta )
{
ClientConnectionToChatService*		pChatServerConnection = ClientConnectionToChatService::Get();

	Uni::Networking::Update( fDelta );

	if ( pChatServerConnection )
	{
		switch( pChatServerConnection->GetState() )
		{
		case ClientConnectionToChatService::CONNECTION_REJECTED:
			if ( pChatServerConnection->GetLoginFailReason() == ClientConnectionToChatService::AUTHENTICATION_ERROR )
			{
//				if ( mulLastAuthConnectTime < SysGetTimeLong() - (8*60*60) )
//				{
					ClientConnectionToChatService::Destroy();
//					UniNetClientStartConnection();
//				}
			}
			break;
		case ClientConnectionToChatService::DISCONNECTED:
			if ( pChatServerConnection->IsConnected() == FALSE )
			{
				// If we're disconnected and not updating the chat server connection the restart it if we haven't attempted to do so in last 15 seconds..
				if ( mulNetClientLastChatServerConnectTime < SysGetTimeLong() - 15 )
				{
					ClientConnectionToChatService::Destroy();
					// Give it a couple of seconds before restarting in case we're clearing up a dead peerConn
					mulNetClientLastChatServerConnectRestartTime = SysGetTimeLong() + 3;
				}
			}
			break;
		}
	}
	else if ( mulNetClientLastChatServerConnectRestartTime != 0 )
	{
		if ( SysGetTimeLong() >= mulNetClientLastChatServerConnectRestartTime )
		{
			ClientConnectionToChatService::Create( NULL, 0 );
			mulNetClientLastChatServerConnectTime = SysGetTimeLong();
			mulNetClientLastChatServerConnectRestartTime = 0;
		}
	}
}


void	NetClientInit( void )
{

	// Initialise networking lib
	Uni::NetworkingInitParams	xInitParams;
	xInitParams.m_bUseNetworkConnection = TRUE;
	xInitParams.m_fnNewConnectionCallback = ClientGetNewPeerConnection;
	xInitParams.m_fnNetworkConnectionMessageHandler = DecodeUDPMessagePacket;
	Uni::Networking::Initialise( &xInitParams);

	NetworkSetMessageScrambleKey( mnStandardNetClientScrambleKeyLength, mabStandardNetClientScrambleKey );


}

void	NetClientShutdown( void )
{
	if ( ClientConnectionToChatService::Get() )
	{
		ClientConnectionToChatService::Get()->CloseConnection(TRUE);
		ClientConnectionToChatService::Destroy();
	}

	Uni::Networking::Shutdown();
}