#ifndef LOCAL_USER_H
#define	LOCAL_USER_H

//--------------------------------------------------------

extern void		LocalUserInitialise( void );
extern void		LocalUserUpdate( float fDelta );
extern void		LocalUserShutdown( void );

//--------------------------------------------------------

extern BOOL		LocalUserGetDefaultLoginDetails( char* szLoginEmail, char* szPassword );

extern void		LocalUserSetDefaultLoginDetails( const char* szLoginEmail, const char* szPassword );

extern void		LocalUserOnConnect( const char* szUserName, u64 ullUserID );

extern void		LocalUserOnLoginComplete( const char* szSessionKey );

extern const char*		LocalUserGetUsername( void );

#endif