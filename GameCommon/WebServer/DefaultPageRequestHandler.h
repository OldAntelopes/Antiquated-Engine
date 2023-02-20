#ifndef BASICWEBSERVER_DEFAULT_PAGE_REQUEST_HANDLER_H
#define BASICWEBSERVER_DEFAULT_PAGE_REQUEST_HANDLER_H

class DefaultRequestBuffer;

//------------------------------------------
//  Each request handler should inherit from RequestHandlerBase and register itself with REGISTER_REQUEST_HANDLER
//   e.g. 
//				class IndexRequest : public RequestHandlerBase
//				{
//				public:
//					virtual BOOL	HandleRequest(  DefaultRequestBuffer*	pDefaultRequestBuffer, UserRecord* pUserRecord )
//					{
//						DoStuff;
//							return (TRUE) - if the request is completed and the buffer can be deleted, 
//								or (FALSE) - if you want to keep the buffer around to allow async operations (you should delete it later with DefaultPageRequestHandlerDeleteDefaultRequestBuffer)
//					}
//				}
//				
//		REGISTER_REQUEST_HANDLER( IndexRequest, "index.html" );
//
//		Note you can also do 
//				(e.g) REGISTER_REQUEST_HANDLER( IndexRequest, "index" );	 - to handle all cases of index.xxx not covered by a specific handler
//		and
//				REGISTER_REQUEST_HANDLER( DefaultRequest, "" );	 - to catch all requests not handled by others
//	
//-------------------------------------------------------------------------


class RequestHandlerBase
{
public:
	// HandleRequest should return TRUE if it has fully handled the request (and the DefaultRequestBuffer can be deleted)
	//  FALSE if not, in which case it needs to clean up the DefaultRequestBuffer itself..
	virtual BOOL	HandleRequest(  DefaultRequestBuffer*	pDefaultRequestBuffer ) = 0;

};


//------------------------------------------------------------------------

// request buffer..
class DefaultRequestBuffer
{
public:
	DefaultRequestBuffer()
	{
		mszRequest = NULL;
		mpClientConnection = NULL;
		mpNext = NULL;
		mszResponseBody = NULL;
	}

	~DefaultRequestBuffer()
	{
		SAFE_FREE( mszRequest );
		SAFE_FREE( mszResponseBody );
	}

	char*				mszRequest;
	RequestParamsList	mRequestParams;
	ClientConnection*	mpClientConnection;
	char*				mszResponseBody;

	DefaultRequestBuffer*		mpNext;
};



extern void		DefaultPageRequestHandlerInit( void );

extern void		DefaultPageRequestHandlerShutdown( void );

extern void		DefaultPageRequestHandlerSendResponse( DefaultRequestBuffer* pOriginalRequest, int nResponseCode, const char* pbBody, int nBodyLen );

extern void		DefaultPageRequestHandlerDeleteDefaultRequestBuffer( DefaultRequestBuffer* pDefaultRequestBufferToDelete );

extern int		DefaultPageRequestHandler( ClientConnection* pConnection, const char* szRequest, RequestParamsList* pRequestParams );

//-----------------------------------------------------
// Request Registration
// 
// Each Request type should include the define 
//  
// REGISTER_REQUEST_HANDLER( [class_name], [text_name] )
//
//--------------------------------------------------------------------

typedef	RequestHandlerBase*	(*RequestHandlerNewFunction)( void );

class RegisteredRequestHandlerList
{
public:
	~RegisteredRequestHandlerList()
	{
		SAFE_DELETE( mpRequestHandlerSingleton );
		SAFE_FREE( mszRegisteredRequestName );
	}

	static BOOL		Register( const char* szRequestName, RequestHandlerNewFunction fnNewRequest );

	char*							mszRegisteredRequestName;
	RequestHandlerNewFunction		mfnRequestNew;
	RequestHandlerBase*				mpRequestHandlerSingleton;

	RegisteredRequestHandlerList*		mpNext;
	
};


// this registers a derived class in the factory method of the base class
// it adds a factory function named create_NAME()
// and calls Base::reg() by the help of a dummy static variable to register the function
#define REGISTER_REQUEST_HANDLER(_classname,_textname) \
namespace { \
	RequestHandlerBase* create_ ## _classname() {  return new _classname; } \
	static BOOL _classname ## _creator_registered = RegisteredRequestHandlerList::Register( _textname, create_ ## _classname); }




#endif
