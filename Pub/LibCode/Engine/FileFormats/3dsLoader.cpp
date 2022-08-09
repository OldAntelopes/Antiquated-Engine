#include <stdio.h>

#include <StandardDef.h>
#include <Engine.h>
#include <System.h>
#include <Rendering.h>

#include "../ModelMaterialData.h"
#include "../ModelFiles.h"		
#include "../ModelRendering.h"
#include "../Loader.h"

#include "3dsLoader.h"

#define LOAD_LOGGING_3DS


typedef struct
{
	int		nNumVertices;
	int		nNumFaces;
	int		nNumMaterials;

} LOADER_INFO; 

LOADER_INFO		mxLoaderInfo;


enum
{
	MAT_NAME = 0xA000,
	MAT_AMBIENT = 0xA010,
	
	MAT_DIFFUSE = 0xA020,
	MAT_SPECULAR = 0xA030,
	MAT_SHININESS = 0xA040,
	MAT_SHIN2PCT = 0xA041,
	
	MAT_SELF_ILPCT = 0xA084,
	
	MAT_MAP_USCALE = 0xA354,
	MAT_MAP_VSCALE = 0xA356,
	MAT_MAP_UOFFSET = 0xA358,
	MAT_MAP_VOFFSET = 0xA35A,
};

ENGINEMATRIX	mx3dsLoadLocalTransform;

//--------------------------------------------------------------------------- Load3ds class
Load3ds::Load3ds()
{
	mBuffer = 0;
	mCurrentChunk = 0;
	mTempChunk = new Chunk;	// initialize any other members you choose to add
}

Load3ds::~Load3ds()
{
	delete mTempChunk;
}

#define		MAX_VERTEX_INDICES		150000

void * Load3ds::Create(char * aFilename)
{
	int lBytesRead = 0;
	mCurrentChunk = new Chunk;
	mFile = fopen(aFilename, "rb");
	ReadChunk(mCurrentChunk);
	if (mCurrentChunk->mID != CHUNKID3DS_PRIMARY)
		exit(1107);
	// Largest possible buffer we'd need is MAX_INDICES * sizeof(triangle)
	mBuffer = new unsigned char[MAX_VERTEX_INDICES * 3 * sizeof(float)];	
	// Begin loading objects, by calling recursive function
	ProcessNextChunk(mCurrentChunk);	
	// All data read, compile them into a friendly format
	CompileObjects();
	// Clean up after everything, to prepare for next time
	CleanUp();
	return 0;
}

int Load3ds::CleanUp()
{	
	fclose(mFile);
	delete [] mBuffer;
	delete mCurrentChunk;
/*
Add any cleanup routines you may need between calls to Load3ds::Create(),
such as deleting temporary vertex/index lists, materials, or whatever
else you choose to add
*/	return 1;
}

int Load3ds::ProcessNextChunk(Chunk * aPreviousChunk)
{
	mCurrentChunk = new Chunk;	
	
	while (aPreviousChunk->mBytesRead < aPreviousChunk->mLength)
	{
		// Read next chunk
		ReadChunk(mCurrentChunk);		
		
		switch (mCurrentChunk->mID)
		{
		case CHUNKID3DS_VERSION:
			// Check version (must be 3 or less)
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			if (*(unsigned short *) mBuffer > 0x03)
				exit(1107);
			break;		
		case CHUNKID3DS_EDITMATERIAL:
			// Proceed to material loading function
			ProcessNextMaterialChunk(mCurrentChunk);
			break;		
		case CHUNKID3DS_EDIT3DS:
			// Check mesh verion, then proceed to mesh loading function			
			ReadChunk(mTempChunk);
			mTempChunk->mBytesRead += fread(mBuffer, 1, mTempChunk->mLength - mTempChunk->mBytesRead, mFile);
			mCurrentChunk->mBytesRead += mTempChunk->mBytesRead;
			if (mTempChunk->mID != CHUNKID3DS_MESHVERSION || *(unsigned short *)mBuffer > 0x03)
				exit(1107);
			ProcessNextChunk(mCurrentChunk);
			break;		
		case CHUNKID3DS_EDITOBJECT:
			mCurrentChunk->mBytesRead += GetString((char *)mBuffer);
			// mBuffer now contains name of the object to be edited
			ProcessNextObjectChunk(mCurrentChunk);
			break;		
		case CHUNKID3DS_EDITKEYFRAME:
			ProcessNextKeyFrameChunk(mCurrentChunk);
			break;		
		default:  // unrecognized/unsupported chunk
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;
		}	
		aPreviousChunk->mBytesRead += mCurrentChunk->mBytesRead;
	}	
	delete mCurrentChunk;
	mCurrentChunk = aPreviousChunk;	
	return 1;
}

int Load3ds::ProcessNextObjectChunk(Chunk * aPreviousChunk)
{
	mCurrentChunk = new Chunk;	
	while (aPreviousChunk->mBytesRead < aPreviousChunk->mLength)
	{
		ReadChunk(mCurrentChunk);		
		switch (mCurrentChunk->mID)
		{
		case CHUNKID3DS_OBJTRIMESH:
			// at this point, mBuffer will contain the name of the object being described
			ProcessNextObjectChunk(mCurrentChunk);
			ComputeNormals();			
			break;		
		case CHUNKID3DS_TRIVERT:
			FillVertexBuffer(mCurrentChunk);
			break;		
		case CHUNKID3DS_TRIFACE:
			FillIndexBuffer(mCurrentChunk);
			break;		
		case CHUNKID3DS_TRIFACEMAT:
			// your getting a list of triangles that belong to a certain material
			SortIndicesByMaterial(mCurrentChunk);
			break;		
		case CHUNKID3DS_TRIUV:
			FillTexCoordBuffer(mCurrentChunk);
			break;		
		default:  // unrecognized/unsupported chunk
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;
		}	
		aPreviousChunk->mBytesRead += mCurrentChunk->mBytesRead;
	}	
	delete mCurrentChunk;
	mCurrentChunk = aPreviousChunk;	
	return 1;
}

int Load3ds::ProcessNextMaterialChunk(Chunk * aPreviousChunk)
{
	mCurrentChunk = new Chunk;	
	while (aPreviousChunk->mBytesRead < aPreviousChunk->mLength)
	{
		ReadChunk(mCurrentChunk);	
		switch (mCurrentChunk->mID)
		{
		case CHUNKID3DS_MATNAME:
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;		
		case CHUNKID3DS_MATLUMINANCE:
			ReadColorChunk(mCurrentChunk, (float *) mBuffer);
			break;		
		case CHUNKID3DS_MATDIFFUSE:
			ReadColorChunk(mCurrentChunk, (float *) mBuffer);
			break;		
		case CHUNKID3DS_MATSPECULAR:
			ReadColorChunk(mCurrentChunk, (float *) mBuffer);
			break;
		case CHUNKID3DS_MATSHININESS:
			ReadColorChunk(mCurrentChunk, (float *) mBuffer);
			break;
		case CHUNKID3DS_MATMAP:
			// texture map chunk, proceed
			ProcessNextMaterialChunk(mCurrentChunk);
			break;		
		case CHUNKID3DS_MATMAPFILE:
			mCurrentChunk->mBytesRead += fread((char *)mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			// mBuffer now contains the filename of the next texture; load it if you wish
			break;
		
		default:  // unrecognized/unsupported chunk
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;
		}	
		aPreviousChunk->mBytesRead += mCurrentChunk->mBytesRead;
	}	
	delete mCurrentChunk;
	mCurrentChunk = aPreviousChunk;
	return 1;
}
int Load3ds::ProcessNextKeyFrameChunk(Chunk * aPreviousChunk)
{
	mCurrentChunk = new Chunk;	
	short int lCurrentID, lCurrentParentID;
	
	while (aPreviousChunk->mBytesRead < aPreviousChunk->mLength)
	{
		ReadChunk(mCurrentChunk);	
		switch (mCurrentChunk->mID)
		{
		case CHUNKID3DS_KFMESH:
			ProcessNextKeyFrameChunk(mCurrentChunk);
			break;		
		case CHUNKID3DS_KFHEIRARCHY:
			mCurrentChunk->mBytesRead += fread(&lCurrentID, 1, 2, mFile);
			// lCurrentID now contains the ID of the current object being described
			//  Save this if you want to support an object hierarchy
			break;		
		case CHUNKID3DS_KFNAME:
			mCurrentChunk->mBytesRead += GetString((char *)mBuffer);
			// mBuffer now contains the name of the object whose KF info will
			//   be described

			mCurrentChunk->mBytesRead += fread(mBuffer, 1, 4, mFile);  // useless, ignore
			mCurrentChunk->mBytesRead += fread(&lCurrentParentID, 1, 2, mFile);
			// lCurrentParentID now contains the ID of the parent of the current object 
			// being described
			break;		
		default:  // unrecognized/unsupported chunk
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;
		}	
		aPreviousChunk->mBytesRead += mCurrentChunk->mBytesRead;
	}	
	delete mCurrentChunk;
	mCurrentChunk = aPreviousChunk;	
	return 1;
}
int Load3ds::GetString(char * aBuffer)
{
	unsigned int lBytesRead = 0;
	int index = 0;	
	fread(aBuffer, 1, 1, mFile);	
	
	while (*(aBuffer + index) != 0)
		fread(aBuffer + ++index, 1, 1, mFile);	
	
	return strlen(aBuffer) + 1;
}
int Load3ds::ReadChunk(Chunk * aChunk)
{
	aChunk->mBytesRead = fread(&aChunk->mID, 1, 2, mFile);
	aChunk->mBytesRead += fread(&aChunk->mLength, 1, 4, mFile);	
	return 1;
}

int Load3ds::ReadColorChunk(Chunk * aChunk, float * aVector)
{
	ReadChunk(mTempChunk);
	mTempChunk->mBytesRead += fread(mBuffer, 1, mTempChunk->mLength - mTempChunk->mBytesRead, mFile);
	aChunk->mBytesRead += mTempChunk->mBytesRead;
	return 1;
}
int Load3ds::ReadPercentChunk(Chunk * aChunk, float * aPercent)
{
	ReadChunk(mTempChunk);
	mTempChunk->mBytesRead += fread(mBuffer, 1, mTempChunk->mLength - mTempChunk->mBytesRead, mFile);
	*aPercent = (float) *((short int *) mBuffer) / 100.0f;
	aChunk->mBytesRead += mTempChunk->mBytesRead;
	return 1;
}
int Load3ds::FillIndexBuffer(Chunk * aPreviousChunk)
{
	short int lNumFaces;
	aPreviousChunk->mBytesRead += fread(&lNumFaces, 1, 2, mFile);
	aPreviousChunk->mBytesRead += fread(mBuffer, 1, aPreviousChunk->mLength - aPreviousChunk->mBytesRead, mFile);
	// mBuffer now contains an array of indices (unsigned short ints)
	//   Careful, the list consists of 3 vertex indices and then an edge
	//   flag (safe to ignore, probably
	//   * bit 0: CA visible, bit 1: BC visible, bit 2: AB visible

	return 1;
}
int Load3ds::SortIndicesByMaterial(Chunk * aPreviousChunk)
{
	unsigned short int lNumFaces;
	aPreviousChunk->mBytesRead += GetString((char *) mBuffer);
	// mBuffer contains the name of the material that is associated
	//  with the following triangles (set of 3 indices which index into the vertex list
	//  of the current object chunk)

	aPreviousChunk->mBytesRead += fread(&lNumFaces, 1, 2, mFile);
	aPreviousChunk->mBytesRead += fread(mBuffer, 1, aPreviousChunk->mLength - aPreviousChunk->mBytesRead, mFile);
	// mBuffer now contains a list of triangles that use the material specified above

	return 1;
}
int Load3ds::FillTexCoordBuffer(Chunk * aPreviousChunk)
{
	int lNumTexCoords;
	aPreviousChunk->mBytesRead += fread(&lNumTexCoords, 1, 2, mFile);
	aPreviousChunk->mBytesRead += fread(mBuffer, 1, aPreviousChunk->mLength - aPreviousChunk->mBytesRead, mFile);
	// mBuffer now contains a list of UV coordinates (2 floats)

	return 1;
}
int Load3ds::FillVertexBuffer(Chunk * aPreviousChunk)
{
	int lNumVertices;
	aPreviousChunk->mBytesRead += fread(&lNumVertices, 1, 2, mFile);
	aPreviousChunk->mBytesRead += fread(mBuffer, 1, aPreviousChunk->mLength - aPreviousChunk->mBytesRead, mFile);
	// mBuffer now contains a list of vertex coordinates (3 floats)

	return 1;
}
int Load3ds::ComputeNormals()
{
/*
Compute your normals here. Quick way of doing it (no smoothing groups, though):  for (each vertex in vertex list)
	{
	for (each triangle in triangle list)
	  {
	  if (vertex is in triangle)
	    {
		compute normal of triangle
		add to total_normal
		}
	  }
	normalize(total_normal)
	save_to_normal_list(total_normal)
	total_normal = 0
	}
	
This seemed to work well for me, though it's my first time having to compute
normals myself. One might think that normalizing the normal right after it's 
computed and added to total_normal would be best, but I've found this not to be
the case. If this is done, then small triangles have just as much influence on the 
final normal as larger triangles. My way, the model comes out looking much more 
smooth, especially if there's small flaws in the model. 
*/	return 1;
}

int Load3ds::CompileObjects()
{/*
By now, you should have a collection of objects (or a single object), with 
proper hierarchy information, vertices, indices, and materials. Now's your
time to assemble these into however you want them to send them down the 
OGL/D3D pipeline.*/
	return 1;
}
///////////////////////////////////////////////////////////////////

Chunk::Chunk()
{
	mID = 0;
	mLength = 0;
	mBytesRead = 0;
}
Chunk::~Chunk()
{
}
 


//----------------------------------------------------------------------------------------------------------------------

/***************************************************************************
 * Function    : ModelExport3dsMaterials
 * Params      :
 * Description : 
 ***************************************************************************/
void		ModelExport3dsMaterials(int nModelHandle, FILE* pFile )
{
ulong	ulMatChunk = 253;//60;//75;
ushort	uwVal;
ulong	ulVal;
char	acString[256];
BYTE	bVal;
float	fVal;

			uwVal = 0xafff;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = ulMatChunk;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );

			uwVal = 0xa000;		// mat name
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 6 + 5;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
			sprintf( acString, "mat1" );
			fwrite( acString, 5, 1, pFile );

			uwVal = MAT_AMBIENT;		// 
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 15;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0011;		// colour chunk
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 9;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				bVal = 0x0;
				fwrite( &bVal, sizeof( byte ), 1, pFile );
				fwrite( &bVal, sizeof( byte ), 1, pFile );
				fwrite( &bVal, sizeof( byte ), 1, pFile );

			uwVal = MAT_DIFFUSE;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 15;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0011;	// colour chunk
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 9;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				bVal = 0xFF;
				fwrite( &bVal, sizeof( byte ), 1, pFile );
				fwrite( &bVal, sizeof( byte ), 1, pFile );
				fwrite( &bVal, sizeof( byte ), 1, pFile );

			uwVal = MAT_SPECULAR;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 15;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0011;		// colour chunk
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 9;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				bVal = 0x0;
				fwrite( &bVal, sizeof( byte ), 1, pFile );
				fwrite( &bVal, sizeof( byte ), 1, pFile );
				fwrite( &bVal, sizeof( byte ), 1, pFile );
	
			uwVal = MAT_SHININESS;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 14;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0030;		// percentage chunk
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 8;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			uwVal = MAT_SHIN2PCT;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 14;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0030;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 8;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			uwVal = 0xa050;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 14;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0030;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 8;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			uwVal = 0xa052;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 14;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0030;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 8;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			uwVal = 0xa053;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 14;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0030;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 8;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			uwVal = 0xa100;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 8;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
			uwVal = 3;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );

			uwVal = MAT_SELF_ILPCT;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 14;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0030;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 8;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			uwVal = 0xa08a;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 6;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );

			uwVal = 0xa087;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 10;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
			fVal = 0.0f;
			fwrite( &fVal, sizeof( float), 1, pFile );
			
			uwVal = 0xa200;
			fwrite( &uwVal, sizeof( ushort ), 1, pFile );
			ulVal = 83;
			fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0x0030;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 8;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 100;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );

				uwVal = 0xa300;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 6 + 5;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				sprintf( acString, "tex1" );
				fwrite( acString, 5, 1, pFile );

				uwVal = 0xa351;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 8;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				uwVal = 0;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );

				uwVal = 0xa353;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 10;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				fVal = 0.0f;
				fwrite( &fVal, sizeof( float ), 1, pFile );
				// chunk so far = 37

				uwVal = MAT_MAP_USCALE;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 10;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				fVal = 1.0f;
				fwrite( &fVal, sizeof( float ), 1, pFile );

				uwVal = MAT_MAP_VSCALE;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 10;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				fVal = 1.0f;
				fwrite( &fVal, sizeof( float ), 1, pFile );

				uwVal = MAT_MAP_UOFFSET;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 10;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				fVal = 0.0f;
				fwrite( &fVal, sizeof( float ), 1, pFile );

				uwVal = MAT_MAP_VOFFSET;
				fwrite( &uwVal, sizeof( ushort ), 1, pFile );
				ulVal = 10;
				fwrite( &ulVal, sizeof( ulong ), 1, pFile );
				fVal = 0.0f;
				fwrite( &fVal, sizeof( float ), 1, pFile );

}

/***************************************************************************
 * Function    : ModelLoaderExport3ds
 * Params      :
 * Description : 
 ***************************************************************************/
void		ModelLoaderExport3ds(int nModelHandle, const char* acFilename )
{
MODEL_RENDER_DATA* pxModelData;
CUSTOMVERTEX*		pxVertices;
ushort*			puwIndices;
FILE*	pFile;
int		nVertexChunkSize;
int		nFaceChunkSize;
int		nMappingChunkSize;
ushort	uwVal;
ulong	ulObjectChunkSize;
ulong	ulVal;
BYTE	bVal;
int		nLoop;
VECT	xVect;
float	fVal;
ulong	ulExtras = 54;
//char	acString[256];
ulong	ulMatChunk = 253;

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pFile = fopen( acFilename, "wb");

	if ( pFile != NULL )
	{
		nVertexChunkSize = 6 + sizeof(ushort) + (pxModelData->xStats.nNumVertices * 3 * sizeof(float));
		nFaceChunkSize = 6 + sizeof(ushort) + (pxModelData->xStats.nNumIndices * sizeof(ushort))+ ((pxModelData->xStats.nNumIndices/3) * sizeof(ushort));
		nMappingChunkSize = 6 + sizeof(ushort) + (pxModelData->xStats.nNumVertices * 2 * sizeof(float));

		ulObjectChunkSize = nVertexChunkSize + nFaceChunkSize + nMappingChunkSize;

		uwVal = 0x4d4d;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = ulObjectChunkSize + 24 + 1 + ulExtras + 30 + ulMatChunk;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );

		uwVal = 0x2;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = 10;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );
		ulVal = 3;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );

		uwVal = 0x3d3d;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = ulObjectChunkSize + 18 + 1 + ulExtras + 20 + ulMatChunk;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );

		uwVal = 0x3d3e;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = 10;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );
		ulVal = 3;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );

			// Write a basic material
			ModelExport3dsMaterials( nModelHandle, pFile );

		uwVal = 0x0100;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = 10;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );
		fVal = 1.0f;
		fwrite( &fVal, sizeof( float ), 1, pFile );

		uwVal = 0x4000;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = ulObjectChunkSize + 12 + 1 + ulExtras;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );
		bVal = 0;
		fwrite( &bVal, sizeof( BYTE ), 1, pFile );

		uwVal = 0x4100;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = 6 + ulObjectChunkSize + ulExtras;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );

		uwVal = 0x4110;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = nVertexChunkSize;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );

		uwVal = (ushort)(pxModelData->xStats.nNumVertices);
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
		{
			xVect = pxVertices[nLoop].position;
			fwrite( &xVect.x, sizeof(float), 1, pFile );
			fwrite( &xVect.y, sizeof(float), 1, pFile );
			fwrite( &xVect.z, sizeof(float), 1, pFile );
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer( );

		uwVal = 0x4140;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = nMappingChunkSize;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );

		uwVal = (ushort)(pxModelData->xStats.nNumVertices);
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
		{
			fwrite( &pxVertices[nLoop].tu, sizeof(float), 1, pFile );
			fwrite( &pxVertices[nLoop].tv, sizeof(float), 1, pFile );
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer( );

		uwVal = 0x4160;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = 54;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );
		fVal = 1.0f;		
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fVal = 0.0f;		
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fVal = 1.0f;		
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fVal = 0.0f;		
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fVal = 1.0f;		
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fVal = 0.0f;		
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fwrite( &fVal, sizeof( float ), 1, pFile );
		fwrite( &fVal, sizeof( float ), 1, pFile );

		uwVal = 0x4120;
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );
		ulVal = nFaceChunkSize;
		fwrite( &ulVal, sizeof( ulong ), 1, pFile );
		uwVal = (ushort)(pxModelData->xStats.nNumIndices / 3);
		fwrite( &uwVal, sizeof( ushort ), 1, pFile );

		uwVal = 0;
		pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumIndices; nLoop++ )
		{
			fwrite( &puwIndices[nLoop], sizeof(ushort), 1, pFile );
			if ( (nLoop % 3) == 2 )
			{
				fwrite( &uwVal, sizeof(ushort), 1, pFile );
			}
		}
		pxModelData->pxBaseMesh->UnlockIndexBuffer();


		fclose( pFile );
	}
}



/***************************************************************************
 * Function    : ModelLoaderGenerateNormals
 * Params      :
 * Description : Generates flat shaded normals
 ***************************************************************************/
void		ModelLoaderGenerateNormals(MODEL_RENDER_DATA* pxModelData )
{
VECT		xVec1;
VECT		xVec2;
VECT		xCross;
CUSTOMVERTEX*	pxVertices;
CUSTOMVERTEX*	pxVert1;
CUSTOMVERTEX*	pxVert2;
CUSTOMVERTEX*	pxVert3;
ushort*		puwIndices;
int		nLoop;
int		nNumFaces;

	nNumFaces = (pxModelData->xStats.nNumIndices/3);
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
	for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
	{
		pxVert1 = &pxVertices[ puwIndices[nLoop*3] ];
		pxVert2 = &pxVertices[ puwIndices[(nLoop*3)+1] ];
		pxVert3 = &pxVertices[ puwIndices[(nLoop*3)+2] ];

		VectSub( &xVec1, &pxVert1->position, &pxVert2->position );
		VectSub( &xVec2, &pxVert1->position, &pxVert3->position );

		VectCross( &xCross, &xVec1, &xVec2 );
		VectNormalize( &xCross );
		
		// (this'll do the smooth shading if the vertices are shared)
		VectAdd( &pxVert1->normal, &pxVert1->normal, &xCross );
		VectNormalize( &pxVert1->normal );
		VectAdd( &pxVert2->normal, &pxVert2->normal, &xCross );
		VectNormalize( &pxVert2->normal );
		VectAdd( &pxVert3->normal, &pxVert3->normal, &xCross );
		VectNormalize( &pxVert3->normal );
	}
	pxModelData->pxBaseMesh->UnlockVertexBuffer( );
	pxModelData->pxBaseMesh->UnlockIndexBuffer( );
}



void	ReadMaterialInfo(FILE* pFile, ulong ulParentChunkLength)
{
ulong		ulBytesRead = 0;
ushort	uwChunkID;
ulong	ulChunkLength;

	while (ulBytesRead < ulParentChunkLength)
	{
		 //Read the chunk header
		fread ( &uwChunkID, 2, 1, pFile );
		fread ( &ulChunkLength, 4, 1, pFile ); 
		ulBytesRead += 6;

		switch (uwChunkID)
		{
		case CHUNKID3DS_MATNAME:
			mxLoaderInfo.nNumMaterials++;
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;		
		case CHUNKID3DS_MATLUMINANCE:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;		
		case CHUNKID3DS_MATDIFFUSE:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;		
		case CHUNKID3DS_MATSPECULAR:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;
		case CHUNKID3DS_MATSHININESS:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;
		case CHUNKID3DS_MATMAP:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;		
		case CHUNKID3DS_MATMAPFILE:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;	
		default:  // unrecognized/unsupported chunk
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;
		}	
		ulBytesRead += ulChunkLength;
	}	
}


//-------------------------------------------------------
// ModelConvGetRequiredInfo
//  First pass of the load process.. Goes through the 3ds file
//  and extracts info, such as the number of vertices and number
//  of faces, which is required to allocate the space for 
//  a new model.
//-------------------------------------------------------
void		ModelConvGetRequiredInfo( FILE* pFile, int nFileSize )
{
ushort	uwChunkID;
ulong	ulChunkLength;
ushort	uwData;
int		nLoop;
char	cChar;

	mxLoaderInfo.nNumVertices = 0;
	mxLoaderInfo.nNumFaces = 0;
	mxLoaderInfo.nNumMaterials = 0;

	//Loop to scan the whole file 
	while ( ftell(pFile) < nFileSize ) 
	{
		 //Read the chunk header
		fread ( &uwChunkID, 2, 1, pFile );
		fread ( &ulChunkLength, 4, 1, pFile ); 

		switch (uwChunkID)
        {
		case CHUNKID3DS_PRIMARY: 
		case CHUNKID3DS_EDIT3DS:
		case CHUNKID3DS_OBJTRIMESH:
		case CHUNKID3DS_MATMAP:
			break;
		case CHUNKID3DS_EDITMATERIAL:
			ReadMaterialInfo(pFile, ulChunkLength);
			break;		
		case CHUNKID3DS_EDITOBJECT:
			nLoop = 0;
			do
			{	
				fread (&cChar, 1, 1, pFile);
				nLoop++;
			} 
			while( cChar != '\0' && nLoop < 20);
			break;
		//--------------- TRI_VERTEXL ---------------
		// Description: Vertices list
		// Chunk ID: 4110 (hex)
		// Chunk Lenght: 1 x unsigned short (number of vertices) 
		//             + 3 x float (vertex coordinates) x (number of vertices)
		//             + sub chunks
		//-------------------------------------------
		case CHUNKID3DS_TRIVERT: 
			fread (&uwData, sizeof(ushort), 1, pFile);
			mxLoaderInfo.nNumVertices += uwData;
			fseek( pFile, ulChunkLength- 6 - 2, SEEK_CUR );
			break;
		case CHUNKID3DS_VERSION:	
			// ???
			fread (&uwData, sizeof(ushort), 1, pFile);
			fseek( pFile, ulChunkLength -6 - 2, SEEK_CUR );
			break;
			//--------------- TRI_FACEL1 ----------------
			// Description: Polygons (faces) list
			// Chunk ID: 4120 (hex)
			// Chunk Lenght: 1 x unsigned short (number of polygons) 
			//             + 3 x unsigned short (polygon points) x (number of polygons)
			//             + sub chunks
			//-------------------------------------------
		case CHUNKID3DS_TRIFACE:
			fread (&uwData, sizeof(ushort), 1, pFile);
			mxLoaderInfo.nNumFaces += uwData;

			fseek( pFile, ulChunkLength -6 - 2, SEEK_CUR );
            break;
		case 0x0030:
		case 0xA100:
		case 0xA351:
		case 0xA353:
		case 0x1101:
		case 0x3d3e:
		case 0x4160:
		case 0x4170:
		default:
			 fseek(pFile, ulChunkLength-6, SEEK_CUR);
			 break;
        } 
	}
}


char	mszCurrentMaterialName[32] = "";

void	ProcessMaterialInfo(FILE* pFile, ulong ulParentChunkLength, MODEL_RENDER_DATA* pxModelData )
{
ulong		ulBytesRead = 0;
ushort	uwChunkID;
ulong	ulChunkLength;
char	szFilenameBuffer[256];
ModelMaterialData*		pMaterialData;


	while (ulBytesRead < ulParentChunkLength)
	{
		 //Read the chunk header
		fread ( &uwChunkID, 2, 1, pFile );
		fread ( &ulChunkLength, 4, 1, pFile ); 
		ulBytesRead += 6;

		switch (uwChunkID)
		{
		case CHUNKID3DS_MATNAME:
			mxLoaderInfo.nNumMaterials++;
			if ( ulChunkLength - 6 < 32 )
			{
				fread( mszCurrentMaterialName, ulChunkLength - 6, 1, pFile );
			}
			else
			{
				strcpy( mszCurrentMaterialName, "NONAME" );
			}
			break;		
		case CHUNKID3DS_MATLUMINANCE:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;		
		case CHUNKID3DS_MATDIFFUSE:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;		
		case CHUNKID3DS_MATSPECULAR:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;
		case CHUNKID3DS_MATSHININESS:
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;
		case CHUNKID3DS_MATMAP:
			ProcessMaterialInfo( pFile, ulChunkLength - 6, pxModelData );
			break;		
		case CHUNKID3DS_MATMAPFILE:
			fread( szFilenameBuffer, ulChunkLength - 6, 1, pFile );
			pMaterialData = ModelAddNewMaterialData( pxModelData, szFilenameBuffer );
			if ( pMaterialData )
			{
				pMaterialData->SetMaterialName( mszCurrentMaterialName );
			}
			break;	
		default:  // unrecognized/unsupported chunk
			fseek( pFile, ulChunkLength - 6, SEEK_CUR );
			break;
		}	
		ulBytesRead += (ulChunkLength-6);
	}	
}


/***************************************************************************
 * Function    : ModelConvRead3ds
 * Params      :
 * Description : Reads the data from a 3ds file
 ***************************************************************************/
void		ModelConvRead3ds( FILE* pFile, int nFileSize, MODEL_RENDER_DATA* pxModelData )
{
CUSTOMVERTEX*	pxVertices;
ushort*			puwIndices = NULL;
ulong*			punIndices = NULL;
ushort	uwChunkID;
ulong	ulChunkLength;
ushort	uwData;
int		nLoop;
char	cChar;
ushort	uwFlags;
int		nVertBase = 0;
int		nFaceBase;
DWORD*		pxMeshAttributes = NULL;
static int	msnObjectTransformCount = 0;

	msnObjectTransformCount = 0;
	EngineMatrixIdentity( &mx3dsLoadLocalTransform );
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	pxModelData->pxBaseMesh->LockAttributeBuffer( 0, (byte**)( &pxMeshAttributes ) );

	if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
	{
		pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &punIndices ) );
	}
	else
	{
		pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
	}

	mxLoaderInfo.nNumVertices = 0;
	mxLoaderInfo.nNumFaces = 0;
	mxLoaderInfo.nNumMaterials = 0;

	//Loop to scan the whole file 
	while ( ftell(pFile) < nFileSize ) 
	{
		 //Read the chunk header
		fread ( &uwChunkID, 2, 1, pFile );
		fread ( &ulChunkLength, 4, 1, pFile ); 

		switch (uwChunkID)
        {
		case CHUNKID3DS_PRIMARY: 
			break;    
		case CHUNKID3DS_EDIT3DS:
			break;
		case CHUNKID3DS_OBJTRIMESH:
			break;
		case CHUNKID3DS_EDITMATERIAL:
			ProcessMaterialInfo(pFile, ulChunkLength, pxModelData );
			break;		
		case CHUNKID3DS_EDITOBJECT:
				nLoop = 0;
				do
				{	
					fread (&cChar, 1, 1, pFile);
					nLoop++;
				} 
				while( cChar != '\0' && nLoop < 20);
				break;
		case CHUNKID3DS_TRIFACEMAT:
			{
			char		acMaterialName[256];
			char*		pcMaterialNameRunner = acMaterialName;
			ModelMaterialData*		pMaterialData;

				nLoop = 0;
				do
				{	
					fread (&cChar, 1, 1, pFile);
					*pcMaterialNameRunner = cChar;
					pcMaterialNameRunner++;
					nLoop++;
				} 
				while( cChar != '\0' && nLoop < 20);

				// Find material by name and get its attribID
				pMaterialData = FindMaterialByMaterialName( pxModelData, acMaterialName );

				if ( pMaterialData )
				{
				int		nAttribID = pMaterialData->GetAttrib();
				short	wNumFaces;
				short	wFaceNum;

					fread (&wNumFaces, 2, 1, pFile);

					for( nLoop = 0; nLoop < wNumFaces; nLoop++ )
					{
						// read the face numbers provided next and set their attribIDs in the attributeBuffer
						fread (&wFaceNum, 2, 1, pFile);
						pxMeshAttributes[wFaceNum] = nAttribID;
					}
						
				}
				else
				{
					fseek(pFile, ulChunkLength-(6+nLoop), SEEK_CUR);
				}
			}
			break;

		//--------------- TRI_VERTEXL ---------------
		// Description: Vertices list
		// Chunk ID: 4110 (hex)
		// Chunk Lenght: 1 x unsigned short (number of vertices) 
		//             + 3 x float (vertex coordinates) x (number of vertices)
		//             + sub chunks
		//-------------------------------------------
		case CHUNKID3DS_TRIVERT: 
			fread (&uwData, sizeof(ushort), 1, pFile);
		
			nVertBase = mxLoaderInfo.nNumVertices;
            for ( nLoop = mxLoaderInfo.nNumVertices; nLoop < mxLoaderInfo.nNumVertices+uwData; nLoop++ )
            {
				fread (&pxVertices[nLoop].position.x, sizeof(float), 1, pFile );
                fread (&pxVertices[nLoop].position.y, sizeof(float), 1, pFile );
				fread (&pxVertices[nLoop].position.z, sizeof(float), 1, pFile );

				VectTransform( &pxVertices[nLoop].position, &pxVertices[nLoop].position, &mx3dsLoadLocalTransform );
				// include the default tractor scale (why oh why :] )
				pxVertices[nLoop].position.x *= 0.0035f;
				pxVertices[nLoop].position.y *= 0.0035f;
				pxVertices[nLoop].position.z *= 0.0035f;
			}
			mxLoaderInfo.nNumVertices += uwData;
			break;
		case CHUNKID3DS_TRILOCAL:
			{
			VECT	xVect;

				fread( &xVect, sizeof(float)*3, 1, pFile );
				mx3dsLoadLocalTransform._11 = xVect.x;
				mx3dsLoadLocalTransform._12 = xVect.y;
				mx3dsLoadLocalTransform._13 = xVect.z;
				fread( &xVect, sizeof(float)*3, 1, pFile );
				mx3dsLoadLocalTransform._21 = xVect.x;
				mx3dsLoadLocalTransform._22 = xVect.y;
				mx3dsLoadLocalTransform._23 = xVect.z;
				fread( &xVect, sizeof(float)*3, 1, pFile );
				mx3dsLoadLocalTransform._31 = xVect.x;
				mx3dsLoadLocalTransform._32 = xVect.y;
				mx3dsLoadLocalTransform._33 = xVect.z;
				fread( &xVect, sizeof(float)*3, 1, pFile );
				mx3dsLoadLocalTransform._41 = xVect.x;// + (msnObjectTransformCount * 5.0f);
				mx3dsLoadLocalTransform._42 = xVect.y;
				mx3dsLoadLocalTransform._43 = xVect.z;
				msnObjectTransformCount++;
			}
			break;
			//--------------- TRI_FACEL1 ----------------
			// Description: Polygons (faces) list
			// Chunk ID: 4120 (hex)
			// Chunk Lenght: 1 x unsigned short (number of polygons) 
			//             + 3 x unsigned short (polygon points) x (number of polygons)
			//             + sub chunks
			//-------------------------------------------
		case CHUNKID3DS_TRIFACE:
			fread (&uwData, sizeof(ushort), 1, pFile);
			nFaceBase = mxLoaderInfo.nNumFaces;
            for ( nLoop = mxLoaderInfo.nNumFaces; nLoop < mxLoaderInfo.nNumFaces+uwData; nLoop++ )
            {
				if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
				{
					fread( &uwData, sizeof(ushort), 1, pFile );
					punIndices[ (nLoop*3) ] = uwData;
					fread( &uwData, sizeof(ushort), 1, pFile );
					punIndices[ (nLoop*3)+1 ] = uwData;
					fread( &uwData, sizeof(ushort), 1, pFile );
					punIndices[ (nLoop*3)+2 ] = uwData;
					fread (&uwFlags, sizeof (unsigned short), 1, pFile);
					punIndices[ (nLoop*3) ] += nVertBase;
					punIndices[ (nLoop*3)+1 ] += nVertBase;
					punIndices[ (nLoop*3)+2 ] += nVertBase;
				}
				else
				{
					fread( &puwIndices[ (nLoop*3) ], sizeof(ushort), 1, pFile );
					fread( &puwIndices[ (nLoop*3)+1 ], sizeof(ushort), 1, pFile );
					fread( &puwIndices[ (nLoop*3)+2 ], sizeof(ushort), 1, pFile );
					fread (&uwFlags, sizeof (unsigned short), 1, pFile);
					puwIndices[ (nLoop*3) ] += nVertBase;
					puwIndices[ (nLoop*3)+1 ] += nVertBase;
					puwIndices[ (nLoop*3)+2 ] += nVertBase;
				}
			}
			mxLoaderInfo.nNumFaces += uwData;
            break;
		case CHUNKID3DS_TRIUV:
			fread (&uwData, sizeof (ushort), 1, pFile);
			if ( uwData == mxLoaderInfo.nNumVertices - nVertBase )
			{
				for ( nLoop=0; nLoop < uwData; nLoop++)
				{			
					fread (&pxVertices[nLoop+nVertBase].tu, sizeof (float), 1, pFile);
					fread (&pxVertices[nLoop+nVertBase].tv, sizeof (float), 1, pFile);
//					pxVertices[nLoop+nVertBase].tu = 1.0f - pxVertices[nLoop+nVertBase].tu;
					pxVertices[nLoop+nVertBase].tv = 1.0f - pxVertices[nLoop+nVertBase].tv;
				}
			}
			else
			{
				fseek(pFile, ulChunkLength-6-2, SEEK_CUR);
			}
			break;
		default:
#ifdef LOAD_LOGGING_3DS
			FILE*	pFile2 = fopen( "load.log", "ab" );
			if ( pFile2 != NULL )
			{
			char	acString[256];
				sprintf( acString, "ignored chunk %x - len %d\r\n", uwChunkID, (int)ulChunkLength );
				fwrite( acString, strlen(acString), 1, pFile2 );
				fclose( pFile2 );
			}
#endif			
			 fseek(pFile, ulChunkLength-6, SEEK_CUR);
			 break;
        } 
	}

	RenderingComputeBoundingBox( pxVertices, pxModelData->xStats.nNumVertices,&pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
	pxModelData->pxBaseMesh->UnlockAttributeBuffer();
	pxModelData->pxBaseMesh->UnlockVertexBuffer( );
	pxModelData->pxBaseMesh->UnlockIndexBuffer( );

	if ( pxModelData->xStats.nNumMaterials > 1 )
	{
		pxModelData->pxBaseMesh->ReorderByMaterial( NULL, NULL );
	}
}

 
#ifdef IW_SDK
// TEMP stub
void	ModelConvInitialiseBlankModel( MODEL_RENDER_DATA* pxModelData, int nNumVerts, int nNumFaces )
{


}
#endif
int		ModelLoad3ds( MODEL_RENDER_DATA* pxModelData, const char* szFilename )
{
FILE*	pFile;
int		nFileSize;

	pFile = fopen( szFilename, "rb" );
	if ( pFile != NULL )
	{
		nFileSize = SysGetFileSize(pFile);

		// Scan through the 3ds to get all the creation info we need (num faces, vertices, materials )
		ModelConvGetRequiredInfo( pFile, nFileSize );

		ModelConvInitialiseBlankModel( pxModelData,  mxLoaderInfo.nNumVertices,  mxLoaderInfo.nNumFaces );
		
		rewind( pFile );
		ModelConvRead3ds( pFile, nFileSize, pxModelData );
		fclose( pFile );
		ModelLoaderGenerateNormals( pxModelData );
		return( 1 );
	}
	return( 0 );
}
