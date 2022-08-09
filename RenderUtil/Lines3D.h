#ifndef GAMECOMMON_RENDERUTIL_LINES3D_H
#define GAMECOMMON_RENDERUTIL_LINES3D_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif


typedef struct
{
	BOOL boFog;
	BOOL boAdditive;
	BOOL boZWrite;

} LINES3D_RENDER_PARAMS;

extern void Lines3DInitialise( int nMaxVertices );

extern void Lines3DShutdown( void );

extern void	Lines3DAddLine( const VECT* pxPoint1, const VECT* pxPoint2, ulong ulCol, ulong ulCol2 );

extern void Lines3DFlush( LINES3D_RENDER_PARAMS* pxRenderParams );



#ifdef __cplusplus
}
#endif


#endif
