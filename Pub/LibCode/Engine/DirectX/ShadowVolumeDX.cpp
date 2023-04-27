#ifdef TUD9

#include "EngineDX.h"

#include <stdio.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
#include <Interface.h>

#include "../ModelRendering.h"
#include "ShadowVolumeDX.h"
//#include "../../Universal/GameCode/Profile.h"		// temp

#define		SHADOWVOLUME_VERTEX_LIST_SIZE	32000

struct SHADOWVERTEX
{
    D3DXVECTOR4 p;
    D3DCOLOR    color;
};

#define D3DFVF_SHADOWVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

IGRAPHICSVERTEXBUFFER* m_pBigSquareVB = NULL;
D3DXMATRIX    m_matObjectMatrix;
int		m_sVolumesRenderedThisFrame = 0;
int		m_sVolumesGeneratedThisFrame = 0;
bool	m_sbUseShadowVolumes = false;
VECT	m_lightPos = { -100.0f, 0.0f, 100.0f };
float	mfShadowStrength = 1.0f;
float	mfLastRenderedShadowStrength = 1.0f;
float	m_sfShadowVolumeCutoffDistance = 10.0f;

//-----------------------------------------------------------------------------
// Name: struct ShadowVolume
// Desc: A shadow volume object
//-----------------------------------------------------------------------------
class ShadowVolume
{
    D3DXVECTOR3 m_pVertices[SHADOWVOLUME_VERTEX_LIST_SIZE]; // Vertex data for rendering shadow volume
    DWORD       m_dwNumVertices;

public:
	ShadowVolume() { m_dwNumVertices = 0; }
	VOID    Reset() { m_dwNumVertices = 0L; }
    HRESULT BuildFromMesh( LPD3DXMESH pObject, D3DXVECTOR3 vLight, VECT* pxOffset = NULL );
    HRESULT Render( LPGRAPHICSDEVICE pd3dDevice );
};

ShadowVolume*	m_pShadowVolume = NULL;

int		m_ShadowVolFrameCount = 0;

//------------------------------------------------------------------------------------
class ShadowRenderList
{
public:
	ShadowRenderList();
	~ShadowRenderList();

	void					Set( int hModel, const VECT* pxPos, const VECT* pxRot, unsigned int uID );
	void					Update( void );	
	void					Render( void );	

	void					SetNext(ShadowRenderList* pNext ) { m_pNext = pNext; }
	ShadowRenderList*		GetNext() { return( m_pNext ); }

	static ShadowRenderList*	Find( int hModel, unsigned int uID );
	static ShadowRenderList*	GetNew( void );

	static void					RenderAll( void );
	static void					UpdateAll( void );
	static void					FreeAll( void );
private:

	void		GenerateShadowVolume( void );
	void		RenderShadowVolume( void );
	void		GenerateShadowVolumeSubModel( int nModelHandle, VECT* pxPos, D3DXVECTOR3* pxMatrix );

	ShadowRenderList*		m_pNext;

	int		m_hModelHandle;
	unsigned int	m_uID;
	VECT	m_pos;
	VECT	m_rot;
	ENGINEMATRIX	m_matWorldTransform;
	bool	m_bHasMatrix;

	int		m_renderFrame;

	VECT	m_lastVolumePos;
	VECT	m_lastVolumeRot;
	VECT	m_lastVolumeLightPos;
	int		m_lastVolumeFrame;

	ShadowVolume*		m_pShadowVolume;
};

ShadowRenderList::ShadowRenderList()
{
	m_pShadowVolume = NULL;
	m_pNext = NULL;
	m_hModelHandle = -1;
	m_lastVolumeFrame = -1;
	m_renderFrame = -1;
	m_uID = 0;
	m_bHasMatrix = false;
}

ShadowRenderList::~ShadowRenderList()
{
	SAFE_DELETE( m_pShadowVolume );
}

ShadowRenderList*		m_pShadowRenderList = NULL;


ShadowRenderList*	ShadowRenderList::Find( int hModel, unsigned int uID )
{
ShadowRenderList*		pList = m_pShadowRenderList;

	while( pList )
	{
		if ( ( pList->m_hModelHandle == hModel ) &&
			 ( pList->m_uID == uID ) )
		{
			return( pList );
		}
		pList = pList->GetNext();
	}
	return( NULL );
}


ShadowRenderList*	ShadowRenderList::GetNew( void )
{
ShadowRenderList*		pList = new ShadowRenderList;

	pList->SetNext( m_pShadowRenderList );
	m_pShadowRenderList = pList;
	return( pList );
}

void		ShadowRenderList::Set( int hModel, const VECT* pxPos, const VECT* pxRot, unsigned int uID )
{
	m_hModelHandle = hModel;
	// If no position passed, we get the info from the world matrix
	if ( pxPos == NULL )
	{
		mpEngineDevice->GetTransform( D3DTS_WORLD, (D3DXMATRIX*)&m_matWorldTransform );	
		m_bHasMatrix = true;
	}
	else
	{
		m_pos = *pxPos;
		if ( pxRot )
		{
			m_rot = *pxRot;
		}
		else
		{
			m_rot.x = 0.0f;
			m_rot.y = 0.0f;
			m_rot.z = 0.0f;
		}
		m_bHasMatrix = false;
	}

	m_uID = uID;
	m_renderFrame = m_ShadowVolFrameCount;

	if ( !m_pShadowVolume )
	{
		// Construct a shadow volume object;
		m_pShadowVolume = new ShadowVolume();
	}
}


void		ShadowRenderList::GenerateShadowVolumeSubModel( int nModelHandle, VECT* pxOffset, D3DXVECTOR3* pxLightInObjectSpace )
{
	MODEL_RENDER_DATA* pxModelData = &maxModelRenderData[ nModelHandle ];
	if ( pxModelData->pxBaseMesh != NULL )
	{
		m_pShadowVolume->BuildFromMesh( (LPD3DXMESH)pxModelData->pxBaseMesh->GetPlatformMeshImpl(), *pxLightInObjectSpace, pxOffset );
	}
}


void		ShadowRenderList::GenerateShadowVolume( void )
{
VECT*	pxPos = &m_pos;
VECT*	pxRot = &m_rot;
D3DXMATRIX		matWorld;

    m_pShadowVolume->Reset();
	if ( m_bHasMatrix )
	{
		EngineSetWorldMatrix( &m_matWorldTransform );
		matWorld = m_matWorldTransform;
	}
	else
	{
		EngineSetMatrixFromRotations( pxRot, &matWorld );
		matWorld._41 = pxPos->x;
		matWorld._42 = pxPos->y;
		matWorld._43 = pxPos->z;
		EngineSetWorldMatrix( &matWorld );
	}

	// transform the light vector from world-space to object-space
	D3DXVECTOR3 vLightInWorldSpace( m_lightPos.x, m_lightPos.y, m_lightPos.z );
    D3DXVECTOR3 vLightInObjectSpace;
    D3DXMATRIXA16 matInverse;
    D3DXMatrixInverse( &matInverse, NULL, &matWorld );
	D3DXVec3TransformNormal( &vLightInObjectSpace, &vLightInWorldSpace, &matInverse );

	MODEL_RENDER_DATA* pxModelData = &maxModelRenderData[ m_hModelHandle ];
	if ( pxModelData->pxBaseMesh != NULL )
	{
		if ( pxModelData->bModelType == ASSETTYPE_KEYFRAME_ANIMATION )
		{
			ModelRenderKeyframeAnimationGenerateBaseMesh( pxModelData, mulLastRenderingTick );
		}

		m_pShadowVolume->BuildFromMesh( (LPD3DXMESH)pxModelData->pxBaseMesh->GetPlatformMeshImpl(), vLightInObjectSpace );


		// TODO - Need to include wheels / turrets / etc
/*
		if ( pxModelData->xWheel1AttachData.nModelHandle != NOTFOUND )
		{
		VECT	xOffset;

			xOffset = pxModelData->xWheel1AttachData.xRawOffset;
			GenerateShadowVolumeSubModel( pxModelData->xWheel1AttachData.nModelHandle, &xOffset, &vLightInObjectSpace );
		}
*/
	}
	m_lastVolumePos = m_pos;
	m_lastVolumeRot = m_rot;
	m_lastVolumeLightPos = m_lightPos;
	m_lastVolumeFrame = 0;
}

bool	g_bTwoSidedStencilsAvailable = false;

void	ShadowRenderList::RenderShadowVolume( void )
{
VECT*	pxPos = &m_pos;
VECT*	pxRot = &m_rot;
D3DXMATRIX		matWorld;

	if ( m_bHasMatrix )
	{
		EngineSetWorldMatrix( &m_matWorldTransform );
		matWorld = m_matWorldTransform;
	}
	else
	{
		EngineSetMatrixFromRotations( pxRot, &matWorld );
		matWorld._41 = pxPos->x;
		matWorld._42 = pxPos->y;
		matWorld._43 = pxPos->z;
		EngineSetWorldMatrix( &matWorld );
	}

	if( g_bTwoSidedStencilsAvailable == true )
    {
        // With 2-sided stencil, we can avoid rendering twice:
        mpEngineDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, TRUE );
        mpEngineDevice->SetRenderState( D3DRS_CCW_STENCILFUNC,  D3DCMP_ALWAYS );
        mpEngineDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP );
        mpEngineDevice->SetRenderState( D3DRS_CCW_STENCILFAIL,  D3DSTENCILOP_KEEP );
//        mpEngineDevice->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR );
        mpEngineDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_INCR );
		
        mpEngineDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_NONE );

        // Draw both sides of shadow volume in stencil/z only
        m_pShadowVolume->Render( mpEngineDevice );

        mpEngineDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, FALSE );
    }
    else
    {
	    // Draw front-side of shadow volume in stencil/z only
	    m_pShadowVolume->Render( mpEngineDevice );

	    // Now reverse cull order so back sides of shadow volume are written.
		mpEngineDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CW );

    // Decrement stencil buffer value
// standard
//    mpEngineDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );
// WEB MODIFIIED
		mpEngineDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR );

	    // Draw back-side of shadow volume in stencil/z only
		m_pShadowVolume->Render( mpEngineDevice );
	}

    // Restore render states
    mpEngineDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );

	// Web modified
    mpEngineDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR  );

//	mpEngineDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );
}

bool	HasChanged( VECT* pxVec1, VECT* pxVec2, float fTolerance )
{
float	x = pxVec1->x - pxVec2->x;
float	y = pxVec1->y - pxVec2->y;
float	z = pxVec1->z - pxVec2->z;

	if ( x < 0.0f ) x = 0.0f - x;
	if ( y < 0.0f ) y = 0.0f - y;
	if ( z < 0.0f ) z = 0.0f - z;

	float	diff = x + y + z;
	if ( diff > fTolerance )
	{
		return( true );
	}
	return( false );
}

void		ShadowRenderList::Update( void )
{
	// Optimise so that the shadow volume is only generated when needed
	//  (i.e. if animated, or when object or light has moved)
	if ( m_renderFrame == m_ShadowVolFrameCount )
	{
		if ( ( ModelIsAnimated( m_hModelHandle ) ) ||
			 ( m_bHasMatrix ) ||
			 ( HasChanged( &m_pos, &m_lastVolumePos, 0.001f ) ) ||
			 ( HasChanged( &m_rot, &m_lastVolumeRot, 0.001f ) ) ||
			 ( HasChanged( &m_lightPos, &m_lastVolumeLightPos, 0.01f ) ) )
		{
			GenerateShadowVolume();
			m_sVolumesGeneratedThisFrame++;
		}
	}
	else	// Wasn't rendered this frame.. morgue it
	{
		// TODO - 
	}
}

void		ShadowRenderList::Render( void )
{
	// TODO - Optimise so that the shadow volume is only generated when needed
	//  (i.e. if animated, or when object or light has moved)
	if ( m_renderFrame == m_ShadowVolFrameCount-1 )
	{
		RenderShadowVolume();
	}
}


void		ShadowRenderList::FreeAll( void )
{
ShadowRenderList*		pList = m_pShadowRenderList;
ShadowRenderList*		pNext;

	while( pList )
	{
		pNext = pList->GetNext();
		delete pList;
		pList = pNext;
	}
	m_pShadowRenderList = NULL;
}

void		ShadowRenderList::UpdateAll( void )
{
ShadowRenderList*		pList = m_pShadowRenderList;

	while( pList )
	{
		pList->Update();
		pList = pList->GetNext();
	}

	m_ShadowVolFrameCount++;
}

float	mfShadowVolProjFOV = PI/4.0f;
float	mfShadowVolProjNear = 0.01f;
float	mfShadowVolProjFar = 21.0f;

void		ShadowVolumeSetProjection( float fFOV, float fNear, float fFar )
{
	mfShadowVolProjFOV = fFOV;
	mfShadowVolProjNear = fNear;
	mfShadowVolProjFar = fFar;
}


void		ShadowRenderList::RenderAll( void )
{
ShadowRenderList*		pList = m_pShadowRenderList;
D3DXMATRIX	matProj;
float	fAspect = (float)(InterfaceGetWidth())/(float)(InterfaceGetHeight());

	EngineCameraUpdate();
	EngineDefaultState();

	mpEngineDevice->SetTexture( 0, NULL );

	// Set initial render states
	// Disable z-buffer writes (note: z-testing still occurs), and enable the stencil-buffer
	EngineEnableZTest( TRUE );
	EngineEnableZWrite( FALSE );
    mpEngineDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );
	EngineEnableLighting( FALSE );
    mpEngineDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );
    mpEngineDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS );
	EngineSetZBias(0);
	// Dont bother with interpolating color
    mpEngineDevice->SetRenderState( D3DRS_SHADEMODE,     D3DSHADE_FLAT );

	float	fVal;
   
// Web modified ------------	
	mpEngineDevice->GetTransform( D3DTS_PROJECTION, &matProj );
//	D3DXMatrixPerspectiveFovLH(&matProj,gfFOV, fAspect, 1.0f, gfFarClipPlane);
	D3DXMatrixPerspectiveFovLH(&matProj,mfShadowVolProjFOV, fAspect, mfShadowVolProjNear, mfShadowVolProjFar);

	// use a far_plane of infinity for projection matrix
	// Limit of far_plane/(far_plane - near_plane)
	// as far_plane approaches infinity is 1.0
	fVal = (mfShadowVolProjFar/(mfShadowVolProjFar-mfShadowVolProjNear));
	matProj(2,2) = fVal;//(mfShadowVolProjFar/(mfShadowVolProjFar-mfShadowVolProjNear));

//	mfShadowVolProjNear *= 10.0f;
//	mfShadowVolProjFar *= 10.0f;
	// Limit of -far_plane*near_plane/(far_plane - near_plane)
	// as far_plane approaches infinity is -1.0
	fVal = ((0.0f - mfShadowVolProjFar)*mfShadowVolProjNear)/(mfShadowVolProjFar-mfShadowVolProjNear);
	matProj(3,2) = fVal;
	EngineSetProjectionMatrix( &matProj );
//---------------------------

    // Set up stencil compare fuction, reference value, and masks.
    // Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
    // Note: since we set up the stencil-test to always pass, the STENCILFAIL
    // renderstate is really not needed.
    mpEngineDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_ALWAYS );
// Standard
//    mpEngineDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
// Web modified
    mpEngineDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR  );
    mpEngineDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );

    // If ztest passes, inc/decrement stencil buffer value
    mpEngineDevice->SetRenderState( D3DRS_STENCILREF,       0x1 );
    mpEngineDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff );
    mpEngineDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );

	// Standard
//	mpEngineDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );
	// Web modified
	mpEngineDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_KEEP );

    // Make sure that no pixels get drawn to the frame buffer
	EngineEnableBlend( TRUE );
    mpEngineDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
    mpEngineDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	while( pList )
	{
		pList->Render();
		pList = pList->GetNext();
	}


	// Restore standard states
	mpEngineDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	EngineEnableZWrite( TRUE );
    mpEngineDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    mpEngineDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	EngineSetZBias(0);
}


//--------------------------------------------------------------------------------------------------------

void		ShadowVolumeSetLightPosition( VECT* pxPos )
{
	m_lightPos = *pxPos;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT ShadowVolume::Render( LPGRAPHICSDEVICE pd3dDevice )
{
	HRESULT		nRet;

	EngineSetVertexFormat( VERTEX_FORMAT_XYZ );

    nRet = pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, m_dwNumVertices/3,
                                        m_pVertices, sizeof(D3DXVECTOR3) );

	return( nRet );
}




//-----------------------------------------------------------------------------
// Name: AddEdge()
// Desc: Adds an edge to a list of silohuette edges of a shadow volume.
//-----------------------------------------------------------------------------
VOID AddEdge( WORD* pEdges, DWORD& dwNumEdges, WORD v0, WORD v1 )
{
    // Remove interior edges (which appear in the list twice)
    for( DWORD i=0; i < dwNumEdges; i++ )
    {
        if( ( pEdges[2*i+0] == v0 && pEdges[2*i+1] == v1 ) ||
            ( pEdges[2*i+0] == v1 && pEdges[2*i+1] == v0 ) )
        {
            if( dwNumEdges > 1 )
            {
                pEdges[2*i+0] = pEdges[2*(dwNumEdges-1)+0];
                pEdges[2*i+1] = pEdges[2*(dwNumEdges-1)+1];
            }
            dwNumEdges--;
            return;
        }
    }

    pEdges[2*dwNumEdges+0] = v0;
    pEdges[2*dwNumEdges+1] = v1;
    dwNumEdges++;
}





//-----------------------------------------------------------------------------
// Name: BuildFromMesh()
// Desc: Takes a mesh as input, and uses it to build a shadowvolume. The
//       technique used considers each triangle of the mesh, and adds it's
//       edges to a temporary list. The edge list is maintained, such that
//       only silohuette edges are kept. Finally, the silohuette edges are
//       extruded to make the shadow volume vertex list.
//-----------------------------------------------------------------------------
HRESULT ShadowVolume::BuildFromMesh( LPD3DXMESH pMesh, D3DXVECTOR3 vLight, VECT* pxOffset )
{
unsigned int	i;

    DWORD dwFVF = pMesh->GetFVF();

    CUSTOMVERTEX* pVertices;
    WORD*       pIndices;
    // Lock the geometry buffers
    pMesh->LockVertexBuffer( 0L, (VERTEX_LOCKTYPE)&pVertices );
    pMesh->LockIndexBuffer( 0L, (VERTEX_LOCKTYPE)&pIndices );
    DWORD dwNumVertices = pMesh->GetNumVertices();
    DWORD dwNumFaces    = pMesh->GetNumFaces();

    // Allocate a temporary edge list
    WORD* pEdges = new WORD[dwNumFaces*6];
    DWORD dwNumEdges = 0;

    // For each face
    for( i=0; i < dwNumFaces; i++ )
    {
        WORD wFace0 = pIndices[3*i+0];
        WORD wFace1 = pIndices[3*i+1];
        WORD wFace2 = pIndices[3*i+2];
		D3DXVECTOR3 v0, v1, v2;

		if ( pxOffset )
		{
			VectAdd( (VECT*)&v0, &pVertices[wFace0].position, pxOffset );
			VectAdd( (VECT*)&v1, &pVertices[wFace1].position, pxOffset );
			VectAdd( (VECT*)&v2, &pVertices[wFace2].position, pxOffset );
		}
		else
		{
			v0 = *( (D3DXVECTOR3*)&pVertices[wFace0].position );
			v1 = *( (D3DXVECTOR3*)&pVertices[wFace1].position );
			v2 = *( (D3DXVECTOR3*)&pVertices[wFace2].position );
		}

        // Transform vertices or transform light?
        D3DXVECTOR3 vNormal;
        D3DXVec3Cross( &vNormal, &(v2-v1), &(v1-v0) );

// Web modified
		if ( m_dwNumVertices >= (SHADOWVOLUME_VERTEX_LIST_SIZE - 3) )
		{
			break;
		}

		if( D3DXVec3Dot( &vNormal, &vLight ) >= 0.0f )
		{
	        AddEdge( pEdges, dwNumEdges, wFace0, wFace1 );
			AddEdge( pEdges, dwNumEdges, wFace1, wFace2 );
		    AddEdge( pEdges, dwNumEdges, wFace2, wFace0 );

			m_pVertices[m_dwNumVertices++] = v2;
			m_pVertices[m_dwNumVertices++] = v1;
			m_pVertices[m_dwNumVertices++] = v0;
	    }
		else
		{
	        D3DXVECTOR3 v3 = v0 - vLight*10;
		    D3DXVECTOR3 v4 = v1 - vLight*10;
			D3DXVECTOR3 v5 = v2 - vLight*10;

			m_pVertices[m_dwNumVertices++] = v5;
			m_pVertices[m_dwNumVertices++] = v4;
			m_pVertices[m_dwNumVertices++] = v3;
	    }
	// Standard
/*
        if( D3DXVec3Dot( &vNormal, &vLight ) >= 0.0f )
        {
            AddEdge( pEdges, dwNumEdges, wFace0, wFace1 );
            AddEdge( pEdges, dwNumEdges, wFace1, wFace2 );
            AddEdge( pEdges, dwNumEdges, wFace2, wFace0 );
        }
*/
		//-------------------------
	
	}

    for( i=0; i<dwNumEdges; i++ )
    {
        D3DXVECTOR3 v1 = *( (D3DXVECTOR3*)&pVertices[pEdges[2*i+0]].position );
        D3DXVECTOR3 v2 = *( (D3DXVECTOR3*)&pVertices[pEdges[2*i+1]].position );
        D3DXVECTOR3 v3 = v1 - vLight*10;
        D3DXVECTOR3 v4 = v2 - vLight*10;

		if ( m_dwNumVertices >= (SHADOWVOLUME_VERTEX_LIST_SIZE - 6) )
		{
			break;
		}
		// Add a quad (two triangles) to the vertex list
		m_pVertices[m_dwNumVertices++] = v1;
		m_pVertices[m_dwNumVertices++] = v2;
		m_pVertices[m_dwNumVertices++] = v3;

		m_pVertices[m_dwNumVertices++] = v2;
		m_pVertices[m_dwNumVertices++] = v4;
		m_pVertices[m_dwNumVertices++] = v3;
    }
    // Delete the temporary edge list
    delete[] pEdges;

    // Unlock the geometry buffers
    pMesh->UnlockVertexBuffer();
    pMesh->UnlockIndexBuffer();

    return S_OK;
}


int		ShadowVolumeInitDX( void )
{
	if ( ( mpEngineDevice ) &&
		 ( !m_pBigSquareVB ) )
	{
		D3DCAPS9 d3dCaps;
		mpEngineDevice->GetDeviceCaps( &d3dCaps );
		if( ( d3dCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED ) != 0 )
		g_bTwoSidedStencilsAvailable = true;

		// Create a big square for rendering the stencilbuffer contents
		if( FAILED( mpEngineDevice->CreateVertexBuffer( 4*sizeof(SHADOWVERTEX),
										   D3DUSAGE_WRITEONLY, D3DFVF_SHADOWVERTEX,
										   D3DPOOL_DEFAULT, &m_pBigSquareVB, NULL ) ) )
			return -1;

		SHADOWVERTEX* v;
		FLOAT sx = (FLOAT)InterfaceGetWidth();
		FLOAT sy = (FLOAT)InterfaceGetHeight();
		m_pBigSquareVB->Lock( 0, 0, (VERTEX_LOCKTYPE)&v, 0 );
		v[0].p = D3DXVECTOR4(  0, sy, 0.0f, 1.0f );
		v[1].p = D3DXVECTOR4(  0,  0, 0.0f, 1.0f );
		v[2].p = D3DXVECTOR4( sx, sy, 0.0f, 1.0f );
		v[3].p = D3DXVECTOR4( sx,  0, 0.0f, 1.0f );
		v[0].color = 0x6E000000;
		v[1].color = 0x6E000000;
		v[2].color = 0x6E000000;
		v[3].color = 0x6E000000;
		m_pBigSquareVB->Unlock();
	}

    return 0;
}



void		ShadowVolumeRenderModel( int nModelHandle, const VECT* pxPos, const VECT* pxRot, unsigned long ulRenderFlags, unsigned int uID )
{
	if ( !m_sbUseShadowVolumes ) return;

	if ( ModelShadowsEnabled( nModelHandle ) == FALSE )
	{
		return;
	}

	if ( pxPos )
	{
	VECT	xCamPos = *EngineCameraGetPos();

		float	fDistFromCam = VectDist( &xCamPos, pxPos );

		if ( fDistFromCam < m_sfShadowVolumeCutoffDistance )
		{
		ShadowRenderList*		pRenderList;
	
			pRenderList = ShadowRenderList::Find( nModelHandle, uID );
			if ( !pRenderList )
			{
				pRenderList = ShadowRenderList::GetNew();
			}
			pRenderList->Set( nModelHandle, pxPos, pxRot, uID );
	
			m_sVolumesRenderedThisFrame++;
		}
	}
}

void	UpdateBigSquareCols( void )
{
SHADOWVERTEX* v;
uint32	ulCol = ( (uint32)( 110.0f * mfShadowStrength ) ) << 24;
	
	m_pBigSquareVB->Lock( 0, 0, (VERTEX_LOCKTYPE)&v, 0 );
	v[0].color = ulCol;
	v[1].color = ulCol;
	v[2].color = ulCol;
	v[3].color = ulCol;
	m_pBigSquareVB->Unlock();

	mfLastRenderedShadowStrength = mfShadowStrength;
}


void	RenderShadowStencil( void )
{
HRESULT ret;
	if ( !m_pBigSquareVB ) return;

	if ( mfShadowStrength != mfLastRenderedShadowStrength )
	{
		UpdateBigSquareCols();
	}

	if ( mfShadowStrength == 0.0f )
	{
		return;
	}

    // Set renderstates (disable z-buffering, enable stencil, disable fog, and
    // turn on alphablending)
	EngineEnableZTest( FALSE );
    mpEngineDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE );
	EngineEnableFog( FALSE );
	EngineEnableBlend( TRUE );
    mpEngineDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    mpEngineDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
//  No diff..    mpEngineDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
/*
    mpEngineDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    mpEngineDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    mpEngineDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    mpEngineDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
*/

	EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
 
    // Only write where stencil val >= 1 (count indicates # of shadows that
    // overlap that pixel)
    mpEngineDevice->SetRenderState( D3DRS_STENCILREF,  0x1 );
    mpEngineDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );
    mpEngineDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );

    // Draw a big, gray square
	EngineSetVertexFormat( VERTEX_FORMAT_SHADOWVERTEX );
    ret = mpEngineDevice->SetStreamSource( 0, m_pBigSquareVB, 0, sizeof(SHADOWVERTEX) );
	if ( ret == D3D_OK )
	{
		mpEngineDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	}

    // Restore render states
    mpEngineDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
}



void		ShadowVolumeRender( void )
{
	if ( !m_sbUseShadowVolumes ) return;

	ShadowRenderList::UpdateAll();
//	ProfileMark( "SV Update" );
	ShadowRenderList::RenderAll();
//	ProfileMark( "SV render" );
	RenderShadowStencil();

}

void		ShadowVolumeFree( void )
{
	ShadowRenderList::FreeAll();
    SAFE_RELEASE( m_pBigSquareVB );

}


void		ShadowVolumeUpdate( void )
{
	m_sVolumesRenderedThisFrame = 0;
	m_sVolumesGeneratedThisFrame = 0;
}


void		ToggleShadowVolumes( void )
{
	m_sbUseShadowVolumes = !m_sbUseShadowVolumes;
}

BOOL		ShadowVolumeRenderIsEnabled( void )
{
	return( m_sbUseShadowVolumes );
}


void		ShadowVolumeRenderingEnable( BOOL bFlag, float fCutoffDistance )
{
	if ( bFlag == TRUE )
	{
		m_sbUseShadowVolumes = true;
		m_sfShadowVolumeCutoffDistance = fCutoffDistance;
	}
	else
	{
		m_sbUseShadowVolumes = false;
	}

}

void		ShadowVolumeSetShadowStrength( float fStrength )
{
	mfShadowStrength = fStrength;

}


#endif // ifdef TUD9