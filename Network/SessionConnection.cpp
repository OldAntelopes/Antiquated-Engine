
#include "StandardDef.h"

#include "SessionConnection.h"

SessionConnection::SessionConnection()
{
	mpDisconnectCallbackList = NULL;
	muwUserData = 0;
	mullUserUID = 0;
}

SessionConnection::~SessionConnection()
{
DisconnectCallbackList*		pCallbacks = mpDisconnectCallbackList;
DisconnectCallbackList*		pNext;

	while( pCallbacks )
	{
		pNext = pCallbacks->mpNext;
		delete pCallbacks;
		pCallbacks = pNext;
	}

}

void	SessionConnection::AddDisconnectCallback( SessionConnectionDisconnectCallback fnCallback, void* pUserData )
{
DisconnectCallbackList*		pCallback = new DisconnectCallbackList;

	pCallback->mfnCallback = fnCallback;
	pCallback->mpUserData = pUserData;
	pCallback->mpNext = mpDisconnectCallbackList;
	mpDisconnectCallbackList = pCallback;

}

void		SessionConnection::OnConnect( void )
{

}


void		SessionConnection::OnDisconnect( void )
{
DisconnectCallbackList*		pCallbacks = mpDisconnectCallbackList;

	while( pCallbacks )
	{
		if ( pCallbacks->mfnCallback )
		{
			pCallbacks->mfnCallback( this, pCallbacks->mpUserData );
		}
		pCallbacks = pCallbacks->mpNext;
	}

}

