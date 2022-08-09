
#if 0

#ifdef ENGINEDX		// ---- DirectX
#include "EngineDX.h"
#include "ShaderLoaderDX.h"
#endif

#include "StandardDef.h"
#include "Engine.h"
#include "Interface.h"
#include "Rendering.h"

#include "CascadeShadowMapDX.h"

const uint SHADOW_MAP_CASCADE_COUNT = 3;
const UINT SHADOW_MAP_SIZE = 1024;
const D3DFORMAT SHADOW_MAP_FORMAT = D3DFMT_R32F;

LPGRAPHICSTEXTURE	g_ShadowMapTextures[SHADOW_MAP_CASCADE_COUNT] = { NULL };
LPGRAPHICSSURFACE	g_ShadowMapSurfaces[SHADOW_MAP_CASCADE_COUNT] = { NULL };

LPGRAPHICSPIXELSHADER				mpCascadeShadowMapPass1PixelShader = NULL; 
LPGRAPHICSVERTEXSHADER				mpCascadeShadowMapPass1VertexShader = NULL; 
LPGRAPHICSCONSTANTBUFFER			mpCascadeShadowMapRenderConstantTable = NULL; 
LPGRAPHICSCONSTANTBUFFER			mpCascadeShadowMapRenderPSConstantTable = NULL; 

BOOL	mbCascadeShadowMapHasLoadedShaders = FALSE;


float	mShadowMappingSplitDepths[SHADOW_MAP_CASCADE_COUNT];
float	g_ShadowSplitLogFactor = 0.9f;


void CascadeShadowMapsCalcSplitDepths( float *outDepths, float fCamZNear, float fCamZFar )
{
float i_f = 1.f, cascadeCount = (float)SHADOW_MAP_CASCADE_COUNT;

	for (uint i = 0; i < SHADOW_MAP_CASCADE_COUNT-1; i++, i_f += 1.f)	
	{
		outDepths[i] = Lerp( fCamZNear + (i_f/cascadeCount)*(fCamZFar - fCamZNear),
							  fCamZNear * powf(fCamZFar / fCamZNear, i_f/cascadeCount),
						     g_ShadowSplitLogFactor);
  }
  outDepths[SHADOW_MAP_CASCADE_COUNT-1] = fCamZFar;
}



void CalcShadowMapMatrices( ENGINEMATRIX* pxOutViewProj,  MATRIX &outShadowMapTexXform,
  const ParamsCamera &camera, SceneObjectPtrVector &objs)
{
  const VEC3 *upDir = &VEC3_POSITIVE_Y;
  if (fabsf(Dot(g_DirToLight, *upDir)) > 0.99f)
    upDir = &VEC3_POSITIVE_Z;
  
  VEC3 axisX, axisY, axisZ = -g_DirToLight;
  Cross(&axisX, axisZ, *upDir);
  Normalize(&axisX);
  Cross(&axisY, axisX, axisZ);
  
  MATRIX view;
  AxesToMatrix(&view, axisX, axisY, axisZ);
  Transpose(&view);





CascadeShadowMapsCalcSplitDepths(mShadowMappingSplitDepths, camera);


#endif // if 0 