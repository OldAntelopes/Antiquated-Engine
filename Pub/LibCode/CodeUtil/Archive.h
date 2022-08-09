#ifndef ARCHIVE_H
#define ARCHIVE_H

#ifdef __cplusplus

class ArchiveFile;
class ArchiveFileRef;


class Archive
{
public:
	Archive();
	~Archive();

	int			MountArchive( const char* szArchiveFilename );
	
	// ------------------------------------------------------------------
	// For querying the contents of an archive
	int				GetNumFiles( void ) { return( mnNumFilesInList ); }
	const char*		GetFileName( int index, int* pnFileSize, int* pnFileOffset );

	// ------------------------------------------------------------------
	// For accessing files within an archive
	int				OpenFile( const char* szFilename );
	int				GetFileSize( int fileHandle );
	int				ReadFile( int fileHandle, byte* pbMem, int nSize );
	int				SeekFile( int fileHandle, int nOffset, int nMode );
	int				CloseFile( int fileHandle );

	// ------------------------------------------------------------------
	// For tools/construction of archives etc
	const char*	GetRootPath( void ) { return( mszCreateArchiveRootPath ); }
	void		SetRootPath( const char* szPath );
	void		AddFileToArchiveList( const char* szFilename );
	void		SaveArchive( const char* szFilename );
	void		ExtractArchive( const char* szPath );
	void		ExtractFile( const char* szPath, const char* szFilename );
	// ------------------------------------------------------------------

private:
	byte*			GenerateHeader( int* pnMemSize );
	void			ResetFileList();
	void			SaveFileData( FILE* pOutFile );
	ArchiveFile*	FindArchiveFile( const char* szFilename );
	ArchiveFileRef*	GetFileRef( int nFileHandle );
	void			MakeConsistentSlashes( char* );

	ushort				muwArchiveEncryptionMode;
	ArchiveFile*		mpFileList;
	int					mnNumFilesInList;
	char*				mszCreateArchiveRootPath;

	char*				mszLoadedArchiveFilename;

	ArchiveFileRef*		m_pArchiveFileReferences;

};

Archive*		ArchiveGetFromHandle( int nArchiveHandle );

#endif //		#ifdef __cplusplus


#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------- C Interface

extern int		ArchiveMount( const char* szArchiveLocalFilename );

extern void		ArchiveRelease( int nArchiveHandle );



#ifdef __cplusplus
}
#endif


#endif //#ifndef ARCHIVE_H



