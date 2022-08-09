
#include "StandardDef.h"

#include "../../GameCommon/Util/Blowfish.h"

#include "WebServerConnectionManager.h"
#include "WebServerTCPConnections.h"
#include "BasicWebServer.h"

BasicWebServerRequestHandler			mfnWebServerGETHandler = NULL;
BasicWebServerRequestHandler			mfnWebServerPOSTHandler = NULL;
BasicWebServerCloseConnectionHandler	mfnWebServerCloseConnectionHandler = NULL;

BLOWFISH_KEY		mxBlowfishClientKey;

//---------------------------------------------------------------------------
// BasicWebServerInit
//   
//---------------------------------------------------------------------------
int			BasicWebServerInit( int nPort )
{
	TCPConnections::InitialiseTCPListener( nPort );

	return( 1 );
}

int		BasicWebServerUpdate( void )
{
	TCPConnections::Update();

	// todo - proper delta time..
	WebServerConnectionManager::Get().UpdateAllConnections( 0.1f );

	return( 1 );
}

int		BasicWebServerShutdown( void )
{
	WebServerConnectionManager::Get().CloseAllConnections();

	TCPConnections::ShutdownTCP();

	WebServerConnectionManager::Get().DeleteAllConnections();

	return( 1 );
}



int			BasicWebServerRegisterGETHandler( BasicWebServerRequestHandler fnHandler )
{
	mfnWebServerGETHandler = fnHandler;
	return( 1 );
}

int			BasicWebServerRegisterCloseConnectionHandler( BasicWebServerCloseConnectionHandler fnHandler )
{
	mfnWebServerCloseConnectionHandler = fnHandler;
	return( 1 );
}

int			BasicWebServerRegisterPOSTHandler( BasicWebServerRequestHandler fnHandler )
{
	mfnWebServerPOSTHandler = fnHandler;
	return( 1 );
}

int			BasicWebServerSendHTTPResponse( ClientConnection* pConnection, const char* szResponseBody, int nBodyLenBytes )
{
	pConnection->SendHTTPResponse( szResponseBody, nBodyLenBytes );
	return( 1 );
}

BasicWebServerRequestHandler			BasicWebServerGetGETHandler( void )
{
	return( mfnWebServerGETHandler );
}

BasicWebServerRequestHandler			BasicWebServerGetPOSTHandler( void )
{
	return( mfnWebServerPOSTHandler );
}

BasicWebServerCloseConnectionHandler		BasicWebServerGetCloseConnectionHandler( void )
{
	return( mfnWebServerCloseConnectionHandler );
}

void			BasicWebServerSetClientKey( const BYTE* pbKey, int nKeyLen )
{
	BlowfishSetKey( pbKey, nKeyLen, &mxBlowfishClientKey );
}

void			BasicWebServerDecrypt( BYTE* pbMem, int nMemLen )
{
int		nNumBlocks;
BYTE*	pbMemBuffIn;
BYTE*	pbMemBuffOut;
int		nBlockLoop;

	nNumBlocks = ((nMemLen-1) / 8) + 1;

	pbMemBuffIn = (BYTE*)malloc( (nNumBlocks*8) + 1 );
	memset( pbMemBuffIn, 0, (nNumBlocks*8) + 1 );
	pbMemBuffOut = (BYTE*)malloc( (nNumBlocks*8) + 1 );

	memcpy( pbMemBuffIn, pbMem, nMemLen );

	for ( nBlockLoop = 0; nBlockLoop < nNumBlocks; nBlockLoop++ )
	{
		BlowfishDecrypt( pbMemBuffIn + (nBlockLoop*8), pbMemBuffOut + (nBlockLoop*8), &mxBlowfishClientKey );
	}
	memcpy( pbMem, pbMemBuffOut, nMemLen );
}
