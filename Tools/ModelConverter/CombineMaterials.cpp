

#include <StandardDef.h>
#include <System.h>
#include <Engine.h>
#include <Rendering.h>

#include "../LibCode/Engine/ModelFiles.h"
#include "../LibCode/Engine/ModelRendering.h"	
#include "../LibCode/Engine/ModelMaterialData.h"		

#include "Tools/MeshManipulate.h"

class CombinerTextureGrid
{
public:
	CombinerTextureGrid()
	{
		mapSubGrids[0] = NULL;
		mapSubGrids[1] = NULL;
		mapSubGrids[2] = NULL;
		mapSubGrids[3] = NULL;
		mhOccupyingTexture = NOTFOUND;
	}
	
	void		InitGrid( int nSizeW, int nSizeH, int nStartPosX, int nStartPosY )
	{
		mnFullSizeH = nSizeH;
		mnFullSizeW = nSizeW;
		mnStartPosX = nStartPosX;
		mnStartPosY = nStartPosY;
	}

	BOOL		AddTexture( int hTexture, int nTextureW, int nTextureH ); 

	int			mnFullSizeW;
	int			mnFullSizeH;
	int			mhOccupyingTexture;
	int			mnStartPosX;
	int			mnStartPosY;


	CombinerTextureGrid*	mapSubGrids[4];
};



BOOL		CombinerTextureGrid::AddTexture( int hTexture, int nTextureW, int nTextureH )
{
int		nLoop;
CombinerTextureGrid*		pGrid;

	// If we have subgrids
	if ( mapSubGrids[0] != NULL )
	{
		for( nLoop = 0; nLoop < 4; nLoop++ )
		{
			pGrid = mapSubGrids[nLoop];
			if ( ( pGrid ) &&
				 ( pGrid->AddTexture( hTexture, nTextureW, nTextureH ) == TRUE ) )
			{
				return( TRUE );
			}
		}
	}
	// Is a leaf ; if not already occupied..
	else if ( mhOccupyingTexture == NOTFOUND )
	{
		// Does it fit within this grid square
		if ( ( mnFullSizeW >= nTextureW ) &&
			 ( mnFullSizeH >= nTextureH ) )
		{
			// If it fits exactly...
			if ( ( mnFullSizeW == nTextureW ) &&
				 ( mnFullSizeH == nTextureH ) )
			{
				mhOccupyingTexture = hTexture;
			}

			// If it fits and leaves some space free, we need to split this gridsquare
			// and add subgrids..
			int		nGridW;
			int		nGridH;
			int		nSubGridNum = 0;

			mapSubGrids[nSubGridNum] = new CombinerTextureGrid;
			mapSubGrids[nSubGridNum]->InitGrid( nTextureW, nTextureH, mnStartPosX, mnStartPosY );
			mapSubGrids[nSubGridNum]->mhOccupyingTexture = hTexture;
			nSubGridNum++;
			
			nGridW = mnFullSizeW - nTextureW;
			nGridH = mnFullSizeH - nTextureW;

			// Any width leftover?
			if ( nGridW > 0 )
			{
				mapSubGrids[nSubGridNum] = new CombinerTextureGrid;
				mapSubGrids[nSubGridNum]->InitGrid( nGridW, nTextureH, mnStartPosX + nTextureW, mnStartPosY );
				nSubGridNum++;
			}

			// Any height leftover?
			if ( nGridH > 0 )
			{
				mapSubGrids[nSubGridNum] = new CombinerTextureGrid;
				mapSubGrids[nSubGridNum]->InitGrid( nTextureW, nGridH, mnStartPosX, mnStartPosY + nTextureH );
				nSubGridNum++;
			}

			// Space in both directions
			if ( ( nGridW > 0 ) &&
				 ( nGridH > 0 ) )
			{
				mapSubGrids[nSubGridNum] = new CombinerTextureGrid;
				mapSubGrids[nSubGridNum]->InitGrid( nGridW, nGridH, mnStartPosX + nTextureW, mnStartPosY + nTextureH );
				nSubGridNum++;
			}

			return( TRUE );

		}
	}

	return( FALSE );

}

//---------------------------------------------------------------------------


class CombinerUniqueTexture
{
public:
	CombinerUniqueTexture()
	{
		mhTexture = NOTFOUND;
		mnTextureSizeH = 0;
		mnTextureSizeW = 0;
		mfNewUBase = 0.0f;
		mfNewVBase = 0.0f;
		mfNewUScale = 0.0f;
		mfNewVScale = 0.0f;
		mpNext = NULL;
	}
	
	int							mhTexture;
	int							mnTextureSizeW;
	int							mnTextureSizeH;

	float						mfNewUBase;
	float						mfNewVBase;
	float						mfNewUScale;
	float						mfNewVScale;

	CombinerUniqueTexture*		mpNext;
};



CombinerUniqueTexture*		CombinerFindTexture( int hTexture, CombinerUniqueTexture** ppxUniqueTextureList )
{
CombinerUniqueTexture*		pxExistingTextures = *ppxUniqueTextureList;

	while( pxExistingTextures )
	{
		if ( pxExistingTextures->mhTexture == hTexture )
		{
			return( pxExistingTextures );
		}
		pxExistingTextures = pxExistingTextures->mpNext;
	}
	return( NULL );
}

CombinerUniqueTexture*		CombinerGetOrCreateUniqueTexture( int hTexture, CombinerUniqueTexture** ppxUniqueTextureList )
{
CombinerUniqueTexture*		pxExistingTextures = *ppxUniqueTextureList;

	while( pxExistingTextures )
	{
		if ( pxExistingTextures->mhTexture == hTexture )
		{
			return( pxExistingTextures );
		}
		pxExistingTextures = pxExistingTextures->mpNext;
	}

	pxExistingTextures = new CombinerUniqueTexture;
	pxExistingTextures->mhTexture = hTexture;
	pxExistingTextures->mnTextureSizeW = EngineTextureGetWidth( hTexture );
	pxExistingTextures->mnTextureSizeH = EngineTextureGetHeight( hTexture );
	pxExistingTextures->mpNext = *ppxUniqueTextureList;
	*ppxUniqueTextureList = pxExistingTextures;

	return( pxExistingTextures );
}


void	CombinerAddToNewTexture( int hOutTexture, CombinerTextureGrid* pxGrid )
{
	if ( pxGrid )
	{
		if ( pxGrid->mhOccupyingTexture != NOTFOUND )
		{
			// todo - copy the texture into place at the appropriate coordinate
			EngineCopyTextureToPosition( pxGrid->mhOccupyingTexture, hOutTexture, pxGrid->mnStartPosX, pxGrid->mnStartPosY ); 
		}
		else
		{
			CombinerAddToNewTexture( hOutTexture, pxGrid->mapSubGrids[0] );
			CombinerAddToNewTexture( hOutTexture, pxGrid->mapSubGrids[1] );
			CombinerAddToNewTexture( hOutTexture, pxGrid->mapSubGrids[2] );
			CombinerAddToNewTexture( hOutTexture, pxGrid->mapSubGrids[3] );
		}
	}
}

CombinerTextureGrid*	CombinerFindTextureInGrid( CombinerTextureGrid*	pxRootCombiner, int hTexture )
{
int		nLoop;
CombinerTextureGrid*	pxFound;

	if ( pxRootCombiner->mhOccupyingTexture == hTexture )
	{
		return( pxRootCombiner );
	}

	for( nLoop = 0; nLoop < 4; nLoop++ )
	{
		if ( pxRootCombiner->mapSubGrids[nLoop] )
		{
			pxFound = CombinerFindTextureInGrid( pxRootCombiner->mapSubGrids[nLoop], hTexture );

			if ( pxFound )
			{
				return( pxFound );
			}
		}
	}
	return( NULL );

}

void	CombinerCalcUVOffsets( CombinerTextureGrid*	pxRootCombiner, CombinerUniqueTexture** ppxUniqueTextureList )
{
float		fFullSizeW;
float			fFullSizeH;
CombinerUniqueTexture*		pxUniqueTextures = *ppxUniqueTextureList;
CombinerTextureGrid*		pxGrid;

	fFullSizeW = (float)pxRootCombiner->mnFullSizeW;
	fFullSizeH = (float)pxRootCombiner->mnFullSizeH;
	
	while( pxUniqueTextures )
	{
		pxGrid = CombinerFindTextureInGrid( pxRootCombiner, pxUniqueTextures->mhTexture );

		if ( pxGrid )
		{
			pxUniqueTextures->mfNewUBase = (float)pxGrid->mnStartPosX / fFullSizeW;
			pxUniqueTextures->mfNewVBase = (float)pxGrid->mnStartPosY / fFullSizeH;
			pxUniqueTextures->mfNewUScale = pxUniqueTextures->mnTextureSizeW / fFullSizeW;
			pxUniqueTextures->mfNewVScale = pxUniqueTextures->mnTextureSizeH / fFullSizeH;		
		}
		
		pxUniqueTextures = pxUniqueTextures->mpNext;
	}


}


void	CombinerRemapUVs( MODEL_RENDER_DATA* pxModelData, CombinerTextureGrid*	pxRootCombiner, CombinerUniqueTexture** ppxUniqueTextureList )
{
CUSTOMVERTEX*		pxVertices;
CUSTOMVERTEX*		pxVert1;
CUSTOMVERTEX*		pxVert2;
CUSTOMVERTEX*		pxVert3;
ushort*			puwIndices = NULL;
ulong*			punIndices = NULL;
int				nFaceLoop;
int				nNumFaces;
//CUSTOMVERTEX*		pxVertex;
ModelMaterialData*	pxMaterialData;
DWORD*			pxAttributes;
int				nMaterialAttrib;
CombinerUniqueTexture*		pxCombinerTexture;

	nNumFaces = pxModelData->xStats.nNumIndices / 3;

	if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
	{
		pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &punIndices ) );
	}
	else
	{
		pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
	}
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_ReadOnly, (byte**)( &pxAttributes ) );
		
	for ( nFaceLoop = 0; nFaceLoop < nNumFaces; nFaceLoop++ )
	{
		nMaterialAttrib = *pxAttributes++;
		pxMaterialData = FindMaterial( pxModelData, nMaterialAttrib );

		if ( pxMaterialData )
		{
			pxCombinerTexture = CombinerFindTexture( pxMaterialData->GetTexture( 0 ), ppxUniqueTextureList );

			if ( pxCombinerTexture )
			{
				if ( punIndices )
				{
					pxVert1 = pxVertices + punIndices[0];
					pxVert2 = pxVertices + punIndices[1];
					pxVert3 = pxVertices + punIndices[2];
					punIndices += 3;
				}
				else
				{
					pxVert1 = pxVertices + puwIndices[0];
					pxVert2 = pxVertices + puwIndices[1];
					pxVert3 = pxVertices + puwIndices[2];
					puwIndices += 3;
				}

				pxVert1->tu = (pxVert1->tu * pxCombinerTexture->mfNewUScale) + pxCombinerTexture->mfNewUBase;
				pxVert1->tv = (pxVert1->tv * pxCombinerTexture->mfNewVScale) + pxCombinerTexture->mfNewVBase;
				pxVert2->tu = (pxVert2->tu * pxCombinerTexture->mfNewUScale) + pxCombinerTexture->mfNewUBase;
				pxVert2->tv = (pxVert2->tv * pxCombinerTexture->mfNewVScale) + pxCombinerTexture->mfNewVBase;
				pxVert3->tu = (pxVert3->tu * pxCombinerTexture->mfNewUScale) + pxCombinerTexture->mfNewUBase;
				pxVert3->tv = (pxVert3->tv * pxCombinerTexture->mfNewVScale) + pxCombinerTexture->mfNewVBase;
			}
		}
		
	}

	pxModelData->pxBaseMesh->UnlockAttributeBuffer();
	pxModelData->pxBaseMesh->UnlockVertexBuffer();
	pxModelData->pxBaseMesh->UnlockIndexBuffer();

}

void	ModelConvReduceToSingleMaterial( int nModelHandle )
{
MODEL_RENDER_DATA*		pxModelData;
int		nNumMaterials;
//int		nNumUniqueTextures;
int		nLoop;
ModelMaterialData*		pxMaterialData;
ModelMaterialData*		pxBaseMaterial;
CombinerUniqueTexture*		pxUniqueTexture;
CombinerUniqueTexture*		pxUniqueTextureList = NULL;
int		hTexture;
int		hNewTexture;
CombinerTextureGrid*		pxRootCombiner;

	// First, separate the verts (as we don't want to modify UVs on shared verts)
	ModelConvSeparateVerts( nModelHandle );

	pxModelData = &maxModelRenderData[ nModelHandle ];

	nNumMaterials = pxModelData->xStats.nNumMaterials;

	pxMaterialData = pxModelData->pMaterialData;

	// Loop through all the textures assigned and get their sizes
	for( nLoop = 0; nLoop < nNumMaterials; nLoop++ )
	{
		hTexture = pxMaterialData->GetTexture( 0 );
		pxUniqueTexture = CombinerGetOrCreateUniqueTexture( hTexture, &pxUniqueTextureList );

		pxMaterialData = pxMaterialData->GetNext();
	}

	if ( pxUniqueTextureList )
	{
	int		nFirstTextureW;
	int		nFirstTextureH;

		nFirstTextureW = pxUniqueTextureList->mnTextureSizeW;
		nFirstTextureH = pxUniqueTextureList->mnTextureSizeH;
		
		pxRootCombiner = new CombinerTextureGrid;
	
		pxRootCombiner->InitGrid( nFirstTextureW, nFirstTextureH, 0, 0 );
		pxRootCombiner->mhOccupyingTexture = pxUniqueTextureList->mhTexture;

		pxUniqueTexture = pxUniqueTextureList->mpNext;

		// Calculate the size for a new texture that contains all the existing ones
		//  (Ideally something square...)
		while( pxUniqueTexture )
		{
			// If no empty grids left..
			if ( pxRootCombiner->AddTexture( pxUniqueTexture->mhTexture, pxUniqueTexture->mnTextureSizeW, pxUniqueTexture->mnTextureSizeH ) == FALSE )
			{
			CombinerTextureGrid*		pxNewRootCombiner;
			int		nRightStartX = pxRootCombiner->mnStartPosX + pxRootCombiner->mnFullSizeW;
				
				// we need to expand the size of the root grid/texture
				pxNewRootCombiner = new CombinerTextureGrid;
				pxNewRootCombiner->mapSubGrids[0] = pxRootCombiner;

				pxNewRootCombiner->mnFullSizeW = pxRootCombiner->mnFullSizeW + pxUniqueTexture->mnTextureSizeW;
				pxNewRootCombiner->mnStartPosX = 0;
				pxNewRootCombiner->mnStartPosY = 0;

				// Add the new texture as a subgrid to the right
				pxNewRootCombiner->mapSubGrids[1] = new CombinerTextureGrid;
				pxNewRootCombiner->mapSubGrids[1]->InitGrid( pxUniqueTexture->mnTextureSizeW, pxUniqueTexture->mnTextureSizeH, nRightStartX, pxRootCombiner->mnStartPosY );
				pxNewRootCombiner->mapSubGrids[1]->mhOccupyingTexture = pxUniqueTexture->mhTexture;

				// If existing height is big enough to include new texture, we just add grid(s) to the right and its job done
				if ( pxRootCombiner->mnFullSizeH >= pxUniqueTexture->mnTextureSizeH )
				{
					pxNewRootCombiner->mnFullSizeH = pxRootCombiner->mnFullSizeH;

					// If width is more than 2x height, expand underneath as well so we end up with a squareish texture rather than a big wide one
					if ( pxNewRootCombiner->mnFullSizeW > pxNewRootCombiner->mnFullSizeH * 2 )
					{
					int		nEmptyH = pxRootCombiner->mnFullSizeH;

						pxNewRootCombiner->mnFullSizeH = pxRootCombiner->mnFullSizeW;

						// Add bottom left
						pxNewRootCombiner->mapSubGrids[2] = new CombinerTextureGrid;
						pxNewRootCombiner->mapSubGrids[2]->InitGrid( pxNewRootCombiner->mnFullSizeW - pxUniqueTexture->mnTextureSizeW, nEmptyH, 0, pxRootCombiner->mnFullSizeH );

						// Add bottom right
						nEmptyH = pxNewRootCombiner->mnFullSizeH - pxUniqueTexture->mnTextureSizeH;

						pxNewRootCombiner->mapSubGrids[3] = new CombinerTextureGrid;
						pxNewRootCombiner->mapSubGrids[3]->InitGrid( pxUniqueTexture->mnTextureSizeW, nEmptyH, nRightStartX, pxRootCombiner->mnStartPosY + pxUniqueTexture->mnTextureSizeH );
					}
					// If current height is bigger than texture we add an unoccupied spot to the lower right as well
					else if ( pxRootCombiner->mnFullSizeH > pxUniqueTexture->mnTextureSizeH )
					{
					int		nEmptyH = pxRootCombiner->mnFullSizeH - pxUniqueTexture->mnTextureSizeH;

						pxNewRootCombiner->mapSubGrids[2] = new CombinerTextureGrid;
						pxNewRootCombiner->mapSubGrids[2]->InitGrid( pxUniqueTexture->mnTextureSizeW, nEmptyH, nRightStartX, pxRootCombiner->mnStartPosY + pxUniqueTexture->mnTextureSizeH );
					}


				}
				else  // existing height needs to expand to include the new texture - i.e. we'll add grid to the right and underneath the existing one..
				{
				int		nEmptyH = pxUniqueTexture->mnTextureSizeH - pxRootCombiner->mnFullSizeH;

					pxNewRootCombiner->mnFullSizeH = pxUniqueTexture->mnTextureSizeH;

					pxNewRootCombiner->mapSubGrids[2] = new CombinerTextureGrid;
					pxNewRootCombiner->mapSubGrids[2]->InitGrid( pxRootCombiner->mnFullSizeW, nEmptyH, pxRootCombiner->mnStartPosX, pxRootCombiner->mnStartPosY + pxRootCombiner->mnFullSizeH );			
				}

				// Make the new root the root...
				pxRootCombiner = pxNewRootCombiner;

				// TODO - This expansion should really scale the total up to a multiple of 2.
				// if the current NewRootCombiner isn't that scale we should add an extra layer here..
				

			}

			pxUniqueTexture = pxUniqueTexture->mpNext;
		}


		// Create a new texture by combining all the existing ones
		hNewTexture = EngineCreateTexture( pxRootCombiner->mnFullSizeW, pxRootCombiner->mnFullSizeH, 1 );

		CombinerAddToNewTexture( hNewTexture, pxRootCombiner );

		// todo - Pop up 'Save as..' dialog to determine output filename
		EngineExportTexture( hNewTexture, "Test.png", 3 );

		EngineReleaseTexture( &hNewTexture );

		CombinerCalcUVOffsets( pxRootCombiner, &pxUniqueTextureList );

		// Remap UVs to new texture / run through the vertices and (depending on their original material/texture assignment)
		//  recalculate their UVs based on the new texture mapping
		CombinerRemapUVs( pxModelData, pxRootCombiner, &pxUniqueTextureList );

		// Change all materials to use the newly generated texture /  Remove all the previous texture handles (setting them all to the new texture)
		pxMaterialData = pxModelData->pMaterialData;
		pxMaterialData->LoadTextureFilename( 0, "Test.png" );

		pxBaseMaterial = pxMaterialData;
		pxMaterialData = pxMaterialData->GetNext();

		while( pxMaterialData )
		{
			pxMaterialData->CloneTextureFromMaterial( DIFFUSEMAP, pxBaseMaterial );
			pxMaterialData = pxMaterialData->GetNext();
		}

	}
}
