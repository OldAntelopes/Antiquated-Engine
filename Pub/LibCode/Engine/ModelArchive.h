
#ifndef ENGINE_MODEL_ARCHIVE_H
#define	ENGINE_MODEL_ARCHIVE_H

typedef struct
{
	BYTE	bArchiveHeaderByte;
	BYTE	bArchiveHeaderByte2;

	BYTE	bArchiveVersionNum;
	BYTE	bArchiveHeaderSize;

	int		nNumberOfFilesInArchive;
	BYTE	bPad1;
	BYTE	bPad2;
	BYTE	bPad3;
	BYTE	bPad4;

} MODEL_ARCHIVE_FILE_HEADER;
 

class ModelArchive
{
public:
	ModelArchive();
	~ModelArchive();

	void		Load( const char* szFilename );
	void		Render( const VECT* pxPos, const VECT* pxRot, uint32 ulRenderFlags );
private:

	class SceneMap*		mpMap;
};

extern void	EngineModelArchiveFillHeader( MODEL_ARCHIVE_FILE_HEADER* pHeader, int nNumberOfFiles );

#endif
