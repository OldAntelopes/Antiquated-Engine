
#ifndef NETWORKING_PEER_FILETRANSFER_H
#define NETWORKING_PEER_FILETRANSFER_H

class PeerConnection;

class FileTransfer
{
friend class PeerConnection;
public:
	FileTransfer( PeerConnection* pConnection );
	~FileTransfer();

	BOOL	InitialiseSend( short wFileID, byte* pbMem, int nMemSize, const char* szFilename, const char* szReceiverFilename );
	BOOL	InitialiseReceive( short wFileID, int nMemSize, const char* szFilename );

	static void		SendFileReceiptCallback( void* param );
	BOOL	Update( float fDeltaTime );

	void	GetFileTransferProgress( int* pnCurrent, int* pnSize, float* pfTransferTime, char* szFilename, int nBufferLen );

	static void		SetMaximumTransferRate( int nBytesPerSec );

	static void		GetTransferThrottleStats( int* pnActiveTransfers, int* pnDesiredBytesPerSec );
protected:
	// Received by the file sender when the remote player signals they're ready to receive the file
	BOOL	DecodeFileSendAcknowledge( void );
	void	DecodePacketReceived( byte*  pbMsgBody );
	BOOL	SendNextPacket( void );
private:
	int		GetPacketSize( void );
	static short	GetNewFileID( void );

	byte*	mpbMem;
	int		mnMemSize;
	int		mnCurrentTransferPos;
	short	mwFileID;
	float	mfTransferTime;
	BOOL	mboIsSend;
	BOOL	mboActive;
	BOOL	mboAwaitingReceipt;
	PeerConnection*		mpConnection;
	FILE*	mpFileHandle;
	char*	mpcFilename;
	ulong	mulLastPacketSend;

	static	int		msnNumberOfActiveFileSends;
	static	int		msnDesiredMaxBytesPerSecond;
	static  int		msnBytesSentInLastSecond;
	static  int		msnBytesSentLastClearTick;

};


#endif
