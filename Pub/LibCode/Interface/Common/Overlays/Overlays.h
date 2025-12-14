#ifndef OVERLAYS_H
#define OVERLAYS_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif

	
#define		NUM_OVERLAY_VERTICES	(8*2048) 

#ifndef USING_OPENGL
#define D3DFVF_FLATVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#endif

typedef struct
{
    float x;
	float y; 
    float z; 
    unsigned long color; 
    float tu;
	float tv;

} FLATVERTEX; 

#define		LINE_VERTEX_BUFFER_SIZE			65536
#define		MAX_NUM_OVERLAY_LAYERS			4

typedef struct
{
	IGRAPHICSVERTEXBUFFER*		mpxVertexBuffer;
	int							mnNextOverlayVertex;
	BOOL						mboBufferIsLocked;
	FLATVERTEX*					mpOverlayVertices;

} OVERLAY_VERTEX_BUFFER_CONTAINER;


class Overlays : public InterfaceModule
{
public:
	void	Initialise( void );
	void	Shutdown();

	void	Render( int nLayer );
	void	EnableAdditive( BOOL bFlag );

	//--------------------------
	void	OutlineBox( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol );
	void	ShadedBox( int nLayer, int nX, int nY, int nWidth, int nHeight, int nStyle );
	void	Triangle( int nLayer, int nX1, int nY1, int nX2, int nY2, int nX3, int nY3, uint32 ulCol1, uint32 ulCol2, uint32 ulCol3 );
	void	Rect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol);
	void	ShadedRect( int nLayer, int nX, int nY, int nWidth, int nHeight, uint32 ulCol1, uint32 ulCol2,uint32 ulCol3, uint32 ulCol4 );
	void	Line( int nLayer, int nX1, int nY1, int nX2, int nY2, uint32 ulCol1, uint32 ulCol2 );


	//--------------------------
	void	LockOverlays( void );
	void	UnlockOverlays( void );

private:
	void	RenderLinesBuffer( void );

	OVERLAY_VERTEX_BUFFER_CONTAINER		maOverlayVertexBuffer[ MAX_NUM_OVERLAY_LAYERS ];

	FLATVERTEX*	mpIconVertices = NULL;

	IGRAPHICSVERTEXBUFFER*		mpxOverlaysLineVertexBuffer = NULL;
	FLATVERTEX*	mpLineVertices = NULL;
	int			mnNextLineVertex = 0;
	BOOL	mbAdditiveOverlays = FALSE;
};

extern void		RenderOverlays( int nLayer );

extern HRESULT	InitialiseOverlays( void );
extern void		FreeOverlays( void );

extern void		SetActiveControlType( int nControlType );
extern int		GetActiveControlType( void );


extern void		LockOverlays( void );
extern void		UnlockOverlays( void );


#ifdef __cplusplus
}
#endif


#endif
