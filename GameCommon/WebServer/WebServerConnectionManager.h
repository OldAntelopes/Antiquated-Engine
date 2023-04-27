
#ifndef NETWORKING_CONNECTION_MANAGER_H
#define NETWORKING_CONNECTION_MANAGER_H

#ifndef SOCKET
typedef unsigned int        SOCKET;
#endif

#include "ClientConnection.h"

class ClientConnectionList;

//---------------------------------------------------------------------------
class WebServerConnectionManager
{
public:
	WebServerConnectionManager();
	~WebServerConnectionManager();

	static WebServerConnectionManager&		Get();
	
	int		GetNumActiveConnections() { return( mnNumActiveConnections ); }

	void	AddConnection( ClientConnection* );
	void	DestroyConnection( ClientConnection* );

	void	UpdateAllConnections( float fDeltaTime );
	void	CloseAllConnections( void );

	void	DeleteAllConnections( void );

	ClientConnection*		DecodeNewTCPConnection( uint32 ulIP );

private:

	int							mnNumActiveConnections;
	ClientConnectionList*		mpConnectionList;
};


#endif
