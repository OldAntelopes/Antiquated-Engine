#ifndef INTERFACE_MODULE_H
#define INTERFACE_MODULE_H

// TODO - this shouldnt be here
#include "../DirectX/InterfaceTypesDX.h"

class InterfaceInstance;

class InterfaceModule
{
public:
	void		InitialiseModule( LPGRAPHICSDEVICE pDevice, InterfaceInstance* pInstance )
	{
		mpInterfaceD3DDevice = pDevice;
		mpInterfaceInstance = pInstance;
	}

	LPGRAPHICSDEVICE		mpInterfaceD3DDevice;
	InterfaceInstance*		mpInterfaceInstance;

};



#endif
