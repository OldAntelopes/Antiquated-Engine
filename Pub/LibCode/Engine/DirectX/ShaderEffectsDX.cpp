
#include <map>

#include <stdio.h>
#include "EngineDX.h"

#include <StandardDef.h>
#include <Engine.h>

#include "ShaderEffectsDX.h"

std::map<int, ID3DXEffect*>		mpEffectsMap;
int		msnNextEffectHandle = 100;


ID3DXEffect*		EngineShaderEffectCreateEffectFromFileInMem( const char* szFilename, BYTE* pbMem, int nMemSize )
{
	DWORD		dwShaderFlags = 0;
	ID3DXEffect* pEffect = NULL;
	LPD3DXBUFFER pCompilationErrors = NULL;

	D3DXCreateEffect( mpEngineDevice, pbMem, nMemSize, NULL, NULL, dwShaderFlags, NULL, &pEffect, &pCompilationErrors );
	if ( pEffect == NULL )
	{
		if (pCompilationErrors)
        {
            // Print the error string to debug output
            const char* errorStr = (const char*)pCompilationErrors->GetBufferPointer();
			if ( errorStr )
			{
	            SysDebugPrint("Shader compilation error in %s:", szFilename );
				SysDebugPrint("%s", errorStr );
			}
            pCompilationErrors->Release();
        }
 	}
	return( pEffect );
}

ID3DXEffect*		EngineShaderEffectCreateEffect( const char* szFilename)
{
	DWORD		dwShaderFlags = 0;
	ID3DXEffect* pEffect = NULL;
	LPD3DXBUFFER pCompilationErrors = NULL;

	D3DXCreateEffectFromFile( mpEngineDevice, szFilename, NULL, NULL, dwShaderFlags, NULL, &pEffect, &pCompilationErrors );
	if ( pEffect == NULL )
	{
		if (pCompilationErrors)
        {
            // Print the error string to debug output
            const char* errorStr = (const char*)pCompilationErrors->GetBufferPointer();
			if ( errorStr )
			{
	            SysDebugPrint("Shader compilation error in %s:", szFilename );
				SysDebugPrint("%s", errorStr );
			}
            pCompilationErrors->Release();
        }
 	}
	return( pEffect );
}


int		EngineShaderEffectLoadFromFileInMem(const char* szFilename, BYTE* pbMem, int nMemSize )
{
ID3DXEffect* pEffect = EngineShaderEffectCreateEffectFromFileInMem( szFilename, pbMem, nMemSize );

	if ( pEffect)
	{
	int		nHandle = msnNextEffectHandle++;
		mpEffectsMap[nHandle] = pEffect;
		return( nHandle );
	}
	return( -2 );
}


int		EngineShaderEffectLoad(const char* szFilename)
{
ID3DXEffect* pEffect = EngineShaderEffectCreateEffect( szFilename );

	if ( pEffect)
	{
	int		nHandle = msnNextEffectHandle++;
		mpEffectsMap[nHandle] = pEffect;
		return( nHandle );
	}
	return( NOTFOUND );
}

void		EngineShaderEffectReload(int handle, const char* szEffectFilename )
{
	if ( mpEffectsMap[handle] )
	{
		mpEffectsMap[handle]->Release();
		mpEffectsMap[handle] = NULL;
	}

	ID3DXEffect* pEffect = EngineShaderEffectCreateEffect(szEffectFilename);
	if ( pEffect)
	{
		mpEffectsMap[handle] = pEffect;
	}
}

void		EngineShaderEffectApply( int handle, fnDrawFunc drawFunc, void* pParamObject )
{
uint		cPasses = 0;
ID3DXEffect* pEffect = mpEffectsMap[handle];

	if ( pEffect )
	{
		pEffect->Begin( &cPasses, 0 );
		for (uint i = 0; i < cPasses; i++)
		{
			pEffect->BeginPass(i);

			// Draw stuff here
			drawFunc(i, pParamObject);

			pEffect->EndPass();
		}
		pEffect->End();
	}
}

void		EngineShaderEffectClear( int handle )
{

}

void EngineShaderEffectSetConstantFloatArray(int handle, const char* szConstantName, float* pfVals, int nNumFloats )
{
	ID3DXEffect* pEffect = mpEffectsMap[handle];
    if (!pEffect) return;

    D3DXHANDLE hParam = pEffect->GetParameterByName(NULL, szConstantName);
    if (!hParam) return;

    D3DXPARAMETER_DESC paramDesc;
    if (FAILED(pEffect->GetParameterDesc(hParam, &paramDesc))) return;

//    if (paramDesc.Type != D3DXPT_FLOAT || paramDesc.Class != D3DXPC_SCALAR) return;

	pEffect->SetFloatArray( hParam, pfVals, nNumFloats );
}


void EngineShaderEffectSetConstantBool(int handle, const char* szConstantName, bool bFlag )
{
    ID3DXEffect* pEffect = mpEffectsMap[handle];
    if (!pEffect) return;

    D3DXHANDLE hParam = pEffect->GetParameterByName(NULL, szConstantName);
    if (!hParam) return;

    D3DXPARAMETER_DESC paramDesc;
    if (FAILED(pEffect->GetParameterDesc(hParam, &paramDesc))) return;

    // Only set if it's a float and scalar
    if (paramDesc.Type != D3DXPT_BOOL) return;

	pEffect->SetBool( hParam, bFlag );
}

void EngineShaderEffectSetConstantFloat(int handle, const char* szConstantName, float fVal)
{
    ID3DXEffect* pEffect = mpEffectsMap[handle];
    if (!pEffect) return;

    D3DXHANDLE hParam = pEffect->GetParameterByName(NULL, szConstantName);
    if (!hParam) return;

    D3DXPARAMETER_DESC paramDesc;
    if (FAILED(pEffect->GetParameterDesc(hParam, &paramDesc))) return;

    // Only set if it's a float and scalar
    if (paramDesc.Type != D3DXPT_FLOAT || paramDesc.Class != D3DXPC_SCALAR) return;

	pEffect->SetFloat( hParam, fVal );
}

// TOOD - matrices, vectors, textures etc..
void		EngineShaderEffectFree( int handle )
{

}
