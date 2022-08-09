#ifndef NETWORKING_LIB
#define NETWORKING_LIB


#ifdef __cplusplus
//--------------------- C++ Interfaces

#define	UDP_PEER_CONNECTION_GUARANTEED	0xB4
#define	UDP_PEER_CONNECTION_SYS			0xB6


typedef	void* (*NewConnectionCallback)( unsigned char, unsigned long, unsigned short, unsigned char );
typedef	int(*NetworkConnectionMessageReceiveHandler)( unsigned char* );
typedef	void(*NetworkStatisticsLogAnnotationCallback)( unsigned char*, int, char* );
typedef	void(*NetworkUserPrintFunction)( int mode, const char* );


namespace Uni
{
	class NetworkingInitParams
	{
	public:
		NetworkingInitParams()
		{
			m_bUseVoiceComms = TRUE;
			m_bUseNetworkConnection = FALSE;
			m_fnNewConnectionCallback = NULL;
			m_fnNetworkConnectionMessageHandler = NULL;
			m_fnLogAnnotationCallback = NULL;
			m_uwNetworkConnectionPort = 0;
			m_nMaxFileTransferSpeed = (20*1024);
			m_bTCPServer = FALSE;
			m_fnNetworkUserPrintFunction = NULL;
		}

		BOOL	m_bUseVoiceComms;
		BOOL	m_bUseNetworkConnection;
		NewConnectionCallback		m_fnNewConnectionCallback;
		NetworkStatisticsLogAnnotationCallback		m_fnLogAnnotationCallback;
		NetworkConnectionMessageReceiveHandler		m_fnNetworkConnectionMessageHandler;
		NetworkUserPrintFunction					m_fnNetworkUserPrintFunction;
		unsigned short		m_uwNetworkConnectionPort;
		int		m_nMaxFileTransferSpeed;
		BOOL	m_bTCPServer;
	};

	class Networking
	{
	public:
		static BOOL	Initialise( NetworkingInitParams* );
		static void	Update( float fDeltaTime );
		static void	Shutdown( void );
	};
}

#endif // _cplusplus


#ifdef __cplusplus
extern "C"
{
#endif

//---------------------- C Interfaces

typedef struct
{
	int		mnStatsTotalBytesRecvd;
	int		mnStatsTotalBytesSent;
	int		mnStatsBytesRecvd;
	int		mnStatsBytesSent;
	int		mnLastStatsBytesRecvd;
	int		mnLastStatsBytesSent;
	unsigned long	mulLastStatsTick;
	unsigned long	mulStatsStartTick;

	int		mnTotalNumberOfResends;

} NETWORK_STATISTICS;

typedef struct
{
	int		mnRecvdBytesPerSecond;
	int		mnSentBytesPerSecond;
	int		mnNumberOfResends;
	int		mnNumberOfAcknowledgementsReceived;
	int		mnNumberOfMessagesSent;
	int		mnNumberOfMessagesReceived;
	int		mnBytesSent;
	int		mnBytesRecvd;

	unsigned long	mulLastStatsTick;
	unsigned long	mulStatsStartTick;

} NEW_NETWORK_STATISTICS;

extern BOOL	NetworkingOnLeftMouseUp( void );

extern void	NetworkingEnableVoiceComms( BOOL bFlag );
extern BOOL	NetworkingIsVoiceCommsEnabled( void );

extern void		NetworkingGetFileTransferStats( int* pnActiveSends, int* pnDesiredBytesPerSec );
extern int		NetworkingPeerConnectionReceiveSysMessage( BYTE* pbMsg );
extern int		NetworkingPeerConnectionReceiveGuaranteedMessage( BYTE* pbMsg );

extern void		NetworkingUpdate( float fDeltaTime );

extern void		NetworkingUserDebugPrint( int, const char*, ... );

extern void		NetworkingDebugPrint( const char*, ... );

extern void		NetworkSetMessageScrambleKey( int nKeyLength, byte* pbBuffer );

extern NETWORK_STATISTICS*		NetworkingGetStatistics( void );
extern NEW_NETWORK_STATISTICS*		NetworkingStatisticsGetRecent( void );
extern NEW_NETWORK_STATISTICS*		NetworkingStatisticsGetTotals( void );

extern void			NetworkingStatisticsActivatePacketLog( BOOL bFlag );

//--------------------------------------------------------------------------
//  Some old functions to make porting some TU stuff easier..
extern struct sockaddr_in*	GetLastGuestAddress( void );
extern unsigned long	GetLastGuestIP( void );
extern unsigned short	GetLastGuestPort( void );

extern void	SetLastGuestAddress( struct sockaddr_in* );

extern int NetworkConnectionGenericSendMessage( const char* pcMsg, int nMsgLength, unsigned long ulIP, unsigned short uwPort, BOOL bIsResend );

extern int NetworkConnectionGenericSendMessageNoScramble( const char* pcMsg, int nMsgLength, unsigned long ulIP, unsigned short uwPort, BOOL bIsResend );

extern int	NetworkingGetSocket( void );
extern void	NetworkConnectionScrambleMessage( byte* pbMsg, int nLen );
extern void NetworkConnectionUnscrambleMessage( byte* pbMsg, int nLen );

extern const char*		NetworkGetIPAddressText( unsigned long ulIP );
extern unsigned long	NetworkGetIPAddress( const char* szIPAddressString );

#ifdef __cplusplus
}
#endif



#endif // #ifndef NETWORKING_LIB

