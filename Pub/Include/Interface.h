#ifndef UNIVERSAL_INTERFACE_H
#define UNIVERSAL_INTERFACE_H

#define INTERFACE_API

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	
#ifndef BASETYPES
#ifndef MIT_TYPES
typedef unsigned short			ushort;
typedef unsigned long			ulong;
typedef unsigned char			uchar;
typedef unsigned int			uint;
#endif
#endif
	
#ifndef TEXTURE_HANDLE
#define		TEXTURE_HANDLE		int
#endif

/**********************************************
 *****   Universal Interface Library     ******
 **********************************************
 **                                          **
 **	This library includes all the basic      **
 ** initialisation calls to create and		 **
 ** render to a graphics window in the early **
 ** 21st century.							 **
 **											 **
 ** It includes 2d drawing					 **
 ** functions for the rendering of text,     **
 ** untextured rectangles, and textured		 **
 ** rectangles & triangles and etc		     **
 **											 **
 **  Made by Mit : 2000 - 2010 ish			 **
 **		           http://theuniversal.net	 **
 **********************************************/

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------
//**********           Main Functions				  ********
//		These are the main group of things you'll probably  --
//      use most often.										--
//------------------------------------------------------------

/*****************************************************
 ***** Interface - Initialisation Functions    *******
 *****************************************************/

extern INTERFACE_API void	InterfaceInitWindow( const char* pcWindowTitle, void* pWinClass, BOOL bAllowResize  );

// First initialisation call - intialises the display device (e.g. Direct3d)
extern INTERFACE_API void	InterfaceInitDisplayDevice( BOOL boMinRenderPageSize );

extern INTERFACE_API void	InterfaceSetInitialSize( BOOL bFullscreen, int windowedWidth, int windowedHeight, BOOL bSmallFlag );

// Second intialisation call - initialises this library (and loads up core textures)
extern INTERFACE_API void	InterfaceInit( BOOL bUseDefaultFont );

extern INTERFACE_API void	InterfaceFreeAllD3D( void );
extern INTERFACE_API void	InterfaceFree( void );

extern BOOL					InterfaceIsOversized( void );

/***************************************************
 ******  Interface - Main Update Functions  ********
 ***************************************************/
extern INTERFACE_API int	InterfaceNewFrame( ulong ulClearCol );

extern INTERFACE_API void	InterfaceBeginRender( void );
extern INTERFACE_API void	InterfaceDraw( void );
extern INTERFACE_API void	InterfaceDrawUI( float fScreenAngleZ );
extern INTERFACE_API void	InterfaceDrawNoMatrix( void );

extern INTERFACE_API void	InterfaceEndRender( void );
extern INTERFACE_API BOOL	InterfaceIsInRender( void );

extern INTERFACE_API void	InterfacePresent( void );

/********************************************************************
 ******  Interface - The good stuff for drawing 2d things   *********
 ********************************************************************/

// ------------  Rendering untextured stuff - rectangles, boxes, lines etc
extern INTERFACE_API void	InterfaceRect( int nLayer, int nX, int nY, int nWidth, int nHeight, ulong ulCol );
extern INTERFACE_API void	InterfaceOutlineBox( int nLayer, int nX, int nY, int nWidth, int nHeight, ulong ulCol );
extern INTERFACE_API void	InterfaceShadedBox( int nLayer, int nX, int nY, int nWidth, int nHeight, int nStyle );
extern INTERFACE_API void	InterfaceShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, ulong ulCol1, ulong ulCol2,ulong ulCol3, ulong ulCol4 );
extern INTERFACE_API void	InterfaceLine( int nLayer, int nX1, int nY1, int nX2, int nY2, ulong ulCol1, ulong ulCol2 );
extern INTERFACE_API void	InterfaceTri( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, ulong ulCol1, ulong ulCol2, ulong ulCol3 );
extern INTERFACE_API void	InterfaceOverlaysAdditive( BOOL );

// ------------- Font drawing
extern INTERFACE_API void	InterfaceText( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int font );
extern INTERFACE_API void	InterfaceTextRight( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int font );
extern INTERFACE_API void	InterfaceTextCenter( int nLayer, int nX1, int nX2, int nY, const char* szString, ulong ulCol, int font );
extern INTERFACE_API void	InterfaceTextCentre( int nLayer, int nX1, int nY, const char* szString, ulong ulCol, int font );

extern INTERFACE_API void	InterfaceTextScaled( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int font, float fSize );

//extern INTERFACE_API void	InterfaceTextCentre( int nLayer, int nX, int nY, const char* szString, ulong ulCol, int font );

extern INTERFACE_API char*	InterfaceTextLimitWidth( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth );
extern INTERFACE_API int	InterfaceTextBox( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, BOOL bLeftAlign );
extern INTERFACE_API int	InterfaceTextRect( int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, int nMaxHeight );
extern INTERFACE_API int	InterfaceTextRectGetUsedWidth( const char* szString, int font, int nMaxWidth );

extern INTERFACE_API int	InterfaceTextGetWidth( const char* pcString, int nFont );
extern INTERFACE_API int	InterfaceTextGetHeight( const char* pcString, int nFont );

enum
{
	FONT_FLAG_UNDERLINED = 0x1,
	FONT_FLAG_BOLD = 0x2,
	FONT_FLAG_ITALIC = 0x4,
	FONT_FLAG_SMALL = 0x8,
	FONT_FLAG_LARGE = 0x10,
	FONT_FLAG_DROP_SHADOW = 0x20,
	FONT_FLAG_GIANT = 0x40,
	FONT_FLAG_MASSIVE = 0x80,
	FONT_FLAG_ENORMOUS = 0x100,
	FONT_FLAG_TINY = 0x200,
};

extern INTERFACE_API void	InterfaceSetFontFlags( int nFlag);
extern INTERFACE_API void	InterfaceSetFontSize( int nFont, int nSize );
extern int		InterfaceTextGetHeightUsed( const char* szString, int nFont, int width );

// ------------- Texture loading 
extern INTERFACE_API int	InterfaceLoadTexture( const char* szFilename, int nFlags );
extern INTERFACE_API int	InterfaceLoadTextureFromArchive( const char* szFilename, int nFlags, int nArchiveHandle );
extern INTERFACE_API int	InterfaceCreateBlankTexture( int nWidth, int nHeight, int Mode);
extern INTERFACE_API int	InterfaceGetTexture( const char* szFilename, int nFlags );
extern INTERFACE_API int	InterfaceGetTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags );
extern INTERFACE_API int	InterfaceGetTextureSize( int nTextureHandle, int* pnW, int* pnH );
extern INTERFACE_API BYTE*	InterfaceLockTexture( int nTextureHandle, int* pnPitch, int* pnFormat, int nFlags );
extern INTERFACE_API void	InterfaceUnlockTexture( int nTextureHandle );
extern INTERFACE_API void	InterfaceExportTexture( int nTextureHandle, const char* szOutputFilename, int nMode );
extern INTERFACE_API void	InterfaceReleaseTexture( int nTextureHandle );

extern INTERFACE_API void	InterfaceTextureGetColourAtPoint( byte* pbLockedTextureData, int nPitch, int nFormat, int x, int y, float* pfRed, float* pfGreen, float* pfBlue, float* pfAlpha );

#define SAFE_RELEASE_TEXTURE(p)      { if(p != NOTFOUND) { InterfaceReleaseTexture(p); p=NOTFOUND; } }

// ------------- Textured overlays - Textures need to loaded once (see above), but each frame a new
// -------------	 				 overlay needs to be created for any textures that you intend to use.
extern INTERFACE_API int	InterfaceCreateNewTexturedOverlay( int nLayer, int nTextureHandle );
extern INTERFACE_API int	InterfaceCreateUntexturedOverlay( int nLayer );

typedef enum
{
	RENDER_TYPE_NORMAL = 0,
	RENDER_TYPE_ADDITIVE,
	RENDER_TYPE_COLORBLEND,
	RENDER_TYPE_LAYER2,
	RENDER_TYPE_ALPHATEST,
	RENDER_TYPE_SUBTRACTIVE,
	RENDER_TYPE_ALPHA_SUBTRACTIVE,
	RENDER_TYPE_ADDITIVE_NOCLAMP,
	RENDER_TYPE_NORMAL_NOCLAMP,
	
} INTF_RENDER_TYPES;

extern INTERFACE_API void	InterfaceOverlayRenderType( int nOverlayNum, INTF_RENDER_TYPES nRenderType );

// ------------- Textured overlay drawing functions - to draw sprites, rectangles and triangles
extern INTERFACE_API void	InterfaceSprite( int nOverlayNum, int nX, int nY, float fTexGrid, int nTexGridNum, ulong ulCol, float fRotation, float fScale );
extern INTERFACE_API void	InterfaceTexturedRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, ulong ulCol, float fU, float fV, float fU2, float fV2 );
extern INTERFACE_API void	InterfaceTexturedTri( int nOverlayNum, int* pnScreenCoords, float* pfTextureCoords, ulong ulCol );
extern INTERFACE_API void	InterfaceTexturedRectShaded( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, ulong ulCol, ulong ulCol2, float fU, float fV, float fU2, float fV2 );

typedef int			IMAGEHANDLE;

// -------------- Images  
//    (For jpegs etc specifically when you want to keep the image size exact and unmodified by it being loaded as a texture)
extern INTERFACE_API IMAGEHANDLE		InterfaceLoadImage( const char* szFilename, unsigned int uFlags );
extern INTERFACE_API IMAGEHANDLE		InterfaceLoadImageFromArchive( const char* szFilename, unsigned int uFlags, int nArchiveHandle );
extern INTERFACE_API IMAGEHANDLE		InterfaceLoadImageEx( const char* szFilename, int scaleToX, int scaleToY, unsigned int uFlags );
extern INTERFACE_API void				InterfaceGetImageSize( IMAGEHANDLE, int* pnW, int* pnH );
extern INTERFACE_API void				InterfaceDrawImage( int layer, IMAGEHANDLE, int X, int Y, int width, int height, unsigned int uFlags );
extern INTERFACE_API void				InterfaceReleaseImage( IMAGEHANDLE );


//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------
//**********           Secondary Functions			  ********
//	The following bits are various lil helper functions,    --
//  screen init stuff, and other miscellaneous bits n pieces -
//  You probably don't need to know about these unless	    --
//  you're using the lib for somethin a bit funky			--
//------------------------------------------------------------
//------------------------------------------------------------

extern INTERFACE_API int ChopTextWidth( char* pcString, int nWidth, int nFont );

/********************************************************
 *********	  General Utility Functions       ***********
 **													   **
 ********************************************************/
extern INTERFACE_API ulong InterfaceGetARGBForColour(ulong ulCol);
extern INTERFACE_API ushort		ReadZAtScreenCoord( int nX, int nY );
extern INTERFACE_API ushort		GetZDepthFromWorldCoord( MVECT* pxWorldCoord );
extern INTERFACE_API FLOAT	GetDifferenceBetweenAngles( FLOAT fAngle1, FLOAT fAngle2 );
extern INTERFACE_API float	GetViewRotationFromVector( const MVECT* pxVect );

/**********************************************
 ******  Interface Info Functions  ************
 **********************************************/
extern INTERFACE_API int InterfaceGetWidth( void );
extern INTERFACE_API int InterfaceGetHeight( void );

extern INTERFACE_API BOOL InterfaceIsFullscreen ( void );
extern INTERFACE_API BOOL InterfaceIsSmall ( void );
extern INTERFACE_API BOOL InterfaceDoesNeedChanging ( void );
extern INTERFACE_API void	InterfaceSetWindowHasChanged( BOOL bFlag );

extern INTERFACE_API int InterfaceGetWindowWidth( void );
extern INTERFACE_API int InterfaceGetWindowHeight( void );

extern INTERFACE_API void InterfaceSetWindowPosition( int nLeft, int nTop );
extern INTERFACE_API int InterfaceGetWindowPositionX( void );
extern INTERFACE_API int InterfaceGetWindowPositionY( void );
extern INTERFACE_API void InterfaceSetWindowSize( BOOL boFullScreen, int nWidth, int nHeight, BOOL boAdjust );
extern INTERFACE_API void InterfaceInitSmall( void );

extern INTERFACE_API void InterfaceGetInitialScreenSize( int*, int* );
extern INTERFACE_API void InterfaceSetInitialSmall( BOOL );

extern INTERFACE_API float	InterfaceGetFPS( void );
extern INTERFACE_API void	InterfaceSetMaximumFrontBufferWidth( int nMaxWidth );


extern INTERFACE_API void	InterfaceSetTextureAsCurrent( int nTextureHandle );
extern INTERFACE_API void	InterfaceSetTextureAsCurrentDirect( void* pTexture );

/**********************************************************
 ***** Secondary font stuff                         *******
 *****											    *******
 **********************************************************/

typedef struct
{
	float	u;
	float	v;
	float	w;
	float	h;

	float	originX;
	float	originY;
	float	advance;

} FONT_UVCHAR;

extern INTERFACE_API BOOL	InterfaceFontLookupChar( int nFontNum, char cChar, FONT_UVCHAR* pOut );
extern INTERFACE_API BOOL	InterfaceFontSetAsCurrentTexture( int nFontNum );
extern INTERFACE_API BOOL	InterfaceFontLoad( int nFontNum, const char* pcImageFileName, const char* pcLayoutFile, ulong ulFlags );
extern INTERFACE_API BOOL	InterfaceFontSetFixedOffsets( int nFontNum, int nPosOffsetX, int nPosOffsetY, int nOccupyWidthReduction, int nOccupyHeightReduction );
extern INTERFACE_API void	InterfaceFontFree( int nFontNum );

/**********************************************************
 ***** Fancy stuff			                        *******
 *****											    *******
 **********************************************************/

extern INTERFACE_API void	InterfaceSetDrawRegion( int nX, int nY, int nWidth, int nHeight );
extern INTERFACE_API int	InterfaceGetDrawRegionWidth( void );
extern INTERFACE_API int	InterfaceGetDrawRegionHeight( void );

typedef enum
{
	INTF_LINES_ALPHA = 0,
	INTF_TEXTURE_FILTERING,
	INTF_FULLSCREEN,
	INTF_NO_RENDER,
	INTF_TEXTURERECT_LIMIT,
	INTF_ANISOTROPIC,

} INTF_DRAW_PARAM;

extern INTERFACE_API void	InterfaceSetGlobalParam( INTF_DRAW_PARAM nParam, int nState );

/**********************************************************
 ***** Canvas						                *******
 *****  (never really pursued this yet)			    *******
 **********************************************************/

#ifdef __cplusplus
extern void		CanvasText( int nLayer, float fX, float fY, const char* szText, ulong ulCol, int nFont = 0, float fMaxWidth = 0.0f, float fScale = 1.0f );
#endif

/**********************************************************
 ***** Odds n ends					                *******
 *****											    *******
 **********************************************************/

extern INTERFACE_API BOOL		InterfaceWaitingForMessageBox( void );

enum
{
	MAX_ANISTROPY,
};

int		InterfaceGetDeviceCaps( int Type );

typedef void(*InterfaceDrawCallback)( void );

extern void	InterfaceSetDrawCallback( InterfaceDrawCallback fnOnDrawCallback );

//-----------------------------------------------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------
//-----------------------------------------------------
//-----   Legacy / Deprecated Stuff
//
//
// The things below should probably be changed or removed 
// ----------------------------------------------------


/**********************************************************
 ***** Interface Secondary Initialisation Functions *******
 *****  For those times when you want to get dirty  *******
 **********************************************************/
#ifdef WINUSERAPI	// Only include this if winuser.h has been previously included
extern INTERFACE_API HWND				InterfaceGetWindow( void );
extern INTERFACE_API void				InterfaceSetWindow( HWND hwndMain );
#endif

#ifdef DIRECT3D_VERSION		// Only include this if directX has been previously included
#if (DIRECT3D_VERSION>=0x0900)
typedef LPDIRECT3DDEVICE9		LPGRAPHICSDEVICE;
typedef LPDIRECT3D9				LPGRAPHICS;
typedef LPDIRECT3DTEXTURE9		LPGRAPHICSTEXTURE;
typedef IDirect3DSurface9		IGRAPHICSSURFACE;
#else
typedef LPDIRECT3DDEVICE8		LPGRAPHICSDEVICE;
typedef LPDIRECT3D8				LPGRAPHICS;
typedef LPDIRECT3DTEXTURE8		LPGRAPHICSTEXTURE;
#endif

extern INTERFACE_API LPGRAPHICSDEVICE	InterfaceInitD3D( BOOL boMinRenderPageSize );
extern INTERFACE_API LPGRAPHICSDEVICE	InterfaceGetD3DDevice( void );
extern INTERFACE_API LPGRAPHICS		InterfaceGetD3D( void );
#endif
//-----------------------------------------------------------------------------------------------------------------------------------

extern INTERFACE_API void	InterfaceSetVRMode( BOOL bVRModeActive );

/*******************************************************
 ************	   Jpeg  Functions     *****************
 **													  **
 **   For loading and displaying Jpeg image files     **
 *******************************************************/
extern INTERFACE_API void	ShowJpeg ( int nX, int nY, int nWidth, int nHeight, int nFlags );
extern INTERFACE_API void	LoadJpeg ( const char* szFilename );
extern INTERFACE_API void	FreeJpeg ( void );

extern INTERFACE_API void	ShowJpegCentered( int nX, int nY, int nWidth, int nHeight, int nFlags );
extern INTERFACE_API void	JpegGetDrawPoint( int* pnX, int *pnY );
extern INTERFACE_API void	ClearBufferedJpegs( void );


#ifndef SERVER
extern void		PanicImpl( const char* szErrorString );

#define		PANIC_IF(a,b)		if( a ) PanicImpl( b );
#endif


/********************************************************
 ************	   Options Functions     ****************
 **													   **
 ** These routines are used to set & get video options **
 ********************************************************/
extern INTERFACE_API int InterfaceGetOption( int );
extern INTERFACE_API void InterfaceSetOption( int, int );

extern INTERFACE_API BOOL InterfaceIsZBufferLockable( void );
extern INTERFACE_API void InterfaceSetZBufferLockable( BOOL );

extern INTERFACE_API void InterfaceSetMipMapBias( float );
extern INTERFACE_API void InterfaceSetFilteringModes( int );

/********************************************************
 ************** Other Functions  ************************
 **													   **
 ** Various old misc stuff that probably should be	   **
 **  avoided										   **
 ********************************************************/
extern INTERFACE_API void InterfaceTextureLoadError( int nRet, const char* szFilename );
extern INTERFACE_API int GetStringWidth( const char* pcString, int nFont );
extern INTERFACE_API int GetStringHeight( const char* pcString, int nFont );
extern INTERFACE_API void InterfaceEnableTextureFiltering( BOOL bFlag );

// These are values for the game vid-options dialog.
enum
{
	TEXTURE_FILTERING = 0,
	BACK_BUFFER,
	VSYNC,
	OLD_STARTUP,
	MINIMUM_SURFACE_RES,
	FOG_MODE,
	FSAA,
	MAX_OPTIONS,
};


#ifdef WINUSERAPI	// Only include this if winuser.h has been previously included
extern INTERFACE_API void InterfaceInitVidOptions( HINSTANCE hInst, HWND hDialogWind ); 
extern INTERFACE_API INT_PTR CALLBACK InterfaceVidOptionsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
#endif

typedef struct
{
	long	left;
    long    top;
    long    right;
    long	bottom;
} WINDOW_DIMENSIONS;

extern INTERFACE_API WINDOW_DIMENSIONS InterfaceGetWindowDimensions( void );

// Old version of the texture overlay sys that used explicit references to direct3d textures
#ifdef DIRECT3D_VERSION
extern INTERFACE_API int	CreateTexturedOverlay( LPDIRECT3DTEXTURE9 );
extern INTERFACE_API IDirect3DTexture9* InterfaceLoadTextureDX( const char* szFilename, int, int );
#endif


#ifdef OLD_FUNCTION_COMPAT
#define		AddFontString(a,b,c,d)					InterfaceText( 0,a,b,c,d, 0 )
#define		AddBox(a,b,c,d)							InterfaceOutlineBox( 0,a,b,c,d, 0xC0000000 )
#define		AddFontStringRight(a,b,c,d)				InterfaceTextRight(0,a,b,c,d,0)
#define		AddFontStringCenter(a,b,c,d,e)			InterfaceTextCenter(0,b,c,a,d,e,0)
#define		AddFontStringCenterLayer(a,b,c,d,e,f)	InterfaceTextCenter(f,b,c,a,d,e,0)
#define		AddFontStringLayer(a,b,c,d,e)			InterfaceText( e,a,b,c,d, 0 )
#endif

//-----------------------------------------------------------
enum // Preset colour enums for font draw
{
	COL_SYS_SPECIAL = 1,
	COL_PLAYER_ANNOUNCE,
	COL_TEAM,
	COL_PUBLIC,
	COL_PRIVATE,
	COL_SYSOP,
	COL_SERVER_MESSAGE,
	COL_SERVER_MESSAGE_2,
	COL_WARNING,
	COL_PLAYER_LABEL,
	COL_PLAYER_LIST,
	COL_FONT_BLACK,
	COL_COMMS_SPECIAL,
	COL_SYS_SPECIAL_FADED,
	COL_COMMS_SPECIAL_TRANS,
	COL_PLAYER_LIST_FADED,
	COL_DISABLED,
	COL_TRANSACTION,
	COL_WHITE,
	COL_SYSMESSAGE_GOOD,
	COL_CHAT,
	COL_CHAT2,
	COL_BARTER,
	COL_PUBLIC_DROP_SHADOW,
	COL_CHATGROUP_DROP_SHADOW,
	COL_SYSMSG_DROP_SHADOW,
	COL_FONT2,
	COL_F2_COMMS_SPECIAL,
	COL_F2_SYS_SPECIAL,
	COL_F2_WHITE,
	COL_F2_PUBLIC,
	COL_F2_BLACK,
	COL_F2_CARDSRED,
	COL_F2_SPARE2,
	COL_F2_SPARE3,
	COL_F2_SPARE4,
	COL_F2_SPARE5,
	COL_F2_SPARE6,
	COL_FONT3,
	COL_F3_COMMS_SPECIAL,
	COL_F3_SYS_SPECIAL,
	COL_F3_WHITE,
	COL_F3_PUBLIC,
	COL_F3_BLACK,
	COL_F3_TEAM,
	COL_F3_INFO1,
	COL_F3_INFO2,
	COL_F3_INFO3,
	COL_F3_WARNING,
	COL_F3_SPARE2,
	COL_F3_SPARE3,
	COL_FONT4,
	COL_F4_COMMS_SPECIAL,
	COL_F4_SYS_SPECIAL,
	COL_F4_WHITE,
	COL_F4_PUBLIC,
	COL_F4_BLACK,
	COL_F4_TEAM,
	COL_F4_INFO1,
	COL_F4_INFO2,
	COL_F4_INFO3,
	COL_F4_WARNING,
};
//-------------

#ifdef __cplusplus
}
#endif


/*
 **********************************************
 **                                          **
 ** You are welcome to distribute, use,      **
 ** abuse, tinker with, criticise, point &   **
 ** laugh at anything to do with this        **
 ** library. 
 **                                          **
 ** Feedback, comments, etc etc to..         **
 **   OldAntelopes on github				 **
 **                                          **
 **********************************************
*/
#endif
