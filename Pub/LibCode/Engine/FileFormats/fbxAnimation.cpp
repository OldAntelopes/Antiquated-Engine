#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>

#ifdef TOOL

// (Avoids the SDK's static data members - FbxAxisSystem::MayaYUp, ClassIds etc - which don't link with the
//  way the FBX SDK import library is used by the existing fbxLoader code)
#include <fbxsdk.h>

#include "../Skeleton/SkeletonMaths.h"
#include "../Skeleton/SkeletalAnimation.h"
#include "fbxAnimation.h"

#define		FBXANIM_MAX_JOINTS		256

typedef struct
{
	FbxNode*			pNode;
	int					nParent;
	ENGINEQUATERNION	xBindRot;
	VECT				xBindPos;

} FBXANIM_JOINT;

//-----------------------------------------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------------------------------------

static void		FbxAnimationSetError( char* szErrorOut, int nErrorLen, const char* szText )
{
	if ( szErrorOut )
	{
		strncpy( szErrorOut, szText, nErrorLen - 1 );
		szErrorOut[nErrorLen - 1] = 0;
	}
	SysDebugPrint( "FbxAnimation: %s\n", szText );
}

static void		FbxAnimationGetMatrixRotPos( const FbxAMatrix& xMatrix, ENGINEQUATERNION* pxRot, VECT* pxPos )
{
FbxQuaternion	xQuat = xMatrix.GetQ();
FbxVector4		xTrans = xMatrix.GetT();

	pxRot->x = (float)xQuat[0];
	pxRot->y = (float)xQuat[1];
	pxRot->z = (float)xQuat[2];
	pxRot->w = (float)xQuat[3];
	QuatNormalise( pxRot );
	pxPos->x = (float)xTrans[0];
	pxPos->y = (float)xTrans[1];
	pxPos->z = (float)xTrans[2];
}

// Euler angles (degrees) for R = Rz(z) * Ry(y) * Rx(x)  - i.e. BVH channel order "Zrotation Yrotation Xrotation",
// which is also FBX's default eEulerXYZ rotation order
static void		FbxAnimationQuatToEulerZYX( const ENGINEQUATERNION* pxQuat, float* pfX, float* pfY, float* pfZ )
{
float	fM00 = 1.0f - 2.0f * ( (pxQuat->y * pxQuat->y) + (pxQuat->z * pxQuat->z) );
float	fM10 = 2.0f * ( (pxQuat->x * pxQuat->y) + (pxQuat->w * pxQuat->z) );
float	fM20 = 2.0f * ( (pxQuat->x * pxQuat->z) - (pxQuat->w * pxQuat->y) );
float	fM21 = 2.0f * ( (pxQuat->y * pxQuat->z) + (pxQuat->w * pxQuat->x) );
float	fM22 = 1.0f - 2.0f * ( (pxQuat->x * pxQuat->x) + (pxQuat->y * pxQuat->y) );

	if ( fM20 > 1.0f ) fM20 = 1.0f;
	if ( fM20 < -1.0f ) fM20 = -1.0f;
	*pfY = RADTODEG( asinf( -fM20 ) );
	*pfX = RADTODEG( atan2f( fM21, fM22 ) );
	*pfZ = RADTODEG( atan2f( fM10, fM00 ) );
}

static int		FbxAnimationCountSkeletonDescendants( FbxNode* pNode )
{
int		nCount = 0;
int		nLoop;

	for ( nLoop = 0; nLoop < pNode->GetChildCount(); nLoop++ )
	{
	FbxNode*	pChild = pNode->GetChild( nLoop );

		if ( ( pChild->GetNodeAttribute() ) && ( pChild->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton ) )
		{
			nCount += 1 + FbxAnimationCountSkeletonDescendants( pChild );
		}
	}
	return( nCount );
}

static FbxNode*	FbxAnimationFindSkeletonRoot( FbxNode* pNode, FbxNode* pBest, int* pnBestCount )
{
int		nLoop;

	if ( ( pNode->GetNodeAttribute() ) && ( pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton ) )
	{
	int		nCount = FbxAnimationCountSkeletonDescendants( pNode );

		if ( nCount > *pnBestCount )
		{
			*pnBestCount = nCount;
			pBest = pNode;
		}
		return( pBest );		// don't search inside a skeleton - its children aren't roots
	}
	for ( nLoop = 0; nLoop < pNode->GetChildCount(); nLoop++ )
	{
		pBest = FbxAnimationFindSkeletonRoot( pNode->GetChild( nLoop ), pBest, pnBestCount );
	}
	return( pBest );
}

// Depth-first so the joint list matches BVH file order
static void		FbxAnimationCollectJoints( FbxNode* pNode, int nParent, FBXANIM_JOINT* pxJoints, int* pnNumJoints )
{
int		nThis = *pnNumJoints;
int		nLoop;

	if ( nThis >= FBXANIM_MAX_JOINTS ) return;
	pxJoints[nThis].pNode = pNode;
	pxJoints[nThis].nParent = nParent;
	(*pnNumJoints)++;

	for ( nLoop = 0; nLoop < pNode->GetChildCount(); nLoop++ )
	{
	FbxNode*	pChild = pNode->GetChild( nLoop );

		if ( ( pChild->GetNodeAttribute() ) && ( pChild->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton ) )
		{
			FbxAnimationCollectJoints( pChild, nThis, pxJoints, pnNumJoints );
		}
	}
}

//-------------------------------------------------------------------------
// Function    : FbxAnimationConvertToBVH
// Description : See fbxAnimation.h
//				 Per joint j with bind global rotation B_j :  D_j(t) = G_j(t) * B_j^-1  (world-space delta)
//				 BVH local rotation = D_parent^-1 * D_j , offsets = bind position differences in world axes
//-------------------------------------------------------------------------
BOOL		FbxAnimationConvertToBVH( const char* szFBXFilename, const char* szBVHFilename, float fSampleFPS, char* szErrorOut, int nErrorLen )
{
FbxManager*		pManager = FbxManager::Create();
FbxIOSettings*	pIOSettings = FbxIOSettings::Create( pManager, IOSROOT );
FbxImporter*	pImporter;
FbxScene*		pScene;
FbxAnimStack*	pAnimStack;
FbxNode*		pRoot;
FBXANIM_JOINT*	pxJoints;
SKELANIM_JOINT*	pxBVHJoints;
ENGINEQUATERNION*	pxDeltas;
float*			pfFrameData;
int				nNumJoints = 0;
int				nBestCount = -1;
int				nNumFrames;
int				nTotalChannels;
int				nFrame;
int				nJoint;
double			dStart;
double			dDuration;
char			acError[512];
BOOL			bResult = FALSE;

	if ( fSampleFPS <= 0.0f ) fSampleFPS = 30.0f;
	pManager->SetIOSettings( pIOSettings );
	pImporter = FbxImporter::Create( pManager, "" );
	if ( pImporter->Initialize( szFBXFilename, -1, pManager->GetIOSettings() ) == false )
	{
		sprintf( acError, "Couldn't open %s : %s", szFBXFilename, pImporter->GetStatus().GetErrorString() );
		FbxAnimationSetError( szErrorOut, nErrorLen, acError );
		pManager->Destroy();
		return( FALSE );
	}
	if ( pImporter->GetAnimStackCount() < 1 )
	{
		FbxAnimationSetError( szErrorOut, nErrorLen, "No animation found in the FBX file" );
		pImporter->Destroy();
		pManager->Destroy();
		return( FALSE );
	}
	{
	FbxTakeInfo*	pTakeInfo = pImporter->GetTakeInfo( 0 );

		dStart = pTakeInfo->mLocalTimeSpan.GetStart().GetSecondDouble();
		dDuration = pTakeInfo->mLocalTimeSpan.GetDuration().GetSecondDouble();
	}
	pScene = FbxScene::Create( pManager, "AnimScene" );
	pImporter->Import( pScene );
	pImporter->Destroy();

	// BVH convention : Y up, right handed (the Engine converts to its own Z-up axes when retargeting)
	FbxAxisSystem( FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded ).ConvertScene( pScene );

	pRoot = FbxAnimationFindSkeletonRoot( pScene->GetRootNode(), NULL, &nBestCount );
	if ( pRoot == NULL )
	{
		FbxAnimationSetError( szErrorOut, nErrorLen, "No skeleton found in the FBX file" );
		pManager->Destroy();
		return( FALSE );
	}

	// (The importer makes the first take the scene's current animation stack, which is what EvaluateGlobalTransform uses)
	pAnimStack = pScene->GetCurrentAnimationStack();
	if ( pAnimStack == NULL )
	{
		SysDebugPrint( "FbxAnimation: no current animation stack - sampling the default stack\n" );
	}
	nNumFrames = (int)( dDuration * fSampleFPS ) + 1;
	if ( nNumFrames < 1 ) nNumFrames = 1;

	pxJoints = (FBXANIM_JOINT*)malloc( FBXANIM_MAX_JOINTS * sizeof( FBXANIM_JOINT ) );
	FbxAnimationCollectJoints( pRoot, NOTFOUND, pxJoints, &nNumJoints );

	// ---- Bind (default, un-animated) pose
	for ( nJoint = 0; nJoint < nNumJoints; nJoint++ )
	{
		FbxAnimationGetMatrixRotPos( pxJoints[nJoint].pNode->EvaluateGlobalTransform( FBXSDK_TIME_INFINITE ), &pxJoints[nJoint].xBindRot, &pxJoints[nJoint].xBindPos );
	}

	// ---- BVH hierarchy
	pxBVHJoints = (SKELANIM_JOINT*)malloc( nNumJoints * sizeof( SKELANIM_JOINT ) );
	memset( pxBVHJoints, 0, nNumJoints * sizeof( SKELANIM_JOINT ) );
	nTotalChannels = 0;
	for ( nJoint = 0; nJoint < nNumJoints; nJoint++ )
	{
	SKELANIM_JOINT*	pxBVHJoint = &pxBVHJoints[nJoint];
	int		nChannel = 0;

		strncpy( pxBVHJoint->acName, pxJoints[nJoint].pNode->GetName(), SKELANIM_MAX_NAME_LEN - 1 );
		pxBVHJoint->nParentIndex = pxJoints[nJoint].nParent;
		pxBVHJoint->nChannelStart = nTotalChannels;
		if ( pxJoints[nJoint].nParent == NOTFOUND )
		{
			pxBVHJoint->anChannelTypes[nChannel++] = BVH_CHANNEL_XPOS;
			pxBVHJoint->anChannelTypes[nChannel++] = BVH_CHANNEL_YPOS;
			pxBVHJoint->anChannelTypes[nChannel++] = BVH_CHANNEL_ZPOS;
		}
		else
		{
			VectSub( &pxBVHJoint->xOffset, &pxJoints[nJoint].xBindPos, &pxJoints[ pxJoints[nJoint].nParent ].xBindPos );
		}
		pxBVHJoint->anChannelTypes[nChannel++] = BVH_CHANNEL_ZROT;
		pxBVHJoint->anChannelTypes[nChannel++] = BVH_CHANNEL_YROT;
		pxBVHJoint->anChannelTypes[nChannel++] = BVH_CHANNEL_XROT;
		pxBVHJoint->nNumChannels = nChannel;
		nTotalChannels += nChannel;
	}

	// ---- Sample the frames
	pfFrameData = (float*)malloc( nNumFrames * nTotalChannels * sizeof( float ) );
	pxDeltas = (ENGINEQUATERNION*)malloc( nNumJoints * sizeof( ENGINEQUATERNION ) );
	for ( nFrame = 0; nFrame < nNumFrames; nFrame++ )
	{
	FbxTime		xTime;
	float*		pfOut = pfFrameData + (nFrame * nTotalChannels);

		xTime.SetSecondDouble( dStart + ( nFrame / (double)fSampleFPS ) );
		for ( nJoint = 0; nJoint < nNumJoints; nJoint++ )
		{
		ENGINEQUATERNION	xRot;
		ENGINEQUATERNION	xBindInv = QuatConjugate( &pxJoints[nJoint].xBindRot );
		ENGINEQUATERNION	xLocal;
		VECT				xPos;
		float				fX, fY, fZ;

			FbxAnimationGetMatrixRotPos( pxJoints[nJoint].pNode->EvaluateGlobalTransform( xTime ), &xRot, &xPos );
			pxDeltas[nJoint] = QuatMul( &xRot, &xBindInv );
			if ( pxJoints[nJoint].nParent == NOTFOUND )
			{
				xLocal = pxDeltas[nJoint];
				*(pfOut++) = xPos.x;
				*(pfOut++) = xPos.y;
				*(pfOut++) = xPos.z;
			}
			else
			{
			ENGINEQUATERNION	xParentInv = QuatConjugate( &pxDeltas[ pxJoints[nJoint].nParent ] );

				xLocal = QuatMul( &xParentInv, &pxDeltas[nJoint] );
			}
			QuatNormalise( &xLocal );
			FbxAnimationQuatToEulerZYX( &xLocal, &fX, &fY, &fZ );
			*(pfOut++) = fZ;
			*(pfOut++) = fY;
			*(pfOut++) = fX;
		}
	}

	bResult = SkeletalAnimation::SaveBVH( szBVHFilename, pxBVHJoints, nNumJoints, pfFrameData, nNumFrames, 1.0f / fSampleFPS );
	if ( bResult == FALSE )
	{
		sprintf( acError, "Couldn't write %s", szBVHFilename );
		FbxAnimationSetError( szErrorOut, nErrorLen, acError );
	}
	else
	{
		SysDebugPrint( "FbxAnimation: %s -> %s : %d joints, %d frames\n", szFBXFilename, szBVHFilename, nNumJoints, nNumFrames );
	}

	free( pxDeltas );
	free( pfFrameData );
	free( pxBVHJoints );
	free( pxJoints );
	pManager->Destroy();
	return( bResult );
}

//-------------------------------------------------------------------------
// Function    : FbxAnimationCreateFromBVH
// Description : Writes a BVH animation out as an FBX skeleton with baked rotation keys (and root position
//				 keys), so mocap can be taken into DCC tools - and used to round-trip test the importer.
//-------------------------------------------------------------------------
BOOL		FbxAnimationCreateFromBVH( const char* szBVHFilename, const char* szFBXFilename, char* szErrorOut, int nErrorLen )
{
SkeletalAnimation	xAnim;
FbxManager*		pManager;
FbxIOSettings*	pIOSettings;
FbxScene*		pScene;
FbxExporter*	pExporter;
FbxAnimStack*	pAnimStack;
FbxAnimLayer*	pAnimLayer;
FbxNode**		ppNodes;
ENGINEQUATERNION*	pxRots;
VECT*			pxPositions;
int				nNumJoints;
int				nJoint;
int				nFrame;
int				nAxis;
FbxTime			xTime;
char			acError[512];
BOOL			bResult;

	if ( xAnim.LoadBVH( szBVHFilename ) == FALSE )
	{
		sprintf( acError, "Couldn't load %s", szBVHFilename );
		FbxAnimationSetError( szErrorOut, nErrorLen, acError );
		return( FALSE );
	}
	nNumJoints = xAnim.GetNumJoints();

	pManager = FbxManager::Create();
	pIOSettings = FbxIOSettings::Create( pManager, IOSROOT );
	pManager->SetIOSettings( pIOSettings );
	pScene = FbxScene::Create( pManager, "BVHScene" );		// (new scenes default to Y-up, right handed - same as BVH)

	ppNodes = (FbxNode**)malloc( nNumJoints * sizeof( FbxNode* ) );
	for ( nJoint = 0; nJoint < nNumJoints; nJoint++ )
	{
	const SKELANIM_JOINT*	pxJoint = xAnim.GetJoint( nJoint );
	FbxSkeleton*			pSkeleton = FbxSkeleton::Create( pScene, pxJoint->acName );

		pSkeleton->SetSkeletonType( (pxJoint->nParentIndex == NOTFOUND) ? FbxSkeleton::eRoot : FbxSkeleton::eLimbNode );
		ppNodes[nJoint] = FbxNode::Create( pScene, pxJoint->acName );
		ppNodes[nJoint]->SetNodeAttribute( pSkeleton );
		ppNodes[nJoint]->LclTranslation.Set( FbxDouble3( pxJoint->xOffset.x, pxJoint->xOffset.y, pxJoint->xOffset.z ) );
		if ( pxJoint->nParentIndex == NOTFOUND )
		{
			pScene->GetRootNode()->AddChild( ppNodes[nJoint] );
		}
		else
		{
			ppNodes[ pxJoint->nParentIndex ]->AddChild( ppNodes[nJoint] );
		}
	}

	pAnimStack = FbxAnimStack::Create( pScene, "Take 001" );
	pAnimLayer = FbxAnimLayer::Create( pScene, "Base Layer" );
	pAnimStack->AddMember( pAnimLayer );

	pxRots = (ENGINEQUATERNION*)malloc( nNumJoints * sizeof( ENGINEQUATERNION ) );
	pxPositions = (VECT*)malloc( nNumJoints * sizeof( VECT ) );
	for ( nJoint = 0; nJoint < nNumJoints; nJoint++ )
	{
	const SKELANIM_JOINT*	pxJoint = xAnim.GetJoint( nJoint );
	FbxAnimCurve*	apRotCurves[3];
	FbxAnimCurve*	apPosCurves[3] = { NULL, NULL, NULL };
	const char*		aszComponents[3] = { FBXSDK_CURVENODE_COMPONENT_X, FBXSDK_CURVENODE_COMPONENT_Y, FBXSDK_CURVENODE_COMPONENT_Z };

		for ( nAxis = 0; nAxis < 3; nAxis++ )
		{
			apRotCurves[nAxis] = ppNodes[nJoint]->LclRotation.GetCurve( pAnimLayer, aszComponents[nAxis], true );
			apRotCurves[nAxis]->KeyModifyBegin();
			if ( pxJoint->nParentIndex == NOTFOUND )
			{
				apPosCurves[nAxis] = ppNodes[nJoint]->LclTranslation.GetCurve( pAnimLayer, aszComponents[nAxis], true );
				apPosCurves[nAxis]->KeyModifyBegin();
			}
		}

		for ( nFrame = 0; nFrame < xAnim.GetNumFrames(); nFrame++ )
		{
		ENGINEQUATERNION	xLocal;
		float				afEuler[3];

			// (Recomputing the whole pose per joint is wasteful, but it's a one-off tool conversion)
			xAnim.GetGlobalPose( nFrame, pxRots, pxPositions );
			if ( pxJoint->nParentIndex == NOTFOUND )
			{
				xLocal = pxRots[nJoint];
			}
			else
			{
			ENGINEQUATERNION	xParentInv = QuatConjugate( &pxRots[ pxJoint->nParentIndex ] );

				xLocal = QuatMul( &xParentInv, &pxRots[nJoint] );
			}
			FbxAnimationQuatToEulerZYX( &xLocal, &afEuler[0], &afEuler[1], &afEuler[2] );
			xTime.SetSecondDouble( nFrame * xAnim.GetFrameTime() );
			for ( nAxis = 0; nAxis < 3; nAxis++ )
			{
			int		nKey = apRotCurves[nAxis]->KeyAdd( xTime );

				apRotCurves[nAxis]->KeySetValue( nKey, afEuler[nAxis] );
				apRotCurves[nAxis]->KeySetInterpolation( nKey, FbxAnimCurveDef::eInterpolationLinear );
				if ( apPosCurves[nAxis] )
				{
				float	afPos[3] = { pxPositions[nJoint].x, pxPositions[nJoint].y, pxPositions[nJoint].z };

					nKey = apPosCurves[nAxis]->KeyAdd( xTime );
					apPosCurves[nAxis]->KeySetValue( nKey, afPos[nAxis] );
					apPosCurves[nAxis]->KeySetInterpolation( nKey, FbxAnimCurveDef::eInterpolationLinear );
				}
			}
		}
		for ( nAxis = 0; nAxis < 3; nAxis++ )
		{
			apRotCurves[nAxis]->KeyModifyEnd();
			if ( apPosCurves[nAxis] ) apPosCurves[nAxis]->KeyModifyEnd();
		}
	}
	{
	FbxTime		xStop;

		xTime.SetSecondDouble( 0.0 );
		xStop.SetSecondDouble( (xAnim.GetNumFrames() - 1) * xAnim.GetFrameTime() );
		pAnimStack->SetLocalTimeSpan( FbxTimeSpan( xTime, xStop ) );
	}

	pExporter = FbxExporter::Create( pManager, "" );
	bResult = pExporter->Initialize( szFBXFilename, pManager->GetIOPluginRegistry()->GetNativeWriterFormat(), pManager->GetIOSettings() );
	if ( bResult )
	{
		bResult = pExporter->Export( pScene );
	}
	if ( bResult == false )
	{
		sprintf( acError, "Couldn't write %s : %s", szFBXFilename, pExporter->GetStatus().GetErrorString() );
		FbxAnimationSetError( szErrorOut, nErrorLen, acError );
	}
	pExporter->Destroy();

	free( pxPositions );
	free( pxRots );
	free( ppNodes );
	pManager->Destroy();
	return( bResult );
}

#endif // TOOL
