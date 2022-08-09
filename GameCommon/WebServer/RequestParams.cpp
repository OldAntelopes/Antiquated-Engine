
#include "StandardDef.h"

#include "RequestParams.h"


void	RequestParamsList::ParseParams( const char* szParams )
{
char	szKey[256];
char	szValue[512];
const char*	pcParamsRunner = szParams;
char*	pcKeyRunner;
char*	pcValueRunner;
RequestParam*		pNewParamKV;
int		nCount;

	while ( *pcParamsRunner != 0 )
	{
		pcKeyRunner = szKey;
		nCount = 0;
		while ( ( *pcParamsRunner != 0 ) &&
			    ( *pcParamsRunner != '=' ) &&
				( nCount < 100 ) )
		{
			*pcKeyRunner++ = *pcParamsRunner++;
			nCount++;
		}
		// Don't bother adding things that aren't valid
		if ( nCount == 100 )
		{
			return;
		}

		*pcKeyRunner = 0;
		if ( *pcParamsRunner == '=' )
		{
			pcParamsRunner++;
			pcValueRunner = szValue;
			nCount = 0;
			while ( ( *pcParamsRunner != 0 ) &&
				    ( *pcParamsRunner != '&' ) &&
					( nCount < 300 ) )
			{
				*pcValueRunner++ = *pcParamsRunner++;
				nCount++;
			}
			*pcValueRunner = 0;

			// Don't bother adding things that aren't valid
			if ( nCount == 300 )
			{
				return;
			}
			pNewParamKV = new RequestParam;
			pNewParamKV->Init( szKey, szValue );
			pNewParamKV->mpNext = mpList;
			mpList = pNewParamKV;

			if ( *pcParamsRunner == '&' )
			{
				pcParamsRunner++;
			}
		}
	}

}

void	RequestParamsList::Copy( RequestParamsList* pOut )
{
RequestParam*		pList = mpList;
RequestParam*		pNew;

	while( pList )
	{
		pNew = new RequestParam;
		pNew->mszKey = (char*)( malloc( strlen( pList->mszKey ) + 1 ) );
		strcpy( pNew->mszKey, pList->mszKey );
		pNew->mszValue = (char*)( malloc( strlen( pList->mszValue ) + 1 ) );
		strcpy( pNew->mszValue, pList->mszValue );
		pNew->mpNext = pOut->mpList;
		pOut->mpList = pNew;

		pList = pList->mpNext;
	}
}
