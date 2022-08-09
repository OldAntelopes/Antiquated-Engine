
//#define INCLUDE_PEER_STREAM

#include <stdio.h>
#include <UnivSocket.h>

#include <StandardDef.h>

#include "../Networking.h"
#include "../NetworkConnection.h"

#ifdef MARMALADE
#include "s3e.h"
#else
#include "../TCP4u/TCP4u.h"
#endif

#include "ConnectionManager.h"
#include "PeerStream.h"
#include "PeerFileTransfer.h"

//#define TRACE_FILE_TRANSFER


#define	PEERCONN_DEFAULT_MESSAGE_BUFFER_SIZE				64

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------

class BufferedMessage
{
public:
	BufferedMessage( PeerConnection* pConnection )
	{
		mpConnection = pConnection;
		mpbMsg = NULL;
		Reset();
	}

	~BufferedMessage()
	{
		SystemFree( mpbMsg );
	}

	void	InitBuffer( void )
	{
		mnBufferSize = PEERCONN_DEFAULT_MESSAGE_BUFFER_SIZE;
		mpbMsg = (byte*)( SystemMalloc( PEERCONN_DEFAULT_MESSAGE_BUFFER_SIZE ) );
	}

	void	SetReceiptCallback( PacketReceiptCallback fnCallback, void* param )
	{
		mfnReceiptCallback = fnCallback;
		mfnReceiptParam = param;
	}

	void	ReceiptReceived( ushort uwTimingInfo )
	{
		if ( uwTimingInfo != 0 )
		{
		ulong		ulCurrentTick = SysGetTick();
		ulong		ulSentTick = ulCurrentTick;
			ulSentTick &= 0xFFFF0000;
			ulSentTick |= uwTimingInfo;
			if ( ulSentTick < ulCurrentTick )
			{
			ulong	ulPingTime = ulCurrentTick - ulSentTick;
//								if ( givebirth>here ) babyHead = sizeof ( small_header );
				if ( ulPingTime < 20000 )
				{
					if ( ( mpConnection ) &&
						 ( mpConnection->IsConnected() ) )
					{
						mpConnection->AddPingTime( (ushort)(ulPingTime) );
					}
					
				}
			}
		}
		if ( mfnReceiptCallback != NULL )
		{
			mfnReceiptCallback(mfnReceiptParam);
			mfnReceiptCallback = NULL;
		}
		mulLastSentTick = 0;
	}

	void	Reset( void )
	{
		mulLastSentTick = 0;
		mfnReceiptCallback = NULL;
		mnMsgLen = 0;
		mulLastSentTick = 0;
		mnRepeatSends = 0;
	}

	void	AddToGuaranteedBuffer( short wMsgID, byte* pbMsg, int nLen )
	{
	byte*	pbMsgStore;
	int		nBufferUse = nLen + sizeof( PeerConnection::PEERCONN_GUARANTEED_MSG_HEADER );

		if ( nBufferUse > mnBufferSize )
		{
			if ( mpbMsg )
			{
				SystemFree( mpbMsg );
			}
			mpbMsg = (BYTE*)SystemMalloc( nBufferUse );
			if ( !mpbMsg )
			{
				NetworkingUserDebugPrint( 0, "PConn Error : No memory available for message buffers" );
				return;
			}
			mnBufferSize = nBufferUse;
		}

		PeerConnection::PEERCONN_GUARANTEED_MSG_HEADER*	pxHeader = (PeerConnection::PEERCONN_GUARANTEED_MSG_HEADER*)mpbMsg;
		pxHeader->bMsgCode = UDP_PEER_CONNECTION_GUARANTEED;
		pxHeader->wMsgID = wMsgID;
		pxHeader->wMsgSize = (short)( nLen );
		pxHeader->bHeaderSize = sizeof( PeerConnection::PEERCONN_GUARANTEED_MSG_HEADER );
		pbMsgStore = (byte*)( pxHeader + 1 );
		memcpy( pbMsgStore, pbMsg, nLen );
		mnMsgLen = nLen + sizeof( PeerConnection::PEERCONN_GUARANTEED_MSG_HEADER );
		mulLastSentTick = 1;
		mnRepeatSends = 0;
		// call update to trigger the initial send
		Update();
	}

	BOOL	Update( void )
	{
		if ( !mpConnection ) return( FALSE );

		if ( mpConnection->IsConnected() )
		{
			if ( mulLastSentTick != 0 )
			{
			ulong	ulTimeSinceLastSend = SysGetTick() - mulLastSentTick;
			ulong	ulDelayTime = 2500;
		
				if ( ulTimeSinceLastSend > ulDelayTime )
				{
				PeerConnection::PEERCONN_GUARANTEED_MSG_HEADER*	pxHeader = (PeerConnection::PEERCONN_GUARANTEED_MSG_HEADER*)mpbMsg;

					// We don't resend on specific counts..
					// RepeatSends = 0 - Initial send		(0s)
					//               1 - First resend	    (2.5s)
					//				 3 - Second resend		(7.5s)
					//				 4 - Third resend		(10s)
					//				 6 - Fourth resend		(15s)
					//				 9 - Fifth resend		(22.5s)
					if ( ( mnRepeatSends == 0 ) ||			// Comment out this line for a simple fake packetloss
						 ( mnRepeatSends == 1 ) ||
						 ( mnRepeatSends == 3 ) ||
						 ( mnRepeatSends == 4 ) ||
						 ( mnRepeatSends == 6 ) ||
						 ( mnRepeatSends == 8 ) )
					{
						// Resend the message
						pxHeader->uwTimingInfo = (ushort)( SysGetTick() & 0xFFFF );
						NetworkConnectionGenericSendMessage( (char*)mpbMsg, mnMsgLen, mpConnection->m_ulIP, mpConnection->m_uwPort, TRUE );
					}

					mulLastSentTick = SysGetTick();
					mnRepeatSends++;

//					NetworkingUserDebugPrint( 0, "PConn:sent id %d to %s (%d)", pxHeader->wMsgID, inet_ntoa(*((struct in_addr*)&xSendAddress.sin_addr)), ntohs( xSendAddress.sin_port ) );

					// When remote user disconnects abruptly, the time before we realise is the delaytime * the number of repeat sends
					// i.e. Delaytime = 2.5 secs, 10 repeats means the remote user is kicked off after 25 seconds if they
					//  don't respond to a guaranteed msg
					if ( mnRepeatSends > 9 )		
					{
						// disconnection..
						NetworkingUserDebugPrint( 0, "P2P Conn Closed %s:%d - No Response", NetworkGetIPAddressText( mpConnection->GetIP() ), mpConnection->GetPort() );
						mpConnection->CloseConnection( FALSE );
						mulLastSentTick = 0;
					}
					return( TRUE );
				}

			}
		}
		return( FALSE );
	}

	BOOL	IsInUse( void ) { if ( mulLastSentTick != 0 ) return TRUE; return FALSE; }
private:
	ulong	mulLastSentTick;
	int		mnRepeatSends;
	byte*	mpbMsg;
	int		mnBufferSize;
	int		mnMsgLen;
	PacketReceiptCallback	mfnReceiptCallback;
	void*	mfnReceiptParam;
	PeerConnection* mpConnection;
};

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------

void PeerConnection::Reset( void )
{
int		Loop;

	m_ConnectionState = PEER_CONNECTION_STATE_NONE;
	m_ulIP = 0;
	m_uwPort = 0;
	mbNextSendID = 0;
	mpFileTransferRecords = NULL;
	mpStreamIn = NULL;
	mpStreamOut = NULL;

	for ( Loop = 0; Loop < NUM_PING_RECORDS; Loop++ )
	{
		mauwPingTimes[Loop] = 0;
	}
	ZeroMemory( m_abReceiptIDs, MAX_MSG_IDS*sizeof(BYTE) );
	mnNextPingSlot = 0;
	mulAveragePingTime = 0;
	mulLastDataSentTick = 0;
	mwNextPeerFileTransferFileID = 0;
	mulRemoteTick = 0;
	m_hTcpSocket = 0;
	mhTCPConnectThread = 0;
	mbClosingConnection = FALSE;

	for ( Loop = 0; Loop < MAX_BUFFERED_MESSAGES; Loop++ )
	{
		mapMessageBuffers[Loop]->Reset();
	}
}

PeerConnection::PeerConnection()
{
int		Loop;

	mbUseOutboundKeepalive = TRUE;
	mbUsePingTracking = TRUE;

	for ( Loop = 0; Loop < MAX_BUFFERED_MESSAGES; Loop++ )
	{
		mapMessageBuffers[Loop] = new BufferedMessage( this );
		mapMessageBuffers[Loop]->InitBuffer();
	}
	
	Reset();
	ConnectionManager::Get().AddConnection( this );
}

PeerConnection::~PeerConnection()
{
int		Loop;

	CloseConnection( FALSE );

	ConnectionManager::Get().RemoveConnection( this );
		
	for ( Loop = 0; Loop < MAX_BUFFERED_MESSAGES; Loop++ )
	{
		delete mapMessageBuffers[Loop];
	}
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void	PeerConnection::AddPingTime( ulong ulPing )
{
int		nLoop;
int		nCount = 0;

	RegisterKeepaliveReceived();

	mauwPingTimes[mnNextPingSlot] = (ushort)( ulPing );
	mnNextPingSlot ++;
	mnNextPingSlot %= NUM_PING_RECORDS;

	for ( nLoop = 0; nLoop < NUM_PING_RECORDS; nLoop++ )
	{
		if ( mauwPingTimes[nLoop] != 0 )
		{
			mulAveragePingTime += mauwPingTimes[nLoop];
			nCount++;
		}
	}

	if ( nCount > 0 )
	{
		mulAveragePingTime /= nCount;
	}
}

typedef struct
{
	BYTE	bCode1;
	BYTE	bCode2;
	short	wMsgLen;

} TCPMSG_HEADER;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL	PeerConnection::SendMessageGuaranteed( byte* pbMsg, int nLen )
{
	if ( nLen > 0 )
	{
#ifdef MARMALADE
		if ( 0 )
		{
			// MARMALADE TODO - TCP PeerConnection support

		}
#else
		if ( m_hTcpSocket != 0 )
		{
		char	acSendBuff[4096];
		TCPMSG_HEADER*	pxHeader = (TCPMSG_HEADER*)( acSendBuff );
		int		nMsgLength = nLen + sizeof( TCPMSG_HEADER );
		int		ret;

			pxHeader->bCode1 = 0x56;
			pxHeader->bCode2 = 0x72;
			pxHeader->wMsgLen = (short)( nLen );
			memcpy( (void*)( pxHeader+1), pbMsg, nLen );
			NetworkConnectionScrambleMessage( (byte*)(pxHeader+1), nLen );
			ret = TcpSend( m_hTcpSocket, acSendBuff, nMsgLength, FALSE, HFILE_ERROR );
			if ( ret != TCP4U_SUCCESS )
			{
				// todo - disconnect?
			}
			return( TRUE );
		}
#endif
		else
		{
		short	wMsgID = mbNextSendID++;
			mbNextSendID %= MAX_MSG_IDS;
			mapMessageBuffers[(wMsgID%MAX_BUFFERED_MESSAGES)]->AddToGuaranteedBuffer( wMsgID, pbMsg, nLen );
			mulLastDataSentTick = SysGetTick();
			return( TRUE );
		}
	}
	return( FALSE );
}

BOOL	PeerConnection::IsBusy()
{
	if ( mhTCPConnectThread != 0 )
	{
		return( TRUE );
	}
	else
	{
	int		nLoop;

		for ( nLoop = 0; nLoop < MAX_BUFFERED_MESSAGES; nLoop++ )
		{
			if ( mapMessageBuffers[nLoop]->IsInUse() )
			{
				return( TRUE );
			}
		}
	}
	return( FALSE );
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL	PeerConnection::SendMessageGuaranteedEx( byte* pbMsg, int nLen, PacketReceiptCallback fnReceiptCallback, void* param )
{
	if ( nLen > 0 )
	{
#ifdef MARMALADE
		if ( 0 )
		{
			// MARMALADE TODO - TCP PeerConnection support

		}
#else
		if ( m_hTcpSocket != 0 )
		{
		char	acSendBuff[8192];
		TCPMSG_HEADER*	pxHeader = (TCPMSG_HEADER*)( acSendBuff );
		int		nMsgLength = nLen + sizeof( TCPMSG_HEADER );

			pxHeader->bCode1 = 0x56;
			pxHeader->bCode2 = 0x72;
			pxHeader->wMsgLen = (short)( nLen );
			memcpy( (void*)( pxHeader+1), pbMsg, nLen );
			NetworkConnectionScrambleMessage( (byte*)(pxHeader+1), nLen );

			TcpSend( m_hTcpSocket, acSendBuff, nMsgLength, FALSE, HFILE_ERROR );
			if ( fnReceiptCallback )
			{
				fnReceiptCallback(param);
			}
			return( TRUE );
		}
#endif
		else
		{
			short	wMsgID = mbNextSendID++;
			mbNextSendID %= MAX_MSG_IDS;
			mapMessageBuffers[(wMsgID%MAX_BUFFERED_MESSAGES)]->AddToGuaranteedBuffer( wMsgID, pbMsg, nLen );
			mapMessageBuffers[(wMsgID%MAX_BUFFERED_MESSAGES)]->SetReceiptCallback( fnReceiptCallback, param );
			mulLastDataSentTick = SysGetTick();
			return( TRUE );
		}
	}
	return( FALSE );
}



BOOL	PeerConnection::IsReceivingFile()
{
FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;

	// Update all file transfers
	while( pFileTransferRecords )
	{
		if ( !pFileTransferRecords->mpFileTransfer->mboIsSend )
		{
			return( TRUE );
		}
		pFileTransferRecords = pFileTransferRecords->mpNext;
	}
	return( FALSE );
}

BOOL	PeerConnection::IsSendingFile()
{
FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;

	// Update all file transfers
	while( pFileTransferRecords )
	{
		if ( pFileTransferRecords->mpFileTransfer->mboIsSend )
		{
			return( TRUE );
		}
		pFileTransferRecords = pFileTransferRecords->mpNext;
	}
	return( FALSE );
}



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
FileTransferRecord*	PeerConnection::AddNewFileTransfer( void )
{
FileTransferRecord*		pNewRecord = new FileTransferRecord();

	pNewRecord->mpFileTransfer = new FileTransfer( this );

	// Add to list
	pNewRecord->mpNext = mpFileTransferRecords;
	mpFileTransferRecords = pNewRecord;
	return( pNewRecord );
}



void	PeerConnection::StripFilename( const char* pcFilename, char* szBuffer )
{
char	acString[256];
const char*	pcFound;

	SysGetCurrentDir( 256, acString );
	pcFound = strstr( pcFilename, acString );
	if ( pcFound )
	{
		if ( pcFound == pcFilename )
		{
			pcFilename += strlen( acString ) + 1;
		}
	}
	strcpy( szBuffer, pcFilename );
}


int		PeerConnection::CountActiveFileTransfers( void )
{
FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;
int		nCount = 0;

	while( pFileTransferRecords )
	{
		nCount++;
		pFileTransferRecords = pFileTransferRecords->mpNext;
	}
	return( nCount );
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL	PeerConnection::RequestFile( const char* pcFilename )
{
char	acFilenameBuffer[256];
#ifdef TRACE_FILE_TRANSFER
	NetworkingUserDebugPrint( 0, "requesting file %s\n", pcFilename );
#endif
	if ( ( !pcFilename ) ||
		 ( pcFilename[0] == 0 ) )
	{
		return( FALSE );
	}
	// if the file is a full file path relative to our current directory (it should be)
	// strip the root directory (e.g.  convert c:\program files\theuniversal\clients\data\custom\file.jpg  to data\custom\file.jpg )
	StripFilename( pcFilename, acFilenameBuffer );

	// Send file request
	SendSysMessageEx( 3, 0, 0, (void*)acFilenameBuffer, strlen( acFilenameBuffer ) + 1 );
	return( TRUE );
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL	PeerConnection::SendFile( const char* pcFilename, const char* pcReceiverFilename )
{
short	wFileID;
FILE*	pFile;
int		nFileSize;
byte*	pbMem;
FileTransferRecord*		pFileTransferRecord;
char	acFilenameBuffer[256];

	if ( ( !pcFilename ) ||
		 ( pcFilename[0] == 0 ) )
	{
		return( FALSE );
	}

	// if the file is a full file path relative to our current directory (it should be)
	// strip the root directory (e.g.  convert c:\program files\theuniversal\clients\data\custom\file.jpg  to data\custom\file.jpg )
	StripFilename( pcFilename, acFilenameBuffer );

	if ( OnSendFileFromMem( pcFilename, &pbMem, &nFileSize ) == TRUE )
	{
		pFileTransferRecord = AddNewFileTransfer();
		if ( pFileTransferRecord )
		{
			wFileID = mwNextPeerFileTransferFileID++;
			pFileTransferRecord->mpFileTransfer->InitialiseSend( wFileID, pbMem, nFileSize, acFilenameBuffer, pcReceiverFilename );
		}
		else
		{
			NetworkingUserDebugPrint(0, "PeerConn::SendFile - Error creating transfer from mem" );
		}
		return( TRUE );
	}
	else
	{
		pFile = fopen( acFilenameBuffer, "rb" );
		if ( pFile != NULL )
		{
			nFileSize = SysGetFileSize( pFile );
			// For any files under 16k, just load em straight into mem and send em that way
			if ( nFileSize < (16*1024) )
			{
				pbMem = (BYTE*)SystemMalloc( nFileSize );
				fread( pbMem, nFileSize, 1, pFile );
				fclose( pFile );

				pFileTransferRecord = AddNewFileTransfer();
				if ( pFileTransferRecord )
				{
					wFileID = mwNextPeerFileTransferFileID++;
					pFileTransferRecord->mpFileTransfer->InitialiseSend( wFileID, pbMem, nFileSize, acFilenameBuffer, pcReceiverFilename );
				}
				else
				{
					NetworkingUserDebugPrint(0, "PeerConn::SendFile - Error creating transfer from mem" );
				}
			}
			else
			{
				fclose( pFile );
				pFileTransferRecord = AddNewFileTransfer();
				if ( pFileTransferRecord )
				{
					wFileID = mwNextPeerFileTransferFileID++;
					if ( !pFileTransferRecord->mpFileTransfer->InitialiseSend( wFileID, NULL, nFileSize, acFilenameBuffer, pcReceiverFilename ) )
					{
						NetworkingUserDebugPrint(0, "PeerConn::SendFile - Error in InitialiseSend" );
					}
				}
				else
				{
					NetworkingUserDebugPrint(0, "PeerConn::SendFile - Error creating transfer from file" );
				}

			}
			return( TRUE );
		}
		else
		{
			// Send a peerconn sysmessage back so the requester can clear up
			SendSysMessageEx( 4, -2, 0, (void*)acFilenameBuffer, strlen( acFilenameBuffer ) + 1 );
			NetworkingUserDebugPrint( 0, "PeerConn::SendFile - Couldnt open file for send (%s)", pcFilename );
			NetworkingUserDebugPrint( 0, "(%s)", acFilenameBuffer );
		}
	}
	return( FALSE );
}


void	PeerConnection::SendCloseConnectionMessage( void )
{

	PEER_STATUS_MSG		xReply;
	memset( &xReply, 0, sizeof( xReply ) );
	xReply.bUDPCode = UDP_PEER_CONNECTION_SYS;
	xReply.bMsgCode = PEER_SYSMSG_CLOSECONNECTION;
	xReply.bSizeOfMsg = sizeof( xReply );
	xReply.bParamCode = 1;
	xReply.bConnectionCode = 0;
	xReply.ulLocalTimer = SysGetTick();
	xReply.ulResponseTimer = 0;

	NetworkConnectionGenericSendMessage( (char*)( &xReply ), xReply.bSizeOfMsg, m_ulIP, m_uwPort, FALSE );
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void	PeerConnection::CloseConnection( BOOL bSendCloseMessage )
{
	if ( mbClosingConnection == FALSE )
	{
		mbClosingConnection = TRUE;
	
		if ( bSendCloseMessage )
		{
			SendCloseConnectionMessage();
		}

	#ifdef MARMALADE
		// MARMALADE TODO - TCP PeerConnection support

	#else
		if ( m_hTcpSocket != 0 )
		{
			TcpClose( &m_hTcpSocket );
		}
	#endif

		if ( mpStreamOut )
		{	
	#ifdef NETWORK_AUDIO
			VoiceComms::Get().CloseVoiceCommsTo( this );
	#endif
			delete mpStreamOut;
			mpStreamOut = NULL;
		}

		if ( mpStreamIn )
		{
			delete 	mpStreamIn;
			mpStreamIn = NULL;
		}

		// Close all file transfers
		FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;
		FileTransferRecord*		pNextRecord;

		while( pFileTransferRecords )
		{
			// Remove from list
			pNextRecord = pFileTransferRecords->mpNext;
			CloseFileTransfer(pFileTransferRecords);
			pFileTransferRecords = pNextRecord;
		}

		if ( m_ConnectionState != PEER_CONNECTION_STATE_NONE )
		{
			m_ConnectionState = PEER_CONNECTION_STATE_NONE;
			OnDisconnected();
		}
		Reset();
	}
}

//---------------------------------------------------------------------------
//	RemoteInitialiseGuaranteed
//  Called in response to a msg from a remote player attempting to initiate a guaranteed connection
//  with this client
//---------------------------------------------------------------------------
BOOL	PeerConnection::RemoteInitialiseGuaranteedUDP( ulong ulIP, ushort uwPort, ulong ulLocalTimer )
{
	// TODO - Here we need to go into "PEER_CONNECTION_INITIALISING_GUARANTEED" until
	// we get our first guaranteed message back from the remote host
	// Only then should we move to connected.
	SetConnected();
	SetIPAndPort( ulIP, uwPort );

	// Send init message back to confirm the connection is active
	PEER_STATUS_MSG		xReply;
	memset( &xReply, 0, sizeof( xReply ) );
	xReply.bUDPCode = UDP_PEER_CONNECTION_SYS;
	xReply.bMsgCode = PEER_SYSMSG_GUARANTEEDINIT;
	xReply.bSizeOfMsg = sizeof( xReply );
	xReply.bParamCode = 1;				// ParamCode = 1, tells the remote client we're all set
	xReply.bConnectionCode = (BYTE)( GetConnectionCode() );
	xReply.ulLocalTimer = SysGetTick();
	xReply.ulResponseTimer = ulLocalTimer;
//	DebugPrint( 0, "RemoteInitialiseGuaranteed sending systick %d", xReply.ulLocalTimer );
	NetworkConnectionGenericSendMessage( (char*)( &xReply ), xReply.bSizeOfMsg, ulIP, uwPort, FALSE );
	return( TRUE );
}


void	PeerConnection::HandleFileReceiveComplete( short wFileID, const char* szFilename, byte* pbMsg, int nMemSize )
{
FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;

	OnFileReceiveComplete( wFileID, szFilename, pbMsg, nMemSize );

	if ( wFileID != -1 )
	{
		// Update all file transfers
		while( pFileTransferRecords )
		{
			if ( !pFileTransferRecords->mpFileTransfer->mboIsSend )
			{
				if ( pFileTransferRecords->mpFileTransfer->mwFileID == wFileID )
				{
					CloseFileTransfer(pFileTransferRecords);
					return;
				}
			}
			pFileTransferRecords = pFileTransferRecords->mpNext;
		}
	}
}

//---------------------------------------------------------------------------
// RemoteInitResponse
//  Acknowledgement received from remote client indicating a guaranteed connection has been 'agreed on'
//---------------------------------------------------------------------------
void	PeerConnection::RemoteInitResponseUDP( ulong ulIP, ushort uwPort, ulong ulRemoteTick, ulong ulOriginalTick )
{
	SetIPAndPort( ulIP, uwPort );
	SetConnected();

	unsigned long ulPing = SysGetTick() - ulOriginalTick;	// How long from the original initGuaranteed to its reply

	unsigned long ulGlobalTimer = ulRemoteTick + (ulPing / 2 );		// Our initial estimate of the tick count on the server as we receive this message

	mulRemoteTick = ulGlobalTimer;
//	NetworkingUserDebugPrint( 0, 0, "Server global tick count @ %d\n", ulGlobalTimer );

	// Now we've got a guaranteed connection.. send a GUAR so the remote can complete the connection state
	BYTE	abBuff[4];

	RegisterKeepaliveReceived();
	mulLastDataSentTick = SysGetTick();
	// send a wee message as a keepalive (and to keep the pingtime updated
	memset( abBuff, 0, 4 );
	abBuff[0] = PEER_GUARANTEED_KEEPALIVE;
	abBuff[2] = 4;
	SendMessageGuaranteed( abBuff, 4 );
}

void	PeerConnection::SetConnected()
{
	RegisterKeepaliveReceived();
	m_ConnectionState = PEER_CONNECTION_STATE_RESPONSIVE; 
	mulLastDataSentTick = SysGetTick();
	OnConnected();
}

void	PeerConnection::OnRemoteTCPConnected( SOCKET hSocket )
{
	m_hTcpSocket = hSocket;
	SetConnected();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void	PeerConnection::CloseFileTransfer( FileTransferRecord* pRecord )
{
	if ( pRecord->mpFileTransfer )
	{
		delete( pRecord->mpFileTransfer );
	}

	if ( mpFileTransferRecords == pRecord )
	{
		mpFileTransferRecords = pRecord->mpNext;
	}
	else
	{
	FileTransferRecord* pPrev = mpFileTransferRecords;

		while ( pPrev )
		{
			if ( pPrev->mpNext == pRecord )
			{
				pPrev->mpNext = pRecord->mpNext;
				break;
			}
			pPrev = pPrev->mpNext;
		}
	}
	delete( pRecord );
}

void	PeerConnection::OnFileReceiveError( short wFileID, const char* szFilename, int nErrorCode )
{
	HandleFileReceiveComplete( wFileID, szFilename, NULL, nErrorCode );

}
void	PeerConnection::RegisterKeepaliveReceived( void )
{
	mulLastResponseTick = SysGetTick();
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void	PeerConnection::UpdateConnection( float fDeltaTime )
{
int		nLoop;
int		nMsgsSent = 0;
FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;
FileTransferRecord*		pNextRecord;

	if ( mulRemoteTick != 0 )
	{
		mulRemoteTick += (ulong)(fDeltaTime * 1000.0f);
	}

	// Update all file transfers
	while( pFileTransferRecords )
	{
		if ( pFileTransferRecords->mpFileTransfer->Update(fDeltaTime) == FALSE )
		{
			NetworkingUserDebugPrint( 0, "File send completed\n");
			// Remove from list
			pNextRecord = pFileTransferRecords->mpNext;
			CloseFileTransfer(pFileTransferRecords);
			pFileTransferRecords = pNextRecord;
		}
		else
		{
			pFileTransferRecords = pFileTransferRecords->mpNext;
		}
	}

	for ( nLoop = 0; nLoop < MAX_BUFFERED_MESSAGES; nLoop++ )
	{
		if ( mapMessageBuffers[nLoop]->Update() )
		{
			nMsgsSent++;
		}
	}
	if ( nMsgsSent != 0 )
	{
		mulLastDataSentTick = SysGetTick();
	}

#ifdef MARMALADE
	// MARMALADE TODO - TCP PeerConnection support
	if ( 0 )
	{



	}
#else
	if ( m_ConnectionState == PEER_CONNECTION_INITIALISING_GUARANTEED )
	{
		switch( mnTCPConnectJobComplete )
		{
		case 1:
			SetConnected();
			break;
		case -1:
			m_ConnectionState = PEER_CONNECTION_STATE_UNRESPONSIVE;
			break;
		case 0:
		default:
			break;
		}
	}

	if ( ( IsConnected() ) &&
		 ( m_hTcpSocket != 0 ) )
	{
	char	acBuff[4096];
	TCPMSG_HEADER*	pxHeader = (TCPMSG_HEADER*)(acBuff);
	int		nRecvRet; 

		nRecvRet = TcpRecv(m_hTcpSocket, acBuff, 4096, TCP4U_DONTWAIT, HFILE_ERROR );

		if ( nRecvRet >= TCP4U_SUCCESS )
		{
		int		nBytesInBuffer = nRecvRet;
		int		nBytesRemainingInBuffer = nRecvRet;
		int		nMsgLen;
		byte*	pbMsg;

			// we may have partial or multiple packets in this buffer?
			while( ( pxHeader->bCode1 == 0x56 ) &&
				   ( pxHeader->bCode2 == 0x72 ))
			{
				nBytesRemainingInBuffer -= sizeof( TCPMSG_HEADER );

				nMsgLen = pxHeader->wMsgLen;
				pbMsg = (byte*)( pxHeader+1 );

				if ( nMsgLen > nBytesRemainingInBuffer )
				{
					// we have a partial packet?

					// TODO?
					NetworkingUserDebugPrint( 0, "Partial TCP packet detected.. please tell mit" );
				}

				NetworkConnectionUnscrambleMessage( pbMsg, nMsgLen );
				DecodeGuaranteedMessagePacket( pbMsg );

				pxHeader = (TCPMSG_HEADER*)( pbMsg + nMsgLen );
			}
		}
		else if ( ( nRecvRet == TCP4U_EMPTYBUFFER ) ||
				  ( nRecvRet == TCP4U_TIMEOUT ) )
		{
			// No data waiting.. continue..
		}
		else if ( nRecvRet == TCP4U_SOCKETCLOSED )
		{
			// Socket closed remotely
			CloseConnection(0);
		}
		else	// Error or connection closed?
		{
			SysUserPrint( 0, "*** TCP Socket Error (Ret %d)\n", nRecvRet );
			CloseConnection(0);
		}
	}
#endif
	else if ( IsConnected() )
	{
		if ( mbUseOutboundKeepalive )
		{
			// If not sent anything for 'ages'  (30 sec)
			if ( SysGetTick() - mulLastDataSentTick > (30*1000) )
			{
			BYTE	abBuff[4];

				if ( IsBusy() == FALSE )
				{
					mulLastDataSentTick = SysGetTick();

					if ( mbUsePingTracking )
					{
						memset( abBuff, 0, 4 );
						abBuff[0] = PEER_GUARANTEED_KEEPALIVE;
						abBuff[2] = 4;
						SendMessageGuaranteed( abBuff, 4 );
					}
					else
					{
						SendSysKeepalive();
					}
				}
			}
		}
		else
		{
			if ( SysGetTick() - mulLastResponseTick > (65*1000) )
			{
			BYTE	abBuff[4];

				mulLastResponseTick = SysGetTick();
				
				// If we haven't received anything for over a minute, send em a guaranteed keepalive
				// (If its not acknowledged, the connection will be closed when the guaranteedmessage fails)
				memset( abBuff, 0, 4 );
				abBuff[0] = PEER_GUARANTEED_KEEPALIVE;
				abBuff[2] = 4;
				SendMessageGuaranteed( abBuff, 4 );
			}
		}
	}
	OnUpdate( fDeltaTime );
}

void	PeerConnection::GetFileReceiveProgress( int nIndex, int* pnCurrent, int* pnSize, float* pfTransferTime, char* szFilename, int nBufferLen )
{
FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;
	*pnCurrent = 0;
	*pnSize = 0;
	szFilename[0] = 0;

	// Update all file transfers
	while( pFileTransferRecords )
	{
		if ( !pFileTransferRecords->mpFileTransfer->mboIsSend )
		{
			pFileTransferRecords->mpFileTransfer->GetFileTransferProgress( pnCurrent, pnSize, pfTransferTime, szFilename, nBufferLen );
			return;
		}
		pFileTransferRecords = pFileTransferRecords->mpNext;
	}
}


void		PeerConnection::ConnectionDecodeFileSendAcknowledge( short wFileID )
{
FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;

	// Update all file transfers
	while( pFileTransferRecords )
	{
		if ( pFileTransferRecords->mpFileTransfer->mboIsSend )
		{
			if ( pFileTransferRecords->mpFileTransfer->mwFileID == wFileID )
			{
				pFileTransferRecords->mpFileTransfer->DecodeFileSendAcknowledge();
				return;
			}
		}
		pFileTransferRecords = pFileTransferRecords->mpNext;
	}
}

void		PeerConnection::ConnectionDecodeFilePacket( byte* pbMsg )
{
PEER_FILE_PACKET_MSG*	pxMsg = (PEER_FILE_PACKET_MSG*)(pbMsg);
FileTransferRecord*		pFileTransferRecords = mpFileTransferRecords;

	// Update all file transfers
	while( pFileTransferRecords )
	{
		if ( !pFileTransferRecords->mpFileTransfer->mboIsSend )
		{
			if ( pFileTransferRecords->mpFileTransfer->mwFileID == pxMsg->wFileID )
			{
				pFileTransferRecords->mpFileTransfer->DecodePacketReceived(pbMsg);
				return;
			}
		}
		pFileTransferRecords = pFileTransferRecords->mpNext;
	}

	NetworkingUserDebugPrint( 0,"PeerConnection Error: File packet received but no transfer-receive active" );
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void		PeerConnection::DecodeGuaranteedMessagePacket( byte* pbMsgBody )
{
	// process the msg
	if ( pbMsgBody[0] == PEER_GUARANTEED_SYS_RESERVE )
	{
	PEER_SYS_MSG* pxMsg = (PEER_SYS_MSG*)(pbMsgBody);
	FileTransferRecord*		pFileTransferRecord;

		switch( pxMsg->bFileInfoMode )
		{
		case 1:	// This is starting a new send.
			pFileTransferRecord = AddNewFileTransfer();
			if ( pFileTransferRecord )
			{
				if ( pxMsg->bFields == 1 )
				{
#ifdef TRACE_FILE_TRANSFER
					NetworkingUserDebugPrint( 0, "transfer receive initialised (fileID %d - %s)\n", pxMsg->wFileID, (const char*)( pxMsg + 1 ) );
#endif
					pFileTransferRecord->mpFileTransfer->InitialiseReceive( pxMsg->wFileID, pxMsg->ulChecksum, (const char*)( pxMsg + 1 ) );
				}
				else
				{
#ifdef TRACE_FILE_TRANSFER
					NetworkingUserDebugPrint( 0, "transfer receive initialised (fileID %d)\n", pxMsg->wFileID );
#endif
					pFileTransferRecord->mpFileTransfer->InitialiseReceive( pxMsg->wFileID, pxMsg->ulChecksum, NULL );
				}
			}
			break;
		case 2:
			ConnectionDecodeFileSendAcknowledge( pxMsg->wFileID );
			break;
		case 3:		// Requesting a file
			{
			const char*		pcFilename = (const char*)( pxMsg + 1 );
					
				// Let implementation of PeerConnection decide if file send should happen
				if ( OnFileRequest( pcFilename ) == TRUE )
				{
					SendFile( pcFilename, pcFilename );
				}
			}
			break;
		case 4:		// File request failed (missing file etc)
			{
			const char*		pcFilename = (const char*)( pxMsg + 1 );
#ifdef TRACE_FILE_TRANSFER
				NetworkingUserDebugPrint( 0, "cancelled transfer - missing file (%s)\n", pcFilename );
#endif
				OnFileReceiveError( -1, pcFilename, pxMsg->wFileID );	// FileID contains error code
			}
			break;
		}
	}
	else if ( pbMsgBody[0] == PEER_GUARANTEED_FILE_PACKET )
	{
		ConnectionDecodeFilePacket( pbMsgBody );
	}
	else if ( pbMsgBody[0] == PEER_GUARANTEED_KEEPALIVE )
	{
		// We don't do anything else with this atm..
	}
	else
	{
		// Call the virtual function to decode all other guaranteed messages
		OnReceiveGuaranteedMsg( pbMsgBody );
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int		PeerConnection::DecodeGuaranteedMessageUDP( byte* pbMsg )
{
PEERCONN_GUARANTEED_MSG_HEADER* pxHeader = (PEERCONN_GUARANTEED_MSG_HEADER*)(pbMsg);
PEERCONN_GUARANTEED_MSG_HEADER	xReply;

	RegisterKeepaliveReceived();

	// if msg size is 0 its a receipt reply
	if ( pxHeader->wMsgSize == 0 )
	{
		mapMessageBuffers[(pxHeader->wMsgID%MAX_BUFFERED_MESSAGES)]->ReceiptReceived( pxHeader->uwTimingInfo );
	}
	else
	{
	byte*	pbMsgBody = (byte*)( pxHeader + 1 );

	// If its a repeat ignore it..
		if ( m_abReceiptIDs[(pxHeader->wMsgID % MAX_MSG_IDS)] == 1 )
		{
//			NetworkingUserDebugPrint( 0,"DecodeGuaranteedMessage : Ignored repeat guar");		
		}
		else
		{
			DecodeGuaranteedMessagePacket( pbMsgBody );
			m_abReceiptIDs[(pxHeader->wMsgID % MAX_MSG_IDS)] = 1;
			m_abReceiptIDs[((pxHeader->wMsgID+(MAX_MSG_IDS/2)) % MAX_MSG_IDS)] = 0;
		}
		// Send receipt back
		ZeroMemory( &xReply, sizeof( xReply ) );
		xReply.bHeaderSize = sizeof( xReply );
		xReply.bMsgCode = UDP_PEER_CONNECTION_GUARANTEED;
		xReply.wMsgID = pxHeader->wMsgID;
		xReply.uwTimingInfo = pxHeader->uwTimingInfo;
		NetworkConnectionGenericSendMessage( (char*)&xReply, sizeof(xReply), m_ulIP, m_uwPort, FALSE );
		mulLastDataSentTick = SysGetTick();
	}
	return( pxHeader->bHeaderSize + pxHeader->wMsgSize );
}


void	PeerConnection::SendSysMessage( BYTE bType, short wParam, ulong ulParam )
{
PEER_SYS_MSG	xMsg;
	memset( &xMsg, 0, sizeof( xMsg ) );
	xMsg.bMsgCode = PEER_GUARANTEED_SYS_RESERVE;
	xMsg.bFileInfoMode = bType;
	xMsg.wFileID = wParam;
	xMsg.ulChecksum = ulParam;
	SendMessageGuaranteed( (BYTE*)&xMsg, sizeof(xMsg) );
}

void	PeerConnection::SendSysMessageEx( BYTE bType, short wParam, ulong ulParam, void* pExtra, int nExtraLen )
{
char	acBuffer[512];
PEER_SYS_MSG*	pxMsg = (PEER_SYS_MSG*)(acBuffer);

	memset( pxMsg, 0, sizeof( *pxMsg ) );
	pxMsg->bMsgCode = PEER_GUARANTEED_SYS_RESERVE;
	pxMsg->bFileInfoMode = bType;
	pxMsg->wFileID = wParam;
	pxMsg->ulChecksum = ulParam;
	pxMsg->bFields = 1;
	memcpy( (char*)(pxMsg+1), pExtra, nExtraLen );
	SendMessageGuaranteed( (BYTE*)pxMsg, sizeof(*pxMsg) + nExtraLen );
}



void	PeerConnection::TcpConnectJob( void )
{
char	acString[256];

	// TODO - TcpConnect presumably blocks, so will need to put this in a thread
	sprintf( acString, NetworkGetIPAddressText( m_ulIP ) );
#ifdef MARMALADE
	// MARMALADE TODO - TCP PeerConnection support
	m_ConnectionState = PEER_CONNECTION_STATE_UNRESPONSIVE;
    s3eDeviceYield(1);

#else
	int		ret;

	ret = TcpConnect( &m_hTcpSocket, acString, NULL, &m_uwPort );

	if ( ret == TCP4U_SUCCESS )
	{
//		NetworkingUserDebugPrint(0, "TCP connection established" );
		mnTCPConnectJobComplete = 1;
	}
	else
	{
		m_ConnectionState = PEER_CONNECTION_STATE_UNRESPONSIVE;
		mnTCPConnectJobComplete = -1;
	}
	mhTCPConnectThread = 0;
#endif
}

long WINAPI PeerConnectionTCPConnectThread(long lParam)
{ 
PeerConnection*	pxPeerConnection = (PeerConnection*)( lParam );

	pxPeerConnection->TcpConnectJob();

	SysExitThread( 0 );
	return( 0 );
}


BOOL	PeerConnection::InitialiseGuaranteedTCP( void )
{
	mnTCPConnectJobComplete = 0;
	m_ConnectionState = PEER_CONNECTION_INITIALISING_GUARANTEED;
//	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)PeerConnectionTCPConnectThread,(LPVOID)(this),0,&iID);
	mhTCPConnectThread = SysCreateThread( (fnThreadFunction)PeerConnectionTCPConnectThread, (void*)this, 0, 0 );
	return( TRUE );
}


BOOL	PeerConnection::InitialiseGuaranteedUDP( void )
{
	// Send init message to remote player to initiate the connection
	PEER_STATUS_MSG		xReply;
	memset( &xReply, 0, sizeof( xReply ) );

	xReply.bUDPCode = UDP_PEER_CONNECTION_SYS;
	xReply.bMsgCode = PEER_SYSMSG_GUARANTEEDINIT;
	xReply.bConnectionCode = (BYTE)( GetConnectionCode() );
	xReply.bSizeOfMsg = sizeof( xReply );
	xReply.bParamCode = 0;				// ParamCode 0 says we're initiating the connection
	xReply.ulLocalTimer = SysGetTick();

	// mmm .. we're assuming they're gonna get this msg from here - no option for packetloss....... mmmm

	NetworkConnectionGenericSendMessage( (char*)( &xReply ), xReply.bSizeOfMsg, m_ulIP, m_uwPort, FALSE );
	return( TRUE );
}


BOOL	PeerConnection::InitialiseGuaranteed( BOOL bUseTCP, ulong ulIP, ushort uwPort )
{
BOOL	bRet = FALSE;

	m_ConnectionState = PEER_CONNECTION_STATE_CONNECTING;
	SetIPAndPort( ulIP, uwPort );
	
	if ( bUseTCP )
	{
		bRet = InitialiseGuaranteedTCP();
	}
	else
	{
		bRet = InitialiseGuaranteedUDP();
	}


	return( bRet );
}





BOOL	PeerConnection::SendStreamPacket( byte* pbMsg, int nLen )
{
BYTE	mabSendBuffer[2048];
ushort*		puwPacketSize;

	mabSendBuffer[0] = UDP_PEER_CONNECTION_SYS;
	mabSendBuffer[1] = PEER_SYSMSG_STREAM_PACKET;

	puwPacketSize = (ushort*)( &mabSendBuffer[2] );
	*puwPacketSize = (ushort)( nLen ) + 4;  // + 4 for header

	memcpy( mabSendBuffer + 4, pbMsg, nLen );

	// Send the message
	NetworkConnectionGenericSendMessage( (char*)mabSendBuffer, nLen+4, m_ulIP, m_uwPort, FALSE );
	return( TRUE );
}

void	PeerConnection::SendSysKeepalive( void )
{
BYTE	mabSendBuffer[2048];
ushort*		puwPacketSize;

	mabSendBuffer[0] = UDP_PEER_CONNECTION_SYS;
	mabSendBuffer[1] = PEER_SYSMSG_KEEPALIVE;

	puwPacketSize = (ushort*)( &mabSendBuffer[2] );
	*puwPacketSize = 4;

	// Send the message
	NetworkConnectionGenericSendMessage( (char*)mabSendBuffer, 4, m_ulIP, m_uwPort, FALSE );
}

BOOL	PeerConnection::SendBasicMessage( byte* pbMsg, int nLen )
{
BYTE	mabSendBuffer[2048];
ushort*		puwPacketSize;

	mabSendBuffer[0] = UDP_PEER_CONNECTION_SYS;
	mabSendBuffer[1] = PEER_SYSMSG_CUSTOM;

	puwPacketSize = (ushort*)( &mabSendBuffer[2] );
	*puwPacketSize = (ushort)( nLen ) + 4;  // + 4 for header

	memcpy( mabSendBuffer + 4, pbMsg, nLen );

	// Send the message
	NetworkConnectionGenericSendMessage( (char*)mabSendBuffer, nLen+4, m_ulIP, m_uwPort, FALSE );
	return( TRUE );
}


//--------------------------------------------------------------
//
int		PeerConnection::DecodeBasicMessage( byte* pbMsg )
{
short	wMsgSize = *( (short*)( pbMsg + 2 ) );

	switch( pbMsg[1] )
	{
	case PEER_SYSMSG_STREAM_PACKET:
#ifdef INCLUDE_PEER_STREAM
		if ( mpStreamIn )
		{
			mpStreamIn->DecodePacketReceived( pbMsg );
		}
		else
		{
			mpStreamIn = new PeerStreamIn( this );
		}
#endif
		break;
	case PEER_SYSMSG_KEEPALIVE:
		RegisterKeepaliveReceived();
		break;
	case PEER_SYSMSG_CUSTOM:
		OnReceiveBasicMessage( pbMsg + 4, wMsgSize );
		break;
	default:
//TEMP-RESTORE THIS		PANIC_IF( TRUE, "Peer connection basic message not ready for anything but stream packets" );
		break;
	}
	return( wMsgSize );
}

BOOL	PeerConnection::AddStreamPacket( BYTE* pbData, int nDataLen )
{
#ifdef INCLUDE_PEER_STREAM
	if ( mpStreamOut )
	{
		mpStreamOut->AddStreamPacket( pbData, nDataLen );
		return( TRUE );
	}
#endif
	return( FALSE );
}


BOOL	PeerConnection::OpenVoiceStream( void )
{
	if ( !mpStreamOut )
	{
		mpStreamOut = new PeerStreamOut( this );
		return( TRUE );
	}
	return( FALSE );
}

BOOL	PeerConnection::CloseVoiceStream( void )
{
	if ( mpStreamOut )
	{
		delete mpStreamOut;
		mpStreamOut = NULL;
		return( TRUE );
	}
	return( FALSE );
}
