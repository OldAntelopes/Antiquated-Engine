#ifndef ENGINE_OCULUS_DX_H
#define ENGINE_OCULUS_DX_H


#ifdef __cplusplus
extern "C"
{
#endif



extern void InitializeOculusVR( void );

extern void ShutdownOculusVR( void );

extern void*	OculusInitWindow( void* );

extern void*	OculusInitD3DDevice( void* pD3D );

extern void		OculusStartSceneLeftEye( ulong ulClearCol );
extern void		OculusStartSceneRightEye( ulong ulClearCol );
extern void		OculusStartEndScene( void );

extern void		OculusReleaseGraphics( void );


extern BOOL		OculusGetHeadOrientation( VECT* pxDir, VECT* pxCamUp );
extern BOOL		OculusGetEulerAngles( float* pfYaw, float* pfPitch, float* pfRoll );
extern BOOL		OculusApplyHeadOrientation( VECT* pxDir, VECT* pxUp );
extern BOOL		OculusGetQuaternion( float* pfQuaternion );

#ifdef __cplusplus
}
#endif



#endif
