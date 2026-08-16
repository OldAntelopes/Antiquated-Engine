#include "stdio.h"
#include "OpenDMX.h"
#include "StandardDef.h"
#include <iostream>


//----------------------------------------------------------------------------------------------------
// Originally forked from https://github.com/chloelle/DMX_CPP  but basically completely rewritten now
// 
// Uses ftd2xx.lib and ftd2xx.dll from FTDI to control DMX output via an FTDI USB to serial converter. 
// Tested with the Enttec Open DMX USB interface but should work with any FTDI-based DMX interface.
//  ( https://ftdichip.com/drivers/d2xx-drivers/ )
// 
// - Call InitOpenDMX to initialise the device and start the reader thread
// - Use setDMXValue to set channel values and writeData to send them to the device.
// - Call Shutdown when finished to close the device and stop the reader thread.
//----------------------------------------------------------------------------------------------------
#ifdef INCLUDE_FTD2XX

#define DMX_WRITE_THREADED

OpenDMX*	mspSingleton = NULL;
bool		msbOpenDMXKillThread = false;
uint32		ms_hDMXReaderThread = 0;
uint32		ms_hDMXWriterThread = 0;
HANDLE		mhThreadEvent = 0;

// Singleton constructor
OpenDMX::OpenDMX(bool isVerbose)
{
	mspSingleton = this;
	bufferLength = DMX_BUFFER_SIZE; //dmx 512
	m_FThandle = 0;
	done = false; 
	connected = false; 
	bytesWritten = 0; 
	status = FT_OK;

	m_dllHandle = NULL;
	bits_8 = 8; //dmx
	stop_bits_2 = 2; //dmx
	parity_none = 0; //dmx
	flow_none = 0; //dmx
	purge_rx = 1; 
	purge_tx = 1; 
	verbose = isVerbose;

	//try to load DLL
	m_dllHandle = LoadLibrary("ftd2xx.dll");

	if ( m_dllHandle == NULL) 
	{
		SysDebugPrint( "Failed to load DMX library" );
	}
	
}


//---------------------------
// WritingProc 
// This is the thread procedure for writing data to the device. 
long OpenDMX::WritingProc(long)
{
FT_STATUS res = NULL;

	while(!msbOpenDMXKillThread)
	{
		// TODO Implement this (and remove the FT_Write from the main thread)

		if ( m_bPendingWriteBufferReady )
		{
			mWriteBufferAccessMutex.WaitForMutex();

			//Sets the BREAK condition for the device.
			res = FT_SetBreakOn(m_FThandle);
			if (verbose)
			{
				SysDebugPrint("Setting break condition..."); 
				printErrorCode(res);
			}
	
			//Resets the BREAK condition for the device.
			res = FT_SetBreakOff(m_FThandle);
			if (verbose)
			{
				SysDebugPrint("Resetting break condition..."); 
				printErrorCode(res);
			}

			res = FT_Write(m_FThandle, m_pendingWriteBuffer, DWORD(bufferLength), &bytesWritten );
			if (verbose)
			{
				SysDebugPrint( "Writing %d bytes.", bytesWritten ); 
				printErrorCode(res);
			}
			
			//	FT_STATUS res = FT_Purge(m_FThandle, purge_tx);

			m_bPendingWriteBufferReady = false;
			mWriteBufferAccessMutex.ReleaseMutex();
		}
		Sleep(1);
	}
	return 0;
}

//---------------------------
// ReadingProc 
// This is the thread procedure for reading data from the device. It waits for data to be available and reads it into a buffer.
long OpenDMX::ReadingProc(long)
{
	static char c = 0;
	char* pstr6 = &c;
	FT_STATUS status;
	
	FT_HANDLE handle = m_FThandle;
	HANDLE hEvent = mhThreadEvent;
		
	while(!msbOpenDMXKillThread)
	{
		DWORD dwRead, dwRXBytes;
		BYTE b;
	//	NumberFormatInfo*   provider = new NumberFormatInfo( );

		// TODO : Put this back in to make this event based rather than waking up every 10ms to check for data.  
//		WaitForSingleObject(hEvent, -1);
		if(handle) 
		{
			status = FT_GetQueueStatus(handle, &dwRead);
			if(status != FT_OK) 
			{
				//MessageBox::Show("GError");
				continue;
			}

			while(dwRead && !msbOpenDMXKillThread) 
			{

				status = FT_Read(handle, &b, 1, &dwRXBytes);
				if(status != FT_OK) {
	//				MessageBox::Show("RError");
					continue;
				}
				else 
				{
//					String* s;
	//				s = Convert::ToString(b);
		//			listBox1->Items->Add(s);
				}
				status = FT_GetQueueStatus(handle, &dwRead);
			}
		}
		SysSleep(10);
	}

	if(handle) 
	{
		FT_Close(handle);
	}

	return 0;
}

long OpenDMX::WritingProcStatic(long param )
{
	return( mspSingleton->WritingProc( param ) );
}


long OpenDMX::ReadingProcStatic(long param )
{
	return( mspSingleton->ReadingProc( param ) );
}

//--------------------------------------------------

void		OpenDMX::GetDeviceInfo()
{
}

void	OpenDMX::Shutdown()
{
FT_STATUS res = NULL;
bool runTimeLinkSuccess = FALSE;

	msbOpenDMXKillThread = true;
	SysSleep(20);

	ms_hDMXReaderThread = 0;
	ms_hDMXWriterThread = 0;

	res = FT_Close( m_FThandle );
	if (verbose)
	{
		SysDebugPrint("Closing DMX...");
		printErrorCode(res);
	}
	FreeLibrary(m_dllHandle); 

}


//Start: Open FTDI device 
bool OpenDMX::start()
{
	bool runTimeLinkSuccess = FALSE; 
	FT_STATUS res;

	res = FT_Open( 0, &m_FThandle );
	status = res; 
	if (verbose)
	{
		SysDebugPrint( "Opening DMX..." );
		printErrorCode(res);
	}
	if ( m_FThandle == 0 )
	{
		return true;		
	}
	return false; 
}

//clears buffer array (host)
void OpenDMX::zerosDMXValue(){ 

	for (int i = 0; i < bufferLength; i++){
		m_activeBuffer[i] = 0; 
	}
}

//sets buffer array (host)
void OpenDMX::setDMXValue(int channel, unsigned char value){

	if (channel > 0 && channel <= bufferLength && value >= 0 && value <= 255){ //bounds checking for dmx 512
		m_activeBuffer[channel] = BYTE(value);
	}
	else{
		if (verbose)
		{
			SysDebugPrint( "Illegal arguments for DMX input" ); 
		}
	}
}

//helper function writes buffer array (device) 
//if we don't care about bytes written? [TO-DO] maybe merge with write()
void OpenDMX::writeData(){

	//write some bytes
	bytesWritten = write();

}

//writes buffer array (device)
int OpenDMX::write()
{
	// For testing - make all channels use value of chan 1
//	for ( int loop = 2; loop < bufferLength; loop++ )
//	{
//		buffer[loop] = buffer[1];
//	}

#ifdef DMX_WRITE_THREADED
	// Dont bother writing if we cant immediately get hold of the buffer
	if ( mWriteBufferAccessMutex.WaitForMutex(1) == true )
	{
		//write some bytes
		memcpy(m_pendingWriteBuffer, m_activeBuffer, bufferLength);
		m_bPendingWriteBufferReady = true;
		mWriteBufferAccessMutex.ReleaseMutex();
	}
#else
	FT_STATUS res;

	//Sets the BREAK condition for the device.
	res = FT_SetBreakOn(m_FThandle);
	if (verbose)
	{
		SysDebugPrint("Setting break condition..."); 
		printErrorCode(res);
	}

	//Resets the BREAK condition for the device.
	res = FT_SetBreakOff(m_FThandle);
	if (verbose)
	{
		SysDebugPrint("Resetting break condition..."); 
		printErrorCode(res);
	}

	//write some bytes
	 res = FT_Write(m_FThandle, m_activeBuffer, DWORD(bufferLength), &bytesWritten );
#endif
	return 1; 
}


bool OpenDMX::initOpenDMX(){

	FT_STATUS res = NULL; 

	res = FT_ResetDevice(m_FThandle);
	if (verbose)
	{
		SysDebugPrint("Resetting device..."); 
		printErrorCode(res);
	}

	
	//SET DIVISOR / baud rate
	res = FT_SetDivisor(m_FThandle, unsigned short(12));
	if (verbose)
	{
		SysDebugPrint( "Setting baud rate..."); 
		printErrorCode(res);
	}

	//SET DATA CHARACTERISTICS
	res = FT_SetDataCharacteristics(m_FThandle, bits_8, stop_bits_2, parity_none);
	if (verbose)
	{
		SysDebugPrint("Setting data characteristics..."); 
		printErrorCode(res);
	}

	//SET FLOW CONTROL
	res = FT_SetFlowControl(m_FThandle,flow_none,0,0);
	if (verbose)
	{
		SysDebugPrint("Setting flow control..."); 
		printErrorCode(res);
	}
	
	mhThreadEvent = CreateEvent(NULL, FALSE, FALSE, "");
	res = FT_SetEventNotification(m_FThandle, FT_EVENT_RXCHAR, mhThreadEvent);

//	res = FT_SetBaudRate(m_FThandle, 9600);

	res = FT_ClrRts(m_FThandle);
	if (verbose)
	{
		SysDebugPrint("Clearing request to send control signal..."); 
		printErrorCode(res);
	}

	//Purge receive and transmit buffers in the device
	res = FT_Purge(m_FThandle, purge_tx);
	if (verbose)
	{
		SysDebugPrint("Purging transmit buffer..."); 
		printErrorCode(res);
	}

	res = FT_Purge(m_FThandle, purge_rx);
	if (verbose)
	{
		SysDebugPrint( "Purging receive buffer..."); 
		printErrorCode(res);
	}
	
	//Sets the BREAK condition for the device.
	res = FT_SetBreakOn(m_FThandle);
	if (verbose)
	{
		SysDebugPrint("Setting break condition..."); 
		printErrorCode(res);
	}

	//Resets the BREAK condition for the device.
	res = FT_SetBreakOff(m_FThandle);
	if (verbose)
	{
		SysDebugPrint("Resetting break condition..."); 
		printErrorCode(res);
	}

	ms_hDMXReaderThread = SysCreateThread( ReadingProcStatic, (void*)NULL, 0, 0);
	ms_hDMXWriterThread = SysCreateThread( WritingProcStatic, (void*)NULL, 0, 0);

	return false;
}

//returns OpenDMX error code based on FTD2XX.h enum
void OpenDMX::printErrorCode( int res )
{
	switch(res){
	case 0: SysDebugPrint( "FT_OK" ); break; 
	case 1: SysDebugPrint( "FT_INVALID_HANDLE" ); break; 		
	case 2: SysDebugPrint( "FT_DEVICE_NOT_FOUND" ); break; 
	case 3: SysDebugPrint( "FT_DEVICE_NOT_OPENED" ); break; 
	case 4: SysDebugPrint( "FT_IO_ERROR" ); break; 
	case 5: SysDebugPrint( "FT_INSUFFICIENT_RESOURCES" ); break; 
	case 6: SysDebugPrint( "FT_INVALID_PARAMETER" ); break; 
	case 7: SysDebugPrint( "FT_INVALID_BAUD_RATE" ); break; 
	case 8: SysDebugPrint( "FT_DEVICE_NOT_OPENED_FOR_ERASE" ); break; 
	case 9: SysDebugPrint( "FT_DEVICE_NOT_OPENED_FOR_WRITE" ); break; 
	case 10: SysDebugPrint( "FT_FAILED_TO_WRITE_DEVICE" ); break; 
	case 11: SysDebugPrint( "FT_EEPROM_READ_FAILED" ); break; 
	case 12: SysDebugPrint( "FT_EEPROM_WRITE_FAILED" ); break; 
	case 13: SysDebugPrint( "FT_EEPROM_ERASE_FAILED" ); break; 
	case 14: SysDebugPrint( "FT_EEPROM_NOT_PRESENT" ); break; 
	case 15: SysDebugPrint( "FT_EEPROM_NOT_PROGRAMMED" ); break; 
	case 16: SysDebugPrint( "FT_INVALID_ARGS" ); break; 
	case 17: SysDebugPrint( "FT_NOT_SUPPORTED" ); break; 
	case 18: SysDebugPrint( "FT_OTHER_ERROR" ); break; 
	}
}


#endif // #ifdef INCLUDE_FTD2XX
