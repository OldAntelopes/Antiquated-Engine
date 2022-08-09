#include <string.h>
#include <stdio.h>
#include <UnivSocket.h>

#include "StandardDef.h"
#include "Networking.h"

#include "../Platform/Platform.h"
#include "../Network/UserLoginClient.h"

#include "PeerConnection/PeerConnection.h"
#include "ChatServiceMessages.h"
#include "ClientConnectionToChatService.h"

ClientConnectionToChatService*		m_spClientConnectionToChatServiceSingleton = NULL;
BOOL	mbUsingTcpForChatServiceConnection = FALSE;
BOOL	mbAttemptTcpForChatServiceConnectionIfUDPFails = TRUE;
ChatServiceMessageCallback			mfnChatMessageHandler = NULL;

ulong		mulDisconnectTime = 0;



class SubscribedChatChannelInfo
{
public:
	SubscribedChatChannelInfo()
	{
		mszChannelName = NULL;
		mulLastReceivedTimestamp = 0;
		mpNext = NULL;
	}

	~SubscribedChatChannelInfo()
	{
		SAFE_FREE( mszChannelName );
	}

	char*		mszChannelName;
	ulong		mulLastReceivedTimestamp;
	SubscribedChatChannelInfo*		mpNext;
};

SubscribedChatChannelInfo*		 mspSubscribedChannelsList = NULL;

void		ClientConnectionToChatServiceRegisterChatHandler( ChatServiceMessageCallback fnChatMessageHandler )
{
	mfnChatMessageHandler = fnChatMessageHandler;
}

void	ClientConnectionToChatService::Reset( void )
{
	m_state = NONE;
	m_loginFailCode = NO_FAIL;

	mulQueuedMsgLastSendTick = 0;
	memset( m_aQueuedMsgBuff, 0, 512 );
	m_nQueuedMsgLen = 0;
	mnQueuedMsgRepeatTries = 0;
	m_ulBaseIP = 0;
	m_uwBasePort = 0;

	m_BaseErrorCode = 0;
	m_szNewAccountUsername[0] = 0;
	m_szNewPassword[0] = 0;
	m_ulOnlineUserCount = 0;
	m_ulOnlineGroupsCount = 0;

}

ClientConnectionToChatService::ClientConnectionToChatService()
{
	Reset();
}



ClientConnectionToChatService::~ClientConnectionToChatService()
{
SubscribedChatChannelInfo*		pChannelInfo = mspSubscribedChannelsList;
SubscribedChatChannelInfo*		pNext;

	while( pChannelInfo )
	{
		pNext = pChannelInfo->mpNext;
		delete pChannelInfo;
		pChannelInfo = pNext;
	}
	mspSubscribedChannelsList = NULL;

	m_spClientConnectionToChatServiceSingleton = NULL;
}

ClientConnectionToChatService*		ClientConnectionToChatService::Get( void )
{
	return( m_spClientConnectionToChatServiceSingleton );
}



//----------------------------------------------------------------
// ClientConnectionToChatService::SetOutboundMessage
//
//----------------------------------------------------------------
bool	ClientConnectionToChatService::SetOutboundMessage( const char* pcMsg, int nMsgLen )
{
	if ( m_nQueuedMsgLen == 0 )
	{
		memcpy( m_aQueuedMsgBuff, pcMsg, nMsgLen );
		m_nQueuedMsgLen = nMsgLen;

		NetworkConnectionGenericSendMessage( m_aQueuedMsgBuff, m_nQueuedMsgLen, m_ulBaseIP, m_uwBasePort, FALSE );
		mulQueuedMsgLastSendTick = SysGetTick();
		mnQueuedMsgRepeatTries = 0;
		m_BaseErrorCode = 0;
		return( true );
	}
	return( false );
}

//----------------------------------------------------------------
// ClientConnectionToChatService::ClearOutboundMessage
//
//----------------------------------------------------------------
void	ClientConnectionToChatService::ClearOutboundMessage( void )
{
	m_nQueuedMsgLen = 0;
	m_BaseErrorCode = 0;
}

//----------------------------------------------------------------
// ClientConnectionToChatService::OnFileReceiveComplete
//
//----------------------------------------------------------------
void	ClientConnectionToChatService::OnFileReceiveComplete( short wFileID, const char* szFilename, byte* pbMsg, int nMemSize )
{

}

void	ClientConnectionToChatService::OnDisconnected( void )
{
	if ( m_state == LOGGING_IN )
	{
		m_state = CONNECTION_REJECTED;
		NetworkingDebugPrint( "NCS disconnected during login. Aborting" );
	}
	else if ( m_state != CONNECTION_REJECTED )
	{
		m_state = DISCONNECTED;
		mulDisconnectTime = SysGetTimeLong();
		NetworkingDebugPrint( "NCS disconnected" );
	}
}


void	ClientConnectionToChatService::ParseAuthenticatedLoginResponse( byte* pbMsgBody )
{
NCHAT_AUTHENTICATED_LOGIN_RESPONSE_MSG*		pxMsg = (NCHAT_AUTHENTICATED_LOGIN_RESPONSE_MSG*)pbMsgBody;
//ulong		ulIP;
char		szLoginFailMsg[128] = "";

	if ( pxMsg->bResponseCode != 0 )
	{
		sprintf( szLoginFailMsg, "NCS connect failed: err %d", pxMsg->bResponseCode );
	}

	switch ( pxMsg->bResponseCode )
	{
	case 0:		// Valid login
		{
//		char	acString[256];

			m_ulOnlineUserCount = pxMsg->ulNumOnline;
			m_ulOnlineGroupsCount = pxMsg->ulNumGroups;
			mulLastStatRequestTime = SysGetTimeLong();
			SendChannelsList();
//			sprintf( acString, "Logged in : %s", UserGetName() );
//			NetworkingDebugPrint( acString );
			m_state = COMPLETE;
		}
		break;
	case 1:		// Invalid login
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = LOGINEMAIL_NOT_RECOGNISED;
		break;
	case 2:	
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = PASSWORD_MISMATCH;
		break;
	case 3:					
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = NEW_USER_CREATE_FAILED;
		break;
	case 4:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = NEW_USER_LOGINEMAIL_EXISTS;
		break;
	case 5:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = NEW_USER_LOGIN_USERNAME_EXISTS;
		break;
	case 6:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = MULTIPLE_ACCOUNTS;
		break;
	case 7:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = BANNED;
		break;
	case 9:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = CLOSED_FOR_MAINTENANCE;
		break;
	case 10:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = PASSWORD_RESET_EMAIL_SENT;
		break;
	case 11:		// Logged in with temporary password, so now the client should prompt for a new one
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = PASSWORD_RESET_PROMPT;	
		break;
	case 12:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = PASSWORD_NEW_PASSWORD_CONFIRMED;	
		break;
	case 13:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = AUTHENTICATION_ERROR;	
		strcpy( szLoginFailMsg, "Connect failed: NCS Authentication error" );
		break;
	case 8:
	default:
		m_state = CONNECTION_REJECTED;
		m_loginFailCode = OTHER;
		break;
	}

	if ( pxMsg->bResponseCode != 0 )
	{
		NetworkingDebugPrint( szLoginFailMsg );
		CloseConnection(FALSE);
	}
}


void	ClientConnectionToChatService::ParseChannelListMsg( byte* pbMsgBody )
{
NCHAT_CHAT_MSG*		pxMsg = (NCHAT_CHAT_MSG*)( pbMsgBody );
const char*			pcText = (const char*)( pxMsg + 1 );
char		acMsgBuff[1024];
int			nSelectedTab = 0xFF;
const char*		pcRunner;
char*		pcOutRunner;

	// ( nSelectedTab = 0xff indicated we're not looking at a particularly group's screen exclusively)
	if ( ( nSelectedTab == 0xFF ) ||
		 ( pxMsg->bGroupNum == nSelectedTab ) )
	{
		sprintf( acMsgBuff, "Currently online: " );
		pcRunner = pcText;
		pcOutRunner = acMsgBuff + strlen( acMsgBuff );
		while( *pcRunner != 0 )
		{
			if ( *pcRunner == '|' )
			{
				*pcOutRunner++ = ',';
				*pcOutRunner++ = ' ';
			}
			else
			{
				*pcOutRunner++ = *pcRunner;
			}
			pcRunner++;
		}
		*pcOutRunner = 0;

		if ( mfnChatMessageHandler )
		{
			mfnChatMessageHandler( 1, NULL, acMsgBuff, pxMsg->ulTimestamp );
		}
	}
}


void	ClientConnectionToChatService::ParseStatMsg( byte* pbMsgBody )
{
NCHAT_CHAT_MSG*		pxMsg = (NCHAT_CHAT_MSG*)( pbMsgBody );
const char*			pcText = (const char*)( pxMsg + 1 );
int		nNumOnlineInChannel;

	nNumOnlineInChannel = strtol( pcText, NULL, 10 );

	m_ulOnlineUserCount = (ulong)( nNumOnlineInChannel );
}

void	ClientConnectionToChatService::ParseCommandResponseMsg( byte* pbMsgBody )
{
NCHAT_CHAT_MSG*		pxMsg = (NCHAT_CHAT_MSG*)( pbMsgBody );
const char*			pcText = (const char*)( pxMsg + 1 );

	if ( mfnChatMessageHandler )
	{
		mfnChatMessageHandler( 1, NULL, pcText, pxMsg->ulTimestamp );
	}

}

void	ClientConnectionToChatService::ParseChatMsg( byte* pbMsgBody )
{
NCHAT_CHAT_MSG*		pxMsg = (NCHAT_CHAT_MSG*)( pbMsgBody );
const char*			pcText = (const char*)( pxMsg + 1 );
char		acMsgBuff[1024];
char*		pcFromName;
char*		pcMessage;

	strcpy( acMsgBuff, pcText );
	pcFromName = acMsgBuff;
	pcMessage = SplitStringSep( acMsgBuff, '|' );
	if ( *pcMessage == 0 )
	{
		pcMessage = pcFromName;
		pcFromName = NULL;
	}

	if ( pxMsg->bGroupNum < MAX_CHANNELS_PER_USER )
	{
//		UserSetChannelLastMsgTimestamp( pxMsg->bGroupNum, pxMsg->ulTimestamp );
	}

	if ( mfnChatMessageHandler )
	{
		mfnChatMessageHandler( pxMsg->bGroupNum, pcFromName, pcMessage, pxMsg->ulTimestamp );
	}
}


bool	ClientConnectionToChatService::IsServerAddress( ulong ulIP, ushort uwPort )
{
ulong		ulLocalRedirect = NetworkGetIPAddress( "192.168.1.67" );		// TEMP - this is for me

	if ( ( (ulIP == m_ulBaseIP) || (ulIP == ulLocalRedirect ) ) &&
		 ( uwPort == m_uwBasePort ) )
	{
		return( true );
	}
	return( false );
}


//----------------------------------------------------------------
// ClientConnectionToChatService::OnReceiveGuaranteedMsg
//
//----------------------------------------------------------------
void	ClientConnectionToChatService::OnReceiveGuaranteedMsg( byte* pbMsgBody )
{
	// TODO - On receipt of valid login authorisation

	switch( *pbMsgBody )
	{
	case NCHATMSGID_AUTHENTICATED_LOGIN_RESPONSE:
		ParseAuthenticatedLoginResponse( pbMsgBody );
		break;
	case NCHATMSGID_STATS_MSG:
		ParseStatMsg( pbMsgBody );
		break;
	case NCHATMSGID_STD_CHAT_MSG:
		ParseChatMsg( pbMsgBody );
		break;
	case NCHATMSGID_CHAT_CMD_MSG:
		ParseCommandResponseMsg( pbMsgBody );
		break;
	case NCHATMSGID_USER_LIST_CHAT_MSG:
		ParseChannelListMsg( pbMsgBody );
		break;
	default:
		// mm.. unknown response from auth server.. error?
		break;
	}

}


//----------------------------------------------------------------
// ClientConnectionToChatService::RequestServerFile
//
//----------------------------------------------------------------
void	ClientConnectionToChatService::RequestServerFile( const char* szFilename )
{

}



void ClientConnectionToChatService::ResetChatChannels( void )
{
SubscribedChatChannelInfo*		pChannelInfo = mspSubscribedChannelsList;
SubscribedChatChannelInfo*		pNext;

	while( pChannelInfo )
	{
		pNext = pChannelInfo->mpNext;
		delete pChannelInfo;
		pChannelInfo = pNext;
	}
	mspSubscribedChannelsList = NULL;
}

void ClientConnectionToChatService::AddChatChannel( const char* szChannelName, ulong ulLastReceivedTimestamp )
{
SubscribedChatChannelInfo*		pChannelInfo = new SubscribedChatChannelInfo;

	if ( mspSubscribedChannelsList == NULL )
	{
		mspSubscribedChannelsList = pChannelInfo;
	}
	else
	{
	SubscribedChatChannelInfo*		pLast = mspSubscribedChannelsList;

		while( pLast->mpNext != NULL )
		{
			pLast = pLast->mpNext;
		}
		pLast->mpNext = pChannelInfo;
	}
	pChannelInfo->mpNext = NULL;

	pChannelInfo->mulLastReceivedTimestamp = ulLastReceivedTimestamp;
	pChannelInfo->mszChannelName = (char*)( malloc( strlen( szChannelName ) + 1 ) );
	strcpy( pChannelInfo->mszChannelName, szChannelName );
}

void	ClientConnectionToChatService::SendChannelsList( void )
{
char	acMsgBuffer[1024];
NCHAT_CHAT_MSG*		pxMsg = (NCHAT_CHAT_MSG*)( acMsgBuffer );
char*				pcTextBuffer = (char*)( pxMsg + 1 );
char*		pcOut = pcTextBuffer;
const char*		pcChannelName;
ulong			ulChannelTimestamp;
//const char*		pcChannelPW;
SubscribedChatChannelInfo*		pChannelInfo = mspSubscribedChannelsList;

	while( pChannelInfo )
	{
		pcChannelName = pChannelInfo->mszChannelName;
		if ( ( pcChannelName ) &&
			 ( pcChannelName[0] != 0 ) )
		{
			strcpy( pcOut, pcChannelName );
			pcOut += strlen( pcChannelName );
			*pcOut++ = ':';
			ulChannelTimestamp = pChannelInfo->mulLastReceivedTimestamp;
			if ( ulChannelTimestamp == 0 )
			{
				sprintf( pcOut, "0" );
				pcOut++;
			}
			else
			{
				sprintf( pcOut, "%08x", (unsigned int)ulChannelTimestamp );
				pcOut += 8;
			}
			*pcOut++ = '|';
		}
		pChannelInfo = pChannelInfo->mpNext;
	}
	*pcOut = 0;

	pxMsg->uwTextLen = strlen( pcTextBuffer );
	if ( pxMsg->uwTextLen > 0 )
	{
		pcOut--;
		if ( *pcOut == '|' )
		{
			*pcOut = 0;
		}
	}
	pxMsg->bGroupNum = 0xFF;
	pxMsg->bMsgID = NCHATMSGID_STD_CHAT_MSG;

	SendMessageGuaranteed( (byte*)( acMsgBuffer ), pxMsg->uwTextLen + 1 + sizeof( NCHAT_CHAT_MSG ) );
}



void	ClientConnectionToChatService::SendChatMessage( int nGroupNum, const char* pcMessage )
{
char	acMsgBuffer[1024];
NCHAT_CHAT_MSG*		pxMsg = (NCHAT_CHAT_MSG*)( acMsgBuffer );
char*				pcTextBuffer = (char*)( pxMsg + 1 );

	strcpy( pcTextBuffer, pcMessage );
	pxMsg->bGroupNum = (BYTE)( nGroupNum );
	pxMsg->uwTextLen = strlen( pcTextBuffer );
	pxMsg->bMsgID = NCHATMSGID_STD_CHAT_MSG;

	SendMessageGuaranteed( (byte*)( acMsgBuffer ), pxMsg->uwTextLen + 1 + sizeof( NCHAT_CHAT_MSG ) );

}


void	ClientConnectionToChatService::SendChatCommandMessage( int nMsgType, const char* pcMessage )
{
char	acMsgBuffer[1024];
NCHAT_CHAT_MSG*		pxMsg = (NCHAT_CHAT_MSG*)( acMsgBuffer );
char*				pcTextBuffer = (char*)( pxMsg + 1 );

	memset( pxMsg, 0, sizeof( NCHAT_CHAT_MSG ) );

	strcpy( pcTextBuffer, pcMessage );
	pxMsg->bGroupNum = (BYTE)( nMsgType );
	pxMsg->uwTextLen = strlen( pcTextBuffer );
	pxMsg->bMsgID = NCHATMSGID_CHAT_CMD_MSG;

	SendMessageGuaranteed( (byte*)( acMsgBuffer ), pxMsg->uwTextLen + 1 + sizeof( NCHAT_CHAT_MSG ) );

}

 


//----------------------------------------------------------------
// ClientConnectionToChatService::Update
//
//----------------------------------------------------------------
void	ClientConnectionToChatService::OnUpdate( float fDeltaTime )
{
	switch( m_state )
	{
	case COMPLETE:
/*  REMOVED NOV 17 - Do we really need to know how many ppl are on the chat server at regular intervals???
		{
		ulong	ulCurrentTime = SysGetTimeLong();
		
			// TODO - If we haven't sent anything for > 30 seconds we probably got suspended.. so disconnect..
		
			// Update chat player count every 30 seconds (todo : should only bother with this when on front screen)
			if ( ulCurrentTime - mulLastStatRequestTime > 30 ) 
			{
			char			acMsgBuffer[256];
			NCHAT_CHAT_MSG*		pxResponse = (NCHAT_CHAT_MSG*)( acMsgBuffer );
			char*			pcResponseText = (char*)( pxResponse + 1 );
			
				memset( pxResponse, 0, sizeof( *pxResponse ) );
				pcResponseText[0] = 0;

				pxResponse->bMsgID = NCHATMSGID_CHAT_CMD_MSG;
				pxResponse->ulTimestamp = 0;
				pxResponse->bGroupNum = 0;
				strcpy( pcResponseText, "stats" );
				pxResponse->uwTextLen = strlen( pcResponseText ) + 1;
				SendMessageGuaranteed( (BYTE*)( pxResponse ), sizeof( *pxResponse ) + pxResponse->uwTextLen );

				mulLastStatRequestTime = ulCurrentTime;
			}
		}
*/
		break;
	case LOGGING_IN:
		m_fConnectWaitTime += fDeltaTime;
		if ( IsConnected() )
		{
			ClearOutboundMessage();

			m_state = CONNECTED;
			// send our login details..
			SendInitialLoginMessage();
		}
		else
		{
			if ( IsBusy() == FALSE )
			{
				if ( mbUsingTcpForChatServiceConnection == TRUE )
				{
					// If TCP connect failed, give up
					m_state = CONNECT_ERROR;
					if ( mfnChatMessageHandler )
					{
						mfnChatMessageHandler( 0, NULL, "[GlobalChat] Connect error", SysGetTimeLong() );
					}
				}
				// SetOutbound queued message buffer failed to get a response and gave up..			
				else if ( m_BaseErrorCode == UDP_CONNECT_TIMEOUT_ERROR )
				{
					// Try to establish connection by TCP if direct udp route fails
					if ( mbAttemptTcpForChatServiceConnectionIfUDPFails )
					{
						mbUsingTcpForChatServiceConnection = TRUE;
						if ( mfnChatMessageHandler )
						{
							mfnChatMessageHandler( 0, NULL, "[GlobalChat] No direction connection - Attempting secondary", SysGetTimeLong() );
						}

						// Resend InitialConnectMessage if waited a while.
						SendInitialConnectMessage();
					}
					else
					{
						// Give up.. its not available :/
						m_state = CONNECT_ERROR;
					}
				}				
			}
		}
		break;
	case CONNECTED:
		// Just waiting for server response
		break;
	case DISCONNECTED:
		// After 15 seconds
		if ( mulDisconnectTime < SysGetTimeLong() - 15 )
		{
			SendInitialConnectMessage();
		}
		break;
	case CONNECT_ERROR:
		break;
	default:
		break;
	}

	if ( m_nQueuedMsgLen != 0 )
	{
		if ( mulQueuedMsgLastSendTick < SysGetTick() - 3000 )		// Resend outbound message every 3 secs
		{
			if ( mnQueuedMsgRepeatTries > 4 )
			{
				m_nQueuedMsgLen = 0;
				m_BaseErrorCode = UDP_CONNECT_TIMEOUT_ERROR;
			}
			else
			{
				NetworkConnectionGenericSendMessage( m_aQueuedMsgBuff, m_nQueuedMsgLen, m_ulBaseIP, m_uwBasePort, FALSE );

				mulQueuedMsgLastSendTick = SysGetTick();
				mnQueuedMsgRepeatTries++;
			}
		}
	}

}


//----------------------------------------------------------------
// ClientConnectionToChatService::IsMyMessage
//
//----------------------------------------------------------------
bool	ClientConnectionToChatService::IsMyMessage( void )
{
	// todo !!
/*
ulong	ulIP;
ushort	uwPort;

	NetworkConnection::Get().GetLastMessageSource( &ulIP, &uwPort );

	if ( ( ulIP == GetBaseIP() ) &&
		 ( uwPort == GetBasePort() ) )
	{
		return( true );
	}
*/
	return( false );
}

//----------------------------------------------------------------
// ClientConnectionToChatService::ReceiveMessage
//  Receive a non-guaranteed message
//----------------------------------------------------------------
int		ClientConnectionToChatService::ReceiveMessage( const byte* pcMsg )
{
int		nMsgLen = 0;
	// nothing to do here.. all messages will be sent guaranteed

	return( nMsgLen );
}


//----------------------------------------------------------------
// ClientConnectionToChatService::SendInitialLoginMessage
//  Once we've got a guaranteed connection to the chat service,
//  this sends on the session key (that we got from the user login service)
//  and the user's details (Username, UID)
//----------------------------------------------------------------
void	ClientConnectionToChatService::SendInitialLoginMessage( void )
{
CHATSERVICE_INITIAL_LOGIN_MSG	xMsg;
//char	acString[256];
const char*		pcSessionKey = UserLoginClientGetSessionKey();

	if ( ( pcSessionKey ) &&
		 ( pcSessionKey[0] != 0 ) )
	{
		memset( &xMsg, 0, sizeof( xMsg ) );

		xMsg.bMsgID = NCHATMSGID_INITIAL_LOGIN;
	//	UserSetSessionID( ulSessionID );
		xMsg.bProtocolVersion = 1;
		PlatformGetMACaddress( &xMsg.acMACAddress[0] );
		strcpy(	xMsg.szSessionKey, pcSessionKey );
		strcpy( xMsg.szLoginUserName, UserLoginClientGetUserName() ); 
		xMsg.ullUserUID = UserLoginClientGetUserUID();

		SendMessageGuaranteed( (byte*)&xMsg, sizeof( xMsg ) );
	}
	else
	{
		m_state = NO_CREDENTIALS;
	}
}

//----------------------------------------------------------------
// ClientConnectionToChatService::SendInitialConnectMessage
//  Called to initiate a guaranteed connection with the chat service
//----------------------------------------------------------------
void	ClientConnectionToChatService::SendInitialConnectMessage( void )
{
const char*		pcSessionKey = UserLoginClientGetSessionKey();

	m_BaseErrorCode = 0;
	m_fConnectWaitTime = 0.0f;

	if ( ( pcSessionKey ) &&
		 ( pcSessionKey[0] != 0 ) )
	{
		// Using TCP connections
		if ( mbUsingTcpForChatServiceConnection )
		{
			InitialiseGuaranteed( TRUE, m_ulBaseIP, m_uwBasePort + 1 );		// TCP version
			m_state = LOGGING_IN;
		}
		else
		{
			// Using custom UDP guaranteed
			char	szBuffer[256];
			int		nMsgLen;

			// TODO - send  
				//		"CLNTLOG1"
				//  along with our details to the island server address to agree on the initial connection
			sprintf( szBuffer, "CLNTLOG1" );
			nMsgLen = strlen( szBuffer ) + 1;

			if ( SetOutboundMessage( szBuffer, nMsgLen ) )
			{
				m_state = LOGGING_IN;
			}
			else
			{
				NetworkingDebugPrint( "NCS Connect error" );
				m_state = CONNECT_ERROR;
			}
			// once thats done, call  InitialiseGuaranteed  to start off the PeerConnection
		}
	}
	else
	{
		NetworkingDebugPrint( "Can't login to chat service - no user identity" );
		m_state = NO_CREDENTIALS;
	}
}

void	ClientConnectionToChatService::Destroy( void )
{
	SAFE_DELETE( m_spClientConnectionToChatServiceSingleton );
}




ulong		mulChatServiceInitialLocation = 0;
ushort		muwChatServiceInitialPort = 0;

//----------------------------------------------------------------
// ClientConnectionToChatService::Create
//
//----------------------------------------------------------------
void	ClientConnectionToChatService::Create( ulong ulChatServiceIP, ushort uwChatServicePort )
{
//ulong	ulNewChatServerIP = NetworkGetIPAddress( "127.0.0.1" );
ulong	ulNewChatServerIP;	
ushort	uwNewChatServerPort = uwChatServicePort;

	m_spClientConnectionToChatServiceSingleton = new ClientConnectionToChatService;
	m_spClientConnectionToChatServiceSingleton->SetUsePingTracking( FALSE );

	if ( ulChatServiceIP != 0 )
	{
		ulNewChatServerIP = ulChatServiceIP;
		uwNewChatServerPort = uwChatServicePort;
		mulChatServiceInitialLocation = ulNewChatServerIP;
		muwChatServiceInitialPort = uwNewChatServerPort;
	}
	else
	{
		// use last provided location
		ulNewChatServerIP = mulChatServiceInitialLocation;
		uwNewChatServerPort = muwChatServiceInitialPort;
	}
	m_spClientConnectionToChatServiceSingleton->m_ulBaseIP = ulNewChatServerIP;
	m_spClientConnectionToChatServiceSingleton->m_uwBasePort = uwNewChatServerPort;

	m_spClientConnectionToChatServiceSingleton->SendInitialConnectMessage();
	m_spClientConnectionToChatServiceSingleton->m_fConnectWaitTime = 0.0f;

}


