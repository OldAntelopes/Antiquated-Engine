#ifndef SECURITY_LIB
#define SECURITY_LIB


#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_CHARS_IN_PASSWORD		24

extern void		EncryptPassword( char* pcPassword );
extern void		DecryptPassword( char* pcPassword );

extern uint32	GetUniqueID ( void );
extern int		SetUniqueID ( int nIDVal );
extern int		LoadUniqueID ( void );

extern BOOL		SecureStrings ( char* pcComString, ushort uwBillingID, short wWhoFrom );
extern BOOL		CheckRegistration ( int nServerID, char* szPassword, char* szOwner );

extern BOOL		RegisteredIslandInitCheck( char* szIslandName, uint32 ulUID, char* szRegisteredError );



#ifdef __cplusplus
}
#endif




#endif