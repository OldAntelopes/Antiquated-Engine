
#include "StandardDef.h"

#include "../SessionConnection.h"
#include "ArenaPlayerList.h"



void	ArenaPlayerList::Initialise( int nMaxNumPlayers )
{
	mnMaxNumPlayers = nMaxNumPlayers;
}

ArenaPlayer*	ArenaPlayerList::FindPlayer( u64 ullUserUID )
{
ArenaPlayer*	pPlayers = mpPlayerList;
	
	while( pPlayers )
	{
		if ( pPlayers->mullUserUID == ullUserUID )
		{
			return( pPlayers );
		}
		pPlayers = pPlayers->mpNext;
	}
	return( NULL );
}

ArenaPlayer*	ArenaPlayerList::AddPlayer( const char* szUsername, u64 ullUserUID, SessionConnection* pConnection )
{
	if ( mnNumPlayers < mnMaxNumPlayers )
	{
	ArenaPlayer*	pNewPlayer = new ArenaPlayer;

		pNewPlayer->mpNext = mpPlayerList;
		mpPlayerList = pNewPlayer;
		mnNumPlayers++;

		pNewPlayer->mpConnection = pConnection;
		strcpy( pNewPlayer->mszUsername, szUsername );
		pNewPlayer->mullUserUID = ullUserUID;
		
		// TODO - nextArenaPlayerID should shrink back when the highest playerID leaves..
		pNewPlayer->mwArenaPlayerID = mwNextArenaPlayerID;
		mwNextArenaPlayerID++;

		return( pNewPlayer );
	}
	return( NULL );
}


BOOL		ArenaPlayerList::RemovePlayer( u64 ullUserUID )
{
ArenaPlayer*	pPlayers = mpPlayerList;
ArenaPlayer*	pLast = NULL;
	
	while( pPlayers )
	{
		if ( pPlayers->mullUserUID == ullUserUID )
		{
			if ( pLast == NULL )
			{
				mpPlayerList = pPlayers->mpNext;
			}
			else
			{
				pLast->mpNext = pPlayers->mpNext;
			}
			SysUserPrint( 0, "ArenaPlayer %s removed - disconnected", pPlayers->mszUsername );
			delete pPlayers;
			mnNumPlayers--;
			return( TRUE );
		}
		else
		{
			pLast = pPlayers;
		}
		pPlayers = pPlayers->mpNext;
	}
	return( FALSE );
}

