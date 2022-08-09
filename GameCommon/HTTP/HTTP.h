#ifndef COMMON_HTTP_H
#define COMMON_HTTP_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef	void		(*HTTPResponseHandler)( int nResponseCode, unsigned char* szResponseBody, int nResponseLen, void* vpParam );


extern int		HTTPGet( const char *acFullURL, HTTPResponseHandler fnResponseHandler, void* vpParam );

extern int		HTTPPost( const char *acFullURL, BYTE* pbBody, int nBodyLen, HTTPResponseHandler fnResponseHandler, void* vpParam );


void		HTTPInitialise( void );

//-  HTTPUpdate ---------- Must be called regularly to receive callbacks
void		HTTPUpdate( float fDelta );


void		HTTPShutdown( void );


//--------------------------------------------------

#ifdef __cplusplus
}
#endif


#endif