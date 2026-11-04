
#include <ctype.h>
#include <StandardDef.h>
#include <Engine.h>

#include "SkeletonMaths.h"
#include "SkeletalAnimation.h"

//-----------------------------------------------------------------------------------------------------------

void		SkeletalAnimationNormaliseBoneName( const char* szIn, char* szOut, int nOutLen )
{
const char*		pcColon = strrchr( szIn, ':' );
int				nLoop = 0;

	if ( pcColon ) szIn = pcColon + 1;		// strip "mixamorig:" style prefixes
	while ( ( *szIn != 0 ) && ( nLoop < nOutLen - 1 ) )
	{
		if ( ( *szIn != '_' ) && ( *szIn != ' ' ) )
		{
			szOut[nLoop++] = (char)tolower( (unsigned char)*szIn );
		}
		szIn++;
	}
	szOut[nLoop] = 0;
}

//-----------------------------------------------------------------------------------------------------------
// Minimal tokeniser for the BVH text format
//-----------------------------------------------------------------------------------------------------------
class BVHTokeniser
{
public:
	BVHTokeniser( const char* pcText, int nLen ) { mpcRunner = pcText; mpcEnd = pcText + nLen; }

	BOOL	Next( char* szTokenOut, int nMaxLen )
	{
	int		nLen = 0;

		while ( ( mpcRunner < mpcEnd ) && ( (unsigned char)*mpcRunner <= ' ' ) ) mpcRunner++;
		if ( mpcRunner >= mpcEnd ) return( FALSE );
		while ( ( mpcRunner < mpcEnd ) && ( (unsigned char)*mpcRunner > ' ' ) )
		{
			if ( nLen < nMaxLen - 1 ) szTokenOut[nLen++] = *mpcRunner;
			mpcRunner++;
		}
		szTokenOut[nLen] = 0;
		return( TRUE );
	}

	float	NextFloat( void )
	{
	char	acToken[64];

		if ( Next( acToken, 64 ) == FALSE ) return( 0.0f );
		return( (float)atof( acToken ) );
	}

private:
	const char*		mpcRunner;
	const char*		mpcEnd;
};

//-----------------------------------------------------------------------------------------------------------

SkeletalAnimation::SkeletalAnimation()
{
	macFilename[0] = 0;
	mpxJoints = NULL;
	mnNumJoints = 0;
	mnMaxJoints = 0;
	mnTotalChannels = 0;
	mnNumFrames = 0;
	mfFrameTime = 1.0f / 30.0f;
	mpfFrameData = NULL;
}

SkeletalAnimation::~SkeletalAnimation()
{
	Free();
}

void	SkeletalAnimation::Free( void )
{
	SAFE_FREE( mpxJoints );
	SAFE_FREE( mpfFrameData );
	mnNumJoints = 0;
	mnMaxJoints = 0;
	mnTotalChannels = 0;
	mnNumFrames = 0;
}

int		SkeletalAnimation::AddJoint( const char* szName, int nParent )
{
SKELANIM_JOINT*		pxJoint;

	if ( mnNumJoints >= mnMaxJoints )
	{
		mnMaxJoints += 32;
		mpxJoints = (SKELANIM_JOINT*)realloc( mpxJoints, mnMaxJoints * sizeof( SKELANIM_JOINT ) );
	}
	pxJoint = &mpxJoints[mnNumJoints];
	memset( pxJoint, 0, sizeof( SKELANIM_JOINT ) );
	strncpy( pxJoint->acName, szName, SKELANIM_MAX_NAME_LEN - 1 );
	pxJoint->nParentIndex = nParent;
	return( mnNumJoints++ );
}

int		SkeletalAnimation::FindJoint( const char* szName )
{
char	acWanted[SKELANIM_MAX_NAME_LEN];
char	acJoint[SKELANIM_MAX_NAME_LEN];
int		nLoop;

	SkeletalAnimationNormaliseBoneName( szName, acWanted, SKELANIM_MAX_NAME_LEN );
	for ( nLoop = 0; nLoop < mnNumJoints; nLoop++ )
	{
		SkeletalAnimationNormaliseBoneName( mpxJoints[nLoop].acName, acJoint, SKELANIM_MAX_NAME_LEN );
		if ( strcmp( acWanted, acJoint ) == 0 )
		{
			return( nLoop );
		}
	}
	return( NOTFOUND );
}

//-------------------------------------------------------------------------
// Function    : LoadBVH
// Description : Loads a .bvh (Biovision hierarchy) animation file
//-------------------------------------------------------------------------
BOOL	SkeletalAnimation::LoadBVH( const char* szFilename )
{
int		nFileSize = 0;
byte*	pbFile = SysFileAllocateAndLoad( szFilename, &nFileSize );
BOOL	bResult;

	if ( pbFile == NULL )
	{
		SysDebugPrint( "SkeletalAnimation: Couldn't load %s\n", szFilename );
		return( FALSE );
	}
	bResult = LoadBVHFromMem( (const char*)pbFile, nFileSize );
	SystemFree( pbFile );

	strncpy( macFilename, szFilename, 255 );
	macFilename[255] = 0;
	if ( bResult == FALSE )
	{
		SysDebugPrint( "SkeletalAnimation: Error parsing %s\n", szFilename );
	}
	return( bResult );
}

BOOL	SkeletalAnimation::LoadBVHFromMem( const char* pcText, int nTextLen )
{
BVHTokeniser	xTokens( pcText, nTextLen );
char	acToken[256];
int		anJointStack[256];
int		nStackDepth = 0;
int		nCurrentJoint = NOTFOUND;
BOOL	bInEndSite = FALSE;
int		nLoop;
int		nNumValues;
SKELANIM_JOINT*		pxJoint;

	Free();

	if ( ( xTokens.Next( acToken, 256 ) == FALSE ) || ( tinstricmp( acToken, "HIERARCHY" ) != 0 ) )
	{
		return( FALSE );
	}

	// ---- Hierarchy
	while ( xTokens.Next( acToken, 256 ) )
	{
		if ( ( tinstricmp( acToken, "ROOT" ) == 0 ) || ( tinstricmp( acToken, "JOINT" ) == 0 ) )
		{
			xTokens.Next( acToken, 256 );
			nCurrentJoint = AddJoint( acToken, (nStackDepth > 0) ? anJointStack[nStackDepth-1] : NOTFOUND );
		}
		else if ( tinstricmp( acToken, "End" ) == 0 )
		{
			xTokens.Next( acToken, 256 );		// "Site"
			bInEndSite = TRUE;
		}
		else if ( strcmp( acToken, "{" ) == 0 )
		{
			if ( bInEndSite == FALSE )
			{
				if ( nStackDepth >= 256 ) return( FALSE );
				anJointStack[nStackDepth++] = nCurrentJoint;
			}
		}
		else if ( strcmp( acToken, "}" ) == 0 )
		{
			if ( bInEndSite )
			{
				bInEndSite = FALSE;
			}
			else if ( nStackDepth > 0 )
			{
				nStackDepth--;
			}
		}
		else if ( tinstricmp( acToken, "OFFSET" ) == 0 )
		{
		VECT	xOffset;

			xOffset.x = xTokens.NextFloat();
			xOffset.y = xTokens.NextFloat();
			xOffset.z = xTokens.NextFloat();
			if ( ( nStackDepth > 0 ) && ( anJointStack[nStackDepth-1] != NOTFOUND ) )
			{
				pxJoint = &mpxJoints[ anJointStack[nStackDepth-1] ];
				if ( bInEndSite )
				{
					pxJoint->bHasEndSite = TRUE;
					pxJoint->xEndSiteOffset = xOffset;
				}
				else
				{
					pxJoint->xOffset = xOffset;
				}
			}
		}
		else if ( tinstricmp( acToken, "CHANNELS" ) == 0 )
		{
			if ( ( nStackDepth <= 0 ) || ( anJointStack[nStackDepth-1] == NOTFOUND ) ) return( FALSE );
			pxJoint = &mpxJoints[ anJointStack[nStackDepth-1] ];
			pxJoint->nNumChannels = (int)xTokens.NextFloat();
			if ( pxJoint->nNumChannels > 6 ) return( FALSE );
			pxJoint->nChannelStart = mnTotalChannels;
			mnTotalChannels += pxJoint->nNumChannels;
			for ( nLoop = 0; nLoop < pxJoint->nNumChannels; nLoop++ )
			{
				xTokens.Next( acToken, 256 );
				if ( tinstricmp( acToken, "Xposition" ) == 0 ) pxJoint->anChannelTypes[nLoop] = BVH_CHANNEL_XPOS;
				else if ( tinstricmp( acToken, "Yposition" ) == 0 ) pxJoint->anChannelTypes[nLoop] = BVH_CHANNEL_YPOS;
				else if ( tinstricmp( acToken, "Zposition" ) == 0 ) pxJoint->anChannelTypes[nLoop] = BVH_CHANNEL_ZPOS;
				else if ( tinstricmp( acToken, "Xrotation" ) == 0 ) pxJoint->anChannelTypes[nLoop] = BVH_CHANNEL_XROT;
				else if ( tinstricmp( acToken, "Yrotation" ) == 0 ) pxJoint->anChannelTypes[nLoop] = BVH_CHANNEL_YROT;
				else if ( tinstricmp( acToken, "Zrotation" ) == 0 ) pxJoint->anChannelTypes[nLoop] = BVH_CHANNEL_ZROT;
				else return( FALSE );
			}
		}
		else if ( tinstricmp( acToken, "MOTION" ) == 0 )
		{
			break;
		}
	}

	if ( mnNumJoints == 0 ) return( FALSE );

	// ---- Motion
	xTokens.Next( acToken, 256 );		// "Frames:"
	mnNumFrames = (int)xTokens.NextFloat();
	xTokens.Next( acToken, 256 );		// "Frame"
	xTokens.Next( acToken, 256 );		// "Time:"
	mfFrameTime = xTokens.NextFloat();
	if ( mfFrameTime <= 0.0f ) mfFrameTime = 1.0f / 30.0f;
	if ( mnNumFrames <= 0 ) return( FALSE );

	nNumValues = mnNumFrames * mnTotalChannels;
	mpfFrameData = (float*)SystemMalloc( (nNumValues + 1) * sizeof( float ) );
	for ( nLoop = 0; nLoop < nNumValues; nLoop++ )
	{
		if ( xTokens.Next( acToken, 256 ) == FALSE )
		{
			// Truncated file - keep the complete frames we did get
			mnNumFrames = nLoop / mnTotalChannels;
			break;
		}
		mpfFrameData[nLoop] = (float)atof( acToken );
	}
	return( mnNumFrames > 0 );
}

//-------------------------------------------------------------------------
// Function    : GetGlobalPose
// Description : Joints are stored parent-first, so a single forward pass gives global transforms
//-------------------------------------------------------------------------
void	SkeletalAnimation::GetGlobalPose( int nFrame, ENGINEQUATERNION* pxRotsOut, VECT* pxPositionsOut )
{
const float*	pfFrame;
int				nJoint;
int				nChannel;
SKELANIM_JOINT*	pxJoint;
ENGINEQUATERNION	xLocalRot;
ENGINEQUATERNION	xChannelRot;
VECT			xLocalPos;
VECT			xRotatedOffset;
float			fVal;

	if ( nFrame < 0 ) nFrame = 0;
	if ( nFrame >= mnNumFrames ) nFrame = mnNumFrames - 1;
	pfFrame = mpfFrameData + (nFrame * mnTotalChannels);

	for ( nJoint = 0; nJoint < mnNumJoints; nJoint++ )
	{
		pxJoint = &mpxJoints[nJoint];
		xLocalRot = QuatIdentity();
		xLocalPos = pxJoint->xOffset;

		for ( nChannel = 0; nChannel < pxJoint->nNumChannels; nChannel++ )
		{
			fVal = pfFrame[ pxJoint->nChannelStart + nChannel ];
			switch( pxJoint->anChannelTypes[nChannel] )
			{
			// Position channels replace the offset (only usually present on the root)
			case BVH_CHANNEL_XPOS: xLocalPos.x = fVal; break;
			case BVH_CHANNEL_YPOS: xLocalPos.y = fVal; break;
			case BVH_CHANNEL_ZPOS: xLocalPos.z = fVal; break;
			// Rotations compose in channel order : R = R(ch0) * R(ch1) * R(ch2)
			case BVH_CHANNEL_XROT:
				xChannelRot = QuatFromAxisAngle( 1.0f, 0.0f, 0.0f, DEGTORAD( fVal ) );
				xLocalRot = QuatMul( &xLocalRot, &xChannelRot );
				break;
			case BVH_CHANNEL_YROT:
				xChannelRot = QuatFromAxisAngle( 0.0f, 1.0f, 0.0f, DEGTORAD( fVal ) );
				xLocalRot = QuatMul( &xLocalRot, &xChannelRot );
				break;
			case BVH_CHANNEL_ZROT:
				xChannelRot = QuatFromAxisAngle( 0.0f, 0.0f, 1.0f, DEGTORAD( fVal ) );
				xLocalRot = QuatMul( &xLocalRot, &xChannelRot );
				break;
			}
		}

		if ( pxJoint->nParentIndex == NOTFOUND )
		{
			pxRotsOut[nJoint] = xLocalRot;
			pxPositionsOut[nJoint] = xLocalPos;
		}
		else
		{
			pxRotsOut[nJoint] = QuatMul( &pxRotsOut[pxJoint->nParentIndex], &xLocalRot );
			xRotatedOffset = QuatRotate( &pxRotsOut[pxJoint->nParentIndex], &xLocalPos );
			pxPositionsOut[nJoint].x = pxPositionsOut[pxJoint->nParentIndex].x + xRotatedOffset.x;
			pxPositionsOut[nJoint].y = pxPositionsOut[pxJoint->nParentIndex].y + xRotatedOffset.y;
			pxPositionsOut[nJoint].z = pxPositionsOut[pxJoint->nParentIndex].z + xRotatedOffset.z;
		}
	}
}

void	SkeletalAnimation::GetRestPositions( VECT* pxPositionsOut )
{
int		nJoint;

	for ( nJoint = 0; nJoint < mnNumJoints; nJoint++ )
	{
		if ( mpxJoints[nJoint].nParentIndex == NOTFOUND )
		{
			pxPositionsOut[nJoint] = mpxJoints[nJoint].xOffset;
		}
		else
		{
			VectAdd( &pxPositionsOut[nJoint], &pxPositionsOut[ mpxJoints[nJoint].nParentIndex ], &mpxJoints[nJoint].xOffset );
		}
	}
}

//-------------------------------------------------------------------------
// Function    : SaveBVH
// Description : Writes a joint hierarchy + motion data out as a .bvh file
//-------------------------------------------------------------------------
static void		SkeletalAnimationWriteBVHJoint( FILE* pFile, const SKELANIM_JOINT* pxJoints, int nNumJoints, int nJoint, int nDepth )
{
const SKELANIM_JOINT*	pxJoint = &pxJoints[nJoint];
const char*		aszChannelNames[] = { "Xposition", "Yposition", "Zposition", "Xrotation", "Yrotation", "Zrotation" };
char			acIndent[64];
int				nLoop;
BOOL			bHasChildren = FALSE;

	for ( nLoop = 0; ( nLoop < nDepth ) && ( nLoop < 60 ); nLoop++ ) acIndent[nLoop] = '\t';
	acIndent[nLoop] = 0;

	fprintf( pFile, "%s%s %s\n%s{\n", acIndent, (pxJoint->nParentIndex == NOTFOUND) ? "ROOT" : "JOINT", pxJoint->acName, acIndent );
	fprintf( pFile, "%s\tOFFSET %f %f %f\n", acIndent, pxJoint->xOffset.x, pxJoint->xOffset.y, pxJoint->xOffset.z );
	fprintf( pFile, "%s\tCHANNELS %d", acIndent, pxJoint->nNumChannels );
	for ( nLoop = 0; nLoop < pxJoint->nNumChannels; nLoop++ )
	{
		fprintf( pFile, " %s", aszChannelNames[ pxJoint->anChannelTypes[nLoop] ] );
	}
	fprintf( pFile, "\n" );

	for ( nLoop = 0; nLoop < nNumJoints; nLoop++ )
	{
		if ( pxJoints[nLoop].nParentIndex == nJoint )
		{
			bHasChildren = TRUE;
			SkeletalAnimationWriteBVHJoint( pFile, pxJoints, nNumJoints, nLoop, nDepth + 1 );
		}
	}
	if ( ( bHasChildren == FALSE ) || ( pxJoint->bHasEndSite ) )
	{
	VECT	xEnd = pxJoint->xEndSiteOffset;

		if ( pxJoint->bHasEndSite == FALSE )
		{
			xEnd = pxJoint->xOffset;
			VectScale( &xEnd, &xEnd, 0.25f );
		}
		fprintf( pFile, "%s\tEnd Site\n%s\t{\n%s\t\tOFFSET %f %f %f\n%s\t}\n", acIndent, acIndent, acIndent, xEnd.x, xEnd.y, xEnd.z, acIndent );
	}
	fprintf( pFile, "%s}\n", acIndent );
}

BOOL	SkeletalAnimation::SaveBVH( const char* szFilename, const SKELANIM_JOINT* pxJoints, int nNumJoints, const float* pfFrameData, int nNumFrames, float fFrameTime )
{
FILE*	pFile = fopen( szFilename, "wb" );
int		nTotalChannels = 0;
int		nFrame;
int		nLoop;

	if ( pFile == NULL ) return( FALSE );

	for ( nLoop = 0; nLoop < nNumJoints; nLoop++ ) nTotalChannels += pxJoints[nLoop].nNumChannels;

	fprintf( pFile, "HIERARCHY\n" );
	for ( nLoop = 0; nLoop < nNumJoints; nLoop++ )
	{
		if ( pxJoints[nLoop].nParentIndex == NOTFOUND )
		{
			SkeletalAnimationWriteBVHJoint( pFile, pxJoints, nNumJoints, nLoop, 0 );
		}
	}
	fprintf( pFile, "MOTION\nFrames: %d\nFrame Time: %f\n", nNumFrames, fFrameTime );
	for ( nFrame = 0; nFrame < nNumFrames; nFrame++ )
	{
		for ( nLoop = 0; nLoop < nTotalChannels; nLoop++ )
		{
			fprintf( pFile, (nLoop == 0) ? "%.4f" : " %.4f", pfFrameData[ (nFrame * nTotalChannels) + nLoop ] );
		}
		fprintf( pFile, "\n" );
	}
	fclose( pFile );
	return( TRUE );
}
