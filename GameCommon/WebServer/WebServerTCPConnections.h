#ifndef TCP_CONNECTIONS_H
#define	TCP_CONNECTIONS_H


typedef	int(*NetworkConnectionMessageReceiveHandler)( unsigned char* );

class TCPConnections
{
public:
	static void	InitialiseTCPListener( ushort uwPort );

	static void	Update( void );

	static void	ShutdownTCP( void );

private:

	static void	UpdateNewTCPConnections( void );


};


enum eHeaderFlags
{
	NO_FLAGS = 0,
	CLOSE_CONNECTION = 0x1,
	CONTENT_TYPE_TEXT = 0x2,
};

extern void	TcpConnectionSendHTTPResponse( SOCKET hSocket, int nResponseCode, const char* szBody, int nBodyLen, eHeaderFlags flags = NO_FLAGS );

enum
{
	TCPSOCKET_ERROR_DISCONNECT = -1,
};

extern int		TCPSocketGetRequestLine( SOCKET hSocket, char* recvBuff, int size);


#endif
