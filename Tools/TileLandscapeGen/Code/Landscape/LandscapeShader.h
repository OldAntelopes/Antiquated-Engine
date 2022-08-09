
#ifndef LANDSCAPE_SHADER_H
#define LANDSCAPE_SHADER_H

typedef struct
{
	int		nMapX;
	int		nMapY;
	int		nResolution;

} RENDER_TILE_ITEM;



extern void		LandscapeShaderInitShaders( void );
extern void		LandscapeShaderFreeShaders( void );

extern void		LandscapeShaderSetSourceTexture( int nInputNum, int hTexture  );
extern void		LandscapeShaderSetOthorgonalView( int width, int height );

extern void		LandscapeShaderDrawSingleTile( RENDER_TILE_ITEM* pxRenderList, int nDrawX1, int nDrawY1, int nDrawX2, int nDrawY2, float fSourceTextureScale );

extern BOOL		LandscapeShaderBeginRender( void );
extern void		LandscapeShaderCleanup( void );

extern void		LandscapeShaderSetRangeConstants( int* anHeightValues );
extern int*		LandscapeShaderGetRangeConstants( void );

#endif