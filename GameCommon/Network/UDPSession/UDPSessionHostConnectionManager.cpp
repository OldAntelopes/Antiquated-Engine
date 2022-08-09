
#include "StandardDef.h"

#include "../HostConnectionToLoginService.h"

#include "UDPSessionConnection.h"
#include "UDPSessionHostConnectionManager.h"


UDPSessionConnection*		mspUDPSessionHostConnections = NULL;

float		mfTimeSinceLastHostConnectionsUpdate = 0.0f;

UDPSessionConnection*		UDPSessionHostConnectionManagerGetConnection( ulong ulIP, ushort uwPort )
{
UDPSessionConnection*		pConnections = mspUDPSessionHostConnections;

	while( pConnections )
	{
		if ( pConnections->IsThisAddress( ulIP, uwPort ) )
		{
			return( pConnections );
		}
		pConnections = pConnections->GetNext();
	}

	return( NULL );
}

void	UDPSessionHostConnectionValidateResponse( int nResponseCode, u64 ullUserUID, const char* szSessionKey, void* vpCBParam )
{
UDPSessionConnection*		pUDPSessionConnection = (UDPSessionConnection*)( vpCBParam );

	if ( nResponseCode == 200 )
	{
		pUDPSessionConnection->OnSessionValidated();
	}
	else
	{
		// On Fail.. 
		// TODO - Delete this connection .. put a block on it??
		SysUserPrint( 0, "Session validation fail response %d", nResponseCode );
		pUDPSessionConnection->Disconnect();
	}
}

void		UDPSessionHostConnectionManagerInitialise( const char* szLoginServiceURL )
{

	// TODO - LoginServiceURL needs to come from a reloadable config

	SysUserPrint( 0, "Login Service URL: %s", szLoginServiceURL );
	HostConnectionToLoginServiceInitialise( szLoginServiceURL );
	
	HostLoginServiceRegisterSessionResponse( UDPSessionHostConnectionValidateResponse );


}

#define SHOW_CONNECTION_NUMBERS

UDPSessionConnection*		UDPSessionHostConnectionManagerCreateNew( ulong ulIP, ushort uwPort, const char* szSessionKey, u64 ullUserUID )
{
UDPSessionConnection*		pUDPSessionConnection = new UDPSessionConnection;
#ifdef SHOW_CONNECTION_NUMBERS
UDPSessionConnection*		pCountConnections;
int		nCount = 0;
#endif

	pUDPSessionConnection->SetNext( mspUDPSessionHostConnections );
	mspUDPSessionHostConnections = pUDPSessionConnection;

#ifdef SHOW_CONNECTION_NUMBERS
	pCountConnections = mspUDPSessionHostConnections;
	while( pCountConnections )
	{
		nCount++;
		pCountConnections = pCountConnections->GetNext();
	}
	SysUserPrint( 0, "%d connections now active", nCount );
#endif
	pUDPSessionConnection->InitConnectionOnHost( ulIP, uwPort, szSessionKey, ullUserUID );

	HostLoginServiceValidateSessionKey( szSessionKey, ullUserUID, pUDPSessionConnection );

	return( pUDPSessionConnection );
}


void							UDPSessionHostConnectionManagerUpdate( float delta )
{
	// (Note shouldn't need to update the connections regularly, as all host behaviour should be in resposne to incoming 
	// messages - keep the host CPU activity down)

	mfTimeSinceLastHostConnectionsUpdate += delta;
	// Intermittently check connections for timeouts..
	if ( mfTimeSinceLastHostConnectionsUpdate > 1.0f )
	{
	UDPSessionConnection*		pConnections = mspUDPSessionHostConnections;
	UDPSessionConnection*		pLast = NULL;
	UDPSessionConnection*		pNext;

		while( pConnections )
		{
			pNext = pConnections->GetNext();
			pConnections->UpdateConnectionHost( mfTimeSinceLastHostConnectionsUpdate );

			if ( pConnections->IsDead() )
			{
				if ( pLast == NULL )
				{
					mspUDPSessionHostConnections = pNext;
				}
				else
				{
					pLast->SetNext( pNext );
				}
				SysUserPrint( 0, "Connection destroyed" );
				delete pConnections;
			}
			else
			{
				pLast = pConnections;
			}
			pConnections = pNext;
		}

		mfTimeSinceLastHostConnectionsUpdate = 0.0f;
	}

}
