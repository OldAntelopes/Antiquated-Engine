#ifndef ENGINE_CAMERA_DX_H
#define ENGINE_CAMERA_DX_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif
	

extern void		EngineCameraInitDX( void );

extern void	EngineCameraSetEyeOffset( BOOL bRight );

// The camera stuff is externed in Engine.h
extern void		EngineCameraSetViewMatrix( ENGINEMATRIX* pMat );

#ifdef __cplusplus
}
#endif


#endif //#ifndef ENGINE_CAMERA_H