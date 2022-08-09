#ifndef HTTP_H
#define HTTP_H

#ifdef __cplusplus
extern "C"
{
#endif

extern int HTTPGetFile( char *acFullURL, char* acLocalFileName, BOOL fnDownloadCallbackFunc(long, long, long), long lParam, BOOL bIgnoreCache  );

extern void		HTTPGetCacheFilename( const char* szFullURL, char* szLocalFilenameOut );

#ifdef __cplusplus
}
#endif


#endif //not defined HTTP_H