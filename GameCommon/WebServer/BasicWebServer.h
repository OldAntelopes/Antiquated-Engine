#ifndef BASIC_WEBSERVER_H
#define BASIC_WEBSERVER_H

#include "ClientConnection.h"
#include "RequestParams.h"

// -------------------------------------------------
// BasicWebServer
//   Opens up a TCP port and provides basic HTTP request handling. 
// 
//  Usage: 
//    Call BasicWebServerInit with the port you want the webserver to appear on
//    Call BasicWebServerUpdate at regular intervals to process requests
// 
//   RequestHandling
//  *****
//     Option 1) Use the DefaultPageRequestHandler:
//  
//				Each request handler should inherit from RequestHandlerBase and register itself with REGISTER_REQUEST_HANDLER
//				The request handler will be given a RequestBuffer containing all the details of the request (endpoint, connection details, request params and body)
//
//				  e.g. 
//						--------------------------------------------------------------------------------------------- Your code 
//						class IndexRequest : public RequestHandlerBase
//						{
//						public:
//							virtual BOOL	HandleRequest(  RequestBuffer*	pRequestBuffer )
//							{
//								DoStuff;
//								DefaultPageRequestSendResponse( response, responsecode, etc )
//								return (TRUE) - if the request is completed and the buffer can be deleted, 
//									or (FALSE) - if you want to keep the buffer around to allow async operations (you should delete it later with DefaultPageRequestHandlerDeleteRequestBuffer)
//							}
//						}
//				
//						REGISTER_REQUEST_HANDLER( IndexRequest, "index.html" );
//						--------------------------------------------------------------------------------------------- end your code 
//
//		Note you can also use
//				(e.g) REGISTER_REQUEST_HANDLER( IndexRequest, "index" );	 - to handle all cases of index.xxx not covered by a specific handler
//		and
//				REGISTER_REQUEST_HANDLER( DefaultRequest, "" );	 - to catch all requests not handled by others
//
//  *****						
//	   Option 2) Call BasicWebServerRegister[GET/POST]Handler  to provide a custom page request handler that will get called with every request
//		
//		  e.g  BasicWebServerRegisterGetHandler( MyPageHandler )
//					->  Hit IP:Port/foo.html
//					->  MyPageHandler( connectionDetails, "foo.html", params ) will be triggered
//
//		 Use Option 2 particularly if you want to do extra stuff like having custom validation before responding to the request
//----------------------------------------------------------

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

#include "DefaultPageRequestHandler.h"


#endif		// BASIC_WEBSERVER_H