
#include "StandardDef.h"

#include "../Landscape/LandscapeCoords.h"
#include "../Landscape/LandscapeHeightmap.h"

#include "Trees.h"
#include "TreeLists.h"

#define		TREE_LISTS_SIZE_X		64
#define		TREE_LISTS_SIZE_Y		64
#define		NUM_TREE_LISTS			(TREE_LISTS_SIZE_X*TREE_LISTS_SIZE_Y)


typedef struct 
{
	uint32		ulRenderFrameTick;
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
	}


}


void	TreeListsAddTree( Tree* pTree )
{
int		nListIndex;

	nListIndex = TreeListGetListPosWorld( pTree->mxPos.x, pTree->mxPos.y );
	pTree->mpNext = maTreeLists[nListIndex].pTreeList;
	maTreeLists[nListIndex].pTreeList = pTree;
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

void	TreeListsProcessView( fnProcessTreeCallback fnProcess, uint32 ulParam  )
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
				fnProcess( pTree, ulParam );
				pTree = pTree->mpNext;
			}
		}
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
