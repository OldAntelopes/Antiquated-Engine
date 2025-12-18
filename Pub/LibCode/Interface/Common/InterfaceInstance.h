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

class InterfaceInstance
{
public:
	InterfaceInstance();

	//-----------------------------------------------------------------------

	// Font
	void	Text( int nLayer, int nX, int nY, uint32 ulCol, int nFont, const char* szString, ...  );
	void	TextCentre( int nLayer, int nX1, int nY, uint32 ulCol, int nFont, const char* szString, ... );
	void	TextRight( int nLayer, int nX, int nY, uint32 ulCol, int nFont, const char* szString, ... );

	int		GetStringWidth( const char* pcString, int nFont );
	int		GetStringHeight( const char* pcString, int nFont );

	// Textures
	int			GetTexture( const char* szFilename, int nFlags );
	void		ReleaseTexture( int nTextureHandle );
	
	// Textured Overlays
	int			CreateNewTexturedOverlay( int nLayer, int nTextureHandle );
	void		TexturedRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, float fU, float fV, float fU2, float fV2 );

	// Overlays
	void	Rect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol);
	void	ShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol1, uint32 ulCol2,uint32 ulCol3, uint32 ulCol4 );
	void	Triangle( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3 );

	//------------------------------------------------------------------
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

	int			GetHeight( void );
	int			GetWidth( void );
	int			GetCentreX( void );
	int			GetCentreY( void );

	void		SetTextureAsCurrentDirect( void* pTexture );
	void		EnableTextureFiltering( BOOL bFlag );
	void		InitWindow( const char* pcString, void* pVoidWinClass, BOOL bAllowResize );
	int			GetWindowWidth();
	int			GetWindowHeight();
	void		SetInitialWindowSize( int sizeX, int sizeY );
	void		SetWindowSize( BOOL boFullScreen, int nWidth, int nHeight, BOOL boAdjust );

	const InterfaceRECT&		GetDrawDimensions() const { return( m_DrawRect );} 
	void		SetDrawRegion( int nX, int nY, int nWidth, int nHeight );
	void		SetRenderSurfaceSize( int W, int H ) { mnRenderSurfaceWidth = W; mnRenderSurfaceHeight = H; }

	// TODO - Make these private and expose all the functionality through this top level interface
	TexturedOverlays*		mpTexturedOverlays;
	Overlays*				mpOverlays;
	InterfaceInternalsDX*	mpInterfaceInternals;
	FontSystem*				mpFontSystem;

	void		DrawAllElements( void );
protected:
	void		SetDevice( void* pDevice );		// oldschool


	BOOL	mboInterfaceInitialised = FALSE;
	bool	mbIsInScene = false;
	HWND	mhWindow;

	InterfaceRECT		m_DrawRect;
	int		mnRenderSurfaceWidth = 900;
	int		mnRenderSurfaceHeight = 700;  
	BOOL	mboTextureFilteringCurrentState = FALSE;
	int		mnWindowWidth = 700;
	int		mnWindowHeight = 600;

};



#endif
