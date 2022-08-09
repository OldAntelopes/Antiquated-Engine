
#include <stdio.h>
#include <StandardDef.h>

#include "../Networking.h"

#include "ConnectionManager.h"
#include "PeerFileTransfer.h"

//#ifndef SERVER
//#define TRACE_FILE_RECEIVE
//#endif

#define	MAX_PACKET_SIZE		(800)
#define	PACKET_SIZE			(MAX_PACKET_SIZE-500)


int		FileTransfer::msnNumberOfActiveFileSends = 0;
int		FileTransfer::msnDesiredMaxBytesPerSecond = (20*1024);

int		FileTransfer::msnBytesSentInLastSecond = 0;
int		FileTransfer::msnBytesSentLastClearTick = 0;

FileTransfer::FileTransfer( PeerConnection* pConnection )
{
	mpbMem = NULL;
	mnMemSize = 0;
	mnCurrentTransferPos = 0;
	mboAwaitingReceipt = FALSE;
	mboIsSend = FALSE;
	mboActive = FALSE;
	mpConnection = pConnection;
	mfTransferTime = 0.0f;
	mpFileHandle = NULL;
	mpcFilename = NULL;
	mulLastPacketSend = 0;
	mwFileID = -1;
}

FileTransfer::~FileTransfer()
{
	if ( mpbMem != NULL )
	{
		SystemFree( mpbMem );
	}
	if ( mpcFilename != NULL )
	{
		SystemFree( mpcFilename );
	}
	if ( mboIsSend )
	{
		msnNumberOfActiveFileSends--;
	}
	if ( mpFileHandle )
	{
		fclose( mpFileHandle );
	}

}


void	FileTransfer::SetMaximumTransferRate( int nBytesPerSec )
{
	msnDesiredMaxBytesPerSecond = nBytesPerSec;
}

void	FileTransfer::GetTransferThrottleStats( int* pnActiveTransfers, int* pnDesiredBytesPerSec )
{
	*pnActiveTransfers = msnNumberOfActiveFileSends;
	*pnDesiredBytesPerSec = msnDesiredMaxBytesPerSecond;
}


int		FileTransfer::GetPacketSize( void )
{
int		nPacketSize = PACKET_SIZE;
int		nAvgPingTime = 0;
int		nDesiredBytesPerSec;
float	fSendsPerSecond;

	// TODO - Change the packetsize based on the number of sends we're doing and the 
	// overall target for bandwidth use...
	if ( mpConnection )
	{
		nAvgPingTime = mpConnection->GetPingTime();
	}
	if ( nAvgPingTime < 50 ) nAvgPingTime = 50;
	if ( nAvgPingTime > 1000 ) nAvgPingTime = 1000;

	fSendsPerSecond = (1000.0f / (float)nAvgPingTime);

	if ( msnNumberOfActiveFileSends > 1 )
	{
		nDesiredBytesPerSec = msnDesiredMaxBytesPerSecond / msnNumberOfActiveFileSends;
	}
	else
	{
		nDesiredBytesPerSec = msnDesiredMaxBytesPerSecond;
	}

	if ( fSendsPerSecond > 1.0f )
	{
		nPacketSize = (int)(nDesiredBytesPerSec / fSendsPerSecond);
	}
	else
	{	// When ping gets over 1000ms we start to reduce the packet size as we're probably overloading the connection
		nPacketSize = (int)((nDesiredBytesPerSec/2) * fSendsPerSecond);
	}

	// Reduce the calculated packetsize a bit so UDP & FT headers etc are included
	if ( nPacketSize > 40 ) nPacketSize -= 40;
	if ( nPacketSize > MAX_PACKET_SIZE )
	{
		nPacketSize = MAX_PACKET_SIZE;
	}

	// If packetsize is really small, dont send for a bit..
	if ( nPacketSize < 30 ) 
	{
		nPacketSize = 0;
	}
	else if ( nPacketSize < 50 ) 
	{
		nPacketSize = 50;
	}

	if ( nPacketSize < 100 )
	{
		// Override all this and send a 100byte packet at least once every three seconds
		if ( (SysGetTick() - mulLastPacketSend) >= 2000 )
		{
			nPacketSize = 100;
		}
	}
	return( nPacketSize );
}


void	FileTransfer::SendFileReceiptCallback( void* param )
{
FileTransfer*	pTransfer = (FileTransfer*)(param);
	pTransfer->mboAwaitingReceipt = FALSE;
}


BOOL	FileTransfer::DecodeFileSendAcknowledge( void )
{
	if ( ( mpbMem ) ||
		 ( mpFileHandle ) )
	{
		mboActive = TRUE;
		return( TRUE );
	}
	return( FALSE );
}


BOOL	FileTransfer::InitialiseSend( short wFileID, byte* pbMem, int nMemSize, const char* pcFilename, const char* pcReceiverFilename )
{
int		nFileSize;

	if ( pbMem )
	{
		mpbMem = pbMem;
		mnMemSize = nMemSize;
		mwFileID = wFileID;
		mnCurrentTransferPos = 0;
		mboIsSend = TRUE;
		mboActive = FALSE;

		if ( pcFilename )
		{
			mpConnection->SendSysMessageEx( 1, wFileID, (ulong)(nMemSize), (void*)pcReceiverFilename, strlen( pcReceiverFilename ) + 1 );
		}
		else
		{
			mpConnection->SendSysMessage( 1, wFileID, (ulong)(nMemSize) );
		}
		msnNumberOfActiveFileSends++;
		return( TRUE );
	}
	else
	{
		mpFileHandle = fopen( pcFilename, "rb" );
		if ( mpFileHandle )
		{
			mpcFilename = (char*)SystemMalloc( strlen( pcFilename ) + 1 );
			strcpy( mpcFilename, pcFilename );
			nFileSize = SysGetFileSize( mpFileHandle );
			mwFileID = wFileID;
			mnCurrentTransferPos = 0;
			mboIsSend = TRUE;
			mboActive = FALSE;

			mpbMem = NULL;
			mnMemSize = nFileSize;
		
			NetworkingUserDebugPrint( 0,"PConn File send (%s) started\n", pcFilename );
			mpConnection->SendSysMessageEx( 1, wFileID, (ulong)(nFileSize), (void*)pcReceiverFilename, strlen( pcReceiverFilename ) + 1 );
			msnNumberOfActiveFileSends++;
			return( TRUE );
		}
		else
		{
			NetworkingUserDebugPrint( 0,"PConn File send failed (%s) - couldn't open\n", pcFilename );
		}
	}
	return( FALSE );
}


BOOL	FileTransfer::InitialiseReceive( short wFileID, int nMemSize, const char* szFilename )
{
	mboIsSend = FALSE;
	if ( szFilename == NULL )
	{
		mpbMem = (byte*)SystemMalloc( nMemSize );
		if ( mpbMem )
		{
			mwFileID = wFileID;
			mnMemSize = nMemSize;
			mnCurrentTransferPos = 0;
			mboActive = TRUE;
			mpConnection->SendSysMessage( 2, wFileID, 0 );
			mpConnection->OnFileReceiveStarted( wFileID, "" );
			return( TRUE );
		}
	}
	else
	{
	char	acString[256];
	const char*	pcFind = strstr( szFilename,"Clients");
		// Check in case filename has arrived with full path..
		if ( pcFind != NULL )
		{
			pcFind += 8;
			mpcFilename = (char*)SystemMalloc( strlen( pcFind ) + 1 );
			strcpy( mpcFilename, pcFind );
		}
		else
		{
			mpcFilename = (char*)SystemMalloc( strlen( szFilename ) + 1 );
			strcpy( mpcFilename, szFilename );
		}
		
		// TODO - Check here if the folder we're wanting to write to exists (and create it otherwise)
		MakeSubDirectory( mpcFilename, "" );

		sprintf( acString, "%s.tmp", mpcFilename );
		mpFileHandle = fopen( acString, "wb" );
		if ( mpFileHandle )
		{
			mwFileID = wFileID;
			mnMemSize = nMemSize;
			mnCurrentTransferPos = 0;
			mboActive = TRUE;
			mpConnection->SendSysMessage( 2, wFileID, 0 );
			mpConnection->OnFileReceiveStarted( wFileID, mpcFilename );
			return( TRUE );
		}
		else
		{
			NetworkingUserDebugPrint( 0, "Error receiving file - %s\n", mpcFilename );
		}
	}
	return( FALSE );
}


void FileTransfer::DecodePacketReceived( byte*  pbMsgBody )
{
PEER_FILE_PACKET_MSG*	pxMsg = (PEER_FILE_PACKET_MSG*)(pbMsgBody);

	if ( pxMsg->wFileID == mwFileID )
	{
		if ( mnCurrentTransferPos + pxMsg->wPacketLen > mnMemSize )
		{
			// error - overflow
		int	nError = 0;
			NetworkingUserDebugPrint( 0, "Overflow error in PeerFileTransfer.cpp\n" );
		}
		else
		{
			if ( mpFileHandle )
			{
				fwrite( (byte*)(pxMsg+1), pxMsg->wPacketLen, 1, mpFileHandle );
			}
			else
			{
				memcpy( mpbMem + mnCurrentTransferPos, (byte*)(pxMsg+1), pxMsg->wPacketLen );
			}

#ifdef TRACE_FILE_RECEIVE
			NetworkingUserDebugPrint( 0,"FilePacket FileID #%d (%s): from %d, len %d\n", mwFileID, mpcFilename, mnCurrentTransferPos, pxMsg->wPacketLen );
#endif
			mnCurrentTransferPos += pxMsg->wPacketLen;
			if ( mnCurrentTransferPos >= mnMemSize )
			{
				if ( mnCurrentTransferPos > mnMemSize )
				{
					NetworkingUserDebugPrint( 0,"File receive overflow warning in PeerFileTransfer.cpp" );
				}
//				else
//				{
//					PrintConsoleCR("PeerFileTransfer - receive complete", COL_SYS_SPECIAL );
//				}
				// Transfer complete
				if ( mpFileHandle )
				{
				char	acString[256];
				FILE*	pFile;

					fclose( mpFileHandle );
					sprintf( acString, "%s.tmp", mpcFilename );
					// If destination file already exists, delete it
					pFile = fopen( mpcFilename, "rb" );
					if ( pFile )
					{
						fclose( pFile );
						SysDeleteFile( mpcFilename );
					}
					rename( acString, mpcFilename );
					mpConnection->HandleFileReceiveComplete( mwFileID, mpcFilename, NULL, mnMemSize );
				}
				else
				{
				
					mpConnection->HandleFileReceiveComplete( mwFileID, mpcFilename, mpbMem, mnMemSize );
				}
			}
		}
	}
	else
	{
		// urm
		NetworkingUserDebugPrint( 0, "Unknown FileID in PeerFileTransfer.cpp\n" );
	}

}


void	FileTransfer::GetFileTransferProgress( int* pnCurrent, int* pnSize, float* pfTransferTime, char* szFilename, int nBufferLen )
{
	*pnCurrent = mnCurrentTransferPos;
	*pnSize = mnMemSize;
	*pfTransferTime = mfTransferTime;
	if ( mpcFilename )
	{
		strcpy( szFilename, mpcFilename );
	}
}


BOOL	FileTransfer::SendNextPacket( void )
{
int		nMemToSend;
int		nPacketSize = GetPacketSize();

	nMemToSend = mnMemSize - mnCurrentTransferPos;
	if ( nMemToSend == 0 )
	{
		// finished.. shut up shop
		return( FALSE );
	}

	if ( nMemToSend < nPacketSize )
	{
		nPacketSize = nMemToSend;
	}

	if ( !mpConnection->IsBusy() )
	{
		// if the packet size is 0, we're telling this ft to hold for a bit (to keep under bandwidth-use limits)
		if ( nPacketSize > 0 )
		{
		BYTE	abBuff[MAX_PACKET_SIZE+100];
		PEER_FILE_PACKET_MSG*	pxMsg = (PEER_FILE_PACKET_MSG*)(abBuff);
			pxMsg->bMsgCode = PEER_GUARANTEED_FILE_PACKET;
			pxMsg->wFileID = mwFileID;
			pxMsg->wPacketLen = (short)nPacketSize;

			if ( mpbMem )
			{
				memcpy( (byte*)( pxMsg+1 ), mpbMem + mnCurrentTransferPos, nPacketSize );
			}
			else
			{
				fread( (byte*)(pxMsg+1), nPacketSize, 1, mpFileHandle );
			}
			
			if ( mpConnection->SendMessageGuaranteedEx( (byte*)( pxMsg ), nPacketSize + sizeof(PEER_FILE_PACKET_MSG), SendFileReceiptCallback, (void*)(this) ) )
			{
//				NetworkingUserDebugPrint( 0, "Filesend #%d: Packet %d, len %d\n", mwFileID, mnCurrentTransferPos, nPacketSize );
				mulLastPacketSend = SysGetTick();
				mnCurrentTransferPos += nPacketSize;
				if ( !mpConnection->IsUsingTcp() )
				{
					mboAwaitingReceipt = TRUE;
				}
				if ( mnCurrentTransferPos == mnMemSize )
				{
					return( FALSE );
				}
			}
			else	// Guaranteed send failed
			{
				NetworkingUserDebugPrint( 0, "World->Client : Send failure\n" );
				if ( !mpbMem )
				{
					fseek( mpFileHandle, -nPacketSize, SEEK_CUR );
				}
			}
		}
	}
	return( TRUE );
}

BOOL	FileTransfer::Update( float fDeltaTime )
{
	if ( mboActive )
	{
		if ( mboIsSend )
		{
			if ( mboAwaitingReceipt == FALSE )
			{
				if ( mnCurrentTransferPos < mnMemSize  )
				{
					SendNextPacket();
					return( TRUE );
				}
				else
				{
					// finished.. shut up shop
					return( FALSE );
				}
			}
		}
		else	// is receive
		{
			if ( mnCurrentTransferPos != 0 )
			{
				mfTransferTime += fDeltaTime;
			}

			if ( mnCurrentTransferPos == mnMemSize )
			{
				// done.. finish up
				return( FALSE );
			}
		}
	}
	return( TRUE );
}
