#ifndef TOOLS_RECALC_NORMALS_H
#define TOOLS_RECALC_NORMALS_H

enum eNormalFixMode
{
	FLAT_FACES,
	SMOOTHED_NORMALS,
	ALL_NORMALS_UP,
};

extern void		ModelConvFixNormals( int nHandle, eNormalFixMode nMode );
extern void		ModelConvSetNormals( int nHandle, eNormalFixMode nMode );
extern void		ModelConvReverseNormals( int nHandle );


#endif