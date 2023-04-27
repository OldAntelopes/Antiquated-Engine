
#include "StandardDef.h"
#include "Engine.h"
#include "Rendering.h"

#include "../../../GameCommon/Landscape/LandscapeCoords.h"
#include "../../../GameCommon/Landscape/LandscapeHeightmap.h"

#include "Trees.h"
#include "TreeLists.h"

#define		TREE_LISTS_SIZE_X		64
#define		TREE_LISTS_SIZE_Y		64
#define		NUM_TREE_LISTS			(TREE_LISTS_SIZE_X*TREE_LISTS_SIZE_Y)


typedef struct 
{
	uint32		ulRenderFrameTick;
	int			nNumTrees;
	Tree*		pTreeList;

} TREE_LIST;

TREE_LIST	maTreeLists[NUM_TREE_LISTS];

int		mnTreeListViewMinX = 0;
int		mnTreeListViewMaxX = 0;
int		mnTreeListViewMinY = 0;
int		mnTreeListViewMaxY = 0;


int		TreeListGetListPosWorld( float fWorldX, float fWorldY )
{
int		nTreeListX, nTreeListY;
float	fMapModX = (float)TREE_LISTS_SIZE_X / LandscapeGetMapSizeX();
float	fMapModY = (float)TREE_LISTS_SIZE_Y / LandscapeGetMapSizeY();
float	fWorldToGameMap = LandscapeWorldToMapScale();
int		nIndex;

	nTreeListX = (int)( (fWorldX * fWorldToGameMap) * fMapModX );
	nTreeListY = (int)( (fWorldY * fWorldToGameMap) * fMapModY );
	nIndex = abs( (nTreeListY * TREE_LISTS_SIZE_X) + nTreeListX );
	nIndex %= NUM_TREE_LISTS;
	return( nIndex );
}


//--------------------------------------------

void	TreeListsInit( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < NUM_TREE_LISTS; nLoop++ )
	{
		maTreeLists[nLoop].pTreeList = NULL;
		maTreeLists[nLoop].nNumTrees = 0;
	}
}


void	TreeListsAddTree( Tree* pTree )
{
int		nListIndex;

	nListIndex = TreeListGetListPosWorld( pTree->mxPos.x, pTree->mxPos.y );
	pTree->mpNext = maTreeLists[nListIndex].pTreeList;
	maTreeLists[nListIndex].pTreeList = pTree;
	maTreeLists[nListIndex].nNumTrees++;
}

void	TreeListsUpdate( float fDelta )
{


}

void		TreeListsSetView( int nGameMapMinX, int nGameMapMinY, int nGameMapMaxX, int nGameMapMaxY )
{
float	fMapModX = (float)TREE_LISTS_SIZE_X / LandscapeGetMapSizeX();
float	fMapModY = (float)TREE_LISTS_SIZE_Y / LandscapeGetMapSizeY();
float	fWorldToGameMap = LandscapeWorldToMapScale();

	mnTreeListViewMinX = (int)( nGameMapMinX * fMapModX ) - 1;
	mnTreeListViewMaxX = (int)( nGameMapMaxX * fMapModX ) + 1;
	mnTreeListViewMinY = (int)( nGameMapMinY * fMapModY ) - 1;
	mnTreeListViewMaxY = (int)( nGameMapMaxY * fMapModY ) + 1;

	if ( mnTreeListViewMinX < 0 )
	{
		mnTreeListViewMinX = 0;
	}
	else if ( mnTreeListViewMinX > TREE_LISTS_SIZE_X-1 )
	{
		mnTreeListViewMinX = TREE_LISTS_SIZE_X-1;
	}

	if ( mnTreeListViewMinY < 0 )
	{
		mnTreeListViewMinY = 0;
	}
	else if ( mnTreeListViewMinY > TREE_LISTS_SIZE_Y-1 )
	{
		mnTreeListViewMinY = TREE_LISTS_SIZE_Y-1;
	}

	if ( mnTreeListViewMaxX < 0 )
	{
		mnTreeListViewMaxX = 0;
	}
	else if ( mnTreeListViewMaxX > TREE_LISTS_SIZE_X-1 )
	{
		mnTreeListViewMaxX = TREE_LISTS_SIZE_X-1;
	}
	if ( mnTreeListViewMaxY < 0 )
	{
		mnTreeListViewMaxY = 0;
	}
	else if ( mnTreeListViewMaxY > TREE_LISTS_SIZE_Y-1 )
	{
		mnTreeListViewMaxY = TREE_LISTS_SIZE_Y-1;
	}
}

void	TreeListsRenderPass1( void )
{
int		nListLoopY;
int		nListLoopX;
int		nListIndex;
Tree*	pTree;

	for ( nListLoopY = mnTreeListViewMinY; nListLoopY < mnTreeListViewMaxY; nListLoopY++ )
	{
		for ( nListLoopX = mnTreeListViewMinX; nListLoopX < mnTreeListViewMaxX; nListLoopX++ )
		{
			nListIndex = (nListLoopY*TREE_LISTS_SIZE_X)+nListLoopX;
			pTree = maTreeLists[nListIndex].pTreeList;
			while( pTree )
			{
				pTree->RenderTrunk();
				pTree = pTree->mpNext;
			}
		}
	}
//	ModelRenderingFlush();

}

void	TreeListsRenderPass2( void )
{
int		nListLoopY;
int		nListLoopX;
int		nListIndex;
Tree*	pTree;

	for ( nListLoopY = mnTreeListViewMinY; nListLoopY <= mnTreeListViewMaxY; nListLoopY++ )
	{
		for ( nListLoopX = mnTreeListViewMinX; nListLoopX <= mnTreeListViewMaxX; nListLoopX++ )
		{
			nListIndex = (nListLoopY*TREE_LISTS_SIZE_X)+nListLoopX;
			pTree = maTreeLists[nListIndex].pTreeList;
			while( pTree )
			{
				pTree->Render();
				pTree = pTree->mpNext;
			}
		}
	}

}

void	TreeListsRender( void )
{


}

void	TreeListsFree( void )
{
Tree*	pTree;
Tree*	pNext;
int		nLoop;

	for ( nLoop = 0; nLoop < NUM_TREE_LISTS; nLoop++ )
	{
		pTree = maTreeLists[nLoop].pTreeList;
		while( pTree )
		{
			pNext = pTree->mpNext;
			delete pTree;
			pTree = pNext;
		}
		maTreeLists[nLoop].pTreeList = NULL;
	}


}




//--------------------------------------- 


int		TreeListGetListPos( int nGameMapX, int nGameMapY )
{
int		nTreeListX, nTreeListY;
float	fMapModX = (float)TREE_LISTS_SIZE_X / LandscapeGetMapSizeX();
float	fMapModY = (float)TREE_LISTS_SIZE_Y / LandscapeGetMapSizeY();
float	fWorldToGameMap = LandscapeWorldToMapScale();
int		nIndex;

	nTreeListX = (int)( nGameMapX * fMapModX );
	nTreeListY = (int)( nGameMapY * fMapModY );
	nIndex = abs( (nTreeListY * TREE_LISTS_SIZE_X) + nTreeListX );
	nIndex %= NUM_TREE_LISTS;
	return( nIndex );
}


Tree*	TreesGetRandomTree( int nGameMapMinX, int nGameMapMinY, int nGameMapMaxX, int nGameMapMaxY )
{
int		nListIndex;
int		nTreeNum;
int		nPickX;
int		nPickY;

	nPickX = nGameMapMaxX - nGameMapMinX;
	if ( nPickX > 1 )
	{
		nPickX = rand() % nPickX;
	}
	nPickX += nGameMapMinX;

	nPickY = nGameMapMaxY - nGameMapMinY;
	if ( nPickY > 1 )
	{
		nPickY = rand() % nPickY;
	}
	nPickY += nGameMapMinY;

	nListIndex = TreeListGetListPos( nPickX, nPickY );

	while ( maTreeLists[nListIndex].nNumTrees == 0 )
	{
		nPickX++;
		if ( nPickX > nGameMapMaxX )
		{
			nPickX = nGameMapMinX;
			nPickY++;
			if ( nPickY > nGameMapMaxY )
			{
				return( NULL );
			}
		}
		nListIndex = TreeListGetListPos( nPickX, nPickY );
	}

	if ( maTreeLists[nListIndex].nNumTrees > 0 )
	{
	Tree*		pTree;
	int			nCount = 0;

		nTreeNum = rand() % maTreeLists[nListIndex].nNumTrees;
		pTree = maTreeLists[nListIndex].pTreeList;
		while( pTree )
		{
			if ( nCount == nTreeNum )
			{
				return( pTree );
			}
			nCount++;
			pTree = pTree->mpNext;
		}
	}
	return( NULL );
}

