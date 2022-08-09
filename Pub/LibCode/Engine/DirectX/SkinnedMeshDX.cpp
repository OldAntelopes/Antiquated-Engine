
#include "EngineDX.h"

#include <StandardDef.h>


#include "SkinMeshLoaderDX.h"
#include "SkinMeshRenderDX.h"
#include "SkinnedMeshDX.h"


METHOD  m_method = D3DNONINDEXED;

LPD3DXMATRIXA16 m_pBoneMatrices = NULL;
DWORD m_maxBones = 0;
DWORD m_dwIndexedVertexShader[4] = { 0, 0, 0, 0 };

D3DCAPS8	m_d3dCaps;
//------------------------------------------------------------------------------------------------------------------------

SkinnedMeshDX::SkinnedMeshDX()
{

	m_pmcSelectedMesh = NULL;
    m_pframeSelected = NULL;
    m_pdeSelected = NULL;
	m_pdeHead = NULL;
}


SkinnedMeshDX::~SkinnedMeshDX()
{


}

void	SkinnedMeshDX::Render( void )
{
     D3DXMATRIXA16 matIdent;
    
    D3DXMatrixIdentity(&matIdent);

	SDrawElement *pdeCur;

	D3DXMATRIXA16 mat;
    pdeCur = m_pdeHead;
    while (pdeCur != NULL)
    {
        pdeCur->pframeRoot->matRot = matIdent;
        pdeCur->pframeRoot->matTrans = matIdent;
        pdeCur = pdeCur->pdeNext;
    }

    SFrame *pframeCur;

    pdeCur = m_pdeHead;
    while (pdeCur != NULL)
    {
        pdeCur->fCurTime += 50.0f;//m_fElapsedTime * 4800;
        if (pdeCur->fCurTime > 1.0e15f)
            pdeCur->fCurTime = 0;

        pframeCur = pdeCur->pframeAnimHead;
        while (pframeCur != NULL)
        {
            pframeCur->SetTime(pdeCur->fCurTime);
            pframeCur = pframeCur->pframeAnimNext;
        }

        pdeCur = pdeCur->pdeNext;
    }

	
	UINT cTriangles = 0;
		HRESULT hr;
//        SDrawElement *pdeCur;
        D3DXMATRIXA16 mCur;
        D3DXVECTOR3 vTemp;
		
		pdeCur = m_pdeHead;
        while (pdeCur != NULL)
        {
            D3DXMatrixIdentity(&mCur);

            hr = UpdateFrames(pdeCur->pframeRoot, mCur);
            if (FAILED(hr))
                return;
            hr = DrawFrames(pdeCur->pframeRoot, cTriangles);
            if (FAILED(hr))
                return;

            pdeCur = pdeCur->pdeNext;
        }

}

 

bool	SkinnedMeshDX::Load( const char* szFilename )
{
    HRESULT hr;

	mpEngineDevice->GetDeviceCaps( &m_d3dCaps );

	hr = LoadMeshHierarchy( this, szFilename );

    if(SUCCEEDED(hr))
    {
		return( true );
	}
	return( false );
}

