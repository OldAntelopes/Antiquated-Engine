
#ifndef NETWORKING_PEER_CONNECTION_H
#define NETWORKING_PEER_CONNECTION_H

class	FileTransfer;
class	ConnectionManager;

typedef	void(*PacketReceiptCallback)( void* );

#ifndef SOCKET
typedef unsigned int        SOCKET;
#endif

enum		// These are the raw message codes, sent without guarantee and decoded in DecodeBasicMessage
{	
	PEER_SYSMSG_CUSTOM = 0xF0,
	PEER_SYSMSG_CLOSECONNECTION = 0xFC,
	PEER_SYSMSG_GUARANTEEDINIT = 0xFD,
	PEER_SYSMSG_KEEPALIVE = 0xFE,
	PEER_SYSMSG_STREAM_PACKET = 0xFF,
};

enum eConnectionCodes
{
	PEERCONNECTION_BASIC = 0,
	PEERCONNECTION_SPACE,
};

enum	// Reserved Msg codes for the peer guaranteed connection
		// Used in PEERCONN_GUARANTEED_MSG_HEADER, which starts.. 
		//		BYTE	bMsgCode - UDP_PEER_CONNECTION_GUARANTEED
		//		BYTE	bHeaderSize - Size of the PEERCONN_GUARANTEED_MSG_HEADER 
		//		short	wMsgID - Either a user msgID or one of the system reserves below

{
	PEER_GUARANTEED_KEEPALIVE = 0xFC,
	PEER_GUARANTEED_STREAM_PACKET = 0xFD,
	PEER_GUARANTEED_FILE_PACKET = 0xFE,
	PEER_GUARANTEED_SYS_RESERVE = 0xFF,	// special message code for the GuaranteedConnection file transfer
};

typedef struct
{
	BYTE	bUDPCode;			// Note - first 4 bytes of this msg must not be changed
	BYTE	bMsgCode;
	BYTE	bSizeOfMsg;
	BYTE	bParamCode;

	BYTE	bConnectionCode;
	BYTE	bPad1;
	short	wParam2;
	ulong	ulLocalTimer;
	ulong	ulResponseTimer;

} PEER_STATUS_MSG;

typedef struct
{
	BYTE	bMsgCode;
	BYTE	bParam1;
	short	wMsgLen;
} PEER_CHAT_MSG;

typedef struct
{
	BYTE	bMsgCode;
	BYTE	bFileInfoMode;
	BYTE	bPad1;
	BYTE	bPad2;
	short	wFilenameLen;
	short	wFileID;
	ulong	ulChecksum;

} PEER_FILE_INFO_MSG;

typedef struct
{
	BYTE	bMsgCode;
	BYTE	bFileInfoMode;
	BYTE	bFields;
	BYTE	bPad2;
	short	wFilenameLen;
	short	wFileID;
	ulong	ulChecksum;

} PEER_SYS_MSG;


typedef struct
{
	BYTE	bMsgCode;
	BYTE	bPacketNum;
	short	wFileID;
	short	wPacketLen;

	ulong	ulPad1;

} PEER_FILE_PACKET_MSG;

class	BufferedMessage;
class	PeerStreamIn;
class	PeerStreamOut;

class FileTransferRecord
{
public:
	FileTransferRecord()
	{
		mpFileTransfer = NULL;
		mpNext = NULL;
	}
	FileTransfer*	mpFileTransfer;
	FileTransferRecord*	mpNext;
};


//---------------------------------------------------------------------------------------------------------------------------
// PeerConnection class
// A generic class used to maintain a connection between peers
// 
// This class provides functions for NAT resolution, file transfers &
// guaranteed messaging between peers.
//
class PeerConnection
{

	friend class ConnectionManager;
	friend class BufferedMessage;

	#define	MAX_BUFFERED_MESSAGES			16
	#define	MAX_MSG_IDS						256
	#define	NUM_PING_RECORDS				32

	enum
	{
		PEER_CONNECTION_STATE_NONE,
		PEER_CONNECTION_STATE_CONNECTING,
		PEER_CONNECTION_INITIALISING_GUARANTEED,
		PEER_CONNECTION_STATE_RESOLVING_NAT,
		PEER_CONNECTION_STATE_RESPONSIVE,
		PEER_CONNECTION_STATE_UNRESPONSIVE,
	};
public:

	typedef struct
	{
		BYTE	bMsgCode;
		BYTE	bHeaderSize;
		short	wMsgID;
		short	wMsgSize;
		ushort	uwTimingInfo;

	} PEERCONN_GUARANTEED_MSG_HEADER;

	PeerConnection();
	virtual ~PeerConnection();

	// - Basic Functions
	BOOL	InitialiseGuaranteed( BOOL bUseTCP, ulong ulIP, ushort uwPort );
 	void	UpdateConnection( float fDeltaTime );
	void	CloseConnection( BOOL bSendCloseMsg = TRUE );

	BOOL	SendMessageGuaranteed( byte* pbMsg, int nLen );
	BOOL	SendMessageGuaranteedEx( byte* pbMsg, int nLen, PacketReceiptCallback fnReceiptCallback, void* param );

	BOOL	SendBasicMessage( byte* pbMsg, int nLen );
	BOOL	SendStreamPacket( byte* pbMsg, int nLen );

	BOOL	SendFile( const char* pcFilename, const char* pcReceiverFilename );
	BOOL	RequestFile( const char* pcFilename );

	// - Essential virtuals 
	virtual void	OnFileReceiveComplete( short wFileID, const char* szFilename, byte* pbMsg, int nMemSize ) = 0;
	virtual void	OnFileReceiveError( short wFileID, const char* szFilename, int nErrorCode );
	virtual	void	OnReceiveGuaranteedMsg( byte* pbMsgBody ) = 0;

	// - Optional virtuals
	virtual void	OnReceiveBasicMessage( byte* pbMem, short wMsgSize ) {}
	virtual	void	OnConnected( void ) { }
	virtual	void	OnDisconnected( void ) { }
	virtual	void	OnFileReceiveStarted( short wFileID, const char* pcFilename ) { }
	virtual BOOL	OnFileRequest( const char* pcFilename ) { return( FALSE ); }
	virtual BOOL	OnSendFileFromMem( const char* pcFilename, byte** ppMem, int* pnMemSize ) { return( FALSE ); }
	virtual void	OnUpdate( float delta ) { }

	// - Status queries
	BOOL	IsConnected() { return( (m_ConnectionState == PEER_CONNECTION_STATE_RESPONSIVE) ); }
	BOOL	IsSendingFile();
	BOOL	IsReceivingFile();
	void	GetFileReceiveProgress( int nIndex, int* pnCurrent, int* pnSize, float* pfTransferTime, char* szFilename, int nBufferLen );
	void	GetFileSendProgress( int nIndex, int* pnCurrent, int* pnSize, char* szFilename, int nBufferLen );
	ulong	GetPingTime( void ) { return( mulAveragePingTime ); }
	ulong	GetRemoteTickEstimate( void ) { return( mulRemoteTick ); }

		// NB This shouldnt be needed really, but atm, the guaranteed buffers are small
		// and so can run out easily..
		// Currently, if this returns true, then a SendMessageGuaranteed will fail and return false
	BOOL	IsBusy();

	// - Voice comms functionality
	BOOL	OpenVoiceStream( void );
	BOOL	AddStreamPacket( BYTE* pbData, int nDataLen );
	BOOL	CloseVoiceStream( void );

	// - A readable identifier for this connection (for debug prints etc)
	const char*		GetIdentifier( void );
	void			SetIdentifier( const char* szIdentifier );

	//--------------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------
	int		DecodeGuaranteedMessageUDP( byte* pbMsg );
	int		DecodeBasicMessage( byte* pbMsg );

	static int		GetMsgLen( byte* pbMsg )
	{
	PEERCONN_GUARANTEED_MSG_HEADER* pxHeader = (PEERCONN_GUARANTEED_MSG_HEADER*)(pbMsg);
		return( pxHeader->bHeaderSize + pxHeader->wMsgSize );
	}

	void	GetSendAddress( ulong ulIP, ushort uwPort );
	void	SendSysMessage( BYTE bType, short wParam, ulong ulParam );
	void	SendSysMessageEx( BYTE bType, short wParam, ulong ulParam, void* pExtra, int nExtraLen );

	virtual	BOOL				UseOldConnectionMessages( void ) { return( FALSE ); }
	virtual eConnectionCodes	GetConnectionCode( void ) { return( PEERCONNECTION_BASIC ); }

	// SetUseOutboundKeepalive - Default : TRUE
	//  Send a message out at regular intervals to maintain connection status
	//  Some servers (e.g. chat server) turn this off to minimise bandwidth use, and rely on the client keeping the connection open with regular keepalives
	void	SetUseOutboundKeepalive( BOOL bFlag ) { mbUseOutboundKeepalive = bFlag; }
	// SetUsePingTracking - Default : TRUE
	//   Ping tracking requires the server to respond to a regular guaranteed keepalive msg, meaning more bandwidth use on the server
	//   If ping data is not required (e.g. Chat server connection) it can be turned off and the keepalive is done via SYS msgs that aren't responded to by the server
	void	SetUsePingTracking( BOOL bFlag ) { mbUsePingTracking = bFlag; }

	void	HandleFileReceiveComplete( short wFileID, const char* szFilename, byte* pbMsg, int nMemSize );

	ulong	GetIP( void ) { return( m_ulIP ); }
	ushort	GetPort( void ) { return( m_uwPort ); }

	int		CountActiveFileTransfers( void );

	// ----------- UDP guaranteed system handlers
	BOOL	RemoteInitialiseGuaranteedUDP( ulong ulIP, ushort uwPort, ulong ulRemoteTick );
	void	RemoteInitResponseUDP( ulong ulIP, ushort uwPort, ulong ulRemoteTick, ulong ulOriginalTick );

	BOOL	IsUsingTcp() { if ( m_hTcpSocket != 0 ) return( TRUE ); return( FALSE ); }
	void	TcpConnectJob( void );
	void	OnRemoteTCPConnected( SOCKET hSocket );
	void	RegisterKeepaliveReceived( void );
	void	SendSysKeepalive( void );

protected:
	void	Reset( void );
	void	AddPingTime( ulong ulPing );
	void	SetIPAndPort( ulong ulIP, ushort uwPort ) { m_ulIP = ulIP; m_uwPort = uwPort; }
	void	SetConnected();
	void	ConnectionDecodeFileSendAcknowledge( short wFileID );
	void	ConnectionDecodeFilePacket( byte* pbMsg );
	void	CloseFileTransfer( FileTransferRecord* pRecord );
	void	StripFilename( const char* pcFilename, char* szBuffer );
	void	SendCloseConnectionMessage( void );
	void	DecodeGuaranteedMessagePacket( byte* pbMsgBody );

	BOOL	InitialiseGuaranteedUDP( void );
	BOOL	InitialiseGuaranteedTCP( void );

	FileTransferRecord*		AddNewFileTransfer( void );
	BufferedMessage*		mapMessageBuffers[MAX_BUFFERED_MESSAGES];

	ulong	m_ulIP;
	ushort	m_uwPort;

	unsigned int	mhTCPConnectThread;

	ushort	mauwPingTimes[NUM_PING_RECORDS];
	int		mnNextPingSlot;
	ulong	mulAveragePingTime;
	ulong	mulLastDataSentTick;
	ulong	mulRemoteTick;
	ulong	mulLastResponseTick;
	short	mwNextPeerFileTransferFileID;

	int		m_ConnectionState;
	SOCKET	m_hTcpSocket;

	BYTE	mbNextSendID;
	BYTE	m_abReceiptIDs[MAX_MSG_IDS];
	
	int		mnTCPConnectJobComplete;

	FileTransferRecord*	mpFileTransferRecords;

	PeerStreamIn*		mpStreamIn;
	PeerStreamOut*		mpStreamOut;

	BOOL				mbClosingConnection;
	BOOL				mbUseOutboundKeepalive;
	BOOL				mbUsePingTracking;

};



#endif
