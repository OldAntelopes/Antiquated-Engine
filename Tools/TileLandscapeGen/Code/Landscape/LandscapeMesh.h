#ifndef LANDSCAPE_MESH_H
#define LANDSCAPE_MESH_H


extern void		LandscapeRenderGenerateFullLandscapeMeshDX( void );

extern BOOL		LandscapeMeshRayTest( const VECT* pxRayStart, const VECT* pxRayDir, VECT* pxIntersect );

extern void		LandscapeRenderGenerateBlocks( void );



#endif