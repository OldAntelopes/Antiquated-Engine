#include <curl/curl.h>

#include <StandardDef.h>
#include <interface.h>
#include <System.h>


#include "HTTP.h"

BOOL (*mfnDownloadCallbackFunc)(long, long, long);


int CurlProgressCallback(void *clientp, double dltotal,
                                      double dlnow,
                                      double ultotal,
                                      double ulnow)
{
	if ( mfnDownloadCallbackFunc( (long)dlnow, (long)dltotal , (long)(clientp) ) == TRUE )
	{
		return(0);
	}
	// if progress callback returns false, abort the download
	return(1);
}


typedef enum 
{
	kNotInCache = 0,
	kInCacheAndUTD, 
	kInCacheAndNeedsTagCheck,
} eCacheCheckResult;

typedef struct
{
	BYTE	bVersion;
	BYTE	bHeaderSize;
	BYTE	bPad1;
	BYTE	bPad2;
} INF_FILE_HEADER;

typedef struct
{
	long	lDownloadedLocalTime;
	long	lModServerTime;
	long	lCacheTag;
	long	lFileSize;

	long	lFileChecksum;
	long	lPad1;

	short	wURLLength;
	short	wPad1;

} INF_FILE_BODY;

eCacheCheckResult		CheckWebCacheInfFile( const char* szFullURL, const char* szLocalFilename, int* pnCachedFileSize )
{
char	szInfFilename[512];
FILE*	pFile;
eCacheCheckResult		nRet = kNotInCache;
char	acBodyBuff[1024];
INF_FILE_BODY*		pInfBody = (INF_FILE_BODY*)( acBodyBuff );
INF_FILE_HEADER		xInfHeader;
char*	pcInfURL = (char*)( pInfBody + 1 );
char	szLocalFilenameRepathed[256];

#ifdef SERVER
	sprintf( szLocalFilenameRepathed, "%s", szLocalFilename );
#else
	sprintf( szLocalFilenameRepathed, "%s\\%s", SysGetWritableDataFolderPath( NULL ), szLocalFilename );
#endif
	*pnCachedFileSize = 0;

	// Check to see if we have an inf file for the local file
	strcpy( szInfFilename, szLocalFilenameRepathed );
	strcat( szInfFilename, ".cfi" );

	pFile = fopen( szInfFilename, "rb" );
	// If a .inf file exists
	if ( pFile )
	{
		fread( &xInfHeader, sizeof( xInfHeader ), 1, pFile );

		if ( xInfHeader.bVersion == 0 )
		{
			fread( pInfBody, sizeof( INF_FILE_BODY ), 1, pFile );
			fread( pcInfURL, pInfBody->wURLLength, 1, pFile );
		}
		fclose( pFile );

		// Make sure the URLs match
		if ( _stricmp( pcInfURL, szFullURL ) == 0 )
		{
			pFile = fopen( szLocalFilenameRepathed, "rb" );
			// If the local file exists
			if ( pFile )
			{	
			int		nLocalFilesize = SysGetFileSize(pFile);
	
				fclose( pFile );

				if ( nLocalFilesize == pInfBody->lFileSize )
				{
				//   TODO if the file was downloaded in the last hour, just return it

					*pnCachedFileSize = nLocalFilesize;
					// TEMP - Anything with a matching inf is considered UTD
					return( kInCacheAndUTD );
				}
			}
		}


	}

	return( nRet );
}


// Note: Local filename is already repathed
void	WriteWebCacheInfFile( const char* szFullURL, const char* szLocalFilename )
{
char	szInfFilename[512];
FILE*	pFile;
char	acBodyBuff[1024];
INF_FILE_BODY*		pInfBody = (INF_FILE_BODY*)( acBodyBuff );
INF_FILE_HEADER		xInfHeader;
int		nLocalFileSize;
FILE*	pLocalFile;

	pLocalFile = fopen( szLocalFilename, "rb" );
	if ( pLocalFile )
	{
		nLocalFileSize = SysGetFileSize( pLocalFile );
		fclose( pLocalFile );

		memset( acBodyBuff, 0, 1024 );
		memset( &xInfHeader, 0, sizeof( xInfHeader ) );
		strcpy( szInfFilename, szLocalFilename );
		strcat( szInfFilename, ".cfi" );

		pFile = fopen( szInfFilename, "wb" );
		if ( pFile )
		{
			xInfHeader.bHeaderSize = sizeof( xInfHeader );

			pInfBody->lDownloadedLocalTime = SysGetTimeLong();
			pInfBody->lFileSize = nLocalFileSize;
			pInfBody->wURLLength = strlen( szFullURL ) + 1;

			fwrite( &xInfHeader, sizeof( xInfHeader ), 1, pFile );
			fwrite( pInfBody, sizeof(INF_FILE_BODY), 1, pFile );
			fwrite( szFullURL, pInfBody->wURLLength, 1, pFile );
			fclose( pFile );
		}
		else
		{
			PANIC_IF( TRUE, "Couldn't write cache inf file" );
		}
	}
}
/***************************************************************************
 * Function    : HTTPGetFile
 * Params      : Names should be self explanatory... Don't they ?
 * Returns     : 0 = ok, -1 = cannot open file, -2 = cannot init,
 *               everything above 0 is explained in curl.h
 * Description : It can actually download from http:// , ftp:// and prolly more...
 *               Isn't that nice ? :)
 *			(Function blocks until the transfer is complete)
 ***************************************************************************/
int HTTPGetFile( char *acFullURL, char* acLocalFileName, BOOL fnDownloadCallbackFunc(long, long, long), long lParam, BOOL bIgnoreCache )
{
CURL		*xCurl;
CURLcode	nRes;
FILE		*pFile ;
char		acErrorMsg[CURL_ERROR_SIZE];
double		dTotalBytes = 0;
double		dTransferedBytes = 0;
long		lRetCode = 0;
char	acTempFile[256];
char	acString[512];
int			nCachedFileSize;
eCacheCheckResult		nCacheFileRet;
	
	if ( !bIgnoreCache )
	{
		nCacheFileRet = CheckWebCacheInfFile( acFullURL, acLocalFileName, &nCachedFileSize );

		if ( nCacheFileRet == kInCacheAndUTD )
		{	
			// Call callback with filesize of local file
			fnDownloadCallbackFunc( (long)nCachedFileSize, (long)nCachedFileSize, lParam );
			return( CURLE_OK );
		}
	}

	// Store the download in a temporary '.prt' file - this is so that, if the client quits midway thru the download
	// we're not left with .atm's etc that are half-downloaded (which could cause the game to feck up irretrievably)
	// (this also makes it thread-safe, as the game thread can sometimes try to access the file mid-download..
	// particularly if the model is referenced in more than one place)
	sprintf( acTempFile, "%s\\%s", SysGetWritableDataFolderPath(NULL), acLocalFileName );
	strcat( acTempFile, ".prt");

	pFile = fopen( acTempFile, "wb" );

	if (! pFile)
	{
		sprintf( acString, "HTTP Download Error: Couldnt write to local file: %s", acTempFile);
#ifndef TOOL
//		PANIC_IF( TRUE, acString );
#endif
		return( CURLE_FAILED_INIT );
	}
//		sprintf( acString, "Opened file %s", acTempFile );
//		PrintConsoleCR(acString, COL_WARNING );

	/* This will init the winsock stuff if needed */
	curl_global_init( CURL_GLOBAL_ALL );

	/* get a curl handle */
	xCurl = curl_easy_init();
	if( ! xCurl)
	{
#ifndef TOOL
//		PANIC_IF( TRUE, "HTTP download : cannot initialise !" );
#endif
		return( CURLE_FAILED_INIT );
	}

	curl_easy_setopt( xCurl, CURLOPT_ERRORBUFFER, &acErrorMsg );
	curl_easy_setopt( xCurl, CURLOPT_NOPROGRESS, 0);
//	curl_easy_setopt( xCurl, CURLOPT_MUTE, 1);
	curl_easy_setopt( xCurl, CURLOPT_URL, acFullURL );
	curl_easy_setopt( xCurl, CURLOPT_WRITEFUNCTION, &fwrite );
	curl_easy_setopt( xCurl, CURLOPT_WRITEDATA, pFile );
	mfnDownloadCallbackFunc = fnDownloadCallbackFunc;
	if ( mfnDownloadCallbackFunc )
	{
		curl_easy_setopt( xCurl, CURLOPT_PROGRESSFUNCTION, CurlProgressCallback );
		curl_easy_setopt( xCurl, CURLOPT_PROGRESSDATA, lParam );
	}
	nRes = curl_easy_perform( xCurl );
	
	curl_easy_getinfo( xCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dTotalBytes );
	curl_easy_getinfo( xCurl, CURLINFO_SIZE_DOWNLOAD, &dTransferedBytes );
	curl_easy_getinfo( xCurl, CURLINFO_RESPONSE_CODE, &lRetCode );

	curl_easy_cleanup(xCurl);
	curl_global_cleanup();
  
	fclose(pFile);

	// if there was an error, remove the file we created
	if ( ( nRes ) ||
		 ( lRetCode < 200 ) ||
		 ( lRetCode >= 300 ) )
	{
		if ( remove( acTempFile ) == -1 )
		{
#ifndef TOOL
//			sprintf( acString, "Error removing file %s", acTempFile );
//			PrintConsoleCR(acString, COL_WARNING );
#endif
		}
	}
	else	// If successful, rename the temporary file to the real one
	{
	char	acLocalFileNameRepathed[256];
	
		sprintf(acLocalFileNameRepathed, "%s\\%s", SysGetWritableDataFolderPath(NULL), acLocalFileName );
		remove( acLocalFileNameRepathed );
		if ( rename( acTempFile, acLocalFileNameRepathed ) != 0 )
		{
			// If rename failed, it often because we were in the middle of streaming from the file that was being downloaded
			// so copy it instead. (It'll mean the .prt files are left around, but we can easily clean em all up lata)
			SysCopyFile( acTempFile, acLocalFileNameRepathed, TRUE );
		}
		else
		{
			remove( acTempFile );
		}
		// Write out the cache INF file
		WriteWebCacheInfFile( acFullURL, acLocalFileNameRepathed );
	}

	// Now call the download complete callback func
	if ( ( lRetCode == 404 ) ||
		 ( lRetCode == 400 ) )
		// TODO should handle other http response codes here
	{
//		sprintf( acString, "%s not found", acTempFile );
//		PrintConsoleCR(acString, COL_WARNING );
		nRes = CURLE_HTTP_NOT_FOUND;
	}
	else
	{
//		sprintf( acString, "%s callback", acTempFile );
//		PrintConsoleCR(acString, COL_WARNING );
		fnDownloadCallbackFunc( (long)dTransferedBytes, (long)dTotalBytes , lParam );
	}

	return( nRes );

}



void	HTTPGetCacheFilename( const char* szFullURL, char* szLocalFilenameOut )
{
char	acBuff[256];
char	acBuff2[512];
int		nPos = strlen( szFullURL );
int		nBuffPos = nPos;

	if ( nBuffPos > 64 )
	{
		nBuffPos = 64;
	}
	while ( nBuffPos >= 0 )
	{
		acBuff[nBuffPos] = szFullURL[nPos--];
		if ( ( acBuff[nBuffPos] == '/' ) ||
			 ( acBuff[nBuffPos] == '\\' ) ||
			 ( acBuff[nBuffPos] == ':' ) ||
			 ( acBuff[nBuffPos] == '?' ) ||
			 ( acBuff[nBuffPos] == ' ' ) ||
			 ( acBuff[nBuffPos] == '@' ) ||
			 ( acBuff[nBuffPos] == ';' ) )
		{
			acBuff[nBuffPos] = '_';
		}
		nBuffPos--;
	}

	sprintf( acBuff2, "Data\\Temp\\%s", acBuff );
	strcpy( szLocalFilenameOut, acBuff2 );
}