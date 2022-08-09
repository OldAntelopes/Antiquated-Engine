
#include "StandardDef.h"

#include "SortedList.h"

SortedList::~SortedList()
{
	Reset();
}

void	SortedList::Reset( void )
{
SortedListEntry*	pEntries = mpListEntries;
SortedListEntry*	pNext;

	while( pEntries )
	{
		pNext = pEntries->mpNext;
		delete pEntries;
		pEntries = pNext;
	}
	mpListEntries = NULL;
	mfLowestScore = 0.0f;
	mnNumInList = 0;
}

void	SortedList::Add( void* pData, float fScore )
{
SortedListEntry*	pEntries = mpListEntries;
SortedListEntry*	pLast = NULL;
SortedListEntry*	pNewEntry = new SortedListEntry;

	if ( ( mfLowestScore == -1.0f ) || 
		 ( fScore < mfLowestScore ) )
	{
		mfLowestScore = fScore;
	}
	pNewEntry->mfScore = fScore;
	pNewEntry->mpData = pData;
	mnNumInList++;

	while( pEntries )
	{
		if ( (( mbSortHighestFirst ) && ( fScore > pEntries->mfScore )) ||
			 (( !mbSortHighestFirst ) && ( fScore < pEntries->mfScore )) )
		{
			if ( pLast == NULL )
			{
				pNewEntry->mpNext = mpListEntries;
				mpListEntries = pNewEntry;
				return;
			}
			else
			{
				pNewEntry->mpNext = pEntries;
				pLast->mpNext = pNewEntry;
				return;
			}
		}
		pLast = pEntries;
		pEntries = pEntries->mpNext;
	}

	pNewEntry->mpNext = NULL;
	if ( pLast == NULL )
	{
		mpListEntries = pNewEntry;
	}
	else
	{
		pLast->mpNext = pNewEntry;
	}
}
	
void*	SortedList::GetEntry( int index )
{
int		nCount = 0;
SortedListEntry*	pEntries = mpListEntries;

	while( pEntries )
	{
		if ( nCount == index )
		{
			return( pEntries->mpData );
		}
		nCount++;
		pEntries = pEntries->mpNext;
	}
	return( NULL );
}
