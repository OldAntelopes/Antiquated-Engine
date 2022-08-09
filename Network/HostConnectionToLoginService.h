#ifndef GAMECOMMON_HOST_CONNECTION_TO_LOGIN_SERVICE_H
#define GAMECOMMON_HOST_CONNECTION_TO_LOGIN_SERVICE_H

//------------------------------------------------
// HostConnectionToLoginService
//
//  Include this module on a server that wants to validate a user with the login service.
//  Procedure being:
//   Client logs in to login service and gets a session key & ullUserUID
//   Client sends session key and userUID to the host server it wants to connect to
//   Host server sends session key and userUID to login service to confirm the identity of the client
//   
//  To implement, host server should :
//
//   1) Call  HostConnectionToLoginServiceInitialise  on init
//   2) Call  HostLoginServiceRegisterSessionResponse  to register handler
//   3) Call  HostLoginServiceValidateSessionKey   for each client that joins 
//   4)  If response is 200, client is all good and can be allowed in
//
//---------------------------------------------------------------------

typedef	void(*ValidateSessionResponse)( int nResponseCode, u64 ullUserUID, const char* szSessionKey, void* vpCBParam );

extern void		HostConnectionToLoginServiceInitialise( const char* szLoginServiceURL );

extern void		HostLoginServiceRegisterSessionResponse( ValidateSessionResponse fnResponseCallback );

extern void		HostLoginServiceValidateSessionKey( const char* szSessionKey, u64 ullUserUID, void* vpCBParam );



#endif