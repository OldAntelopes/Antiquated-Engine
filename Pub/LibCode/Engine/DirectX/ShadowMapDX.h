#ifndef SHADOW_MAP_DX_H
#define SHADOW_MAP_DX_H



#ifdef __cplusplus
extern "C"
{
#endif

extern void		EngineShadowMapInit( void );
extern void		EngineShadowMapFree( void );

extern void		EngineShadowMapInitDepthRenderPass( const VECT* pxCamTarget );
extern void		EngineShadowMapInitDepthRenderPassAutoTarget( float fAutoTargetDist );
extern void		EngineShadowMapEndDepthRenderPass( void );

extern int		EngineShadowMapGetDepthMap( void );
extern float		EngineShadowMapGetFarClipDist( void );


extern void		EngineShadowMapLoadShaders( void );
extern void		EngineShadowMapReleaseShaders( void );

extern void		EngineShadowMapGetLightViewProjMatrix( ENGINEMATRIX* pxMatrix );

extern void		EngineShadowMapSetWorldMatrix( const ENGINEMATRIX*	pxWorldMatrix );
extern int		EngineShadowMapGetSize( void );
extern float	EngineShadowMapGetStrength( void );

extern void		EngineShadowMapSetSizeOption( int nSizeOption );

// For receiver shaders' gShadowCascadeInfo: x = cascades, y = near scale, z = 1 / atlas width, w = cascade blend band
extern void		EngineShadowMapGetCascadeInfo( float* pafOut4 );

#ifdef __cplusplus
}
#endif





#endif // ifndef SHADOW_MAP_DX_H
