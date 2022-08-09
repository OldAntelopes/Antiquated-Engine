
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../LibCode/Engine/ModelRendering.h"

#include "../Scene/ModelEditorSceneObject.h"
#include "../Util/Line Renderer.h"
#include "../ModelConverter.h"
#include "PlaneCutter.h"

BOOL		msbPlaneCutterActive = FALSE;
int			mhPlaneCutterVertexBuffer = NOTFOUND;


VECT		maxPlanePointsBase[8] = 
{
	{ 0.0f, 2.4f, 1.2f },
	{ 0.0f, -2.4f, 1.2f },
	{ 0.0f, -2.4f, 1.2f },
	{ 0.0f, -2.4f, -1.2f },
	{ 0.0f, -2.4f, -1.2f },
	{ 0.0f, 2.4f, -1.2f },
	{ 0.0f, 2.4f, -1.2f },
	{ 0.0f, 2.4f, 1.2f },
};

VECT		maxPlanePoints[8];
float		msfCutterPlaneRotateZ = 0.0f;
float		msfCutterPlaneOriginalRotateZ = 0.0f;
float		msfCutterPlaneRotateY = 0.0f;
float		msfCutterPlaneOriginalRotateY = 0.0f;

VECT		mxPlaneCutterBaseOffset = { 0.0f, 0.0f, 0.0f };
VECT		mxPlaneCutterOffset = { 0.0f, 0.0f, 0.0f };


ENGINEPLANE		mxPolySlicePlane;

void		PlaneCutterInitGraphics( void )
{
	if ( mhPlaneCutterVertexBuffer == NOTFOUND )
	{
	ENGINEBUFFERVERTEX	xVertex;
	int					nLoop;

		mhPlaneCutterVertexBuffer = EngineCreateVertexBuffer( 8, 0 ); 

		EngineVertexBufferLock( mhPlaneCutterVertexBuffer, TRUE );

		for( nLoop = 0; nLoop < 8; nLoop++ )
		{
			xVertex.position = maxPlanePoints[nLoop];
			xVertex.color = 0xFFF0D040;
			EngineVertexBufferAdd( mhPlaneCutterVertexBuffer, &xVertex );
		}
		EngineVertexBufferUnlock( mhPlaneCutterVertexBuffer );
	}
}

void		PlaneCutterFreeGraphics( void )
{
	EngineVertexBufferFree( mhPlaneCutterVertexBuffer );
	mhPlaneCutterVertexBuffer = NOTFOUND;

}

void		PlaneCutterInit( void )
{
	PlaneCutterInitGraphics();
}

void		PlaneCutterRender( void )
{
	if ( msbPlaneCutterActive )
	{
	VECT	xMid1;
	VECT	xMid2;
	int		nLoop;

		for( nLoop = 0; nLoop < 8; nLoop++ )
		{
			maxPlanePoints[nLoop] = maxPlanePointsBase[nLoop];
			VectRotateAboutY( &maxPlanePoints[nLoop], msfCutterPlaneRotateY );
			VectRotateAboutZ( &maxPlanePoints[nLoop], msfCutterPlaneRotateZ );
			VectAdd( &maxPlanePoints[nLoop], &maxPlanePoints[nLoop], &mxPlaneCutterOffset );
		}

		AddLine( &maxPlanePoints[0], &maxPlanePoints[1], 0xD0F0C040, 0xD0F0C040 );
		AddLine( &maxPlanePoints[2], &maxPlanePoints[3], 0xD0F0C040, 0xD0F0C040 );
		AddLine( &maxPlanePoints[4], &maxPlanePoints[5], 0xD0F0C040, 0xD0F0C040 );
		AddLine( &maxPlanePoints[6], &maxPlanePoints[7], 0xD0F0C040, 0xD0F0C040 );

		VectAdd( &xMid1, &maxPlanePoints[0], &maxPlanePoints[1] );
		VectScale( &xMid1, &xMid1, 0.5f );

		VectAdd( &xMid2, &maxPlanePoints[4], &maxPlanePoints[5] );
		VectScale( &xMid2, &xMid2, 0.5f );

		AddLine( &xMid1, &xMid2, 0xC0E0B040, 0xC0E0B040 );

		VectAdd( &xMid1, &maxPlanePoints[2], &maxPlanePoints[3] );
		VectScale( &xMid1, &xMid1, 0.5f );

		VectAdd( &xMid2, &maxPlanePoints[6], &maxPlanePoints[7] );
		VectScale( &xMid2, &xMid2, 0.5f );

		AddLine( &xMid1, &xMid2, 0xC0E0B040, 0xC0E0B040 );

	//	EngineVertexBufferRender( mhPlaneCutterVertexBuffer, LINE_LIST );
	}
}


BOOL		PlaneCutterIsActive( void )
{
	return( msbPlaneCutterActive );
}


void		PlaneCutterActivate( BOOL bFlag )
{
	msbPlaneCutterActive = bFlag;
}

void		PlaneCutterMouseDownStore( void )
{
	msfCutterPlaneOriginalRotateZ = msfCutterPlaneRotateZ;
	msfCutterPlaneOriginalRotateY = msfCutterPlaneRotateY;
	mxPlaneCutterBaseOffset = mxPlaneCutterOffset;

}

void		PlaneCutterLeftMouseUp( CSceneObject* pSceneObject, float fScreenXDelta, float fScreenYDelta )
{

}

void		PlaneCutterLeftMouseDownMove( CSceneObject* pSceneObject, float fScreenXDelta, float fScreenYDelta )
{
VECT	xMove;

	xMove = ModelConvGetScreenMoveVector( fScreenXDelta, fScreenYDelta, 0.01f );
	VectAdd( &mxPlaneCutterOffset, &mxPlaneCutterBaseOffset, &xMove );
}

void		PlaneCutterRightMouseDownMove( float fScreenXDelta, float fScreenYDelta )
{
	msfCutterPlaneRotateZ = msfCutterPlaneOriginalRotateZ + ( fScreenXDelta * 0.01f);
	if ( msfCutterPlaneRotateZ > A360 )
	{
		msfCutterPlaneRotateZ -= A360;
	}
	else if ( msfCutterPlaneRotateZ < 0.0f )
	{
		msfCutterPlaneRotateZ += A360;
	}

	msfCutterPlaneRotateY = msfCutterPlaneOriginalRotateY + ( fScreenYDelta * 0.01f);
	if ( msfCutterPlaneRotateY > A360 )
	{
		msfCutterPlaneRotateY -= A360;
	}
	else if ( msfCutterPlaneRotateY < 0.0f )
	{
		msfCutterPlaneRotateY += A360;
	}
}

float		PlaneCutterSnapAngleToAxis( float fAngle, float fTolerance )
{
	if ( ( fAngle > (A360 - fTolerance) ) ||
		 ( fAngle < fTolerance ) )
	{
		fAngle = 0.0f;
	}
	else if ( ( fAngle > (A90 - fTolerance) ) &&
			  ( fAngle < A90 + fTolerance ) )
	{
		fAngle = A90;
	}
	else if ( ( fAngle > (A180 - fTolerance) ) &&
			  ( fAngle < A180 + fTolerance ) )
	{
		fAngle = A180;
	}
	else if ( ( fAngle > (A270 - fTolerance) ) &&
			  ( fAngle < A270 + fTolerance ) )
	{
		fAngle = A270;
	}
	return( fAngle );
}

void		PlaneCutterAxisSnap( void )
{
	msfCutterPlaneRotateZ = PlaneCutterSnapAngleToAxis( msfCutterPlaneRotateZ, A10 );
	msfCutterPlaneRotateY = PlaneCutterSnapAngleToAxis( msfCutterPlaneRotateY, A10 );

}

void		PlaneCutterResetPlane( void )
{
	msfCutterPlaneRotateZ = 0.0f;
	msfCutterPlaneRotateY = 0.0f;
	mxPlaneCutterOffset.x = 0.0f;
	mxPlaneCutterOffset.y = 0.0f;
	mxPlaneCutterOffset.z = 0.0f;

}


class SliceFaceList
{
public:
	int		mnFaceNum;

	SliceFaceList*	mpNext;

};


BOOL	 PlaneCutterDoesTriIntersectPlane( const ENGINEPLANE* pxPlane, const VECT* pxVert1, const VECT* pxVert2, const VECT* pxVert3 )
{
VECT	xIntersect;

	if ( EnginePlaneIntersectLine( &xIntersect, pxPlane, pxVert1, pxVert2 ) == TRUE )
	{
		return( TRUE );
	}
	else if ( EnginePlaneIntersectLine( &xIntersect, pxPlane, pxVert2, pxVert3 ) == TRUE )
	{
		return( TRUE );
	}
	return( FALSE );
}

void	PlaneCutterSliceTri( ENGINEPLANE* pxPlane, ushort* puwFaceIndices, CUSTOMVERTEX* pxVertices, int* pnOldVertA, int* pnOldVertB, int* pnOldVertC, VECT* pxNewPointX, VECT* pxNewPointY )
{
VECT		xIntersect;
VECT		xIntersect01;
VECT		xIntersect12;
VECT		xIntersect02;
VECT*		pxVert0 = &pxVertices[puwFaceIndices[0]].position;
VECT*		pxVert1 = &pxVertices[puwFaceIndices[1]].position;
VECT*		pxVert2 = &pxVertices[puwFaceIndices[2]].position;

	// x = mid a & c
	// y = mid b & c
	if ( EnginePlaneIntersectLine( &xIntersect, pxPlane, pxVert0, pxVert1 ) == TRUE )
	{
		xIntersect01 = xIntersect;
		// intersects 0 and 1
		if ( EnginePlaneIntersectLine( &xIntersect, pxPlane, pxVert1, pxVert2 ) == TRUE )
		{
			xIntersect12 = xIntersect;
			*pxNewPointX = xIntersect01;
			*pxNewPointY = xIntersect12;

			// intersects 1 and 2, so c is 1
			*pnOldVertA = puwFaceIndices[0];
			*pnOldVertB = puwFaceIndices[2];
			*pnOldVertC = puwFaceIndices[1];
		}
		else
		{
			EnginePlaneIntersectLine( &xIntersect02, pxPlane, pxVert0, pxVert2 );
			*pxNewPointX = xIntersect01;
			*pxNewPointY = xIntersect02;

			// must intersect 0 and 2, so c is 0
			*pnOldVertA = puwFaceIndices[1];
			*pnOldVertB = puwFaceIndices[2];
			*pnOldVertC = puwFaceIndices[0];
		}
	}
	else  /// must intersect  1 and 2 + 0 and 2 so c is 2
	{
		EnginePlaneIntersectLine( &xIntersect12, pxPlane, pxVert1, pxVert2 );
		EnginePlaneIntersectLine( &xIntersect02, pxPlane, pxVert0, pxVert2 );
		*pxNewPointX = xIntersect12;
		*pxNewPointY = xIntersect02;

		*pnOldVertA = puwFaceIndices[1];
		*pnOldVertB = puwFaceIndices[0];
		*pnOldVertC = puwFaceIndices[2];
	}
}

VECT	PlaneCutterGetFaceNormal( CUSTOMVERTEX* pxVertices, int nVert1, int nVert2, int nVert3 )
{
VECT	xNormal;

	xNormal = pxVertices[nVert1].normal;
	VectAdd( &xNormal, &xNormal, &pxVertices[nVert2].normal );
	VectAdd( &xNormal, &xNormal, &pxVertices[nVert3].normal );
	VectNormalize( &xNormal );
	return( xNormal );
}


BOOL		PlaneCutterDoesNeedWindingFix( ushort* puwNewIndices, CUSTOMVERTEX* pxNewVertices, VECT* pxFaceNormal )
{
VECT	xVert1, xVert2, xVert3;
VECT	xCross1, xCross2, xCross3;
float	fDot;

	xVert1 = pxNewVertices[ puwNewIndices[0] ].position;
	xVert2 = pxNewVertices[ puwNewIndices[1] ].position;
	xVert3 = pxNewVertices[ puwNewIndices[2] ].position;

	// Calculate the normal for the new face with current winding
	VectCross( &xCross1, &xVert1, &xVert2 );
	VectCross( &xCross2, &xVert2, &xVert3 );
	VectCross( &xCross3, &xVert3, &xVert1 );

	xCross1.x = ( xCross1.x + xCross2.x + xCross3.x ) / 3.0f;
	xCross1.y = ( xCross1.y + xCross2.y + xCross3.y ) / 3.0f;
	xCross1.z = ( xCross1.z + xCross2.z + xCross3.z ) / 3.0f;

	VectNormalize( &xCross1 );

	fDot = VectDot( &xCross1, pxFaceNormal );
	// face is backwards so we need to flip em
	if ( fDot < 0.0f )
	{
		return( TRUE );
	}
	return( FALSE );
}

BOOL		PlaneCutterFixWinding( ushort* puwNewIndices, CUSTOMVERTEX* pxNewVertices, VECT* pxFaceNormal )
{
VECT	xVert1, xVert2, xVert3;
VECT	xCross1, xCross2, xCross3;
float	fDot;

	xVert1 = pxNewVertices[ puwNewIndices[0] ].position;
	xVert2 = pxNewVertices[ puwNewIndices[1] ].position;
	xVert3 = pxNewVertices[ puwNewIndices[2] ].position;

	// Calculate the normal for the new face with current winding
	VectCross( &xCross1, &xVert1, &xVert2 );
	VectCross( &xCross2, &xVert2, &xVert3 );
	VectCross( &xCross3, &xVert3, &xVert1 );

	xCross1.x = ( xCross1.x + xCross2.x + xCross3.x ) / 3.0f;
	xCross1.y = ( xCross1.y + xCross2.y + xCross3.y ) / 3.0f;
	xCross1.z = ( xCross1.z + xCross2.z + xCross3.z ) / 3.0f;

	VectNormalize( &xCross1 );

	fDot = VectDot( &xCross1, pxFaceNormal );
	// face is backwards so we need to flip em
	if ( fDot < 0.0f )
	{
	ushort	uwSwap = puwNewIndices[0];

		puwNewIndices[0] = puwNewIndices[1];
		puwNewIndices[1] = uwSwap;
		return( TRUE );
	}
	return( FALSE );
}


// --- do the slice thing...

void 	PlaneCutterSlice( CSceneObject* pSceneObject )
{
int		nNumSelectedFaces = pSceneObject->GetNumFacesSelected();

	EnginePlaneFromPoints( &mxPolySlicePlane, &maxPlanePoints[0], &maxPlanePoints[1], &maxPlanePoints[3] );

	// todo - if no faces selected we should operate on the entire model....
	if ( nNumSelectedFaces > 0 )
	{
	int		nLoop;
	int		nFaceNum;
	ushort*	puwIndices;
	ulong*	punIndices;
	int		nModelHandle = pSceneObject->GetModelHandle();
	MODEL_RENDER_DATA*	pxModelData;
	SliceFaceList*	pFaceList = NULL;
	SliceFaceList*	pSliceFace = NULL;
	SliceFaceList*	pLastFaceList = NULL;
	CUSTOMVERTEX*	pxVertices;
	CUSTOMVERTEX*	pxVert1;
	CUSTOMVERTEX*	pxVert2;
	CUSTOMVERTEX*	pxVert3;
	EngineMesh*		pNewMesh;
//	VECT			xVert1, xVert2, xVert3;
	ulong		unIndex1, unIndex2, unIndex3;
	int			nNewNumVerts;
	int			nNewNumFaces;

		pxModelData = maxModelRenderData + nModelHandle;

		nNewNumFaces = pxModelData->xStats.nNumIndices/3;
		nNewNumVerts = pxModelData->xStats.nNumVertices;

		pxModelData->pxBaseMesh->LockIndexBuffer( 0, (BYTE**)&puwIndices );
		if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
		{
			punIndices = (ulong*)( puwIndices );
		}

		pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &pxVertices ) );

		// --- First.. go through all the selected faces
		for ( nLoop = 0; nLoop < nNumSelectedFaces; nLoop++ )
		{
			nFaceNum = pSceneObject->GetSelectedFace( nLoop );

			if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
			{
				unIndex1 = punIndices[ (nFaceNum*3) ];
				unIndex2 = punIndices[ (nFaceNum*3)+1 ];
				unIndex3 = punIndices[ (nFaceNum*3)+2 ];
			}
			else
			{
				unIndex1 = puwIndices[ (nFaceNum*3) ];
				unIndex2 = puwIndices[ (nFaceNum*3)+1 ];
				unIndex3 = puwIndices[ (nFaceNum*3)+2 ];
			}

			pxVert1 = pxVertices + unIndex1;
			pxVert2 = pxVertices + unIndex2;
			pxVert3 = pxVertices + unIndex3;

			// count how many faces intersect the plane.. (and store them in a list)
			if ( PlaneCutterDoesTriIntersectPlane( &mxPolySlicePlane, &pxVert1->position, &pxVert2->position, &pxVert3->position ) == TRUE )
			{
				// add to list of faces to slice..
				pSliceFace = new SliceFaceList;
				pSliceFace->mnFaceNum = nFaceNum;
				if ( pLastFaceList )
				{
					pLastFaceList->mpNext = pSliceFace;
				}
				else
				{
					pFaceList = pSliceFace;
				}
				pLastFaceList = pSliceFace;
				pSliceFace->mpNext = NULL;
				// for each face that intersects we will add 2 new vertices, and 2 new faces
				nNewNumFaces += 2;
				nNewNumVerts += 2;
			}
		}
		
		// If we have stuff to do..
		if ( nNewNumFaces > pxModelData->xStats.nNumIndices/3 )
		{
		int		nOldFaceLoop;
		CUSTOMVERTEX*	pxNewVertices;
		ushort*			puwNewIndices;
		int		nOldVertsLoop;
		int		nNewVertNum;
		int		nNewVertX;
		int		nNewVertY;
		int		nOldVertA, nOldVertB, nOldVertC;
		VECT	xNewPointX, xNewPointY;
		VECT	xFaceNormal;
		BOOL	bWoundingFix;

			// create a new mesh, with the increased amount of vertices and faces.
			EngineCreateMesh( nNewNumFaces, nNewNumVerts, &pNewMesh, 0 );

			pNewMesh->LockVertexBuffer( NULL, (byte**)( &pxNewVertices ) );
			// copy all the old verts first..
			for ( nOldVertsLoop = 0; nOldVertsLoop < pxModelData->xStats.nNumVertices; nOldVertsLoop++ )
			{
				pxNewVertices[nOldVertsLoop] = pxVertices[nOldVertsLoop];
			}
			nNewVertNum = nOldVertsLoop;

			pNewMesh->LockIndexBuffer( 0, (BYTE**)( &puwNewIndices ) );
			pSliceFace = pFaceList;
			// copy the mesh, and for each face that is intersected..

				//          a ___________ b
				//            |        /
				//            |       /
				//			  |      /
				//            |     /
				//	   p1____x|____/y______p2
				//			  |   /
				//			  |  /
				//			  | /
				//			  |/
				//			 c
				// insert 2 new verts at the midpoint of the edges that intersect
				// replace a,b,c face with  x,y,c     a,x,y     a,b,y
				//  where (c is the vert shared between the 2 intersecting edges)
			for ( nOldFaceLoop = 0; nOldFaceLoop < pxModelData->xStats.nNumIndices/3; nOldFaceLoop++ )
			{
				if ( ( pSliceFace ) &&
					 ( nOldFaceLoop == pSliceFace->mnFaceNum ) )
				{
					nNewVertX = nNewVertNum++;
					nNewVertY = nNewVertNum++;

					PlaneCutterSliceTri( &mxPolySlicePlane, puwIndices + (nOldFaceLoop*3), pxVertices, &nOldVertA, &nOldVertB, &nOldVertC, &xNewPointX, &xNewPointY );

					xFaceNormal = PlaneCutterGetFaceNormal( pxVertices, nOldVertA, nOldVertB, nOldVertC );
					// add new verts (x,y)
					pxNewVertices[nNewVertX] = pxVertices[ nOldVertA ];
					pxNewVertices[nNewVertY] = pxVertices[ nOldVertB ];
	
					pxNewVertices[nNewVertX].position = xNewPointX;
					pxNewVertices[nNewVertY].position = xNewPointY;

					// replace a,b,c face with  x,y,c     a,x,y     a,b,y
					//  where (c is the vert shared between the 2 intersecting edges)
					puwNewIndices[0] = (ushort)( nNewVertY );
					puwNewIndices[1] = (ushort)( nNewVertX );
					puwNewIndices[2] = (ushort)( nOldVertC );
					bWoundingFix = PlaneCutterFixWinding( puwNewIndices, pxNewVertices, &xFaceNormal );
					puwNewIndices += 3;
				
					// todo - depending on the relative orientation of the tri (no, i dont know what that means yet..)
					// we should reverse the criteria for this.. i.e in some orientations, this method does the best job
					// but in others it does the worst. At least it is consistent across tris in the same plane :)

					// should be either A,Y,X + A,B,Y  or B,Y,X and A,B,X depending on winding?
					if ( bWoundingFix )
					{
						puwNewIndices[0] = (ushort)( nOldVertB );
						puwNewIndices[1] = (ushort)( nNewVertY );
						puwNewIndices[2] = (ushort)( nNewVertX );
						PlaneCutterFixWinding( puwNewIndices, pxNewVertices, &xFaceNormal );
						puwNewIndices += 3;

						puwNewIndices[0] = (ushort)( nOldVertA );
						puwNewIndices[1] = (ushort)( nOldVertB );
						puwNewIndices[2] = (ushort)( nNewVertX );
						PlaneCutterFixWinding( puwNewIndices, pxNewVertices, &xFaceNormal );
						puwNewIndices += 3;
					}
					else
					{
						puwNewIndices[0] = (ushort)( nOldVertA );
						puwNewIndices[1] = (ushort)( nNewVertY );
						puwNewIndices[2] = (ushort)( nNewVertX );
						PlaneCutterFixWinding( puwNewIndices, pxNewVertices, &xFaceNormal );
						puwNewIndices += 3;

						puwNewIndices[0] = (ushort)( nOldVertA );
						puwNewIndices[1] = (ushort)( nOldVertB );
						puwNewIndices[2] = (ushort)( nNewVertY );
						PlaneCutterFixWinding( puwNewIndices, pxNewVertices, &xFaceNormal );
						puwNewIndices += 3;
					}

					pSliceFace = pSliceFace->mpNext;
				}
				else
				{
					puwNewIndices[0] = puwIndices[ (nOldFaceLoop*3) ];
					puwNewIndices[1] = puwIndices[ (nOldFaceLoop*3)+1 ];
					puwNewIndices[2] = puwIndices[ (nOldFaceLoop*3)+2 ];
					puwNewIndices += 3;
				}
			}


			pNewMesh->UnlockIndexBuffer();
			pNewMesh->UnlockVertexBuffer( );

			pxModelData->pxBaseMesh->UnlockIndexBuffer();
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
			delete pxModelData->pxBaseMesh;
			pxModelData->pxBaseMesh = pNewMesh;
			pxModelData->xStats.nNumVertices = nNewNumVerts;
			pxModelData->xStats.nNumIndices = nNewNumFaces*3;

			while( pFaceList )
			{
				pSliceFace = pFaceList->mpNext;
				delete pFaceList;
				pFaceList = pSliceFace;
			}

			pSceneObject->OnModelChanged( TRUE );
		}
		else
		{
			pxModelData->pxBaseMesh->UnlockIndexBuffer();
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
	
}
