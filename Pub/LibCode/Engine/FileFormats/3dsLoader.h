#ifndef MODEL_CONVERTER_3DS_LOADER_H
#define MODEL_CONVERTER_3DS_LOADER_H

//>------ Primary Chunk, at the beginning of each file
#define CHUNKID3DS_PRIMARY       0x4D4D
//>------ Main Chunks
#define CHUNKID3DS_EDIT3DS       0x3D3D
#define CHUNKID3DS_VERSION       0x0002
#define CHUNKID3DS_MESHVERSION   0x3D3E
#define CHUNKID3DS_EDITKEYFRAME  0xB000
//>------ sub defines of EDIT3DS
#define CHUNKID3DS_EDITMATERIAL  0xAFFF
#define CHUNKID3DS_EDITOBJECT    0x4000
//>------ sub defines of EDITMATERIAL
#define CHUNKID3DS_MATNAME       0xA000  
#define CHUNKID3DS_MATLUMINANCE  0xA010
#define CHUNKID3DS_MATDIFFUSE    0xA020
#define CHUNKID3DS_MATSPECULAR   0xA030
#define CHUNKID3DS_MATSHININESS  0xA040
#define CHUNKID3DS_MATMAP        0xA200
#define CHUNKID3DS_MATMAPFILE    0xA300
#define CHUNKID3DS_OBJTRIMESH    0x4100
//>------ sub defines of OBJTRIMESH
#define CHUNKID3DS_TRIVERT       0x4110
#define CHUNKID3DS_TRIFACE       0x4120
#define CHUNKID3DS_TRIFACEMAT    0x4130
#define CHUNKID3DS_TRIUV         0x4140
#define CHUNKID3DS_TRISMOOTH     0x4150
#define CHUNKID3DS_TRILOCAL      0x4160
//>------ sub defines of EIDTKEYFRAME
#define CHUNKID3DS_KFMESH        0xB002
#define CHUNKID3DS_KFHEIRARCHY   0xB030
#define CHUNKID3DS_KFNAME        0xB010
//>>------  these define the different color chunk types
#define CHUNKID3DS_RGBF			 0x0010
#define CHUNKID3DS_RGB24		 0x0011


#ifdef __cplusplus

	
class Chunk;

class Load3ds
{
public:
	Load3ds();
	~Load3ds();
	void * Create(char *);private:
	int GetString(char *);
	int ReadChunk(Chunk *);
	int ReadColorChunk(Chunk *, float *);
	int ReadPercentChunk(Chunk *, float *);	int ProcessNextChunk(Chunk *);
	int ProcessNextObjectChunk(Chunk *);
	int ProcessNextMaterialChunk(Chunk *);
	int ProcessNextKeyFrameChunk(Chunk *);
	int FillVertexBuffer(Chunk *);
	int FillIndexBuffer(Chunk *);
	int FillTexCoordBuffer(Chunk *);
	int SortIndicesByMaterial(Chunk *);
	
	int ComputeNormals();
	int CompileObjects();
	int CleanUp();
	
	FILE * mFile;
	BYTE* mBuffer;
	
	Chunk * mCurrentChunk;
	Chunk * mTempChunk;
};

class Chunk
{
public:
	Chunk();
	~Chunk();	unsigned short int mID;
	unsigned int mLength;
	unsigned int mBytesRead;
};


#endif

//---------------------------------------------------------------------- External interface

#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif


extern int		ModelLoad3ds( MODEL_RENDER_DATA* pxModelData, const char* szFilename );


#ifdef __cplusplus
}
#endif


#endif
