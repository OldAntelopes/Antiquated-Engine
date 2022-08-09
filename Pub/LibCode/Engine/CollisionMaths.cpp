
#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>

#include "ModelRendering.h"

#include "CollisionMaths/collmathsdefines.h"
#include "CollisionMaths/intr_spheretri.h"

#include "CollisionMaths.h"

//#define DEBUG_COLLISION_TRIS

//#include "../../../GameCommon/RenderUtil/Lines3D.h"
BOOL	mbCollisionDebugOn = FALSE;

VECT	mxCollResultNormal;
VECT	mxCollResultPos;
VECT	mxCollResultImpulse;
float	mfCollResultDist;


//--------------------------------------------------------------------
// GetBoundSphereForTri
//  Generates a bounding sphere from 3 points of a triangle
//--------------------------------------------------------------------
void	GetBoundSphereForTri(VECT* pTri1,VECT* pTri2,VECT* pTri3, VECT* pxPos, float* pfOutRadius )
{
float	fMaxDist = 0.0f;
float	fNewDist;

	pxPos->x = (pTri1->x + pTri2->x + pTri3->x ) / 3;
	pxPos->y = (pTri1->y + pTri2->y + pTri3->y ) / 3;
	pxPos->z = (pTri1->z + pTri2->z + pTri3->z ) / 3;
	fMaxDist = VectDist( pxPos, pTri1 );
	fNewDist = VectDist( pxPos, pTri2 );
	if ( fNewDist > fMaxDist )
	{
		fMaxDist = fNewDist;
	}
	fNewDist = VectDist( pxPos, pTri3 );
	if ( fNewDist > fMaxDist )
	{
		fMaxDist = fNewDist;
	}
	*pfOutRadius = fMaxDist;
}

//-----------------------------------------------------------------------
// SweepSphereTriBoundsCheck
// A simplified (i.e. fast)(ish) collision bounds check between a moving sphere and a triangle
// Returns true if the bounding spheres of the moving sphere volume and the bounding sphere of
// the triangle overlap. Intended to be used as a first pass before using the 
// SweepSphereTriCollision function (below).
//------------------------------------------------------------------------
BOOL	SweepSphereTriBoundsCheck( VECT* pTri1,VECT* pTri2,VECT* pTri3,VECT* pSphereStart, VECT* pSphereEnd, float fSphereRadius )
{
VECT	xTriSpherePos;
float	fTriSphereRadius;
float	fDist;

	GetBoundSphereForTri( pTri1, pTri2, pTri3, &xTriSpherePos, &fTriSphereRadius );
	fDist = VectDist( pSphereStart, pSphereEnd );
	fSphereRadius += fDist;
	fDist = VectDist( &xTriSpherePos, pSphereStart );
	if ( fDist < (fSphereRadius+fTriSphereRadius) )
	{
		return( TRUE );
	}	
	return( FALSE );
}


//-----------------------------------------------------------------------
// SweepSphereTriCollision
//  Performs an collision check between a moving sphere and a triangle.
//  Returns true (and fills in the Hit point and collision normal) if a
//  collision occurs
//----------------------------------------------------------------------
BOOL	SweepSphereTriCollision( VECT* pTri1,VECT* pTri2,VECT* pTri3,VECT* pSphereStart, VECT* pSphereEnd, float fSphereRadius, VECT* pxHit, VECT* pxNormal, float* pfDistToCollision )
{
Vec3f	aTriPts[3];
Vec3f	xTriNormal;
Sphere	xSphere;
Vec3f	xSphereVel;
float	fDistTravelled;
Vec3f	xReaction;
VECT	xNormal;
float	fDistToCompare;
VECT	xEdge1;
VECT	xEdge2;

	aTriPts[0].x = pTri1->x;
	aTriPts[0].y = pTri1->y;
	aTriPts[0].z = pTri1->z;
	aTriPts[1].x = pTri2->x;
	aTriPts[1].y = pTri2->y;
	aTriPts[1].z = pTri2->z;
	aTriPts[2].x = pTri3->x;
	aTriPts[2].y = pTri3->y;
	aTriPts[2].z = pTri3->z;
	
	VectSub( &xEdge1, pTri1, pTri2 );
	VectSub( &xEdge2, pTri1, pTri3 );

	VectCross( &xNormal, &xEdge1, &xEdge2 );
	VectNormalize( &xNormal );
	xTriNormal.x = xNormal.x;
	xTriNormal.y = xNormal.y;
	xTriNormal.z = xNormal.z;

	xSphere.center.x = pSphereStart->x;
	xSphere.center.y = pSphereStart->y;
	xSphere.center.z = pSphereStart->z;
	xSphere.radius = fSphereRadius;
	xSphereVel.x = pSphereEnd->x - pSphereStart->x;
	xSphereVel.y = pSphereEnd->y - pSphereStart->y;
	xSphereVel.z = pSphereEnd->z - pSphereStart->z;
	fDistToCompare = VectGetLength( (VECT*)&xSphereVel ) + fSphereRadius;
	if ( testIntersectionTriSphere( (const Vec3f*)&aTriPts[0], xTriNormal, xSphere, xSphereVel, fDistTravelled, &xReaction ) == true )
	{
		if ( fDistTravelled < fDistToCompare )
		{
			if ( pxNormal )
			{
				pxNormal->x = xTriNormal.x;
				pxNormal->y = xTriNormal.y;
				pxNormal->z = xTriNormal.z;
			}
			*pfDistToCollision = fDistTravelled;
			return( TRUE );
		}
	}
	return( FALSE );
}



BOOL	EngineCollisionSphereSweepTest( float fMovingSphereRadius, VECT* pxPos1, VECT* pxPos2, int nStaticModel, VECT* pxStaticPos, VECT* pxRot, int nFlags )
{
MODEL_RENDER_DATA*	pxStaticModel;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVert1;
CUSTOMVERTEX*	pxVert2;
CUSTOMVERTEX*	pxVert3;
VECT			axVerts[3];
ushort*			puwIndices;
int				nLoop;
VECT		xHit;
VECT		xNormal;
VECT		xRotated1;
VECT		xRotated2;

	// TODO - Do a quick sphere test between the expanded movement sphere and the static model bounds, just
	// to see if its worth even checking the triangles in it 
	if ( nStaticModel == NOTFOUND ) return( FALSE );

	pxStaticModel = &maxModelRenderData[ nStaticModel ];
	if ( pxStaticModel->pxBaseMesh == NULL ) return( FALSE );

#ifdef DEBUG_COLLISION_TRIS
	if ( mbCollisionDebugOn )
	{
	VECT		xPos;
	VECT		xPos2;
		xPos = *pxPos1;
		xPos2 = xPos;
		xPos2.z += fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.x += fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.y += fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.z -= fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.x -= fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.y -= fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos = *pxPos2;
		xPos2 = xPos;
		xPos2.z += fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.x += fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.y += fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.z -= fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.x -= fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
		xPos2 = xPos;
		xPos2.y -= fMovingSphereRadius;
		Lines3DAddLine( &xPos, &xPos2, 0xFFFF0000, 0xFFFF0000 );
	}
#endif

	if ( pxRot->z != 0.0f )
	{
		// Use the static object's rotation by inversely rotating the position of the spheres
		VectSub( &xRotated1, pxPos1, pxStaticPos );
		VectSub( &xRotated2, pxPos2, pxStaticPos );
		VectRotateAboutZ( &xRotated1, A360 - pxRot->z );
		VectRotateAboutZ( &xRotated2, A360 - pxRot->z );
		VectAdd( &xRotated1, &xRotated1, pxStaticPos );
		VectAdd( &xRotated2, &xRotated2, pxStaticPos );
		pxPos1 = &xRotated1;
		pxPos2 = &xRotated2;
	}

	// Go through all the triangles in the static model and compare them with the swept sphere
	// First hit will return true and collision response values
	pxStaticModel->pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );
	pxStaticModel->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );
	// For each of the tris in model 1
	for ( nLoop = 0; nLoop < pxStaticModel->xStats.nNumIndices; nLoop += 3 )
	{
		pxVert1 = pxVertices + puwIndices[ nLoop ];
		pxVert2 = pxVertices + puwIndices[ nLoop+1 ];
		pxVert3 = pxVertices + puwIndices[ nLoop+2 ];
		VectAdd( &axVerts[0],(VECT*)&pxVert1->position, pxStaticPos );
		VectAdd( &axVerts[1],(VECT*)&pxVert2->position, pxStaticPos );
		VectAdd( &axVerts[2],(VECT*)&pxVert3->position, pxStaticPos );

		// If the quick bounds check succeeds...
		if ( SweepSphereTriBoundsCheck( &axVerts[0], &axVerts[1], &axVerts[2], pxPos1, pxPos2, fMovingSphereRadius ) == TRUE )
		{
		float	fDistToCollision;
//			InterfaceText( 1, 200,200,"Early coll", COL_WARNING, 0 );
			// Check the accurate sweep check
			if ( SweepSphereTriCollision( &axVerts[0], &axVerts[1], &axVerts[2], pxPos1, pxPos2, fMovingSphereRadius, &xHit, &xNormal, &fDistToCollision ) == TRUE )
			{
//				InterfaceText(1, 200,220,"Full coll", COL_WARNING, 0 );
#ifdef DEBUG_COLLISION_TRIS
				if ( mbCollisionDebugOn )
				{
					if ( pxRot->z != 0.0f )
					{
					VECT	xVect;
						VectSub( &xVect, &axVerts[0], pxStaticPos );
						VectRotateAboutZ( &xVect, pxRot->z );
						VectAdd( &axVerts[0], &xVect, pxStaticPos );
						VectSub( &xVect, &axVerts[1], pxStaticPos );
						VectRotateAboutZ( &xVect, pxRot->z );
						VectAdd( &axVerts[1], &xVect, pxStaticPos );
						VectSub( &xVect, &axVerts[2], pxStaticPos );
						VectRotateAboutZ( &xVect, pxRot->z );
						VectAdd( &axVerts[2], &xVect, pxStaticPos );
					}
					axVerts[0].z += 0.001f;
					axVerts[1].z += 0.001f;
					axVerts[2].z += 0.001f;
					Lines3DAddLine( &axVerts[0], &axVerts[1], 0xFFFF80FF, 0xFFFF80FF );
					Lines3DAddLine( &axVerts[1], &axVerts[2], 0xFFFF80FF, 0xFFFF80FF );
					Lines3DAddLine( &axVerts[0], &axVerts[2], 0xFFFF80FF, 0xFFFF80FF );
					axVerts[0].x += 0.001f;
					axVerts[1].x += 0.001f;
					axVerts[2].x += 0.001f;
					Lines3DAddLine( &axVerts[0], &axVerts[1], 0xFFFFFFFF, 0xFFFFFFFF );
					Lines3DAddLine( &axVerts[1], &axVerts[2], 0xFFFFFFFF, 0xFFFFFFFF );
					Lines3DAddLine( &axVerts[0], &axVerts[2], 0xFFFFFFFF, 0xFFFFFFFF );
				}
#endif
				mxCollResultNormal = xNormal;
				mxCollResultPos = xHit;
				mfCollResultDist = fDistToCollision;

				if ( pxRot->z != 0.0f )
				{
					VectRotateAboutZ( &mxCollResultNormal, pxRot->z );
					// todo rotate pos
				}

				pxStaticModel->pxBaseMesh->UnlockVertexBuffer();
				pxStaticModel->pxBaseMesh->UnlockIndexBuffer();
				return( TRUE );
			}

		}
	}

	pxStaticModel->pxBaseMesh->UnlockVertexBuffer();
	pxStaticModel->pxBaseMesh->UnlockIndexBuffer();
	return( FALSE );
}


//--------------------------------------------------------------------------------------------------------------------


//#define COLL_DEBUG_OUTPUT

class MATRIX33
{
public:
	VECT		C[3];
};

class BASIS
{
public:
	VECT	v[3];
public:
	BASIS() { v[0].x = 1.0f;v[0].y = 0.0f;v[0].z = 0.0f;v[1].x = 0.0f;v[1].y = 1.0f;v[1].z = 0.0f;v[2].x = 0.0f;v[2].y = 0.0f;v[2].z = 1.0f;}

/*
	// Right-Handed Rotations
	void rotateAboutX( float a )
	{
		if( 0 != a )//don’t rotate by 0
		{
			D3DXVECTOR3 b1 = this->Y()*cos(a) + this->Z()*sin(a);
			D3DXVECTOR3 b2 = -this->Y()*sin(a) + this->Z()*cos(a);
			//set basis
			this->R.C[1] = b1;
			this->R.C[2] = b2;
			//x is unchanged
		}
	}
	
	void rotateAboutY( float a )
	{
		if( 0 != a )//don’t rotate by 0
		{
		D3DXVECTOR3 b2 = this->Z()*cos(a) + this->X()*sin(a); //rotate z
		D3DXVECTOR3 b0 = -this->Z()*sin(a) + this->X()*cos(a); //rotate x

			//set basis
			this->R.C[2] = b2;
			this->R.C[0] = b0;

			//y is unchanged

		}
	}
*/
	void rotateAboutZ( float a )
	{
		if( 0 != a )//don’t rotate by 0
		{
		float	fCosA = cosf(a);
		float	fSinA = sinf(a);
		VECT b1;
		VECT b0;

			//don’t over-write basis before calculation is done
			b0.x = (float)((v[0].x * fCosA) + (v[1].x * fSinA));
			b0.y = (float)((v[0].y * fCosA) + (v[1].y * fSinA));
			b0.z = (float)((v[0].z * fCosA) + (v[1].z * fSinA));

			b1.x = (float)((-v[0].x * fSinA) + (v[1].x * fCosA));
			b1.y = (float)((-v[0].y * fSinA) + (v[1].y * fCosA));
			b1.z = (float)((-v[0].z * fSinA) + (v[1].z * fCosA));

			v[0] = b0;
			v[1] = b1;
		}
	}

};


//check if two oriented bounding boxes overlap
BOOL	OBBOverlap2( //A
float* a, //extents
const VECT* Pa, //position
BASIS& A, //orthonormal basis
//B
float* b, //extents
const VECT* Pb, //position
BASIS& B //orthonormal basis
)

{
//translation, in parent frame
VECT v;
//translation, in A's frame
float T[3];
//B's basis with respect to A's local frame
float R[3][3];
float ra, rb, t;
long i, k;

	VectSub( &v, Pb, Pa );
	T[0] = VectDot( &v, &A.v[0] );
	T[1] = VectDot( &v, &A.v[1] );
	T[2] = VectDot( &v, &A.v[2] );

	//calculate rotation matrix
	for( i=0 ; i<3 ; i++ )
	{
		for( k=0 ; k<3 ; k++ )
		{
			R[i][k] = VectDot( &A.v[i], &B.v[k] ); 
		}
	}

	/*ALGORITHM: Use the separating axis test for all 15 potential 
	separating axes. If a separating axis could not be found, the two 
	boxes overlap. */
	//A's basis vectors
	for( i=0 ; i<3 ; i++ )
	{
		ra = a[i];
		rb = (float)(b[0]*fabs(R[i][0]) + b[1]*fabs(R[i][1]) + b[2]*fabs(R[i][2]));

		t = (float)(fabs( T[i] ));

		if( t >	ra + rb ) return false;
	}

	//B's basis vectors
	for( k=0 ; k<3 ; k++ )
	{
		ra = (float)(a[0]*fabs(R[0][k]) + a[1]*fabs(R[1][k]) + a[2]*fabs(R[2][k]));
		rb = b[k];
		t = (float)(fabs( T[0]*R[0][k] + T[1]*R[1][k] + T[2]*R[2][k] ));

		if( t > ra + rb ) return false;

	}

	//9	cross products
	//L = A0 x B0
	ra = (float)(a[1]*fabs(R[2][0]) + a[2]*fabs(R[1][0]));
	rb = (float)(b[1]*fabs(R[0][2]) + b[2]*fabs(R[0][1]));

	t = (float)(fabs( T[2]*R[1][0] - T[1]*R[2][0] ));

	if( t > ra + rb ) return false;

	//L = A0 x B1
	ra = (float)(a[1]*fabs(R[2][1]) + a[2]*fabs(R[1][1]));
	rb = (float)(b[0]*fabs(R[0][2]) + b[2]*fabs(R[0][0]));

	t = (float)(fabs( T[2]*R[1][1] - T[1]*R[2][1] ));

	if( t > ra + rb ) return false;

	//L = A0 x B2
	ra = (float)(a[1]*fabs(R[2][2]) + a[2]*fabs(R[1][2]));
	rb = (float)(b[0]*fabs(R[0][1]) + b[1]*fabs(R[0][0]));

	t = (float)(fabs( T[2]*R[1][2] - T[1]*R[2][2] ));

	if( t > ra + rb ) return false;

	//L = A1 x B0
	ra = (float)(a[0]*fabs(R[2][0]) + a[2]*fabs(R[0][0]));
	rb = (float)(b[1]*fabs(R[1][2]) + b[2]*fabs(R[1][1]));

	t = (float)(fabs( T[0]*R[2][0] - T[2]*R[0][0] ));

	if( t > ra + rb ) return false;

	//L = A1 x B1
	ra = (float)(a[0]*fabs(R[2][1]) + a[2]*fabs(R[0][1]));
	rb = (float)(b[0]*fabs(R[1][2]) + b[2]*fabs(R[1][0]));
	t = (float)(fabs( T[0]*R[2][1] - T[2]*R[0][1] ));

	if( t > ra + rb ) return false;

	//L = A1 x B2
	ra = (float)(a[0]*fabs(R[2][2]) + a[2]*fabs(R[0][2]));
	rb = (float)(b[0]*fabs(R[1][1]) + b[1]*fabs(R[1][0]));
	t = (float)(fabs( T[0]*R[2][2] - T[2]*R[0][2] ));
	if( t > ra + rb )return false;

	//L = A2 x B0
	ra = (float)(a[0]*fabs(R[1][0]) + a[1]*fabs(R[0][0]));
	rb = (float)(b[1]*fabs(R[2][2]) + b[2]*fabs(R[2][1]));
	t = (float)(fabs( T[1]*R[0][0] - T[0]*R[1][0] ));

	if( t > ra + rb ) return false;

	//L = A2 x B1
	ra = (float)(a[0]*fabs(R[1][1]) + a[1]*fabs(R[0][1]));
	rb = (float)(b[0] *fabs(R[2][2]) + b[2]*fabs(R[2][0]));
	t = (float)(fabs( T[1]*R[0][1] - T[0]*R[1][1] ));
	if( t > ra + rb ) return false;

	//L = A2 x B2
	ra = (float)(a[0]*fabs(R[1][2]) + a[1]*fabs(R[0][2]));
	rb = (float)(b[0]*fabs(R[2][1]) + b[1]*fabs(R[2][0]));
	t = (float)(fabs( T[1]*R[0][2] - T[0]*R[1][2] ));
	if( t > ra + rb ) return false;
	/*no separating axis found,
	the two boxes overlap */

	return true;
}

#define		MAX_TRIANGLES_TO_CONSIDER	64

ENGINEMATRIX	mxModel1RotMatrix;
ENGINEMATRIX	mxModel2RotMatrix;


typedef struct
{
	int		nIndexNum;
	float	fDist;
	
	VECT		xBoundExtents;
	VECT		xBoundCentre;
	VECT		xNormal;

} CONSIDERED_TRIS;

/***************************************************************************
 * Function    : EngineCollisionOBBIsOverlapping
 * Params      :
 * Returns     :
 * Description : 
 ***************************************************************************/
BOOL EngineCollisionOBBIsOverlapping( VECT* pxPos, VECT* pxExtents, float fZRot, VECT* pxPos2, VECT* pxExtents2, float fZRot2 )
{

	return( FALSE );
}

float	 EngineGetDist( const VECT* pxPos1, const VECT* pxPos2 )
{
VECT		xVect;

	VectSub( &xVect, pxPos1, pxPos2 );
	return( VectGetLength( &xVect ) );
}

BOOL	 EngineMeshMeshTriangleCheck( MODEL_RENDER_DATA* pxModel, VECT* pxPos1, VECT* pxRotAngles1, BASIS* pxBasis1, MODEL_RENDER_DATA* pxOtherModel, VECT* pxPos2, VECT* pxRotAngles2,BASIS* pxBasis2,int nFlags )
{
CONSIDERED_TRIS*	pModel1Triangles;
CONSIDERED_TRIS*	pCollisionTriangles;
CUSTOMVERTEX*	pxVertices;
ushort*			puwIndices;
CUSTOMVERTEX*	pxVert1;
CUSTOMVERTEX*	pxVert2;
CUSTOMVERTEX*	pxVert3;
CUSTOMVERTEX	axVerts[3];
int		nLoop;
int		nLoop2;
BASIS	xTriBasis;
BASIS	xOtherRotBasis;
VECT		xBoundMin;
VECT		xBoundMax;
VECT		xBoundExtents;
VECT		xBoundCentre;
VECT	xPos;
//VECT	xTriPos;
VECT	xOtherPos;
int		nNumMod1TriToConsider = 0;
int		nNumCollisionTris = 0;

	pModel1Triangles = (CONSIDERED_TRIS*)(SystemMalloc( MAX_TRIANGLES_TO_CONSIDER * sizeof( CONSIDERED_TRIS ) ) );
	pCollisionTriangles = (CONSIDERED_TRIS*)(SystemMalloc( MAX_TRIANGLES_TO_CONSIDER * sizeof( CONSIDERED_TRIS ) ) );

	pxModel->pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );
	pxModel->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );

	xOtherPos.x = pxPos2->x + pxOtherModel->xStats.xBoundBoxCentre.x;
	xOtherPos.y = pxPos2->y + pxOtherModel->xStats.xBoundBoxCentre.y;
	xOtherPos.z = pxPos2->z + pxOtherModel->xStats.xBoundBoxCentre.z;

	for ( nLoop = 0; nLoop < pxModel->xStats.nNumIndices; nLoop += 3 )
	{
		pxVert1 = pxVertices + puwIndices[ nLoop ];
		pxVert2 = pxVertices + puwIndices[ nLoop+1 ];
		pxVert3 = pxVertices + puwIndices[ nLoop+2 ];
		axVerts[0] = *pxVert1;
		axVerts[1] = *pxVert2;
		axVerts[2] = *pxVert3;

		RenderingComputeBoundingBox( axVerts, 3, &xBoundMin, &xBoundMax );
		xBoundExtents.x = (xBoundMax.x - xBoundMin.x)*0.5f;
		xBoundExtents.y = (xBoundMax.y - xBoundMin.y)*0.5f;
		xBoundExtents.z = (xBoundMax.z - xBoundMin.z)*0.5f;
		xBoundCentre.x = (xBoundExtents.x) + xBoundMin.x;
		xBoundCentre.y = (xBoundExtents.y) + xBoundMin.y;
		xBoundCentre.z = (xBoundExtents.z) + xBoundMin.z;
		VectTransform( &xBoundCentre, &xBoundCentre, &mxModel1RotMatrix );

		xPos.x = pxPos1->x + xBoundCentre.x;
		xPos.y = pxPos1->y + xBoundCentre.y;
		xPos.z = pxPos1->z + xBoundCentre.z;

		if ( OBBOverlap2( (float*)(&xBoundExtents), &xPos, *pxBasis1, (float*)(&pxOtherModel->xStats.xBoundBoxExtents), &xOtherPos, *pxBasis2 ) )
		{
			pModel1Triangles[ nNumMod1TriToConsider ].nIndexNum = nLoop;
			pModel1Triangles[ nNumMod1TriToConsider ].xBoundCentre = xPos;
			pModel1Triangles[ nNumMod1TriToConsider ].xBoundExtents = xBoundExtents;
			if ( nNumMod1TriToConsider < MAX_TRIANGLES_TO_CONSIDER - 1 )
			{
				nNumMod1TriToConsider++;
			}
		}
	}
	pxModel->pxBaseMesh->UnlockVertexBuffer();
	pxModel->pxBaseMesh->UnlockIndexBuffer();

	if ( nNumMod1TriToConsider > 0 )
	{
		pxOtherModel->pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );
		pxOtherModel->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );
		xOtherPos.x = pxPos1->x + pxModel->xStats.xBoundBoxCentre.x;
		xOtherPos.y = pxPos1->y + pxModel->xStats.xBoundBoxCentre.y;
		xOtherPos.z = pxPos1->z + pxModel->xStats.xBoundBoxCentre.z;
		for ( nLoop = 0; nLoop < pxOtherModel->xStats.nNumIndices; nLoop += 3 )
		{
			// For each tri in the other model, first check the tri box against the model1 overall bounding box
			pxVert1 = pxVertices + puwIndices[ nLoop ];
			pxVert2 = pxVertices + puwIndices[ nLoop+1 ];
			pxVert3 = pxVertices + puwIndices[ nLoop+2 ];
			axVerts[0] = *pxVert1;
			axVerts[1] = *pxVert2;
			axVerts[2] = *pxVert3;

			RenderingComputeBoundingBox( axVerts, 3, &xBoundMin, &xBoundMax );
			xBoundExtents.x = (xBoundMax.x - xBoundMin.x)*0.5f;
			xBoundExtents.y = (xBoundMax.y - xBoundMin.y)*0.5f;
			xBoundExtents.z = (xBoundMax.z - xBoundMin.z)*0.5f;
			xBoundCentre.x = (xBoundExtents.x) + xBoundMin.x;
			xBoundCentre.y = (xBoundExtents.y) + xBoundMin.y;
			xBoundCentre.z = (xBoundExtents.z) + xBoundMin.z;
			xPos.x = pxPos2->x + xBoundCentre.x;
			xPos.y = pxPos2->y + xBoundCentre.y;
			xPos.z = pxPos2->z + xBoundCentre.z;

			// If that collides.. now check against the triangles to consider..
			if ( OBBOverlap2( (float*)(&xBoundExtents), &xPos, *pxBasis2, (float*)(&pxModel->xStats.xBoundBoxExtents), &xOtherPos, *pxBasis1 ) )
			{
				for ( nLoop2 = 0; nLoop2 < 	nNumMod1TriToConsider; nLoop2++ )
				{
					if ( OBBOverlap2( (float*)(&xBoundExtents), &xPos, *pxBasis2, (float*)(&pModel1Triangles[nLoop2].xBoundExtents), &pModel1Triangles[nLoop2].xBoundCentre, *pxBasis1 ) )
					{
						// we have a definite overlap here (well, according to the bound boxes.
						// we could now go further and check planes,
						// or add the tri to a list of collisions to find the closest and calc a normal
						pCollisionTriangles[ nNumCollisionTris ].nIndexNum = nLoop2;
						pCollisionTriangles[ nNumCollisionTris ].fDist = EngineGetDist( &pModel1Triangles[nLoop2].xBoundCentre, &xPos );
						if ( nNumCollisionTris < MAX_TRIANGLES_TO_CONSIDER - 1 )
						{
							nNumCollisionTris++;
						}
					}
				}

			}

		}
		pxOtherModel->pxBaseMesh->UnlockVertexBuffer();
		pxOtherModel->pxBaseMesh->UnlockIndexBuffer();
	}
	SystemFree( pModel1Triangles );

	if ( nNumCollisionTris > 0 )
	{
//	float	fClosestDist;
	
		SystemFree( pCollisionTriangles );
		return( TRUE );
	}
	SystemFree( pCollisionTriangles );
	return( FALSE );
}

float	EngineCollisionResultDistance( void )
{
	return( mfCollResultDist );
}

VECT*	 EngineCollisionResultNormal( void )
{
	return( &mxCollResultNormal );
}

VECT*	EngineCollisionResultHitPoint( void )
{
	return( &mxCollResultPos );
}


BOOL	 EngineBoxMeshTriangleCheck( MODEL_RENDER_DATA* pxModel, VECT* pxPos1, VECT* pxRotAngles1, BASIS* pxBasis1, MODEL_RENDER_DATA* pxOtherModel, VECT* pxPos2, VECT* pxRotAngles2,BASIS* pxBasis2,int nFlags )
{
CONSIDERED_TRIS*	pCollisionTriangles;
CUSTOMVERTEX*	pxVertices;
ushort*			puwIndices;
CUSTOMVERTEX*	pxVert1;
CUSTOMVERTEX*	pxVert2;
CUSTOMVERTEX*	pxVert3;
CUSTOMVERTEX	axVerts[4];
int		nLoop;
BASIS	xTriBasis;
BASIS	xOtherRotBasis;
VECT		xBoundMin;
VECT		xBoundMax;
VECT		xBoundExtents;
VECT		xBoundCentre;
VECT	xPos;
VECT	xCalcPos;
VECT	xTriPos;
VECT	xOtherPos;
int		nFaceNum = 0;
int		nNumCollisionTris = 0;
#ifdef COLL_DEBUG_OUTPUT
char	acString[256];
#endif
	if ( pxModel->pxBaseMesh == NULL ) return( FALSE );

	pCollisionTriangles = (CONSIDERED_TRIS*)(SystemMalloc( MAX_TRIANGLES_TO_CONSIDER * sizeof( CONSIDERED_TRIS ) ) );
	pxModel->pxBaseMesh->LockVertexBuffer(kLock_ReadOnly, (byte**)( &pxVertices ) );
	pxModel->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &puwIndices ) );

	xOtherPos.x = pxPos2->x + pxOtherModel->xStats.xBoundBoxCentre.x;
	xOtherPos.y = pxPos2->y + pxOtherModel->xStats.xBoundBoxCentre.y;
	xOtherPos.z = pxPos2->z + pxOtherModel->xStats.xBoundBoxCentre.z;

#ifdef COLL_DEBUG_OUTPUT
	sprintf( acString, "checking %d indices", pxModel->xStats.nNumIndices );
	InterfaceText( 2, 200, 440, acString, COL_WARNING, 0 );
#endif

	// For each of the tris in model 1
	for ( nLoop = 0; nLoop < pxModel->xStats.nNumIndices; nLoop += 3 )
	{
		pxVert1 = pxVertices + puwIndices[ nLoop ];
		pxVert2 = pxVertices + puwIndices[ nLoop+1 ];
		pxVert3 = pxVertices + puwIndices[ nLoop+2 ];
		axVerts[0].position = pxVert1->position;
		axVerts[1].position = pxVert2->position;
		axVerts[2].position = pxVert3->position;
		axVerts[3] = *pxVert3;

		RenderingComputeBoundingBox( axVerts, 4, &xBoundMin, &xBoundMax );
		xBoundExtents.x = ((xBoundMax.x - xBoundMin.x)*0.5f)+0.001f;
		xBoundExtents.y = ((xBoundMax.y - xBoundMin.y)*0.5f)+0.001f;
		xBoundExtents.z = ((xBoundMax.z - xBoundMin.z)*0.5f)+0.001f;
		xBoundCentre.x = (xBoundExtents.x) + xBoundMin.x;
		xBoundCentre.y = (xBoundExtents.y) + xBoundMin.y;
		xBoundCentre.z = (xBoundExtents.z) + xBoundMin.z;
		VectTransform( &xBoundCentre, &xBoundCentre, &mxModel1RotMatrix );
		xPos.x = pxPos1->x + xBoundCentre.x;
		xPos.y = pxPos1->y + xBoundCentre.y;
		xPos.z = pxPos1->z + xBoundCentre.z;

		// If the tri OBB overlaps the OBB for model2 (model2 usually the player who is generally box shaped :])
		if ( OBBOverlap2( (float*)(&xBoundExtents), &xPos, *pxBasis1, (float*)(&pxOtherModel->xStats.xBoundBoxExtents), &xOtherPos, *pxBasis2 ) )
		{
		VECT			xFaceEdge1;
		VECT			xFaceEdge2;
			VectSub( &xFaceEdge1, &pxVert1->position, &pxVert2->position );
			VectSub( &xFaceEdge2, &pxVert2->position, &pxVert3->position );
			VectCross( &mxCollResultNormal, &xFaceEdge1, &xFaceEdge2 );
			// Store collision info
			VectNormalize( &mxCollResultNormal );
	
//			mxCollResultNormal.x = (pxVert1->normal.x +pxVert2->normal.x +pxVert3->normal.x) * 0.33333f;
//			mxCollResultNormal.y = (pxVert1->normal.y +pxVert2->normal.y +pxVert3->normal.y) * 0.33333f;
//			mxCollResultNormal.z = (pxVert1->normal.z +pxVert2->normal.z +pxVert3->normal.z) * 0.33333f;
			xTriPos.x = (pxVert1->position.x + pxVert2->position.x + pxVert3->position.x) * 0.3333333f;
			xTriPos.y = (pxVert1->position.y + pxVert2->position.y + pxVert3->position.y) * 0.3333333f;
			xTriPos.z = (pxVert1->position.z + pxVert2->position.z + pxVert3->position.z) * 0.3333333f;
			xTriPos.z = 0.0f;
			xCalcPos = xPos;
			xCalcPos.z = 0.0f;
			// we have a definite overlap here (well, according to the bound boxes.
			// we could now go further and check planes,
			// or add the tri to a list of collisions to find the closest and calc a normal
			pCollisionTriangles[ nNumCollisionTris ].xNormal = mxCollResultNormal;
			pCollisionTriangles[ nNumCollisionTris ].fDist = EngineGetDist( &xTriPos, &xCalcPos );
			if ( nNumCollisionTris < MAX_TRIANGLES_TO_CONSIDER - 1 )
			{
				nNumCollisionTris++;
			}
		}
		nFaceNum++;
	}
	pxModel->pxBaseMesh->UnlockVertexBuffer();
	pxModel->pxBaseMesh->UnlockIndexBuffer();

	if ( nNumCollisionTris > 0 )
	{
	float	fClosestDist = 99999999.0f;
	int		nClosest = 0;

		for ( nLoop= 0; nLoop < nNumCollisionTris; nLoop++ )
		{
			// Always favour vertical planes
			if ( pCollisionTriangles[nLoop].xNormal.z != 0.0f )
			{
				pCollisionTriangles[nLoop].fDist *= 1.01f;
			}

			if ( pCollisionTriangles[nLoop].fDist < fClosestDist )
			{
				nClosest = nLoop;
				fClosestDist = pCollisionTriangles[nLoop].fDist;
			}
			// else use equals to add adjoining things when dist is calculated to shortest point rather than midface
		}
				
		VectTransform( &mxCollResultNormal, &pCollisionTriangles[nClosest].xNormal, &mxModel1RotMatrix );

#ifdef COLL_DEBUG_OUTPUT
		{
			sprintf( acString, "Normal :%.3f,%.3f,%.3f", mxCollResultNormal.x, mxCollResultNormal.y, mxCollResultNormal.z );
			InterfaceText( 2, 200, 420, acString, COL_WARNING, 0 );
		}
#endif
		SystemFree( pCollisionTriangles );
		return( TRUE);

	}
	SystemFree( pCollisionTriangles );
	return( FALSE );
}

/*
BOOL		EngineMeshMeshSphereCheck( int nHandle1, VECT* pxPos1, int nHandle2, VECT* pxPos2 )
{

}
*/


BOOL		EngineModelRayTest( int nModelHandle, const VECT* pxModelPos, const VECT* pxModelRot, const VECT* pxRayStart, const VECT* pxRayEnd, int flags )
{
BOOL	bRet;
int		nFaceNum;

	bRet = ModelRayTest( nModelHandle, pxModelPos, pxModelRot, pxRayStart, pxRayEnd, &mxCollResultPos, &mxCollResultNormal, &nFaceNum, flags );
	return( bRet );
}

BOOL		EngineModelOBBCollision( int nModelHandle, const VECT* pxModelPos, const VECT* pxModelRot, const VECT* pxBoxCentre, const VECT* pxBoxBounds, float fBoxZRot )
{
MODEL_RENDER_DATA*	pxModel;
//VECT			xVect;
//VECT		xVect2;
//float			fDist;
VECT		xModelPos1;
//VECT		xModelPos2;
#ifdef COLL_DEBUG_OUTPUT
char	acString[256];
#endif
//	VECT			xPos;
VECT		xExtents;
//	VECT		xOtherPos;
VECT		xOtherExtents;
BASIS	xRotBasis;
BASIS	xOtherRotBasis;

	pxModel = &maxModelRenderData[ nModelHandle ];
    EngineMatrixRotationZ( &mxModel1RotMatrix, pxModelRot->z );
	VectTransform( &xModelPos1, &pxModel->xStats.xBoundSphereCentre, &mxModel1RotMatrix );
	VectAdd( &xModelPos1, pxModelPos, &xModelPos1 );

	xExtents = pxModel->xStats.xBoundBoxExtents;
	xRotBasis.rotateAboutZ( pxModelRot->z );
			
	xOtherExtents = *pxBoxBounds;
	xOtherRotBasis.rotateAboutZ( fBoxZRot);

	if ( OBBOverlap2( (float*)(&xExtents), &xModelPos1, xRotBasis, (float*)(&xOtherExtents), pxBoxCentre, xOtherRotBasis ) )
	{
						
		return( TRUE );
	}	
	return( FALSE );
}


BOOL		EngineModelSphereCollision( int nModelHandle, const VECT* pxModelPos, const VECT* pxModelRot, const VECT* pxSpherePos, float fRadius, int nFlags )
{
MODEL_RENDER_DATA*	pxModel;
//VECT			xVect;
//VECT		xVect2;
float			fDist;
VECT		xModelPos1;
//VECT		xModelPos2;
#ifdef COLL_DEBUG_OUTPUT
char	acString[256];
#endif

	pxModel = &maxModelRenderData[ nModelHandle ];

	EngineMatrixRotationZ( &mxModel1RotMatrix, pxModelRot->z );
	VectTransform( &xModelPos1, &pxModel->xStats.xBoundSphereCentre, &mxModel1RotMatrix );
	
	// Do a quick sphere test first
	xModelPos1.x = pxModelPos->x + xModelPos1.x;
	xModelPos1.y = pxModelPos->y + xModelPos1.y;
	xModelPos1.z = pxModelPos->z + xModelPos1.z;

	fDist = EngineGetDist( &xModelPos1, pxSpherePos );

	if ( fDist < pxModel->xStats.fBoundSphereRadius + fRadius )
	{
//	VECT			xPos;
	VECT		xExtents;
//	VECT		xOtherPos;
	VECT		xOtherExtents;
	BASIS	xRotBasis;
	BASIS	xOtherRotBasis;

#ifdef COLL_DEBUG_OUTPUT
		sprintf( acString, "Sphere bounds coll rot1 %f rot2 %f", pxRotAngles1->z, pxRotAngles2->z );
		InterfaceText(2, 200,340,acString, COL_WARNING, 0 );
#endif
		xExtents.x = pxModel->xStats.xBoundBoxExtents.x;
		xExtents.y = pxModel->xStats.xBoundBoxExtents.y;
		xExtents.z = pxModel->xStats.xBoundBoxExtents.z;
		xRotBasis.rotateAboutZ( pxModelRot->z );
			
		xOtherExtents.x = fRadius;
		xOtherExtents.y = fRadius;
		xOtherExtents.z = fRadius;

		if ( OBBOverlap2( (float*)(&xExtents), &xModelPos1, xRotBasis, (float*)(&xOtherExtents), pxSpherePos, xOtherRotBasis ) )
		{					
			return( TRUE );
		}
	}	
	return( FALSE );
}


BOOL		EngineCheckCollision( int nHandle1, VECT* pxPos1, VECT* pxRotAngles1, int nHandle2, VECT* pxPos2, VECT* pxRotAngles2, int nFlags )
{
MODEL_RENDER_DATA*	pxModel;
MODEL_RENDER_DATA*	pxOtherModel;
//VECT		xVect;
//VECT		xVect2;
float			fDist;
VECT		xModelPos1;
VECT		xModelPos2;
#ifdef COLL_DEBUG_OUTPUT
char	acString[256];
#endif

	if ( ( nHandle1 == NOTFOUND ) ||
		 ( nHandle2 == NOTFOUND ) )
	{
		return( FALSE );
	}
	pxModel = &maxModelRenderData[ nHandle1 ];
	if ( pxModel->xCollisionAttachData.nModelHandle != NOTFOUND )
	{
		nHandle1 = pxModel->xCollisionAttachData.nModelHandle;
		pxModel = &maxModelRenderData[ nHandle1 ];
	}
	pxOtherModel = &maxModelRenderData[ nHandle2 ];
	if ( pxOtherModel->xCollisionAttachData.nModelHandle != NOTFOUND )
	{
		nHandle2 = pxOtherModel->xCollisionAttachData.nModelHandle;
		pxOtherModel = &maxModelRenderData[ nHandle2 ];
	}

    EngineMatrixRotationZ( &mxModel1RotMatrix, pxRotAngles1->z );
	EngineMatrixRotationZ( &mxModel2RotMatrix, pxRotAngles2->z );
	VectTransform( &xModelPos1, &pxModel->xStats.xBoundSphereCentre, &mxModel1RotMatrix );
	VectTransform( &xModelPos2, &pxOtherModel->xStats.xBoundSphereCentre, &mxModel2RotMatrix );

	// Do a quick sphere test first
	xModelPos1.x = pxPos1->x + xModelPos1.x;
	xModelPos1.y = pxPos1->y + xModelPos1.y;
	xModelPos1.z = pxPos1->z + xModelPos1.z;
	xModelPos2.x = pxPos2->x + xModelPos2.x;
	xModelPos2.y = pxPos2->y + xModelPos2.y;
	xModelPos2.z = pxPos2->z + xModelPos2.z;

	fDist = EngineGetDist( &xModelPos1, &xModelPos2 );

	if ( fDist < pxModel->xStats.fBoundSphereRadius + pxOtherModel->xStats.fBoundSphereRadius )
	{
//	VECT		xPos;
	VECT		xExtents;
//	VECT		xOtherPos;
	VECT		xOtherExtents;
	BASIS	xRotBasis;
	BASIS	xOtherRotBasis;

#ifdef COLL_DEBUG_OUTPUT
		sprintf( acString, "Sphere bounds coll rot1 %f rot2 %f", pxRotAngles1->z, pxRotAngles2->z );
		InterfaceText(2, 200,340,acString, COL_WARNING, 0 );
#endif
		xExtents.x = pxModel->xStats.xBoundBoxExtents.x;
		xExtents.y = pxModel->xStats.xBoundBoxExtents.y;
		xExtents.z = pxModel->xStats.xBoundBoxExtents.z;
		xRotBasis.rotateAboutZ( pxRotAngles1->z );
			
		xOtherExtents.x = pxOtherModel->xStats.xBoundBoxExtents.x;
		xOtherExtents.y = pxOtherModel->xStats.xBoundBoxExtents.y;
		xOtherExtents.z = pxOtherModel->xStats.xBoundBoxExtents.z;

		xOtherRotBasis.rotateAboutZ( pxRotAngles2->z );

		if ( OBBOverlap2( (float*)(&xExtents), pxPos1, xRotBasis, (float*)(&xOtherExtents), pxPos2, xOtherRotBasis ) )
		{
			// If flag set to indicate we're only interested in the bounding box collision,
			// return true here..

			// Otherwise.. now go into the mesh check against individual triangles..
			if ( nFlags	> 1 )
			{
				return( EngineMeshMeshTriangleCheck( pxModel, pxPos1, pxRotAngles1, &xRotBasis, pxOtherModel, pxPos2, pxRotAngles2,&xOtherRotBasis, nFlags ) );
			} 
			else if ( nFlags > 0 )
			{
#ifdef COLL_DEBUG_OUTPUT
				InterfaceText(2, 200,400,"Model bounds Collision active", COL_WARNING, 0 );
#endif
				return( EngineBoxMeshTriangleCheck( pxModel, pxPos1, pxRotAngles1, &xRotBasis, pxOtherModel, pxPos2, pxRotAngles2,&xOtherRotBasis, nFlags ) );
			} 
						
			return( TRUE );
		}
	}	
	return( FALSE );
}


BOOL		ModelSphereTest( int nModelHandle, const VECT* pxPos, const VECT* pxRot, const VECT* pxSpherePos, float fSphereRadius, VECT* pxHit, VECT* pxHitNormal )
{

	return( FALSE );
}



BOOL	EngineCollisionModelSphereSweepTest( int nMovingModel, VECT* pxPos1, VECT* pxPos2, int nStaticModel, VECT* pxStaticPos, VECT* pxRot, int nFlags )
{
MODEL_RENDER_DATA*	pxMovingModel;
float			fMovingSphereRadius;

	pxMovingModel = &maxModelRenderData[ nMovingModel ];
	fMovingSphereRadius = pxMovingModel->xStats.fBoundSphereRadius;

	return( EngineCollisionSphereSweepTest( fMovingSphereRadius, pxPos1, pxPos2, nStaticModel, pxStaticPos, pxRot, nFlags ) );
}



BOOL	EngineCollisionLineSphere( const VECT* pxLinePos1, const VECT* pxLinePos2, const VECT* pxSpherePos, float fSphereRadius, int nFlags )
{
VECT	xRayDir;
VECT	xTemp;
float	fDist;
float	fLen;
float	fDot;
float	fRayLen;
float	fSqrDist;

	VectSub( &xRayDir, pxLinePos1, pxLinePos2 );
//	fLen = VectGetLength( &xRayDir );
	fRayLen = VectNormalize( &xRayDir );

	// Get vect from start pos to sphere origin
	VectSub( &xTemp, pxSpherePos, pxLinePos2 );
	fLen = VectGetLength( &xTemp );
	fDot = VectDot( &xTemp, &xRayDir );
	fSqrDist = fSphereRadius * fSphereRadius - (fLen * fLen - fDot * fDot);

	if ( fSqrDist > 0.0f)
	{
		fDist = fDot - sqrtf( fSqrDist );

		if ( ( fDist > 0.0f ) &&
			 ( fDist < fRayLen ) )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}

	/*
Sphere	xSphere;
Vec3f	vecLine1;
Vec3f	vecLine2;
int		nNumIntersections;
float	fIntersectDist1;
float	fIntersectDist2;

	xSphere.center = *((Vec3f*)pxSpherePos);
	xSphere.radius = fRadius;
	vecLine1 = *((Vec3f*)pxLinePos1);
	vecLine2 = *((Vec3f*)pxLinePos1);
	vecLine2.x += xRayDir.x;
	vecLine2.y += xRayDir.y;
	vecLine2.z += xRayDir.z;
	if ( testIntersectionSphereLine( xSphere, vecLine1, vecLine2, &nNumIntersections, &fIntersectDist1, &fIntersectDist2 ) == true )
	{
	float	fRayLen = VectDist( pxLinePos1, pxLinePos2 );
		// The line intersects, now we need to check the distance to see if its within the ray
		if ( nNumIntersections == 2 )
		{
			if ( ( fabs(fIntersectDist1) > fRayLen ) &&
				 ( fabs(fIntersectDist2) > fRayLen ) )
			{
				return( FALSE );
			}
		}
		else
		{
			if ( fIntersectDist1 > fRayLen )
			{
				return( FALSE );
			}
		}
		return( TRUE );	
	}
	return( FALSE );
}

*/