#ifndef TOOLS_MESH_MANIPULATE_H
#define TOOLS_MESH_MANIPULATE_H


extern void		ModelConvSeparateVerts( int nHandle );
extern void		ModelConvConvertYUpToZUp( int nHandle );
extern void		ModelConvFixInsideOutModel( int nHandle );

extern void		ModelConvDeleteIsolatedVertices( int nModelHandle );

extern void		ModelConvReverseFaceOrientation( int nHandle );

#endif