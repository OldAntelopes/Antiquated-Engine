#ifndef CLIENT_CONNECTION_TO_CHAT_SERVICE_H
#define CLIENT_CONNECTION_TO_CHAT_SERVICE_H

#ifdef __cplusplus

#include "PeerConnection/PeerConnection.h"

#define	MAX_CHANNELS_PER_USER		8


//---------------------------------------------------------------------------------------------------------
// ClientConnectionToChatService
//
//
//---------------------------------------------------------------------------------------------------------
class ClientConnectionToChatService : public PeerConnection
{
public:
	#define UDP_CONNECT_TIMEOUT_ERROR	-1
	#define TCP_CONNECT_ERROR			-2

	enum eSTATE
	{
		NONE,
		LOGGING_IN,
		CONNECTED,
		QUERYING_WORLD_DESTINATION,
		COMPLETE,
		CONNECT_ERROR,
		CONNECTION_REJECTED,
		HAS_UNIVERSE_DESTINATION,
		DISCONNECTED,
		NO_CREDENTIALS,
	};
	ClientConnectionToChatService();
	~ClientConnectionToChatService();

	static ClientConnectionToChatService*		Get();

	static void	Create( ulong ulChatServiceIP, ushort uwChatServicePort );

	static void AddChatChannel( const char* szChannelName, ulong ulLastReceivedTimestamp );
	static void ResetChatChannels( void );
	void	SendChannelsList( void );

//	static void	SetUsingLocalAuthServer( BOOL bFlag ) { m_sbLocalAuthServer = bFlag; }
	static void	Destroy( void );

	void	Reset( void );

	bool	IsServerAddress( ulong ulIP, ushort uwPort );
	bool	IsMyMessage( void );
	int		ReceiveMessage( const byte* pcMsg );

	void	SendChatMessage( int nGroupID, const char* pcMessage );
	void	SendChatCommandMessage( int nMsgType, const char* pcMessage );

	ulong	GetOnlineUserCount( void ) { return( m_ulOnlineUserCount ); }
	ulong	GetOnlineGroupsCount( void ) { return( m_ulOnlineGroupsCount ); }

	// -- PeerConnection virtuals
	virtual void	OnFileReceiveComplete( short wFileID, const char* szFilename, byte* pbMsg, int nMemSize );
	virtual	void	OnReceiveGuaranteedMsg( byte* pbMsgBody );
	virtual void	OnUpdate( float delta );
	virtual	void	OnDisconnected( void );

	eSTATE	GetState( void ) { return( m_state ); }

	enum eLOGIN_FAIL_REASON
	{
		NO_FAIL,
		LOGINEMAIL_NOT_RECOGNISED,
		PASSWORD_MISMATCH,
		NEW_USER_CREATE_FAILED,
		NEW_USER_LOGINEMAIL_EXISTS,
		NEW_USER_LOGIN_USERNAME_EXISTS,
		MULTIPLE_ACCOUNTS,
		BANNED,
		OTHER,
		CLOSED_FOR_MAINTENANCE,
		PASSWORD_RESET_EMAIL_SENT,
		PASSWORD_RESET_PROMPT,
		PASSWORD_NEW_PASSWORD_CONFIRMED,
		AUTHENTICATION_ERROR,
	};

	eLOGIN_FAIL_REASON		GetLoginFailReason( void ) { return( m_loginFailCode ); }

	float		GetConnectWaitTime( void ) { return( m_fConnectWaitTime ); }
private:

	bool	SetOutboundMessage( const char* pcMsg, int nMsgLen );
	void	ClearOutboundMessage( void );

	void	SendInitialConnectMessage( void );
	void	RequestServerFile( const char* szFilename );
	void	SendLoginDetails( void );
	void	ParseAuthenticatedLoginResponse( byte* );
	void	SendInitialLoginMessage( void );
	void	ParseAuthenticateResponse( byte* pbMsgBody );
	void	ParseChatMsg( byte* pbMsgBody );
	void	ParseChannelListMsg( byte* pbMsgBody );
	void	ParseStatMsg( byte* pbMsgBody );
	void	ParseCommandResponseMsg( byte* pbMsgBody );

	eSTATE				m_state;
	eLOGIN_FAIL_REASON	m_loginFailCode;

	ulong		m_ulBaseIP;
	ushort		m_uwBasePort;
	char		m_szNewAccountUsername[32];
	char		m_szNewPassword[32];

	ulong		mulQueuedMsgLastSendTick;
	ulong		mulLastStatRequestTime;
	int			mnQueuedMsgRepeatTries;
	char		m_aQueuedMsgBuff[512];
	int			m_nQueuedMsgLen;
	float		m_fConnectWaitTime;

	ulong		m_ulOnlineUserCount;
	ulong		m_ulOnlineGroupsCount;

	int			m_BaseErrorCode;
	static BOOL		m_sbLocalAuthServer;
};



extern "C"
{
#endif // #ifdef __cplusplus

extern void		AuthOverrideTestCommand( const char* szParam );
extern void		ClientConnectionToChatServiceSetUsingTcp( BOOL bFlag );

typedef	void		(*ChatServiceMessageCallback)( int nType, const char* szFromUser, const char* szMsg, ulong ulTimestamp );

extern void		ClientConnectionToChatServiceRegisterChatHandler( ChatServiceMessageCallback fnChatMessageHandler );

#ifdef __cplusplus
}
#endif








#endif // #ifndef CLIENT_CONNECTION_TO_AUTH_SERVER_H
