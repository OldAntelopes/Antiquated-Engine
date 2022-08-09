#ifndef WEBSERVER_REQUEST_PARAMS_H
#define WEBSERVER_REQUEST_PARAMS_H

class RequestParam
{
public:
	RequestParam()
	{
		mszKey = NULL;
		mszValue = NULL;
		mpNext = NULL;
	}

	~RequestParam()
	{
		if ( mszKey )
		{
			free( mszKey );
		}
		if ( mszValue )
		{
			free( mszValue );
		}
	}

	void	Init( const char* szKey, const char* szValue )
	{
		mszKey = (char*)malloc( strlen( szKey ) + 1 );
		strcpy( mszKey, szKey );
		mszValue = (char*)malloc( strlen( szValue ) + 1 );
		strcpy( mszValue, szValue );
	}
	
	char*			mszKey;
	char*			mszValue;
	RequestParam*		mpNext;
};


class RequestParamsList
{
public:
	RequestParamsList()
	{
		mpList = NULL;
	}
	~RequestParamsList()
	{
		Clear();
	}

	void	ParseParams( const char* szParams );

	const char*		GetValue( const char* szKey )
	{
	RequestParam*		pList = mpList;

		while( pList )
		{
			if ( stricmp( pList->mszKey, szKey ) == 0 )
			{
				return( pList->mszValue );
			}
			pList = pList->mpNext;
		}
		return( NULL );
	}

	void	Copy( RequestParamsList* pOut );

	void	Clear()
	{
	RequestParam*		pList = mpList;
	RequestParam*		pNext;

		while( pList )
		{
			pNext = pList->mpNext;
			delete pList;
			pList = pNext;
		}
		mpList = NULL;
	}

	RequestParam*		mpList;
};



#endif