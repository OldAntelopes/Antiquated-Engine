
#ifndef SHADOW_VOLUME_DX_H
#define SHADOW_VOLUME_DX_H

#ifdef __cplusplus
extern "C"
{
#endif

extern void		ShadowVolumeRenderingEnable( BOOL, float fCutoffDistance );
extern BOOL		ShadowVolumeRenderIsEnabled( void );

extern int		ShadowVolumeInitDX( void );
extern void		ShadowVolumeRenderModel( int nModelHandle, const VECT* pxPos, const VECT* pxRot, unsigned long ulRenderFlags, unsigned int uID );
extern void		ShadowVolumeSetLightPosition( VECT* pxPos );
extern void		ShadowVolumeSetShadowStrength( float fStrength );
extern void		ShadowVolumeSetProjection( float fFOV, float fNear, float fFar );

extern void		ShadowVolumeUpdate( void );
extern void		ShadowVolumeRender( void );

extern void		ShadowVolumeFree( void );
extern void		ToggleShadowVolumes( void );

extern int		m_sVolumesRenderedThisFrame;
extern int		m_sVolumesGeneratedThisFrame;


#ifdef __cplusplus
}
#endif


#endif // #ifndef SHADOW_VOLUME_DX_H
