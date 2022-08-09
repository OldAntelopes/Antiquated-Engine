
#include "StandardDef.h"

#include "LocalUser.h"


class LocalUserData
{
public:
	LocalUserData()
	{
		mpcUserName = NULL;
		mpcSessionKey = NULL;
		mpcLoginEmail = NULL;
		mpcPassword = NULL;
		mullUserID = 0;
	}
	~LocalUserData()
	{
		Cleanup();
	}

	void	Cleanup()
	{
		SAFE_FREE( mpcUserName );
		SAFE_FREE( mpcSessionKey );
		SAFE_FREE( mpcLoginEmail );
		SAFE_FREE( mpcPassword );
	}

	char*	mpcUserName;
	char*	mpcSessionKey;
	char*	mpcLoginEmail;
	char*	mpcPassword;
	u64		mullUserID;	
};

//------------ Singleton
LocalUserData		msLocalUserData;

//-----------------------------------------------------------------------------

void		LocalUserInitialise( void )
{
}

void		LocalUserUpdate( float fDelta )
{

}

void		LocalUserShutdown( void )
{
	msLocalUserData.Cleanup();
}


//--------------------------------------------------------

BOOL		LocalUserGetDefaultLoginDetails( char* szLoginEmail, char* szPassword )
{
BOOL	bRet = TRUE;
	if ( msLocalUserData.mpcLoginEmail )
	{
		strcpy( szLoginEmail, msLocalUserData.mpcLoginEmail );
	}
	else
	{
		bRet = FALSE;
	}

	if ( msLocalUserData.mpcPassword )
	{
		strcpy( szPassword, msLocalUserData.mpcPassword );
	}
	else
	{
		bRet = FALSE;
	}
	return( bRet );

}


void		LocalUserSetDefaultLoginDetails( const char* szLoginEmail, const char* szPassword )
{
	if ( msLocalUserData.mpcLoginEmail )
	{
		SystemFree( msLocalUserData.mpcLoginEmail );
	}
	msLocalUserData.mpcLoginEmail = (char*)( SystemMalloc( strlen( szLoginEmail ) + 1 ) );
	strcpy( msLocalUserData.mpcLoginEmail, szLoginEmail );

	if ( msLocalUserData.mpcPassword )
	{
		SystemFree( msLocalUserData.mpcPassword );
	}
	msLocalUserData.mpcPassword = (char*)( SystemMalloc( strlen( szPassword ) + 1 ) );
	strcpy( msLocalUserData.mpcPassword, szPassword );

}

void		LocalUserOnConnect( const char* szUserName, u64 ullUserID )
{
	msLocalUserData.mullUserID = ullUserID;

	if ( msLocalUserData.mpcUserName )
	{
		SystemFree( msLocalUserData.mpcUserName );
	}
	msLocalUserData.mpcUserName = (char*)( SystemMalloc( strlen( szUserName ) + 1 ) );
	strcpy( msLocalUserData.mpcUserName, szUserName );

}


const char*		LocalUserGetUsername( void )
{
	return( msLocalUserData.mpcUserName );
}


void		LocalUserOnLoginComplete( const char* szSessionKey )
{
	if ( msLocalUserData.mpcSessionKey )
	{
		SystemFree( msLocalUserData.mpcSessionKey );
	}
	msLocalUserData.mpcSessionKey = (char*)( SystemMalloc( strlen( szSessionKey ) + 1 ) );
	strcpy( msLocalUserData.mpcSessionKey, szSessionKey );

}

