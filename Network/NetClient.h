
#ifndef GAMECOMMON_NET_CLIENT_H
#define GAMECOMMON_NET_CLIENT_H

//----------------------------------------------------------------------------------------
// NetClient
//
// Maintains the state of our UDP connections to net servers (e.g. chat server, RT game server)
//----------------------------------------------------------------------------------------

extern void		NetClientInit( void );
extern void		NetClientUpdate( float fDelta );
extern void		NetClientShutdown( void );

//extern void		NetClientStartConnection( void );



#endif //  ifndef GAMECOMMON_NET_CLIENT_H