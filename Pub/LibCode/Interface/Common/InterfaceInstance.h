#ifndef INTERFACE_INSTANCE_H
#define INTERFACE_INSTANCE_H


class Overlays;
class TexturedOverlays;
class InterfaceInternalsDX;
class FontSystem;

struct InterfaceRECT
{
	int		x = 0;
	int		y = 0;
	int		w = 0;
	int		h = 0;
};

typedef	void(*fnInterfaceCallback)( InterfaceInstance* pxInterface );


class InterfaceInstance
{
public:
	InterfaceInstance();

	void	Shutdown();
	//-----------------------------------------------------------------------

	// Font
	void	Text( int nLayer, int nX, int nY, uint32 ulCol, int nFont, const char* szString, ...  );
	void	TextCentre( int nLayer, int nX1, int nY, uint32 ulCol, int nFont, const char* szString, ... );
	void	TextRight( int nLayer, int nX, int nY, uint32 ulCol, int nFont, const char* szString, ... );
	void	TextBox( int nLayer, int nX, int nY, int nWidth, uint32 ulCol, int nFont, int flags, const char* szString, ... );

	const char*	TextLimitWidth( int nLayer, int nX, int nY, int nMaxWidth, uint32 ulCol, int nFont, const char* szString, ...  );
	const char*	TextLimitWidthCentred( int nLayer, int nX, int nY, int nMaxWidth, uint32 ulCol, int nFont, const char* text, ... );

	void	SetFontFlags( int flags );

	int		GetStringWidth( const char* pcString, int nFont );
	int		GetStringHeight( const char* pcString, int nFont );

	// Textures
	int			LoadTextureAsync(const char* szFilename, int nFlags);
	BOOL		DidLoadFail( int nHandle );
	BOOL		HasFullyLoaded(int nHandle);
	int			GetTexture( const char* szFilename, int nFlags, BOOL bAsync = FALSE );
	void		ReleaseTexture( int nTextureHandle );
	int			LoadTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags );
	// Textured Overlays
	int			CreateNewTexturedOverlay( int nLayer, int nTextureHandle );
	void		TexturedRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, float fU, float fV, float fU2, float fV2 );
	int			CreateNewTexturedOverlayForEngineTexture( int nLayer, int nTextureHandle );

	// Overlays
	void	OutlineBox( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol);
	void	Rect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol);
	void	ShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol1, uint32 ulCol2,uint32 ulCol3, uint32 ulCol4 );
	void	Triangle( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3 );
	void	Line( int nLayer, int nX, int nY, int x2, int y2, uint32 ulCol);

	//------------------------------------------------------------------
	void		InitD3D( HWND hWindow, BOOL bMinBackBufferSize, int nBackBufferMinW = 0, int nBackBufferMinH = 0 );
	void		SetIsLinkedToEngine( BOOL bFlag ) { mbIsLinkedToEngine = bFlag; }
	void		InitialiseInstance( BOOL bUseDefaultFonts );

	int			NewFrame( uint32 ulCol );
	void		BeginRender( void);
	void		Draw( void );
	void		EndRender( void);
	void		Present( void );

	BOOL		IsInRender( void );
	void		FreeAll( void );
	void		ChangeDisplay( BOOL bFullScreen, int nMonitorNum );
	void		ReleaseForDeviceReset( void );
	void		RestorePostDeviceReset( void );

	int			GetHeight( void );
	int			GetWidth( void );
	int			GetCentreX( void );
	int			GetCentreY( void );
	BOOL		IsFullscreen() { return(mboFullScreen); }	

	void		SetTextureAsCurrentDirect( void* pTexture );
	void		EnableTextureFiltering( BOOL bFlag );
	void		InitWindow( const char* pcString, void* pVoidWinClass, BOOL bAllowResize );
	int			GetWindowWidth();
	int			GetWindowHeight();
	int			GetWindowX();
	int			GetWindowY();
	void		SetWindowPosition(int nLeft, int nTop);	
	void		SetRenderBufferSize( int width, int height );
	void		SetInitialWindowSize( int sizeX, int sizeY );
	void		SetWindowSize( BOOL boFullScreen, int nWidth, int nHeight, BOOL boAdjust );
	void		SetViewport( int X, int Y, int nWidth, int nHeight );
	void		SetRenderCanvas();
	void		SetBorderlessFullscreenMode( BOOL bFlag ) { mboBorderlessFullscreen = bFlag; }
	BOOL		IsBorderlessFullscreenMode() { return(mboBorderlessFullscreen); }
	void		CopyRenderCanvasToBackBuffer( int X, int Y, int W, int H );

	const InterfaceRECT&		GetDrawDimensions() const { return( m_DrawRect );} 
	void		SetDrawRegion( int nX, int nY, int nWidth, int nHeight );
	void		SetRenderSurfaceSize( int W, int H ) { mnRenderSurfaceWidth = W; mnRenderSurfaceHeight = H; }
	void		SetInitialSize( BOOL bFullscreen, int width, int height, BOOL bSmallMode = FALSE );
	BOOL		LoadFont( int nFontNum, const char* pcImageFileName, const char* pcLayoutFile, uint32 ulFlags );

	// TODO - Make these private and expose all the functionality through this top level interface
	TexturedOverlays*		mpTexturedOverlays;
	Overlays*				mpOverlays;
	InterfaceInternalsDX*	mpInterfaceInternals;
	FontSystem*				mpFontSystem;

	void		DrawAllElements( void );
	float		GetFPS() { return( mfInterfaceFPS ); }
	BOOL		HasWindowChanged() { return( mboHasWindowChanged );}
	BOOL		WasFullscreen() { return( mboCurrentlyFullscreen ); }
	void		RegisterDeviceResetCallbacks(fnInterfaceCallback fnPreReset, fnInterfaceCallback fnPostReset) { mfnPreDeviceResetCallback = fnPreReset; mfnPostDeviceResetCallback = fnPostReset; }
	void*		GetRawTexture( int nHandle );

protected:
	void		SetDevice( void* pDevice );		// oldschool
	void		CreateD3DInstanceIfNeeded();

	void		UpdateWindowStyle( HWND hWindow, BOOL boFullScreen );

	fnInterfaceCallback	mfnPreDeviceResetCallback = NULL;
	fnInterfaceCallback	mfnPostDeviceResetCallback = NULL;

	BOOL	mboInterfaceInitialised = FALSE;
	bool	mbIsInScene = false;
	HWND	mhWindow;
	int		mnAdapterUsedForDevice = 0;
	BOOL	mboBorderlessFullscreen = FALSE;
	BOOL	mbIsLinkedToEngine = FALSE;

	InterfaceRECT		m_DrawRect;
	int		mnRenderSurfaceWidth = 900;
	int		mnRenderSurfaceHeight = 700;  
	BOOL	mboTextureFilteringCurrentState = FALSE;
	int		mnWindowWidth = 960;
	int		mnWindowHeight = 540;
	int		mnWindowLeft = 100;
	int		mnWindowTop = 100;
	float	mfInterfaceFPS = 0.0f;
	BOOL	mboFullScreen = FALSE;
	BOOL	mboCurrentlyFullscreen = FALSE;
	BOOL	mboHasWindowChanged = FALSE;
	BOOL	mboMinPageSize = TRUE;
	int		mnMinBackBufferSizeW = 0;
	int		mnMinBackBufferSizeH = 0;

	int		mnRequestedMonitorNum = NOTFOUND;
	DWORD		mdwWindowStyle = 0;


};



#endif
