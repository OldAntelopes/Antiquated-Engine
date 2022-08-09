#ifndef GAMECOMMON_UDPSESSION_CLIENT_H
#define GAMECOMMON_UDPSESSION_CLIENT_H

#include "UDPSessionConnection.h"

extern void		UDPSessionClientRegisterMessageHandler( int nConnectionID, UDPSessionConnectionMessageReceiveHandler fnMessageHandler );

extern BOOL		UDPSessionClientInitConnection( int nConnectionID, ulong ulHostIP, ushort uwHostPort, const char* szSessionKey, u64 ullUserUID );

extern void		UDPSessionClientUpdate( float delta );

extern int		UDPSessionClientOnUDPMsgReceive( char* pcMsg, ulong ulFromIP, ushort uwFromPort );

extern BOOL		UDPSessionClientIsConnected( int nConnectionID );

extern BOOL		UDPSessionClientDidError( int nConnectionID );

extern void		UDPSessionClientSendGuaranteedMessage( int nConnectionID, void* pcMsg, int nMsgLen );

extern void		UDPSessionClientSendNonGuaranteedMessage( int nConnectionID, void* pcMsg, int nMsgLen );

extern void		UDPSessionClientShutdown( BOOL bSendDisconnectMsg );

#endif