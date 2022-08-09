
#include <stdio.h>

#include <UnivSocket.h>
#include <StandardDef.h>

#include "../Networking.h"

#include "ConnectionManager.h"
#include "PeerStream.h"

#define	UDP_PEER_CONNECTION_SYS			0xB6

BOOL	msbConnectionManagerShutdownMutex = FALSE;



ConnectionManager::ConnectionManager()
{
	mpConnectionList = NULL;
	mpfnNewConnectionCallback = NULL;
}

ConnectionManager::~ConnectionManager()
{
PeerConnectionList*		pListEntry = mpConnectionList;
PeerConnectionList*		pNext;

	msbConnectionManagerShutdownMutex = TRUE;
	while( pListEntry )
	{
		pNext = pListEntry->mpNext;
		if ( pListEntry->mpConnection )
		{
			delete( pListEntry->mpConnection );
		}
		delete pListEntry;
		pListEntry = pNext;
	}
}

ConnectionManager&	ConnectionManager::Get()
{
static	ConnectionManager		m_singleton;
	return( m_singleton );
}

//-----------------------------------------------------------------------------------------------------
// ConnectionManager::AddConnection
//
//
//-----------------------------------------------------------------------------------------------------
void	ConnectionManager::AddConnection( PeerConnection* pConnection )
{
PeerConnectionList*		pListEntry;

	// Make sure the connection doesnt get duplicated in this list, by attempting to remove it first
	RemoveConnection( pConnection );

	pListEntry = new PeerConnectionList;
	pListEntry->mpConnection = pConnection;
	pListEntry->mpNext = mpConnectionList;
	mpConnectionList = pListEntry;
}

//-----------------------------------------------------------------------------------------------------
// ConnectionManager::RemoveConnection
//
//
//-----------------------------------------------------------------------------------------------------
void	ConnectionManager::RemoveConnection( PeerConnection* pConnection )
{
PeerConnectionList*		pListEntry = mpConnectionList;
PeerConnectionList*		pPrev = NULL;

	if ( msbConnectionManagerShutdownMutex ) return;

	while( pListEntry )
	{
		if ( pListEntry->mpConnection == pConnection )
		{
			if ( pPrev )
			{
				pPrev->mpNext = pListEntry->mpNext;
			}
			else
			{
				mpConnectionList = pListEntry->mpNext;
			}
			delete pListEntry;
			return;
		}
		pPrev = pListEntry;
		pListEntry = pListEntry->mpNext;
	}
}


//-----------------------------------------------------------------------------------------------------
// ConnectionManager::FindMessageSource
//
//
//-----------------------------------------------------------------------------------------------------
PeerConnection*		ConnectionManager::FindMessageSource( void )
{
PeerConnectionList*		pListEntry = mpConnectionList;
ulong	ulIP = GetLastGuestIP();
ushort	uwPort = GetLastGuestPort();

	while( pListEntry )
	{
		if ( pListEntry->mpConnection )
		{
			if ( ( pListEntry->mpConnection->m_ulIP == ulIP ) &&
				 ( pListEntry->mpConnection->m_uwPort == uwPort ) )
			{
				return( pListEntry->mpConnection );
			}
		}
		pListEntry = pListEntry->mpNext;
	}
	return( NULL );
}


ulong		msulConnectionManagerLastBounceIP = 0;
ushort		msuwConnectionManagerLastBouncePort = 0;
ulong		msulConnectionManagerLastBounceTime = 0;

//-----------------------------------------------------------------------------------------------------
// ConnectionManager::ReceiveMessage
//
//
//-----------------------------------------------------------------------------------------------------
int		ConnectionManager::ReceiveMessage( byte* pbMsg )
{
PeerConnection*		pConnection = FindMessageSource();

	if ( pConnection )
	{
		return( pConnection->DecodeGuaranteedMessageUDP( pbMsg ) );
	}
	else
	{
	PEER_STATUS_MSG		xReply;

		memset( &xReply, 0, sizeof( xReply ) );
		xReply.bUDPCode = UDP_PEER_CONNECTION_SYS;
		xReply.bMsgCode = PEER_SYSMSG_CLOSECONNECTION;
		xReply.bSizeOfMsg = sizeof( xReply );
		xReply.bParamCode = 2;
		xReply.bConnectionCode = 0;
		xReply.ulLocalTimer = SysGetTick();
		xReply.ulResponseTimer = 0;

		// If this didnt come from the same IP and port as the last bounce, or we last responded to the bounce over 5 seconds ago
		if ( ( GetLastGuestIP() != msulConnectionManagerLastBounceIP ) ||
			 ( GetLastGuestPort() != msuwConnectionManagerLastBouncePort ) ||
			 ( msulConnectionManagerLastBounceTime < SysGetTimeLong() - 5 ) )
		{
			msulConnectionManagerLastBounceIP = GetLastGuestIP();
			msuwConnectionManagerLastBouncePort = GetLastGuestPort();

			NetworkConnectionGenericSendMessage( (char*)( &xReply ), xReply.bSizeOfMsg, msulConnectionManagerLastBounceIP, msuwConnectionManagerLastBouncePort, FALSE );
			msulConnectionManagerLastBounceTime = SysGetTimeLong();
			NetworkingUserDebugPrint( 0, "** Bouncing PeerGuar from unknown source" );
		}
		else
		{
//			mnConnectionManagerBounceIgnoreCount++;
		}

	}
	return( PeerConnection::GetMsgLen( pbMsg ) );
}



//-----------------------------------------------------------------------------------------------------
// ConnectionManager::ReceiveSysMessage
//   Called when a UDP_PEER_CONNECTION_SYS packet is received
//
//-----------------------------------------------------------------------------------------------------
int		ConnectionManager::ReceiveSysMessage( byte* pbMsg )
{
short	wMsgSize = 1;
PeerConnection*		pConnection = FindMessageSource();

//	NetworkingUserDebugPrint( 0, "Received sys message %d", pbMsg[1] );

	// Guaranteed init messages are handled by the mpfnNewConnectionCallback function
	if( pbMsg[1] == PEER_SYSMSG_GUARANTEEDINIT )
	{
	PEER_STATUS_MSG* pxMsg = (PEER_STATUS_MSG*)( pbMsg );

		wMsgSize = (short)( pbMsg[2] );
		// Create a new connection to this peer?
		DecodeNewConnection( pxMsg );
	}
	else if ( pbMsg[1] == PEER_SYSMSG_CLOSECONNECTION )
	{
		wMsgSize = (short)( pbMsg[2] );
		if ( pConnection )
		{
			pConnection->CloseConnection(FALSE);
			RemoveConnection( pConnection );	// mm
		}
	}		
	else if ( pConnection )
	{
		wMsgSize = pConnection->DecodeBasicMessage( pbMsg );
	}

	return( wMsgSize );
}

//-----------------------------------------------------------------------------------------------------
// ConnectionManager::UpdateAllConnections
//
//
//-----------------------------------------------------------------------------------------------------
void	ConnectionManager::UpdateAllConnections( float fDeltaTime )
{
PeerConnectionList*		pListEntry = mpConnectionList;

	while( pListEntry )
	{
		if ( pListEntry->mpConnection )
		{
			pListEntry->mpConnection->UpdateConnection( fDeltaTime );
		}
		pListEntry = pListEntry->mpNext;
	}

}


//-----------------------------------------------------------------------------------------------------
// ConnectionManager::CloseAllConnections
//
//
//-----------------------------------------------------------------------------------------------------
void	ConnectionManager::CloseAllConnections( void )
{
PeerConnectionList*		pListEntry = mpConnectionList;

	while( pListEntry )
	{
		if ( pListEntry->mpConnection )
		{
			pListEntry->mpConnection->CloseConnection();
		}
		pListEntry = pListEntry->mpNext;
	}
}



//-----------------------------------------------------------------------------------------------------
// ConnectionManager::DecodeNewTCPConnection
//
//
//-----------------------------------------------------------------------------------------------------
PeerConnection*		ConnectionManager::DecodeNewTCPConnection( ulong ulFromIP, ushort uwFromPort )
{ 
PeerConnection*		pNewConnection = NULL;

	if ( mpfnNewConnectionCallback )
	{
		// pxMsg->bParamCOde = 0  Received when a remote client does 'InitialiseGuaranteed' - it asks us to form a PeerConnection to the remote user
		// pxMsg->bParamCOde = 1  Received in response to requesting a new connection.. Means the connection is active
		pNewConnection = (PeerConnection*)mpfnNewConnectionCallback( 0xFF, ulFromIP, 0, 0 );
	}
	return( pNewConnection );
}


//-----------------------------------------------------------------------------------------------------
// ConnectionManager::DecodeNewConnection
//
//
//-----------------------------------------------------------------------------------------------------
void		ConnectionManager::DecodeNewConnection( PEER_STATUS_MSG* pxMsg )
{
PeerConnection*		pNewConnection;
ulong	ulIP = GetLastGuestIP();
ushort	uwPort = GetLastGuestPort();

	if ( mpfnNewConnectionCallback )
	{
		// pxMsg->bParamCOde = 0  Received when a remote client does 'InitialiseGuaranteed' - it asks us to form a PeerConnection to the remote user
		// pxMsg->bParamCOde = 1  Received in response to requesting a new connection.. Means the connection is active
		pNewConnection = (PeerConnection*)mpfnNewConnectionCallback( pxMsg->bConnectionCode, ulIP, uwPort, pxMsg->bParamCode );

//		pNewConnection->Reset();

		if ( pNewConnection )
		{
			switch ( pxMsg->bParamCode )
			{
			case 0:
				pNewConnection->RemoteInitialiseGuaranteedUDP( ulIP, uwPort, pxMsg->ulLocalTimer );
				break;
			case 1:
				pNewConnection->RemoteInitResponseUDP( ulIP, uwPort, pxMsg->ulLocalTimer, pxMsg->ulResponseTimer );
				break;
			}
		}
	}
	else
	{
		// Error ...
	}
}