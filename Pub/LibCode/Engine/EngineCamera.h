#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif


extern void		EngineCameraInit( void );



//------------------------------------------------------------------------------------------------
// ------------------ Not very nice.. this stuff is only supposed to be used by the platform-specific camera operations
// ------------------  but i can't be arsed to enforce that with a suitable inheritance structure yet..
extern ENGINEMATRIX			mEngineViewMatrix;

extern VECT				mxEngineCamPos;
extern VECT				mxEngineCamVect;
extern VECT				mxEngineCamUpVect;
//------------------------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif


#endif //#ifndef ENGINE_CAMERA_H