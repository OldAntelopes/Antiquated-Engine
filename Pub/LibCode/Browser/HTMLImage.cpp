
#include <stdio.h>
#include <string.h>
#include <d3dx9.h>
#include <StandardDef.h>


#include <Interface.h>

#include "../../../Universal/GameCode/Globals/Globals.h"		// TEMP - Needs fixing
#include "../../../Universal/LibCode/Networking/HTTP/HttpDownload.h"		// TEMP - Needs fixing

#include "HTMLParser.h"
#include "HTMLImage.h"
#include "NoddyBrowser.h"


class HTMLImageSessionCacheElement
{
public:
	char	maszImageFullPathName[256];
	char	maszImageCacheFileName[64];
};

class HTMLImageSessionCache
{
public:
	HTMLImageSessionCache();

	void	AddImageToSessionCache( const char* mszFullPath, const char* );
	char*	CheckForFileInSessionCache( const char* mszFullPath );
private:
#define MAX_IMAGES_IN_SESSION_CACHE	128
	int		mnSessionCacheUsed;
	HTMLImageSessionCacheElement*		mapElements[MAX_IMAGES_IN_SESSION_CACHE];
};

HTMLImageSessionCache::HTMLImageSessionCache()
{
	ZeroMemory( mapElements, sizeof( HTMLImageSessionCacheElement* ) * MAX_IMAGES_IN_SESSION_CACHE );
	mnSessionCacheUsed = 0;
}

void	HTMLImageSessionCache::AddImageToSessionCache( const char* mszFullPath, const char* szLocalImageName )
{
int		Loop;

	for ( Loop = 0; Loop < mnSessionCacheUsed; Loop++ )
	{
		if ( mapElements[ Loop ] )
		{
			if ( tinstricmp( mapElements[ Loop ]->maszImageFullPathName, mszFullPath ) == 0 )
			{
				return;
			}
		}
	}

	if ( mnSessionCacheUsed < MAX_IMAGES_IN_SESSION_CACHE )
	{
	char	acFullPathToCacheFile[256];

		mapElements[ mnSessionCacheUsed ] = new HTMLImageSessionCacheElement;
		strncpy( mapElements[ mnSessionCacheUsed ]->maszImageFullPathName, mszFullPath, 256 );
		sprintf( mapElements[ mnSessionCacheUsed ]->maszImageCacheFileName, "Data/Temp/Cache%04d.dat", mnSessionCacheUsed );
		sprintf( acFullPathToCacheFile, "%s/%s", SysGetWritableDataFolderPath( "The Universal" ), mapElements[ mnSessionCacheUsed ]->maszImageCacheFileName );
		SysCopyFile( szLocalImageName, acFullPathToCacheFile, FALSE );
		mnSessionCacheUsed++;
	}
}

char*	HTMLImageSessionCache::CheckForFileInSessionCache( const char* mszFullPath )
{
int		Loop;

	for ( Loop = 0; Loop < mnSessionCacheUsed; Loop++ )
	{
		if ( mapElements[ Loop ] )
		{
			if ( tinstricmp( mapElements[ Loop ]->maszImageFullPathName, mszFullPath ) == 0 )
			{
				return( mapElements[ Loop ]->maszImageCacheFileName );
			}
		}
	}
	return( NULL );
}

class HTMLImageFactory
{
public:
	HTMLImageFactory();

	HTMLImageTarget*		FindImage( char* szName );
	void			RequestDownload( HTMLImageTarget* );
	void		ImageDownloadCallback( int, long, const char*, int );
	void		FreeAll( void );
private:
	HTMLImageTarget*		CreateNew( char* szName );

#define	MAX_IMAGES_PER_PAGE		128
	
	HTMLImageTarget*		m_apHTMLImageList[MAX_IMAGES_PER_PAGE];
	int				m_nNumInImageList;
	int				m_DownloadPipe;

};

HTMLImageFactory			m_sImageFactorySingleton;
HTMLImageSessionCache		m_sImageSessionCache;

HTMLImageFactory::HTMLImageFactory()
{
	m_nNumInImageList = 0;
	m_DownloadPipe = 0;
	ZeroMemory( m_apHTMLImageList, sizeof( HTMLImageTarget* ) * MAX_IMAGES_PER_PAGE );
}

void	HTMLImageFactory::ImageDownloadCallback( int nRet, long lParam, const char* szName, int nHTTPResult )
{
HTMLImageTarget*	pImage = (HTMLImageTarget*)( lParam );

	if ( m_DownloadPipe != 0 )
	{
		m_DownloadPipe = 0;
	}

	if ( nRet == FETCHFILE_SUCCESS )
	{
	char*	pcExtension;

		pcExtension = pImage->m_szImageName +  ((strlen(pImage->m_szImageName) - 3) );
		if ( ( stricmp( pcExtension,"jpg" ) == 0 ) ||
			 ( stricmp( pcExtension,"bmp" ) == 0 ) ||
			 ( stricmp( pcExtension,"png" ) == 0 ) )
		{
		FILE*	pFile;
		int		nFileSize;
		char	acFullPath[256];
			sprintf( acFullPath, "%s\\%s", SysGetWritableDataFolderPath("The Universal" ), szName );
			pFile = fopen( acFullPath, "rb" );
			if ( pFile )
			{
				fseek( pFile, 0, SEEK_END );
				nFileSize = ftell(pFile);
				rewind( pFile );
				fclose( pFile );

				if ( nFileSize > 64 )
				{
					m_sImageSessionCache.AddImageToSessionCache( pImage->m_szImageName, acFullPath );

	// TODO - Mmmmm, need to sort this out
/*   (THIS IS THE OLD CODE PRIOR TO CONVERSION TO DX9.. REPLACEMENT BELOW PROBABLY DOESNT DO SCALING RIGHT
					D3DSURFACE_DESC		xSurface;
					int		nActualHeight;
					int		nActualWidth;
					LPGRAPHICSTEXTURE	pSurface = LoadJpegDirect( szName, 0, 0, 0 );
					if ( pSurface )
					{
						pSurface->GetDesc( &xSurface );
						pSurface->Release();
						nActualWidth = xSurface.Width;
						nActualHeight = xSurface.Height;
						pImage->m_pxTexture = InterfaceLoadTextureDX( szName, 1, 1 );
						if ( pImage->m_pxTexture )
						{
						D3DSURFACE_DESC		xSurface;
							pImage->m_pxTexture->GetLevelDesc( 0, &xSurface );
							pImage->m_ImageWidth = xSurface.Width;
							pImage->m_ImageHeight = xSurface.Height;
							pImage->m_LoadState = BROWSER_IMG_STATE_LOADED;
							if ( pImage->m_ImageHeight > 1 )
							{
								pImage->m_fVMod = (float)(nActualHeight) / (pImage->m_ImageHeight);
							}
							if ( pImage->m_ImageWidth > 1 )
							{
								pImage->m_fUMod = (float)(nActualWidth) / (pImage->m_ImageWidth);
							}
						}
						else
						{
							pImage->m_LoadState = BROWSER_IMG_STATE_NOT_AVAILABLE;
						}
*/
					pImage->m_hTexture = InterfaceLoadTexture( acFullPath, 0 );
					if ( pImage->m_hTexture > 0 )
					{
						InterfaceGetTextureSize( pImage->m_hTexture, &pImage->m_ImageWidth, &pImage->m_ImageHeight );
						pImage->m_LoadState = BROWSER_IMG_STATE_LOADED;

						pImage->m_fVMod = 1.0f;
						pImage->m_fUMod = 1.0f;
						/*
							if ( pImage->m_ImageHeight > 1 )
							{
								pImage->m_fVMod = (float)(nActualHeight) / (pImage->m_ImageHeight);
							}
							if ( pImage->m_ImageWidth > 1 )
							{
								pImage->m_fUMod = (float)(nActualWidth) / (pImage->m_ImageWidth);
							}
							*/
					}
					else
					{
						pImage->m_LoadState = BROWSER_IMG_STATE_NOT_AVAILABLE;
					}
				}
				else
				{
					pImage->m_LoadState = BROWSER_IMG_STATE_NOT_AVAILABLE;
				}
			}
			else
			{
				pImage->m_LoadState = BROWSER_IMG_STATE_NOT_AVAILABLE;
			}
		}
		else
		{
			pImage->m_LoadState = BROWSER_IMG_STATE_NOT_AVAILABLE;
		}

	}
	else
	{
		pImage->m_LoadState = BROWSER_IMG_STATE_NOT_AVAILABLE;
	}
}

void HTMLImageDownloadCallback( int nRet, long lParam, const char* szName, int nHTTPResult )
{
	m_sImageFactorySingleton.ImageDownloadCallback( nRet, lParam, szName, nHTTPResult );
}


void	HTMLImageFactory::RequestDownload( HTMLImageTarget* pImage )
{
	if ( m_DownloadPipe == 0 )
	{
		m_DownloadPipe = 1;
		pImage->m_LoadState = BROWSER_IMG_STATE_DOWNLOADING;
		HTTPGetCacheFilename( pImage->m_szImageName, pImage->m_szLocalFileName );
		HTTPDownloadInitiate( 0, pImage->m_szImageName, pImage->m_szLocalFileName, HTMLImageDownloadCallback,NULL, (long)( pImage ), FALSE );
	}
}

HTMLImageTarget*	HTMLImageFactory::CreateNew( char* szName )
{
HTMLImageTarget*	pNewImage;

	if ( m_nNumInImageList < MAX_IMAGES_PER_PAGE )
	{
		pNewImage = new HTMLImageTarget;
		pNewImage->Load( szName );
		m_apHTMLImageList[ m_nNumInImageList ] = pNewImage;
		m_nNumInImageList++;
		return( pNewImage );
	}
	return( NULL );
}

HTMLImageTarget*	HTMLImageFactory::FindImage( char* szName )
{
int		Loop;
char*	pcImageName;

	for ( Loop = 0; Loop < m_nNumInImageList; Loop++ )
	{
		pcImageName = m_apHTMLImageList[Loop]->GetName();
		if ( tinstricmp( pcImageName, szName ) == 0 )
		{
			return( m_apHTMLImageList[Loop] );
		}
	}
	return( CreateNew( szName ) );
}

void	HTMLImageFactory::FreeAll( void )
{
int	Loop;

	for ( Loop = 0; Loop < m_nNumInImageList; Loop++ )
	{
		if ( m_apHTMLImageList[Loop]->m_hTexture > 0 )
		{
			InterfaceReleaseTexture( m_apHTMLImageList[Loop]->m_hTexture );
			m_apHTMLImageList[Loop]->m_hTexture = NOTFOUND;
		}
		delete m_apHTMLImageList[Loop];
	}
	m_nNumInImageList = 0;
}

//---------------------------------------------------------
void	HTMLImageTarget::FreeAllImages( void )
{
	m_sImageFactorySingleton.FreeAll();
}


HTMLImageTarget::HTMLImageTarget()
{
	m_LoadState = BROWSER_IMG_STATE_NONE;
	m_hTexture = NOTFOUND;
	m_fUMod = 1.0f;
	m_fVMod = 1.0f;
	m_ImageWidth = -1;
	m_ImageHeight = -1;
}


void HTMLImageTarget::Load(char* szName )
{
	strcpy( m_szImageName, szName );
}

void	HTMLImageTarget::GetImageTexture( void  )
{
	if ( strnicmp( m_szImageName, "http", 4 ) == 0 )
	{
	char*	szCachedFile;

		szCachedFile = m_sImageSessionCache.CheckForFileInSessionCache( m_szImageName );
		if ( szCachedFile )
		{
			m_sImageFactorySingleton.ImageDownloadCallback( 1, (long)(this), szCachedFile, 0 );
		}
		else
		{
			m_sImageFactorySingleton.RequestDownload( this );
		}
	}
	else
	{
	FILE*	pFile;

		// TODO - Will need to use the browser root and trigger HTTP download if necc
		pFile = fopen( m_szImageName, "rb" );
		if ( pFile != NULL )
		{
			fclose( pFile );

			m_hTexture = InterfaceLoadTexture( m_szImageName, 0 );
			if ( m_hTexture > 0 )
			{
				InterfaceGetTextureSize( m_hTexture, &m_ImageWidth, &m_ImageHeight );
				m_LoadState = BROWSER_IMG_STATE_LOADED;

				m_fVMod = 1.0f;
				m_fUMod = 1.0f;
			}
			else
			{
				m_LoadState = BROWSER_IMG_STATE_NOT_AVAILABLE;
			}
		}
	}
}

//--------------------------------------------------------------------------------------
// HTMLImage
//
//
HTMLImage::HTMLImage()
{
	m_pImageTarget = NULL;
	m_DrawWidth = 0;
	m_DrawHeight = 0;
}


void	HTMLImage::Display( void )
{
ELEMENTRECT	xBounds;
int		nOverlayHandle;

	if ( !m_pImageTarget ) return;

	if ( m_pImageTarget->m_LoadState == BROWSER_IMG_STATE_NONE )
	{
		m_pImageTarget->GetImageTexture();
	}

	if ( ( m_pImageTarget->m_LoadState == BROWSER_IMG_STATE_LOADED ) &&
		 ( m_pImageTarget->m_ImageWidth > 0 ) &&
		 ( m_pImageTarget->m_ImageHeight > 0 ) )
	{
	int	nDisplayedHeight;
	int	nDisplayedWidth;
	float	fUEnd = 1.0f;
	float	fVEnd = 1.0f;
	int	nXOffset = 0;

		if ( m_DrawWidth == 0 )
		{
			nDisplayedWidth = (int)( m_pImageTarget->m_ImageWidth*m_pImageTarget->m_fUMod );
		}
		else
		{
			nDisplayedWidth = m_DrawWidth;
		}
	
		if ( m_DrawHeight == 0 )
		{
			nDisplayedHeight = (int)( m_pImageTarget->m_ImageHeight*m_pImageTarget->m_fVMod );
		}
		else
		{
			nDisplayedHeight = m_DrawHeight;
		}
		GetCurrentBounds( &xBounds );
		// Dont bother to draw if the bounds are off screen already
		if ( ( xBounds.width > 0 ) &&
			 ( xBounds.height > 0 ) )
		{
			if ( nDisplayedWidth > xBounds.width )
			{
				fUEnd = ((xBounds.width) / (float)(nDisplayedWidth) );
				nDisplayedWidth = xBounds.width;
			}
			if ( nDisplayedHeight > xBounds.height )
			{
				fVEnd = ((xBounds.height) / (float)( nDisplayedHeight ) );
				nDisplayedHeight = xBounds.height;
			}
			xBounds.y -= NoddyBrowser::Get().GetScrollTop();
			if ( xBounds.y >= NoddyBrowser::Get().GetDisplayTop() )
			{
				if ( m_Flags & ELEMENT_FLAG_CENTERED )
				{
					nXOffset = ((xBounds.width - nDisplayedWidth) / 2 );
					xBounds.x += nXOffset;
				}
				// TODO - Shouldnt create a new overlay each time - will need to track
				// when one is created for this image each frame and reuse it if poss
				nOverlayHandle = InterfaceCreateNewTexturedOverlay( 2, m_pImageTarget->m_hTexture );
				if ( ( gwMouseX >= xBounds.x ) &&
					 ( gwMouseY >= xBounds.y ) &&
					 ( gwMouseX < xBounds.x + nDisplayedWidth ) &&
					 ( gwMouseY < xBounds.y + nDisplayedHeight ) &&
					 ( m_szLink != 0 ) )
				{
					DoSetMouseOverCursor( TRUE );
					BOUNDS	xActiveBounds;
					xActiveBounds.nMinX = xBounds.x;
					xActiveBounds.nMinY = xBounds.y;
					xActiveBounds.nMaxX = xBounds.x + nDisplayedWidth;
					xActiveBounds.nMaxY = xBounds.y + nDisplayedHeight;

					NoddyBrowser::Get().SetActiveLink( m_szLink, &xActiveBounds );
					InterfaceTexturedRect( nOverlayHandle, xBounds.x, xBounds.y, nDisplayedWidth, nDisplayedHeight, 0xFFFFFFFF, 0.0f, 2.0f/m_pImageTarget->m_ImageHeight, fUEnd, fVEnd - (1.0f/m_pImageTarget->m_ImageHeight) );
				}
				else
				{
					if ( m_szLink != 0 )
					{
						InterfaceTexturedRect( nOverlayHandle, xBounds.x, xBounds.y, nDisplayedWidth, nDisplayedHeight, 0xD0FFFFFF, 0.0f, 2.0f/m_pImageTarget->m_ImageHeight, fUEnd, fVEnd - (1.0f/m_pImageTarget->m_ImageHeight) );
					}
					else
					{
						InterfaceTexturedRect( nOverlayHandle, xBounds.x, xBounds.y, nDisplayedWidth, nDisplayedHeight, 0xFFFFFFFF, 0.0f, 2.0f/m_pImageTarget->m_ImageHeight, fUEnd, fVEnd - (1.0f/m_pImageTarget->m_ImageHeight) );
					}
				}
			}
			if ( m_xUsedSize.width < nDisplayedWidth + nXOffset )
			{
				m_xUsedSize.width = nDisplayedWidth + nXOffset;
			}
			if ( m_xUsedSize.height < nDisplayedHeight )
			{
				m_xUsedSize.height = nDisplayedHeight;
			}
		}
	}

	if ( m_pParentElement )
	{
		m_pParentElement->ChildOccupied( 0, 0, m_xUsedSize.width, m_xUsedSize.height );
	}
}

void	HTMLImage::CalcSize( void )
{
	if ( m_pImageTarget )
	{
		m_xUnrestrictedSize.width = m_pImageTarget->m_ImageWidth;
		m_xUnrestrictedSize.height = m_pImageTarget->m_ImageHeight;
	}
	else
	{
		m_xUnrestrictedSize.width = 0;
		m_xUnrestrictedSize.height = 0;
	}
}

char*	HTMLImage::ParseTag( char* pRunner )
{
char*	pcNextProp;
char*	pcValue;
char*	pParam;
char*	pcVal;
char*	pcCheck;
char*	pcPercent;
char	acFullFilename[256];
int		nDrawWidth = 0;
int		nDrawHeight = 0;
int		nPercentage;
float	fPageWidth;


	acFullFilename[0] = 0;
	pcPercent="%";
	pcNextProp = pRunner;
	do
	{
		pcNextProp = HTMLParser::Get().GetNextPropAndVal( pcNextProp, &pParam, &pcValue );
		// If we've got a real value
		if ( pParam && pcValue )
		{
			if ( tinstricmp( pParam, "src" ) == 0 )
			{
				if ( strnicmp( pcValue, "http", 4 ) == 0 )
				{
					strcpy(acFullFilename, pcValue );
				}
				else
				{
					sprintf( acFullFilename, "%s//%s", NoddyBrowser::Get().GetRoot(), pcValue );
				}
			}
			else if ( tinstricmp( pParam, "align" ) == 0 )
			{
				if (tinstricmp( pcValue, "center" ) == 0 )
				{
					m_Flags |= ELEMENT_FLAG_CENTERED;			
				}
			}
			else if ( tinstricmp( pParam, "height" ) == 0 )
			{
				nDrawHeight = strtol( pcValue,NULL,10 );
			}
			else if ( tinstricmp( pParam, "width" ) == 0 )
			{
				//test if the width tag is a % rather than a value

				pcCheck = (char *) memchr (pcValue, '%' , strlen (pcValue));
				if (pcCheck !=NULL)
				{
					//remove % from string
					pcVal = strtok(pcValue,pcPercent);
					
					nPercentage = atol( pcVal );
					fPageWidth = (float)( NoddyBrowser::Get().GetPageWidth() );

					m_DrawWidth = (int)( fPageWidth / 100 * nPercentage );

				}
				//not a % so treat as a value
				else
				{
					m_DrawWidth =  strtol( pcValue,NULL,10);
				}
				
			}
		}
	} while ( *pcNextProp != 0 );

	if ( acFullFilename[0] != 0 )
	{
		m_pImageTarget = m_sImageFactorySingleton.FindImage( acFullFilename );
	}
	return( pRunner );
}
