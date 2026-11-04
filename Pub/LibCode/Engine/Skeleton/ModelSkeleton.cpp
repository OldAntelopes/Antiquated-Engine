
#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>

#include "../ModelRendering.h"
#include "SkeletonMaths.h"
#include "SkeletalAnimation.h"
#include "ModelSkeleton.h"

static const char*	maszHumanoidBoneNames[HUMANOID_NUM_BONES] =
{
	"Hips", "Spine", "Spine1", "Neck", "Head",
	"LeftShoulder", "LeftArm", "LeftForeArm", "LeftHand",
	"RightShoulder", "RightArm", "RightForeArm", "RightHand",
	"LeftUpLeg", "LeftLeg", "LeftFoot", "LeftToeBase",
	"RightUpLeg", "RightLeg", "RightFoot", "RightToeBase",
};

static const int	manHumanoidBoneParents[HUMANOID_NUM_BONES] =
{
	NOTFOUND, HUMANOID_HIPS, HUMANOID_SPINE, HUMANOID_SPINE1, HUMANOID_NECK,
	HUMANOID_SPINE1, HUMANOID_LEFT_SHOULDER, HUMANOID_LEFT_ARM, HUMANOID_LEFT_FOREARM,
	HUMANOID_SPINE1, HUMANOID_RIGHT_SHOULDER, HUMANOID_RIGHT_ARM, HUMANOID_RIGHT_FOREARM,
	HUMANOID_HIPS, HUMANOID_LEFT_UPLEG, HUMANOID_LEFT_LEG, HUMANOID_LEFT_FOOT,
	HUMANOID_HIPS, HUMANOID_RIGHT_UPLEG, HUMANOID_RIGHT_LEG, HUMANOID_RIGHT_FOOT,
};

// Converts source animation axes (Y up, facing +Z, left +X) to engine axes (Z up, facing +X, left +Y)
// - a 120 degree rotation about (1,1,1)
static const ENGINEQUATERNION	mxSourceToModelAxes = { 0.5f, 0.5f, 0.5f, 0.5f };

// Auto-weight tuning : capsule radii as fractions of the model height (see ModelSkeleton::AutoGenerateWeights)
static float	msfAutoWeightTorsoRadius = 0.05f;
static float	msfAutoWeightHeadRadius = 0.03f;

//-----------------------------------------------------------------------------------------------------------

ModelSkeleton::ModelSkeleton()
{
	mpxBones = NULL;
	mnNumBones = 0;
	mpxWeights = NULL;
	mnNumWeights = 0;
	mpxBindPositions = NULL;
	mpxBindNormals = NULL;
	mnNumBindVertices = 0;
	memset( maxAnims, 0, sizeof( maxAnims ) );
	mnNumAnims = 0;
	mnCurrentAnim = NOTFOUND;
	mulAnimStartTick = 0;
	mbKeepRootMotion = FALSE;
	mpxCurrentRots = NULL;
	mpxCurrentJoints = NULL;
	mpLastSkinMesh = NULL;
	mulLastSkinTick = 0;
	mnLastSkinAnim = NOTFOUND;
	mulLastSkinStartTick = 0;
	mfLastSkinSpeed = 0.0f;
	mbLastSkinKeepRootMotion = FALSE;
}

ModelSkeleton::~ModelSkeleton()
{
int		nLoop;

	for ( nLoop = 0; nLoop < mnNumAnims; nLoop++ )
	{
		SAFE_DELETE( maxAnims[nLoop].pSource );
		SAFE_FREE( maxAnims[nLoop].pxBoneRots );
		SAFE_FREE( maxAnims[nLoop].pxRootPos );
	}
	SAFE_FREE( mpxBones );
	SAFE_FREE( mpxWeights );
	SAFE_FREE( mpxBindPositions );
	SAFE_FREE( mpxBindNormals );
	SAFE_FREE( mpxCurrentRots );
	SAFE_FREE( mpxCurrentJoints );
}

void	ModelSkeleton::Create( int nNumBones, const MODEL_SKELETON_BONE* pxBones )
{
int		nLoop;

	InvalidateSkinCache();
	SAFE_FREE( mpxBones );
	SAFE_FREE( mpxCurrentRots );
	SAFE_FREE( mpxCurrentJoints );
	mnNumBones = nNumBones;
	mpxBones = (MODEL_SKELETON_BONE*)malloc( nNumBones * sizeof( MODEL_SKELETON_BONE ) );
	memcpy( mpxBones, pxBones, nNumBones * sizeof( MODEL_SKELETON_BONE ) );
	mpxCurrentRots = (ENGINEQUATERNION*)malloc( nNumBones * sizeof( ENGINEQUATERNION ) );
	mpxCurrentJoints = (VECT*)malloc( nNumBones * sizeof( VECT ) );
	for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
	{
		mpxBones[nLoop].acName[MODEL_SKELETON_BONE_NAME_LEN-1] = 0;
		mpxCurrentRots[nLoop] = QuatIdentity();
		mpxCurrentJoints[nLoop] = mpxBones[nLoop].xBindPos;
	}
}

int		ModelSkeleton::FindBone( const char* szName )
{
char	acWanted[SKELANIM_MAX_NAME_LEN];
char	acBone[SKELANIM_MAX_NAME_LEN];
int		nLoop;

	SkeletalAnimationNormaliseBoneName( szName, acWanted, SKELANIM_MAX_NAME_LEN );
	for ( nLoop = 0; nLoop < mnNumBones; nLoop++ )
	{
		SkeletalAnimationNormaliseBoneName( mpxBones[nLoop].acName, acBone, SKELANIM_MAX_NAME_LEN );
		if ( strcmp( acWanted, acBone ) == 0 ) return( nLoop );
	}
	return( NOTFOUND );
}

//-------------------------------------------------------------------------
// Function    : GetDirectionChild
// Description : The child joint that defines a bone's direction. Prefers the spine chain over limbs
//				 (so Hips points at Spine, and Spine1 at Neck rather than a shoulder)
//-------------------------------------------------------------------------
int		ModelSkeleton::GetDirectionChild( int nBone )
{
int		nLoop;
int		nFirstChild = NOTFOUND;
char	acName[SKELANIM_MAX_NAME_LEN];

	for ( nLoop = nBone + 1; nLoop < mnNumBones; nLoop++ )
	{
		if ( mpxBones[nLoop].nParentIndex == nBone )
		{
			if ( nFirstChild == NOTFOUND ) nFirstChild = nLoop;
			SkeletalAnimationNormaliseBoneName( mpxBones[nLoop].acName, acName, SKELANIM_MAX_NAME_LEN );
			if ( strstr( acName, "spine" ) || strstr( acName, "neck" ) || strstr( acName, "head" ) )
			{
				return( nLoop );
			}
		}
	}
	return( nFirstChild );
}

//-----------------------------------------------------------------------------------------------------------
// Bind mesh / weights
//-----------------------------------------------------------------------------------------------------------

void	ModelSkeleton::CaptureBindMesh( MODEL_RENDER_DATA* pxModelData )
{
CUSTOMVERTEX*	pxVertices = NULL;
int				nLoop;
int				nNumVertices = pxModelData->xStats.nNumVertices;

	if ( ( mpxBindPositions != NULL ) && ( mnNumBindVertices == nNumVertices ) ) return;
	if ( pxModelData->pxBaseMesh == NULL ) return;

	SAFE_FREE( mpxBindPositions );
	SAFE_FREE( mpxBindNormals );
	mnNumBindVertices = nNumVertices;
	mpxBindPositions = (VECT*)malloc( nNumVertices * sizeof( VECT ) );
	mpxBindNormals = (VECT*)malloc( nNumVertices * sizeof( VECT ) );

	pxModelData->pxBaseMesh->LockVertexBuffer( kLock_ReadOnly, (byte**)( &pxVertices ) );
	if ( pxVertices )
	{
		for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
		{
			mpxBindPositions[nLoop] = pxVertices[nLoop].position;
			mpxBindNormals[nLoop] = pxVertices[nLoop].normal;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
}

void	ModelSkeleton::RestoreBindPose( MODEL_RENDER_DATA* pxModelData )
{
CUSTOMVERTEX*	pxVertices = NULL;
int				nLoop;

	if ( ( mpxBindPositions == NULL ) || ( pxModelData->pxBaseMesh == NULL ) ) return;
	if ( mnNumBindVertices != pxModelData->xStats.nNumVertices ) return;

	InvalidateSkinCache();
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	if ( pxVertices )
	{
		for ( nLoop = 0; nLoop < mnNumBindVertices; nLoop++ )
		{
			pxVertices[nLoop].position = mpxBindPositions[nLoop];
			pxVertices[nLoop].normal = mpxBindNormals[nLoop];
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
}

void	ModelSkeleton::SetWeights( int nNumVertices, const MODEL_SKIN_WEIGHT* pxWeights )
{
	InvalidateSkinCache();
	SAFE_FREE( mpxWeights );
	mnNumWeights = nNumVertices;
	mpxWeights = (MODEL_SKIN_WEIGHT*)malloc( nNumVertices * sizeof( MODEL_SKIN_WEIGHT ) );
	memcpy( mpxWeights, pxWeights, nNumVertices * sizeof( MODEL_SKIN_WEIGHT ) );
}

void	ModelSkeleton::RemapVertices( int nNumVertices, const DWORD* pulNewToOld )
{
MODEL_SKIN_WEIGHT*	pxNewWeights;
int					nLoop;

	if ( mpxWeights == NULL ) return;
	InvalidateSkinCache();
	pxNewWeights = (MODEL_SKIN_WEIGHT*)malloc( nNumVertices * sizeof( MODEL_SKIN_WEIGHT ) );
	for ( nLoop = 0; nLoop < nNumVertices; nLoop++ )
	{
		if ( (int)pulNewToOld[nLoop] < mnNumWeights )
		{
			pxNewWeights[nLoop] = mpxWeights[ pulNewToOld[nLoop] ];
		}
		else
		{
			memset( &pxNewWeights[nLoop], 0, sizeof( MODEL_SKIN_WEIGHT ) );
			pxNewWeights[nLoop].afWeight[0] = 1.0f;
		}
	}
	free( mpxWeights );
	mpxWeights = pxNewWeights;
	mnNumWeights = nNumVertices;
	SAFE_FREE( mpxBindPositions );		// recaptured on next use
	SAFE_FREE( mpxBindNormals );
}

static float	ModelSkeletonDistToSegment( const VECT* pxPoint, const VECT* pxStart, const VECT* pxEnd, float* pfPhaseOut )
{
VECT	xSeg;
VECT	xToPoint;
VECT	xClosest;
float	fSegLenSq;
float	fPhase;

	VectSub( &xSeg, pxEnd, pxStart );
	VectSub( &xToPoint, pxPoint, pxStart );
	fSegLenSq = VectDot( &xSeg, &xSeg );
	fPhase = ( fSegLenSq > 0.0000001f ) ? ( VectDot( &xToPoint, &xSeg ) / fSegLenSq ) : 0.0f;
	if ( fPhase < 0.0f ) fPhase = 0.0f;
	if ( fPhase > 1.0f ) fPhase = 1.0f;
	if ( pfPhaseOut ) *pfPhaseOut = fPhase;
	xClosest.x = pxStart->x + (xSeg.x * fPhase);
	xClosest.y = pxStart->y + (xSeg.y * fPhase);
	xClosest.z = pxStart->z + (xSeg.z * fPhase);
	return( VectDist( pxPoint, &xClosest ) );
}

//-------------------------------------------------------------------------
// Function    : AutoGenerateWeights
// Description : Quick'n'dirty automatic skinning.
//				 1) Each vertex is bound to the nearest bone segment, then cleaned up by surface connectivity
//				    (each bone keeps only its island of vertices nearest the bone - see below)
//				 2) Weights are smoothed across the mesh surface (so blending follows connectivity, and an arm
//				    hanging next to the body doesn't pick up the hips)
//				 3) Top 4 influences are kept and normalised
//				 Vertices at the same position (uv seams) are welded so they always get identical weights.
//-------------------------------------------------------------------------
void	ModelSkeleton::AutoGenerateWeights( MODEL_RENDER_DATA* pxModelData, int nSmoothingPasses )
{
int		nNumVertices = pxModelData->xStats.nNumVertices;
int		nNumBones = mnNumBones;
int*	pnWeldRep;

	InvalidateSkinCache();
float*	pfWeights;
float*	pfNewWeights;
float*	pfBoneDist;
float*	pfBonePhase;
int*	pnLabel;
VECT*	pxSegStart;
VECT*	pxSegEnd;
int*	pnNeighbourCount;
int*	pnNeighbourStart;
int*	pnNeighbours;
int		nNumFaces;
byte*	pbIndices = NULL;
BOOL	b32Bit;
int		nVert, nVert2, nBone, nPass, nFace, nCorner, nLoop;
float	fHeight = pxModelData->xStats.xBoundMax.z - pxModelData->xStats.xBoundMin.z;
MODEL_SKIN_WEIGHT*	pxSkin;

	if ( ( nNumBones == 0 ) || ( pxModelData->pxBaseMesh == NULL ) ) return;
	CaptureBindMesh( pxModelData );
	if ( mpxBindPositions == NULL ) return;
	if ( fHeight <= 0.0f ) fHeight = 1.0f;

	// ---- Weld coincident vertices
	pnWeldRep = (int*)malloc( nNumVertices * sizeof( int ) );
	for ( nVert = 0; nVert < nNumVertices; nVert++ )
	{
		pnWeldRep[nVert] = nVert;
		for ( nVert2 = 0; nVert2 < nVert; nVert2++ )
		{
			if ( ( pnWeldRep[nVert2] == nVert2 ) &&
				 ( VectDist( &mpxBindPositions[nVert], &mpxBindPositions[nVert2] ) < fHeight * 0.0005f ) )
			{
				pnWeldRep[nVert] = nVert2;
				break;
			}
		}
	}

	// ---- Bone segments
	pxSegStart = (VECT*)malloc( nNumBones * sizeof( VECT ) );
	pxSegEnd = (VECT*)malloc( nNumBones * sizeof( VECT ) );
	for ( nBone = 0; nBone < nNumBones; nBone++ )
	{
	int		nChild = GetDirectionChild( nBone );
	int		nParent = mpxBones[nBone].nParentIndex;

		pxSegStart[nBone] = mpxBones[nBone].xBindPos;
		pxSegEnd[nBone] = mpxBones[nBone].xBindPos;
		if ( nChild != NOTFOUND )
		{
			pxSegEnd[nBone] = mpxBones[nChild].xBindPos;
		}
		else if ( nParent != NOTFOUND )
		{
		VECT	xDir;
		float	fLen;

			// Leaf bone (head, hands, toes) - extend in the parent->bone direction
			VectSub( &xDir, &mpxBones[nBone].xBindPos, &mpxBones[nParent].xBindPos );
			fLen = VectNormalize( &xDir ) * 0.5f;
			if ( fLen < fHeight * 0.06f ) fLen = fHeight * 0.06f;
			pxSegEnd[nBone].x += xDir.x * fLen;
			pxSegEnd[nBone].y += xDir.y * fLen;
			pxSegEnd[nBone].z += xDir.z * fLen;
		}
	}

	// ---- Initial labels - nearest bone, treating torso & head bones as thick capsules
	// Plain distance-to-bone-line favours thin limbs over the thick torso (the side of the chest is nearer the upper arm
	// line than the spine line). So the spine chain and head get a fixed radius (a fraction of the model height) and
	// labelling uses ( distance - radius ). The radius tapers to nothing at the bone ends so a thick bone doesn't
	// swallow its neighbours at the joints. (Radii estimated from the mesh itself were tried - they feed back on
	// themselves and the chest ends up taking the head.)
	pfWeights = (float*)malloc( nNumVertices * nNumBones * sizeof( float ) );
	pfNewWeights = (float*)malloc( nNumVertices * nNumBones * sizeof( float ) );
	pfBoneDist = (float*)malloc( nNumVertices * nNumBones * sizeof( float ) );
	pfBonePhase = (float*)malloc( nNumVertices * nNumBones * sizeof( float ) );
	pnLabel = (int*)malloc( nNumVertices * sizeof( int ) );
	memset( pfWeights, 0, nNumVertices * nNumBones * sizeof( float ) );
	{
	float*	pfRawDist = (float*)malloc( nNumVertices * nNumBones * sizeof( float ) );
	float*	pfTaper = (float*)malloc( nNumVertices * nNumBones * sizeof( float ) );
	float*	pfRadius = (float*)malloc( nNumBones * sizeof( float ) );
	int*	pnCount = (int*)malloc( nNumBones * sizeof( int ) );

		for ( nVert = 0; nVert < nNumVertices; nVert++ )
		{
			if ( pnWeldRep[nVert] != nVert ) continue;
			for ( nBone = 0; nBone < nNumBones; nBone++ )
			{
			float	fPhase;
			float	fTaper;

				pfRawDist[ (nVert * nNumBones) + nBone ] = ModelSkeletonDistToSegment( &mpxBindPositions[nVert], &pxSegStart[nBone], &pxSegEnd[nBone], &fPhase );
				pfBonePhase[ (nVert * nNumBones) + nBone ] = fPhase;
				fTaper = ( (fPhase < 0.5f) ? fPhase : (1.0f - fPhase) ) * 4.0f;
				pfTaper[ (nVert * nNumBones) + nBone ] = (fTaper > 1.0f) ? 1.0f : fTaper;
			}
		}
		for ( nBone = 0; nBone < nNumBones; nBone++ )
		{
		char	acName[SKELANIM_MAX_NAME_LEN];

			SkeletalAnimationNormaliseBoneName( mpxBones[nBone].acName, acName, SKELANIM_MAX_NAME_LEN );
			pfRadius[nBone] = 0.0f;
			if ( strstr( acName, "spine" ) || ( strcmp( acName, "hips" ) == 0 ) ) pfRadius[nBone] = fHeight * msfAutoWeightTorsoRadius;
			else if ( strcmp( acName, "head" ) == 0 ) pfRadius[nBone] = fHeight * msfAutoWeightHeadRadius;
		}

		for ( nVert = 0; nVert < nNumVertices; nVert++ )
		{
		float	fBest = 1000000.0f;
		int		nBest = 0;
		float	fDist;

			pnLabel[nVert] = NOTFOUND;
			if ( pnWeldRep[nVert] != nVert ) continue;
			for ( nBone = 0; nBone < nNumBones; nBone++ )
			{
				fDist = pfRawDist[ (nVert * nNumBones) + nBone ] - ( pfRadius[nBone] * pfTaper[ (nVert * nNumBones) + nBone ] );
				pfBoneDist[ (nVert * nNumBones) + nBone ] = fDist;
				if ( fDist < fBest )
				{
					fBest = fDist;
					nBest = nBone;
				}
			}
			pnLabel[nVert] = nBest;
		}
		free( pnCount );
		free( pfRadius );
		free( pfTaper );
		free( pfRawDist );
	}
	// ---- Adjacency (between weld representatives) from the index buffer
	nNumFaces = pxModelData->pxBaseMesh->GetNumFaces();
	b32Bit = pxModelData->pxBaseMesh->Is32BitIndexBuffer();
	pnNeighbourCount = (int*)malloc( nNumVertices * sizeof( int ) );
	pnNeighbourStart = (int*)malloc( (nNumVertices + 1) * sizeof( int ) );
	pnNeighbours = (int*)malloc( nNumFaces * 6 * sizeof( int ) );
	memset( pnNeighbourCount, 0, nNumVertices * sizeof( int ) );

	pxModelData->pxBaseMesh->LockIndexBuffer( kLock_ReadOnly, &pbIndices );
	if ( pbIndices )
	{
	int		nPass2;

		// Two passes over the faces - count, then fill
		for ( nPass2 = 0; nPass2 < 2; nPass2++ )
		{
			if ( nPass2 == 1 )
			{
				pnNeighbourStart[0] = 0;
				for ( nVert = 0; nVert < nNumVertices; nVert++ )
				{
					pnNeighbourStart[nVert+1] = pnNeighbourStart[nVert] + pnNeighbourCount[nVert];
					pnNeighbourCount[nVert] = 0;
				}
			}
			for ( nFace = 0; nFace < nNumFaces; nFace++ )
			{
			int		anCorner[3];

				for ( nCorner = 0; nCorner < 3; nCorner++ )
				{
					anCorner[nCorner] = b32Bit ? ((uint32*)pbIndices)[ (nFace*3) + nCorner ] : ((ushort*)pbIndices)[ (nFace*3) + nCorner ];
					if ( anCorner[nCorner] >= nNumVertices ) anCorner[nCorner] = 0;
					anCorner[nCorner] = pnWeldRep[ anCorner[nCorner] ];
				}
				for ( nCorner = 0; nCorner < 3; nCorner++ )
				{
				int		nFrom = anCorner[nCorner];

					for ( nLoop = 1; nLoop < 3; nLoop++ )
					{
						if ( nPass2 == 1 )
						{
							pnNeighbours[ pnNeighbourStart[nFrom] + pnNeighbourCount[nFrom] ] = anCorner[ (nCorner + nLoop) % 3 ];
						}
						pnNeighbourCount[nFrom]++;
					}
				}
			}
		}
		pxModelData->pxBaseMesh->UnlockIndexBuffer();
	}

	// ---- Connectivity cleanup
	// Nearest-segment labelling goes wrong where parts of the mesh sit close together in the bind pose
	// (a hand hanging against the loincloth, forearms next to the waist). For each bone, only the surface-connected
	// island of vertices nearest to the bone is kept; vertices in any other island of that bone are unlabelled and
	// then refilled from their surface neighbours (taking whichever neighbouring bone is closest). A refilled vertex
	// can't take a bone from the same limb chain it was wrongly labelled with (otherwise e.g. the upper arm label just
	// grows across the chest instead of the forearm label).
	{
	int*	pnIsland = (int*)malloc( nNumVertices * sizeof( int ) );
	int*	pnStack = (int*)malloc( nNumVertices * sizeof( int ) );
	int*	pnNewLabel = (int*)malloc( nNumVertices * sizeof( int ) );
	int*	pnChainHead = (int*)malloc( nNumBones * sizeof( int ) );
	int		nNumIslands = 0;
	int		nStackSize;
	int		nNumRelabelled = 0;
	BOOL	bChanged;

		// Limb chains : walk up from each bone while the parent has only this one child
		// (e.g. LeftHand, LeftForeArm, LeftArm and LeftShoulder all share the chain head LeftShoulder)
		for ( nBone = 0; nBone < nNumBones; nBone++ )
		{
		int		nHead = nBone;

			while ( mpxBones[nHead].nParentIndex != NOTFOUND )
			{
			int		nParent = mpxBones[nHead].nParentIndex;
			int		nNumChildren = 0;

				for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
				{
					if ( mpxBones[nLoop].nParentIndex == nParent ) nNumChildren++;
				}
				if ( ( nNumChildren != 1 ) || ( mpxBones[nParent].nParentIndex == NOTFOUND ) ) break;
				nHead = nParent;
			}
			pnChainHead[nBone] = nHead;
		}

		for ( nVert = 0; nVert < nNumVertices; nVert++ ) pnIsland[nVert] = NOTFOUND;

		// Flood fill islands of same-labelled vertices
		for ( nVert = 0; nVert < nNumVertices; nVert++ )
		{
			if ( ( pnLabel[nVert] == NOTFOUND ) || ( pnIsland[nVert] != NOTFOUND ) ) continue;
			pnIsland[nVert] = nNumIslands;
			nStackSize = 0;
			pnStack[nStackSize++] = nVert;
			while ( nStackSize > 0 )
			{
			int		nCurrent = pnStack[--nStackSize];

				for ( nLoop = 0; nLoop < pnNeighbourCount[nCurrent]; nLoop++ )
				{
				int		nNeighbour = pnNeighbours[ pnNeighbourStart[nCurrent] + nLoop ];

					if ( ( pnIsland[nNeighbour] == NOTFOUND ) && ( pnLabel[nNeighbour] == pnLabel[nVert] ) )
					{
						pnIsland[nNeighbour] = nNumIslands;
						pnStack[nStackSize++] = nNeighbour;
					}
				}
			}
			nNumIslands++;
		}

		// For each bone, pick its 'real' island : big, close to the bone, and spread along the bone's length
		//   score = vertex count * ( range of positions along the bone ) / mean distance
		// (Just taking the island with the single nearest vertex picks stray bits - e.g. a hair spike instead of the head;
		//  count/distance alone can prefer a patch of loincloth near the top of the thigh over the thigh itself)
		{
		int*	pnIslandCount = (int*)malloc( nNumIslands * sizeof( int ) );
		float*	pfIslandDist = (float*)malloc( nNumIslands * sizeof( float ) );
		float*	pfIslandPhaseMin = (float*)malloc( nNumIslands * sizeof( float ) );
		float*	pfIslandPhaseMax = (float*)malloc( nNumIslands * sizeof( float ) );

			memset( pnIslandCount, 0, nNumIslands * sizeof( int ) );
			memset( pfIslandDist, 0, nNumIslands * sizeof( float ) );
			for ( nVert = 0; nVert < nNumIslands; nVert++ )
			{
				pfIslandPhaseMin[nVert] = 1.0f;
				pfIslandPhaseMax[nVert] = 0.0f;
			}
			for ( nVert = 0; nVert < nNumVertices; nVert++ )
			{
			int		nIsland = pnIsland[nVert];
			float	fPhase;

				if ( nIsland == NOTFOUND ) continue;
				fPhase = pfBonePhase[ (nVert * nNumBones) + pnLabel[nVert] ];
				pnIslandCount[nIsland]++;
				pfIslandDist[nIsland] += pfBoneDist[ (nVert * nNumBones) + pnLabel[nVert] ];
				if ( fPhase < pfIslandPhaseMin[nIsland] ) pfIslandPhaseMin[nIsland] = fPhase;
				if ( fPhase > pfIslandPhaseMax[nIsland] ) pfIslandPhaseMax[nIsland] = fPhase;
			}
			for ( nVert = 0; nVert < nNumIslands; nVert++ )
			{
				// (distances can be negative inside the torso capsules - shift so the score stays positive)
				pfIslandDist[nVert] = ( pfIslandDist[nVert] / pnIslandCount[nVert] ) + ( fHeight * (msfAutoWeightTorsoRadius + 0.01f) );
				pfIslandDist[nVert] = ( pnIslandCount[nVert] * ( pfIslandPhaseMax[nVert] - pfIslandPhaseMin[nVert] + 0.1f ) ) / pfIslandDist[nVert];		// now the score
			}
			free( pfIslandPhaseMax );
			free( pfIslandPhaseMin );

		for ( nBone = 0; nBone < nNumBones; nBone++ )
		{
		float	fBest = -1.0f;
		int		nBestIsland = NOTFOUND;

			for ( nVert = 0; nVert < nNumVertices; nVert++ )
			{
				if ( ( pnLabel[nVert] == nBone ) && ( pfIslandDist[ pnIsland[nVert] ] > fBest ) )
				{
					fBest = pfIslandDist[ pnIsland[nVert] ];
					nBestIsland = pnIsland[nVert];
				}
			}
			for ( nVert = 0; nVert < nNumVertices; nVert++ )
			{
				if ( ( pnLabel[nVert] == nBone ) && ( pnIsland[nVert] != nBestIsland ) )
				{
					pnNewLabel[nVert] = NOTFOUND;
				}
				else if ( pnLabel[nVert] == nBone )
				{
					pnNewLabel[nVert] = nBone;
				}
			}
		}
			free( pfIslandDist );
			free( pnIslandCount );
		}
		for ( nVert = 0; nVert < nNumVertices; nVert++ )
		{
			if ( pnLabel[nVert] == NOTFOUND ) pnNewLabel[nVert] = NOTFOUND;
			else if ( pnNewLabel[nVert] == NOTFOUND ) nNumRelabelled++;
		}

		// Grow the remaining labels back into the dropped vertices across the surface
		do
		{
			bChanged = FALSE;
			memcpy( pnStack, pnNewLabel, nNumVertices * sizeof( int ) );
			for ( nVert = 0; nVert < nNumVertices; nVert++ )
			{
			float	fBest = 1000000.0f;
			int		nBest = NOTFOUND;

				if ( ( pnLabel[nVert] == NOTFOUND ) || ( pnNewLabel[nVert] != NOTFOUND ) ) continue;
				for ( nLoop = 0; nLoop < pnNeighbourCount[nVert]; nLoop++ )
				{
				int		nNeighbourLabel = pnNewLabel[ pnNeighbours[ pnNeighbourStart[nVert] + nLoop ] ];

					if ( ( nNeighbourLabel != NOTFOUND ) &&
						 ( pnChainHead[nNeighbourLabel] != pnChainHead[ pnLabel[nVert] ] ) &&
						 ( pfBoneDist[ (nVert * nNumBones) + nNeighbourLabel ] < fBest ) )
					{
						fBest = pfBoneDist[ (nVert * nNumBones) + nNeighbourLabel ];
						nBest = nNeighbourLabel;
					}
				}
				if ( nBest != NOTFOUND )
				{
					pnStack[nVert] = nBest;
					bChanged = TRUE;
				}
			}
			memcpy( pnNewLabel, pnStack, nNumVertices * sizeof( int ) );
		} while ( bChanged );

		// Anything that couldn't be reached keeps its original label
		for ( nVert = 0; nVert < nNumVertices; nVert++ )
		{
			if ( pnLabel[nVert] == NOTFOUND ) continue;
			if ( pnNewLabel[nVert] == NOTFOUND ) pnNewLabel[nVert] = pnLabel[nVert];
			pfWeights[ (nVert * nNumBones) + pnNewLabel[nVert] ] = 1.0f;
		}
		SysDebugPrint( "Skeleton: auto-weights - %d islands, %d vertices relabelled by connectivity\n", nNumIslands, nNumRelabelled );

		free( pnChainHead );
		free( pnNewLabel );
		free( pnStack );
		free( pnIsland );
	}

	// ---- Smooth across the surface
	for ( nPass = 0; nPass < nSmoothingPasses; nPass++ )
	{
		memcpy( pfNewWeights, pfWeights, nNumVertices * nNumBones * sizeof( float ) );
		for ( nVert = 0; nVert < nNumVertices; nVert++ )
		{
		int		nCount = pnNeighbourCount[nVert];

			if ( ( pnWeldRep[nVert] != nVert ) || ( nCount == 0 ) ) continue;
			for ( nBone = 0; nBone < nNumBones; nBone++ )
			{
			float	fSum = 0.0f;

				for ( nLoop = 0; nLoop < nCount; nLoop++ )
				{
					fSum += pfWeights[ (pnNeighbours[ pnNeighbourStart[nVert] + nLoop ] * nNumBones) + nBone ];
				}
				pfNewWeights[ (nVert * nNumBones) + nBone ] = (pfWeights[ (nVert * nNumBones) + nBone ] * 0.5f) + ( (fSum / nCount) * 0.5f );
			}
		}
		memcpy( pfWeights, pfNewWeights, nNumVertices * nNumBones * sizeof( float ) );
	}

	// ---- Keep top 4 & normalise
	pxSkin = (MODEL_SKIN_WEIGHT*)malloc( nNumVertices * sizeof( MODEL_SKIN_WEIGHT ) );
	for ( nVert = 0; nVert < nNumVertices; nVert++ )
	{
	const float*	pfVertWeights = &pfWeights[ pnWeldRep[nVert] * nNumBones ];
	float			fTotal = 0.0f;
	int				nSlot;

		memset( &pxSkin[nVert], 0, sizeof( MODEL_SKIN_WEIGHT ) );
		for ( nBone = 0; nBone < nNumBones; nBone++ )
		{
			// insert into the sorted top 4
			for ( nSlot = 0; nSlot < 4; nSlot++ )
			{
				if ( pfVertWeights[nBone] > pxSkin[nVert].afWeight[nSlot] )
				{
					for ( nLoop = 3; nLoop > nSlot; nLoop-- )
					{
						pxSkin[nVert].afWeight[nLoop] = pxSkin[nVert].afWeight[nLoop-1];
						pxSkin[nVert].abBoneIndex[nLoop] = pxSkin[nVert].abBoneIndex[nLoop-1];
					}
					pxSkin[nVert].afWeight[nSlot] = pfVertWeights[nBone];
					pxSkin[nVert].abBoneIndex[nSlot] = (BYTE)nBone;
					break;
				}
			}
		}
		// Drop tiny influences
		for ( nSlot = 0; nSlot < 4; nSlot++ )
		{
			if ( pxSkin[nVert].afWeight[nSlot] < 0.02f ) pxSkin[nVert].afWeight[nSlot] = 0.0f;
			fTotal += pxSkin[nVert].afWeight[nSlot];
		}
		if ( fTotal <= 0.0f )
		{
			pxSkin[nVert].afWeight[0] = 1.0f;
			fTotal = 1.0f;
		}
		for ( nSlot = 0; nSlot < 4; nSlot++ )
		{
			pxSkin[nVert].afWeight[nSlot] /= fTotal;
		}
	}
	SetWeights( nNumVertices, pxSkin );

	free( pxSkin );
	free( pnNeighbours );
	free( pnNeighbourStart );
	free( pnNeighbourCount );
	free( pfNewWeights );
	free( pfBoneDist );
	free( pfBonePhase );
	free( pnLabel );
	free( pfWeights );
	free( pxSegEnd );
	free( pxSegStart );
	free( pnWeldRep );
}

//-----------------------------------------------------------------------------------------------------------
// Animations
//-----------------------------------------------------------------------------------------------------------

int		ModelSkeleton::AddAnimation( const char* szLinkName, const char* szFullPath, int nAnimUse, BOOL bLinkInModel )
{
MODEL_SKELETON_ANIM*	pxAnim;
SkeletalAnimation*		pSource;

	InvalidateSkinCache();

	if ( mnNumAnims >= MODEL_SKELETON_MAX_ANIMS ) return( NOTFOUND );
	if ( mnNumBones == 0 ) return( NOTFOUND );

	pSource = new SkeletalAnimation;
	if ( pSource->LoadBVH( szFullPath ) == FALSE )
	{
		delete pSource;
		return( NOTFOUND );
	}

	pxAnim = &maxAnims[mnNumAnims];
	memset( pxAnim, 0, sizeof( MODEL_SKELETON_ANIM ) );
	strncpy( pxAnim->acFilename, szLinkName, 63 );
	pxAnim->nAnimUse = nAnimUse;
	pxAnim->bLinkInModel = bLinkInModel;
	pxAnim->pSource = pSource;
	BakeAnimation( pxAnim );
	return( mnNumAnims++ );
}

//-------------------------------------------------------------------------
// Function    : BakeAnimation
// Description : Retargets every frame of the source clip onto this skeleton (see ModelSkeleton.h)
//-------------------------------------------------------------------------
void	ModelSkeleton::BakeAnimation( MODEL_SKELETON_ANIM* pxAnim )
{
SkeletalAnimation*	pSource = pxAnim->pSource;
int		nNumJoints = pSource->GetNumJoints();
int		nNumFrames = pSource->GetNumFrames();
int		nFirstFrame = (nNumFrames > 1) ? 1 : 0;		// Frame 0 of mocap BVHs is often a calibration T-pose
int*	pnBoneToJoint;
ENGINEQUATERNION*	pxAlign;
ENGINEQUATERNION*	pxSrcRots;
VECT*	pxSrcPos;
VECT*	pxSrcRest;
ENGINEQUATERNION	xSourceToModelInv = QuatConjugate( &mxSourceToModelAxes );
ENGINEQUATERNION	xYaw;
VECT	xMeanRoot = { 0.0f, 0.0f, 0.0f };
VECT	xHeading = { 0.0f, 0.0f, 0.0f };
float	fScale = 1.0f;
int		nRootJoint;
int		nBone, nFrame;

	pnBoneToJoint = (int*)malloc( mnNumBones * sizeof( int ) );
	pxAlign = (ENGINEQUATERNION*)malloc( mnNumBones * sizeof( ENGINEQUATERNION ) );
	pxSrcRots = (ENGINEQUATERNION*)malloc( nNumJoints * sizeof( ENGINEQUATERNION ) );
	pxSrcPos = (VECT*)malloc( nNumJoints * sizeof( VECT ) );
	pxSrcRest = (VECT*)malloc( nNumJoints * sizeof( VECT ) );

	pSource->GetRestPositions( pxSrcRest );

	// ---- Map bones by name, and work out the per-bone alignment rotations
	for ( nBone = 0; nBone < mnNumBones; nBone++ )
	{
		pnBoneToJoint[nBone] = pSource->FindJoint( mpxBones[nBone].acName );
	}
	for ( nBone = 0; nBone < mnNumBones; nBone++ )
	{
	int		nChild = GetDirectionChild( nBone );
	int		nParent = mpxBones[nBone].nParentIndex;
	BOOL	bAligned = FALSE;

		if ( ( nChild != NOTFOUND ) && ( pnBoneToJoint[nBone] != NOTFOUND ) && ( pnBoneToJoint[nChild] != NOTFOUND ) )
		{
		VECT	xSrcDir;
		VECT	xModelDir;

			VectSub( &xSrcDir, &pxSrcRest[ pnBoneToJoint[nChild] ], &pxSrcRest[ pnBoneToJoint[nBone] ] );
			xSrcDir = QuatRotate( &mxSourceToModelAxes, &xSrcDir );
			VectSub( &xModelDir, &mpxBones[nChild].xBindPos, &mpxBones[nBone].xBindPos );
			if ( ( VectDot( &xSrcDir, &xSrcDir ) > 0.0000001f ) && ( VectDot( &xModelDir, &xModelDir ) > 0.0000001f ) )
			{
				pxAlign[nBone] = QuatFromTo( &xModelDir, &xSrcDir );
				bAligned = TRUE;
			}
		}
		if ( bAligned == FALSE )
		{
			pxAlign[nBone] = ( nParent != NOTFOUND ) ? pxAlign[nParent] : QuatIdentity();
		}
	}

	// ---- Root : find it, average position and heading, and the scale between the two skeletons
	nRootJoint = pnBoneToJoint[0];
	if ( nRootJoint == NOTFOUND ) nRootJoint = 0;
	for ( nFrame = nFirstFrame; nFrame < nNumFrames; nFrame++ )
	{
	ENGINEQUATERNION	xRootRot;
	VECT				xForward = { 1.0f, 0.0f, 0.0f };

		pSource->GetGlobalPose( nFrame, pxSrcRots, pxSrcPos );
		VectAdd( &xMeanRoot, &xMeanRoot, &pxSrcPos[nRootJoint] );
		xRootRot = QuatMul( &mxSourceToModelAxes, &pxSrcRots[nRootJoint] );
		xRootRot = QuatMul( &xRootRot, &xSourceToModelInv );
		xForward = QuatRotate( &xRootRot, &xForward );
		xHeading.x += xForward.x;
		xHeading.y += xForward.y;
	}
	VectScale( &xMeanRoot, &xMeanRoot, 1.0f / (float)( nNumFrames - nFirstFrame ) );
	xYaw = QuatFromAxisAngle( 0.0f, 0.0f, 1.0f, -atan2f( xHeading.y, xHeading.x ) );

	{
	int		nUpLeg = FindBone( "LeftUpLeg" );
	int		nLeg = FindBone( "LeftLeg" );
	int		nFoot = FindBone( "LeftFoot" );

		if ( ( nUpLeg != NOTFOUND ) && ( nLeg != NOTFOUND ) && ( nFoot != NOTFOUND ) &&
			 ( pnBoneToJoint[nUpLeg] != NOTFOUND ) && ( pnBoneToJoint[nLeg] != NOTFOUND ) && ( pnBoneToJoint[nFoot] != NOTFOUND ) )
		{
		float	fModelLen = VectDist( &mpxBones[nUpLeg].xBindPos, &mpxBones[nLeg].xBindPos ) + VectDist( &mpxBones[nLeg].xBindPos, &mpxBones[nFoot].xBindPos );
		float	fSourceLen = VectDist( &pxSrcRest[ pnBoneToJoint[nUpLeg] ], &pxSrcRest[ pnBoneToJoint[nLeg] ] ) + VectDist( &pxSrcRest[ pnBoneToJoint[nLeg] ], &pxSrcRest[ pnBoneToJoint[nFoot] ] );

			if ( fSourceLen > 0.0f ) fScale = fModelLen / fSourceLen;
		}
	}

	// ---- Bake each frame
	pxAnim->nNumFrames = nNumFrames;
	pxAnim->fFrameTime = pSource->GetFrameTime();
	pxAnim->pxBoneRots = (ENGINEQUATERNION*)malloc( nNumFrames * mnNumBones * sizeof( ENGINEQUATERNION ) );
	pxAnim->pxRootPos = (VECT*)malloc( nNumFrames * sizeof( VECT ) );

	for ( nFrame = 0; nFrame < nNumFrames; nFrame++ )
	{
	ENGINEQUATERNION*	pxFrameRots = &pxAnim->pxBoneRots[ nFrame * mnNumBones ];
	VECT				xRootDelta;

		pSource->GetGlobalPose( nFrame, pxSrcRots, pxSrcPos );

		for ( nBone = 0; nBone < mnNumBones; nBone++ )
		{
			if ( pnBoneToJoint[nBone] != NOTFOUND )
			{
			ENGINEQUATERNION	xRot;

				xRot = QuatMul( &mxSourceToModelAxes, &pxSrcRots[ pnBoneToJoint[nBone] ] );
				xRot = QuatMul( &xRot, &xSourceToModelInv );
				xRot = QuatMul( &xYaw, &xRot );
				xRot = QuatMul( &xRot, &pxAlign[nBone] );
				QuatNormalise( &xRot );
				pxFrameRots[nBone] = xRot;
			}
			else if ( mpxBones[nBone].nParentIndex != NOTFOUND )
			{
				pxFrameRots[nBone] = pxFrameRots[ mpxBones[nBone].nParentIndex ];		// unmapped bones follow their parent rigidly
			}
			else
			{
				pxFrameRots[nBone] = xYaw;
			}
		}

		VectSub( &xRootDelta, &pxSrcPos[nRootJoint], &xMeanRoot );
		xRootDelta = QuatRotate( &mxSourceToModelAxes, &xRootDelta );
		xRootDelta = QuatRotate( &xYaw, &xRootDelta );
		VectScale( &xRootDelta, &xRootDelta, fScale );
		VectAdd( &pxAnim->pxRootPos[nFrame], &mpxBones[0].xBindPos, &xRootDelta );
	}

	FindLoop( pxAnim );

	// ---- Ground the feet : lowest foot/toe joint over the loop should match the bind pose
	{
	VECT*	pxJoints = (VECT*)malloc( mnNumBones * sizeof( VECT ) );
	float	fMinBind = 1000000.0f;
	float	fMinAnim = 1000000.0f;
	char	acName[SKELANIM_MAX_NAME_LEN];
	VECT	xRoot;

		for ( nFrame = pxAnim->nLoopStart; nFrame < pxAnim->nLoopEnd; nFrame++ )
		{
			xRoot = pxAnim->pxRootPos[nFrame];
			xRoot.x = mpxBones[0].xBindPos.x;
			xRoot.y = mpxBones[0].xBindPos.y;
			ComputeJointPositions( &pxAnim->pxBoneRots[ nFrame * mnNumBones ], &xRoot, pxJoints );
			for ( nBone = 0; nBone < mnNumBones; nBone++ )
			{
				SkeletalAnimationNormaliseBoneName( mpxBones[nBone].acName, acName, SKELANIM_MAX_NAME_LEN );
				if ( strstr( acName, "foot" ) || strstr( acName, "toe" ) )
				{
					if ( pxJoints[nBone].z < fMinAnim ) fMinAnim = pxJoints[nBone].z;
					if ( mpxBones[nBone].xBindPos.z < fMinBind ) fMinBind = mpxBones[nBone].xBindPos.z;
				}
			}
		}
		if ( ( fMinAnim < 100000.0f ) && ( fMinBind < 100000.0f ) )
		{
			for ( nFrame = 0; nFrame < nNumFrames; nFrame++ )
			{
				pxAnim->pxRootPos[nFrame].z += (fMinBind - fMinAnim);
			}
		}
		free( pxJoints );
	}

	SysDebugPrint( "Skeleton: baked %s - %d frames @ %.1ffps, loop %d-%d, scale %.4f\n", pxAnim->acFilename, nNumFrames, 1.0f / pxAnim->fFrameTime, pxAnim->nLoopStart, pxAnim->nLoopEnd, fScale );

	free( pxSrcRest );
	free( pxSrcPos );
	free( pxSrcRots );
	free( pxAlign );
	free( pnBoneToJoint );
}

//-------------------------------------------------------------------------
// Function    : FindLoop
// Description : Finds the most seamless loop in the clip (two frames with the closest poses, at least
//				 ~0.45s apart). Good enough for mocap run/walk cycles that weren't trimmed to loop.
//-------------------------------------------------------------------------
void	ModelSkeleton::FindLoop( MODEL_SKELETON_ANIM* pxAnim )
{
int		nNumFrames = pxAnim->nNumFrames;
int		nFirst = (nNumFrames > 1) ? 1 : 0;
int		nMinLen = (int)( 0.45f / pxAnim->fFrameTime );
int		nSearchEnd = nNumFrames;
int		nA, nB, nBone;
float	fBestCost = 1000000.0f;

	pxAnim->nLoopStart = nFirst;
	pxAnim->nLoopEnd = nNumFrames;
	if ( nMinLen < 2 ) nMinLen = 2;
	if ( nNumFrames - nFirst < nMinLen + 2 ) return;
	// Long clips (a whole performance rather than a cycle - e.g. a karate kata) play through in full
	if ( (nNumFrames - nFirst) * pxAnim->fFrameTime > 4.0f ) return;
	if ( nSearchEnd > nFirst + 900 ) nSearchEnd = nFirst + 900;

	for ( nA = nFirst; nA < nSearchEnd - nMinLen; nA++ )
	{
		for ( nB = nA + nMinLen; nB < nSearchEnd; nB++ )
		{
		float	fCost = 0.0f;
		float	fDot;

			for ( nBone = 0; nBone < mnNumBones; nBone++ )
			{
			const ENGINEQUATERNION*	pxQA = &pxAnim->pxBoneRots[ (nA * mnNumBones) + nBone ];
			const ENGINEQUATERNION*	pxQB = &pxAnim->pxBoneRots[ (nB * mnNumBones) + nBone ];

				fDot = (pxQA->x*pxQB->x) + (pxQA->y*pxQB->y) + (pxQA->z*pxQB->z) + (pxQA->w*pxQB->w);
				fCost += 1.0f - fabsf( fDot );
			}
			fCost += fabsf( pxAnim->pxRootPos[nA].z - pxAnim->pxRootPos[nB].z ) * 2.0f;
			// Slight preference for longer loops (more variety, less obvious repetition)
			fCost -= (float)( nB - nA ) * 0.00001f;
			if ( fCost < fBestCost )
			{
				fBestCost = fCost;
				pxAnim->nLoopStart = nA;
				pxAnim->nLoopEnd = nB;
			}
		}
	}
}

void	ModelSkeleton::PlayAnimation( int nAnimUse, uint32 ulTick )
{
int		nLoop;

	for ( nLoop = 0; nLoop < mnNumAnims; nLoop++ )
	{
		if ( maxAnims[nLoop].nAnimUse == nAnimUse )
		{
			if ( mnCurrentAnim != nLoop )
			{
				mnCurrentAnim = nLoop;
				mulAnimStartTick = ulTick;
			}
			return;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
// Per frame
//-----------------------------------------------------------------------------------------------------------

void	ModelSkeleton::SamplePose( const MODEL_SKELETON_ANIM* pxAnim, float fTime, ENGINEQUATERNION* pxRotsOut, VECT* pxRootPosOut )
{
int		nLoopLen = pxAnim->nLoopEnd - pxAnim->nLoopStart;
float	fFrame;
float	fPhase;
int		nFrame0;
int		nFrame1;
int		nBone;

	if ( nLoopLen < 1 ) nLoopLen = 1;
	fFrame = fmodf( fTime / pxAnim->fFrameTime, (float)nLoopLen );
	if ( fFrame < 0.0f ) fFrame += nLoopLen;
	nFrame0 = (int)fFrame;
	fPhase = fFrame - nFrame0;
	nFrame0 += pxAnim->nLoopStart;
	nFrame1 = nFrame0 + 1;
	if ( nFrame1 >= pxAnim->nLoopEnd ) nFrame1 = pxAnim->nLoopStart;

	for ( nBone = 0; nBone < mnNumBones; nBone++ )
	{
		pxRotsOut[nBone] = QuatSlerp( &pxAnim->pxBoneRots[ (nFrame0 * mnNumBones) + nBone ], &pxAnim->pxBoneRots[ (nFrame1 * mnNumBones) + nBone ], fPhase );
	}
	VectLerp( pxRootPosOut, &pxAnim->pxRootPos[nFrame0], &pxAnim->pxRootPos[nFrame1], fPhase );
	if ( mbKeepRootMotion == FALSE )
	{
		pxRootPosOut->x = mpxBones[0].xBindPos.x;
		pxRootPosOut->y = mpxBones[0].xBindPos.y;
	}
}

void	ModelSkeleton::ComputeJointPositions( const ENGINEQUATERNION* pxRots, const VECT* pxRootPos, VECT* pxJointsOut )
{
int		nBone;
int		nParent;
VECT	xOffset;

	for ( nBone = 0; nBone < mnNumBones; nBone++ )
	{
		nParent = mpxBones[nBone].nParentIndex;
		if ( nParent == NOTFOUND )
		{
			VectSub( &xOffset, &mpxBones[nBone].xBindPos, &mpxBones[0].xBindPos );
			VectAdd( &pxJointsOut[nBone], pxRootPos, &xOffset );
		}
		else
		{
			VectSub( &xOffset, &mpxBones[nBone].xBindPos, &mpxBones[nParent].xBindPos );
			xOffset = QuatRotate( &pxRots[nParent], &xOffset );
			VectAdd( &pxJointsOut[nBone], &pxJointsOut[nParent], &xOffset );
		}
	}
}

int		ModelSkeleton::GetCurrentJointPositions( VECT* pxOut, int nMax )
{
int		nCount = (mnNumBones < nMax) ? mnNumBones : nMax;

	if ( mpxCurrentJoints == NULL ) return( 0 );
	memcpy( pxOut, mpxCurrentJoints, nCount * sizeof( VECT ) );
	return( nCount );
}

//-------------------------------------------------------------------------
// Function    : UpdateAndSkin
// Description : Samples the current animation and writes the skinned positions/normals into the base mesh
//-------------------------------------------------------------------------
void	ModelSkeleton::UpdateAndSkin( MODEL_RENDER_DATA* pxModelData, uint32 ulTick )
{
CUSTOMVERTEX*	pxVertices = NULL;
VECT			xRootPos;
int				nVert;
int				nSlot;

	if ( ( mnNumBones == 0 ) || ( mnNumAnims == 0 ) || ( mpxWeights == NULL ) ) return;
	if ( mnNumWeights != pxModelData->xStats.nNumVertices ) return;

	CaptureBindMesh( pxModelData );
	if ( mpxBindPositions == NULL ) return;

	if ( mnCurrentAnim == NOTFOUND )
	{
		mnCurrentAnim = 0;
		mulAnimStartTick = ulTick;
	}

	// Animate once per frame (opt-in): the mesh already holds this exact pose, written earlier this frame
	float	fSpeed = (pxModelData->fAnimSpeedMod != 0.0f) ? pxModelData->fAnimSpeedMod : 1.0f;
	if ( ( ModelRenderingAnimateOncePerFrame() == TRUE ) &&
		 ( mpLastSkinMesh == pxModelData->pxBaseMesh ) &&
		 ( mulLastSkinTick == ulTick ) &&
		 ( mnLastSkinAnim == mnCurrentAnim ) &&
		 ( mulLastSkinStartTick == mulAnimStartTick ) &&
		 ( mfLastSkinSpeed == fSpeed ) &&
		 ( mbLastSkinKeepRootMotion == mbKeepRootMotion ) )
	{
		ModelRenderingNoteAnimationBlend( TRUE );
		return;
	}

	SamplePose( &maxAnims[mnCurrentAnim], (float)( ulTick - mulAnimStartTick ) * 0.001f * ( (pxModelData->fAnimSpeedMod != 0.0f) ? pxModelData->fAnimSpeedMod : 1.0f ), mpxCurrentRots, &xRootPos );
	ComputeJointPositions( mpxCurrentRots, &xRootPos, mpxCurrentJoints );

	pxModelData->pxBaseMesh->LockVertexBuffer( kLock_Normal, (byte**)( &pxVertices ) );
	if ( pxVertices == NULL ) return;

	for ( nVert = 0; nVert < mnNumBindVertices; nVert++ )
	{
	const MODEL_SKIN_WEIGHT*	pxSkin = &mpxWeights[nVert];
	VECT	xPos = { 0.0f, 0.0f, 0.0f };
	VECT	xNormal = { 0.0f, 0.0f, 0.0f };

		for ( nSlot = 0; nSlot < 4; nSlot++ )
		{
		float	fWeight = pxSkin->afWeight[nSlot];
		int		nBone = pxSkin->abBoneIndex[nSlot];
		VECT	xLocal;
		VECT	xRotated;

			if ( ( fWeight <= 0.0f ) || ( nBone >= mnNumBones ) ) continue;

			VectSub( &xLocal, &mpxBindPositions[nVert], &mpxBones[nBone].xBindPos );
			xRotated = QuatRotate( &mpxCurrentRots[nBone], &xLocal );
			xPos.x += ( xRotated.x + mpxCurrentJoints[nBone].x ) * fWeight;
			xPos.y += ( xRotated.y + mpxCurrentJoints[nBone].y ) * fWeight;
			xPos.z += ( xRotated.z + mpxCurrentJoints[nBone].z ) * fWeight;

			xRotated = QuatRotate( &mpxCurrentRots[nBone], &mpxBindNormals[nVert] );
			xNormal.x += xRotated.x * fWeight;
			xNormal.y += xRotated.y * fWeight;
			xNormal.z += xRotated.z * fWeight;
		}
		VectNormalize( &xNormal );
		pxVertices[nVert].position = xPos;
		pxVertices[nVert].normal = xNormal;
	}
	pxModelData->pxBaseMesh->UnlockVertexBuffer();

	ModelRenderingNoteAnimationBlend( FALSE );
	mpLastSkinMesh = pxModelData->pxBaseMesh;
	mulLastSkinTick = ulTick;
	mnLastSkinAnim = mnCurrentAnim;
	mulLastSkinStartTick = mulAnimStartTick;
	mfLastSkinSpeed = fSpeed;
	mbLastSkinKeepRootMotion = mbKeepRootMotion;
}

//-----------------------------------------------------------------------------------------------------------
// C interface (see Rendering.h)
//-----------------------------------------------------------------------------------------------------------

static ModelSkeleton*	ModelSkeletonGet( int nModelHandle )
{
	if ( ( nModelHandle < 0 ) || ( nModelHandle >= MAX_MODELS_LOADED ) ) return( NULL );
	return( maxModelRenderData[nModelHandle].pSkeleton );
}

const char*		ModelSkeletonGetHumanoidBoneName( int nHumanoidBone )
{
	if ( ( nHumanoidBone < 0 ) || ( nHumanoidBone >= HUMANOID_NUM_BONES ) ) return( "" );
	return( maszHumanoidBoneNames[nHumanoidBone] );
}

BOOL	ModelHasSkeleton( int nModelHandle )
{
	return( ModelSkeletonGet( nModelHandle ) != NULL );
}

BOOL	ModelSkeletonCreate( int nModelHandle, int nNumBones, const MODEL_SKELETON_BONE* pxBones )
{
MODEL_RENDER_DATA*	pxModelData;
int		nLoop;

	if ( ( nModelHandle < 0 ) || ( nModelHandle >= MAX_MODELS_LOADED ) ) return( FALSE );
	if ( ( nNumBones <= 0 ) || ( nNumBones > MODEL_SKELETON_MAX_BONES ) ) return( FALSE );
	for ( nLoop = 0; nLoop < nNumBones; nLoop++ )
	{
		if ( pxBones[nLoop].nParentIndex >= nLoop ) return( FALSE );		// parents must come first
	}

	pxModelData = &maxModelRenderData[nModelHandle];
	SAFE_DELETE( pxModelData->pSkeleton );
	pxModelData->pSkeleton = new ModelSkeleton;
	pxModelData->pSkeleton->Create( nNumBones, pxBones );
	return( TRUE );
}

BOOL	ModelSkeletonCreateHumanoid( int nModelHandle, const VECT* pxJointPositions )
{
MODEL_SKELETON_BONE		axBones[HUMANOID_NUM_BONES];
int		nLoop;

	for ( nLoop = 0; nLoop < HUMANOID_NUM_BONES; nLoop++ )
	{
		memset( &axBones[nLoop], 0, sizeof( MODEL_SKELETON_BONE ) );
		strcpy( axBones[nLoop].acName, maszHumanoidBoneNames[nLoop] );
		axBones[nLoop].nParentIndex = manHumanoidBoneParents[nLoop];
		axBones[nLoop].xBindPos = pxJointPositions[nLoop];
	}
	return( ModelSkeletonCreate( nModelHandle, HUMANOID_NUM_BONES, axBones ) );
}

//-------------------------------------------------------------------------
// Function    : ModelSkeletonEstimateHumanoidJoints
// Description : Crude proportional guess at joint positions, as fractions of the model's bounds.
//				 Proportions were measured from caveman1.atm - expect to tweak them for other models.
//-------------------------------------------------------------------------
void	ModelSkeletonEstimateHumanoidJoints( int nModelHandle, VECT* pxJointPositionsOut )
{
MODEL_STATS*	pxStats = ModelGetStats( nModelHandle );
// x = forward offset (fraction of height) from body centre, y = side (fraction of height), z = fraction of height
static const float	afProportions[HUMANOID_NUM_BONES][3] =
{
	{ -0.011f, 0.0f, 0.489f },		// Hips
	{ -0.017f, 0.0f, 0.568f },		// Spine
	{  0.000f, 0.0f, 0.682f },		// Spine1
	{  0.011f, 0.0f, 0.807f },		// Neck
	{  0.023f, 0.0f, 0.852f },		// Head
	{ -0.006f, 0.040f, 0.773f },	// LeftShoulder
	{ -0.011f, 0.114f, 0.750f },	// LeftArm
	{ -0.011f, 0.142f, 0.563f },	// LeftForeArm
	{  0.045f, 0.153f, 0.415f },	// LeftHand
	{ -0.006f, -0.040f, 0.773f },	// RightShoulder
	{  0.000f, -0.114f, 0.750f },	// RightArm
	{ -0.051f, -0.134f, 0.574f },	// RightForeArm
	{ -0.026f, -0.142f, 0.423f },	// RightHand
	{ -0.006f, 0.054f, 0.466f },	// LeftUpLeg
	{  0.028f, 0.077f, 0.287f },	// LeftLeg
	{ -0.040f, 0.094f, 0.043f },	// LeftFoot
	{  0.028f, 0.097f, 0.011f },	// LeftToeBase
	{ -0.017f, -0.054f, 0.466f },	// RightUpLeg
	{  0.026f, -0.072f, 0.284f },	// RightLeg
	{ -0.037f, -0.093f, 0.040f },	// RightFoot
	{  0.028f, -0.097f, 0.011f },	// RightToeBase
};
float	fHeight = pxStats->xBoundMax.z - pxStats->xBoundMin.z;
float	fCentreX = pxStats->xBoundMin.x + ( (pxStats->xBoundMax.x - pxStats->xBoundMin.x) * 0.45f );
float	fCentreY = (pxStats->xBoundMin.y + pxStats->xBoundMax.y) * 0.5f;
int		nLoop;

	for ( nLoop = 0; nLoop < HUMANOID_NUM_BONES; nLoop++ )
	{
		pxJointPositionsOut[nLoop].x = fCentreX + (afProportions[nLoop][0] * fHeight);
		pxJointPositionsOut[nLoop].y = fCentreY + (afProportions[nLoop][1] * fHeight);
		pxJointPositionsOut[nLoop].z = pxStats->xBoundMin.z + (afProportions[nLoop][2] * fHeight);
	}
}

void	ModelSkeletonFree( int nModelHandle )
{
	if ( ( nModelHandle < 0 ) || ( nModelHandle >= MAX_MODELS_LOADED ) ) return;
	SAFE_DELETE( maxModelRenderData[nModelHandle].pSkeleton );
}

int		ModelSkeletonGetNumBones( int nModelHandle )
{
ModelSkeleton*	pSkeleton = ModelSkeletonGet( nModelHandle );
	return( pSkeleton ? pSkeleton->GetNumBones() : 0 );
}

const MODEL_SKELETON_BONE*	ModelSkeletonGetBone( int nModelHandle, int nBoneNum )
{
ModelSkeleton*	pSkeleton = ModelSkeletonGet( nModelHandle );

	if ( ( pSkeleton == NULL ) || ( nBoneNum < 0 ) || ( nBoneNum >= pSkeleton->GetNumBones() ) ) return( NULL );
	return( pSkeleton->GetBone( nBoneNum ) );
}

const MODEL_SKIN_WEIGHT*	ModelSkeletonGetVertexWeights( int nModelHandle, int* pnNumWeightsOut )
{
ModelSkeleton*	pSkeleton = ModelSkeletonGet( nModelHandle );

	if ( pnNumWeightsOut ) *pnNumWeightsOut = pSkeleton ? pSkeleton->GetNumWeights() : 0;
	return( pSkeleton ? pSkeleton->GetWeights() : NULL );
}

void	ModelSkeletonSetVertexWeights( int nModelHandle, const MODEL_SKIN_WEIGHT* pxWeights )
{
ModelSkeleton*	pSkeleton = ModelSkeletonGet( nModelHandle );

	if ( pSkeleton == NULL ) return;
	pSkeleton->SetWeights( maxModelRenderData[nModelHandle].xStats.nNumVertices, pxWeights );
}

void	ModelSkeletonAutoGenerateWeights( int nModelHandle, int nSmoothingPasses )
{
ModelSkeleton*	pSkeleton = ModelSkeletonGet( nModelHandle );

	if ( pSkeleton == NULL ) return;
	pSkeleton->AutoGenerateWeights( &maxModelRenderData[nModelHandle], nSmoothingPasses );
}

int		ModelSkeletonAddAnimation( int nModelHandle, const char* szAnimFilename, int nAnimUse, BOOL bLinkInModel )
{
	if ( ( nModelHandle < 0 ) || ( nModelHandle >= MAX_MODELS_LOADED ) ) return( NOTFOUND );
	return( ModelSkeletonAddAnimationForModel( &maxModelRenderData[nModelHandle], szAnimFilename, nAnimUse, bLinkInModel ) );
}

int		ModelSkeletonAddAnimationForModel( MODEL_RENDER_DATA* pxModelData, const char* szAnimFilename, int nAnimUse, BOOL bLinkInModel )
{
ModelSkeleton*	pSkeleton = pxModelData->pSkeleton;
char	acFullPath[256];
char	acLinkName[256];

	if ( pSkeleton == NULL ) return( NOTFOUND );

	if ( SysIsAbsolutePath( szAnimFilename ) || SysFileDoesExist( szAnimFilename ) )
	{
		strncpy( acFullPath, szAnimFilename, 255 );
		acFullPath[255] = 0;
		SysExtractFilenameFromPath( szAnimFilename, acLinkName );
	}
	else
	{
		SysExtractPathFromFilename( pxModelData->xStats.acFilename, acFullPath );
		if ( ( acFullPath[0] != 0 ) && ( acFullPath[ strlen(acFullPath) - 1 ] != '\\' ) && ( acFullPath[ strlen(acFullPath) - 1 ] != '/' ) )
		{
			strcat( acFullPath, "\\" );
		}
		strncat( acFullPath, szAnimFilename, 255 - strlen( acFullPath ) );
		strncpy( acLinkName, szAnimFilename, 255 );
		acLinkName[255] = 0;
	}
	return( pSkeleton->AddAnimation( acLinkName, acFullPath, nAnimUse, bLinkInModel ) );
}

int		ModelSkeletonGetNumAnimations( int nModelHandle )
{
ModelSkeleton*	pSkeleton = ModelSkeletonGet( nModelHandle );
	return( pSkeleton ? pSkeleton->GetNumAnimations() : 0 );
}

void	ModelSkeletonSetRootMotion( int nModelHandle, BOOL bKeepHorizontalRootMotion )
{
ModelSkeleton*	pSkeleton = ModelSkeletonGet( nModelHandle );
	if ( pSkeleton ) pSkeleton->SetKeepRootMotion( bKeepHorizontalRootMotion );
}

int		ModelSkeletonGetCurrentJointPositions( int nModelHandle, VECT* pxPositionsOut, int nMaxBones )
{
ModelSkeleton*	pSkeleton = ModelSkeletonGet( nModelHandle );
	return( pSkeleton ? pSkeleton->GetCurrentJointPositions( pxPositionsOut, nMaxBones ) : 0 );
}

void	ModelRemoveVertexAnimation( int nModelHandle )
{
MODEL_RENDER_DATA*	pxModelData;

	if ( ( nModelHandle < 0 ) || ( nModelHandle >= MAX_MODELS_LOADED ) ) return;
	pxModelData = &maxModelRenderData[nModelHandle];

	if ( pxModelData->pxVertexKeyframes ) SystemFree( pxModelData->pxVertexKeyframes );
	if ( pxModelData->pxNormalKeyframes ) SystemFree( pxModelData->pxNormalKeyframes );
	pxModelData->pxVertexKeyframes = NULL;
	pxModelData->pxNormalKeyframes = NULL;
	pxModelData->xStats.nNumVertKeyframes = 0;
	memset( pxModelData->axKeyframeData, 0, sizeof( pxModelData->axKeyframeData ) );
	pxModelData->bModelType = ASSETTYPE_STATIC_MESH;
}
