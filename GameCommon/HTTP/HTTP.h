#ifndef COMMON_HTTP_H
#define COMMON_HTTP_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef void		(*HTTPLoggingFunction)( const char* szTextOut );
typedef	void		(*HTTPResponseHandler)( int nResponseCode, unsigned char* szResponseBody, int nResponseLen, void* vpParam );


extern int		HTTPGet( const char *acFullURL, HTTPResponseHandler fnResponseHandler, void* vpParam );

extern int		HTTPPost( const char *acFullURL, BYTE* pbBody, int nBodyLen, HTTPResponseHandler fnResponseHandler, void* vpParam );
extern int		HTTPPostEx( const char *acFullURL, BYTE* pbBody, int nBodyLen, HTTPResponseHandler fnResponseHandler, void* vpParam, int nTimeoutSecs );


//-----------------------------------------------

extern void		HTTPInitialise( void );
extern void		HTTPSetVerboseLoggingFunction( HTTPLoggingFunction fnLogOutputFunction );

//-  HTTPUpdate ---------- Must be called regularly to receive callbacks
extern void		HTTPUpdate( float fDelta );



extern void		HTTPShutdown( void );


//--------------------------------------------------

#ifdef __cplusplus
}
#endif


#endif