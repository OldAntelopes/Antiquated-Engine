#ifndef GAMECOMMON_UDPSESSION_HOSTCONNECTIONMANAGER_H
#define GAMECOMMON_UDPSESSION_HOSTCONNECTIONMANAGER_H

class UDPSessionConnection;


extern void							UDPSessionHostConnectionManagerInitialise( const char* szLoginServiceURL );

extern UDPSessionConnection*		UDPSessionHostConnectionManagerGetConnection( ulong ulIP, ushort uwPort );

extern UDPSessionConnection*		UDPSessionHostConnectionManagerCreateNew( ulong ulIP, ushort uwPort, const char* szSessionKey, u64 ullUserUID );

extern void							UDPSessionHostConnectionManagerUpdate( float delta );

extern void							UDPSessionHostConnectionManagerShutdown( void );



#endif
