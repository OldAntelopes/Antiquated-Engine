#ifndef FONT_COMMON_H
#define FONT_COMMON_H

#include "../../DirectX/InterfaceTypesDX.h"

#define		MAX_FONTS_IN_GAME		8
#define		SIZE_OF_FONT_VERTEX_BUFFER		1020

typedef struct
{
	float	x1;	
	float	x2;	
	float	y1;	
	float	y2;	

} FLOATRECT;

// One glyph of a TrueType font (InterfaceFontLoadTrueType)
typedef struct
{
	int		nX0;			// Bitmap's offset from the pen, in baked pixels
	int		nY0;			// Bitmap's offset from the baseline, in baked pixels (negative is up)
	int		nWidth;			// Bitmap size in baked pixels (0 for a space)
	int		nHeight;
	int		nAdvance;		// Whole 2D units, the same at every scale
	int		nGlyphIndex;	// In the font file, 0 if it hasn't got one
	BOOL	bFromFallback;	// Drawn from the fallback bitmap font instead, from this rect of its image:
	int		nFallbackX;
	int		nFallbackY;
	int		nFallbackW;
	int		nFallbackH;

} FONT_TTGLYPH;

//------------------------------------------------------------------------------------------------------
class CFontDef
{
public:
	CFontDef()
	{
		mbIsTrueType = FALSE;
		mpbTrueTypeData = NULL;
		mpTrueTypeInfo = NULL;
		mfTrueTypeScale = 1.0f;
		mfTrueTypeBakedScaleX = 0.0f;
		mfTrueTypeBakedScaleY = 0.0f;
		mnTrueTypeCellHeight = 0;
		mfTrueTypeBaseline = 0.0f;
		mpbFallbackCoverage = NULL;
		mnFallbackWidth = 0;
		mnFallbackHeight = 0;
		m_TextureSizeX = 0;
		m_TextureSizeY = 0;
		mhTexture = NOTFOUND;
#ifdef TUD9
		mpTexture = NULL;
#endif
		m_bFilteringOn = TRUE;
		m_bIsFixedWidth = FALSE;
		m_bIsBottomAligned = FALSE;
		m_szTextureFilename[0] = 0;
		mnFontOccupyWidthReduction = 0;
		mnFontOccupyHeightReduction = 0;
		mnFontOverallSize = 32;
		mnFontDrawOffsetX = 0;
		mnFontDrawOffsetY = 0;
	}
	~CFontDef()
	{
		FreeTexture( NULL );
		FreeTrueType();
	}
	void 	Initialise( const char* szDefName, int nGlobalVerticalOffset );

	void	SetTextureFileName( const char* szTextureFilename )
	{
		strcpy( m_szTextureFilename, szTextureFilename );
	}
	void 	LoadTexture( InterfaceInstance* pInterfaceInstance );


	void	LookupChar( BYTE cChar, FONT_UVCHAR* pOut )
	{
		*pOut = maFontLookup[cChar];
	}
	int		GetTextureSizeX( void ) { return m_TextureSizeX; }
	int		GetTextureSizeY( void ) { return m_TextureSizeY; }

	void	FreeTexture( InterfaceInstance* pInterface );
	void	SetTextureAsCurrent( InterfaceInstance* );

	void	EnableFiltering( BOOL bEnable ) { m_bFilteringOn = bEnable; }
	void	SetFixedWidth( BOOL bEnable, int nWidth ) { m_bIsFixedWidth = bEnable; }
	BOOL	IsFilteringOn( void ) const { return( m_bFilteringOn ); }
	BOOL	IsFixedWidth( void ) const { return( m_bIsFixedWidth ); }
	BOOL	IsBottomAligned( void ) const { return( m_bIsBottomAligned ); }
	int		GetGlobalVerticalOffset( void ) const { return( m_GlobalVerticalOffset );}

	// TrueType fonts (FontTrueType.cpp)
	BOOL	InitialiseTrueType( InterfaceInstance* pInterfaceInstance, const char* szTTFFilename, const INTERFACE_TRUETYPE_PARAMS* pxParams );
	BOOL	BakeTrueType( InterfaceInstance* pInterfaceInstance, float fScaleX, float fScaleY );
	BOOL	IsTrueType( void ) const { return( mbIsTrueType ); }
	BOOL	TrueTypeNeedsBake( float fScaleX, float fScaleY ) const { return( ( fScaleX != mfTrueTypeBakedScaleX ) || ( fScaleY != mfTrueTypeBakedScaleY ) ); }
	const FONT_TTGLYPH*	GetTrueTypeGlyph( BYTE cChar ) const { return( &maTrueTypeGlyphs[cChar] ); }
	int		GetTrueTypeCellHeight( void ) const { return( mnTrueTypeCellHeight ); }
	float	GetTrueTypeBaseline( void ) const { return( mfTrueTypeBaseline ); }
	float	GetTrueTypeBakedScaleX( void ) const { return( mfTrueTypeBakedScaleX ); }
	float	GetTrueTypeBakedScaleY( void ) const { return( mfTrueTypeBakedScaleY ); }
	BOOL	GetTrueTypeInkExtent( const char* szString, float* pfAbove, float* pfBelow );

	int			mnFontOccupyWidthReduction;
	int			mnFontOccupyHeightReduction;
	int			mnFontOverallSize;
	int			mnFontDrawOffsetX;
	int			mnFontDrawOffsetY;

private:
	void			ReadFontrastFile( const char* szFontDefinitionFilename );
	void			ReadJSONLayoutFile( const char* szFontDefinitionFilename );
	void			FreeTrueType( void );
	BOOL			CreateTextureFromAlpha( InterfaceInstance* pInterfaceInstance, const byte* pbAlpha, int nWidth, int nHeight );
	byte*			LoadCoverageImage( InterfaceInstance* pInterfaceInstance, const char* szFilename, int* pnWidth, int* pnHeight );
	void			InitialiseTrueTypeFallback( InterfaceInstance* pInterfaceInstance, const char* szImage, const char* szLayout );

	BOOL			mbIsTrueType;
	byte*			mpbTrueTypeData;			// The .ttf file, kept for re-baking
	void*			mpTrueTypeInfo;				// stbtt_fontinfo
	float			mfTrueTypeScale;			// Font units to 2D units
	float			mfTrueTypeBakedScaleX;		// Render pixels per 2D unit the texture was made for
	float			mfTrueTypeBakedScaleY;
	int				mnTrueTypeCellHeight;
	float			mfTrueTypeBaseline;
	FONT_TTGLYPH	maTrueTypeGlyphs[256];
	byte*			mpbFallbackCoverage;		// The fallback bitmap font's image as coverage, kept for re-baking
	int				mnFallbackWidth;
	int				mnFallbackHeight;

	FONT_UVCHAR		maFontLookup[256];
	int			mhTexture; 
#ifdef TUD9
	LPGRAPHICSTEXTURE	 mpTexture;
#endif
	int			m_TextureSizeX;
	int			m_TextureSizeY;
	char		m_szTextureFilename[256];
	BOOL		m_bFilteringOn;
	BOOL		m_bIsFixedWidth;
	BOOL		m_bIsBottomAligned;
	int			m_GlobalVerticalOffset = 0;

};



#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

enum
{
	FONT_TYPE_1 = 0,
	FONT_TYPE_2,
	FONT_TYPE_3,
	FONT_TYPE_4,
	MAX_FONT_TYPES,
};

enum
{
	ALIGN_LEFT = 0,
	ALIGN_CENTRE,
	ALIGN_RIGHT,
	ALIGN_SCALED,
};

#define MAX_CHARS_IN_STRING			256
#define MAX_STRINGS_IN_BUFFER		4096

typedef struct 
{
	int		nX;
	int		nY;
	uint32	nCol;
	float	fScale;

	char	acString[ MAX_CHARS_IN_STRING ];

	RECT	xAlignRect;
	
	short	nAlign;
	short	nLayer;
	short	wFlag;
	BYTE	bFont;
	BYTE	bPad1;
	
} TEXT_BUFFER;





extern void AddFontStringAlign( char* szString, int, int, RECT* );

extern float GetFontU( BYTE cChar, int nFont );
extern float GetFontV( BYTE cChar, int nFont );
extern float GetFontUWidth( BYTE cChar, int nFont );

extern BOOL	InterfaceFontIsFixedWidth( int nFontNum );
extern BOOL	InterfaceFontIsFilteringOn( int nFontNum );


extern HRESULT InitialiseFont( BOOL bUseDefaultFonts );

extern void InitialiseFontBuffers( void );

extern int GetFontTextureHeight( int nFont );

extern int					mnCurrentFontFlags;
extern TEXT_BUFFER			maxTextBuffer[];
extern FLATVERTEX*			mpFontVertices;
extern int		mnPosInTextBuffer;

extern int FontDrawText( char* pcString, RECT* pxAlignRect, int nAlign, uint32 ulCol, int nFont, int nFlag, float fTextScale );
extern void	FontDefFreeAll( void );

#ifdef __cplusplus
}
#endif


class FontSystem : public InterfaceModule
{
public:
	void	Shutdown();

	BOOL	LoadFont( int nFontNum, const char* pcImageFileName, const char* pcLayoutFile, uint32 ulFlags, int nGlobalVerticalOffset = 0 );
	BOOL	LoadTrueTypeFont( int nFontNum, const char* szTTFFilename, const INTERFACE_TRUETYPE_PARAMS* pxParams );
	BOOL	IsFontTrueType( int nFontNum );
	BOOL	GetTrueTypeInkExtent( int nFontNum, const char* szString, float* pfAbove, float* pfBelow );
	void	InitialiseFontBuffers( void );
	HRESULT	InitialiseFonts( BOOL bUseDefaultFonts );
	
	void	Text( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int nFont );
	void	TextCentre( int nLayer, int nX1, int nX2, int nY, const char* szString, uint32 ulCol, int nFont );
	void	TextRight( int nLayer, int nX, int nY, const char* szString, uint32 ulCol, int nFont );
	const char*	TextLimitWidth( int nLayer, int nX1, int nY, const char* szString, uint32 ulCol, int nFont, int nMaxWidth );
	const char*	TextLimitWidthCentred( int nLayer, int nX1, int nY, const char* szString, uint32 ulCol, int nFont, int nMaxWidth );
	
	void	SetFontFlags( int flags );

	void	RenderStrings( int nLayer );
	void	ClearStrings( void );
	void	FreeResources( BOOL bFreeEverything );

	BOOL	IsFontFixedWidth( int nFontNum );
	BOOL	IsFontFilteringOn( int nFontNum );
	BOOL	FontSetAsCurrentTexture( int nFontNum );
	BOOL	FontLookupChar( int nFontNum, char cChar, FONT_UVCHAR* pOut );
	BOOL	FontSetFixedOffsets( int nFontNum, int nPosOffsetX, int nPosOffsetY, int nOccupyWidthReduction, int nOccupyHeightReduction );
	void	FreeFont( int nFontNum );
	BOOL	IsFontLoaded( int nFontNum );
	int		ChopTextWidth( char* pcString, int nWidth, int nFont );
	int		GetStringWidth( const char* pcString, int nFont );
	int		GetStringHeight( const char* pcString, int nFont );
	BOOL	GetStringGlyphExtent( const char* pcString, int nFont, float* pfTop, float* pfBottom );
	int		TextBoxMaxHeight(int nLayer, int nX, int nY, const char* szString, int ulCol, int font, int nMaxWidth, int nMaxHeight, BOOL bLeftAlign);
	
	virtual void		ReleasePreGraphicsDeviceReset();
	virtual void		RestorePostGraphicsDeviceReset();

private:
	void	InitialiseFontBuffersDX( void );

	void	StringRenderBegin( void );

	void	AddFontString( int nX, int nY, const char* szString, uint32 ulCol, int nFlag );
	void	AddFontStringCenter( int nY, int nX1, int nX2, const char* szString, uint32 ulCol );

	int		FontDrawText( char* pcString, RECT* pxAlignRect, int nAlign, uint32 ulCol, int nFont, int nFlag, float fTextScale );
	void	AddCharVertices( FLOATRECT* pxScrRect, FLOATRECT* pxTexRect, uint32 ulCol, int nFlag );
	float	FontDrawChar( BYTE cChar, int nX, int nY, uint32 ulCol, int nFont, int nFlag, float fScaleOverride );
	float	FontGetCharUWidthAndScreenWidth( char cChar, int nFont, int nFlags, float* pfUWidth, int* pnScrWidth, float *pfAdvance );
	float	GetFontAdvance( BYTE cChar, int nFont );
	int		GetFontTextureWidth( int nFont );
	int		GetFontTextureHeight( int nFont );
	float	GetFontVHeight( BYTE cChar, int nFont );
	float	GetFontV( BYTE cChar, int nFont );
	float	GetFontU( BYTE cChar, int nFont );
	float	GetFontUWidth( BYTE cChar, int nFont );

	void	DrawFontBufferDX( void );
	void	FontDefFreeAll( void );

	// TrueType fonts (FontTrueType.cpp)
	float	GetFontCellHeight( BYTE cChar, int nFont );
	float	GetFontFlagScaling( int nFlags );
	float	FontGetTrueTypeCharWidth( BYTE cChar, int nFont, int nFlags, float* pfUWidth, int* pnScrWidth, float *pfAdvance );
	float	FontDrawTrueTypeChar( BYTE cChar, int nX, int nY, uint32 ulCol, int nFont, int nFlag );
	void	GetTrueTypeRenderScale( float* pfScaleX, float* pfScaleY );
	void	UpdateTrueTypeFonts( void );

	int							mnCurrentRenderFont = 0;

	IGRAPHICSVERTEXBUFFER*		mpxFontVertexBuffer1 = NULL;
	IGRAPHICSVERTEXBUFFER*		mpxFontVertexBuffer2 = NULL;
	IGRAPHICSVERTEXBUFFER*		mpxCurrentFontVertexBuffer = NULL;

	CFontDef*	mpFontDefs[MAX_FONTS_IN_GAME] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

	FLATVERTEX*			mpFontVertices = NULL;
	int					mnFontVertexIndex = 0;
};

#endif