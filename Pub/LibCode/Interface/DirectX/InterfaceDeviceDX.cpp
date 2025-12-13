
#include <stdio.h>
#include <vector>
#include "InterfaceInternalsDX.h"

#include <StandardDef.h>
#include <Interface.h>

#include "DX9Device/DX9Device.h"
#include "../Common/InterfaceDevice.h"

std::vector<LPGRAPHICSDEVICE>		m_InterfaceDeviceList;



INTERFACE_API INTERFACE_DEVICE_HANDLE	InterfaceAdditionalDisplayDevice( HWND hWindow )
{
	if ( m_InterfaceDeviceList.empty() )
	{
		m_InterfaceDeviceList.push_back( mpInterfaceD3DDevice );
	}
	
	LPGRAPHICSDEVICE	pNewDevice = InterfaceCreateNewGraphicsDevice( hWindow, FALSE );

	INTERFACE_DEVICE_HANDLE		hHandle = m_InterfaceDeviceList.size();
	m_InterfaceDeviceList.push_back( pNewDevice );
	return( hHandle );

}


INTERFACE_API void	InterfaceSwitchDevice( INTERFACE_DEVICE_HANDLE hNewDevice )
{
	if ( !m_InterfaceDeviceList.empty() )
	{
		if ( hNewDevice == NOTFOUND )
		{
			mpInterfaceD3DDevice = m_InterfaceDeviceList[0];
		}
		else
		{
			mpInterfaceD3DDevice = m_InterfaceDeviceList[hNewDevice];
		}
	}

}
