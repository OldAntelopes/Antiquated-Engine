
#include <stdio.h>

#include <StandardDef.h>
#include <System.h>
#include <Engine.h>

#include "ModelArchive.h"
#include "Scene/SceneMap.h"

//-------------------------------------------------------------------------------------------------


void	EngineModelArchiveFillHeader( MODEL_ARCHIVE_FILE_HEADER* pHeader, int nNumberOfFiles )
{
	pHeader->bArchiveHeaderByte = 0x40;
	pHeader->bArchiveHeaderByte2 = 0x41;
	pHeader->bArchiveHeaderSize = sizeof( MODEL_ARCHIVE_FILE_HEADER );
	pHeader->bArchiveVersionNum = 1;

	pHeader->nNumberOfFilesInArchive = nNumberOfFiles;

}

//--------------------------------------------------------


void	ModelArchive::Render( const VECT* pxPos, const VECT* pxRot, uint32 ulRenderFlags )
{
	if ( mpMap )
	{
#ifndef TOOL
		mpMap->Update( EngineGetGameInterface()->GetPlayerPosition() );
#else
		mpMap->Update(NULL );
#endif
		mpMap->Display( pxPos, pxRot, ulRenderFlags );
	}
}

void	ModelArchive::Load( const char* szFilename )
{
MODEL_ARCHIVE_FILE_HEADER*		pHeader;
void*	pMem;
FILE*		pFile;
int		nFileSize;
int*	pnFileOffset;
int*	pnFileLen;
BYTE*	pbFilenameLen;
//char*	pcFilename;
char	acString[256];
char	acOutFile[256];
BYTE*	pbRunner;
char*	pcFileData;
char*	pcFileDataStart;
FILE*	pOutFile;

	pFile =	fopen( szFilename, "rb" );
	if ( pFile )
	{
		nFileSize = SysGetFileSize( pFile );
		pMem = malloc( nFileSize );
		if ( pMem )
		{
			fread( pMem, nFileSize, 1, pFile );
			fclose( pFile );
		
			pHeader = (MODEL_ARCHIVE_FILE_HEADER*)( pMem );
			if ( ( pHeader->bArchiveHeaderByte == 0x40 ) &&
				 ( pHeader->bArchiveHeaderByte2 == 0x41 ) )
			{
			int		nNumberOfFilesInArchive = pHeader->nNumberOfFilesInArchive;
			int		nLoop;

				pbRunner = (BYTE*)( pHeader );
				pbRunner += pHeader->bArchiveHeaderSize;

				for( nLoop = 0; nLoop < nNumberOfFilesInArchive; nLoop++ )
				{
					pnFileOffset = (int*)( pbRunner );
					pnFileLen = pnFileOffset+1;
					pbRunner += 8;
					pbFilenameLen = pbRunner;
					pbRunner += 1;
					pbRunner += *(pbFilenameLen);
				}
				pcFileDataStart = (char*)pbRunner;

				pbRunner = (BYTE*)( pHeader );
				pbRunner += pHeader->bArchiveHeaderSize;

				for( nLoop = 0; nLoop < nNumberOfFilesInArchive; nLoop++ )
				{
					pnFileOffset = (int*)( pbRunner );
					pnFileLen = pnFileOffset+1;
					pbRunner += 8;
					pbFilenameLen = pbRunner;
					pbRunner += 1;
					memcpy( acString, pbRunner, *pbFilenameLen );
					pbRunner += *pbFilenameLen;
					acString[*pbFilenameLen] = 0;
					pcFileData = pcFileDataStart + *pnFileOffset;
					nFileSize = *pnFileLen;
					// Now we have the name, size and data pointer for a file in mem..

					// TODO - This should use SysGetWritableData
					sprintf( acOutFile, "Data/Temp/%s", acString );
					pOutFile = fopen( acOutFile, "wb" );
					if ( pOutFile )
					{
						fwrite( pcFileData, nFileSize,1, pOutFile );
						fclose( pOutFile );
					}
				}

				mpMap = new SceneMap;
				mpMap->Load( "Data/Temp/00001.uim" );
			}
		}

	}
}


ModelArchive::ModelArchive()
{


}

ModelArchive::~ModelArchive()
{


}

