
#include <stdio.h>
#include <StdWinInclude.h>
#include <Iphlpapi.h>			// For MAC address - must include "IPHlpApi.Lib"
#include <combaseapi.h>

#include "Platform.h"

// Fetches the MAC address and prints it
void PlatformGetMACaddress( unsigned char* szOutBuffer )
{
IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information for up to 16 NICs
DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer
DWORD dwStatus = GetAdaptersInfo( AdapterInfo, &dwBufLen );

	if ( dwStatus == ERROR_SUCCESS ) 
	{
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to
                                               // current adapter info

		// temp? - Just use the first adaptor's MAC address...
		memcpy( szOutBuffer, pAdapterInfo->Address, pAdapterInfo->AddressLength );
/*
		do 
		{
			memcpy( acMACAddress, pAdapterInfo->Address, pAdapterInfo->AddressLength );
		    pAdapterInfo = pAdapterInfo->Next;    // Progress through 
		}
		while(pAdapterInfo);             
*/
	}
}


void PlatformGetGUID( char* szGUIDOut )
{
GUID *pguid = 0x00;
 
	pguid = new GUID;
	CoCreateGuid(pguid);
	// Convert the GUID to a string
	sprintf( szGUIDOut, "%08x-%04x%04x-%08x-%08x", pguid->Data1, pguid->Data2, pguid->Data3, (unsigned int*)&pguid->Data4[0],(unsigned int*)&pguid->Data4[4] );
}
