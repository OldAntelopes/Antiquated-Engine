
#include <stdio.h>
#include <stdarg.h>

#include <UnivSocket.h>
#include <StandardDef.h>

#include "Networking.h"

#include "PeerConnection/ConnectionManager.h"
#include "PeerConnection/PeerFileTransfer.h"

#include "NetworkConnection.h"


NetworkUserPrintFunction		mfnUserPrintFunction = NULL;

namespace Uni
{


BOOL	Networking::Initialise( NetworkingInitParams* pParams )
{
BOOL	bRet = TRUE;

#ifdef NETWORK_AUDIO
	VoiceComms::Get().Initialise();
#endif // #ifdef NETWORK_AUDIO
	ConnectionManager::Get().RegisterNewConnectionCallback( pParams->m_fnNewConnectionCallback );

	FileTransfer::SetMaximumTransferRate( pParams->m_nMaxFileTransferSpeed );

	if ( pParams->m_bUseNetworkConnection )
	{
		if ( NetworkConnection::Get().InitialiseUDP( pParams->m_uwNetworkConnectionPort, pParams->m_fnNetworkConnectionMessageHandler, 1 ) == FALSE )
		{
			return( FALSE );
		}
		NetworkConnection::Get().SetLogAnnotationCallback( pParams->m_fnLogAnnotationCallback );
		mfnUserPrintFunction = pParams->m_fnNetworkUserPrintFunction;
		if ( pParams->m_bTCPServer )
		{
			NetworkConnection::Get().InitialiseTCPListener( pParams->m_uwNetworkConnectionPort + 1, pParams->m_fnNetworkConnectionMessageHandler, 1 );
		}
	}

#ifdef NETWORK_UPNP
	UPNPInit();
#endif

	return( bRet );
}


void	Networking::Update( float fDeltaTime )
{
#ifdef NETWORK_AUDIO
	VoiceComms::Get().Update( fDeltaTime );
#endif // #ifdef NETWORK_AUDIO
	ConnectionManager::Get().UpdateAllConnections(fDeltaTime);
	NetworkConnection::Get().Update(fDeltaTime);

#ifdef NETWORK_UPNP
	UPNPUpdate();
#endif
}



void	Networking::Shutdown( void )
{
#ifdef NETWORK_UPNP
	UPNPFree();
#endif

#ifdef NETWORK_AUDIO
	VoiceComms::Get().Shutdown();
#endif // #ifdef NETWORK_AUDIO
	ConnectionManager::Get().CloseAllConnections();
	NetworkConnection::Get().ShutdownTCP();
	NetworkConnection::Get().ShutdownUDP();
}



} // namespace Uni

void		NetworkingGetFileTransferStats( int* pnActiveSends, int* pnDesiredBytesPerSec )
{
	FileTransfer::GetTransferThrottleStats( pnActiveSends, pnDesiredBytesPerSec );
}


BOOL	NetworkingOnLeftMouseUp( void )
{
#ifdef NETWORK_AUDIO
	return( VoiceComms::Get().OnLeftMouseUp() );
#else
	return( FALSE );	
#endif // #ifdef NETWORK_AUDIO
}


void	NetworkingEnableVoiceComms( BOOL bFlag )
{
#ifdef NETWORK_AUDIO
	if ( bFlag )
	{
		VoiceComms::Get().Enable( true );
	}
	else
	{
		VoiceComms::Get().Enable( false );
	}
#endif // #ifdef NETWORK_AUDIO
}

BOOL	NetworkingIsVoiceCommsEnabled( void )
{
#ifdef NETWORK_AUDIO
	return(	VoiceComms::Get().IsEnabled() );
#else
	return( FALSE );	
#endif // #ifdef NETWORK_AUDIO
}



int		NetworkingPeerConnectionReceiveGuaranteedMessage( BYTE* pbMsg )
{
//	NetworkingUserDebugPrint( 0, "PeerConnGuar : msgID %d", *((short*)pbMsg +2) );
	return( ConnectionManager::Get().ReceiveMessage( pbMsg ) );
}

int		NetworkingPeerConnectionReceiveSysMessage( BYTE* pbMsg )
{
	return( ConnectionManager::Get().ReceiveSysMessage( pbMsg ) );

}

void	NetworkingUpdate( float fDeltaTime )
{
	Uni::Networking::Update( fDeltaTime );
}


void		NetworkingDebugPrint( const char* text, ... )
{
char		acString[8192];
va_list		marker;
ulong*		pArgs;
int			nLen;

	pArgs = (ulong*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );

	nLen = strlen(acString) ;

	if ( nLen > 0)
	{
		SysDebugPrint( acString );
	}

}

void		NetworkingUserDebugPrint( int mode, const char* text, ... )
{
char		acString[1024];
va_list		marker;
ulong*		pArgs;
int			nLen;

	pArgs = (ulong*)( &text ) + 1;

    va_start( marker, text );     
	vsprintf( acString, text, marker );

	nLen = strlen(acString) ;
	if ( nLen > 0)
	{
		if ( mfnUserPrintFunction )
		{
			mfnUserPrintFunction( mode, acString );
		}
		else
		{
			NetworkingDebugPrint( acString );
		}
	}
}

