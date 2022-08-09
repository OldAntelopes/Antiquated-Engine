#include <stdio.h>
#include "StandardDef.h"

#include "../HTTP/HTTP.h"
#include "../Util/cJSON.h"

#include "HostConnectionToLoginService.h"

class SessionCache
{
public:
	SessionCache()
	{
		mszSessionKey = NULL;
		mullSteamID = 0;
		mulLastValidatedTime = 0;
		mulLastReferencedTime = 0;
		mpNext = NULL;
	}

	ulong		mulSessionKeyHash;
	char*		mszSessionKey;
	u64			mullSteamID;
	ulong		mulLastValidatedTime;
	ulong		mulLastReferencedTime;

	SessionCache*		mpNext;

};

// Need to get an equivalent of the msServiceConfig in here...
char		mszUserLoginServiceURL[256] = "";
//char		mszUserLoginServiceURL[256] = "http://52.91.46.154:8080";

SessionCache*		mspSessionCache = NULL;
ValidateSessionResponse		mfnSessionValidateResponseCallback = NULL;

void		HostConnectionToLoginServiceInitialise( const char* szLoginServiceURL )
{
	if ( szLoginServiceURL )
	{
		strcpy( mszUserLoginServiceURL, szLoginServiceURL );
		SysUserPrint( 0, "HostConnectionToLoginService at %s", szLoginServiceURL );
	}

	// todo - we should authenticate this service with the UserLoginService
}

SessionCache*	HostLoginServiceValidateFromCache( const char* szSessionKey, u64 ullSteamID )
{
SessionCache*		pSessionCache = mspSessionCache;
ulong			ulSessionHash = BasicHash( szSessionKey );

	while( pSessionCache )
	{
		if ( ( pSessionCache->mulSessionKeyHash == ulSessionHash ) &&
			 ( stricmp( pSessionCache->mszSessionKey, szSessionKey ) == 0 ) )
		{
			if ( ( ullSteamID == 0 ) ||
				 ( pSessionCache->mullSteamID == ullSteamID ) )
			{
				pSessionCache->mulLastReferencedTime = SysGetTimeLong();
				return( pSessionCache );
			}
		}
		pSessionCache = pSessionCache->mpNext;
	}
	return( NULL );
}

void	HostLoginServiceAddToSessionCache( u64 ullSteamID, const char* pcSessionKey )
{
SessionCache*		pSessionCache = new SessionCache;

	pSessionCache->mszSessionKey = (char*)( malloc( strlen( pcSessionKey ) + 1 ) );
	strcpy( pSessionCache->mszSessionKey, pcSessionKey );
	pSessionCache->mulLastValidatedTime = SysGetTimeLong();
	pSessionCache->mulLastReferencedTime = SysGetTimeLong();
	pSessionCache->mullSteamID = ullSteamID;
	pSessionCache->mulSessionKeyHash = BasicHash( pcSessionKey );

	pSessionCache->mpNext = mspSessionCache;
	mspSessionCache = pSessionCache;

	// todo - when required, trim the cache size down every so often by getting rid of 
	// oldest referenced items
	
}

void		HostLoginServiceRegisterSessionResponse( ValidateSessionResponse fnResponseCallback )
{
	mfnSessionValidateResponseCallback = fnResponseCallback;
}

void		HostLoginServiceValidateSessionResponse( int nResponseCode, unsigned char* szResponseBody, int nResponseLen, void* vpParam )
{
	if ( nResponseCode == 200 )
	{
	u64		ullUserUID;
	char*	pcSessionKey;
	cJSON*	pJSONRoot;
	cJSON*	pItem;

		pJSONRoot = cJSON_Parse( (char*)szResponseBody );
		pItem = cJSON_GetObjectItem( pJSONRoot, "USERID" );

		ullUserUID = (u64)( pItem->valuedouble );

		pItem = cJSON_GetObjectItem( pJSONRoot, "SESSION" );

		pcSessionKey = pItem->valuestring;

		if ( ullUserUID != 0 )
		{
			// ADD to session cache
			HostLoginServiceAddToSessionCache( ullUserUID, pcSessionKey );
		}

		mfnSessionValidateResponseCallback( 200, ullUserUID, pcSessionKey, vpParam );
//		SysDebugPrint( "Session validation 200 - user %lld", ullUserUID );
	}
	else
	{
		SysDebugPrint( "Session validation error %d", nResponseCode );
		mfnSessionValidateResponseCallback( nResponseCode, 0, NULL, vpParam );
	}

}

void		HostLoginServiceValidateSessionKey( const char* szSessionKey, u64 ullSteamID, void* vpCBParam )
{
	if ( mfnSessionValidateResponseCallback )
	{
	SessionCache*		pSessionCache = HostLoginServiceValidateFromCache( szSessionKey, ullSteamID );
	char		acValidateURL[256];

		if ( pSessionCache )
		{
			mfnSessionValidateResponseCallback( 200, pSessionCache->mullSteamID, szSessionKey, vpCBParam );
			return;
		}
	
		// request validation from login service
		if ( ullSteamID == 0 )
		{
			sprintf( acValidateURL, "%s/validate?session=%s", mszUserLoginServiceURL, szSessionKey );
		}
		else
		{
			sprintf( acValidateURL, "%s/validate?user=%lld&session=%s", mszUserLoginServiceURL, ullSteamID, szSessionKey );
		}

		SysUserPrint( 0, "Validating session %s user: %ld", szSessionKey, ullSteamID );
		// todo - we should have a lower timeout limit on this call (as we can assume the loginservice is on 
		// a fast response path)
		HTTPGet( acValidateURL, HostLoginServiceValidateSessionResponse, vpCBParam ); 
	}

}



