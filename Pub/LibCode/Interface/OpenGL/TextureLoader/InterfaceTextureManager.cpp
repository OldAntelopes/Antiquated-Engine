
#include <windows.h>			// For OpenGL
#include <StandardDef.h>
#include <Interface.h>

#include "TextureLoader.h"
#include "InterfaceTextureManager.h"
#include "../../Common/InterfaceUtil.h"

static TextureLoader		ms_TextureLoader;


class InterfaceTextureListItem
{
public:
	InterfaceTextureListItem()
	{
		mpNext = NULL;
		m_Handle = -1;
		mpGLTexture = NULL;
	}

	~InterfaceTextureListItem()
	{

	}
	
	InterfaceTextureListItem*	mpNext;
	int							m_Handle;
	glTexture*					mpGLTexture;
};


InterfaceTextureListItem*		m_pManagedTextures = NULL;
int			m_nInterfaceTextureManagerNextHandle = 1;

void		InterfaceTextureManagerInit( void )
{


}


void		InterfaceTextureManagerUpdate( void )
{


}


void		InterfaceTextureManagerFree( void )
{

}


int		InterfaceTextureManagerGetNewTexture( glTexture** ppTexture )
{
int		nHandle = m_nInterfaceTextureManagerNextHandle++;
glTexture*		pNewTexture;
InterfaceTextureListItem*		pNewListItem;

	pNewListItem = new InterfaceTextureListItem;
	pNewTexture = new glTexture;

	*ppTexture = pNewTexture;

	pNewListItem->m_Handle = nHandle;
	pNewListItem->mpGLTexture = pNewTexture;

	pNewListItem->mpNext = m_pManagedTextures;
	m_pManagedTextures = pNewListItem;

	return( nHandle );
}

int	InterfaceGetTextureSize( int nTextureHandle, int* pnW, int* pnH )
{
	// TODO
	return( 0 );
}

BYTE*	InterfaceLockTexture( int nTextureHandle, int* pnPitch, int* pnFormat, int nFlags )
{
	// TODO
	return( 0 );
}

void	InterfaceUnlockTexture( int nTextureHandle )
{
	// TODO
}

void	InterfaceTextureGetColourAtPoint( byte* pbLockedTextureData, int nPitch, int nFormat, int x, int y, float* pfRed, float* pfGreen, float* pfBlue, float* pfAlpha )
{
	// todo
}

InterfaceTextureListItem*		InterfaceTextureManagerGetManagedItem( int nTextureHandle )
{
	if ( nTextureHandle > 0 )
	{
	InterfaceTextureListItem*		pList = m_pManagedTextures;

		while( pList )
		{
			if ( pList->m_Handle == nTextureHandle )
			{
				return( pList );
			}
			pList = pList->mpNext;
		}
	}
	return( NULL );
}

INTERFACE_API void	InterfaceSetTextureAsCurrent( int nTextureHandle )
{
InterfaceTextureListItem*		pItem = InterfaceTextureManagerGetManagedItem( nTextureHandle );

	if ( pItem )
	{
		glBindTexture (GL_TEXTURE_2D, pItem->mpGLTexture->TextureID);
	}
}

//------------------------------------------------------------------------------- External APIs


//-------------------------------------------------------
// Function : InterfaceGetTexture
//
//-------------------------------------------------------
INTERFACE_API int	InterfaceGetTexture( const char* szFilename, int nFlags )
{
glTexture*	pTexture;
int			nHandle;

	nHandle = InterfaceTextureManagerGetNewTexture( &pTexture );

	// Quickee test main thread implementation
	ms_TextureLoader.LoadTextureFromDisk( szFilename, pTexture );

	return( nHandle );
}



INTERFACE_API int  InterfaceGetTextureFromFileInMem( const char* szFilename, unsigned char* pbMem, int nMemSize, int nFlags )
{
glTexture*	pTexture;
int			nHandle;

	nHandle = InterfaceTextureManagerGetNewTexture( &pTexture );

	// Quickee test main thread implementation
	ms_TextureLoader.LoadTextureFromRam( pbMem, nMemSize, pTexture, txUnknown );

	return( nHandle );
}


 int InterfaceGetTextureInternal( const char* szFilename, int nFlags, int nArchiveHandle )
{
	// TODO  

	return( 0 );
}
