#ifndef LINE_RENDERER_H
#define LINE_RENDERER_H

#ifdef __cplusplus
extern "C"
{
#endif


#define MAX_LINE_VERTICES		32768



extern void InitialiseLineRenderer( int nMaxVertices );
extern void FreeLineRenderer( void );

extern void RenderLines( BOOL, BOOL, BOOL , BOOL);

extern void AddLine( MVECT* pxPoint1, MVECT* pxPoint2, ulong ulCol, ulong ulCol2 );





#ifdef __cplusplus
}
#endif



#endif
