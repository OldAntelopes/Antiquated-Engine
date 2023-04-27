#ifndef MODEL_RENDERING_GL_H
#define	MODEL_RENDERING_GL_H

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif

typedef struct 
{
    FLOAT Position;
    FLOAT Boundary;
    FLOAT Normal;
    FLOAT Diffuse;
    FLOAT Specular;
    FLOAT Tex[8];
} SIMPLIFICATION_PARAMS;

extern int		EngineSimplifyMesh( int nModelHandle, float fReduceAmount, SIMPLIFICATION_PARAMS* pxParams );

extern int		ModelRenderImplGL( int nModelHandle, const VECT* pxPos, const VECT* pxRot, uint32 ulRenderFlags );

extern void	ModelRenderSetMaterialRenderStates( MODEL_RENDER_DATA* pxModelData );

extern void	ModelRenderingPlatformInit( void );
extern void	ModelRenderingPlatformFree( void );

#ifdef __cplusplus
}
#endif


#endif
