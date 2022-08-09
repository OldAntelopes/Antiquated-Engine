#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include <CodeUtil.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	HTTPDOWNLOAD_INIT_ERROR = 0,
	HTTPDOWNLOAD_SUCCESS = 1,
	HTTPDOWNLOAD_PAGE_NOT_FOUND = 2,
	HTTPDOWNLOAD_FAILED = 3,

} HTTPDOWNLOAD_RETURN_VAL;

typedef void (*HTTPDownloadCompleteCallback)( HTTPDOWNLOAD_RETURN_VAL, long, char* );
typedef BOOL (*HTTPDownloadUpdateCallback)( int, int, int, long, char* );

// Init function to prepare the download buffers etc
extern void HTTPDownloadsInit( void );

// HTTPDownloadsUpdate called every frame to update status of completed downloads
extern void	HTTPDownloadsUpdate( void );

// Returns file name and current status of any active downloads
extern char* HTTPDownloadsGetActiveDetails( int*, int*, int*);

// This method retrieves a file with a specific URL
extern void	HTTPDownloadInitiate( int nType, const char* szFullURL, const char* szLocalFile, fnFetchFileCallback,HTTPDownloadUpdateCallback, long lCBParam, BOOL bIgnoreCache );

// This method is used to retrieve a file using the standard URL paths defined by the server
extern void	HTTPDownloadGetFile( int nType, const char* szLocalFile, fnFetchFileCallback, long lCBParam );

// This method retrieves part of a file with a specific URL.
extern void	HTTPDownloadInitiatePartial( int nType, char* szFullURL, char* szLocalFile, fnFetchFileCallback,HTTPDownloadUpdateCallback, long lCBParam, int nBytesFromStart, int nBytesToRead );

extern void	HTTPDownloadSetContentPath( int nIndex, const char* szContentPathRoot );


extern void		HTTPGetCacheFilename( const char* szFullURL, char* szLocalFilenameOut );

#ifdef __cplusplus
}
#endif



#endif