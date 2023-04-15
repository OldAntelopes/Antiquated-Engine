#include <stdio.h>
#include <string.h>

#include <StandardDef.h>
#include <Interface.h>
#include <CodeUtil.h>

#include "../../../Universal/GameCode/Globals/Globals.h"		// Temp needs fixing
#include "../../../Universal/LibCode/Networking/HTTP/HttpDownload.h"		// Temp needs fixing

#include "NoddyBrowser.h"
#include "HTMLParser.h"
#include "HTMLElement.h"
#include "HTMLImage.h"

#define		MAX_IMAGES_PER_PAGE		64

//--------------------------------------------------------------------------

NoddyBrowser::NoddyBrowser()
{
	m_pcLoadedPage = NULL;
	m_PageLoadState = 0;
	m_szPageRoot[0] = 0;
	m_szPageName[0] = 0;
	m_szLastReadFile[0] = 0;
	m_fnBrowserPageChangeCallback = NULL;
	m_PageScrollY = 0;
	m_PageDisplayY = 0;
	m_PageHeight = 2000;
	m_PageDisplayWidth = 0;
	m_PageDisplayHeight = 500;
	m_bCacheValid = FALSE;
	m_szCachedRoot[0] = 0;
	m_nPageDownloadRetVal = -1;
	m_PageBounds.nMinX = 0;
	m_PageBounds.nMaxX = 0;
	m_PageBounds.nMinY = 0;
	m_PageBounds.nMaxY = 0;
}


NoddyBrowser& NoddyBrowser::Get()
{
	static NoddyBrowser		singleton;

	return singleton;
}

void	NoddyBrowser::SetActiveLink( char* szLink, BOUNDS* pBounds )
{
	if ( ( szLink ) &&
		 ( pBounds ) )
	{
		strcpy( m_szActiveLink, szLink );
		SplitStringSep( m_szActiveLink, '\"' );
		m_ActiveLinkBounds = *pBounds;
	}
	else
	{
		m_szActiveLink[0] = 0;
	}
}

BOOL	NoddyBrowser::LeftMouseUp( int X, int Y )
{
	if ( m_szActiveLink[0] != 0 )
	{
		if ( ( X >= m_ActiveLinkBounds.nMinX ) &&
			 ( X <= m_ActiveLinkBounds.nMaxX ) &&
			 ( Y >= m_ActiveLinkBounds.nMinY ) &&
			 ( Y <= m_ActiveLinkBounds.nMaxY ) )
		{
			if ( m_fnBrowserPageChangeCallback != NULL )
			{
				m_fnBrowserPageChangeCallback( m_szActiveLink );
			}
			LoadPage( m_szActiveLink, FALSE );
			return( TRUE );
		}
	}
	m_ScrollbarDown = 0;
	return( FALSE );
}

BOOL	NoddyBrowser::OnMouseWheelMove( float fOffset )
{
	if ( m_pcLoadedPage )
	{
		if ( ( gwMouseX >= m_PageBounds.nMinX ) &&
			 ( gwMouseX <= m_PageBounds.nMaxX ) &&
			 ( gwMouseY >= m_PageBounds.nMinY ) &&
			 ( gwMouseY <= m_PageBounds.nMaxY ) )
		{
			if ( fOffset > 0.0f )
			{
				m_PageScrollY -= 18;
				if ( m_PageScrollY < 0 ) m_PageScrollY = 0;
			}
			else
			{
				m_PageScrollY += 18;
				if ( m_PageScrollY > m_nScrollMaxY ) m_PageScrollY = m_nScrollMaxY;
			}	
			return( TRUE );
		}
	}
	return( FALSE );
}


BOOL		NoddyBrowserOnMouseWheelMove( float fOffset )
{	
	return(	NoddyBrowser::Get().OnMouseWheelMove( fOffset ) );
}


BOOL	NoddyBrowser::LeftMouseDown( int X, int Y )
{
	if ( m_ScrollbarHover > 0 )
	{
		m_ScrollbarDown = m_ScrollbarHover;
		m_ScrollbarDragOriginal = Y;
		m_ScrollbarOriginalY = m_PageScrollY;
		return( TRUE );
	}
	return( FALSE );
}



void HTMLPageDownloadCallback( FETCHFILE_RETURN_VAL nRet, long lParam, const char* szName )
{
	NoddyBrowser::Get().SetDownloadReturnVal( nRet );
	if ( nRet == HTTPDOWNLOAD_SUCCESS )
	{
	char	acBufferFile[256];
		
		sprintf( acBufferFile, "%s/Data/buffer.html", SysGetWritableDataFolderPath("The Universal" ) );
		// save buffer.html file as Data/temp.cac
		NoddyBrowser::Get().CacheFile( acBufferFile );
		// Load the buffer.html file
		NoddyBrowser::Get().LoadPage( acBufferFile, TRUE );
	}
}

void NoddyBrowser::CacheFile( char* szTempFilename )
{
char	acTemporaryFile[256];
	
	sprintf( acTemporaryFile, "%s/Data/Temp/webtemp.cac", SysGetWritableDataFolderPath("The Universal" ) );
	SysCopyFile( szTempFilename, acTemporaryFile, FALSE );
	m_bCacheValid = TRUE;
	strcpy( m_szCachedRoot, m_szPageRoot );
}

void NoddyBrowser::CleanupPage( void )
{
	HTMLParser::Get().Cleanup();
	HTMLImageTarget::FreeAllImages();

	SystemFree(m_pcLoadedPage);
	m_pcLoadedPage = NULL;
	SystemFree(m_pcElementBuffer);
	m_pcElementBuffer = NULL;
	InterfaceFontFree( 5 );
	InterfaceFontFree( 6 );
	m_nPageDownloadRetVal = -1;
}

void NoddyBrowser::Close( void )
{
	CleanupPage();
}

//--------------------------------------------------
// NoddyBrowser::LoadPage
//	Loads an html page into mem 
// 
//--------------------------------------------------
void NoddyBrowser::LoadPage( char* szFilename, BOOL bLocalFileOnly )
{
FILE*	pFile;
int		nFileSize;
char	acString[256];
char	acCacheFile[256];
int		Loop;
	
	sprintf( acCacheFile, "%s/Data/Temp/webtemp.cac", SysGetWritableDataFolderPath("The Universal" ) );

	if ( m_PageLoadState != 0 )
	{
		CleanupPage();
	}

	if ( !bLocalFileOnly )
	{
		// If is the same file as the last one we loaded
		if ( ( stricmp( m_szLastReadFile, szFilename ) == 0 ) &&
			 ( m_bCacheValid == TRUE ) )
		{
			// Load the cached version from disk
			szFilename = acCacheFile;
			strcpy( m_szPageRoot, m_szCachedRoot );
			bLocalFileOnly = TRUE;
		}
		else
		{
			m_bCacheValid = FALSE;
		}
	}
	m_PageLoadState = 0;

	// TODO - If first part of name is http: download it
	if ( strnicmp( szFilename, "http", 4 ) == 0 )
	{
		m_PageLoadState = 1;
		strcpy( m_szLastReadFile, szFilename );
		HTTPDownloadInitiate( 0, szFilename, "Data/buffer.html", HTMLPageDownloadCallback,NULL, (long)( 0 ), TRUE );		
		strcpy( acString, szFilename );
		Loop = strlen( acString );
		while ( ( Loop > 0 ) &&
				( acString[Loop] != '\\' ) &&
				( acString[Loop] != '/' ) )
		{
			Loop--;
		}
		// if its not longer than 6 then all we got was http://url  
		if ( Loop > 6 )
		{
			acString[Loop] = 0;
		}
		strcpy( m_szPageRoot, acString );
	}
	else
	{
		sprintf( acString, "%s/%s", m_szPageRoot, szFilename );
		if ( ( bLocalFileOnly == FALSE ) &&
			 ( strnicmp( m_szPageRoot, "http", 4 ) == 0 ) )
		{
			m_PageLoadState = 1;
			strcpy( m_szLastReadFile, acString );
			HTTPDownloadInitiate( 0, acString, "Data/buffer.html", HTMLPageDownloadCallback,NULL, (long)( 0 ), TRUE );
		}
		else
		{
			pFile = fopen( szFilename, "rb" );
			if ( pFile )
			{
				fseek( pFile, 0, SEEK_END );
				nFileSize = ftell(pFile);
				rewind( pFile );

				m_pcLoadedPage = (char*)SystemMalloc( nFileSize + 1 );
				m_pcElementBuffer = (char*)SystemMalloc( nFileSize * 2 );
			
				fread( m_pcLoadedPage, nFileSize, 1, pFile );
				// Make sure its null terminated
				m_pcLoadedPage[ nFileSize ] = 0;
				m_pcElementBuffer[ (nFileSize*2)-1 ] = 0;
				fclose( pFile );	
				m_PageLoadState = 2;
				m_PageScrollY = 0;
				InterfaceFontLoad( 5, "Data/Fonts/arial000.bmp", "Data/Fonts/arial.txt", 0 );
				InterfaceFontLoad( 6, "Data/Fonts/verdana000.bmp", "Data/Fonts/verdana.txt", 0 );
			}
			else
			{
				m_PageLoadState = 3;
			}
		}
	}
}

BOOL	NoddyBrowser::IsDownloading( void )
{
	if ( m_PageLoadState == 1 )
	{
		return( TRUE );
	}
	return( FALSE );
}


void	NoddyBrowser::LoadPageFromMem( char* pbMem, int nMemSize )
{
	if ( m_PageLoadState != 0 )
	{
		CleanupPage();
	}
	m_PageLoadState = 0;

	m_pcLoadedPage = (char*)SystemMalloc( nMemSize + 1 );
	m_pcElementBuffer = (char*)SystemMalloc( nMemSize * 2 );
			
	memcpy( m_pcLoadedPage, pbMem, nMemSize );
	// Make sure its null terminated
	m_pcLoadedPage[ nMemSize ] = 0;
	m_pcElementBuffer[ (nMemSize*2)-1 ] = 0;
	m_PageLoadState = 2;
	m_PageScrollY = 0;
	InterfaceFontLoad( 5, "Data/Fonts/arial000.bmp", "Data/Fonts/arial.txt", 0 );
	InterfaceFontLoad( 6, "Data/Fonts/verdana000.bmp", "Data/Fonts/verdana.txt", 0 );
}

//---------------------------------------------------------------
//NoddyBrowser::GetNextElement
// params : pRunner [IN] - Next bit of the html doc to read thru   
//			pBuffer [OUT] - Pointer to destination where the next element is stored
//			pbTag [OUT] - Bool Pointer set to TRUE if the element we read was a tag
// returns : Position of runner following this element
//---------------------------------------------------------------
char*	NoddyBrowser::GetNextElement( char* pRunner, char*	pcBuffer, BOOL* pbTag )
{
BOOL	bLastSpace = FALSE;
	// ignore newlines and tabs
	while ( ( *pRunner == '\r' ) ||
		    ( *pRunner == '\n' ) ||
			( *pRunner == '\t' ) )
	{
		pRunner++;
	}

	// check for comment areas and ignore
	if ( ( pRunner[0] == '<' ) &&
		 ( pRunner[1] == '!' ) &&
		 ( pRunner[2] == '-' ) )
	{
		while ( *pRunner != 0 )
		{
			if ( ( *pRunner == '>' ) &&
				 ( pRunner[-1] == '-' ) &&
				 ( pRunner[-2] == '-' ) )
			{
				break;
			}
			pRunner++;
		}
		pRunner++;
	}

	if ( ( pRunner[0] == '&' ) &&
		 ( pRunner[1] == 'l' ) &&
		 ( pRunner[2] == 't' ) &&
		 ( pRunner[3] == ';' ) )
	{
		pRunner[0] = '\r';
		pRunner[1] = '\r';
		pRunner[2] = '\r';
		pRunner[3] = '<';
		pRunner += 3;
	}

	// We're about to read a html tag within < > braces..
	// we should return the stuff inside these braces
	if ( *pRunner == '<' )
	{
		*pbTag = TRUE;
		pRunner++;

		while ( ( *pRunner != 0 ) &&
				( *pRunner != '>' ) )
		{
			if ( ( *pRunner == '\r' ) ||
				 ( *pRunner == '\n' ) )
			{
				pRunner++;
			}
			else
			{
				if ( *pRunner == '&' ) 
				{
					if ( strnicmp(&pRunner[1],"gt", 2) == 0 )
					{
						pRunner += 3;
						*pRunner = '>';
						break;
					}
				}
				*pcBuffer++ = *pRunner++;
			}
		}
		*pcBuffer = 0;
		if ( *pRunner == '>' )
		{
			pRunner++;
		}
	}
	else	// we're reading stuff outside <>, so return up to the next occurence of <
	{
		*pbTag = FALSE;
		while ( ( *pRunner != '<' ) &&
				( *pRunner != 0 ) )
		{
			// ignore these chars
			if ( ( *pRunner == '\r' ) ||
				 ( *pRunner == '\n' ) ||
				 ( *pRunner == '\t' ) )
			{
				pRunner++;
			}
			else
			{
				if ( *pRunner == '&' ) 
				{
					if ( strnicmp(&pRunner[1],"nbsp;", 5) == 0 )
					{
//						*pcBuffer++ = ' '; 
						pRunner += 6;
					}
					else if ( strnicmp(&pRunner[1],"amp;", 4) == 0 )
					{
						*pcBuffer++ = '&';
						pRunner += 5;
					}
					else if ( strnicmp(&pRunner[1],"lt", 2) == 0 )
					{
						pRunner += 3;
						*pRunner = '<';
						break;
					}
					else if ( strnicmp(&pRunner[1],"gt;", 3) == 0 )
					{
						*pcBuffer++ = '>';
						pRunner += 4;
					}
					else if ( strnicmp(&pRunner[1],"bull;", 5) == 0 )
					{
						*pcBuffer++ = '+';
						pRunner += 6;
					}
					else if ( strnicmp(&pRunner[1],"quot;", 5) == 0 )
					{
						*pcBuffer++ = '\"';
						pRunner += 6;
					}
					else if ( strnicmp(&pRunner[1],"raquo;", 6) == 0 )
					{
						*pcBuffer++ = '}';
						pRunner += 7;
					}
					else if (pRunner[1] == '#' )
					{
						while ( *pRunner != ';' )
						{
							pRunner++;
						}
						if ( *pRunner == ';' )
						{
							pRunner++;
						}
					}
					else if (pRunner[1] == ' ' )
					{
						// do nothin.. print the ampersand as normal
						 bLastSpace = FALSE;
						*pcBuffer++ = *pRunner++;
					}
					else
					{
						*pcBuffer++ = *pRunner++;
					}
				}
				else
				{
					if ( *pRunner == ' ' ) 
					{
						if ( bLastSpace == FALSE )
						{
							bLastSpace = TRUE;
							*pcBuffer++ = *pRunner++;
						}
						else
						{
							pRunner++;			
						}
					}
					else
					{
						bLastSpace = FALSE;
						*pcBuffer++ = *pRunner++;
					}
				}
			}
		}
		*pcBuffer = 0;	
	}
	return( pRunner );
}

//---------------------------------------------------------------
//NoddyBrowser::FindBody
//  Moves the runner along to the start of the <BODY tag
//---------------------------------------------------------------
char*	NoddyBrowser::FindBody( char* pRunner )
{
BOOL	bFound = FALSE;
char*	pcBase = pRunner;

	while ( ( *pRunner != 0 ) &&
			( bFound == FALSE ) )
	{
		while ( ( *pRunner != 0 ) &&
				( *pRunner != '<' ) )
		{
			pRunner++;
		}
		if ( *pRunner != 0 )
		{
			pRunner++;
			if ( strnicmp(pRunner,"body", 4 ) == 0 )
			{
				bFound = TRUE;
			}
		}
	}	 
	if ( bFound == TRUE )
	{
		return( pRunner - 1 ); 
	}
	return( pcBase );
}


//----------------------------------------------------------------------
// NoddyBrowser::DisplayScrollbar
//  Top level function called by the game to display the page we've loaded
//----------------------------------------------------------------------
bool	NoddyBrowser::DisplayScrollbar( int X, int Y, int Width, int Height, int GlobalAlpha )
{
char	acString[128];
int		nHeightOfBar;
int		nTopOfBar;

	if ( m_PageHeight > 0 )
	{
		m_fScrollPixelsPerLine = (float)(Height-22) / m_PageHeight;
	}
	else
	{
		m_fScrollPixelsPerLine = 0;
	}

	nHeightOfBar = (int)( Height * m_fScrollPixelsPerLine );
	if ( nHeightOfBar > (Height-22) )
	{
		nHeightOfBar = (Height-22);
		// Dont show scrollbar if the bar is full (i.e page is all on screen)
		m_nScrollMaxY = 0;
		return( false );
	}

	InterfaceRect( 0, X, Y, Width, Height, 0x60606060 );
	m_nScrollMaxY = m_PageHeight - Height;
	if ( m_nScrollMaxY < 0 ) m_nScrollMaxY = 0;
	/** Add up arrow **/
	acString[0] = (BYTE)( 132 );
	acString[1] = 0;
	if ( m_PageScrollY == 0 )
	{
		InterfaceTextCenter(2, X, X+Width, Y - 1, acString, COL_SYS_SPECIAL_FADED, 2 );
	}
	else
	{
		InterfaceTextCenter( 2, X, X+Width,Y - 1, acString, COL_WHITE, 2 );
	}

	/** Add down arrow **/
	acString[0] = (BYTE)( 134 );
	acString[1] = 0;

	if ( m_PageScrollY + Height < m_PageHeight )
	{
		InterfaceTextCenter( 2, X, X+Width,  Y + Height - 11,acString, COL_WHITE, 2 );
	}
	else
	{
		InterfaceTextCenter( 2, X, X+Width, Y + Height - 11, acString, COL_SYS_SPECIAL_FADED, 2 );
	}

	nTopOfBar = (int)( m_PageScrollY * m_fScrollPixelsPerLine );

	InterfaceRect( 1, X + 1, Y + 11 + nTopOfBar, Width - 2, nHeightOfBar, 0x80D0D0D0 );
	InterfaceRect( 1,X + 1, Y + 11 + nTopOfBar, 1, nHeightOfBar, 0xA0F0F0F0 );
	InterfaceRect( 1,X + 1, Y + 11 + nTopOfBar, Width - 2, 1, 0xA0F0F0F0 );
	InterfaceRect( 1,X + Width-1, Y + 11 + nTopOfBar, 1, nHeightOfBar, 0xA0404040 );
	InterfaceRect( 1,X + 1, Y + 11 + nTopOfBar + nHeightOfBar, Width - 2, 1, 0xA0404040 );

	if ( ( gwMouseX > X ) &&
		 ( gwMouseX < X + Width ) )
	{
		if ( ( gwMouseY > (Y ) ) &&
			 ( gwMouseY < (Y + Height) ) )
		{
			DoSetMouseOverCursor( TRUE );
			if ( gwMouseY < (Y + 12) )
			{
				m_ScrollbarHover = 1;
			}
			else if ( gwMouseY > (Y + Height - 12) )
			{
				m_ScrollbarHover = 2;
			}
			else
			{
				m_ScrollbarHover = 3;
			}		
		}
	}
	else 
	{
		m_ScrollbarHover = 0;
	}
	return( true );
}


//----------------------------------------------------------------------
// NoddyBrowser::DisplayPage
//  Top level function called by the game to display the page we've loaded
//----------------------------------------------------------------------
void	NoddyBrowser::DisplayPage( int X, int Y, int Width, int Height, int GlobalAlpha, int nFlags )
{
char*	pRunner = m_pcLoadedPage;
ulong	ulPageCol = (ulong)( GlobalAlpha );
int		nScrollbarWidth = 15;
int		nScrollbarWidthUsed = 0;

	m_PageDisplayHeight = Height - 8;
	m_PageDisplayWidth	= Width - 29;

	if ( nFlags == 0 )
	{
		// Temp - page col should come from body..
		ulPageCol <<= 24;
		ulGlobalAlpha = ulPageCol;
		ulPageCol |= 0xFFFFFF;
		InterfaceRect( 0, X, Y, Width, Height, 0x60000000 );//ulPageCol );
	}

	m_PageDisplayY = Y + 2;

	m_PageBounds.nMinX = X + 5;
	m_PageBounds.nMaxX = X + m_PageDisplayWidth;
	m_PageBounds.nMinY = m_PageDisplayY;
	m_PageBounds.nMaxY = m_PageDisplayY + m_PageDisplayHeight;

	// Update scrollbar pos if the left mouse was downed over the scrolliebar
	if ( m_ScrollbarDown > 0 )
	{
		switch( m_ScrollbarDown )
		{
		case 1:	// up arrow
			m_PageScrollY--;
			if ( m_PageScrollY < 0 )
			{
				m_PageScrollY = 0;
			}
			break;
		case 2:	// down arrow
			m_PageScrollY++;
			if ( m_PageScrollY > m_nScrollMaxY )
			{
				m_PageScrollY = m_nScrollMaxY;
			}
			break;
		case 3:	// drag bar
			int		nScrollBarMoveY = gwMouseY - m_ScrollbarDragOriginal;
			if ( m_fScrollPixelsPerLine > 0.0f )
			{
				m_PageScrollY = m_ScrollbarOriginalY + (int)(nScrollBarMoveY / m_fScrollPixelsPerLine );
				if ( m_PageScrollY < 0 )
				{
					m_PageScrollY = 0;
				}
				else if ( m_PageScrollY > m_nScrollMaxY )
				{
					m_PageScrollY = m_nScrollMaxY;
				}
			}
			break;
		}
	}

	if ( DisplayScrollbar( X+Width-nScrollbarWidth, Y, nScrollbarWidth, Height, GlobalAlpha ) )
	{
		nScrollbarWidthUsed = nScrollbarWidth;
	}

	if ( m_PageLoadState == 2 )
	{
		if ( pRunner )
		{
			pRunner = FindBody( pRunner );
			if ( pRunner )
			{
//				pRunner = GetNextElement( pRunner, GetElementBuffer(), &bIsTag );
				HTMLParser::Get().ParseDocument( pRunner );
				HTMLParser::Get().DisplayDocument( X + 5, Y+5, Width - (10 + nScrollbarWidthUsed), Height-8 );
			}
		}
	}
	else
	{
	char	acString[512];
		switch( m_nPageDownloadRetVal )
		{
		case -1:		// In progress, dont do anything..
			break;
		case HTTPDOWNLOAD_INIT_ERROR:
			InterfaceSetFontFlags(FONT_FLAG_LARGE|FONT_FLAG_DROP_SHADOW );
			InterfaceText(2, X+5, Y+10,"HTTP Init error", COL_WARNING, 0 );
			sprintf( acString, "URL: %s", m_szLastReadFile );
			InterfaceText(2, X+5, Y+30, acString, COL_SYS_SPECIAL_FADED, 0 );
			InterfaceSetFontFlags(0 );
			break;
		case HTTPDOWNLOAD_SUCCESS:
			InterfaceSetFontFlags(FONT_FLAG_LARGE|FONT_FLAG_DROP_SHADOW );
			InterfaceText(2, X+5, Y+10,"Browser Page Read Error", COL_WARNING, 0 );
			sprintf( acString, "URL: %s", m_szLastReadFile );
			InterfaceText(2, X+5, Y+30, acString, COL_SYS_SPECIAL_FADED, 0 );
			InterfaceSetFontFlags(0);
			break;
		case HTTPDOWNLOAD_PAGE_NOT_FOUND:
			InterfaceSetFontFlags(FONT_FLAG_LARGE|FONT_FLAG_DROP_SHADOW );
			InterfaceText(2, X+5, Y+10,"404 - Page not found", COL_WARNING, 0 );
			sprintf( acString, "URL: %s", m_szLastReadFile );
			InterfaceText(2, X+5, Y+30, acString, COL_SYS_SPECIAL_FADED, 0 );
			InterfaceSetFontFlags(0 );
			break;
		case HTTPDOWNLOAD_FAILED:
			InterfaceSetFontFlags(FONT_FLAG_LARGE|FONT_FLAG_DROP_SHADOW );
			InterfaceText(2, X+5, Y+10,"HTTP Download error", COL_WARNING, 0 );
			sprintf( acString, "URL: %s", m_szLastReadFile );
			InterfaceText(2, X+5, Y+30, acString, COL_SYS_SPECIAL_FADED, 0 );
			InterfaceSetFontFlags(0 );
			break;
		}
	}
}



BOOL	NoddyBrowser::IsMouseOver( BOUNDS* pBounds )
{
int		nMouseY = gwMouseY + GetScrollTop();

	if ( ( gwMouseX >= pBounds->nMinX ) &&
		 ( gwMouseX < pBounds->nMaxX ) &&
		 ( nMouseY >= pBounds->nMinY ) &&
		 ( nMouseY < pBounds->nMaxY ) )
	{
		return( TRUE );
	}
	return( FALSE );
}

