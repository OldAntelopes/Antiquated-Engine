
#include <stdio.h>
#include "StandardDef.h"

#include "RefLocalDB.h"

#define		REF_LOCAL_DB_NUM_FOLDERS	200

void		RefLocalDBInit( const char* szDBName )
{
char	acDirString[256];
int		nLoop;
void*	pFile;

	// See if we can find the DB..
	sprintf( acDirString, "DB\\%s\\%03d\\dbid.dat", szDBName, 0 );
	pFile = SysFileOpen( acDirString, "rb" );
	// Couldn't open dbid.dat
	if ( pFile == NULL )
	{
		SysUserPrint( 0, "Creating DB folder structure for %s..", szDBName );
		// Create the DB folder structure
		SysMkDir( "DB" );
		sprintf( acDirString, "DB\\%s", szDBName );
		SysMkDir( acDirString );

		for( nLoop = 0; nLoop < REF_LOCAL_DB_NUM_FOLDERS; nLoop++ )
		{
			sprintf( acDirString, "DB\\%s\\%03d", szDBName, nLoop );
			SysMkDir( acDirString );
		}

		sprintf( acDirString, "DB\\%s\\%03d\\dbid.dat", szDBName, 0 );
		pFile = SysFileOpen( acDirString, "wb" );
		// Couldn't open dbid.dat
		if ( pFile )
		{
		int			nDBID = 0;

			SysFileWrite( (BYTE*)&nDBID, 4, 1, pFile );
			SysFileClose( pFile );

			SysUserPrint( 0, "DB created: %s", szDBName );
		}
		else
		{
			SysPanicIf( TRUE, "Error creating DB" );
		}
	}
	else
	{
		SysFileClose( pFile );
		SysUserPrint( 0, "DB open: %s", szDBName );
	}

}

int		RefLocalDBRetrieve( const char* szDBName, u64 ulRecordUID, BYTE** ppbMemOut, int* pnMemSize )
{
void*	pFile;
char	acDBFilename[256];
int		nFolderNum = (int)( ulRecordUID % REF_LOCAL_DB_NUM_FOLDERS );
int		nFileSize;
BYTE*	pbMemBuffer;

	sprintf( acDBFilename, "DB\\%s\\%03d\\%lld.dbf", szDBName, nFolderNum, ulRecordUID );
	pFile = SysFileOpen( acDBFilename, "rb" );
	if ( pFile )
	{
		nFileSize = SysGetFileSize( pFile );
		pbMemBuffer = (BYTE*)( malloc( nFileSize + 1 ) );
		pbMemBuffer[nFileSize] = 0;
		SysFileRead( pbMemBuffer, nFileSize, 1, pFile );
		SysFileClose( pFile );

		*pnMemSize = nFileSize;
		*ppbMemOut = pbMemBuffer;
		return( 200 );
	}
	return( 404 );
}

int		RefLocalDBStore( const char* szDBName, u64 ulRecordUID, BYTE* pbMem, int nMemSize )
{
void*	pFile;
char	acTempDBFilename[256];
char	acDBFilename[256];
int		nFolderNum = (int)( ulRecordUID % REF_LOCAL_DB_NUM_FOLDERS );
int		nFileSize;

	sprintf( acDBFilename, "DB\\%s\\%03d\\%lld.dbf", szDBName, nFolderNum, ulRecordUID );
	sprintf( acTempDBFilename, "DB\\%s\\%03d\\%lld.dbnew", szDBName, nFolderNum, ulRecordUID );
	pFile = SysFileOpen( acTempDBFilename, "wb" );
	if ( pFile )
	{
		nFileSize = SysGetFileSize( pFile );
		SysFileWrite( pbMem, nMemSize, 1, pFile );
		SysFileClose( pFile );

		// Now delete old one and rename the new one
		SysDeleteFile( acDBFilename );
		SysRenameFile( acTempDBFilename, acDBFilename );
		return( 200 );
	}
	return( 404 );
}


void		RefLocalDBResetAll( const char* szDBName )
{
char	acDBFolder[256];
	// todo - delete entire folder structure
	sprintf( acDBFolder, "DB\\%s", szDBName );
	SysDeleteFolder( acDBFolder );

	RefLocalDBInit( szDBName );
}
