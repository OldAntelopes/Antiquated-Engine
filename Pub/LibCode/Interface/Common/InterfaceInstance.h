#ifndef INTERFACE_INSTANCE_H
#define INTERFACE_INSTANCE_H

// TODO - this shouldnt be here
#include "../DirectX/InterfaceTypesDX.h"

class TexturedOverlays;
class InterfaceInternals;
class FontSystem;

class InterfaceInstance
{
public:
	void		InitialiseInstance();

	void		InitD3D( BOOL bMinBackBufferSize );
	int			NewFrame( uint32 ulCol );

	TexturedOverlays*		mpTexturedOverlays;
	FontSystem*				mpFontSystem;
	InterfaceInternalsDX*	mpInterfaceInternals;

	LPGRAPHICSDEVICE		mpInterfaceD3DDevice;
protected:
	void		SetDevice( LPGRAPHICSDEVICE pDevice );


};



#endif
