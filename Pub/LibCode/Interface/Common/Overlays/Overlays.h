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


extern void		RenderOverlays( int nLayer );
extern void		AddOverlays( void );

extern HRESULT	InitialiseOverlays( void );
extern void		FreeOverlays( void );

extern void		SetActiveControlType( int nControlType );
extern int		GetActiveControlType( void );


extern void DoOverlayControl( void );

extern void		LockOverlays( void );
extern void		UnlockOverlays( void );


#ifdef __cplusplus
}
#endif


#endif
