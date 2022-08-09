#ifndef NETWORK_ARENA_PLAYER_LIST_H
#define NETWORK_ARENA_PLAYER_LIST_H

class SessionConnection;

class ArenaPlayer
{
public:
	short					mwArenaPlayerID;
	u64						mullUserUID;
	char					mszUsername[32];
	SessionConnection*		mpConnection;

	ArenaPlayer*		mpNext;
};


class ArenaPlayerList
{
public:
	ArenaPlayerList()
	{
		mnNumPlayers = 0;
		mnMaxNumPlayers = 8;
		mpPlayerList = NULL;
		mwNextArenaPlayerID = 1;
	}

	void			Initialise( int nMaxNumPlayers );

	ArenaPlayer*	AddPlayer( const char* szUsername, u64 ullUserUID, SessionConnection* pConnection );
	BOOL			RemovePlayer( u64 ullUserUID );

	ArenaPlayer*	FindPlayer( u64 ullUserUID );

	ArenaPlayer*	GetPlayerList() { return( mpPlayerList ); }

private:
	int					mnNumPlayers;
	int					mnMaxNumPlayers;

	short				mwNextArenaPlayerID;

	ArenaPlayer*		mpPlayerList;
};





#endif