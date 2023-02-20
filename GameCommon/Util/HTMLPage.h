#ifndef GAMECOMMON_UTIL_HTML_PAGE_H
#define GAMECOMMON_UTIL_HTML_PAGE_H

// ------------------------------------------------------------------------------
//  Util function for helping create a buffer containing multiple lines of HTML
//   
//  Usage : Call HTMLPageInitBuffer to start it off.  This will include a basic HTML header and set the BODY tags
//          Add HTML to the body using HTMLPageAdd
//			Once complete, Call HTMLPageFinish to add a footer, close tags etc, and get a const char* buffer containing the finalised html
//
//  (Call HTMLPageShutdown at some point later - e.g. app exit - to clean up buffer memory)
//------------------------------------------------------

extern void		HTMLPageInitBuffer( const char* szTitle );

extern void		HTMLPageAdd( const char* szText, ... );

extern const char*		HTMLPageFinish( int* pnPageLen );


// (TODO - Allow custom headers and footers to be set)



//---------------------------------------------
// Cleanup
extern void		HTMLPageShutdown( void );


#endif