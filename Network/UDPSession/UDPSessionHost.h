#ifndef GAMECOMMON_NETWORK_UDPSESSION_HOST_H
#define GAMECOMMON_NETWORK_UDPSESSION_HOST_H

//----------------------------------------------------
// UDPSessionHost
//
// This module to be included in applications that can accept a UDP connection from a client using the UserLoginService
// to authenticate identity. (Client would use the UDPSessionClient module).
//
//----------------------------------------------------

#include "UDPSessionConnection.h"

extern void		UDPSessionHostRegisterMessageHandler( UDPSessionConnectionMessageReceiveHandler fnMessageHandler );

extern int		UDPSessionHostInitialise( ushort uwHostPort, const char* szLoginServiceURL );

extern void		UDPSessionHostUpdate( float delta );

extern void		UDPSessionHostShutdown( void );





#endif