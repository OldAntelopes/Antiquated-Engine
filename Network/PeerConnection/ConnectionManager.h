
#ifndef NETWORKING_CONNECTION_MANAGER_H
#define NETWORKING_CONNECTION_MANAGER_H

#include "PeerConnection.h"


class PeerConnectionList
{
public:
	PeerConnection*			mpConnection;
	PeerConnectionList*		mpNext;
};


//---------------------------------------------------------------------------
class ConnectionManager
{
public:
	ConnectionManager();
	~ConnectionManager();

	static ConnectionManager&		Get();

	void	AddConnection( PeerConnection* );
	void	RemoveConnection( PeerConnection* pConnection );

	int		ReceiveMessage( byte* pbMsg );
	int		ReceiveSysMessage( byte* pbMsg );

	void	UpdateAllConnections( float fDeltaTime );
	void	CloseAllConnections( void );

	void	RegisterNewConnectionCallback( NewConnectionCallback fnFunc ) { mpfnNewConnectionCallback = fnFunc; }

	PeerConnection*		DecodeNewTCPConnection( ulong ulFromIP, ushort uwFromPort );

protected:
	PeerConnection*		FindMessageSource( void );
	void				DecodeNewConnection( PEER_STATUS_MSG* pxMsg );

private:
	PeerConnectionList*		mpConnectionList;
	NewConnectionCallback	mpfnNewConnectionCallback;
};


#endif
