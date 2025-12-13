#ifndef INTERFACE_INSTANCE_H
#define INTERFACE_INSTANCE_H

// TODO - this shouldnt be here
#include "../DirectX/InterfaceInternalsDX.h"

class TexturedOverlays;
class InterfaceInternals;

class InterfaceInstance
{
public:
	void		InitialiseInstance();

	TexturedOverlays*		mpTexturedOverlays;
	InterfaceInternalsDX*	mpInterfaceInternals;

	LPGRAPHICSDEVICE		mpInterfaceD3DDevice;


};



#endif
