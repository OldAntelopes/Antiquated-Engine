
#include <stdio.h>

#include <UnivSocket.h>
#include <StandardDef.h>

#include "WebServerConnectionManager.h"


BOOL	msbWebServerConnectionManagerShutdownMutex = FALSE;

class ClientConnectionList
{
public:
	ClientConnectionList()
	{
		mpConnection = NULL;
		mbWantsDelete = FALSE;
		mpNext = NULL;
	}

	~ClientConnectionList()
	{
		SAFE_DELETE( mpConnection );
	}

	ClientConnection*			mpConnection;
	BOOL						mbWantsDelete;
	ClientConnectionList*		mpNext;
};


WebServerConnectionManager::WebServerConnectionManager()
{
}

WebServerConnectionManager::~WebServerConnectionManager()
{
/*ClientConnectionList*		pListEntry = mpConnectionList;
ClientConnectionList*		pNext;

	msbWebServerConnectionManagerShutdownMutex = TRUE;
	while( pListEntry )
	{
		pNext = pListEntry->mpNext;
		if ( pListEntry->mpConnection )
		{
			delete( pListEntry->mpConnection );
		}
		delete pListEntry;
		pListEntry = pNext;
	}
	*/
}

WebServerConnectionManager&	WebServerConnectionManager::Get()
{
static	WebServerConnectionManager		m_singleton;
	return( m_singleton );
}

//-----------------------------------------------------------------------------------------------------
// WebServerConnectionManager::AddConnection
//
//
//-----------------------------------------------------------------------------------------------------
void	WebServerConnectionManager::AddConnection( ClientConnection* pConnection )
{
ClientConnectionList*		pListEntry;

	pListEntry = new ClientConnectionList;
	pListEntry->mpConnection = pConnection;
	pListEntry->mpNext = mpConnectionList;
	mpConnectionList = pListEntry;

	mnNumActiveConnections++;
}

void	WebServerConnectionManager::DestroyConnection( ClientConnection* pConnection )
{
ClientConnectionList*		pListEntry = mpConnectionList;
ClientConnectionList*		pPrev = NULL;

	if ( msbWebServerConnectionManagerShutdownMutex ) return;

	while( pListEntry )
	{
		if ( pListEntry->mpConnection == pConnection )
		{
			pListEntry->mbWantsDelete = TRUE;
			break;
		}
		pListEntry = pListEntry->mpNext;
	}

}


//-----------------------------------------------------------------------------------------------------
// WebServerConnectionManager::UpdateAllConnections
//
//
//-----------------------------------------------------------------------------------------------------
void	WebServerConnectionManager::UpdateAllConnections( float fDeltaTime )
{
	if ( mpConnectionList )
	{
	ClientConnectionList*		pListEntry = mpConnectionList;
	ClientConnectionList*		pLast = NULL;
	ClientConnectionList*		pNext;

		while( pListEntry )
		{
			if ( pListEntry->mpConnection )
			{
				pListEntry->mpConnection->UpdateConnection( fDeltaTime );
			}
			
			pNext = pListEntry->mpNext;

			if ( pListEntry->mbWantsDelete == TRUE )
			{
				if ( pLast == NULL )
				{
					mpConnectionList = pNext;
				}
				else
				{
					pLast->mpNext = pNext;
				}
				delete pListEntry;
				pListEntry = pNext;
				if ( mnNumActiveConnections > 0 )
				{
					mnNumActiveConnections--;
				}
			}
			else
			{
				pLast = pListEntry;
				pListEntry = pNext;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------------
// WebServerConnectionManager::CloseAllConnections
//
//
//-----------------------------------------------------------------------------------------------------
void	WebServerConnectionManager::CloseAllConnections( void )
{
ClientConnectionList*		pListEntry = mpConnectionList;

	while( pListEntry )
	{
		if ( pListEntry->mpConnection )
		{
			pListEntry->mpConnection->CloseConnection();
		}
		pListEntry = pListEntry->mpNext;
	}
}

void	WebServerConnectionManager::DeleteAllConnections( void )
{
ClientConnectionList*		pListEntry = mpConnectionList;
ClientConnectionList*		pNext;

	while( pListEntry )
	{
		pNext = pListEntry->mpNext;
		delete pListEntry;
		pListEntry = pNext;
	}
	mpConnectionList = NULL;
	mnNumActiveConnections = 0;
}



//-----------------------------------------------------------------------------------------------------
// WebServerConnectionManager::DecodeNewTCPConnection
//
//
//-----------------------------------------------------------------------------------------------------
ClientConnection*		WebServerConnectionManager::DecodeNewTCPConnection( ulong ulIP )
{
ClientConnection*		pNewConnection;

	pNewConnection = new ClientConnection;
	pNewConnection->SetIP( ulIP );
	AddConnection( pNewConnection );
	return( pNewConnection );
}

