
#include <stdio.h>
#include <UnivSocket.h>
#include <StandardDef.h>

#include "../Networking.h"
//#include "../VoiceComms/VoiceComms.h"
#include "../RingBuffer.h"
#include "PeerConnection.h"
#include "PeerStream.h"

#define	STREAM_PACKET_MAX_SIZE		320
#define	STREAM_RECV_BUFFER_SIZE		32768
#define	DECODE_BUFFER_SIZE			65536


void PeerStreamIn::DecodePacketReceived( byte*  pbMsgBody )
{
BYTE*	pbPacketData;
int		nDataLen;
short	wPacketID;
short	wPacketFlags;
short	wPacketSize;

	wPacketSize = *((short*)(pbMsgBody + 2) );
	wPacketID = *((short*)(pbMsgBody + 4) );
	wPacketFlags = *((short*)(pbMsgBody + 6) );
	nDataLen = wPacketSize - 8;
	pbPacketData = pbMsgBody + 8;

	if ( m_wLastPacketReceived == -1 )
	{
		m_wLastPacketReceived = wPacketID;
	}
	else 
	{
		if ( wPacketID < m_wLastPacketReceived )
		{
			if ( (m_wLastPacketReceived - wPacketID) < 500 )
			{
				// Ignore old packets..
				NetworkingUserDebugPrint( 0,"Ignored old VC packet");
				return;
			}
		}
		m_wLastPacketReceived = wPacketID;
	}
#ifdef NETWORK_AUDIO
	VoiceComms::Get().OnPacketReceived( pbPacketData, nDataLen );
#endif
//	mpReceiveBuffer->AddToBuffer( pbPacketData, nDataLen );

//	if ( mpReceiveBuffer->GetBytesAvailableToRead() >= 320 )
//	{
//	int		bytesToDecode = mpReceiveBuffer->ReadFromBuffer( mpbDecodeBuffer, DECODE_BUFFER_SIZE );
//	}
}

PeerStreamIn::PeerStreamIn( PeerConnection* pPeerConnection )
{
	mpPeerConnection = pPeerConnection;
	mpReceiveBuffer	= new CRingBuffer;
	mpReceiveBuffer->Init( STREAM_RECV_BUFFER_SIZE );

	mpbDecodeBuffer = (byte*)SystemMalloc( DECODE_BUFFER_SIZE );
	m_wLastPacketReceived = -1;
}


PeerStreamIn::~PeerStreamIn()
{
	SystemFree( mpbDecodeBuffer );
	delete( mpReceiveBuffer );
}

BOOL	mbWarningDebugFlag = FALSE;

void	PeerStreamOut::AddStreamPacket( byte* pbData, int nDataLen )
{
BYTE	abStreamOutBuffer[1024];
short	wPacketSize = 0;
short*	pwPacketID = (short*)( abStreamOutBuffer );
short*	pwPacketFlags = (short*)( abStreamOutBuffer + 2 );
int		nMsgSize;

	nMsgSize = nDataLen;

	while ( nMsgSize > 400 )
	{
		if ( !mbWarningDebugFlag )
		{
			NetworkingUserDebugPrint( 0, "audio stream packet split" );
			mbWarningDebugFlag = TRUE;
		}
		wPacketSize = (short)( 400 + 4 );  // + 4 for the packetID and packetFlags	
		*pwPacketID = m_wNextPacketNum;
		*pwPacketFlags = 0;
		memcpy( abStreamOutBuffer + 4, pbData, 400 );
		mpPeerConnection->SendStreamPacket( abStreamOutBuffer, wPacketSize );
		m_wNextPacketNum++;
		m_wNextPacketNum %= 30000;
		pbData += 400;
		nMsgSize -= 400;
	}

	if ( nMsgSize > 0 )
	{
		wPacketSize = (short)( nMsgSize + 4 );	// + 4 for the packetID and packetFlags
		*pwPacketID = m_wNextPacketNum;
		*pwPacketFlags = 0;
		memcpy( abStreamOutBuffer + 4, pbData, nMsgSize );
		mpPeerConnection->SendStreamPacket( abStreamOutBuffer, wPacketSize );
		m_wNextPacketNum++;
		m_wNextPacketNum %= 30000;
	}
}
