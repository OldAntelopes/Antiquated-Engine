#ifndef TRACTOR_ENGINE_H		// haha  (no.. really.. hahahahha )
#define TRACTOR_ENGINE_H

//------------------------------------
//   Engine.h
//
// The intention for this module is to provide a generic layer of graphics 'engine' functions
// that can be used for all the graphical stuff the game does, but without
//  referring directly to DirectX functions. (Theoretically making conversion to other 
//	rendering engines easier - In a theoretical imaginary world. )
//
//  Closely related to this is "Rendering.h", which has all the includes specific
//  to the ATM model format and associated display calls
//-----------------------------------

#include "EngineMaths.h"
#include "CodeUtil.h"


// ---------------------------------------------------- Engine - Main C interfaces
#ifdef __cplusplus

class InterfaceInstance;

extern "C"
{
#endif

//----------------------------------------------------------- Enums
//  Values used in the EngineSetBlendMode, EngineSetColourMode functions etc.
//---------------------------------------------------------------------
enum
{
	BLEND_MODE_ALPHABLEND = 0,
	BLEND_MODE_SRCALPHA_ADDITIVE,
	BLEND_MODE_ALPHA_SUBTRACTIVE,
	BLEND_MODE_COLOUR_SUBTRACTIVE,
	BLEND_MODE_COLOUR_ADDITIVE,
	BLEND_MODE_COLOUR_BLEND,
	BLEND_MODE_COLOUR_INVALPHA,
	BLEND_MODE_INVCOLOUR_SUBTRACTIVE,
	BLEND_MODE_COLOUR_SUBTRACTIVE_ALPHA,
	BLEND_MODE_COLOUR_BOTHALPHA,
	BLEND_MODE_DESTINV,
	BLEND_MODE_DESTADD,
	BLEND_MODE_MASK,
	BLEND_MODE_INVERSE_MASK,
	BLEND_MODE_COLOUR_SRCDEST,
	BLEND_MODE_COLOUR_ALPHA,
	// ....
};

enum
{
	COLOUR_MODE_TEXTURE_MODULATE = 0,
	COLOUR_MODE_DIFFUSE_ONLY,
	COLOUR_MODE_TEXTURE_MODULATE_NO_ALPHA_TEXTURE,
	COLOUR_MODE_TEXTURE_ONLY,
	COLOUR_MODE_DIFFUSE_ALPHA_TEXTURE,
	COLOUR_MODE_TEXTURE_DIFFUSE_ALPHA,
};

enum
{
	VERTEX_FORMAT_NORMAL = 0,
	VERTEX_FORMAT_2UVS,
	VERTEX_FORMAT_XYZ,
	VERTEX_FORMAT_SHADOWVERTEX,
	VERTEX_FORMAT_FLATVERTEX,
};

enum eSurfaceFormat
{
	SURFACEFORMAT_UNKNOWN,
	SURFACEFORMAT_A8R8G8B8,
	SURFACEFORMAT_X8R8G8B8,
	SURFACEFORMAT_D24S8,
	SURFACEFORMAT_DXT1,
	SURFACEFORMAT_DXT3,
	SURFACEFORMAT_DXT5,
	SURFACEFORMAT_A1R5G5B5,
};

typedef enum
{
	OUTSIDE,
	INSIDE,
	INTERSECTING,
} CULL_RESULT;


enum		// Second param of EngineLoadTexture
{
	NORMAL = 0,
	NO_DDS_CACHE = 0x1,
	NO_MIPMAPPING = 0x2,
	POINT_FILTER = 0x4,
	NO_CHROMAKEY = 0x8,
	REMOVE_ALPHA = 0x10,
	FORCE_A8R8G8B8 = 0x20,
};
//------------------------------------------------------------------


typedef struct
{
	BOOL	bDoCacheImagesToDDS;
} ENGINE_OPTIONS;


//------------------------------------------------------------------
//  Engine functions
//------------------------------------------------------------------
extern	void	EngineInit( void );
extern  void	EngineInitFromInterface(void);

extern	void	EngineUpdate( BOOL nNewFrame );
extern  void	EngineFinaliseRender( void );
extern  void	EngineRestart( void );
extern	void	EngineFree( BOOL bFreeForShutdown );
extern	void	EngineSetOptions( ENGINE_OPTIONS* );
extern void		EngineReloadShaders( void );
extern	BOOL	EngineIsInitialised( void );

extern	void	EngineDefaultState( void );
// Opt-in: makes EngineDefaultState also set the alpha-test compare function (a D3DCMP_* value, e.g. D3DCMP_GREATEREQUAL).
// EngineInitDX sets GREATEREQUAL once, but a device Reset puts it back to the D3D default (ALWAYS). Off (NOTFOUND)
// by default, so EngineDefaultState behaves as before unless the app asks.
extern	void	EngineSetDefaultStateAlphaFunc( int nCompareFunc );
// Opt-in: animated models (keyframe blending and skeletal skinning) write their pose into the vertex buffer once per
// frame, and later passes in the same frame (shadow map, reflection, second VR eye, shadow volumes) reuse it instead of
// redoing it. Keyframe clones share a buffer, so a clone drawn in between still makes the next one re-blend.
// Off by default (every draw animates, as before).
extern	void	EngineSetAnimateOncePerFrame( BOOL bFlag );
// Opt-in: draw the height maps a model's materials carry (ATMFormat.md, chunk 0x1F), as parallax and surface lighting. Needs
// the app's ATMMaterial pixel shader to have the relief path. Off by default, so such models draw plain. fDepthScale scales
// each material's depth, fMaxSteps is the most parallax steps per pixel, fNormalStrength how much it tilts the lighting
// (0 = parallax only), and it fades out between fFadeStart and fFadeEnd world units from the camera.
extern	void	EngineSetModelRelief( BOOL bEnable, float fDepthScale, float fMaxSteps, float fNormalStrength, float fFadeStart, float fFadeEnd );
// State channels (ATMFormat.md, chunk 0x20): a model's material can be tagged with a channel, 1 to MODEL_MATERIAL_NUM_CHANNELS, and a
// lit colour, multiplied by fStrength while fStrength is above 0. It's for lamps that follow what their owner is doing, like a
// vehicle's brake lamps. The strengths apply to whatever is drawn while they're set, so clones sharing a model's materials can
// differ: set them just before drawing the model, and clear them straight after. All 0 by default, so nothing is lit.
#define		MODEL_MATERIAL_NUM_CHANNELS		8
extern	void	EngineSetModelMaterialChannel( int nChannel, float fStrength );
// The same, but lit in this colour for this draw instead of the one in the model. For lamps that do two jobs, like brake and reverse
extern	void	EngineSetModelMaterialChannelColour( int nChannel, float fStrength, float fRed, float fGreen, float fBlue );
extern	void	EngineClearModelMaterialChannels( void );
// How many separate lamps the model has on this channel, with the middle and facing of up to nMax of them in model space. Faces touching
// at a corner are one lamp. Reads the mesh, so call it once per model, not per frame.
extern	int		ModelGetMaterialChannelLamps( int nModelHandle, int nChannel, VECT* pxPositions, VECT* pxNormals, int nMax );
// The model's triangles in model space: returns how many, with *ppxVerts pointing at three corners each. They're the mesh's own copy in
// ordinary memory (the one collision uses), read from the mesh on first use, so call it once per model, not per frame. Valid until the
// model is freed. 0 (and NULL) if there's no mesh or it couldn't be read.
extern	int		ModelGetTriangles( int nModelHandle, const VECT** ppxVerts );
// Running totals of keyframe blends done and skipped (by the above). Compare two readings for a per-frame figure.
extern	void	EngineGetModelAnimationStats( int* pnBlendsDone, int* pnBlendsSkipped );
// Opt-in: the DX9 model renderer saves the stage-0 texture with GetTexture (which AddRefs) on every model draw and
// never released it, leaking a reference per draw and keeping textures alive after they're freed. TRUE releases the
// previous saved reference each time, so only one is held. Off by default, because it changes texture lifetimes.
extern	void	EngineSetModelRenderReleasesSavedTexture( BOOL bFlag );

extern	void	EngineSetBlendMode( int nBlendMode );
extern	void	EngineSetColourMode( int nTexLayer, int nBlendMode );
extern void		EngineResetColourMode( void );

extern	void	EngineSetVertexFormat( int nVertexFormat );
extern	void	EngineSetTextureFiltering( int nMode );

extern	void	EngineEnableBlend( int nFlag );
extern	void	EngineEnableFog( int nFlag );
extern	void	EngineEnableLighting( int nFlag );
extern	void	EngineEnableZTest( int nFlag );
extern	void	EngineEnableZWrite( int nFlag );
extern	void	EngineEnableCulling( int nMode );
extern	void	EngineEnableAlphaTest( int nFlag );
extern	void	EngineEnableSpecular( int nFlag );
extern	void	EngineEnableTextureAddressClamp( int nFlag );
extern	void	EngineEnableWireframe( int nFlag );
extern  void	EngineEnablePointFill( int nFlag );
extern  void	EngineSetShadeMode( int nFlag );
extern	void	EngineEnableAnisotropicFiltering( BOOL bFlag );
extern BOOL		EngineCanUseAnisotropic( void );

extern void		EngineSetViewport( int X, int Y, int W, int H );
extern void		EngineRestoreViewport( void );

extern	void	EngineSetZBias( int Value );
extern	void	EngineSetFog( uint32 ulFogCol, float fFogStart, float fFogEnd );
extern	void	EngineSetFogColour( uint32 uARGB );
// Opt-in fog extras for shaders that declare them (all zeros = off, the default): pafHeight4 = (base height, falloff per
// unit, density per unit, 1 = on) -> gFogHeight; pafSunColour4 = (r, g, b, strength) -> gFogSunColour; pafSunDir4 =
// (x, y, z towards the sun, 0) -> gFogSunDir. Passed on by the same code that sets gFogStart/gFogEnd. NULL keeps a value.
extern	void	EngineSetFogExtras( const float* pafHeight4, const float* pafSunColour4, const float* pafSunDir4 );
// Opt-in: fog that takes the colour of the sky rendered behind each pixel. The app copies the frame after drawing the
// sky to a texture on sampler 12 and passes the main camera's view-projection (-> gFogSkyViewProj) and pafInfo4 =
// (1 = on, horizon v in the copy, 0, 0) (-> gFogSkyInfo). NULL matrix or info = off (the default).
extern	void	EngineSetFogSky( const ENGINEMATRIX* pxViewProj, const float* pafInfo4 );

extern  void	EngineSetPointRenderSize( float fPointSize );
extern void		EngineClearZBuffer( void );
extern void		EngineSetMaterialColourSource( BOOL );
extern void		EngineSetMaterialBlendOverride( BOOL );
extern BOOL		EngineGetMaterialBlendOverride( void );

extern uint32	EngineGetColValue( int R, int G, int B, int A );
extern void		EngineSetMipBias( float );

//-----------------------------------------------------------------------------
//  .fx / Shader interface
//-----------------------------------------------------------------------------

typedef	void(*fnDrawFunc)( int pass, void* pParamObject );


extern int		EngineShaderEffectLoad(const char* szFilename);
extern int		EngineShaderEffectLoadFromFileInMem(const char* szFilename, BYTE* pbMem, int nMemSize );
extern void		EngineShaderEffectApply( int handle, fnDrawFunc drawFunc, void* pParamObject );
extern void		EngineShaderEffectClear( int handle );
extern void		EngineShaderEffectReload(int handle, const char* szEffectFilename );
extern void		EngineShaderEffectSetConstantFloat(int handle, const char* szContanstName, float fVal);
extern void		EngineShaderEffectSetConstantBool(int handle, const char* szConstantName, bool bFlag );
extern void		EngineShaderEffectSetConstantInt(int handle, const char* szContanstName, int val );
extern void		EngineShaderEffectSetConstantFloatArray(int handle, const char* szContanstName, float* pfVals, int nNumFloats);
// TOOD - matrices, vectors, textures etc..
extern void		EngineShaderEffectFree( int handle );

//-----------------------------------------------------------------------------
//  Scene shadows
//-----------------------------------------------------------------------------
extern void		EngineSceneShadowMapActivate( BOOL bFlag );
extern BOOL		EngineSceneShadowMapIsActive( void );

// EngineSceneShadowsSetRenderParams
//  Adjustable settings for the shadow map system
//  If object shadows are being culled, lightcamtargetdist needs increasing (at expense of quality)
//  near and far planes must be set appropriately and as tightly as poss
//  fFOVMod can be used to tweak resolution
extern void		EngineSceneShadowsSetRenderParams( float fLightCamTargetDist, float fLightNearPlane, float fLightFarPlane, float fFOVMod );
extern void		EngineSceneShadowsInitShadowPass( const VECT* pxCamTarget );
extern void		EngineSceneShadowsInitShadowPassAutoTarget( float fTargetDist );
extern void		EngineSceneShadowsEndShadowPass( void );
// The light's view-projection (orthographic) for the current shadow pass, valid after EngineSceneShadowsInitShadowPass.
// Lets the app cull shadow casters against the light's volume rather than the camera's.
extern void		EngineSceneShadowsGetLightViewProjection( ENGINEMATRIX* pxMatrixOut );
// Opt-in (default off): snap the shadow map's centre to whole shadow-map texels in the light's view, so shadow edges
// don't crawl and shimmer as the camera moves. Takes effect from the next EngineSceneShadowsInitShadowPass.
extern void		EngineSceneShadowsSetTexelSnap( BOOL bFlag );
// Opt-in (default 1, as before): 2 cascades puts a second, fNearScale times smaller view of the same light and centre
// beside the usual one (a 2:1 atlas), for sharper shadows close to the centre. Draw the casters once per cascade, calling
// EngineSceneShadowsBeginCascade( n ) before each, between InitShadowPass and EndShadowPass. Receiver shaders need the
// gShadowCascadeInfo constant to use it (EngineShadowMapGetCascadeInfo; see The Universal's Shadows.psh).
extern void		EngineSceneShadowsSetCascades( int nCascades, float fNearScale );
extern int		EngineSceneShadowsGetCascades( void );
extern void		EngineSceneShadowsBeginCascade( int nCascade );
// Width of the band where receivers mix the two cascades, as a fraction of the near view's half-width (default 0.2;
// 0 = a hard border). Passed to receiver shaders in gShadowCascadeInfo.w.
extern void		EngineSceneShadowsSetCascadeBlend( float fBand );
extern void		EngineShadowMapActivateTexture( BOOL bFlag );
extern BOOL		EngineShadowMapTexturesAreActive( void );

extern void		EngineSceneShadowsStartRender( BOOL bHasDiffuseMap, BOOL bHasNormalMap, BOOL bHasSpecularMap );
extern void		EngineSceneShadowsEndRender( void );

extern void		EngineRestoreMainShaderState( BOOL bShouldRenderShadows );

extern void		EngineSceneShadowsSetShaderEpsilon( float fVal );
extern float	EngineSceneShadowsGetShaderEpsilon( void );

//-----------------------------------------------------------------------------
//  Oculus / VR
//-----------------------------------------------------------------------------
extern void		EngineEnableVR( BOOL bEnable );
extern BOOL		EngineIsVRMode( void );
extern BOOL		EngineHasOculus( void );

//-----------------------------------------------------------------------------
//  Texture Manager stuff
//-----------------------------------------------------------------------------
#ifndef TEXTURE_HANDLE
#define		TEXTURE_HANDLE		int
#endif

#ifdef __cplusplus
extern TEXTURE_HANDLE	EngineLoadTexture( const char*, int nLoadMode, int* pnErrorFlag = NULL );
extern TEXTURE_HANDLE	EngineLoadTextureFromArchive( const char*, int nLoadMode, int nArchiveHandle, int* pnErrorFlag = NULL );
#else
extern TEXTURE_HANDLE	EngineLoadTexture( const char*, int nLoadMode, int* pnErrorFlag );
extern TEXTURE_HANDLE	EngineLoadTextureFromArchive( const char*, int nLoadMode, int nArchiveHandle, int* pnErrorFlag );
#endif

extern BOOL				EngineTextureIsFullyLoaded( TEXTURE_HANDLE );
extern BOOL				EngineTextureDidLoadFail( TEXTURE_HANDLE );
extern int				EngineTextureGetWidth( TEXTURE_HANDLE nTexHandle );
extern int				EngineTextureGetHeight( TEXTURE_HANDLE nTexHandle );
extern void				EngineTextureSetBestFiltering( int nChannel, int nMaxAnisotropy );

extern TEXTURE_HANDLE	EngineLoadTextureFromFileInMem( byte* pbMem, int nMipMode, int* pnErrorFlag, int nMemSize, const char* szOptionalFilename );
extern TEXTURE_HANDLE	EngineLoadTextureFromMem( byte* pbMem, int nMemSize, int width, int height, int format, int nMipMode, int* pnErrorFlag );
// Opt-in: on the D3D9Ex device D3DPOOL_MANAGED is invalid, so EngineLoadTextureFromFileInMem (and textures embedded
// in .atm models) fail. TRUE makes it load into D3DPOOL_DEFAULT instead. Off by default, so existing apps see no change.
extern void				EngineSetLoadTextureFromFileInMemUsesDefaultPool( BOOL bFlag );
// Opt-in: EngineCreateTexture's formats 0 and 1 ask for D3DPOOL_MANAGED, which the D3D9Ex device rejects (it returns 0).
// TRUE creates every format in D3DPOOL_DEFAULT with D3DUSAGE_DYNAMIC instead. Off by default, so existing apps see no change.
extern void				EngineSetCreateTextureUsesDefaultPool( BOOL bFlag );
// Returns 0 (not NOTFOUND) if the texture couldn't be created.
extern TEXTURE_HANDLE	EngineCreateTexture( int nWidth, int nHeight, int format );
extern BYTE*			EngineLockTexture( TEXTURE_HANDLE handle, int* pnPitch, BOOL bClear );
extern void				EngineUnlockTexture( TEXTURE_HANDLE handle );
extern void				EngineReleaseTexture( TEXTURE_HANDLE* );

extern void			EngineSetTexture( int nTex, TEXTURE_HANDLE nTexHandle );
extern void			EngineSetTextureNoDebugOverride( int nTex, TEXTURE_HANDLE nTexHandle );
extern void			EngineSetTextureEx( int nTex, TEXTURE_HANDLE nTexHandle, int nFlags );
extern void			EngineSetShadowMultitexture( BOOL bFlag );


extern void			EngineExportTexture( TEXTURE_HANDLE, const char* szFilename, int nMode );
extern void			EngineProcessTexture( TEXTURE_HANDLE, int nProcessMode );
extern void			EngineCopyTexture( TEXTURE_HANDLE hSrcTexture, TEXTURE_HANDLE hDestTexture );
extern void			EngineCopyTextureToPosition( TEXTURE_HANDLE hSrcTexture, TEXTURE_HANDLE hDestTexture, int nDestPosX, int nDestPosY );

extern TEXTURE_HANDLE	EngineCreateRenderTargetTexture( int nWidth, int nHeight, int mode, const char* szTrackingName );
extern void				EngineSetRenderTargetTexture( TEXTURE_HANDLE handle, uint32 ulClearCol, BOOL bClear );
extern void				EngineRestoreRenderTarget( void );

extern int				EngineTextureCreateInterfaceOverlayEx( InterfaceInstance* pInterface, int nLayer, TEXTURE_HANDLE);
extern int				EngineTextureCreateInterfaceOverlay( int nLayer, TEXTURE_HANDLE );
extern TEXTURE_HANDLE	ModelRenderGeneratePreviewImage( int nModelHandle, VECT* pxPos, VECT* pxRot, float fScale );

extern BOOL			EngineIsLoadingTextures( void );

//-------------------------------------------------------------
// Camera
//-------------------------------------------------
extern void		EngineCameraSetPos( float fX, float fY, float fZ );
extern void		EngineCameraSetPosVect( const VECT* );
extern VECT*	EngineCameraGetPos( void );
extern void		EngineCameraSetDirection( float fX, float fY, float fZ );
extern void		EngineCameraSetDirectionVect( const VECT* );
extern VECT*	EngineCameraGetDirection( void );
extern void		EngineCameraSetProjection( float fFOV, float fNearClip, float fFarClip );
extern void		EngineCameraGetCurrentProjectionSettings( float* pfFOV, float* pfNearClip, float* pfFarClip );
extern void		EngineCameraSetProjectionOrtho( void );
extern void		EngineCameraSetOthorgonalView( int width, int height );


extern void		EngineCameraSetUpVect( float fX, float fY, float fZ );
extern VECT*	EngineCameraGetUpVect( void );

extern void		EngineCameraStoreCurrent( );
extern void		EngineCameraRestore( );

extern void		EngineCameraUpdate( void );

extern void		EngineCameraSetMatrix( ENGINEMATRIX* pMat );
extern void		EngineCameraGetMatrix( ENGINEMATRIX* pMat );

extern void		EngineSetOrthogonalCameraView(int nWidth, int nHeight);
extern void		EngineSetWorldMatrix( const ENGINEMATRIX* pMat );
extern void		EngineSetViewMatrix( const ENGINEMATRIX* pMat );
extern void		EngineSetProjectionMatrix( const ENGINEMATRIX* pMat );

extern void		EngineGetWorldMatrix( ENGINEMATRIX* pMat );
extern void		EngineGetProjectionMatrix( ENGINEMATRIX* pMat );
extern void		EngineGetViewMatrix( ENGINEMATRIX* pMat );

extern void		EngineGetRayForScreenCoord( int nScreenX, int nScreenY, VECT* pxRayStart, VECT* pxRayDir );
extern void		EngineGetScreenCoordForWorldCoord( const VECT* pPos, int* pX, int* pY );
extern void		EngineGetScreenCoordForWorldCoordWithWorldMatrix( const VECT* pPos, int* pX, int* pY );

extern void		EngineCameraSetViewAspectOverride( float fAspect );

extern void		EngineActivateClippingPlane( BOOL bFlag, float fZHeight );
extern void		EngineSetTextureReductionMode( int nMode );

extern void		EngineDumpState();
//-------------------------------------------------------------
// Culling
//-------------------------------------------------
CULL_RESULT	EngineIsBoundingSphereInView( const VECT* pxOrigin, float fRadius );

//-------------------------------------------------------------
// Lighting
//-------------------------------------------------
typedef struct
{
    float r;
    float g;
    float b;
    float a;
} ENGINE_COLOUR;

typedef enum
{
    POINT_LIGHT          = 1,
    SPOT_LIGHT           = 2,
    DIRECTIONAL_LIGHT    = 3,

} ENGINELIGHT_TYPE;

typedef struct
{
    ENGINELIGHT_TYPE    Type;            /* Type of light source */
    ENGINE_COLOUR   Diffuse;         /* Diffuse color of light */
    ENGINE_COLOUR   Specular;        /* Specular color of light */
    ENGINE_COLOUR   Ambient;         /* Ambient color of light */
    VECT			Position;         /* Position in world space */
    VECT			Direction;        /* Direction in world space */
    float           Range;            /* Cutoff range */
    float           Falloff;          /* Falloff */
    float           Attenuation0;     /* Constant attenuation */
    float           Attenuation1;     /* Linear attenuation */
    float           Attenuation2;     /* Quadratic attenuation */
    float           Theta;            /* Inner angle of spotlight cone */
    float           Phi;              /* Outer angle of spotlight cone */

} ENGINE_LIGHT;

extern void		EngineActivateLight( int lightNum, ENGINE_LIGHT* pLight );
extern void		EngineActivateSimpleLight( int lightNum, int mode, unsigned int ulCol, VECT* pxVect );
extern void		EngineDeactivateLight( int lightNum );
// Opt-in, off by default: lights keep one slot each in what shaders are given, and deactivating really removes them (EngineDX.cpp)
extern void		EngineSetLightSlotTracking( BOOL bFlag );
extern void		EngineGetPrimaryLight( ENGINE_LIGHT* pxOut );
extern void		EngineResetLighting( void );

//--------------------------------------------------------
// --------- Collisions
//---------------------------------------------------------
extern BOOL		EngineCheckCollision( int nHandle1, VECT* pxPos1, VECT* pxRot, int nHandle2, VECT* pxPos2, VECT* pxRot2, int nFlags );

extern BOOL		EngineModelSphereCollision( int nModelHandle, const VECT* pxModelPos, const VECT* pxModelRot, const VECT* pxSpherePos, float fRadius, int nFlags );
extern BOOL		EngineModelRayTest( int nModelHandle, const VECT* pxModelPos, const VECT* pxModelRot, const VECT* pxRayStart, const VECT* pxRayEnd, int flags );
extern BOOL		EngineModelOBBCollision( int nModelHandle, const VECT* pxModelPos, const VECT* pxModelRot, const VECT* pxBoxCentre, const VECT* pxBoxBounds, float fBoxZRot );

extern BOOL		EngineCollisionOBBIsOverlapping( VECT* pxPos, VECT* pxExtents, float fZRot, VECT* pxPos2, VECT* pxExtents2, float fZRot2 );
extern BOOL		EngineCollisionModelSphereSweepTest( int nMovingModel, VECT* pxPos1, VECT* pxPos2, int nStaticModel, VECT* pxStaticPos, VECT* pxRot, int nFlags );
extern BOOL		EngineCollisionBoxBoundProbe( const VECT* pxBoxMin, const VECT* pxBoxMax, const VECT* pxRayStart, const VECT* pxRayDir ); 
extern BOOL		EngineCollisionRayBoundBox( const VECT* pxBoxMin, const VECT* pxBoxMax, const VECT* pxRayStart, const VECT* pxRayLength ); 
extern BOOL		EngineCollisionBoxSphere( VECT* pxSpherePos, float fSphereRadius, VECT* pxBoundMin, VECT* pxBoundMax );
extern BOOL		EngineCollisionSphereSweepTest( float fMovingSphereRadius, VECT* pxPos1, VECT* pxPos2, int nStaticModel, VECT* pxStaticPos, VECT* pxRot, int nFlags );
extern BOOL		EngineCollisionLineSphere( const VECT* pxLinePos1, const VECT* pxLinePos2, const VECT* pxSpherePos, float fRadius, int nFlags );
extern BOOL		EngineCollisionRayPlane( VECT* pxPlanePos1, VECT* pxPlanePos2, VECT* pxPlanePos3, VECT* pxRayStart, VECT* pxRayDir  );
extern BOOL		EngineCollisionRayIntersectTri( const VECT* pTri1, const VECT* pTri2, const VECT* pTri3, const VECT* pRayStart, const VECT* pRayDir, float* pfU, float* pfV, float* pfDist );

extern VECT*	EngineCollisionResultNormal( void );
extern VECT*	EngineCollisionResultHitPoint( void );
extern float	EngineCollisionResultDistance( void );

//--------------------------------------------------------
//---------- Vertex Buffers
//---------------------------------------------------------
typedef int		VERTEX_BUFFER_HANDLE;

typedef struct 
{
    VECT	position;
    VECT	normal; 
    unsigned long   color;
    float	tu;
	float	tv;

	VECT		tangent;

} ENGINEBUFFERVERTEX;

typedef enum
{
	TRIANGLE_LIST		= 0,
	POINT_LIST			= 1,
	LINE_LIST			= 2,

} ENGINEPRIMITIVE_TYPE;


extern VERTEX_BUFFER_HANDLE		EngineCreateVertexBuffer( int nMaxVertices, int nFlags, const char* szTrackingName );
extern VERTEX_BUFFER_HANDLE		EngineCreateVertexBufferQuad( int width, int height, uint32 ulCol, const char* szTrackingName );

extern BOOL		EngineVertexBufferRender( VERTEX_BUFFER_HANDLE, ENGINEPRIMITIVE_TYPE nPrimType );
extern void		EngineVertexBufferReset( VERTEX_BUFFER_HANDLE );
extern void		EngineVertexBufferFree( VERTEX_BUFFER_HANDLE );

// Copying vertex buffers
extern BOOL		EngineVertexBufferCopy(  VERTEX_BUFFER_HANDLE hDestination,  VERTEX_BUFFER_HANDLE hSource );
extern BOOL		EngineVertexBufferCopyWithCol(  VERTEX_BUFFER_HANDLE hDestination,  VERTEX_BUFFER_HANDLE hSource, unsigned long ulCol );

// Modifying vertex buffers
extern uint32*	EngineVertexBufferLockColourStream( VERTEX_BUFFER_HANDLE, int* pnStride );
extern void		EngineVertexBufferUnlockColourStream( VERTEX_BUFFER_HANDLE );

extern VECT*	EngineVertexBufferLockPositionStream( VERTEX_BUFFER_HANDLE, int* pnStride );
extern void		EngineVertexBufferUnlockPositionStream( VERTEX_BUFFER_HANDLE );

extern VECT*	EngineVertexBufferLockNormalStream( VERTEX_BUFFER_HANDLE, int* pnStride );
extern void		EngineVertexBufferUnlockNormalStream( VERTEX_BUFFER_HANDLE );

extern float*	EngineVertexBufferLockUVStream( VERTEX_BUFFER_HANDLE, int* pnStride );
extern void		EngineVertexBufferUnlockUVStream( VERTEX_BUFFER_HANDLE );

extern VECT*	EngineVertexBufferLockTangentStream( VERTEX_BUFFER_HANDLE, int* pnStride );
extern void		EngineVertexBufferUnlockTangentStream( VERTEX_BUFFER_HANDLE );

// These interfaces will be slower on some platforms 
extern ENGINEBUFFERVERTEX*		EngineVertexBufferGetBufferPointer( VERTEX_BUFFER_HANDLE, int nNumVertsRequired );
extern BOOL		EngineVertexBufferLock( VERTEX_BUFFER_HANDLE, BOOL bClean );
extern BOOL		EngineVertexBufferLockAdd( VERTEX_BUFFER_HANDLE );
extern BOOL		EngineVertexBufferAdd( VERTEX_BUFFER_HANDLE, ENGINEBUFFERVERTEX* );
extern BOOL		EngineVertexBufferGet( VERTEX_BUFFER_HANDLE, int, ENGINEBUFFERVERTEX* );
extern BOOL		EngineVertexBufferSet( VERTEX_BUFFER_HANDLE, int, ENGINEBUFFERVERTEX* );
extern void		EngineVertexBufferAddVertsUsed( VERTEX_BUFFER_HANDLE, int nNumVertsUsed );
extern BOOL		EngineVertexBufferUnlock( VERTEX_BUFFER_HANDLE );

extern void		EngineRenderFullscreenQuad();
//--------------------------------------------------------
//------------ Index buffers
//--------------------------------------------------------
typedef int		INDEX_BUFFER_HANDLE;

extern INDEX_BUFFER_HANDLE		EngineCreateIndexBuffer( int nMaxIndices, int nType );
extern unsigned short*			EngineIndexBufferLock( INDEX_BUFFER_HANDLE, int flags );
extern void						EngineIndexBufferUnlock( INDEX_BUFFER_HANDLE );
extern BOOL						EngineIndexBufferRender( INDEX_BUFFER_HANDLE, VERTEX_BUFFER_HANDLE, int numPolys, int flags );
extern void						EngineIndexBufferFree( INDEX_BUFFER_HANDLE );

//--------------------------------------------------------
//---------- Materials
//---------------------------------------------------------
typedef struct 
{
	float	r;
	float	g;
	float	b;
	float	a;

} ENGINEMATERIAL_COLOUR;

typedef struct 
{
	ENGINEMATERIAL_COLOUR	Diffuse;
	ENGINEMATERIAL_COLOUR	Ambient;
	ENGINEMATERIAL_COLOUR	Specular;
	ENGINEMATERIAL_COLOUR	Emissive;
	float					Power;

} ENGINEMATERIAL;

extern void		EngineSetMaterial( ENGINEMATERIAL* );
extern void		EngineGetMaterial( ENGINEMATERIAL* );
extern void		EngineSetStandardMaterial( void );
extern void		EngineSetStandardMaterialWithSpecular( float fPower, float fSpecularBrightness );
extern void		EngineSetStandardMaterialWithAlpha( float fAlphaVal );
extern void		EngineApplyMaterialRenderStates();		// This makes the engine use the material rather than the vertex colours (why not both?)


//--------------------------------------------------------
//---------- Mesh LOD Generation
//---------------------------------------------------------
extern void		EngineGenerateLODs( int nModelHandle );


//----------------------------------------------------------------------
//   Misc Bits....
//----------------------------------------------------------------------
extern	void*	EngineGetTextureDirect( int nTex );
extern	void	EngineSetTextureDirect( int nTex, void* pTexture );

extern void		EngineDebugNoTextures( BOOL );
TEXTURE_HANDLE	EngineCreateTextureFromBackBuffer( int nNewTexW, int nNewTexH );

//-------------------------------------------------------------------------
// DX Specific functions 
//  References to any of these functions obviously assume DirectX is being used.
//  The engine should really support all the stuff you need in an abstracted form, so you should try to
//  avoid using these calls wherever possible
//-----------------------------------------
#ifdef DIRECT3D_VERSION		// only include this if directx has been included already

#ifndef USE_D3DEX_INTERFACE
#define USE_D3DEX_INTERFACE
#endif

#ifdef USE_D3DEX_INTERFACE
void	EngineInitDX( LPDIRECT3DDEVICE9EX );
void	EngineRemoveDXDevice( );
LPDIRECT3DDEVICE9EX		EngineGetDXDevice(void);
#else
void	EngineInitDX( LPDIRECT3DDEVICE9 );
LPDIRECT3DDEVICE9		EngineGetDXDevice(void);
#endif

// Direct access to the directx texture - should only be used in special circumstances!
LPDIRECT3DTEXTURE9		EngineGetTextureDirectDX( TEXTURE_HANDLE hTexture );

#endif // DIRECT3D_VERSION

#ifdef __cplusplus
}
#endif


//-------------------------------------------------------------------------------------------------------------------------------


//------------------------------------ Define C++ interfaces
#ifdef __cplusplus

//----------------------------------------------------------------------
//   EngineGameInterface
//----------------------------------------------------------------------
class EngineGameInterface
{
public:
	EngineGameInterface()
	{
		mxNullPos.x = 0.0f;
		mxNullPos.y = 0.0f;
		mxNullPos.z = 0.0f;
	}

	virtual const VECT*			GetPlayerPosition( void ) { return( &mxNullPos ); }
	virtual int				GetUniqueMachineID( void ) { return( -1 ); }
	virtual bool			GetFogSetting( void ) { return( false ); }				// Temp - should be internalised
	virtual void			GetShadowMatrix( ENGINEMATRIX* pxMatrix, VECT* pxOffset ) {}		// Temp - should be internalised

	virtual void			AddParticle( int nType, VECT* pxPos, int nParam = 0 ) {}
	virtual void			AddLight( VECT* pxPos, float, float, float, float R, float G, float B ) {}

	virtual void			DebugPrint( int level, const char* szText ) {}
	virtual bool			IsDaytime( void ) { return( false ); }		// For night-only effects (type 7) and night-only materials. An app that doesn't say gets them all the time, as it always did
	// A model's fire effect (chunk 0x18 type 3, campfire) at pxPos in world space, called every frame the model draws, outside the shadow
	// pass. fScale is the model's drawn scale, ulParam the effect's param1. Return true if the app draws the fire itself: the engine then
	// adds none of its own particles or light. An app that doesn't override it gets the engine's fire, as it always did
	virtual bool			AddFire( const VECT* pxPos, int nFireType, float fScale, uint32 ulParam ) { return( false ); }

private:
	VECT			mxNullPos;
};

//----------------------------------------------------------------------
//   SceneMap Game Interface
//----------------------------------------------------------------------
class SceneMapGameInterface
{
public:
	virtual BOOL	FetchSceneFile( const char* szName, const char* szLocalDownloadPath, const char* szHTTPRoot, fnFetchFileCallback ) { return( FALSE ); }

	virtual BOOL	FetchElementFile( const char* szName, const char* szLocalDownloadPath, const char* szHTTPRoot, fnFetchFileCallback ) { return( FALSE ); }

	virtual void	OnElementLoaded( int nModelHandle, VECT* pxOffsetPos, VECT* pxOffsetRotation ) {}

};



void					EngineSetGameInterface( EngineGameInterface* );
EngineGameInterface*	EngineGetGameInterface( void );

void					EngineSetSceneMapGameInterface( SceneMapGameInterface* );
SceneMapGameInterface*	EngineGetSceneMapGameInterface( void );

class EngineInstance;
extern EngineInstance* 		EngineInstanceMain();


#endif // #ifdef __cplusplus


#endif /// TRACTOR_ENGINE_H
