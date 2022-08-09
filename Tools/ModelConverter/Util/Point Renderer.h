#ifndef POINT_RENDERER_H
#define POINT_RENDERER_H

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef TOOL
#define MAX_POINTS		20000
#else
#define MAX_POINTS		4096
#endif



extern void InitialisePointRenderer( void );
extern void FreePointRenderer( void );

extern void RenderPoints( float, BOOL, BOOL );

extern void PointRenderAdd( MVECT* pxPoint1, ulong ulCol );



#ifdef __cplusplus
}
#endif



#endif
