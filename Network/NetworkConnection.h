#ifndef CONNECTIONS_NETWORKCONNECTION_H
#define CONNECTIONS_NETWORKCONNECTION_H

//--------------------------------------------------------------------------
#define MAXBUFLEN 8192

typedef	int(*NetworkConnectionMessageReceiveHandler)( unsigned char* );
typedef	void(*NetworkStatisticsLogAnnotationCallback)( byte*, int, char* );


#ifdef __cplusplus
//--------------------- C++ Interfaces


class NetworkConnection
{
public:
	NetworkConnection();
	~NetworkConnection();

	static NetworkConnection&		Get();

	BOOL		InitialiseUDP( ushort uwPort, NetworkConnectionMessageReceiveHandler, int );
	void		InitialiseTCPListener( ushort uwPort, NetworkConnectionMessageReceiveHandler messageHandler, int nMode );
	void		SetLogAnnotationCallback( NetworkStatisticsLogAnnotationCallback logHandler ) { m_fnLogAnnotationHandler = logHandler; }

	void		Update( float fDeltaTime );

	int			Send( const char* pMsg, int nMsgLength, ulong ulIP, ushort uwPort, BOOL	bIsResend, BOOL bScramble );

	void		ShutdownUDP( void );
	void		ShutdownTCP( void );

	void		GetLastMessageSource( ulong* pulIP, ushort* puwPort );
	
	NetworkStatisticsLogAnnotationCallback		GetAnnotationCallback( void ) { return( m_fnLogAnnotationHandler ); }
#ifdef MARMALADE
#else
	// For compat
	int			GetSocket() { return( m_hSocket ); }
#endif

private:
	void		UpdateNewTCPConnections( void );
	int			HandleSystemMessages( unsigned char* pcMsg );

#ifdef MARMALADE
	s3eSocket*		m_pUDPSocket;
#else
	// For compat
	int			m_hSocket;
#endif
	byte		m_abMsgBuffer[MAXBUFLEN];
	NetworkConnectionMessageReceiveHandler		m_fnIncomingMessageHandler;
	NetworkStatisticsLogAnnotationCallback		m_fnLogAnnotationHandler;
};

#endif // #ifdef __cplusplus


#ifdef __cplusplus
extern "C"
{
#endif

extern void	NetworkingStatsAddSend( byte* pbMsg, int nBytesSent, BOOL bIsResend );
extern void	NetworkingStatsAddReceive( byte* pbMsg, int nBytesReceived, int nType );
extern const char*		NetworkGetIPAddressText( ulong ulIP );

#ifdef __cplusplus
}
#endif




#endif // #ifndef CONNECTIONS_NETWORKCONNECTION_H
