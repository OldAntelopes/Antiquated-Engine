
#ifndef TEXTURED_OVERLAYS_H
#define TEXTURED_OVERLAYS_H

// TODO - this shouldnt be here
#include "../../DirectX/InterfaceInternalsDX.h"
#include "Overlays.h"

#define		MAX_TEX_OVERLAY_VERTICES			2048

#define		MAX_DIFFERENT_TEXTURED_OVERLAYS		128
#define		MAX_RECTS							12000

/*** This structure is used to specify a shape in the Textured Overlays module ***/
typedef struct
{
	BYTE	nType;
	BYTE	bPad;
	short	 nX;
	short	 nY;
	short	nWidth;
	short	nHeight;
	short	nX2;
	short	nY2;

	uint32	ulCol;
	float	fU1;
	float	fU2;
	union {	float	fU3;	float fRot; };
	float	fV1;
	float	fV2;
	union { float	fV3;	uint32 ulCol2; };

	void*	pNext;

} TEXTURED_RECT_DEF;
 

#define		MAX_INTERNAL_TEXTURES_LOADED		256


typedef struct
{
	LPGRAPHICSTEXTURE		pTexture;
	char					acFilename[128];
	uint32					ulLastTouched;
	int						nRefCount;

} INTERNAL_TEXTURES;

typedef struct
{
	int						nLayerNum;
	INTF_RENDER_TYPES		nRenderType;
	LPGRAPHICSTEXTURE		pTexture;
	TEXTURED_RECT_DEF*		pxRectsInOverlay;
	int						hEngineTexture;

} OVERLAY_DATA;



/*** These enum lists define settings for the Textured Overlays module **/
enum
{
	TEX_OVLY_NONE = 0,
	TEX_OVLY_RECT,
	TEX_OVLY_TRI,
	TEX_OVLY_SPRITE,
};

class TexturedOverlays 
{
public:
	void		Initialise();
	void		Shutdown();

	void		Reset();
	
	void		Render( int nLayer );

	void		SetLimit( int nState );

	int			CreateOverlay( int nLayer, TEXTURE_HANDLE hTexture );
	int			CreateOverlayDirect( int nLayer, void* pTexture, int hEngineTexture );


	void		SetRenderType( int nOverlayNum, INTF_RENDER_TYPES nRenderType );
	void		SetTextureAsCurrent( int nTextureHandle );

	void		AddTri( int nOverlayNum, int* pnVerts, float* pfUVs, uint32 ulCol );
	void		AddRect( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, float fU, float fV, float fUWidth, float fUHeight );
	void		AddRectShaded( int nOverlayNum, int nX, int nY, int nWidth, int nHeight, uint32 ulCol, uint32 ulCol2, float fU, float fV, float fUWidth, float fUHeight );
	void		AddSprite( int nOverlayNum, int nX, int nY, float fTexGrid, int nTexGridNum, uint32 ulCol, float fRotAngle, float fScale );
	
	int			GetTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags );
	int			GetTextureInternal( const char* szFilename, int nFlags, int nArchiveHandle );
	int			CreateBlankTexture( int nWidth, int nHeight, int Mode);
	void		ReleaseTexture( int nTextureHandle );

	BYTE*		LockTexture( int nTextureHandle, int* pnPitch, int* pnFormat, int nFlags );
	void		UnlockTexture( int nTextureHandle );
	int			FindTexture( const char* szFilename );

	int			GetTextureSize( int nTextureHandle, int* pnW, int* pnH );
	void		ExportTexture( int nTextureHandle, const char* szFilename, int nMode );

private:
	TEXTURED_RECT_DEF*		GetNextRect( int nOverlayNum );
	void					AddTexturedTriVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef );
	void					AddTexturedRectVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef );
	void					AddSpriteVertices( FLATVERTEX** ppVertices, TEXTURED_RECT_DEF* pxRectDef );
	void					FlushTexOverlayBuffer( void );
	void					FreeTextRectLists( void );
	void					InitTextRectLists( void );
	int						GetNewInternalTextureHandle( void );

	BOOL		mboInterfaceTexturesFirstInitialise = TRUE;

	INTERNAL_TEXTURES	maxInternalTextures[MAX_INTERNAL_TEXTURES_LOADED];
	OVERLAY_DATA		maxOverlayData[MAX_DIFFERENT_TEXTURED_OVERLAYS];

	TEXTURED_RECT_DEF*		mpxRectBuffer = NULL;
	TEXTURED_RECT_DEF*		mpxNextFreeRect = NULL;

	int				mnTexRectBufferSize = 0;
	int				mnTexRectMaxBufferSize = MAX_RECTS;

#ifdef USING_OPENGL
	VertexBuffer*		mpCurrentTexOverlayVertexBuffer; // Buffer to hold vertices
	VertexBuffer*		mpTexOverlayVertexBuffer1; // Buffer to hold vertices
	VertexBuffer*		mpTexOverlayVertexBuffer2; // Buffer to hold vertices

#else
	IGRAPHICSVERTEXBUFFER*		mpCurrentTexOverlayVertexBuffer; // Buffer to hold vertices
	IGRAPHICSVERTEXBUFFER*		mpTexOverlayVertexBuffer1; // Buffer to hold vertices
	IGRAPHICSVERTEXBUFFER*		mpTexOverlayVertexBuffer2; // Buffer to hold vertices
#endif

	int			mnNextTexOverlayVertex = 0;
	int			mnNumActiveTexOverlays = 0;
	int			mnCurrentTexOverlayRenderTexture = 0;
	int		mnCurrentRenderType = 0;

};

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif

	
extern void InitTexturedOverlays( void );
extern void FreeTexturedOverlays( void );

extern void ResetTexturedOverlays( void );


extern void DrawTexturedOverlays( int nLayer );
extern void	InterfaceTexturedOverlaysSetLimit( int nState );

extern int	TexturedOverlayCreate( int nLayer, TEXTURE_HANDLE hTexture );

extern int	TexturedOverlayCreateDirect( int nLayer, void* pTexture, int hEngineTexture );


#ifdef __cplusplus
}
#endif

#endif


