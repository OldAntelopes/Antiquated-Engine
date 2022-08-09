#ifndef VERTEX_MANIPULATE_H
#define VERTEX_MANIPULATE_H

class CSceneObject;

enum
{
	VERTEX_MANIPULATE_MOVE,
	VERTEX_MANIPULATE_ROTATE,
	VERTEX_MANIPULATE_SCALE,
};


extern void		VertexManipulateMouseDownStore( CSceneObject* pSceneObject );

extern void		VertexManipulateMouseMoveUpdate( CSceneObject* pSceneObject, float fScreenDeltaX, float fScreenDeltaY );

extern void		VertexManipulateSetControlMode( int mode );

extern void		VertexManipulateMouseUp( CSceneObject* pSceneObject );

#endif