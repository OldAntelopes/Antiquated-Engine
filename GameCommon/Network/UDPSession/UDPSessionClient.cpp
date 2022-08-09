
#include "StandardDef.h"

#include "../../Console/Console.h"
#include "../Networking.h"

#include "UDPSessionConnection.h"
#include "UDPSessionClient.h"

//#define		SIMULATE_PACKETLOSS

UDPSessionConnection	msTempSingleUDPSessionConnection;

void	UDPSessionClientMessageSend( char* pcMsg, int nMsgLen, ulong ulToIP, ushort uwToPort, int nFlags )
{
#ifdef SIMULATE_PACKETLOSS
	if ( (rand() % 2) == 0 )
	{
		return;
	}
#endif
	NetworkConnectionGenericSendMessageNoScramble( pcMsg, nMsgLen, ulToIP, uwToPort, FALSE );
}

BOOL		UDPSessionClientInitConnection( int nConnectionID, ulong ulHostIP, ushort uwHostPort, const char* szSessionKey, u64 ullUserUID )
{
	UDPSessionRegisterMessageSendFunction( UDPSessionClientMessageSend );

	return( msTempSingleUDPSessionConnection.InitConnectionOnClient( ulHostIP, uwHostPort, szSessionKey, ullUserUID ) );
}

BOOL		UDPSessionClientIsConnected( int nConnectionID )
{
	return( msTempSingleUDPSessionConnection.IsVerified() );
}

BOOL		UDPSessionClientDidError( int nConnectionID )
{
	 // TODO
	return( FALSE );
}

void		UDPSessionClientRegisterMessageHandler( int nConnectionID, UDPSessionConnectionMessageReceiveHandler fnMessageHandler )
{
	msTempSingleUDPSessionConnection.RegisterMessageHandler( fnMessageHandler, (void*)nConnectionID );
}

void		UDPSessionClientSendGuaranteedMessage( int nConnectionID, void* pcMsg, int nMsgLen )
{
	msTempSingleUDPSessionConnection.SendGuaranteedMessage( pcMsg, nMsgLen );
}

void		UDPSessionClientSendNonGuaranteedMessage( int nConnectionID, void* pcMsg, int nMsgLen )
{
	msTempSingleUDPSessionConnection.SendNonGuaranteedMessage( pcMsg, nMsgLen );
}

void		UDPSessionClientShutdown( BOOL bSendDisconnectMsg )
{
	msTempSingleUDPSessionConnection.Disconnect();

}

void		UDPSessionClientUpdate( float delta )
{
	msTempSingleUDPSessionConnection.UpdateConnectionClient( delta );

}


int		UDPSessionClientOnUDPMsgReceive( char* pcMsg, ulong ulFromIP, ushort uwFromPort )
{
UDPSESSION_MSG_HEADER*		pxUDPSessionMsgHeader = (UDPSESSION_MSG_HEADER*)( pcMsg );
UDPSessionConnection*		pFromConnection;
int		nRead = 0;

	// TODO - We'll need to lookup ip and port to identify the from connection when we want to handle
	// multiple UDPSessions
	pFromConnection = &msTempSingleUDPSessionConnection;

	switch( pxUDPSessionMsgHeader->uwFixedSystemIdentifier )
	{
	case UDPSESSION_CONNECT_FIXED_SYSTEM_IDENTIFIER:
		pFromConnection->OnConnectAccept( (UDPSESSION_CONNECT_MESSAGE*) pcMsg );
		ConsolePrint( 0, "UDPSession connect accept" );
		nRead = pxUDPSessionMsgHeader->uwMsgLen;
		break;
	case UDPSESSION_GUARANTEED_FIXED_SYSTEM_IDENTIFIER:
		pFromConnection->OnReceiveGuaranteedMessage( pcMsg );
		nRead = pxUDPSessionMsgHeader->uwMsgLen;
		break;
	case UDPSESSION_NONGUARANTEED_FIXED_SYSTEM_IDENTIFIER:
		pFromConnection->OnReceiveNonGuaranteedMessage( pcMsg );
		nRead = pxUDPSessionMsgHeader->uwMsgLen;
		break;
	case UDPSESSION_TERMINATE_FIXED_SYSTEM_IDENTIFIER:
		pFromConnection->OnReceiveTerminationMessage( pcMsg );
		nRead = pxUDPSessionMsgHeader->uwMsgLen;
		break;
	}

	return( nRead );
}
