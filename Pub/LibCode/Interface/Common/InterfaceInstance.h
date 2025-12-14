#ifndef INTERFACE_INSTANCE_H
#define INTERFACE_INSTANCE_H

// TODO - this shouldnt be here
#include "../DirectX/InterfaceTypesDX.h"

#include "InterfaceModule.h"
#include "../Common/Overlays/Overlays.h"
#include "../Common/Font/FontCommon.h"

class TexturedOverlays;
class InterfaceInternalsDX;
class FontSystem;

class InterfaceInstance
{
public:
	InterfaceInstance();

	void		InitD3D( HWND hWindow, BOOL bMinBackBufferSize );

	void		InitialiseInstance( BOOL bUseDefaultFonts );

	int			NewFrame( uint32 ulCol );
	void		BeginRender( void);
	void		Draw( void );
	void		EndRender( void);
	void		Present( void );

	BOOL		IsInRender( void );

	TexturedOverlays*		mpTexturedOverlays;
	FontSystem*				mpFontSystem;
	InterfaceInternalsDX*	mpInterfaceInternals;

	LPGRAPHICSDEVICE		mpInterfaceD3DDevice;
protected:
	void		SetDevice( LPGRAPHICSDEVICE pDevice );

	BOOL	mboInterfaceInitialised = FALSE;
	bool	mbIsInScene = false;
	HWND	mhWindow;

};



#endif
