#ifndef NODDY_BROWSER_H
#define NODDY_BROWSER_H

#ifdef __cplusplus		//---------------------- C++ Interfaces


typedef struct
{
	int	nMinX;
	int	nMaxX;
	int	nMinY;
	int	nMaxY;
} BOUNDS;

typedef	void(*BrowserPageChangeCallback)( char* );

//------------------------------------------------------------------
// Universal Browser
class	NoddyBrowser
{
friend class NoddyBrowserSection;
friend class HTMLParser;
public:
	NoddyBrowser();
	static NoddyBrowser&	Get();

	void	LoadPageFromMem( char* pbMem, int nMemSize );
	void	LoadPage( char*, BOOL bLocalFileOnly = FALSE );
	void	DisplayPage( int X, int Y, int Width, int Height, int nGlobalAlpha, int nFlags );
	void	CleanupPage( void );
	void	Close( void );

	// Interface to the mouse buttons - need to be called by the external code
	BOOL	LeftMouseUp( int X, int Y );
	BOOL	LeftMouseDown( int X, int Y );
	BOOL	OnMouseWheelMove( float fOffset );

	char*	GetRoot( void ) { return( m_szPageRoot ); }
	void	SetActiveLink( char* szLink, BOUNDS* pBounds );
	char*	GetActiveLink( void ) { return( m_szActiveLink ); }
	BOOL	IsMouseOver( BOUNDS* pBounds );
	int		GetScrollTop( void ) { return m_PageScrollY; }
	int		GetDisplayTop( void ) { return m_PageDisplayY; }
	int		GetPageWidth( void ) { return (m_PageDisplayWidth); }
	int		GetDisplayBottom( void ) { return m_PageDisplayY + m_PageDisplayHeight; }
	void	SetPageChangeCallback( BrowserPageChangeCallback fnFunc ) { m_fnBrowserPageChangeCallback = fnFunc; }
	int		GetPageLoadState( void ) { return m_PageLoadState; }
	void	SetPageFullHeight( int height ) { m_PageHeight = height; }

	void	CacheFile( char* szTempFilename );

	void	SetDownloadReturnVal( int nVal ) { m_nPageDownloadRetVal = nVal; }
	BOOL	IsDownloading( void );
protected:
	static char*	GetNextElement( char* pRunner, char* pcOutBuffer, BOOL* pbIsTag );
	char*	GetElementBuffer( void ) { return( m_pcElementBuffer ); }

private:
	char*	m_pcLoadedPage;
	char*	m_pcElementBuffer;
	
	void	SetSectionRegion( int X, int Y, int Width, int Height );
	void	DisplaySection( char* pRunner );
	bool	DisplayScrollbar( int X, int Y, int Width, int Height, int GlobalAlpha );

	char*	FindBody( char* pRunner );

	uint32	ulBackgroundCol;
	uint32	ulGlobalAlpha;

	char	m_szLastReadFile[256];
	char	m_szCachedRoot[256];
	BOOL	m_bCacheValid;
	int		m_nPageDownloadRetVal;
	char	m_szPageName[256];
	char	m_szPageRoot[256];
	char	m_szActiveLink[256];
	int		m_PageLoadState;
	BOUNDS	m_ActiveLinkBounds;
	BOUNDS	m_PageBounds;
	static BOOL	mboBodyLess;
	BrowserPageChangeCallback		m_fnBrowserPageChangeCallback;
	int		m_PageHeight;
	int		m_PageDisplayHeight;
	int		m_PageDisplayWidth;
	int		m_PageScrollY;
	int		m_PageDisplayY;
	int		m_ScrollbarDragOriginal;
	int		m_ScrollbarOriginalY;
	int		m_ScrollbarHover;
	int		m_ScrollbarDown;
	int		m_nScrollMaxY;
	float	m_fScrollPixelsPerLine;
};

extern "C"
{
extern BOOL		NoddyBrowserOnMouseWheelMove( float fOffset );
}

#else		// #ifdef __cplusplus		//---------------------- C Interfaces


extern BOOL		NoddyBrowserOnMouseWheelMove( float fOffset );


#endif	///		#ifdef __cplusplus


#endif