
#ifndef WEBSERVER_CLIENT_CONNECTION_H
#define WEBSERVER_CLIENT_CONNECTION_H

#include "RequestParams.h"

#ifndef SOCKET
typedef unsigned int        SOCKET;
#endif

class UserDBRequestResponse;
class CurrentReceive
{
public:
	CurrentReceive()
	{
		mszRequest = NULL;
		mszRequestHeaders = NULL;
		mszBody = NULL;
		mnContentLength = 0;
		mnContentReceived = 0;
		mbHasHeaders = FALSE;
		mbHasRequest = FALSE;
		mbHasBody = FALSE;
		mulLastReceiveTime = SysGetTimeLong();
	}

	void		Reset( void )
	{
		SAFE_FREE( mszRequest );
		SAFE_FREE( mszRequestHeaders );
		SAFE_FREE( mszBody );
		mnContentLength = 0;
		mnContentReceived = 0;
		mbHasHeaders = FALSE;
		mbHasRequest = FALSE;
		mbHasBody = FALSE;
		mulLastReceiveTime = SysGetTimeLong();
	}

	BOOL		Update( int hTCPSocket, ulong ulFromIP );

	BOOL		HasReceivedRequest( void ) { return( mbHasBody ); }

	const char*		GetRequest( void ) { return( mszRequest ); }
	const char*		GetRequestHeaders( void ) { return( mszRequestHeaders ); }
	const char*		GetBody( void ) { return( mszBody ); }
	int				GetBodyLength( void ) { return( mnContentLength ); }

private:
	char*		mszRequest;
	char*		mszRequestHeaders;
	char*		mszBody;
	
	char*		mpcRequestHeadersRunner;
	char*		mpcBodyRunner;

	int			mnContentLength;
	int			mnContentReceived;

	BOOL		mbHasRequest;
	BOOL		mbHasHeaders;
	BOOL		mbHasBody;

	ulong		mulLastReceiveTime;
};


class ClientConnection
{
public:

	ClientConnection::ClientConnection()
	{
		m_hTcpSocket = 0;
		m_ullUserID = 0;
	}

	void	SetTcpSocket( SOCKET hSocket ) { m_hTcpSocket = hSocket; }
	SOCKET	GetTcpSocket( void ) { return( m_hTcpSocket ); }

	void	UpdateConnection( float fDelta );
	void	CloseConnection( void );
	void	DeleteConnection( void );

	void	SendHTTPResponse( const char* szResponseBody, int nBodyLen );

	void	SetUserID( u64 ulID ) { m_ullUserID = ulID; }
	u64		GetUserID( void ) { return( m_ullUserID ); }

	const char*		GetHTTPRequestHeaders( void );
	int				GetHTTPRequestBody( char* pcOutBuffer, int nOutBufferSize );

	void			SetIP( ulong ulIP ) { m_ulIP = ulIP; }
	ulong			GetIP( void ) { return( m_ulIP ); }
private:


	char*			StripParamsFromPath( char* szFullPath );

	const char*		GetNextWord( const char* szLine, char* szWordOut, int nWordOutBufferSize );
	void			ParseRequest( const char* szRecvBuffer );

	void	SendNotImplementedResponse( void );
	void	SendNotFoundResponse( void );

	void	SendStandardHeaderResponse( const char* szContentType, int nContentLen );
	void	SendBody( const char* szContentBody, int nLen );

	char*	WriteStandardHTMLHeader( char* szBodyOut, const char* szTitle );
	char*	WriteStandardHTMLBodyFooter( char* szBodyOut );
	char*	WriteStandardHTMLBodyHeader( char* szBodyOut );

	SOCKET				m_hTcpSocket;

	ulong				m_ulIP;
	ushort				m_uwPort;
	unsigned int		mhTCPConnectThread;
	RequestParamsList	mRequestParamsList;

	CurrentReceive		mCurrentReceive;
	u64					m_ullUserID;
};


#endif


