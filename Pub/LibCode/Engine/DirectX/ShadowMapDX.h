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


#ifdef __cplusplus
}
#endif





#endif // ifndef SHADOW_MAP_DX_H
