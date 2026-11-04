
//-----------------------------------------------------------------------------------------------------------
// ModelSkeletonFiles.cpp
//
//  .atm chunk export & load for the skeletal animation data (chunks 0x1C - 0x1E, see ATMFormat.md)
//  Called from ModelFiles.cpp
//-----------------------------------------------------------------------------------------------------------

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>

#include "../ModelFiles.h"
#include "ModelSkeleton.h"
#include "ModelSkeletonFiles.h"

//-----------------------------------------------------------------------------------------------------------
// Export
//-----------------------------------------------------------------------------------------------------------

byte*		ModelSkeletonExportChunkBones( MODEL_RENDER_DATA* pxModelData, int* pnSize, int* pnElements )
{
ModelSkeleton*				pSkeleton = pxModelData->pSkeleton;
ATM_SKELETON_BONE_ENTRY*	pxEntries;
int							nLoop;

	if ( ( pSkeleton == NULL ) || ( pSkeleton->GetNumBones() == 0 ) ) return( NULL );

	pxEntries = (ATM_SKELETON_BONE_ENTRY*)SystemMalloc( pSkeleton->GetNumBones() * sizeof( ATM_SKELETON_BONE_ENTRY ) );
	memset( pxEntries, 0, pSkeleton->GetNumBones() * sizeof( ATM_SKELETON_BONE_ENTRY ) );
	for ( nLoop = 0; nLoop < pSkeleton->GetNumBones(); nLoop++ )
	{
	const MODEL_SKELETON_BONE*	pxBone = pSkeleton->GetBone( nLoop );

		strncpy( pxEntries[nLoop].acName, pxBone->acName, 31 );
		pxEntries[nLoop].wParentIndex = (short)pxBone->nParentIndex;
		pxEntries[nLoop].xBindPos = pxBone->xBindPos;
		pxEntries[nLoop].xBindRot.w = 1.0f;
	}
	*(pnSize) = sizeof( ATM_SKELETON_BONE_ENTRY );
	*(pnElements) = pSkeleton->GetNumBones();
	return( (byte*)pxEntries );
}

byte*		ModelSkeletonExportChunkWeights( MODEL_RENDER_DATA* pxModelData, int* pnSize, int* pnElements )
{
ModelSkeleton*				pSkeleton = pxModelData->pSkeleton;
const MODEL_SKIN_WEIGHT*	pxWeights;
ATM_SKIN_WEIGHT_ENTRY*		pxEntries;
int							nLoop;
int							nSlot;

int							nNumEntries;

	if ( ( pSkeleton == NULL ) || ( pSkeleton->GetWeights() == NULL ) || ( pxModelData->pxBaseMesh == NULL ) ) return( NULL );
	if ( pSkeleton->GetNumWeights() != pxModelData->xStats.nNumVertices ) return( NULL );

	// The vertex chunk is written using the mesh's vertex count, which can be larger than xStats.nNumVertices
	// (e.g. older keyframe models whose meshes were allocated one vertex per index). The extra vertices are
	// unused, so pad them with a rigid weight to the root bone to keep the counts matching.
	nNumEntries = pxModelData->pxBaseMesh->GetNumVertices();
	if ( nNumEntries < pSkeleton->GetNumWeights() ) nNumEntries = pSkeleton->GetNumWeights();

	pxWeights = pSkeleton->GetWeights();
	pxEntries = (ATM_SKIN_WEIGHT_ENTRY*)SystemMalloc( nNumEntries * sizeof( ATM_SKIN_WEIGHT_ENTRY ) );
	memset( pxEntries, 0, nNumEntries * sizeof( ATM_SKIN_WEIGHT_ENTRY ) );
	for ( nLoop = pSkeleton->GetNumWeights(); nLoop < nNumEntries; nLoop++ )
	{
		pxEntries[nLoop].abWeight[0] = 255;
	}
	for ( nLoop = 0; nLoop < pSkeleton->GetNumWeights(); nLoop++ )
	{
	int		nTotal = 0;
	int		nBiggest = 0;

		for ( nSlot = 0; nSlot < 4; nSlot++ )
		{
		int		nWeight = (int)( ( pxWeights[nLoop].afWeight[nSlot] * 255.0f ) + 0.5f );

			if ( nWeight > 255 ) nWeight = 255;
			if ( nWeight < 0 ) nWeight = 0;
			pxEntries[nLoop].abBoneIndex[nSlot] = pxWeights[nLoop].abBoneIndex[nSlot];
			pxEntries[nLoop].abWeight[nSlot] = (BYTE)nWeight;
			nTotal += nWeight;
			if ( nWeight > pxEntries[nLoop].abWeight[nBiggest] ) nBiggest = nSlot;
		}
		// Make the bytes sum to exactly 255 (rounding error goes on the biggest influence)
		pxEntries[nLoop].abWeight[nBiggest] = (BYTE)( pxEntries[nLoop].abWeight[nBiggest] + (255 - nTotal) );
	}
	*(pnSize) = sizeof( ATM_SKIN_WEIGHT_ENTRY );
	*(pnElements) = nNumEntries;
	return( (byte*)pxEntries );
}

byte*		ModelSkeletonExportChunkAnimLinks( MODEL_RENDER_DATA* pxModelData, int* pnSize, int* pnElements )
{
ModelSkeleton*					pSkeleton = pxModelData->pSkeleton;
ATM_SKELETAL_ANIM_LINK_ENTRY*	pxEntries;
int		nNumLinks = 0;
int		nLoop;

	if ( pSkeleton == NULL ) return( NULL );
	for ( nLoop = 0; nLoop < pSkeleton->GetNumAnimations(); nLoop++ )
	{
		if ( pSkeleton->GetAnimation( nLoop )->bLinkInModel ) nNumLinks++;
	}
	if ( nNumLinks == 0 ) return( NULL );

	pxEntries = (ATM_SKELETAL_ANIM_LINK_ENTRY*)SystemMalloc( nNumLinks * sizeof( ATM_SKELETAL_ANIM_LINK_ENTRY ) );
	memset( pxEntries, 0, nNumLinks * sizeof( ATM_SKELETAL_ANIM_LINK_ENTRY ) );
	nNumLinks = 0;
	for ( nLoop = 0; nLoop < pSkeleton->GetNumAnimations(); nLoop++ )
	{
	const MODEL_SKELETON_ANIM*	pxAnim = pSkeleton->GetAnimation( nLoop );

		if ( pxAnim->bLinkInModel )
		{
			strncpy( pxEntries[nNumLinks].acFilename, pxAnim->acFilename, 63 );
			pxEntries[nNumLinks].uwAnimUse = (ushort)pxAnim->nAnimUse;
			pxEntries[nNumLinks].fSpeed = 1.0f;
			nNumLinks++;
		}
	}
	*(pnSize) = sizeof( ATM_SKELETAL_ANIM_LINK_ENTRY );
	*(pnElements) = nNumLinks;
	return( (byte*)pxEntries );
}

//-----------------------------------------------------------------------------------------------------------
// Load
//-----------------------------------------------------------------------------------------------------------

void	ModelSkeletonLoadChunkBones( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
ATM_SKELETON_BONE_ENTRY*	pxEntries = (ATM_SKELETON_BONE_ENTRY*)pbMem;
MODEL_SKELETON_BONE*		pxBones;
int		nNumBones = pxChunkHeader->nNumElementsInChunk;
int		nLoop;

	if ( ( nNumBones <= 0 ) || ( nNumBones > MODEL_SKELETON_MAX_BONES ) ) return;
	if ( pxChunkHeader->nSizeOfElement < (int)sizeof( ATM_SKELETON_BONE_ENTRY ) ) return;

	pxBones = (MODEL_SKELETON_BONE*)malloc( nNumBones * sizeof( MODEL_SKELETON_BONE ) );
	for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
	{
		// (Step by the stored element size, so later versions can append fields)
		pxEntries = (ATM_SKELETON_BONE_ENTRY*)( pbMem + (nLoop * pxChunkHeader->nSizeOfElement) );
		memset( &pxBones[nLoop], 0, sizeof( MODEL_SKELETON_BONE ) );
		strncpy( pxBones[nLoop].acName, pxEntries->acName, MODEL_SKELETON_BONE_NAME_LEN - 1 );
		pxBones[nLoop].nParentIndex = pxEntries->wParentIndex;
		if ( pxBones[nLoop].nParentIndex >= nLoop ) pxBones[nLoop].nParentIndex = NOTFOUND;
		pxBones[nLoop].xBindPos = pxEntries->xBindPos;
	}
	SAFE_DELETE( pxModelData->pSkeleton );
	pxModelData->pSkeleton = new ModelSkeleton;
	pxModelData->pSkeleton->Create( nNumBones, pxBones );
	free( pxBones );
}

void	ModelSkeletonLoadChunkWeights( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
MODEL_SKIN_WEIGHT*		pxWeights;
ATM_SKIN_WEIGHT_ENTRY*	pxEntry;
int		nNumVerts = pxChunkHeader->nNumElementsInChunk;
int		nLoop;
int		nSlot;

	if ( pxModelData->pSkeleton == NULL ) return;
	if ( pxChunkHeader->nSizeOfElement < (int)sizeof( ATM_SKIN_WEIGHT_ENTRY ) ) return;

	pxWeights = (MODEL_SKIN_WEIGHT*)malloc( nNumVerts * sizeof( MODEL_SKIN_WEIGHT ) );
	for ( nLoop = 0; nLoop < nNumVerts; nLoop++ )
	{
		pxEntry = (ATM_SKIN_WEIGHT_ENTRY*)( pbMem + (nLoop * pxChunkHeader->nSizeOfElement) );
		for ( nSlot = 0; nSlot < 4; nSlot++ )
		{
			pxWeights[nLoop].abBoneIndex[nSlot] = pxEntry->abBoneIndex[nSlot];
			pxWeights[nLoop].afWeight[nSlot] = pxEntry->abWeight[nSlot] / 255.0f;
		}
	}
	pxModelData->pSkeleton->SetWeights( nNumVerts, pxWeights );
	free( pxWeights );
}

void	ModelSkeletonLoadChunkAnimLinks( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
ATM_SKELETAL_ANIM_LINK_ENTRY*	pxEntry;
char	acFilename[68];
int		nLoop;

	if ( pxModelData->pSkeleton == NULL ) return;
	if ( pxChunkHeader->nSizeOfElement < (int)sizeof( ATM_SKELETAL_ANIM_LINK_ENTRY ) ) return;

	for ( nLoop = 0; nLoop < pxChunkHeader->nNumElementsInChunk; nLoop++ )
	{
		pxEntry = (ATM_SKELETAL_ANIM_LINK_ENTRY*)( pbMem + (nLoop * pxChunkHeader->nSizeOfElement) );
		memcpy( acFilename, pxEntry->acFilename, 64 );
		acFilename[64] = 0;
		ModelSkeletonAddAnimationForModel( pxModelData, acFilename, pxEntry->uwAnimUse, TRUE );
	}
}
