#ifndef USER_LOGIN_CLIENT_H
#define USER_LOGIN_CLIENT_H

enum eUserLoginClientState
{
	LOGIN_IDLE,
	LOGIN_IN_PROGRESS,
	LOGIN_SUCCESS,
	LOGIN_ERROR,
	LOGIN_REFUSED_INCORRECT_PASSWORD,
	LOGIN_REFUSED_UNRECOGNISED_USERNAME,
	LOGIN_REFUSED_BANNED,
	LOGIN_CREATE_NEW_USER_IN_PROGRESS,
	LOGIN_TIMEOUT,
	LOGIN_CLOSED,
};

extern void		UserLoginClientSetKey( BYTE* pbUserKey, int nKeyLen );

extern void		UserLoginClientLogin( const char* szWebServerURL, const char* szLoginName, const char* szPassword, const char* szVersion );
extern void		UserLoginClientSteamLogin( const char* szWebServerURL, u64 ullSteamID, const char* szPersonaName, const char* szVersion );
extern void		UserLoginRemoveUnsupportedCharactersFromLoginName( char* pcUserName );

//extern void		UserLoginClientUpdate( float fDelta );

extern void		UserLoginClientShutdown( void );
extern void		UserLoginClientReset( void );

extern eUserLoginClientState		UserLoginClientGetLoginState( void );
extern int							UserLoginClientGetLoginPhase( void );
extern int							UserLoginGetLastResponseCode( void );

extern const char*		UserLoginClientGetSessionKey( void );
extern const char*		UserLoginClientGetUserName( void );
extern u64				UserLoginClientGetUserUID( void );
extern const char*		UserLoginClientGetLastResponseText( void );
extern int				UserLoginClientGetMasterGameState( void );

// ulProductID == 0 to see if the user has 'OldMan' membership or use specific product codes for specific 'products'
extern BOOL				UserLoginClientHasTicket( ulong ulProductID );

//---------------------------------------------------------------
// Common 'Create New User' interface to be shared across games

typedef	void		(*UserLoginClientNewUserDialogMessageCallback)( int nResponseCode );


extern void		UserLoginClientNewUserDialogInit( const char* szWebServerURL, UserLoginClientNewUserDialogMessageCallback fnCallback );
extern void		UserLoginClientNewUserDialogRender( int X, int Y, int W, int H );

extern BOOL				UserLoginClientNewUserCreated( void );
extern const char*		UserLoginClientNewUserGetEmail( void );
extern const char*		UserLoginClientNewUserGetPassword( void );

//--------------------------------------------------------------

#endif