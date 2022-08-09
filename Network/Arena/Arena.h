#ifndef NETWORK_ARENA_H
#define NETWORK_ARENA_H

#include "ArenaPlayerList.h"

class Arena
{
public:
	Arena()
	{
		mpNext = NULL;
	}
	~Arena();

	void		InitialiseArena( ushort uwArenaID, ulong ulLevelID, int nMaxNumPlayers );

	BOOL		AddToPlayerList( const char* szUsername, u64 ullUserUID, ulong ulSkillRank, SessionConnection* pConnection, short* pwArenaPlayerIDOut );
	BOOL		RemoveFromPlayerList( u64 ullUserUID );

	ArenaPlayer*	FindPlayer( u64 ullUserUID );
	
	ulong			GetLevelID( void ) { return( mulLevelID ); }
	ushort			GetArenaID( void ) { return( muwArenaID ); }

	void		BroadcastGuaranteed( void* pcMsg, int nMsgLen, SessionConnection* pExcludeConnection );
	void		BroadcastNonGuaranteed( void* pcMsg, int nMsgLen, SessionConnection* pExcludeConnection );

	void		SetNext( Arena* pNext ) { mpNext = pNext; }
	Arena*		GetNext( void ) { return( mpNext ); }
	
	ArenaPlayer*	GetPlayerList( void );
private:

	ushort					muwArenaID;
	ulong					mulLevelID;

	ArenaPlayerList			mPlayerList;
	Arena*		mpNext;
};



#endif
