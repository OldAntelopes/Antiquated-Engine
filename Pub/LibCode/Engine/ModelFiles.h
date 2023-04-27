
#ifndef MODEL_FILES_H
#define	MODEL_FILES_H

#ifdef __cplusplus
extern "C"
{
#endif


#define		ATM_CHUNK_FILE_HEADER_CODE		0x923b
#define		ATM_CHUNK_FILE_VERSION_NUM		2
#define		ATM_CHUNK_HEADER_CODE			0x55

enum		
{
	ATM_CHUNKID_OLD_BASIC_CONTENTS = 0,		// must always be 0. (WHY?!?)
	ATM_CHUNKID_BASIC_ANIMATION_DESCRIPTION,
	ATM_CHUNKID_VERTS,
	ATM_CHUNKID_NORMALS,
	ATM_CHUNKID_COLOURS,
	ATM_CHUNKID_UVS,
	ATM_CHUNKID_INDICES,
	ATM_CHUNKID_SIMPLE_MATERIALS,
	ATM_CHUNKID_SHARED_NORMALS,
	ATM_CHUNKID_VERT_KEYFRAMES,
	ATM_CHUNKID_NORMAL_KEYFRAMES,
	ATM_CHUNKID_VERY_COMPRESSED_UVS,
	ATM_CHUNKID_COMPRESSED_VERTS,
	ATM_CHUNKID_COMPRESSED_NORMALS,
	ATM_CHUNKID_COMPRESSED_VERT_KEYFRAMES,
	ATM_CHUNKID_COMPRESSED_NORMAL_KEYFRAMES,
 	ATM_CHUNKID_ANIMATED_UVS,
	ATM_CHUNKID_SHARED_COMPRESSED_NORMALS,
	ATM_CHUNKID_KEYFRAME_DATA,
	ATM_CHUNKID_TURRET_DATA,
	ATM_CHUNKID_ATTACH_DATA,
	ATM_CHUNKID_GLOBALPROPERTIES,
	ATM_CHUNKID_CREATOR_INFO,
	ATM_CHUNKID_LOD_DATA,
	ATM_CHUNKID_EFFECT_DATA,
	ATM_CHUNKID_TEXMATERIAL,	// TBI
	ATM_CHUNKID_ATTRIBUTEDATA,	// TBI
	ATM_CHUNKID_BASIC_CONTENTS,
	ATM_MAX_CHUNKID,
	
	ATM_CHUNKID_MODEL_SEPARATOR = 255,		// Always at end of list
};



enum
{
	ATM_EXPORT_FLAGS_NONE = 0,
	ATM_EXPORT_FLAGS_VERY_COMPRESSED_UVS = 0x1,
	ATM_EXPORT_FLAGS_COMPRESSED_NORMALS = 0x2,
	ATM_EXPORT_FLAGS_COLOURS = 0x4,
	ATM_EXPORT_FLAGS_COMPRESSED_VERTICES = 0x8,
	ATM_EXPORT_FLAGS_MATERIALS = 0x10,
	ATM_EXPORT_FLAGS_COMPRESSION = 0x20,
//	ATM_EXPORT_FLAGS_EFFECTS = 0x40,		// removed.. now automatic when an effect has been added.
};

typedef struct
{
	ushort		uwFileHeader;			// These first 4 fields must not change
	ushort		uwFileVersionNum;
	BYTE		bSizeOfFileHeader;
	BYTE		bSizeOfChunkHeader;
	BYTE		bCompressedFlag;
	BYTE		bPad2;

	uint32		ulUncompressedSize;

} ATM_FILE_HEADER;


typedef struct
{
	BYTE	bChunkCode;
	BYTE	bChunkDataNum;
	ushort	uwChunkID;						

	int		nNumElementsInChunk;
	int		nSizeOfElement;

} ATM_CHUNK_HEADER;


typedef struct
{
	BYTE	bModelType;
	BYTE	bNumMaterials;
	ushort	uwNumVertices;
	
	uint32	ulUniqueModelID;

	ushort	uwNumFaces;
	ushort	uwContentsFlags;

	ushort	uwNumNormals;
	byte	bWheelDisplayMode;
	byte	bPad1;

	uint32	ulLockID;
	uint32	ulLockCode;

} ATM_OLD_BASIC_CONTENTS;

typedef struct
{
	BYTE	bModelType;
	BYTE	bNumMaterials;
	ushort	uwPad1;

	int		nNumVertices;
	
	uint32	ulUniqueModelID;

	int		nNumFaces;
	
	ushort	uwContentsFlags;
	ushort	uwPad2;

	int		nNumNormals;
	byte	bWheelDisplayMode;
	byte	bPad1;
	ushort	uwPad3;

	uint32	ulLockID;
	uint32	ulLockCode;

} ATM_BASIC_CONTENTS;

extern void		ModelExportATM( int nModelHandle, const char* szFilename, uint32 ulSaveFlags, uint32 ulLockFlags );


extern void		ModelLoaderExport3ds( int nModelHandle, const char* acFilename );
extern void		ModelLoaderExportFBX( int nModelHandle, const char* acFilename );

 
#ifdef __cplusplus
}
#endif


#endif
