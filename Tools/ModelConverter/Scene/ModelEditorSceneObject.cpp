// ModelEditor.cpp : Defines the entry point for the application.
//


#include <stdio.h>
#include <windows.h>

#include <d3dx9.h>

#include <StandardDef.h>
#include <Rendering.h>
#include <Interface.h>
#include <System.h>
#include <Engine.h>

#include "../Util/Line Renderer.h"
#include "../Util/Point Renderer.h"

#include "../LibCode/Engine/ModelRendering.h"
#include "../LibCode/Engine/DirectX/ShadowVolumeDX.h"
#include "../LibCode/Engine/ModelMaterialData.h"


//#include "ModelEditor.h"
//#include "ModelEditorView.h"
//#include "ModelEditorToolbar.h"
#include "../ModelConverter.h"
#include "ModelEditorSceneObject.h"



CSceneObject::CSceneObject()
{
	mnModelHandle = NOTFOUND;
	mhTexture = NOTFOUND;
	mpbVertexSelections = NULL;
	mpbFaceSelections = NULL;
	mhFaceBuffer = NOTFOUND;
	mxPos.x = 0.0f;
	mxPos.y = 0.0f;
	mxPos.z = 0.0f;
	mxRot = mxPos;
	mnNumFacesSelected = 0;
	mnNumVerticesSelected = 0;
	mbSelectedFaceHighlight = true;
	mbRenderWireframe = false;

}

CSceneObject::~CSceneObject()
{
	if ( mnModelHandle != NOTFOUND )
	{
		ModelFree( mnModelHandle );
		mnModelHandle = NOTFOUND;
	}
	if ( mhTexture != NOTFOUND )
	{
		EngineReleaseTexture( &mhTexture );
	}
	if ( mhFaceBuffer != NOTFOUND )
	{
		EngineVertexBufferFree( mhFaceBuffer );
		mhFaceBuffer = NOTFOUND;
	}
}

void	CSceneObject::UpdateFaceMeshList(  )
{
MODEL_RENDER_DATA*		pxModelData;
int		nNumVerts = ModelGetStats(mnModelHandle)->nNumVertices;
int		nNumFaces = ModelGetStats(mnModelHandle)->nNumIndices/3;
int		nLoop;
ENGINEBUFFERVERTEX	xVertex1;
ENGINEBUFFERVERTEX	xVertex2;
ENGINEBUFFERVERTEX	xVertex3;
CUSTOMVERTEX*	pxVert1;
CUSTOMVERTEX*	pxVert2;
CUSTOMVERTEX*	pxVert3;
CUSTOMVERTEX*	pxVertices;
ushort*			puwIndices = NULL;
unsigned int*	punIndices = NULL;

	if ( mhFaceBuffer != NOTFOUND )
	{
		pxModelData = &maxModelRenderData[ mnModelHandle ];

		pxModelData->pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );

		if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
		{
			pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &punIndices ) );
		}
		else
		{
			pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );
		}

		EngineVertexBufferLock( mhFaceBuffer, TRUE );
		for ( nLoop = 0; nLoop < nNumFaces*3; nLoop += 3 )
		{
			// For each tri in the other model, first check the tri box against the model1 overall bounding box
			if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
			{
				pxVert1 = pxVertices + punIndices[ nLoop ];
				pxVert2 = pxVertices + punIndices[ nLoop+1 ];
				pxVert3 = pxVertices + punIndices[ nLoop+2 ];
			}
			else
			{
				pxVert1 = pxVertices + puwIndices[ nLoop ];
				pxVert2 = pxVertices + puwIndices[ nLoop+1 ];
				pxVert3 = pxVertices + puwIndices[ nLoop+2 ];
			}
			xVertex1 = *((ENGINEBUFFERVERTEX*)(pxVert1));
			xVertex2 = *((ENGINEBUFFERVERTEX*)(pxVert2));
			xVertex3 = *((ENGINEBUFFERVERTEX*)(pxVert3));
			
			EngineVertexBufferAdd( mhFaceBuffer, &xVertex1 );
			EngineVertexBufferAdd( mhFaceBuffer, &xVertex2 );
			EngineVertexBufferAdd( mhFaceBuffer, &xVertex3 );
		}
		EngineVertexBufferUnlock( mhFaceBuffer );
	}
}

void	CSceneObject::InitDX()
{
	if ( mnModelHandle != NOTFOUND )
	{
	MODEL_RENDER_DATA*		pxModelData;

		pxModelData = &maxModelRenderData[ mnModelHandle ];
		if ( pxModelData->pxBaseMesh )
		{
		int		nNumVerts = ModelGetStats(mnModelHandle)->nNumVertices;
		int		nNumFaces = ModelGetStats(mnModelHandle)->nNumIndices/3;

			mpbVertexSelections = (BYTE*)malloc( nNumVerts * sizeof(BYTE) );
			mpbFaceSelections = (BYTE*)malloc( nNumFaces * sizeof(BYTE) );
			memset( mpbVertexSelections, 0, nNumVerts * sizeof(BYTE) );
			memset( mpbFaceSelections, 0, nNumFaces * sizeof(BYTE) );
			mnNumFacesSelected = 0;
			mnNumVerticesSelected = 0;

//			if ( (nNumFaces * 3) < (65000 )
//			{
				mhFaceBuffer = EngineCreateVertexBuffer( nNumFaces*3, TRIANGLE_LIST, "CSceneObject FaceBuffer" );
				UpdateFaceMeshList();
//			}
		}
	}
}

void	CSceneObject::ResetFaceList()
{
int		nNumFaces = ModelGetStats(mnModelHandle)->nNumIndices/3;

	EngineVertexBufferFree( mhFaceBuffer );
//	if ( (nNumFaces * 3) < 65000 )
	//{
		mhFaceBuffer = EngineCreateVertexBuffer( nNumFaces*3, TRIANGLE_LIST, "CScene Object FaceBuffer" );
		UpdateFaceMeshList();
//	}
}


void	CSceneObject::OnModelChanged( BOOL bNewVerts )
{
	if ( bNewVerts )
	{
		// Reallocate and update face list
		ReleaseDX();
		InitDX();
	}
	else
	{
		UpdateFaceMeshList();
	}
}


void	CSceneObject::ReleaseDX()
{
	if ( mhTexture != NOTFOUND )
	{
		EngineReleaseTexture( &mhTexture );
	}
	if ( mhFaceBuffer != NOTFOUND )
	{
		EngineVertexBufferFree( mhFaceBuffer );
		mhFaceBuffer = NOTFOUND;
	}
}

void		CSceneObject::ReleaseTexture( void )
{
	if ( mhTexture != NOTFOUND )
	{
		EngineReleaseTexture( &mhTexture );
	}
}

void	CSceneObject::SetTexture( TEXTURE_HANDLE hTexture)
{
	if ( mhTexture != NOTFOUND )
	{
		EngineReleaseTexture( &mhTexture );
	}
	mhTexture = hTexture;
}

void	CSceneObject::Reset( void )
{
	ReleaseDX();

	ModelFree( mnModelHandle );
	mnModelHandle = NOTFOUND;
	if ( mpbVertexSelections )
	{
		free( (void*)mpbVertexSelections );
		mnNumVerticesSelected = 0;
		mpbVertexSelections = NULL;
	}
	if ( mpbFaceSelections )
	{
		free( (void*)mpbFaceSelections );
		mnNumFacesSelected = 0;
		mpbFaceSelections = NULL;
	}

}


void	CSceneObject::SetModel( int nModelHandle)
{
	if ( mnModelHandle != NOTFOUND )
	{ 
		Reset();
	}
	mnModelHandle = nModelHandle;
	InitDX();
}



void	CSceneObject::ApplyMappingListUVs( MAPPING_LIST* pMappingList, int nNumMaps )
{
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVert;
int		nLoop;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer(0, (byte**)( &pxVertices ) );

		if ( pxVertices )
		{
			for ( nLoop = 0; nLoop < nNumMaps; nLoop++ )
			{
				pxVert = pxVertices + pMappingList->nVertIndex1;
				pxVert->tu = pMappingList->u1;
				pxVert->tv = pMappingList->v1;
				pxVert = pxVertices + pMappingList->nVertIndex2;
				pxVert->tu = pMappingList->u2;
				pxVert->tv = pMappingList->v2;
				pxVert = pxVertices + pMappingList->nVertIndex3;
				pxVert->tu = pMappingList->u3;
				pxVert->tv = pMappingList->v3;

				pMappingList++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
}

TEXTURE_HANDLE		CSceneObject::GetTextureHandle( int materialAttrib )
{
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];

	if ( pxModelData->pMaterialData )
	{
	ModelMaterialData*	pMaterial = FindMaterial( pxModelData, materialAttrib );

		if ( pMaterial )
		{
			return( pMaterial->GetTexture( DIFFUSEMAP ) );
		}
	}
	return( mhTexture );

}



//--------------------------------------------------------------------------------------------------------
void	CSceneObject::GetSelectedMappingList( MAPPING_LIST* pMappingList )
{
int		nLoop;
BYTE*	pbSelections = mpbFaceSelections;
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];
CUSTOMVERTEX*	pxVertices;
ushort*			puwIndices;
unsigned int*	punIndices;
CUSTOMVERTEX*	pxVert1;
CUSTOMVERTEX*	pxVert2;
CUSTOMVERTEX*	pxVert3;
unsigned int	unIndex1, unIndex2, unIndex3;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );

		if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
		{
			pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &punIndices ) );
		}
		else
		{
			pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );
		}

		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumIndices; nLoop += 3 )
		{
			if ( *pbSelections )
			{
				if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
				{
					unIndex1 = punIndices[ nLoop ];
					unIndex2 = punIndices[ nLoop+1 ];
					unIndex3 = punIndices[ nLoop+2 ];
				}
				else
				{
					unIndex1 = puwIndices[ nLoop ];
					unIndex2 = puwIndices[ nLoop+1 ];
					unIndex3 = puwIndices[ nLoop+2 ];
				}
				pxVert1 = pxVertices + unIndex1;
				pxVert2 = pxVertices + unIndex2;
				pxVert3 = pxVertices + unIndex3;

				pMappingList->nFaceNum = nLoop/3;
				pMappingList->nVertIndex1 = unIndex1;
				pMappingList->nVertIndex2 = unIndex2;
				pMappingList->nVertIndex3 = unIndex3;
				pMappingList->u1 = pxVert1->tu;
				pMappingList->u2 = pxVert2->tu;
				pMappingList->u3 = pxVert3->tu;
				pMappingList->v1 = pxVert1->tv;
				pMappingList->v2 = pxVert2->tv;
				pMappingList->v3 = pxVert3->tv;
				pMappingList->nFlags = 0;
				pMappingList++;
			}
			pbSelections++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
		pxModelData->pxBaseMesh->UnlockIndexBuffer();
	}

}

int		CSceneObject::GetSelectionMaterial( void )
{
int		nAttribID = -1;
int		nNewAttrib;
int		nLoop;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
BYTE*	pbSelections = mpbFaceSelections;
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];
DWORD*		pxMeshAttributes = NULL;

	if ( ( pbSelections ) &&
		 ( pxModelData->pMaterialData ) )
	{
		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_ReadOnly, (BYTE**)&pxMeshAttributes );
			if ( pxMeshAttributes )
			{
				for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
				{ 
					if ( *pbSelections )
					{
						nNewAttrib = (int)*pxMeshAttributes;
						if ( nAttribID == -1 )
						{
							nAttribID = nNewAttrib;
						}
						else if ( nNewAttrib != nAttribID )
						{
							// Multiple materials selected.. return -1
							return( -2 );
						}
					}
					pxMeshAttributes++;
					pbSelections++;
				}
				pxModelData->pxBaseMesh->UnlockAttributeBuffer();
			}
		}
	}
	return( nAttribID );
}

void	CSceneObject::ApplyMaterialToSelectedFaces( int attribID )
{
int		nLoop;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
BYTE*	pbSelections = mpbFaceSelections;
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];
DWORD*		pxMeshAttributes = NULL;

	if ( ( pbSelections ) &&
		 ( pxModelData->pMaterialData ) )
	{
		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_Normal, (BYTE**)&pxMeshAttributes );
			if ( pxMeshAttributes )
			{
				for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
				{ 
					if ( *pbSelections )
					{
						*pxMeshAttributes = (DWORD)(attribID);
					}
					pxMeshAttributes++;
					pbSelections++;
				}
				pxModelData->pxBaseMesh->UnlockAttributeBuffer();
			}
		}
	}	
}


void	CSceneObject::MoveSelected( VECT* pxMove )
{
int		nLoop;
int		nNumVertices = ModelGetStats( mnModelHandle )->nNumVertices;
int		nStride;
VECT*	pxVertices = ModelGetVertexList( mnModelHandle, &nStride );
BYTE*	pbSelections = mpbVertexSelections;

	if ( ( pxVertices ) &&
		 ( pbSelections ) )
	{
		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			if ( *pbSelections )
			{
				VectAdd( pxVertices, pxVertices, pxMove );
			}
			pxVertices = (VECT*)( (byte*)(pxVertices) + nStride );
			pbSelections++;
		}
	}
	ModelStoreVertexList(mnModelHandle);
}





void	CSceneObject::SelectVertexAtScreenPoint( int X, int Y )
{
BYTE*	pbSelections = mpbVertexSelections;
int		nNumVertices = ModelGetStats( mnModelHandle )->nNumVertices;
int		nStride;
VECT*	pxVertices = ModelGetVertexList( mnModelHandle, &nStride );
VECT	xRayStart;
VECT	xRayDir;
VECT	xRayEnd;
VECT	xHit;
VECT	xHitNormal;
int		nFaceHit = 0;
BOOL	bShiftHeld = FALSE;
BOOL	bCtrlHeld = FALSE;
int		nLoop;
float	fDist;
float	fClosestDist = 999999.0f;
int		nClosestVert = NOTFOUND;

	if ( SysCheckKeyState( KEY_SHIFT ) )
	{
		bShiftHeld = TRUE;
	}
	else if ( SysCheckKeyState( KEY_CTRL ) )
	{
		bCtrlHeld = TRUE;
	}

	if ( ( !bShiftHeld ) &&
		 ( !bCtrlHeld ) )
	{
		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			pbSelections[nLoop] = 0;
			mnNumVerticesSelected = 0;
		}
	}

	mpViewInterface->GetRayForScreenCoord( X, Y, &xRayStart, &xRayDir );
	xRayEnd = xRayDir;
	VectNormalize( &xRayEnd );
	VectScale( &xRayEnd, &xRayEnd, 100000.0f );
	VectAdd( &xRayEnd, &xRayEnd, &xRayStart );
	if ( ModelRayTest( mnModelHandle, &mxPos, &mxRot, &xRayStart, &xRayEnd, &xHit, &xHitNormal, &nFaceHit, 0 ) == TRUE )
	{
		pxVertices = ModelGetVertexList( mnModelHandle, &nStride );

		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			fDist = VectDist( pxVertices, &xHit );
			if ( fDist < fClosestDist )
			{
				fClosestDist = fDist;
				nClosestVert = nLoop;
			}		
			pxVertices = (VECT*)( (byte*)(pxVertices) + nStride );
		}

		ModelUnlockVertexBuffer( mnModelHandle );
		if ( nClosestVert != NOTFOUND )
		{
			if ( bCtrlHeld )
			{
				if ( pbSelections[nClosestVert] == 1 )
				{
					mnNumVerticesSelected--;
					pbSelections[nClosestVert] = 0;
				}
			}
			else if ( pbSelections[nClosestVert] == 0 )
			{
				mnNumVerticesSelected++;
				pbSelections[nClosestVert] = 1;
			}
		}
	}

	if ( mnNumVerticesSelected == 1 )
	{
	int		nSelectedVertexNum = GetSingleSelectedVertex();

 		ModelConvTextBoxSet( "1 vertex selected\n (Vertex number %d)\n", nSelectedVertexNum );
	}
	else
	{
		ModelConvTextBoxSet( "%d vertices selected\n", mnNumVerticesSelected );
	}
	ModelUnlockVertexBuffer( mnModelHandle );

}


void	CSceneObject::SubdivideSelectedFaces( void )
{
	// TODO

}

void	CSceneObject::CopySelectedFaces( EngineMesh* pxNewMesh )
{
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];
BYTE*	pbSelections = mpbFaceSelections;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
int		nLoop;
CUSTOMVERTEX*		pxOldVertices;
CUSTOMVERTEX*		pxNewVertices;
ushort*				puwOldIndices;
ushort*				puwNewIndices;
unsigned int		unNewIndexCount = 0;
CUSTOMVERTEX		xOldVert;

	// TODO - Support 32bit index buffers here

	// TODO
	pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwOldIndices ) );
	pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxOldVertices ) );

	pxNewMesh->LockVertexBuffer( NULL, (byte**)( &pxNewVertices ) );
	pxNewMesh->LockIndexBuffer( NULL, (byte**)( &puwNewIndices ) );

	if ( mpbFaceSelections )
	{
		for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
		{
			if ( pbSelections[nLoop] == 1 )
			{
				puwNewIndices[unNewIndexCount] = unNewIndexCount;
				puwNewIndices[unNewIndexCount+1] = unNewIndexCount+1;
				puwNewIndices[unNewIndexCount+2] = unNewIndexCount+2;

				xOldVert = pxOldVertices[ puwOldIndices[ nLoop*3 ] ];
				pxNewVertices[unNewIndexCount] = xOldVert;
				xOldVert = pxOldVertices[ puwOldIndices[ (nLoop*3)+1 ] ];
				pxNewVertices[unNewIndexCount+1] = xOldVert;
				xOldVert = pxOldVertices[ puwOldIndices[ (nLoop*3)+2 ] ];
				pxNewVertices[unNewIndexCount+2] = xOldVert;
				
				unNewIndexCount += 3;
			}

		}
	}

	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	pxModelData->pxBaseMesh->UnlockVertexBuffer();

	pxNewMesh->UnlockIndexBuffer();
	pxNewMesh->UnlockVertexBuffer();
}

void	CSceneObject::SetSelectedFaces( byte* pbSelectedFaceList )
{
BYTE*	pbSelections = mpbFaceSelections;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
int		nLoop;

	if ( mpbFaceSelections )
	{
		mnNumFacesSelected = 0;
		for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
		{
			if ( pbSelectedFaceList[nLoop] == 1 )
			{
				pbSelections[nLoop] = 1;
				mnNumFacesSelected++;
			}
			else
			{
				pbSelections[nLoop] = 0;
			}
		}
	}
}


int		CSceneObject::GetSelectedFace( int nIndex )
{
BYTE*	pbSelections = mpbFaceSelections;
int		nLoop;
int		nCount = 0;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;

	// Clear previous selection
	for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
	{
		if ( pbSelections[nLoop] == 1 )
		{
			if ( nCount == nIndex )
			{
				return( nLoop );
			}
			nCount++;
		}
	}
	return( NOTFOUND );
}

int		CSceneObject::GetSingleSelectedFace( void )
{
BYTE*	pbSelections = mpbFaceSelections;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
int		nLoop;

	if ( ( mpbFaceSelections ) &&
		 ( mnNumFacesSelected == 1 ) )
	{
		// Clear previous selection
		for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
		{
			if ( pbSelections[nLoop] == 1 )
			{
				return( nLoop );
			}
		}
	}
	return( NOTFOUND );
}

void	CSceneObject::InvertSelection( void )
{
BYTE*	pbSelections = mpbFaceSelections;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
int		nLoop;

	if ( mpbFaceSelections )
	{
		// Clear previous selection
		for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
		{
			if ( pbSelections[nLoop] == 0 )
			{
				pbSelections[nLoop] = 1;
			}
			else
			{
				pbSelections[nLoop] = 0;
			}
		}
		mnNumFacesSelected = nNumFaces - mnNumFacesSelected;
		ModelConvTextBoxSet( "%d faces selected\n", mnNumFacesSelected );
	}
}

void	CSceneObject::SelectFacesByMaterial( int attribID )
{
BYTE*	pbSelections = mpbFaceSelections;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];
DWORD*		pxMeshAttributes = NULL;
int		nLoop;

	if ( mpbFaceSelections )
	{
		// Clear previous selection
		for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
		{
			pbSelections[nLoop] = 0;
			mnNumFacesSelected = 0;
		}

		if ( ( pbSelections ) &&
			 ( pxModelData->pMaterialData ) )
		{
			if ( pxModelData->pxBaseMesh != NULL )
			{
				pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_Discard, (BYTE**)&pxMeshAttributes );
				if ( pxMeshAttributes )
				{
					for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
					{ 
						if ( *pxMeshAttributes == (DWORD)attribID )
						{
							*pbSelections = 1;
							mnNumFacesSelected++;
						}
						pxMeshAttributes++;
						pbSelections++;
					}
					pxModelData->pxBaseMesh->UnlockAttributeBuffer();
				}
			}
		}	

		ModelConvTextBoxSet( "%d faces selected\n", mnNumFacesSelected );
	}
}

void	CSceneObject::SelectFaceAtScreenPoint( int X, int Y )
{
BYTE*	pbSelections = mpbFaceSelections;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;

	if ( ( pbSelections ) &&
		 ( mpViewInterface ) )
	{
	VECT	xRayStart;
	VECT	xRayDir;
	VECT	xRayEnd;
	VECT	xHit;
	VECT	xHitNormal;
	int		nFaceHit = 0;
	BOOL	bShiftHeld = FALSE;
	BOOL	bCtrlHeld = FALSE;

		if ( SysCheckKeyState( KEY_SHIFT ) )
		{
			bShiftHeld = TRUE;
		}
		else if ( SysCheckKeyState( KEY_CTRL ) )
		{
			bCtrlHeld = TRUE;
		}

		if ( ( !bShiftHeld ) &&
			 ( !bCtrlHeld ) )
		{
		int		nLoop;

			for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
			{
				pbSelections[nLoop] = 0;
				mnNumFacesSelected = 0;
			}
		}

		mpViewInterface->GetRayForScreenCoord( X, Y, &xRayStart, &xRayDir );
		xRayEnd = xRayDir;
		VectNormalize( &xRayEnd );
		VectScale( &xRayEnd, &xRayEnd, 100000.0f );
		VectAdd( &xRayEnd, &xRayEnd, &xRayStart );
		if ( ModelRayTest( mnModelHandle, &mxPos, &mxRot, &xRayStart, &xRayEnd, &xHit, &xHitNormal, &nFaceHit, 0 ) == TRUE )
		{
			if ( ( nFaceHit >= 0 ) &&
				 ( nFaceHit < nNumFaces ) )
			{
				if ( pbSelections[nFaceHit] == 1 )
				{
					pbSelections[nFaceHit] = 0;
					mnNumFacesSelected--;
				}
				else
				{
					if ( !bCtrlHeld )
					{
						pbSelections[nFaceHit] = 1;
						mnNumFacesSelected++;

						if ( mnNumFacesSelected == 1 )
						{
						char	acString[1024];
						char	acBuffer[256];
						VECT	xVertPos;
						ENGINEBUFFERVERTEX	xVertex;

							if ( EngineVertexBufferLock( mhFaceBuffer, FALSE ) )
							{
								sprintf( acString, "Face selected with verts at:\n" );
								EngineVertexBufferGet( mhFaceBuffer, nFaceHit*3, &xVertex );
								xVertPos = xVertex.position;
								sprintf( acBuffer, "(1) %.5f %.5f %.5f\n", xVertPos.x, xVertPos.y, xVertPos.z );
								strcat( acString, acBuffer );
								EngineVertexBufferGet( mhFaceBuffer, (nFaceHit*3)+1, &xVertex );
								xVertPos = xVertex.position;
								sprintf( acBuffer, "(2) %.5f %.5f %.5f\n", xVertPos.x, xVertPos.y, xVertPos.z );
								strcat( acString, acBuffer );
								EngineVertexBufferGet( mhFaceBuffer, (nFaceHit*3)+2, &xVertex );
								xVertPos = xVertex.position;
								sprintf( acBuffer, "(3) %.5f %.5f %.5f\n", xVertPos.x, xVertPos.y, xVertPos.z );
								strcat( acString, acBuffer );
								EngineVertexBufferUnlock( mhFaceBuffer );
								ModelConvTextBoxSet( acString );
							}
						}
						else
						{
							ModelConvTextBoxSet( "%d faces selected\n", mnNumFacesSelected );
						}

					}
				}
			}
		}
	}
}



void	CSceneObject::DeleteSelectedFaces( void )
{
BYTE*	pbSelections = mpbFaceSelections;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
int		nNumVerts = ModelGetStats( mnModelHandle )->nNumVertices;
int		nLoop;
int		nNewNumFaces = nNumFaces - mnNumFacesSelected;
EngineMesh*		pNewMesh = NULL;
ushort*		puwNewIndices;
ushort*		puwOldIndices;
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];
CUSTOMVERTEX*	pxOldVertices = NULL;
CUSTOMVERTEX*	pxNewVertices = NULL;

	EngineCreateMesh( nNewNumFaces, nNumVerts, &pNewMesh, 0 );

	if ( ( pNewMesh ) &&
		 ( pxModelData->pxBaseMesh != NULL ) )
	{
		//******************* TODO!! **********************************
		// Needs to support 32 bit index buffers 
		//***********************************************************

		pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwOldIndices ) );
		pNewMesh->LockIndexBuffer( NULL, (byte**)( &puwNewIndices ) );

		if ( puwNewIndices != NULL )
		{
			for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
			{
				if ( pbSelections[nLoop] == 1 )
				{
					// Face is selected, so we wouldn't include it..	
					puwOldIndices += 3;
				}
				else
				{
					// Face is not selected so we would include it..
					*(puwNewIndices++) = *(puwOldIndices++);
					*(puwNewIndices++) = *(puwOldIndices++);
					*(puwNewIndices++) = *(puwOldIndices++);
				}
			}
		}
		pNewMesh->UnlockIndexBuffer();
		pxModelData->pxBaseMesh->UnlockIndexBuffer();
	
		// Copy all the vertices from the old mesh to the new

		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxOldVertices ) );
		pNewMesh->LockVertexBuffer( NULL, (byte**)( &pxNewVertices ) );
		for ( nLoop = 0; nLoop < nNumVerts; nLoop++ )
		{
			*pxNewVertices++ = *pxOldVertices++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
		pNewMesh->UnlockVertexBuffer();
	}

	// TODO - Should run a process to delete isolated vertices

	// TODO - Sort attribute buffer if it exists...

	pxModelData->pxBaseMesh->Release();
	pxModelData->pxBaseMesh = pNewMesh;

	// Clear face selection
	for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
	{
		pbSelections[nLoop] = 0;
		mnNumFacesSelected = 0;
	}

	ModelConvTextBoxSet( "" );

	pxModelData->xStats.nNumIndices = nNewNumFaces * 3;
	ResetFaceList();
}

void	CSceneObject::FixInsideOutSelectedFaces( void )
{
BYTE*	pbSelections = mpbFaceSelections;
int		nNumFaces = ModelGetStats( mnModelHandle )->nNumIndices/3;
int		nNumVerts = ModelGetStats( mnModelHandle )->nNumVertices;
ushort*		puwIndices;
ushort		uwIndex;
MODEL_RENDER_DATA*		pxModelData = &maxModelRenderData[mnModelHandle];
CUSTOMVERTEX*	pxVertices = NULL;
BYTE*	pbAffectedVertices = (BYTE*)malloc( nNumVerts );

	memset( pbAffectedVertices, 0, nNumVerts );

	pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );

	if ( puwIndices != NULL )
	{
	int nFaceLoop;

		if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
		{
		int*	punIndices;
		int		nIndex;
			
			punIndices = (int*)( puwIndices );
			for ( nFaceLoop = 0; nFaceLoop < nNumFaces; nFaceLoop++ )
			{
				if ( pbSelections[nFaceLoop] == 1 )
				{
					// Swap indices
					nIndex = punIndices[1];
					punIndices[1] = punIndices[2];
					punIndices[2] = nIndex;

					// Flag the verts referenced as needing to have their normals flipped
					pbAffectedVertices[punIndices[0]] = 1;
					pbAffectedVertices[punIndices[1]] = 1;
					pbAffectedVertices[punIndices[2]] = 1;
				}
				punIndices += 3;
			}
		}
		else
		{
			for ( nFaceLoop = 0; nFaceLoop < nNumFaces; nFaceLoop++ )
			{
				// If face is selected
				if ( pbSelections[nFaceLoop] == 1 )
				{
					// Swap indices
					uwIndex = puwIndices[1];
					puwIndices[1] = puwIndices[2];
					puwIndices[2] = uwIndex;

					// Flag the verts referenced as needing to have their normals flipped
					pbAffectedVertices[puwIndices[0]] = 1;
					pbAffectedVertices[puwIndices[1]] = 1;
					pbAffectedVertices[puwIndices[2]] = 1;
				}
				puwIndices += 3;
			}
		}
			
		pxModelData->pxBaseMesh->UnlockIndexBuffer();

		int		nLoop;

		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );
		for ( nLoop = 0; nLoop < nNumVerts; nLoop++ )
		{
			if ( pbAffectedVertices[nLoop] == 1 )
			{
				pxVertices->normal.x *= -1.0f;
				pxVertices->normal.y *= -1.0f;
				pxVertices->normal.z *= -1.0f;
			}
			pxVertices++;

		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}

	free( pbAffectedVertices );
}


void	CSceneObject::SelectAllFacesInScreenRegion( int X, int Y, int W, int H )
{
BYTE*	pbSelections = mpbFaceSelections;
MODEL_RENDER_DATA*		pxModelData;
int		nNumVerts = ModelGetStats(mnModelHandle)->nNumVertices;
int		nNumFaces = ModelGetStats(mnModelHandle)->nNumIndices/3;
int		nLoop;
int		X2 = X+W;
int		Y2 = Y+H;
ENGINEBUFFERVERTEX	xVertex1;
ENGINEBUFFERVERTEX	xVertex2;
ENGINEBUFFERVERTEX	xVertex3;
VECT			xMidVertex;
CUSTOMVERTEX*	pxVert1;
CUSTOMVERTEX*	pxVert2;
CUSTOMVERTEX*	pxVert3;
CUSTOMVERTEX*	pxVertices;
ushort*			puwIndices;
unsigned int*	punIndices;
int				nVertexX, nVertexY;
int				nFaceNum;
BOOL	bShiftHeld = FALSE;
BOOL	bCtrlHeld = FALSE;
BOOL	bIsFaceSelected = FALSE;

	if ( SysCheckKeyState( KEY_SHIFT ) )
	{
		bShiftHeld = TRUE;
	}
	else if ( SysCheckKeyState( KEY_CTRL ) )
	{
		bCtrlHeld = TRUE;
	}

	if ( ( !bShiftHeld ) &&
		 ( !bCtrlHeld ) )
	{
	int		nLoop;
		for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
		{
			pbSelections[nLoop] = 0;
			mnNumFacesSelected = 0;
		}
	}

	pxModelData = &maxModelRenderData[ mnModelHandle ];

	pxModelData->pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );

	if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
	{
		pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &punIndices ) );
	}
	else
	{
		pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );
	}

	nFaceNum = 0;
	for ( nLoop = 0; nLoop < nNumFaces*3; nLoop += 3 )
	{
		// For each tri in the other model, first check the tri box against the model1 overall bounding box
		if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
		{
			pxVert1 = pxVertices + punIndices[ nLoop ];
			pxVert2 = pxVertices + punIndices[ nLoop+1 ];
			pxVert3 = pxVertices + punIndices[ nLoop+2 ];
		}
		else
		{
			pxVert1 = pxVertices + puwIndices[ nLoop ];
			pxVert2 = pxVertices + puwIndices[ nLoop+1 ];
			pxVert3 = pxVertices + puwIndices[ nLoop+2 ];
		}
		xVertex1 = *((ENGINEBUFFERVERTEX*)(pxVert1));
		xVertex2 = *((ENGINEBUFFERVERTEX*)(pxVert2));
		xVertex3 = *((ENGINEBUFFERVERTEX*)(pxVert3));
		
		VectAdd( &xMidVertex, (VECT*)&xVertex1, (VECT*)&xVertex2 );
		VectAdd( &xMidVertex, &xMidVertex, (VECT*)&xVertex3 );
		VectScale( &xMidVertex, &xMidVertex, 0.33333333f );

		bIsFaceSelected = FALSE;

		mpViewInterface->GetScreenCoordForWorldCoord( (VECT*)&xVertex1, &nVertexX, &nVertexY );
		if ( ( nVertexX >= X ) &&
			 ( nVertexX <= X2 ) &&
			 ( nVertexY >= Y ) &&
			 ( nVertexY <= Y2 ) )
		{
			bIsFaceSelected = TRUE;
		}
		else
		{
			mpViewInterface->GetScreenCoordForWorldCoord( (VECT*)&xVertex2, &nVertexX, &nVertexY );
			if ( ( nVertexX >= X ) &&
				 ( nVertexX <= X2 ) &&
				 ( nVertexY >= Y ) &&
				 ( nVertexY <= Y2 ) )
			{
				bIsFaceSelected = TRUE;
			}
			else
			{
				mpViewInterface->GetScreenCoordForWorldCoord( (VECT*)&xVertex3, &nVertexX, &nVertexY );
				if ( ( nVertexX >= X ) &&
					 ( nVertexX <= X2 ) &&
					 ( nVertexY >= Y ) &&
					 ( nVertexY <= Y2 ) )
				{
					bIsFaceSelected = TRUE;
				}
				else
				{
					mpViewInterface->GetScreenCoordForWorldCoord( (VECT*)&xMidVertex, &nVertexX, &nVertexY );
					if ( ( nVertexX >= X ) &&
						 ( nVertexX <= X2 ) &&
						 ( nVertexY >= Y ) &&
						 ( nVertexY <= Y2 ) )
					{
						bIsFaceSelected = TRUE;
					}
				}
			}
		}

		if ( bIsFaceSelected )
		{
			// One or more of the 3 verts + midpoint is within the selection rect so select this face
			if ( pbSelections[nFaceNum] == 1 )
			{
				if ( !bShiftHeld )
				{
					pbSelections[nFaceNum] = 0;
					mnNumFacesSelected--;
				}
			}
			else if ( !bCtrlHeld )
			{
				pbSelections[nFaceNum] = 1;
				mnNumFacesSelected++;
			}
		}
		nFaceNum++;
	}
	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	pxModelData->pxBaseMesh->UnlockVertexBuffer();
	ModelConvTextBoxSet( "%d faces selected\n", mnNumFacesSelected );
}

int		CSceneObject::GetFirstSelectedVertex( void )
{
int		nLoop;
int		nNumVertices = ModelGetStats( mnModelHandle )->nNumVertices;

	if ( mpbVertexSelections )
	{
		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			if ( mpbVertexSelections[nLoop] == 1 )
			{
				return( nLoop );
			}
		}
	}
	return( NOTFOUND );
}

int		CSceneObject::GetSingleSelectedVertex( void )
{
int		nLoop;
int		nNumVertices = ModelGetStats( mnModelHandle )->nNumVertices;

	if ( ( mpbVertexSelections ) &&
		 ( mnNumVerticesSelected == 1 ) )
	{
		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			if ( mpbVertexSelections[nLoop] == 1 )
			{
				return( nLoop );
			}
		}
	}
	return( NOTFOUND );
}


void	CSceneObject::SelectAllVerticesInScreenRegion( int X, int Y, int W, int H )
{
int		nLoop;
int		nNumVertices = ModelGetStats( mnModelHandle )->nNumVertices;
int		nStride;
VECT*	pxVertices = ModelGetVertexList( mnModelHandle, &nStride );
BYTE*	pbSelections = mpbVertexSelections;
int		X2 = X+W;
int		Y2 = Y+H;
int		nVertexX;
int		nVertexY;
VECT	xWorldPos;
BOOL	bShiftHeld = FALSE;
BOOL	bCtrlHeld = FALSE;
BOOL	bSelectFirstOnly = TRUE;

	if ( ( pxVertices ) &&
		 ( pbSelections ) &&
		 ( mpViewInterface ) )
	{
		if ( SysCheckKeyState( KEY_SHIFT ) )
		{
			bShiftHeld = TRUE;
		}
		else if ( SysCheckKeyState( KEY_CTRL ) )
		{
			bCtrlHeld = TRUE;
			if ( mnNumVerticesSelected == 0 )
			{
				bSelectFirstOnly = TRUE;
			}
		}

		mnNumVerticesSelected = 0;

		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			xWorldPos = *pxVertices;

			mpViewInterface->GetScreenCoordForWorldCoord( (VECT*)&xWorldPos, &nVertexX, &nVertexY );
			if ( ( nVertexX >= X ) &&
				 ( nVertexX <= X2 ) &&
				 ( nVertexY >= Y ) &&
				 ( nVertexY <= Y2 ) )
			{
				if ( bCtrlHeld ) 
				{
					if ( bSelectFirstOnly == TRUE )
					{
						*pbSelections = 1;
						mnNumVerticesSelected++;
						break;
					}
					*pbSelections = 0;
				}
				else
				{
					*pbSelections = 1;
					mnNumVerticesSelected++;
				}
			}
			else
			{
				if ( ( !bCtrlHeld ) &&
					 ( !bShiftHeld ) )
				{
					*pbSelections = 0;
				}

				if ( *pbSelections == 1 )
				{
					mnNumVerticesSelected++;
				}
			}

			pxVertices = (VECT*)( (byte*)(pxVertices) + nStride );
			pbSelections++;
		}
	}

	if ( mnNumVerticesSelected == 1 )
	{
	int		nSelectedVertexNum = GetSingleSelectedVertex();

 		ModelConvTextBoxSet( "1 vertex selected\n (Vertex number %d)\n", nSelectedVertexNum );
	}
	else
	{
		ModelConvTextBoxSet( "%d vertices selected\n", mnNumVerticesSelected );
	}
	ModelUnlockVertexBuffer( mnModelHandle );
}


void	CSceneObject::RenderNormals( VECT* pxPos, VECT* pxRot )
{
//uint32 ulCol;
VECT	xVert1, xVert2;
int		nLoop;
int		nStride;
int		nNumVertices = ModelGetStats( mnModelHandle )->nNumVertices;
	// Get Normal list
VECT*	pxNormals = (VECT*)malloc( sizeof( VECT ) * nNumVertices );
VECT*	pxNormalsInModel = ModelGetNormalList( mnModelHandle, &nStride );

	if ( pxNormalsInModel )
	{
		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			pxNormals[nLoop] = *pxNormalsInModel;
			pxNormalsInModel = (VECT*)( (byte*)(pxNormalsInModel) + nStride );
		}
	}

	ModelStoreVertexList( mnModelHandle );		// Obscure lock/unlock vertex buffer procedure needs tidying

	VECT*	pxVertices = ModelGetVertexList( mnModelHandle, &nStride );

	float	fNormalScale = ModelGetStats( mnModelHandle )->fBoundSphereRadius * 0.1f;

	if (  pxVertices )
	{
		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			xVert1 = *pxVertices;
			xVert2 = pxNormals[nLoop];
			VectNormalize( &xVert2 );
			VectScale( &xVert2, &xVert2, fNormalScale );
			xVert2.x += xVert1.x;
			xVert2.y += xVert1.y;
			xVert2.z += xVert1.z;
			AddLine( &xVert1, &xVert2, 0xF0D04020, 0xF0C03020 );
			pxVertices = (VECT*)( (byte*)(pxVertices) + nStride );
		}
	}
	ModelStoreVertexList(mnModelHandle);

	free( pxNormals );
}

//--------------------------------------------------------------------------------------------------------

void	CSceneObject::RenderVertexPoints( VECT* pxPos, VECT* pxRot )
{
uint32 ulCol;
//VECT	xVertex;
int		nLoop;
int		nNumVertices = ModelGetStats( mnModelHandle )->nNumVertices;
int		nStride;
VECT*	pxVertices = ModelGetVertexList( mnModelHandle, &nStride );
BYTE*	pbSelections = mpbVertexSelections;

	if ( ( pxVertices ) &&
		 ( pbSelections ) )
	{
		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			if ( *pbSelections )
			{
				ulCol = 0xF0F04030;
			}
			else
			{
				ulCol = 0xF0F8F8F8;
			}
			PointRenderAdd( pxVertices, ulCol );
			pxVertices = (VECT*)( (byte*)(pxVertices) + nStride );
			pbSelections++;
		}
	}
	ModelStoreVertexList(mnModelHandle);
}


void	CSceneObject::RenderPolys( VECT* pxPos, VECT* pxRot )
{
ENGINEBUFFERVERTEX	xVertex;
int		nLoop;
BYTE*	pbSelections = mpbFaceSelections;

	if ( ( mhFaceBuffer != NOTFOUND ) &&
		 ( pbSelections ) )
	{
		if ( EngineVertexBufferLock( mhFaceBuffer, FALSE ) )
		{
		int		nNumVerts = ModelGetStats(mnModelHandle)->nNumIndices;

			EngineSetBlendMode( BLEND_MODE_ALPHABLEND );
			EngineEnableLighting(FALSE);
			EngineEnableBlend(TRUE);
			EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
			EngineSetTexture(0,0);
			EngineSetZBias(8);
			if ( mbSelectedFaceHighlight )
			{
				// Render highlight on selected face
				for ( nLoop = 0; nLoop < nNumVerts; nLoop++ )
				{
					EngineVertexBufferGet( mhFaceBuffer, nLoop, &xVertex );
					if ( *pbSelections )
					{
						xVertex.color = 0xD0F02000;
					}
					else
					{
						xVertex.color = 0x10F0F0F0;
					}
					if ( (nLoop % 3) == 2 )
					{
						pbSelections++;
					}
					EngineVertexBufferSet( mhFaceBuffer, nLoop, &xVertex );
				}
				EngineVertexBufferUnlock( mhFaceBuffer );
				
				//		EngineEnableWireframe(1);
				EngineVertexBufferRender( mhFaceBuffer, TRIANGLE_LIST );
			}

			// Render the lines
			if ( EngineVertexBufferLock( mhFaceBuffer, FALSE ) )
			{
				pbSelections = mpbFaceSelections;
				for ( nLoop = 0; nLoop < nNumVerts; nLoop++ )
				{
					EngineVertexBufferGet( mhFaceBuffer, nLoop, &xVertex );
					if ( *pbSelections )
					{
						xVertex.color = 0x40F0F0A0;
					}
					else
					{
						xVertex.color = 0x18F0F0F0;
					}
					EngineVertexBufferSet( mhFaceBuffer, nLoop, &xVertex );
					if ( (nLoop % 3) == 2 )
					{
						pbSelections++;
					}
				}
			}
			EngineVertexBufferUnlock( mhFaceBuffer );

			EngineEnableWireframe(1);
			EngineSetZBias(12);
			EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );

			EngineVertexBufferRender( mhFaceBuffer, TRIANGLE_LIST );
			EngineSetZBias(0);
			EngineEnableWireframe(0);
			EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
		}
	}
}


int		CSceneObject::Render( eRENDER_MODE mode )
{
ENGINEMATRIX	xMatrix;
int		nPolys = 0;

	EngineSetColourMode( 0, COLOUR_MODE_TEXTURE_MODULATE );
	EngineSetTexture(0,mhTexture);

	if ( mbRenderWireframe )
	{
		EngineEnableWireframe(1);
		EngineEnableCulling(0);
		EngineEnableZWrite( FALSE );
		nPolys = ModelRender( mnModelHandle, &mxPos, &mxRot, RENDER_FLAGS_NO_STATE_CHANGE );
		EngineEnableWireframe(0);
		EngineEnableCulling(1);
		EngineEnableZWrite( TRUE );
	}
	else
	{
		nPolys = ModelRender( mnModelHandle, &mxPos, &mxRot, 0 );
	}

 
	if ( ModelShadowsEnabled( mnModelHandle ) )
	{
		//-------------------------------------------------------------------
		// TEMP - For testing only!!!!!!!!!!!!!!
//		ShadowVolumeRenderModel( mnModelHandle, &mxPos, &mxRot, 0, 0 );
		//-------------------------------------------------------------------
	} 

	EngineMatrixIdentity( &xMatrix );
	EngineSetWorldMatrix( &xMatrix );

	// If selecting vertices, draw a point at each vertex of the model
	if ( mode == NORMAL_WITH_VERTEX_POINTS ) // ModelEditorGetSelectionMode() == SELECTION_VERTICES )
	{ 
		// If animated, updated points
		if ( ModelGetStats(mnModelHandle)->nNumVertKeyframes > 0 )
		{
			UpdateFaceMeshList();
		}
		RenderPolys( &mxPos, &mxRot );
		RenderVertexPoints( &mxPos, &mxRot );
	}
	else if ( ( mode == NORMAL_WIREFRAME_FACES ) || //ModelEditorGetSelectionMode() == SELECTION_FACES )
			  ( mode == NORMAL_WIREFRAME_FACES_WITH_SHOWNORMALS ) )
	{
		// If animated, updated points
		if ( ModelGetStats(mnModelHandle)->nNumVertKeyframes > 0 )
		{
			UpdateFaceMeshList();
		}
		RenderPolys( &mxPos, &mxRot );
		if ( mode == NORMAL_WIREFRAME_FACES_WITH_SHOWNORMALS )
		{
			RenderNormals( &mxPos, &mxRot );
		}
	}
	else if ( mode == NORMAL_SHOWNORMALS ) //ModelEditorGetSelectionMode() == SELECTION_FACES )
	{
		// If animated, updated points
		if ( ModelGetStats(mnModelHandle)->nNumVertKeyframes > 0 )
		{
			UpdateFaceMeshList();
		}
		RenderNormals( &mxPos, &mxRot );
	}

	return( nPolys );
}
