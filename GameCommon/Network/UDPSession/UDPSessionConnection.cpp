
#include <stdio.h>
#include "StandardDef.h"

#include "../Networking.h"

#include "UDPSessionConnection.h"
#include "../HostConnectionToLoginService.h"

#define	NO_COMMUNICATION_TIMEOUT_MS		12000			// 12 sec

#define DEBUG_GUAR


UDPSendMessageFunction		mfnUDPSessionMessageSendFunc = NULL;

void		UDPSessionRegisterMessageSendFunction( UDPSendMessageFunction fnUDPMessageSend )
{
	mfnUDPSessionMessageSendFunc = fnUDPMessageSend;
}


UDPSessionConnection::UDPSessionConnection()
{
	mpNext = NULL;
	m_bIsVerifying = FALSE;
	m_bHasVerified = FALSE;
	m_bIsHost = FALSE;
	m_ulIP = 0;
	m_uwPort = 0;
	m_uwNextSendGuarID = 100;
	m_uwLastRecvGuarID = 100;
	memset( m_abScrambleKey, 0, 24 );
	mnScrambleKeyLength = 0;
	mpGuaranteedMessageSendBuffer = NULL;
	m_uwGuarIDsReceivedUpTo = 99;
	mpOutOfOrderReceipts = NULL;
	mpFreeOutOfOrderReceipts = NULL;
	m_bHasInitialisedBuffers = FALSE;
	mfnMessageHandler = NULL;
	mpMessageHandlerUserParam = NULL;
	m_bHasDisconnected = FALSE;
}

UDPSessionConnection::~UDPSessionConnection()
{
	// TODO!!

}

BOOL	UDPSessionConnection::IsConnected( void )
{
	return( IsVerified() );
}

void		UDPSessionConnection::WriteMsgHeader( UDPSESSION_MSG_HEADER* pxHeader )
{
	memset( pxHeader, 0, sizeof( UDPSESSION_MSG_HEADER ) );

	pxHeader->bConnectionVersion = 1;
	pxHeader->bEncryptKeySet = 1;
	pxHeader->bProtocolVersion = 1;
}


void	UDPSessionConnection::ScrambleMessage( BYTE* pbMsg, int nLen, ushort uwTimeStamp )
{
int		nLoop;
BYTE	bExtra = (BYTE)( uwTimeStamp );

	for ( nLoop = 0; nLoop < 4; nLoop++ )
	{
		pbMsg[ nLoop ] += m_abScrambleKey[ (nLoop % mnScrambleKeyLength) ];
	}

	for ( nLoop = 4; nLoop < nLen; nLoop++ )
	{
		pbMsg[ nLoop ] += m_abScrambleKey[ (nLoop % mnScrambleKeyLength) ] + bExtra;
	}
}

void UDPSessionConnection::UnscrambleMessage( BYTE* pbMsg, int nLen )
{
int		nLoop;
ushort	uwTimestamp;
BYTE	bExtra;

	for ( nLoop = 0; nLoop < 4; nLoop++ )
	{
		pbMsg[ nLoop ] -= m_abScrambleKey[ (nLoop % mnScrambleKeyLength) ];
	}
	uwTimestamp = *( (ushort*)( pbMsg + 2 ) );
	bExtra = (BYTE)( uwTimestamp );
	for ( nLoop = 4; nLoop < nLen; nLoop++ )
	{
		pbMsg[ nLoop ] -= m_abScrambleKey[ (nLoop % mnScrambleKeyLength) ] + bExtra;
	}
}

void		UDPSessionConnection::UDPSessionGuaranteedMessage::Send( UDPSessionConnection* pConnection )
{
BYTE						abMsgBuffer[8192];
UDPSESSION_MSG_HEADER*		pxHeader = (UDPSESSION_MSG_HEADER*)( abMsgBuffer );
UDPSESSION_SCRAMBLED_MSG_HEADER*		pxScrambleHeader;
UDPSESSION_GUARANTEED_MSG_HEADER*		pxGuarHeader;

	mulLastSendTimestamp = SysGetTick();

	pConnection->WriteMsgHeader( pxHeader );
	pxHeader->uwFixedSystemIdentifier = UDPSESSION_GUARANTEED_FIXED_SYSTEM_IDENTIFIER;
	pxHeader->uwMsgLen = (ushort)( mnMsgLen ) + sizeof( UDPSESSION_SCRAMBLED_MSG_HEADER ) + sizeof( UDPSESSION_MSG_HEADER ) + sizeof( UDPSESSION_GUARANTEED_MSG_HEADER );

	pxScrambleHeader = (UDPSESSION_SCRAMBLED_MSG_HEADER*)( pxHeader + 1 );
	pxScrambleHeader->uwSystemValidationIdentifier = pConnection->m_uwSystemValidationIdentifier;
	pxScrambleHeader->uwTimestamp = (ushort)SysGetTick();

	pxGuarHeader = (UDPSESSION_GUARANTEED_MSG_HEADER*)( pxScrambleHeader + 1 );

	pxGuarHeader->bIsReceiptFlag = 0;
	pxGuarHeader->bPad1 = 0;
	pxGuarHeader->uwGuarID = m_uwGuarID;

	memcpy( (void*)(pxGuarHeader + 1), mpcMsg, mnMsgLen );
	
	pConnection->ScrambleMessage( (BYTE*)( pxScrambleHeader ), pxHeader->uwMsgLen - sizeof(UDPSESSION_MSG_HEADER), pxScrambleHeader->uwTimestamp );
	mfnUDPSessionMessageSendFunc( (char*)abMsgBuffer, pxHeader->uwMsgLen, pConnection->m_ulIP, pConnection->m_uwPort, 0 );
	
}


void		UDPSessionConnection::SendGuaranteedMessage( void* pcMsg, int nMsgLen )
{
	if ( m_bHasVerified )
	{
	UDPSessionGuaranteedMessage*		pNewGuaranteedMessage = new UDPSessionGuaranteedMessage;

		pNewGuaranteedMessage->mpNext = mpGuaranteedMessageSendBuffer;
		mpGuaranteedMessageSendBuffer = pNewGuaranteedMessage;

		pNewGuaranteedMessage->mnMsgLen = nMsgLen;
		pNewGuaranteedMessage->m_uwGuarID = m_uwNextSendGuarID++;
		pNewGuaranteedMessage->mpcMsg = (char*)malloc( nMsgLen );
		memcpy( pNewGuaranteedMessage->mpcMsg, pcMsg, nMsgLen );
		pNewGuaranteedMessage->mnNumRepeatSends = 0;
	
		pNewGuaranteedMessage->Send( this );
	}
}


void		UDPSessionConnection::SendNonGuaranteedMessage( void* pcMsg, int nMsgLen )
{
	if ( m_bHasVerified )
	{
	BYTE						abMsgBuffer[8192];
	UDPSESSION_MSG_HEADER*		pxHeader = (UDPSESSION_MSG_HEADER*)( abMsgBuffer );
	UDPSESSION_SCRAMBLED_MSG_HEADER*		pxScrambleHeader;

		WriteMsgHeader( pxHeader );
		pxHeader->uwFixedSystemIdentifier = UDPSESSION_NONGUARANTEED_FIXED_SYSTEM_IDENTIFIER;
		pxHeader->uwMsgLen = (ushort)( nMsgLen ) + sizeof( UDPSESSION_SCRAMBLED_MSG_HEADER ) + sizeof( UDPSESSION_MSG_HEADER );

		pxScrambleHeader = (UDPSESSION_SCRAMBLED_MSG_HEADER*)( pxHeader + 1 );
		pxScrambleHeader->uwSystemValidationIdentifier = m_uwSystemValidationIdentifier;
		pxScrambleHeader->uwTimestamp = (ushort)( SysGetTick() );

		memcpy( (void*)(pxScrambleHeader + 1), pcMsg, nMsgLen );

		ScrambleMessage( (BYTE*)( pxScrambleHeader ), pxHeader->uwMsgLen - sizeof(UDPSESSION_MSG_HEADER), pxScrambleHeader->uwTimestamp  );
		mfnUDPSessionMessageSendFunc( (char*)abMsgBuffer, pxHeader->uwMsgLen, m_ulIP, m_uwPort, 0 );
	}
}


void	UDPSessionConnection::OnConnectAccept( UDPSESSION_CONNECT_MESSAGE* pxConnectAcceptMsg )
{
int		nLoop;

	if ( mnScrambleKeyLength == 0 )
	{
		// TODO - We should blowfish encrypt the generated scramblekey in this response using the private key
	
		mnScrambleKeyLength = 24;
		for( nLoop = 0; nLoop < mnScrambleKeyLength; nLoop++ )
		{
			m_abScrambleKey[nLoop] = (BYTE)( pxConnectAcceptMsg->acSessionKey[nLoop] );
		}

		m_uwSystemValidationIdentifier = pxConnectAcceptMsg->uwSystemValidationIdentifier;
	}

	ClearOutboundMessage();
	m_bIsVerifying = FALSE;
	m_bHasVerified = TRUE;
}

//--------------------------------------------------------------
// OnSessionValidated
// Called on host when the sessionKey has been validated with the loginservice.
//   (Note may get triggered multiple times if the client re-requests the connection while still active - e.g. if the (non-guaranteed) connect accept response doesnt arrive back)
//---------------------------------------------------------------
void		UDPSessionConnection::OnSessionValidated( void )
{
	m_bIsVerifying = FALSE;
	m_bHasVerified = TRUE;

	if ( m_bIsHost )
	{
	UDPSESSION_CONNECT_MESSAGE		xConnectMsg;
	int		nLoop;

		memset( &xConnectMsg, 0, sizeof( xConnectMsg ) );

		if ( mnScrambleKeyLength == 0 )
		{
			// Generate scramble key
			mnScrambleKeyLength = 24;
			for( nLoop = 0; nLoop < mnScrambleKeyLength; nLoop++ )
			{
				m_abScrambleKey[nLoop] = (BYTE)( rand() & 0xFF );
			}

			m_uwSystemValidationIdentifier = (ushort)( rand() );
		}
	
		WriteMsgHeader( &xConnectMsg.xMsgHeader );

		// Send connect accept response..
		xConnectMsg.xMsgHeader.uwFixedSystemIdentifier = UDPSESSION_CONNECT_FIXED_SYSTEM_IDENTIFIER;
		xConnectMsg.xMsgHeader.uwMsgLen = sizeof( UDPSESSION_CONNECT_MESSAGE );

		for( nLoop = 0; nLoop < mnScrambleKeyLength; nLoop++ )
		{
			xConnectMsg.acSessionKey[nLoop] = (char)( m_abScrambleKey[nLoop] );
		}
		xConnectMsg.uwSystemValidationIdentifier = m_uwSystemValidationIdentifier;

		mfnUDPSessionMessageSendFunc( (char*)( &xConnectMsg ), sizeof( xConnectMsg ), m_ulIP, m_uwPort, 0 );
		SysUserPrint( 0, "Session validated" );
	}
}

void		UDPSessionConnection::InitBuffers( void )
{
int		nLoop;

	if ( m_bHasInitialisedBuffers == FALSE )
	{
		mpFreeOutOfOrderReceipts = (UDPSessionOutOfOrderReceipt*)( malloc( sizeof(UDPSessionOutOfOrderReceipt) * MAX_OUT_OF_ORDER_RECEIPTS ) );
		m_bHasInitialisedBuffers = TRUE;
	}
	mpOutOfOrderReceipts = NULL;
	for ( nLoop = 0; nLoop < MAX_OUT_OF_ORDER_RECEIPTS; nLoop++ )
	{
		mpFreeOutOfOrderReceipts[nLoop].uwGuarID = 0;
		mpFreeOutOfOrderReceipts[nLoop].mpNext = &mpFreeOutOfOrderReceipts[nLoop + 1];
	}
	mpFreeOutOfOrderReceipts[MAX_OUT_OF_ORDER_RECEIPTS-1].mpNext = NULL;
}

void	UDPSessionConnection::ClearOutboundMessage( void )
{
	m_nOutboundMsgLen = 0;
	mnOutboundMsgRepeatTries = 0;
}

bool	UDPSessionConnection::SetOutboundMessage( const char* pcMsg, int nMsgLen )
{
	if ( m_nOutboundMsgLen == 0 )
	{
		memcpy( m_aOutboundMsgBuff, pcMsg, nMsgLen );
		m_nOutboundMsgLen = nMsgLen;

		mfnUDPSessionMessageSendFunc( m_aOutboundMsgBuff, m_nOutboundMsgLen, m_ulIP, m_uwPort, 0 );
		mulOutboundMsgLastSendTick = SysGetTick();
		mnOutboundMsgRepeatTries = 0;
		return( true );
	}
	return( false );
}


BOOL		UDPSessionConnection::InitConnectionOnClient( ulong ulHostIP, ushort uwHostPort, const char* szSessionKey, u64 ullUserUID )
{
UDPSESSION_CONNECT_MESSAGE		xConnectMsg;

	if ( ( szSessionKey ) &&
		 ( szSessionKey[0] != 0 ) )
	{
		InitBuffers();
		OnCommunicationReceive();

		m_ulIP = ulHostIP;
		m_uwPort = uwHostPort;
		m_bIsVerifying = TRUE;
		m_bHasVerified = FALSE;

		memset( &xConnectMsg, 0, sizeof( xConnectMsg ) );
		// TODO - Should do 'SetOutboundMessage' so its repeat sent until acknowledged..
	
		xConnectMsg.xMsgHeader.uwFixedSystemIdentifier = UDPSESSION_CONNECT_FIXED_SYSTEM_IDENTIFIER;
		xConnectMsg.xMsgHeader.bConnectionVersion = 1;
		xConnectMsg.xMsgHeader.bEncryptKeySet = 1;
		xConnectMsg.xMsgHeader.bProtocolVersion = 1;
		xConnectMsg.xMsgHeader.uwMsgLen = sizeof( UDPSESSION_CONNECT_MESSAGE );
//		xConnectMsg.xMsgHeader.uwTimestamp = (ushort)( SysGetTick() );
		xConnectMsg.ullUserUID = ullUserUID;
		strcpy( xConnectMsg.acSessionKey, szSessionKey );

		SetOutboundMessage( (char*)( &xConnectMsg ), sizeof( xConnectMsg ) );
//		mfnUDPSessionMessageSendFunc( (char*)( &xConnectMsg ), sizeof( xConnectMsg ), ulHostIP, uwHostPort, 0 );

		m_bIsHost = FALSE;
		return( TRUE );
	}
	return( FALSE );
}

void		UDPSessionConnection::InitConnectionTest( void )
{
	InitBuffers();
	OnCommunicationReceive();
	m_bIsVerifying = TRUE;
	m_bHasVerified = FALSE;
	m_uwSystemValidationIdentifier = 1;
	mnScrambleKeyLength = 24;
	m_uwGuarIDsReceivedUpTo = 65534;
}

void		UDPSessionConnection::InitConnectionOnHost( ulong ulIP, ushort uwPort, const char* szSessionKey, u64 ullUserUID )
{
	InitBuffers();
	OnCommunicationReceive();
	SetUserUID( ullUserUID );

	m_ulIP = ulIP;
	m_uwPort = uwPort;
	m_bIsVerifying = TRUE;
	m_bHasVerified = FALSE;
	m_bIsHost = TRUE;

	m_ulLoginStartTime = SysGetTimeLong();
}

BOOL		UDPSessionConnection::IsThisAddress( ulong ulIP, ushort uwPort )
{
	if ( ( m_ulIP == ulIP ) &&
		 ( m_uwPort == uwPort ) )
	{
		return( TRUE );
	}
	return( FALSE );
}


void		UDPSessionConnection::UpdateConnectionClient( float delta )
{
	if ( m_bHasVerified )
	{
			// TODO - This time should be dependent on ping
	float	fTimeBetweenGuarRequests = 2.0f;

		// If we've got any out of order in the buffer, request frequency increases
		if ( mpOutOfOrderReceipts != NULL )
		{
			fTimeBetweenGuarRequests = 1.0f;
		}

		m_fClientTimeSinceLastGuarRequest += delta;
		m_fClientTimeSinceLastGuarRepeatSend += delta;

		// Every so often we should send to the host with the last Received GuarID
		// (So it can resend them if we've missed any).
		if ( m_fClientTimeSinceLastGuarRequest > fTimeBetweenGuarRequests )
		{
		BYTE		abMsgBuff[256];
		UDPSESSION_MSG_HEADER*		pxMsgHeader = (UDPSESSION_MSG_HEADER*)( abMsgBuff );
		UDPSESSION_SCRAMBLED_MSG_HEADER*	pxScrambleHeader = (UDPSESSION_SCRAMBLED_MSG_HEADER*)( pxMsgHeader + 1 );
		UDPSESSION_GUARANTEED_MSG_HEADER*	pxGuarHeader = (UDPSESSION_GUARANTEED_MSG_HEADER*)( pxScrambleHeader + 1 );

			// Send request for next guar from server
			WriteMsgHeader( pxMsgHeader );
			pxMsgHeader->uwFixedSystemIdentifier = UDPSESSION_CLIENTGUARREQUEST_FIXED_SYSTEM_IDENTIFIER;
			pxMsgHeader->uwMsgLen = sizeof( UDPSESSION_MSG_HEADER ) + sizeof( UDPSESSION_SCRAMBLED_MSG_HEADER ) + sizeof( UDPSESSION_GUARANTEED_MSG_HEADER );
			pxScrambleHeader->uwSystemValidationIdentifier = m_uwSystemValidationIdentifier;
			pxScrambleHeader->uwTimestamp = (ushort)( SysGetTick() );
			pxGuarHeader->uwGuarID = m_uwGuarIDsReceivedUpTo + 1;
			pxGuarHeader->bPad1 = 0;
			pxGuarHeader->bIsReceiptFlag = 0;

#ifdef DEBUG_GUAR
			SysUserPrint( 0, "Requesting guar %d", pxGuarHeader->uwGuarID );
#endif

			ScrambleMessage( (BYTE*)pxScrambleHeader, pxMsgHeader->uwMsgLen - sizeof( UDPSESSION_MSG_HEADER ), pxScrambleHeader->uwTimestamp );

			if ( m_ulIP != 0 )
			{
				mfnUDPSessionMessageSendFunc( (char*)( abMsgBuff ), pxMsgHeader->uwMsgLen, m_ulIP, m_uwPort, 0 );
			}
				
			m_fClientTimeSinceLastGuarRequest = 0.0f;
		}

		// TODO - We should also resend our outgoing guar messages if we've not received a receipt for em recently

		if ( m_fClientTimeSinceLastGuarRepeatSend > 0.5f )
		{
			UDPSessionGuaranteedMessage*	pxGuarMsgs = mpGuaranteedMessageSendBuffer;

			// Find the message the client wants in our buffer
			while( pxGuarMsgs )
			{
				// TODO - This time should be dependent on ping

				// If we last sent it more than 2 sec ago
				if ( pxGuarMsgs->mulLastSendTimestamp < SysGetTick() - 2000 )
				{
#ifdef DEBUG_GUAR
					SysUserPrint( 0, "Repeat send guar %d", pxGuarMsgs->m_uwGuarID );
#endif
					// Send it again
					pxGuarMsgs->Send( this );
					m_fClientTimeSinceLastGuarRepeatSend = 0.0f;
				}
				pxGuarMsgs = pxGuarMsgs->mpNext;
			}
		}
	}
	else if ( m_bIsVerifying )
	{
	ulong	ulTimeSinceLoginStart = SysGetTimeLong() - m_ulLoginStartTime;

		// TODO - If we remain in verifying state for too long, timeout...
		if ( ulTimeSinceLoginStart > 15 ) 
		{
			// todo - ClearOutboundMessage
			// flag error state
		}
	}
}

void		UDPSessionConnection::UpdateConnectionHost( float delta )
{
	if ( m_bHasVerified == TRUE )
	{
	ulong	ulCurrentTick = SysGetTick();

		// Host shouldn't do much..
		// Just timeout if we haven't received any messages from this client in a while. Everything else should be actioned
		// off a message receive from the client to minimise our CPU overhead here.
		if ( ulCurrentTick - m_ulLastMessageReceived > NO_COMMUNICATION_TIMEOUT_MS )
		{
			SysUserPrint( 0, "Connection timeout - Disconnect" );
			Disconnect();
		}
	}
}

void	UDPSessionConnection::SendGuaranteedReceipt( ushort uwGuarID, ushort uwTimestamp )
{
char			abMsgBuff[256];
UDPSESSION_MSG_HEADER*		pxUDPSessionMsgHeader = (UDPSESSION_MSG_HEADER*)( abMsgBuff );
UDPSESSION_SCRAMBLED_MSG_HEADER*		pxScrambleHeader;
UDPSESSION_GUARANTEED_MSG_HEADER*		pxGuarHeader;

	WriteMsgHeader( pxUDPSessionMsgHeader );
	pxUDPSessionMsgHeader->uwFixedSystemIdentifier = UDPSESSION_GUARANTEED_FIXED_SYSTEM_IDENTIFIER;
	pxUDPSessionMsgHeader->uwMsgLen = sizeof( UDPSESSION_MSG_HEADER ) + sizeof( UDPSESSION_SCRAMBLED_MSG_HEADER ) + sizeof( UDPSESSION_GUARANTEED_MSG_HEADER );
//	pxUDPSessionMsgHeader->uwTimestamp = uwTimestamp;
	
	pxScrambleHeader = (UDPSESSION_SCRAMBLED_MSG_HEADER*)( pxUDPSessionMsgHeader + 1 );
	pxGuarHeader = ( UDPSESSION_GUARANTEED_MSG_HEADER*)( pxScrambleHeader + 1 );

	pxScrambleHeader->uwSystemValidationIdentifier = m_uwSystemValidationIdentifier;
	pxScrambleHeader->uwTimestamp = (ushort)( SysGetTick() );

	pxGuarHeader->bIsReceiptFlag = 1;
	pxGuarHeader->bPad1 = 0;
	pxGuarHeader->uwGuarID = uwGuarID;

	ScrambleMessage( (BYTE*)pxScrambleHeader, pxUDPSessionMsgHeader->uwMsgLen - sizeof( UDPSESSION_MSG_HEADER ), pxScrambleHeader->uwTimestamp );

	if ( m_ulIP != 0 )
	{
		mfnUDPSessionMessageSendFunc( (char*)( abMsgBuff ), pxUDPSessionMsgHeader->uwMsgLen, m_ulIP, m_uwPort, 0 );
	}
}	

void	UDPSessionConnection::RegisterMessageHandler( UDPSessionConnectionMessageReceiveHandler fnMessageHandler, void* pUserData )
{
	mfnMessageHandler = fnMessageHandler;
	mpMessageHandlerUserParam = pUserData;

}


void	UDPSessionConnection::OnClientRequestGuarUpdate( const char* pcMsg )
{
char			abMsgBuff[256];
UDPSESSION_MSG_HEADER*		pxUDPSessionMsgHeader = (UDPSESSION_MSG_HEADER*)( pcMsg );
BYTE*						pbMsgBody = (BYTE*)( pxUDPSessionMsgHeader + 1 );
int			nBodyLen = pxUDPSessionMsgHeader->uwMsgLen - sizeof( UDPSESSION_MSG_HEADER );
UDPSESSION_GUARANTEED_MSG_HEADER*		pxGuarHeader;
UDPSESSION_SCRAMBLED_MSG_HEADER*		pxScrambleHeader;

	OnCommunicationReceive();
	
	if ( ( nBodyLen > 0 ) &&
		 ( nBodyLen < 256 ) )
	{
		memcpy( abMsgBuff, pbMsgBody, nBodyLen );
		UnscrambleMessage( (BYTE*)abMsgBuff, nBodyLen );

		pxScrambleHeader = (UDPSESSION_SCRAMBLED_MSG_HEADER*)( abMsgBuff );
		if ( pxScrambleHeader->uwSystemValidationIdentifier == m_uwSystemValidationIdentifier )
		{
			pxGuarHeader = (UDPSESSION_GUARANTEED_MSG_HEADER*)( pxScrambleHeader + 1 );

			if ( pxGuarHeader->uwGuarID == m_uwNextSendGuarID )
			{
				// Client is up to date with our guars.. no need to do anything..
			}
			else
			{
			UDPSessionGuaranteedMessage*	pxGuarMsgs = mpGuaranteedMessageSendBuffer;

				// Find the message the client wants in our buffer
				while( pxGuarMsgs )
				{
					if ( pxGuarMsgs->m_uwGuarID == pxGuarHeader->uwGuarID )
					{
						// If we last sent it more than 1 sec ago
						if ( pxGuarMsgs->mulLastSendTimestamp < SysGetTick() - 1000 )
						{
#ifdef DEBUG_GUAR
							SysUserPrint( 0, "Repeat send guar %d", pxGuarHeader->uwGuarID );
#endif
							// Send it again
							pxGuarMsgs->Send( this );
						}
					}
					pxGuarMsgs = pxGuarMsgs->mpNext;
				}
			}
		}
	}
}

void	UDPSessionConnection::SendTermination( void )
{
BYTE		abMsgBuff[256];
UDPSESSION_MSG_HEADER*		pxMsgHeader = (UDPSESSION_MSG_HEADER*)( abMsgBuff );
	
	WriteMsgHeader( pxMsgHeader );

	pxMsgHeader->uwFixedSystemIdentifier = UDPSESSION_TERMINATE_FIXED_SYSTEM_IDENTIFIER;
	pxMsgHeader->uwMsgLen = sizeof( UDPSESSION_MSG_HEADER );

	if ( m_ulIP != 0 )
	{
		mfnUDPSessionMessageSendFunc( (char*)( abMsgBuff ), pxMsgHeader->uwMsgLen, m_ulIP, m_uwPort, 0 );
	}
}

void	UDPSessionConnection::ResendTermination( void )
{
	if ( SysGetTick() - m_ulLastTerminateSend > 2000 )
	{
		SendTermination();
		m_ulLastTerminateSend = SysGetTick();
	}
}

void	UDPSessionConnection::Disconnect( void )
{
	OnDisconnect();
	SendTermination();
	m_bHasDisconnected = TRUE;
	m_bHasVerified = FALSE;
	m_ulLastTerminateSend = SysGetTick();
}

void	UDPSessionConnection::OnReceiveTerminationMessage( const char* pcMsg )
{
	OnDisconnect();
	SysUserPrint( 0, "Connection terminated by remote host" );
	m_bHasDisconnected = TRUE;
	m_bHasVerified = FALSE;
}

BOOL	UDPSessionConnection::IsDead( void )
{
	if ( ( m_bHasDisconnected ) &&
		 ( SysGetTick() - m_ulLastMessageReceived > 5000 ) &&
		 ( SysGetTick() - m_ulLastTerminateSend > 2000 ) )
	{
		return( TRUE );
	}
	return( FALSE );
}


void	UDPSessionConnection::OnReceiveGuaranteedMessage( const char* pcMsg )
{
char			abMsgBuff[8192];
UDPSESSION_MSG_HEADER*		pxUDPSessionMsgHeader = (UDPSESSION_MSG_HEADER*)( pcMsg );
BYTE*						pbMsgBody = (BYTE*)( pxUDPSessionMsgHeader + 1 );
int			nBodyLen = pxUDPSessionMsgHeader->uwMsgLen - sizeof( UDPSESSION_MSG_HEADER );
UDPSESSION_GUARANTEED_MSG_HEADER*		pxGuarHeader;
UDPSESSION_SCRAMBLED_MSG_HEADER*		pxScrambleHeader;

	OnCommunicationReceive();

	if ( ( nBodyLen > 0 ) &&
		 ( nBodyLen < 8000 ) )
	{
		memcpy( abMsgBuff, pbMsgBody, nBodyLen );
		UnscrambleMessage( (BYTE*)abMsgBuff, nBodyLen );

		pxScrambleHeader = (UDPSESSION_SCRAMBLED_MSG_HEADER*)( abMsgBuff );
		if ( pxScrambleHeader->uwSystemValidationIdentifier == m_uwSystemValidationIdentifier )
		{
			pxGuarHeader = (UDPSESSION_GUARANTEED_MSG_HEADER*)( pxScrambleHeader + 1 );
		
			if ( pxGuarHeader->bIsReceiptFlag == 1 )
			{
			UDPSessionGuaranteedMessage*	pxGuaranteedMessages = mpGuaranteedMessageSendBuffer;
			UDPSessionGuaranteedMessage*	pLast = NULL;

				while( pxGuaranteedMessages )
				{
					if ( pxGuaranteedMessages->m_uwGuarID == pxGuarHeader->uwGuarID )
					{
						// We got a receipt for this guar msg now, so we can delete it from our buffer
						if ( pLast == NULL )
						{
							mpGuaranteedMessageSendBuffer = pxGuaranteedMessages->mpNext;
						}
						else
						{
							pLast->mpNext = pxGuaranteedMessages->mpNext;
						}
						delete pxGuaranteedMessages;
						return;
					}
					pLast = pxGuaranteedMessages;
					pxGuaranteedMessages = pxGuaranteedMessages->mpNext;
				}

				// Duplicate receipt? ignore..
			}
			else
			{
			BOOL		bDuplicate = FALSE;
			int			nConfirmedGuarsUpTo = m_uwGuarIDsReceivedUpTo;

				pbMsgBody = (BYTE*)( pxGuarHeader + 1 );

				// send receipt back
				SendGuaranteedReceipt( pxGuarHeader->uwGuarID, pxScrambleHeader->uwTimestamp );
				
				// Check for loop on ushort
				if ( nConfirmedGuarsUpTo > pxGuarHeader->uwGuarID + 32768 )
				{
					nConfirmedGuarsUpTo -= 65536;
				}

				// check if we've already received this GuarID (and ignore if so)
				if ( ( (int)pxGuarHeader->uwGuarID > nConfirmedGuarsUpTo ) &&
					 ( (int)pxGuarHeader->uwGuarID < nConfirmedGuarsUpTo + 32768 ) )
				{
				UDPSessionOutOfOrderReceipt*		pOutOfOrderReceipts = mpOutOfOrderReceipts;

					while( pOutOfOrderReceipts )
					{
						if ( pOutOfOrderReceipts->uwGuarID == pxGuarHeader->uwGuarID )
						{
							bDuplicate = TRUE;
							break;
						}
						pOutOfOrderReceipts = pOutOfOrderReceipts->mpNext;
					}
				}
				else
				{
					bDuplicate = TRUE;
				}

				if ( bDuplicate == FALSE )
				{
					// pass message on to game message handler
					if ( mfnMessageHandler )
					{
						mfnMessageHandler( pbMsgBody, this, mpMessageHandlerUserParam );
					}

					// Store receipt
					if ( pxGuarHeader->uwGuarID == nConfirmedGuarsUpTo + 1 )
					{
					UDPSessionOutOfOrderReceipt*		pOutOfOrderReceipts = mpOutOfOrderReceipts;
					UDPSessionOutOfOrderReceipt*		pLast = NULL;
					UDPSessionOutOfOrderReceipt*		pNext;

						m_uwGuarIDsReceivedUpTo++;
						m_fClientTimeSinceLastGuarRequest = 0.0f;

						while( pOutOfOrderReceipts )
						{
							pNext = pOutOfOrderReceipts->mpNext;
							if ( pOutOfOrderReceipts->uwGuarID == m_uwGuarIDsReceivedUpTo + 1 )
							{
#ifdef DEBUG_GUAR
								SysUserPrint( 0, "Cleared out of order guar %d", pOutOfOrderReceipts->uwGuarID );
#endif
								m_uwGuarIDsReceivedUpTo++;
								if ( pLast == NULL )
								{
									mpOutOfOrderReceipts = pOutOfOrderReceipts->mpNext;
								}
								else
								{
									pLast->mpNext = pOutOfOrderReceipts->mpNext;
								}
								pOutOfOrderReceipts->mpNext = mpFreeOutOfOrderReceipts;
								mpFreeOutOfOrderReceipts = pOutOfOrderReceipts;
							}
							else
							{
								pLast = pOutOfOrderReceipts;
							}
							pOutOfOrderReceipts = pNext;
						}
					}
					else  // not the next expected guar, so we add it to the out of order receipts
					{
					UDPSessionOutOfOrderReceipt*		pOutOfOrderReceipts = mpFreeOutOfOrderReceipts;
					char	acString[256];
					char	acBuff[256];
#ifdef DEBUG_GUAR
						SysUserPrint( 0, "Received out of order guar %d", pxGuarHeader->uwGuarID );
#endif
						if ( pOutOfOrderReceipts )
						{
							mpFreeOutOfOrderReceipts = pOutOfOrderReceipts->mpNext;

							pOutOfOrderReceipts->uwGuarID = pxGuarHeader->uwGuarID;

							if ( mpOutOfOrderReceipts == NULL )
							{
								mpOutOfOrderReceipts = pOutOfOrderReceipts;
								pOutOfOrderReceipts->mpNext = NULL;
							}
							else
							{
								if ( mpOutOfOrderReceipts->uwGuarID > pxGuarHeader->uwGuarID )
								{
									pOutOfOrderReceipts->mpNext = mpOutOfOrderReceipts;
									mpOutOfOrderReceipts = pOutOfOrderReceipts;
								}
								else
								{
								UDPSessionOutOfOrderReceipt*		pPendingList = mpOutOfOrderReceipts;
								UDPSessionOutOfOrderReceipt*		pLast = mpOutOfOrderReceipts;

									while( pPendingList != NULL )
									{
										// If next pending is larger, insert into list
										// TODO - Cope with wrap
										if ( pPendingList->uwGuarID > pxGuarHeader->uwGuarID )
										{
											pOutOfOrderReceipts->mpNext = pPendingList;
											pLast->mpNext = pOutOfOrderReceipts;
											break;
										}
										else
										{
											pLast = pPendingList;
										}
										pPendingList = pPendingList->mpNext;
									}

									// Add at end
									if ( pPendingList == NULL )
									{
										pLast->mpNext = pOutOfOrderReceipts;
										pOutOfOrderReceipts->mpNext = NULL;
									}
								}
							}
#ifdef DEBUG_GUAR
							sprintf( acString, "Pending: " );
							UDPSessionOutOfOrderReceipt*		pPendingList = mpOutOfOrderReceipts;

							while( pPendingList != NULL )
							{
								sprintf( acBuff, "[%d] ", pPendingList->uwGuarID );
								strcat( acString, acBuff );
								pPendingList = pPendingList->mpNext;
							}
							SysUserPrint( 0, acString );
#endif		
						}
						else
						{
#ifdef DEBUG_GUAR
							SysUserPrint( 0, "No out of order receipts left - disconnected" );
#endif
							// Out of free space.. disconnect??
							Disconnect();
						}
					}
				}
#ifdef DEBUG_GUAR
				else
				{
					SysUserPrint( 0, "Ignored duplicate guar %d", pxGuarHeader->uwGuarID );
				}
#endif
			}
		}
	}
}

void	UDPSessionConnection::OnCommunicationReceive( void )
{
	m_ulLastMessageReceived = SysGetTick();
}

void	UDPSessionConnection::OnReceiveNonGuaranteedMessage( const char* pcMsg )
{
char			abMsgBuff[8192];
UDPSESSION_MSG_HEADER*		pxUDPSessionMsgHeader = (UDPSESSION_MSG_HEADER*)( pcMsg );
BYTE*						pbMsgBody = (BYTE*)( pxUDPSessionMsgHeader + 1 );
int			nBodyLen = pxUDPSessionMsgHeader->uwMsgLen - sizeof( UDPSESSION_MSG_HEADER );
UDPSESSION_SCRAMBLED_MSG_HEADER*		pxScrambleHeader;

	OnCommunicationReceive();

	if ( ( nBodyLen > 0 ) &&
		 ( nBodyLen < 8000 ) )
	{
		memcpy( abMsgBuff, pbMsgBody, nBodyLen );
		UnscrambleMessage( (BYTE*)abMsgBuff, nBodyLen );

		pxScrambleHeader = (UDPSESSION_SCRAMBLED_MSG_HEADER*)( abMsgBuff );
		if ( pxScrambleHeader->uwSystemValidationIdentifier == m_uwSystemValidationIdentifier )
		{
			pbMsgBody = (BYTE*)( pxScrambleHeader + 1 );

			// pass message on to game message handler
			if ( mfnMessageHandler )
			{
				mfnMessageHandler( pbMsgBody, this, mpMessageHandlerUserParam );
			}
		}

	}
	
}
