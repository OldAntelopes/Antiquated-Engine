#ifndef INTERFACE_INSTANCE_H
#define INTERFACE_INSTANCE_H

// TODO - this shouldnt be here
#include "../DirectX/InterfaceTypesDX.h"		// Temp - only for LPGRAPHICSDEVICE 

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
	void		FreeAll( void );
	void		ReleaseForDeviceReset( void );
	void		RestorePostDeviceReset( void );

	void		SetTextureAsCurrentDirect( void* pTexture );

	TexturedOverlays*		mpTexturedOverlays;
	FontSystem*				mpFontSystem;
	InterfaceInternalsDX*	mpInterfaceInternals;

	// TODO - Move this into InterfaceInternals and make sure all systems only refer to this vers
		// (So we can remove the DX.h headers from this file)
	LPGRAPHICSDEVICE		mpInterfaceD3DDevice;

	void		DrawAllElements( void );
protected:
	void		SetDevice( LPGRAPHICSDEVICE pDevice );

	BOOL	mboInterfaceInitialised = FALSE;
	bool	mbIsInScene = false;
	HWND	mhWindow;

};



#endif
