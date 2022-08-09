#ifndef GAMECOMMON_UTIL_SORTEDLIST_H
#define GAMECOMMON_UTIL_SORTEDLIST_H

class SortedListEntry
{
public:
	void*		mpData;
	float		mfScore;
	SortedListEntry*		mpNext;
};

class SortedList
{
public:
	SortedList()
	{
		mpListEntries = NULL;
		mbSortHighestFirst = TRUE;
		mnNumInList = 0;
		mfLowestScore = -1.0f;
	}

	~SortedList();

	void	Reset( void );
	void	SetSortOrder( BOOL bHighestFirst ) { mbSortHighestFirst = bHighestFirst; }

	void	Add( void* pData, float fScore );
	
	void*	GetEntry( int index );
	int		GetNumInList( void ) { return( mnNumInList ); }
	float	GetLowestScore( void ) { return( mfLowestScore ); }

private:
	SortedListEntry*		mpListEntries;
	BOOL	mbSortHighestFirst;
	int		mnNumInList;
	float	mfLowestScore;
};



#endif
