#ifndef NETWORKING_PEER_STREAM_H
#define NETWORKING_PEER_STREAM_H


//-------------------------------------------------------------------
class	PeerConnection;
class	CRingBuffer;

class PeerStreamOut
{
public:
	PeerStreamOut( PeerConnection* pPeerConnection )
	{
		mpPeerConnection = pPeerConnection;
		m_wNextPacketNum = 0;
	}
	~PeerStreamOut() {}

	void	AddStreamPacket( byte* pbData, int nDataLen );
private:
	PeerConnection* mpPeerConnection;
	short		m_wNextPacketNum;
};


//-------------------------------------------------------------------

class PeerStreamIn
{
public:
	PeerStreamIn( PeerConnection* pPeerConnection );
	~PeerStreamIn();

	void DecodePacketReceived( byte*  pbMsgBody );
private:
	PeerConnection* mpPeerConnection;
	CRingBuffer*	mpReceiveBuffer;
	BYTE*			mpbDecodeBuffer;
	short			m_wLastPacketReceived;

};



#endif
