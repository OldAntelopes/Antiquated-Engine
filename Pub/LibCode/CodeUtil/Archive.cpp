
//#include "stdafx.h"

#include <StandardDef.h>
#include <stdio.h>
//#include <Interface.h>

#include "Archive.h"

//--------------------------------------------------------------------------------
BYTE	mabLightEncryptKey[37] = { 0xD0, 0x34, 0x76, 0xA8, 0x15, 0x31, 0x89, 0xC2,
								  0x75, 0x12, 0x22, 0x49, 0x33, 0xB2, 0x77, 0xA6,
								  0x49, 0x1D, 0x2B, 0x94, 0x9D, 0xB8, 0xE2, 0x9E,
								  0xF1, 0x68, 0x5A, 0xC1, 0xA3, 0x68, 0xE5, 0xB4,
								  0x28, 0x54, 0x60, 0x0A, 0xDA };

void	ApplyLightEncryption( byte* pbDataBuffer, int nDataSize, int nOffsetFromFileStart )
{
int		nLoop;
int		nKeyPoint = nOffsetFromFileStart % 37;
BYTE*	pbRunner = pbDataBuffer;

	for ( nLoop = 0; nLoop < nDataSize; nLoop++ )
	{
		*pbRunner = *pbRunner + mabLightEncryptKey[nKeyPoint];
		pbRunner++;
		nKeyPoint++;
		nKeyPoint %= 37;
	}
}

void	ApplyLightDecryption( byte* pbDataBuffer, int nDataSize, int nOffsetFromFileStart )
{
int		nLoop;
int		nKeyPoint = nOffsetFromFileStart % 37;
BYTE*	pbRunner = pbDataBuffer;

	for ( nLoop = 0; nLoop < nDataSize; nLoop++ )
	{
		*pbRunner = *pbRunner - mabLightEncryptKey[nKeyPoint];
		pbRunner++;
		nKeyPoint++;
		nKeyPoint %= 37;
	}
}



//--------------------------------------------------------------------------------
//
class ArchiveFile
{
public:
	ArchiveFile()
	{
		mpNext = NULL;
		mszFullPathFilename = NULL;
		mszFilename = NULL;
		mnFileSize = 0;
		mnFileOffset = 0;
		mnCompressedFileSize = 0;
	}

	~ArchiveFile()
	{
		if ( mszFullPathFilename )
		{
			delete [] mszFullPathFilename;
		}
		if ( mszFilename )
		{
			delete [] mszFilename;
		}
	}

	const char*		GetFilename( ) { return( mszFilename ); }
	int				GetFileSize() { return( mnFileSize ); }
	int				GetFileOffset() { return( mnFileOffset ); }

	const char*		GetFullFilename( ) { return( mszFullPathFilename ); }

	void			SetFileSize( int nSize ) { mnFileSize = nSize; }
	void			SetFileOffset( int offset ) { mnFileOffset = offset; }

	void			MakeConsistentSlashes( char* szText )
	{
	char*	pcRunner;

		pcRunner = szText;
	
		while( *pcRunner != 0 )
		{
			if ( *pcRunner == '/' )
			{
				*pcRunner = '\\';
			}
			pcRunner++;
		}
	}

	void			SetFilename( const char* szFilename, bool bIsFullPath )
	{
		if ( bIsFullPath )
		{
			mszFullPathFilename = new char [ strlen( szFilename ) + 1 ];
			strcpy( mszFullPathFilename, szFilename );
			MakeConsistentSlashes( mszFullPathFilename );
		}
		else
		{
			mszFilename = new char [ strlen( szFilename ) + 1 ];
			strcpy( mszFilename, szFilename );
			MakeConsistentSlashes( mszFilename );
		}
	}

	void		MakeArchiveLocalFilename( const char* szRootPath )
	{
		if ( mszFullPathFilename )
		{
		const char*		pcRunner1 = szRootPath;
		const char*		pcRunner2 = mszFullPathFilename;

			while ( *pcRunner1 != 0 )
			{
				if ( *pcRunner1 != *pcRunner2 )
				{
					break;
				}
				pcRunner1++;
				pcRunner2++;
			}
			// If matched
			if ( *pcRunner1 == 0 )
			{
				if ( *pcRunner2 == '\\' )
				{
					pcRunner2++;
				}
				mszFilename = new char [ strlen( pcRunner2 ) + 1 ];
				strcpy( mszFilename, pcRunner2 );		
			}
		}
	}

	void			SetNext( ArchiveFile*	pNext ) { mpNext = pNext; }
	ArchiveFile*	GetNext( void ) { return( mpNext ); }

private:
	ArchiveFile*		mpNext;
	char*				mszFilename;
	int					mnFileSize;
	int					mnFileOffset;
	int					mnCompressedFileSize;
	char*				mszFullPathFilename;
};


//--------------------------------------------------------------------------------
class ArchiveFileRef
{
public:
	ArchiveFileRef()
	{
		m_nArchiveFileHandle = NOTFOUND;
		m_pArchiveFileSystem = NULL;
		m_pArchiveFile = NULL;
		m_pNext = NULL;
	}
	~ArchiveFileRef()
	{

	}

	int		Init( ArchiveFile* pArchiveFile, const char* szArchivePath );
	int		GetFileSize( void );
	int		GetHandle( void ) { return( m_nArchiveFileHandle ); }

	int		ReadData( byte* pbMem, int nSize );
	int		Seek( int nOffset, int nMode );

	void	Close();

	ArchiveFileRef*		GetNext() { return( m_pNext ); }
	void				SetNext( ArchiveFileRef* pNext ) { m_pNext = pNext; }

private:
	int				m_nArchiveFileHandle;
	FILE*			m_pArchiveFileSystem;
	ArchiveFile*	m_pArchiveFile;

	int				mnArchiveReadPositionAbsolute;
	int				mnArchiveReadPositionFromFileStart;


	ArchiveFileRef*		m_pNext;
};


volatile int		msnNextArchiveFileHandleNum = 1;

void	ArchiveGetConvertedFilePath( const char* szFilename, char* szOut )
{
	// If we've been given a full, local path (e.g. c:\archive.tua )
	if ( szFilename[1] == ':' )
	{
		strcpy( szOut, szFilename );
	}
	else if ( szFilename[0] == '.' )
	{
		// Starting with ./ means we read from the client folder, not the writable/download path
		strcpy( szOut, szFilename + 2 );
	}
	else
	{
		sprintf( szOut, "%s/%s", SysGetWritableDataFolderPath( NULL ), szFilename );
	}
}



void	ArchiveFileRef::Close( void )
{
	if ( m_pArchiveFileSystem )
	{
		fclose( m_pArchiveFileSystem );
		m_pArchiveFileSystem = NULL;
		mnArchiveReadPositionAbsolute = 0;
		m_nArchiveFileHandle = 0;
	}
}

int		ArchiveFileRef::Init( ArchiveFile* pArchiveFile, const char* szArchivePath )
{
char	acArchiveFileRepathed[256];

	ArchiveGetConvertedFilePath( szArchivePath, acArchiveFileRepathed );
	// Open the archive on the native FS
	m_pArchiveFileSystem = fopen( acArchiveFileRepathed, "rb" );

	if ( m_pArchiveFileSystem )
	{
		fseek( m_pArchiveFileSystem, pArchiveFile->GetFileOffset(), 0 );
		mnArchiveReadPositionAbsolute = pArchiveFile->GetFileOffset();
		mnArchiveReadPositionFromFileStart = 0;
		m_pArchiveFile = pArchiveFile;
		m_nArchiveFileHandle = msnNextArchiveFileHandleNum++;

		return( m_nArchiveFileHandle );
	}
	return( -1 );
}

int		ArchiveFileRef::Seek( int nOffset, int nMode )
{
int		ret;
int		nFileStart = m_pArchiveFile->GetFileOffset();

	ret = fseek( m_pArchiveFileSystem, nFileStart + nOffset, SEEK_SET );
	mnArchiveReadPositionAbsolute = nFileStart + nOffset;
	mnArchiveReadPositionFromFileStart = nOffset;
	return( 0 );
}

int		ArchiveFileRef::ReadData( byte* pbMem, int nSize )
{
int		bytesRead = 0;

	bytesRead = fread( pbMem, 1, nSize, m_pArchiveFileSystem );
	ApplyLightDecryption( pbMem, nSize, mnArchiveReadPositionFromFileStart );
	mnArchiveReadPositionAbsolute += bytesRead;
	mnArchiveReadPositionFromFileStart += bytesRead;

	return( bytesRead );
}

int		ArchiveFileRef::GetFileSize( void )
{
	return( m_pArchiveFile->GetFileSize() );
}


//--------------------------------------------------------------------------------
// Archive file structures

typedef struct
{
	char	acHeader[4];
	char	bVersion;
	char	bHeaderSize;
	char	bTOCEntryHeaderSize;
	char	bPad3;						// First 8 bytes should not change
	uint32	ulNumFiles;
	ushort	uwEncryptionMode;
	ushort	uwPad1;
	
} ARCHIVE_HEADER_V1;


typedef struct
{
	uint32	ulFileOffset;
	uint32	ulFileSize;
	uint32	ulCompressedFileSize;
	byte	bTOCEntryFlags;
	byte	bFilenameLen;
	byte	bPad2;
	byte	bPad3;
	
} ARCHIVE_TOC_ENTRY_HEADER_V1;



//--------------------------------------------------------------------------------
//
//--------------------------------------------------------
Archive::Archive()
{
	mpFileList = NULL;
	mszCreateArchiveRootPath = NULL;
	mnNumFilesInList = 0;
	muwArchiveEncryptionMode = 1;
	mszLoadedArchiveFilename = NULL;
	
	m_pArchiveFileReferences = NULL;
}


void	Archive::ResetFileList()
{
ArchiveFile*		pFile = mpFileList;
ArchiveFile*		pNext;

	while ( pFile )
	{
		pNext = pFile->GetNext();
		delete( pFile );
		pFile = pNext;
	}
	mpFileList = NULL;
	mnNumFilesInList = 0;
}

Archive::~Archive()
{
	ResetFileList();

	if ( mszLoadedArchiveFilename )
	{
		free( mszLoadedArchiveFilename );
		mszLoadedArchiveFilename = NULL;
	}
}


ArchiveFileRef*	Archive::GetFileRef( int nHandle )
{
ArchiveFileRef*		pFileRef = m_pArchiveFileReferences;

	while( pFileRef != NULL )
	{
		if ( pFileRef->GetHandle() == nHandle )
		{
			return( pFileRef );
		}
		pFileRef = pFileRef->GetNext();
	}
	return( NULL );
}


//-------------------------------------------------------------- Accessing files within an archive



int		Archive::OpenFile( const char* szFilename )
{
ArchiveFileRef*		pFileRef;
int			nHandle;
ArchiveFile*		pArchiveFile;

	if ( mszLoadedArchiveFilename == NULL )
	{
		// Assert?
		return( -3 );
	}
	
	pArchiveFile = FindArchiveFile( szFilename );

	// If the filename exists within the archive
	if ( pArchiveFile )
	{
		pFileRef = new ArchiveFileRef;
		nHandle = pFileRef->Init( pArchiveFile, mszLoadedArchiveFilename );

		if ( nHandle > 0 )
		{
			pFileRef->SetNext( m_pArchiveFileReferences );
			m_pArchiveFileReferences = pFileRef;
			return( nHandle );
		}	
		else
		{
			delete pFileRef;
		}
	}
	return( -1 );
}

int		Archive::SeekFile( int fileHandle, int nOffset, int nMode )
{
int		bytesRead = -1;
ArchiveFileRef*		pFileRef = GetFileRef( fileHandle );

	if ( pFileRef )
	{
		pFileRef->Seek( nOffset, nMode );
	}
	return( 1 );
}

int		Archive::ReadFile( int fileHandle, byte* pbMem, int nSize )
{
int		bytesRead = -1;
ArchiveFileRef*		pFileRef = GetFileRef( fileHandle );

	if ( pFileRef )
	{
		bytesRead = pFileRef->ReadData( pbMem, nSize );
	}
	return( bytesRead );
}


int		Archive::CloseFile( int fileHandle )
{
ArchiveFileRef*		pFileRef = GetFileRef( fileHandle );
ArchiveFileRef*		pNextRef = m_pArchiveFileReferences;
ArchiveFileRef*		pLastRef = NULL;

	if ( pFileRef )
	{
		pFileRef->Close();

		while( pNextRef != NULL )
		{
			if ( pNextRef == pFileRef )
			{
				if ( pLastRef == NULL )
				{
					m_pArchiveFileReferences = pNextRef->GetNext();
				}
				else
				{
					pLastRef->SetNext( pNextRef->GetNext() );
				}
				delete pNextRef;
				return( 0 );
			}
			pLastRef = pNextRef;
			pNextRef = pNextRef->GetNext();
		}
		return( -2 );
	}
	return( -1 );
}

int		Archive::GetFileSize( int fileHandle )
{
int		filesize = -1;
ArchiveFileRef*		pFileRef = GetFileRef( fileHandle );

	if ( pFileRef )
	{
		filesize = pFileRef->GetFileSize();
	}
	return( filesize );
}


//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
int		Archive::MountArchive( const char* szFilename )
{
ArchiveFile*		pNewFileEntry;
char	acDLCFileRepathed[256];
FILE*	pFile;

	ArchiveGetConvertedFilePath( szFilename, acDLCFileRepathed );
	pFile = fopen( acDLCFileRepathed, "rb" );

	if ( pFile )
	{
	ARCHIVE_HEADER_V1		xHeaderStruct;

		mszLoadedArchiveFilename = (char*)malloc( strlen( szFilename ) + 1 );
		strcpy( mszLoadedArchiveFilename, szFilename );

		ResetFileList();
	
		fread( &xHeaderStruct, 8, 1, pFile );
		if ( xHeaderStruct.bHeaderSize == sizeof( ARCHIVE_HEADER_V1 ) )
		{
			// Read the rest of the header
			fread( ((byte*)&xHeaderStruct)+8, xHeaderStruct.bHeaderSize - 8, 1, pFile );
		}
		else
		{	
			// TODO - Cope with newer/larger/older/smaller headers

		}

		muwArchiveEncryptionMode = xHeaderStruct.uwEncryptionMode;

		// Now Read the TOCS and generate the ArchiveFile entry for each.
		if ( xHeaderStruct.bTOCEntryHeaderSize == sizeof( ARCHIVE_TOC_ENTRY_HEADER_V1 ) )
		{
		uint32		loop;
		ARCHIVE_TOC_ENTRY_HEADER_V1		xTOCEntry;
		char			pcFilenameBuffer[256];
		ArchiveFile*	pLast = NULL;
		int			nReadPosition = 0;

			for ( loop = 0; loop < xHeaderStruct.ulNumFiles; loop++ )
			{
				fread( &xTOCEntry, xHeaderStruct.bTOCEntryHeaderSize, 1, pFile );
				// 'Decrypt' the TOC if needed
				switch( muwArchiveEncryptionMode )
				{
				case 1:
					ApplyLightDecryption( (byte*)&xTOCEntry, sizeof(xTOCEntry), nReadPosition );
					break;
				case 0:
				default:
					break;
				}
				nReadPosition += sizeof( xTOCEntry );

				fread( pcFilenameBuffer, xTOCEntry.bFilenameLen, 1, pFile );
				// 'Decrypt' the TOC if needed
				switch( muwArchiveEncryptionMode )
				{
				case 1:
					ApplyLightDecryption( (byte*)pcFilenameBuffer, xTOCEntry.bFilenameLen, nReadPosition );
					break;
				case 0:
				default:
					break;
				}
				nReadPosition += xTOCEntry.bFilenameLen;

				pNewFileEntry = new ArchiveFile;
			
				if ( pLast )
				{
					pLast->SetNext( pNewFileEntry );
				}
				else
				{
					mpFileList = pNewFileEntry;
				}
				pNewFileEntry->SetNext( NULL );
				pLast = pNewFileEntry;
				mnNumFilesInList++;

				pNewFileEntry->SetFilename( pcFilenameBuffer, false );
				pNewFileEntry->SetFileSize( xTOCEntry.ulFileSize );
				pNewFileEntry->SetFileOffset( xTOCEntry.ulFileOffset );
			}
		}
		else
		{
			// TODO - Cope with newer/larger/older/smaller TOC entries
		}
		fclose( pFile );
		return( 1 );
	}
	return( -1 );
}



const char*		Archive::GetFileName( int index, int* pnFileSize, int* pnFileOffset )
{
	if ( mpFileList )
	{
	int		count = 0;
	ArchiveFile*		pFile = mpFileList;

		while( count < index )
		{
			pFile = pFile->GetNext();

			if ( !pFile )
			{
				return( NULL );
			}
			count++;
		}

		if ( pnFileSize )
		{
			*pnFileSize = pFile->GetFileSize();
		}
		if ( pnFileOffset )
		{
			*pnFileOffset = pFile->GetFileOffset();
		}
		return( pFile->GetFilename() );
	}
	return( NULL );
}



ArchiveFile*	Archive::FindArchiveFile( const char* szFilename )
{
ArchiveFile*	pFileList = mpFileList;
char		acFilename[256];

	strncpy( acFilename, szFilename, 255 );
	acFilename[255] = 0;
	pFileList->MakeConsistentSlashes( acFilename );

	while( pFileList )
	{
		// If its a newly added file
		if ( tinstricmp( pFileList->GetFilename(), acFilename ) == 0 )
		{
			return( pFileList );
		}
		pFileList = pFileList->GetNext();
	}

	return( NULL );
}


// For tools/construction of archives etc
void	Archive::MakeConsistentSlashes( char* szText )
{
char*	pcRunner;

	pcRunner = szText;
	while( *pcRunner != 0 )
	{
		if ( *pcRunner == '/' )
		{
			*pcRunner = '\\';
		}
		pcRunner++;
	}
}

void	Archive::SetRootPath( const char* szPath )
{
	if ( mszCreateArchiveRootPath )
	{
		delete [] mszCreateArchiveRootPath;
	}

	mszCreateArchiveRootPath = new char [ strlen( szPath ) + 1 ];
	strcpy( mszCreateArchiveRootPath, szPath );
	MakeConsistentSlashes( mszCreateArchiveRootPath );

	// TODO - Change all existing file entries
}


void	Archive::AddFileToArchiveList( const char* szFilename )
{
	// TODO - See if exists in list first? 

FILE*		pFile = fopen( szFilename, "rb" );

	if ( pFile )
	{
	ArchiveFile*		pNewFile = new ArchiveFile;
	int		nFileSize;

		// Add to end of list
		if ( mpFileList )
		{
		ArchiveFile*		pList = mpFileList;

			while ( pList->GetNext() )
			{
				pList = pList->GetNext();
			}
			pList->SetNext( pNewFile );
		}
		else
		{
			mpFileList = pNewFile;
		}
		pNewFile->SetNext( NULL );

		mnNumFilesInList++;

		nFileSize = SysGetFileSize( pFile );
		
		pNewFile->SetFilename( szFilename, true );
		pNewFile->MakeArchiveLocalFilename( mszCreateArchiveRootPath );
		pNewFile->SetFileSize( nFileSize );

		fclose( pFile );
	}
}


void	Archive::SaveFileData( FILE* pOutFile )
{
ArchiveFile*	pFileList = mpFileList;
FILE*	pDataFile;
BYTE	abReadBuffer[1024];
int		nReadSize;
int		nReadPosition;
int		nFileSize;

	while( pFileList )
	{
		// If its a newly added file
		if ( pFileList->GetFullFilename() )
		{
			pDataFile = fopen( pFileList->GetFullFilename(), "rb" );
			if ( pDataFile )
			{
				nFileSize = SysGetFileSize( pDataFile );
				nReadSize = 0;
				nReadPosition = 0;
				while( nReadPosition < nFileSize )
				{
					if ( nFileSize - nReadPosition > 1024 )
					{
						nReadSize = 1024;
					}
					else
					{
						nReadSize = nFileSize - nReadPosition;					
					}

					fread( abReadBuffer, nReadSize, 1, pDataFile );
						
					switch( muwArchiveEncryptionMode )
					{
					case 1:
						ApplyLightEncryption( abReadBuffer, nReadSize, nReadPosition );
						fwrite( abReadBuffer, nReadSize, 1, pOutFile );
						break;
					case 0:
					default:
						// Write out the file directly
						fwrite( abReadBuffer, nReadSize, 1, pOutFile );
						break;
					}
					nReadPosition += nReadSize;
				}
				fclose( pDataFile );
			}
		}
		else	// Was already in the archive
		{
		int		nExistingFileHandle;
			//  Need to read the data from the existing archive file and then 
			// write it out to the new one
			nExistingFileHandle = OpenFile( pFileList->GetFilename() );
			if ( nExistingFileHandle != NOTFOUND )
			{
				nFileSize = GetFileSize( nExistingFileHandle );
				nReadSize = 0;
				nReadPosition = 0;
				while( nReadPosition < nFileSize )
				{
					if ( nFileSize - nReadPosition > 1024 )
					{
						nReadSize = 1024;
					}
					else
					{
						nReadSize = nFileSize - nReadPosition;					
					}

					ReadFile( nExistingFileHandle, abReadBuffer, nReadSize );
						
					switch( muwArchiveEncryptionMode )
					{
					case 1:
						ApplyLightEncryption( abReadBuffer, nReadSize, nReadPosition );
						fwrite( abReadBuffer, nReadSize, 1, pOutFile );
						break;
					case 0:
					default:
						// Write out the file directly
						fwrite( abReadBuffer, nReadSize, 1, pOutFile );
						break;
					}
					nReadPosition += nReadSize;
				}
				CloseFile( nExistingFileHandle );
			}
		}
		pFileList = pFileList->GetNext();
	}
}



byte*	Archive::GenerateHeader( int* pnMemSize )
{
byte*					pHeaderMem = (byte*)malloc( 100 * 1024 );		// temp 100k max size for TOC + Header
ARCHIVE_HEADER_V1*		pHeaderStruct = (ARCHIVE_HEADER_V1*)pHeaderMem;
ARCHIVE_TOC_ENTRY_HEADER_V1*		pTOCEntry = (ARCHIVE_TOC_ENTRY_HEADER_V1*)( pHeaderStruct + 1 );
char*		pcFilenameRunner = (char*)(pTOCEntry + 1);
ArchiveFile*	pFileList = mpFileList;
int			nCurrentFileOffset = 0;
int			nFilenameLen;

	if ( pHeaderMem )
	{
		strcpy( pHeaderStruct->acHeader, "TUA" );
		pHeaderStruct->bVersion = 1;
		pHeaderStruct->bHeaderSize = sizeof( ARCHIVE_HEADER_V1 );
		pHeaderStruct->bTOCEntryHeaderSize = sizeof( ARCHIVE_TOC_ENTRY_HEADER_V1 );
		pHeaderStruct->ulNumFiles = mnNumFilesInList;
		pHeaderStruct->uwEncryptionMode = muwArchiveEncryptionMode;

		nCurrentFileOffset = 0;

		while( pFileList )
		{
			memset( pTOCEntry, 0, pHeaderStruct->bTOCEntryHeaderSize );
			strcpy( pcFilenameRunner, pFileList->GetFilename() );
			pTOCEntry->bFilenameLen = (BYTE)( strlen( pcFilenameRunner ) + 1 );

			pTOCEntry->bTOCEntryFlags = 0; 
			pTOCEntry->ulFileOffset = nCurrentFileOffset; 
			pTOCEntry->ulFileSize = pFileList->GetFileSize(); 

			nCurrentFileOffset += pTOCEntry->ulFileSize;

			pcFilenameRunner += pTOCEntry->bFilenameLen;
			pTOCEntry = (ARCHIVE_TOC_ENTRY_HEADER_V1*)( pcFilenameRunner );
			pcFilenameRunner += pHeaderStruct->bTOCEntryHeaderSize;
			pFileList = pFileList->GetNext();
		}

		*pnMemSize = (int)( ((byte*)pTOCEntry) - pHeaderMem );

		// Now go through and fix up the file offsets to include the header and TOC
		pTOCEntry = (ARCHIVE_TOC_ENTRY_HEADER_V1*)( pHeaderStruct + 1 );
		pFileList = mpFileList;

		while( pFileList )
		{
			pTOCEntry->ulFileOffset += *pnMemSize; 
			nFilenameLen = pTOCEntry->bFilenameLen;
			pTOCEntry += 1;
			pTOCEntry = (ARCHIVE_TOC_ENTRY_HEADER_V1*)( ((byte*)pTOCEntry) + nFilenameLen );
			pFileList = pFileList->GetNext();	
		}

		// 'Encrypt' the TOC as required
		switch ( muwArchiveEncryptionMode )
		{
		case 1:
			ApplyLightEncryption( pHeaderMem + sizeof( ARCHIVE_HEADER_V1 ), *pnMemSize - sizeof( ARCHIVE_HEADER_V1 ), 0 );
			break;
		case 0:
		default:
			break;
		}
	}
	return( pHeaderMem );
}

void	Archive::SaveArchive( const char* szFilename )
{
FILE*	pFile = fopen( "ArchiveCreateTemp.dat", "wb" );
FILE*	pOutFile;

	if ( pFile )
	{
	byte*		pbHeaderTOCMem;
	int			nHeaderTOCSize = 0;

		pbHeaderTOCMem = GenerateHeader( &nHeaderTOCSize );
		fwrite( pbHeaderTOCMem, nHeaderTOCSize, 1, pFile );

		// Now write in the file data
		SaveFileData( pFile );

		fclose( pFile );
	}

	pOutFile = fopen( szFilename, "wb" );
	// If the destination file exists
	if ( pOutFile )
	{
		fclose( pOutFile );
		SysDeleteFile( szFilename );
	}
	rename( "ArchiveCreateTemp.dat", szFilename );

}

void	Archive::ExtractFile( const char* szPath, const char* szFilename )
{
int		nArchiveFileHandle;
int		fileSize;
FILE*	pOutFile;
char	szOutFilePath[256];
int		nBytesParsed = 0;
BYTE	abReadBuffer[1024];
int		nBytesRemaining;
int		nBytesToRead;
int		nFilePosition = 0;

	strcpy( szOutFilePath, szPath );
	strcat( szOutFilePath, "\\" );
	strcat( szOutFilePath, szFilename );
	
	SysMakeSubDirectories( szFilename, szPath );

	pOutFile = fopen( szOutFilePath, "wb" );
	if ( pOutFile )
	{
		nArchiveFileHandle = OpenFile( szFilename );
		if ( nArchiveFileHandle >= 0 )
		{
			fileSize = GetFileSize( nArchiveFileHandle );

			nBytesRemaining = fileSize;

			while( nBytesRemaining > 0 )
			{
				if ( nBytesRemaining > 1024 )
				{
					nBytesToRead = 1024;
				}
				else 
				{
					nBytesToRead = nBytesRemaining;
				}

				ReadFile( nArchiveFileHandle, abReadBuffer, nBytesToRead );
				fwrite( abReadBuffer, nBytesToRead, 1, pOutFile );

				nBytesRemaining -= nBytesToRead;
			}
			CloseFile( nArchiveFileHandle );
		}
		fclose( pOutFile );
	}
}


void	Archive::ExtractArchive( const char* szPath )
{
ArchiveFile*	pFileList = mpFileList;

	while( pFileList )
	{
		ExtractFile( szPath, pFileList->GetFilename() );
		pFileList = pFileList->GetNext();
	}

}


//--------------------------------------------------------------------------------------------------
//  Archive Manager with C interface

class ManagedArchive
{
public:
	ManagedArchive()
	{
		mpArchive = NULL;
		mhArchiveHandle = 0;
		mpNext = NULL;
		mszArchivePath = NULL;
	}
	
	
	int				mhArchiveHandle;
	Archive*		mpArchive;
	char*			mszArchivePath;

	ManagedArchive*		mpNext;

};

ManagedArchive*		mspManagedArchives = NULL;
int					msnArchiveManagerNextHandle = 101;

Archive*		ArchiveGetFromHandle( int nArchiveHandle )
{
ManagedArchive*		pArchiveList = mspManagedArchives;

	while( pArchiveList )
	{
		if ( pArchiveList->mhArchiveHandle == nArchiveHandle )
		{
			return( pArchiveList->mpArchive );
		}
		pArchiveList = pArchiveList->mpNext;
	}
	return( NULL );
}

void		ArchiveExtract( int nArchiveHandle, const char* szDestinationPath )
{
Archive*		pArchive = ArchiveGetFromHandle( nArchiveHandle );
	
	if ( pArchive )
	{
		pArchive->ExtractArchive( szDestinationPath );
	}
}

ManagedArchive*		ArchiveFind( const char* szArchiveLocalFilename )
{
ManagedArchive*		pArchiveList = mspManagedArchives;

	while( pArchiveList )
	{
		if ( tinstricmp( pArchiveList->mszArchivePath, szArchiveLocalFilename ) == 0 )
		{
			return( pArchiveList );
		}
		pArchiveList = pArchiveList->mpNext;
	}
	return( NULL );
}

int		ArchiveMount( const char* szArchiveLocalFilename )
{
ManagedArchive*		pArchiveList = ArchiveFind( szArchiveLocalFilename );

	if ( pArchiveList == NULL )
	{
	Archive*		pNewArchive;
	char			szActualFilePath[256];
	FILE*			pFile = fopen( szArchiveLocalFilename, "rb" );
	
		ArchiveGetConvertedFilePath( szArchiveLocalFilename, szActualFilePath );

		pFile = fopen( szActualFilePath, "rb" );
		if ( pFile == NULL )
		{
			return( NOTFOUND );
		}
		fclose( pFile );
		pArchiveList = new ManagedArchive;

		pNewArchive = new Archive;
		pNewArchive->MountArchive( szArchiveLocalFilename );

		pArchiveList->mhArchiveHandle = msnArchiveManagerNextHandle++;
		pArchiveList->mpArchive = pNewArchive;
		pArchiveList->mszArchivePath = (char*)( malloc( strlen( szArchiveLocalFilename ) + 1 ));
		strcpy( pArchiveList->mszArchivePath, szArchiveLocalFilename );

		pArchiveList->mpNext = mspManagedArchives;
		mspManagedArchives = pArchiveList;
	}
	return( pArchiveList->mhArchiveHandle );
}

void		ArchiveRelease( int nArchiveHandle )
{
ManagedArchive*		pArchiveList = mspManagedArchives;
ManagedArchive*		pLast = NULL;
ManagedArchive*		pNext;

	while( pArchiveList )
	{
		pNext = pArchiveList->mpNext;

		if ( pArchiveList->mhArchiveHandle == nArchiveHandle )
		{
			if ( pLast )
			{
				pLast->mpNext = pArchiveList->mpNext;
			}
			else
			{
				mspManagedArchives = pArchiveList->mpNext;
			}
			delete pArchiveList->mpArchive;
			free( pArchiveList->mszArchivePath );
			delete pArchiveList;
			return;
		}

		pLast = pArchiveList;
		pArchiveList = pNext;
	}

}



