
#include <stdio.h>
#include <stdarg.h>

#include "StandardDef.h"

#define		HTML_PAGE_BUFFER_MAX_SIZE		8192

char*		mpHTMLPageBuffer = NULL;
int			mnHTMLPageCurrentPageSize = 0;

// TODO - Make a css file??
char*		mpcHTMLPageStandardHTMLHeader = "<HTML><HEAD><TITLE>%s</TITLE></HEAD><BODY>";
char*		mpcHTMLPageStandardHeader = "<CENTER><TABLE WIDTH=800><TR HEIGHT=100 BGCOLOR=#203070><TD ALIGN=CENTER><FONT FACE=ARIAL SIZE=6 COLOR=#FFFFFF><b>%s</b></font></TD></TR><TR HEIGHT=20><TD></TD></TR><TR HEIGHT=100%><TD><font face=arial size=4>";
char*		mpcHTMLPageStandardFooter = "</font></td></tr></table>";
char*		mpcHTMLPageStandardHTMLFooter = "</body></html>";


void		HTMLPageInitBuffer( const char* szPageTitle )
{
char		acString[512];

	if ( mpHTMLPageBuffer == NULL )
	{
		mpHTMLPageBuffer = (char*)( malloc( HTML_PAGE_BUFFER_MAX_SIZE ) );
	}

	mnHTMLPageCurrentPageSize = 0;
	
	sprintf( acString, mpcHTMLPageStandardHTMLHeader, szPageTitle );
	strcpy( mpHTMLPageBuffer, acString );

	sprintf( acString, mpcHTMLPageStandardHeader, szPageTitle );
	strcat( mpHTMLPageBuffer, acString );

	mnHTMLPageCurrentPageSize = strlen( mpHTMLPageBuffer );
}

void		HTMLPageAdd( const char* text, ... )
{
char		acString[2048];
va_list		marker;
uint32*		pArgs;

	pArgs = (uint32*)( &text ) + 1;

	va_start( marker, text );     
	vsprintf( acString, text, marker );

	strcpy( mpHTMLPageBuffer + mnHTMLPageCurrentPageSize, acString );
	mnHTMLPageCurrentPageSize += strlen( acString );
}


const char*		HTMLPageFinish( int* pnPageLen )
{
	strcpy( mpHTMLPageBuffer + mnHTMLPageCurrentPageSize, mpcHTMLPageStandardFooter );
	mnHTMLPageCurrentPageSize += strlen( mpcHTMLPageStandardFooter );

	strcpy( mpHTMLPageBuffer + mnHTMLPageCurrentPageSize, mpcHTMLPageStandardHTMLFooter );
	mnHTMLPageCurrentPageSize += strlen( mpcHTMLPageStandardHTMLFooter );

	if ( pnPageLen )
	{
		*pnPageLen = mnHTMLPageCurrentPageSize;
	}
	return( mpHTMLPageBuffer );
}


void		HTMLPageShutdown( void )
{
	if ( mpHTMLPageBuffer )
	{
		free( mpHTMLPageBuffer );
		mpHTMLPageBuffer = NULL;
	}

}
