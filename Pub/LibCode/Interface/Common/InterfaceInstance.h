#ifndef INTERFACE_INSTANCE_H
#define INTERFACE_INSTANCE_H

// TODO - this shouldnt be here
#include "../DirectX/InterfaceInternalsDX.h"

class  TexturedOverlays;

class InterfaceInstance
{
public:
	void		InitialiseInstance();

	TexturedOverlays*		mpTexturedOverlays;

	LPGRAPHICSDEVICE		mpInterfaceD3DDevice;


};



#endif
