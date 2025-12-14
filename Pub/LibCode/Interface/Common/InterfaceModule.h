#ifndef INTERFACE_MODULE_H
#define INTERFACE_MODULE_H

// TODO - this shouldnt be here
#include "../DirectX/InterfaceTypesDX.h"

class InterfaceInstance;

class InterfaceModule
{
public:
	void	InitialiseModule( InterfaceInstance* pInstance )
	{
		mpInterfaceInstance = pInstance;
		mpInterfaceD3DDevice = NULL;
	}
	
	void		SetGraphicsDevice( LPGRAPHICSDEVICE pDevice )
	{
		mpInterfaceD3DDevice = pDevice;
	}

	LPGRAPHICSDEVICE		mpInterfaceD3DDevice;
	InterfaceInstance*		mpInterfaceInstance;

};



#endif
