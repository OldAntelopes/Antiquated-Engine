
#include "StandardDef.h"

#include "../SessionConnection.h"

#include "Arena.h"

Arena::~Arena()
{

}

void		Arena::InitialiseArena( ushort uwArenaID, ulong ulLevelID, int nMaxNumPlayers )
{
	muwArenaID = uwArenaID;
	mulLevelID = ulLevelID;
	mPlayerList.Initialise( nMaxNumPlayers );
}

ArenaPlayer*	Arena::FindPlayer( u64 ullUserUID )
{
ArenaPlayer*	pPlayer = mPlayerList.FindPlayer( ullUserUID );

	return( pPlayer );
}
	
BOOL		Arena::RemoveFromPlayerList( u64 ullUserUID )
{
	return( mPlayerList.RemovePlayer( ullUserUID ) );
}

BOOL		Arena::AddToPlayerList( const char* szUsername, u64 ullUserUID, ulong ulSkillRank, SessionConnection* pConnection, short* pwArenaPlayerIDOut )
{
ArenaPlayer*	pPlayer = mPlayerList.FindPlayer( ullUserUID );

	// If player already exists in our player list, just update their connection details
	if ( pPlayer )
	{
		if ( pPlayer->mpConnection != pConnection )
		{
			if ( pPlayer->mpConnection->IsConnected() )
			{
				// Disconnect the old connection (mmm? - that'll probably trigger a callback that'll remove this player, so
				// we'lll need to be careful about this! :] )
//				pPlayer->mpConnection->Disconnect();
			}
			pPlayer->mpConnection = pConnection;
		}
		else
		{
			// Player already exists with this connection.. mmm
			SysDebugPrint( "[arena] Duplicate AddToPlayerList?" );
			return( FALSE );
		}
	}
	else  // Not in existing player list
	{
		pPlayer = mPlayerList.AddPlayer( szUsername, ullUserUID, pConnection );		

	}

	if ( pPlayer )
	{
		*pwArenaPlayerIDOut = pPlayer->mwArenaPlayerID;
		return( TRUE );
	}
	return( FALSE );
}


void	Arena::BroadcastGuaranteed( void* pcMsg, int nMsgLen, SessionConnection* pExcludeConnection )
{
ArenaPlayer*	pPlayers = mPlayerList.GetPlayerList();

	while( pPlayers )
	{
		if ( pPlayers->mpConnection != pExcludeConnection )
		{
			pPlayers->mpConnection->SendGuaranteedMessage( pcMsg, nMsgLen );
		}
		pPlayers = pPlayers->mpNext;
	}
	
}

ArenaPlayer*	Arena::GetPlayerList( void )
{
ArenaPlayer*	pPlayers = mPlayerList.GetPlayerList();
	return( pPlayers );
}

void	Arena::BroadcastNonGuaranteed( void* pcMsg, int nMsgLen, SessionConnection* pExcludeConnection )
{
ArenaPlayer*	pPlayers = mPlayerList.GetPlayerList();

	while( pPlayers )
	{
		if ( pPlayers->mpConnection != pExcludeConnection )
		{
			pPlayers->mpConnection->SendNonGuaranteedMessage( pcMsg, nMsgLen );
		}
		pPlayers = pPlayers->mpNext;
	}

}
