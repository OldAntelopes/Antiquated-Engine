#ifndef BASIC_WEBSERVER_H
#define BASIC_WEBSERVER_H

#include "ClientConnection.h"
#include "RequestParams.h"


typedef	int(*BasicWebServerRequestHandler)( ClientConnection* pConnection, const char* szRequest, RequestParamsList* pRequestParams );
typedef	int(*BasicWebServerCloseConnectionHandler)( ClientConnection* pConnection );


extern int			BasicWebServerInit( int nPort );

extern int			BasicWebServerRegisterGETHandler( BasicWebServerRequestHandler fnHandler );
extern int			BasicWebServerRegisterPOSTHandler( BasicWebServerRequestHandler fnHandler );
extern int			BasicWebServerRegisterCloseConnectionHandler( BasicWebServerCloseConnectionHandler fnHandler );

extern int			BasicWebServerUpdate( void );

extern int			BasicWebServerSendHTTPResponse( ClientConnection* pConnection, const char* szResponseBody, int nBodyLenBytes );

extern int			BasicWebServerShutdown( void );

extern void			BasicWebServerSetClientKey( const BYTE* pbKey, int nKeyLen );

extern void			BasicWebServerDecrypt( BYTE* pbMem, int nMemLen );


//---------------------------------------------------------------


#endif		// BASIC_WEBSERVER_H