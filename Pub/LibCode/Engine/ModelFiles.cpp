
#include <stdio.h>
#include <time.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>
 
#ifndef CONSOLE_SERVER

#include <Interface.h>
#include "../LibCode/CodeUtil/Archive.h"	

#endif // ndef CONSOLE_SERVER

#include <System.h>
#include <zlib.h>

#ifndef STANDALONE
#include "UnivObfuscation.h"
#endif

#ifdef SERVER
#include "../GameCode/Server/Server.h"
#include "../GameCode/Server/Files/ServerRegistration.h"
#endif


#include "ModelFiles.h"
#include "BSPRender/BSPModel.h"
#include "ModelArchive.h"
#include "ModelMaterialData.h"
#include "SkinnedModel.h"

#ifdef ENGINEDX
#ifndef CONSOLE_SERVER
#include "DirectX/ModelRenderingDX.h"
#endif 
#endif

#include "FileFormats/3dsLoader.h"
#include "FileFormats/fbxLoader.h"
#include "FileFormats/ObjLoader.h"


//----------------------------------------------------------------------------------------------------

typedef struct
{
	int		nHorizAttachVertex;
	VECT	xHorizAttachOffset;

	int		nVertAttachVertex;
	VECT	xVertAttachOffset;

	int		nWheel1AttachVertex;
	VECT	xWheel1AttachOffset;
	int		nWheel2AttachVertex;
	VECT	xWheel2AttachOffset;
	int		nWheel3AttachVertex;
	VECT	xWheel3AttachOffset;
	int		nWheel4AttachVertex;
	VECT	xWheel4AttachOffset;

} TURRET_DATA_CHUNK;

typedef struct
{
	int		nEffectAttachVertex;
	int		nEffectType;
	uint32	ulEffectParam1;
	uint32	ulEffectParam2;
	VECT	xEffectAttachOffset;

} EFFECT_DATA_CHUNK;

typedef struct
{
	float	fHiDist;
	float	fMedDist;
	float	fLowDist;
	BYTE	bPad1;
	BYTE	bPad2;
	BYTE	bPad3;
	BYTE	bPad4;

} LOD_DATA_CHUNK;

typedef struct
{
	BYTE	bAttachType;
	BYTE	bPad1;
	BYTE	bPad2;
	BYTE	bPad3;

	int		nAttachVertex;
	int		nOrientationVertex;

	VECT	xAttachOffset;

} ATTACH_DATA_CHUNK;

typedef struct
{
	BYTE	bTotalNumMaterials;
	BYTE	bSizeOfHeader;
	BYTE	bPad3;
	BYTE	bPad4;

} TEXMATERIAL_CHUNKHEADER;


typedef struct
{
	MATERIAL_COLOUR		diffuse;
	MATERIAL_COLOUR		specular;
	MATERIAL_COLOUR		emissive;
	MATERIAL_COLOUR		ambient;

	BYTE	bAttrib;
	BYTE	bNumTextures;
	BYTE	bBlendType;
	BYTE	bFlags;

	int		nSizeOfTextureFilename;
	int		nSizeOfTextureData;

	float	fSpecularPower;
	int		nSizeOfNormalMapTextureData;
	
	BYTE	bTex1Clone;
	BYTE	bTex2Clone;
	BYTE	bSizeOfNormalMapTextureFilename;
	BYTE	bSizeOfSpecularMapTextureFilename;

	int		nSizeOfSpecularMapTextureData;

} TEXMATERIAL_CHUNK;


typedef struct
{
	uint32	ulLockID;
	BYTE	bLockMode;
	BYTE	bPad1;
	BYTE	bPad2;
	BYTE	bPad3;

} CREATOR_INFO_CHUNK;


typedef struct
{
	ushort	uwSizeOfHeader;
	ushort	uwNumKeyframes;
	int		nNumVerts;

	VECT	xMin;
	VECT	xMax;

} COMPRESSED_VERTS_HEADER;

typedef struct
{
	ushort	uwX;
	ushort	uwY;
	ushort	uwZ;
} SHORT_VERT;

//----------------------------------------------------------------------------------------------------

uint32		mulExportFlags = 0;
uint32		mulExportLockFlags = 0;

//----------------------------------------------------------------------------------------------------

/***************************************************************************
 * Function    : ModelGetLockState
 * Params      :
 * Description : Opens the model file and checks the determines the lock status
 ***************************************************************************/
int		ModelGetLockState( const char* szFilename )
{
FILE*	pFile;
ATM_FILE_HEADER		xFileHeader;
ATM_CHUNK_HEADER	xChunkHeader;
ATM_OLD_BASIC_CONTENTS	xContentsChunk;

	pFile = SysFileOpen( szFilename, "rb" );
	if ( pFile != NULL )
	{
		SysFileRead( (unsigned char*)&xFileHeader, 8, 1, pFile );
		if ( xFileHeader.uwFileHeader == ATM_CHUNK_FILE_HEADER_CODE )
		{
			SysFileSeek( pFile, xFileHeader.bSizeOfFileHeader - 8, 0 );

			SysFileRead( (unsigned char*)&xChunkHeader, xFileHeader.bSizeOfChunkHeader, 1, pFile );
			if ( ( xChunkHeader.bChunkCode == ATM_CHUNK_HEADER_CODE ) &&
				 ( xChunkHeader.uwChunkID == ATM_CHUNKID_OLD_BASIC_CONTENTS ) )
			{
				SysFileRead( (unsigned char*)&xContentsChunk, sizeof( ATM_OLD_BASIC_CONTENTS ), 1, pFile );
				SysFileClose( pFile );
				if ( xContentsChunk.ulLockID == 0xCDCDCDCD )
				{
					return( 0 );
				}

				if ( xContentsChunk.ulLockID != 0 )
				{
#ifdef SERVER
					if ( xContentsChunk.ulLockID != (uint32)(RegistrationGetServerID()) )
					{
#else
#ifndef TOOL
 					if ( xContentsChunk.ulLockID != (uint32)(EngineGetGameInterface()->GetUniqueMachineID()) )
					{
#else
					if ( 0 )
					{
#endif
#endif
						return( 2 );
					}
				}
				else if ( xContentsChunk.ulLockCode != 0 )
				{
					return( 1 );
				}
			}
			else
			{
				SysFileClose( pFile );
			}
		}
		else
		{
			SysFileClose( pFile );
		}
	}

	return( 0 );

}

#ifndef CONSOLE_SERVER

#ifndef SERVER
/***************************************************************************
 * Function    : ModelExportChunkVerts
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkVerts( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
CUSTOMVERTEX*	pxVertices;
int				nVertsInMesh;
int				nVertLoop;
VECT*	pxVectBuffer;
byte*	pbChunkMem;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();

		pbChunkMem = (byte*)( SystemMalloc( nVertsInMesh * sizeof(VECT) ) );
		pxVectBuffer = (VECT*)( pbChunkMem );

		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			pxVectBuffer->x = pxVertices->position.x;
			pxVectBuffer->y = pxVertices->position.y;
			pxVectBuffer->z = pxVertices->position.z;
			pxVertices++;
			pxVectBuffer++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();

		*(pnSize) = sizeof( VECT );
		*(pnElements) = nVertsInMesh;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}


/***************************************************************************
 * Function    : ModelExportChunkCompressedVerts
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkCompressedVerts( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
CUSTOMVERTEX*	pxVertices;
int				nVertsInMesh;
byte*	pbChunkMem;
int		nMemSize;
COMPRESSED_VERTS_HEADER*	pxHeader;
SHORT_VERT*		pxCompressedVerts;
VECT	xRange;
VECT	xCompressed;
int		nLoop;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();

		nMemSize = sizeof(COMPRESSED_VERTS_HEADER) + (nVertsInMesh * sizeof(SHORT_VERT) );
		pbChunkMem = (byte*)( SystemMalloc( nMemSize ) );
		pxHeader = (COMPRESSED_VERTS_HEADER*)( pbChunkMem );
		pxCompressedVerts = (SHORT_VERT*)( pxHeader + 1 );

		pxHeader->uwNumKeyframes = 1;
		pxHeader->uwSizeOfHeader = sizeof( COMPRESSED_VERTS_HEADER );
		pxHeader->nNumVerts = pxModelData->xStats.nNumVertices;

		nVertsInMesh = pxModelData->xStats.nNumVertices;

		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		// First get the model's min and max bounding boxes 
		RenderingComputeBoundingBox( pxVertices, nVertsInMesh,&pxHeader->xMin, &pxHeader->xMax );
		xRange.x = pxHeader->xMax.x - pxHeader->xMin.x;
		xRange.y = pxHeader->xMax.y - pxHeader->xMin.y;
		xRange.z = pxHeader->xMax.z - pxHeader->xMin.z;

		for( nLoop = 0; nLoop < nVertsInMesh; nLoop++ )
		{
			xCompressed.x = pxVertices->position.x - pxHeader->xMin.x;
			xCompressed.y = pxVertices->position.y - pxHeader->xMin.y;
			xCompressed.z = pxVertices->position.z - pxHeader->xMin.z;
			if ( xRange.x == 0.0f )
			{
				xCompressed.x = 0.0f;
			}
			else
			{
				xCompressed.x = ( xCompressed.x / xRange.x ) * 65535.0f;
			}
			if ( xRange.y == 0.0f )
			{
				xCompressed.y = 0.0f;
			}
			else
			{
				xCompressed.y = ( xCompressed.y / xRange.y ) * 65535.0f;
			}
			if ( xRange.z == 0.0f )
			{
				xCompressed.z = 0.0f;
			}
			else
			{
				xCompressed.z = ( xCompressed.z / xRange.z ) * 65535.0f;
			}
			pxCompressedVerts->uwX = (ushort)( xCompressed.x );
			pxCompressedVerts->uwY = (ushort)( xCompressed.y );
			pxCompressedVerts->uwZ = (ushort)( xCompressed.z );
			pxCompressedVerts++;
			pxVertices++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();

		*(pnSize) = nMemSize;
		*(pnElements) = 1;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}

/***************************************************************************
 * Function    : ModelExportChunkColours
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkColours( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
CUSTOMVERTEX*	pxVertices;
int				nVertsInMesh;
int				nVertLoop;
uint32*	pulColBuffer;
byte*	pbChunkMem;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();

		pbChunkMem = (byte*)( SystemMalloc( nVertsInMesh * sizeof(uint32) ) );
		pulColBuffer = (uint32*)( pbChunkMem );

		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			*(pulColBuffer) = pxVertices->color;
			pxVertices++;
			pulColBuffer++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();

		*(pnSize) = sizeof( uint32 );
		*(pnElements) = nVertsInMesh;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}

/***************************************************************************
 * Function    : ModelExportChunkNormals
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkNormals( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
CUSTOMVERTEX*	pxVertices;
int				nVertsInMesh;
int				nVertLoop;
VECT*	pxVectBuffer;
byte*	pbChunkMem;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();

		pbChunkMem = (byte*)( SystemMalloc( nVertsInMesh * sizeof(VECT) ) );
		pxVectBuffer = (VECT*)( pbChunkMem );

		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			pxVectBuffer->x = pxVertices->normal.x;
			pxVectBuffer->y = pxVertices->normal.y;
			pxVectBuffer->z = pxVertices->normal.z;
			pxVertices++;
			pxVectBuffer++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();

		*(pnSize) = sizeof( VECT );
		*(pnElements) = nVertsInMesh;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}


/***************************************************************************
 * Function    : ModelExportChunkIndices
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkIndices( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
ushort*			puwIndices;
unsigned int*	punIndices;
int				nFacesInMesh;
int				nNumIndices;
int				nVertLoop;
ushort*			puwIndexBuffer;
unsigned int*	punIndexBuffer;
byte*			pbChunkMem;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nFacesInMesh = pxModelData->pxBaseMesh->GetNumFaces();
		nNumIndices = nFacesInMesh * 3;

		if ( pxModelData->xStats.nNumVertices < 65536 )
		{
			pbChunkMem = (byte*)( SystemMalloc( nNumIndices * sizeof(ushort) ) );
			puwIndexBuffer = (ushort*)( pbChunkMem );

			pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
			for ( nVertLoop = 0; nVertLoop < nNumIndices; nVertLoop++ )
			{
				*(puwIndexBuffer++) = *(puwIndices++);
			}
			pxModelData->pxBaseMesh->UnlockIndexBuffer();

			*(pnSize) = sizeof( ushort ) * 3;
			*(pnElements) = nFacesInMesh;
		}
		else
		{
			pbChunkMem = (byte*)( SystemMalloc( nNumIndices * sizeof(unsigned int) ) );
			punIndexBuffer = (unsigned int*)( pbChunkMem );

			if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
			{
				pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &punIndices ) );
				for ( nVertLoop = 0; nVertLoop < nNumIndices; nVertLoop++ )
				{
					*(punIndexBuffer++) = *(punIndices++);
				}
				pxModelData->pxBaseMesh->UnlockIndexBuffer();
			}
			else
			{
			ushort*		puwIndices;

				pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
				for ( nVertLoop = 0; nVertLoop < nNumIndices; nVertLoop++ )
				{
					*(punIndexBuffer++) = (uint32)( *(puwIndices++) );
				}
				pxModelData->pxBaseMesh->UnlockIndexBuffer();
			}

			*(pnSize) = sizeof( unsigned int ) * 3;
			*(pnElements) = nFacesInMesh;
		}

		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}

/***************************************************************************
 * Function    : ModelExportCompressedTexCoords
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkCompressedTexCoords( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
CUSTOMVERTEX*	pxVertices;
int				nVertsInMesh;
int				nVertLoop;
byte*	pbBuffer;
byte*	pbChunkMem;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();

		pbChunkMem = (byte*)( SystemMalloc( nVertsInMesh * sizeof(byte) * 2 ) );
		pbBuffer = (byte*)( pbChunkMem );

		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			pbBuffer[0] = (byte)(pxVertices->tu * 255.0f);
			pbBuffer[1] = (byte)(pxVertices->tv * 255.0f);
			pxVertices++;
			pbBuffer += 2;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();

		*(pnSize) = sizeof( byte ) * 2;
		*(pnElements) = nVertsInMesh;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}

/***************************************************************************
 * Function    : ModelExportChunkTexCoords
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkTexCoords( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
CUSTOMVERTEX*	pxVertices;
int				nVertsInMesh;
int				nVertLoop;
float*	pfUVBuffer;
byte*	pbChunkMem;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();

		pbChunkMem = (byte*)( SystemMalloc( nVertsInMesh * sizeof(float) * 2 ) );
		pfUVBuffer = (float*)( pbChunkMem );

		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		for ( nVertLoop = 0; nVertLoop < nVertsInMesh; nVertLoop++ )
		{
			pfUVBuffer[0] = pxVertices->tu;
			pfUVBuffer[1] = pxVertices->tv;
			pxVertices++;
			pfUVBuffer += 2;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();

		*(pnSize) = sizeof( float ) * 2;
		*(pnElements) = nVertsInMesh;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}



/***************************************************************************
 * Function    : ModelExportChunkTexMaterials
 * Params      :
 * Description : Export the tex material chunk which contains the details of each material (lighting properties etc)
 *               and (when appropriate) any embedded texture data
 ***************************************************************************/
byte*		ModelExportChunkTexMaterials( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
ModelMaterialData*			pModelMaterialData;
TEXMATERIAL_CHUNKHEADER*	pHeader;
TEXMATERIAL_CHUNK*			pChunk;
byte*		pbChunkMem;
byte*		pbChunkRunner;
int			nTotalSize = 0;
int			nNumMaterials = pxModelData->xStats.nNumMaterials;
int			loop;
const char*		szFilename;
FILE*		pFile;
int			nFileSize = 0;
int			nTextSize;
int			texchanloop;
int			texchancount = 0;

	nTotalSize = nNumMaterials * sizeof( TEXMATERIAL_CHUNK );
	nTotalSize += sizeof( TEXMATERIAL_CHUNKHEADER );

	// Now find all the textures to be included, get their filesize and filename size and add em to the totalsize
	for ( loop = 0; loop < nNumMaterials; loop++ )
	{
		pModelMaterialData = FindMaterial( pxModelData, loop );
		if ( pModelMaterialData )
		{
			for ( texchanloop = 0; texchanloop < MAX_NUM_TEX_CHANNELS; texchanloop++ )
			{
				if ( pModelMaterialData->IsEmbeddedTexture(texchanloop) )
				{
				byte*	pbSourceData;

					pbSourceData = pModelMaterialData->GetSourceData( texchanloop, &nFileSize );
					if ( pbSourceData )
					{
						szFilename = pModelMaterialData->GetFilename(texchanloop);
						if ( szFilename )
						{
							nTextSize = strlen( szFilename ) + 1;

							nTotalSize += nTextSize + nFileSize;
							texchancount++;
						}
						else		// no filename?
						{
							PANIC_IF( TRUE, "Missing texture filename" );
						}
					}
					else
					{
						PANIC_IF( TRUE, "Missing texture/material sourcedata" );
					}
				}
				else
				{
					szFilename = pModelMaterialData->GetFilename(texchanloop);
					if ( szFilename )
					{
						pFile = SysFileOpen( szFilename, "rb" );
						if ( pFile )
						{
							nFileSize = SysGetFileSize( pFile );					
							SysFileClose( pFile );
							nTextSize = strlen( szFilename ) + 1;

							nTotalSize += nTextSize + nFileSize;
							texchancount++;
						}
						else	// error opening texture file?
						{
							SysDebugPrint( "WARNING: Missing texture file %s", szFilename );
						}
					}
					else	// No filename?
					{
						// a non-textured colour material?
					}
				}
			}
		}
	}

	pbChunkMem = (byte*)( SystemMalloc( nTotalSize ) );
	pbChunkRunner = pbChunkMem;

	pHeader = (TEXMATERIAL_CHUNKHEADER*)pbChunkRunner;
	pHeader->bTotalNumMaterials = (BYTE)nNumMaterials;
	pHeader->bSizeOfHeader = sizeof( TEXMATERIAL_CHUNKHEADER );
	pbChunkRunner += sizeof( TEXMATERIAL_CHUNKHEADER );

#ifdef TOOL
	ModelMaterialOptimiseTextureClones( pxModelData );
#endif

	for ( loop = 0; loop < nNumMaterials; loop++ )
	{
		pChunk = (TEXMATERIAL_CHUNK*)pbChunkRunner;
		nTextSize = 0;
		nFileSize = 0;
		memset( pChunk, 0, sizeof( TEXMATERIAL_CHUNK ) );
		pbChunkRunner += sizeof( TEXMATERIAL_CHUNK );
		pModelMaterialData = FindMaterial( pxModelData, loop );
		if ( pModelMaterialData )
		{
			pChunk->diffuse = pModelMaterialData->GetColour( ModelMaterialData::DIFFUSE );
			pChunk->ambient = pModelMaterialData->GetColour( ModelMaterialData::AMBIENT );
			pChunk->specular = pModelMaterialData->GetColour( ModelMaterialData::SPECULAR );
			pChunk->emissive = pModelMaterialData->GetColour( ModelMaterialData::EMISSIVE );
			pChunk->fSpecularPower = pModelMaterialData->GetSpecularPower();
			pChunk->bBlendType = (BYTE)pModelMaterialData->GetBlendType();
			pChunk->bFlags = pModelMaterialData->HasActiveMaterialProperties();
			pChunk->bNumTextures = texchancount;

			for ( texchanloop = 0; texchanloop < MAX_NUM_TEX_CHANNELS; texchanloop++ )
			{
				if ( pModelMaterialData->GetCloneTextureMaterialIndex( texchanloop ) >= 0 )
				{
					// if this material uses a clone of another texture
					pChunk->bTex1Clone = (byte)pModelMaterialData->GetCloneTextureMaterialIndex( texchanloop ) + 1;
					pChunk->nSizeOfTextureFilename = 0;
					pChunk->nSizeOfTextureData = 0;
				}
				else if ( pModelMaterialData->IsEmbeddedTexture(texchanloop) )
				{
				byte*	pbSourceData;

					pbSourceData = pModelMaterialData->GetSourceData( texchanloop, &nFileSize );
					if ( pbSourceData )
					{
						szFilename = pModelMaterialData->GetFilename(texchanloop);
						if ( szFilename )
						{
							nTextSize = strlen( szFilename ) + 1;

							switch( texchanloop )
							{
							case 0:
								pChunk->nSizeOfTextureFilename = nTextSize;
								pChunk->nSizeOfTextureData = nFileSize;
								break;
							case 1:
								pChunk->bSizeOfNormalMapTextureFilename = nTextSize;
								pChunk->nSizeOfNormalMapTextureData = nFileSize;
								break;
							case 2:
								pChunk->bSizeOfSpecularMapTextureFilename = nTextSize;
								pChunk->nSizeOfSpecularMapTextureData = nFileSize;
								break;
							}

							strcpy( (char*)pbChunkRunner, szFilename );
							pbChunkRunner += nTextSize;
							memcpy( pbChunkRunner, pbSourceData, nFileSize );
							pbChunkRunner += nFileSize;
						}
						else		// no filename?
						{
							PANIC_IF( TRUE, "Missing texture filename" );
						}
					}
					else
					{
						PANIC_IF( TRUE, "Missing texture/material sourcedata" );
					}
				}
				else
				{
					szFilename = pModelMaterialData->GetFilename(texchanloop);
					if ( szFilename )
					{
						pFile = SysFileOpen( szFilename, "rb" );
						if ( pFile )
						{
							nTextSize = strlen( szFilename ) + 1;
							nFileSize = SysGetFileSize( pFile );					

							switch( texchanloop )
							{
							case 0:
								pChunk->nSizeOfTextureFilename = nTextSize;
								pChunk->nSizeOfTextureData = nFileSize;
								break;
							case 1:
								pChunk->bSizeOfNormalMapTextureFilename = nTextSize;
								pChunk->nSizeOfNormalMapTextureData = nFileSize;
								break;
							case 2:
								pChunk->bSizeOfSpecularMapTextureFilename = nTextSize;
								pChunk->nSizeOfSpecularMapTextureData = nFileSize;
								break;
							}
							strcpy( (char*)pbChunkRunner, szFilename );
							pbChunkRunner += nTextSize;
							SysFileRead( pbChunkRunner, nFileSize, 1, pFile );
							SysFileClose( pFile );
							pbChunkRunner += nFileSize;
						}
						else	// error opening texture file?
						{
							switch( texchanloop )
							{
							case 0:
								pChunk->nSizeOfTextureFilename = 0;
								pChunk->nSizeOfTextureData = 0;
								break;
							case 1:
								pChunk->bSizeOfNormalMapTextureFilename = 0;
								pChunk->nSizeOfNormalMapTextureData = 0;
								break;
							case 2:
								pChunk->bSizeOfSpecularMapTextureFilename = 0;
								pChunk->nSizeOfSpecularMapTextureData = 0;
								break;
							}
						}
					}
					else	// No filename?
					{
						// a non-textured colour material?
						switch( texchanloop )
						{
						case 0:
							pChunk->nSizeOfTextureFilename = 0;
							pChunk->nSizeOfTextureData = 0;
							break;
						case 1:
							pChunk->bSizeOfNormalMapTextureFilename = 0;
							pChunk->nSizeOfNormalMapTextureData = 0;
							break;
						case 2:
							pChunk->bSizeOfSpecularMapTextureFilename = 0;
							pChunk->nSizeOfSpecularMapTextureData = 0;
							break;
						}
					}
				}
			}
		}
	}

	*(pnSize) = nTotalSize;
	*(pnElements) = 1;
	return( pbChunkMem );
}

/*

typedef struct
{
	float	afMaterialColours[16];		// placeholder
	BYTE	bPad1;
	BYTE	bPad2;
	BYTE	bPad3;
	BYTE	bPad4;

	int		nSizeOfTextureFilename;
	int		nSizeOfTextureData;

} TEXMATERIAL_CHUNK;
*/

// TODO
	// Save out all the material data.. based on..
	// pxModelData->xStats.nNumMaterials
	// Using
	// ModelMaterialData FindMaterial( attrib) for each material
	//
	// then ... get the image data (either via filename or memory) and add it to this chunk
	// along with a standard material struct


/***************************************************************************
 * Function    : ModelExportChunkAttributes
 * Params      :
 * Description : THe attribute data is a per face list of material assignments
 ***************************************************************************/
byte*		ModelExportChunkAttributes( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
DWORD*		pxMeshAttributes = NULL;
byte*		pbChunkMem;
byte*		pxATMAttribs;
int			nFaceLoop;
int			nNumFaces;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nNumFaces = (pxModelData->xStats.nNumIndices/3);

		pbChunkMem = (byte*)( SystemMalloc( nNumFaces * sizeof(byte) ) );
		pxATMAttribs = ( pbChunkMem );

		pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_ReadOnly, (BYTE**)&pxMeshAttributes );

		if ( pxMeshAttributes )
		{
			for ( nFaceLoop = 0; nFaceLoop < nNumFaces; nFaceLoop++ )
			{
				*pxATMAttribs = (byte)(*pxMeshAttributes);
				pxMeshAttributes++;
				pxATMAttribs++;
			}
			pxModelData->pxBaseMesh->UnlockAttributeBuffer();
		}
		*(pnSize) = sizeof( byte );
		*(pnElements) = nNumFaces;
		return( pbChunkMem );
	}
	return( NULL );
}

/***************************************************************************
 * Function    : ModelExportChunkAttachData
 * Params      :
 * Description : 
 ***************************************************************************/
byte*		ModelExportChunkAttachData( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
ATTACH_DATA_CHUNK*	pxAttachChunk;
byte*	pbChunkMem = NULL;
int		nNumAttaches = 0;

	if ( pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex != 0 )
	{
		nNumAttaches++;
	}

	if ( nNumAttaches > 0 )
	{
		pbChunkMem = (byte*)( SystemMalloc( sizeof(ATTACH_DATA_CHUNK) ) ); 
		pxAttachChunk = (ATTACH_DATA_CHUNK*)( pbChunkMem );
		pxAttachChunk->nAttachVertex = pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex;
		pxAttachChunk->bAttachType = 0;
		pxAttachChunk->nOrientationVertex = 0;
		pxAttachChunk->xAttachOffset = pxModelData->xAttachData.xGenericWeaponFireAttach.xAttachOffset;
	}

	*(pnSize) = sizeof( ATTACH_DATA_CHUNK );
	*(pnElements) = nNumAttaches;
	return( pbChunkMem );
}

/***************************************************************************
 * Function    : ModelExportChunkLODData
 * Params      :
 * Description : 
 ***************************************************************************/
byte*		ModelExportChunkLODData( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
LOD_DATA_CHUNK*	pxLOD;
byte*	pbChunkMem;

	pbChunkMem = (byte*)( SystemMalloc( sizeof(LOD_DATA_CHUNK) ) ); 
	pxLOD = (LOD_DATA_CHUNK*)( pbChunkMem );
	
	pxLOD->fHiDist = pxModelData->xLodData.fHighDist;
	pxLOD->fMedDist = pxModelData->xLodData.fMedDist;
	
	*(pnSize) = sizeof( LOD_DATA_CHUNK );
	*(pnElements) = 1;
	return( pbChunkMem );
}


/***************************************************************************
 * Function    : ModelExportChunkTurretData
 * Params      :
 * Description : 
 ***************************************************************************/
byte*		ModelExportChunkEffectData( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
EFFECT_DATA_CHUNK*	pxTurret;
byte*	pbChunkMem;

	pbChunkMem = (byte*)( SystemMalloc( sizeof(EFFECT_DATA_CHUNK) ) ); 
	pxTurret = (EFFECT_DATA_CHUNK*)( pbChunkMem );
	
	pxTurret->nEffectAttachVertex = pxModelData->xEffectAttachData.nAttachVertex;;
	pxTurret->xEffectAttachOffset = pxModelData->xEffectAttachData.xAttachOffset;
	pxTurret->nEffectType = pxModelData->xEffectAttachData.nEffectType;
	pxTurret->ulEffectParam1 = pxModelData->xEffectAttachData.ulEffectParam1;
	pxTurret->ulEffectParam2 = pxModelData->xEffectAttachData.ulEffectParam2;
		
	*(pnSize) = sizeof( EFFECT_DATA_CHUNK );
	*(pnElements) = 1;
	return( pbChunkMem );
}


/***************************************************************************
 * Function    : ModelExportChunkTurretData
 * Params      :
 * Description : 
 ***************************************************************************/
byte*		ModelExportChunkTurretData( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
TURRET_DATA_CHUNK*	pxTurret;
byte*	pbChunkMem;

	pbChunkMem = (byte*)( SystemMalloc( sizeof(TURRET_DATA_CHUNK) ) ); 
	pxTurret = (TURRET_DATA_CHUNK*)( pbChunkMem );
	
	pxTurret->nHorizAttachVertex = pxModelData->xHorizTurretData.nAttachVertex;
	pxTurret->xHorizAttachOffset = pxModelData->xHorizTurretData.xAttachOffset;

	pxTurret->nVertAttachVertex = pxModelData->xVertTurretData.nAttachVertex;
	pxTurret->xVertAttachOffset = pxModelData->xVertTurretData.xAttachOffset;
	
	pxTurret->nWheel1AttachVertex = pxModelData->xWheel1AttachData.nAttachVertex;
	pxTurret->xWheel1AttachOffset = pxModelData->xWheel1AttachData.xAttachOffset;
	pxTurret->nWheel2AttachVertex = pxModelData->xWheel2AttachData.nAttachVertex;
	pxTurret->xWheel2AttachOffset = pxModelData->xWheel2AttachData.xAttachOffset;
	pxTurret->nWheel3AttachVertex = pxModelData->xWheel3AttachData.nAttachVertex;
	pxTurret->xWheel3AttachOffset = pxModelData->xWheel3AttachData.xAttachOffset;
	pxTurret->nWheel4AttachVertex = pxModelData->xWheel4AttachData.nAttachVertex;
	pxTurret->xWheel4AttachOffset = pxModelData->xWheel4AttachData.xAttachOffset;
	
	*(pnSize) = sizeof( TURRET_DATA_CHUNK );
	*(pnElements) = 1;
	return( pbChunkMem );
}

/***************************************************************************
 * Function    : ModelExportChunkBasicContents
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkBasicContents( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
int				nVertsInMesh;
int				nFacesInMesh;
ATM_BASIC_CONTENTS*	pxContents;
byte*	pbChunkMem;
uint32	ulCurrentTime;
time_t	xtimet;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		nVertsInMesh = pxModelData->pxBaseMesh->GetNumVertices();
		nFacesInMesh = pxModelData->pxBaseMesh->GetNumFaces();

		pbChunkMem = (byte*)( SystemMalloc( sizeof(ATM_BASIC_CONTENTS) ) ); 

		pxContents = (ATM_BASIC_CONTENTS*)( pbChunkMem );

		ZeroMemory( pxContents, sizeof( ATM_BASIC_CONTENTS ) );
		pxContents->bModelType = pxModelData->bModelType;
		pxContents->nNumVertices = nVertsInMesh;
		pxContents->nNumFaces = nFacesInMesh;
		pxContents->nNumNormals = nVertsInMesh;
		pxContents->bWheelDisplayMode = pxModelData->bWheelDisplayMode;
		if ( pxModelData->xStats.nNumMaterials < 1 ) pxModelData->xStats.nNumMaterials = 1;
		pxContents->bNumMaterials = (byte)pxModelData->xStats.nNumMaterials;
		if ( mulExportLockFlags == 1 )
		{
			pxContents->ulLockID = pxModelData->xStats.ulLockID;
			pxContents->ulLockCode = pxModelData->xStats.ulLockCode;
		}

		time( &xtimet );
		ulCurrentTime = *( (uint32*)(&xtimet) );
		ulCurrentTime &= 0xFFFF;
		ulCurrentTime |= ( ( rand() & 0xFFFF ) << 16 );

		pxContents->ulUniqueModelID = ulCurrentTime;
		*(pnSize) = sizeof( ATM_BASIC_CONTENTS );
		*(pnElements) = 1;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}



/***************************************************************************
 * Function    : ModelExportFindAnimationExtents
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
void	ModelExportFindAnimationExtents( MODEL_RENDER_DATA* pxModelData, VECT* pxMin, VECT* pxMax )
{
VECT*	pxVertKeyFrame;
VECT	xMin = { 0.0f, 0.0f, 0.0f };
VECT	xMax = { 0.0f, 0.0f, 0.0f };
int		nLoop;
int		nVertsTotal;

	if ( pxModelData->pxVertexKeyframes != NULL )
	{
		pxVertKeyFrame = pxModelData->pxVertexKeyframes;
		xMin = pxVertKeyFrame[0];
		xMax = pxVertKeyFrame[0];

		nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;

		for( nLoop = 0; nLoop < nVertsTotal; nLoop++ )
		{
			if ( pxVertKeyFrame->x < xMin.x )
			{
				xMin.x = pxVertKeyFrame->x;
			}
			else if ( pxVertKeyFrame->x > xMax.x )
			{
				xMax.x = pxVertKeyFrame->x;
			}
			if ( pxVertKeyFrame->y < xMin.y )
			{
				xMin.y = pxVertKeyFrame->y;
			}
			else if ( pxVertKeyFrame->y > xMax.y )
			{
				xMax.y = pxVertKeyFrame->y;
			}
			if ( pxVertKeyFrame->z < xMin.z )
			{
				xMin.z = pxVertKeyFrame->z;
			}
			else if ( pxVertKeyFrame->z > xMax.z )
			{
				xMax.z = pxVertKeyFrame->z;
			}
			pxVertKeyFrame++;
		}

	}

	*(pxMin) = xMin;
	*(pxMax) = xMax;
}



void ModelProcessCopyVertexData( CUSTOMVERTEX* pxVert1, CUSTOMVERTEX* pxVert2 )
{
	*(pxVert1) = *(pxVert2);
}

void ModelProcessCopyKeyFrameTrack( VECT* pxDest, VECT* pxSrc, int nNumKeyframes )
{
int		nLoop;

	for ( nLoop = 0; nLoop < nNumKeyframes; nLoop++ )
	{
		*(pxDest++) = *(pxSrc++);
	}

}

BOOL ModelProcessAreTracksIdentical( VECT* pxVerts1, VECT* pxVerts2, int nNumKeyframes )
{
int		nLoop;

	for ( nLoop = 0; nLoop < nNumKeyframes; nLoop++ )
	{
		if ( ( pxVerts1[nLoop].x != pxVerts2[nLoop].x ) ||
			 ( pxVerts1[nLoop].y != pxVerts2[nLoop].y ) ||
			 ( pxVerts1[nLoop].z != pxVerts2[nLoop].z ) )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}

BOOL ModelProcessIsVertexDataIdentical( CUSTOMVERTEX* pxVert1, CUSTOMVERTEX* pxVert2 )
{
	if ( ( pxVert1->normal.x != pxVert2->normal.x ) ||
		 ( pxVert1->normal.y != pxVert2->normal.y ) ||
		 ( pxVert1->normal.z != pxVert2->normal.z ) ||
		 ( pxVert1->tu != pxVert2->tu ) ||
		 ( pxVert1->tv != pxVert2->tv ) ||
		 ( pxVert1->color != pxVert2->color ) )
	{
		return( FALSE );
	}
	return( TRUE );
}


void	ModelExportValidate( MODEL_RENDER_DATA* pxModelData )
{



}


/***************************************************************************
 * Function    : ModelExportRemoveIdenticalVertices
 * Params      :
 * Description : Creates a new base mesh (replacing the old one) with any identical vertices removed
                 and the index buffer remapped to match
 ***************************************************************************/
void		ModelExportRemoveIdenticalVertices( MODEL_RENDER_DATA* pxModelData )
{
CUSTOMVERTEX*	pxVertices;
unsigned int*	pnVertIndexList;
int			nProcessVert;
int			nUnrepeatedVertCount = 0;
int		nLoop;
int		nLoop2;
int		nValidVerts = 0;
CUSTOMVERTEX*	pxReducedVertices;
ushort*			puwIndices = NULL;
unsigned int*	punIndices = NULL;
ushort*			puwOriginalIndices = NULL;
unsigned int*	punOriginalIndices = NULL;
unsigned int	unOldIndex;
BOOL			bOriginalMeshHas32BitIndexBuffer = pxModelData->pxBaseMesh->Is32BitIndexBuffer();
int				nNumFaces;
CUSTOMVERTEX*		pxAddedVert;
CUSTOMVERTEX*		pxNextVert;

	pxReducedVertices = (CUSTOMVERTEX*)( SystemMalloc( pxModelData->xStats.nNumVertices * sizeof(CUSTOMVERTEX) ) );
	pnVertIndexList = (unsigned int*)( SystemMalloc( pxModelData->xStats.nNumVertices * sizeof(unsigned int) ) );

	nProcessVert = 0;
	nUnrepeatedVertCount = 0;

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	int		nVertsInOriginal = pxModelData->pxBaseMesh->GetNumVertices();
	int		nFacesInOriginal = pxModelData->pxBaseMesh->GetNumFaces();

	// Loop through all the vertices in the mesh
	for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
	{
		pxNextVert = pxVertices + nLoop;
		// Loop through all the verts we've previously found to be unique
		for ( nLoop2 = 0; nLoop2 < nValidVerts; nLoop2++ )
		{
			pxAddedVert = pxReducedVertices + nLoop2;
			if ( ( pxNextVert->tu == pxAddedVert->tu ) &&
				 ( pxNextVert->tv == pxAddedVert->tv ) &&
				 ( pxNextVert->position.x == pxAddedVert->position.x ) &&
				 ( pxNextVert->position.y == pxAddedVert->position.y ) &&
				 ( pxNextVert->position.z == pxAddedVert->position.z ) &&
				 ( pxNextVert->normal.x == pxAddedVert->normal.x ) &&
				 ( pxNextVert->normal.y == pxAddedVert->normal.y ) &&
				 ( pxNextVert->normal.z == pxAddedVert->normal.z ) &&
				 ( pxNextVert->color == pxAddedVert->color ) )
			{
				break;
			}
		}

		// This vert didn't match any of the ones previously checked..
		if ( nLoop2 == nValidVerts )
		{
			pxReducedVertices[nValidVerts] = pxVertices[nLoop];
			pnVertIndexList[nLoop] = nValidVerts;
			nValidVerts++;
		}
		else		// This vert exactly matched one we'd added earlier
		{
			pnVertIndexList[nLoop] = nLoop2;
		}

		// TODO - Update progress bar here
	}

	// Now we have a list pnVertexIndexList, which can be used to change the index buffer
	// and a buffer pxReducedVertices which should replace the mesh's vertex buffer.
	SysDebugPrint( "Mesh reduced from %d to %d unique vertices", pxModelData->xStats.nNumVertices, nValidVerts );
	EngineMesh*		pxNewMesh = new EngineMesh;

	pxNewMesh->Create( pxModelData->xStats.nNumIndices / 3, nValidVerts, 0 );

	CUSTOMVERTEX*	pNewMeshVertices = NULL;

	// Write out the new vert list
	pxNewMesh->LockVertexBuffer( 0, (byte**)&pNewMeshVertices );
	if ( pNewMeshVertices )
	{
		for ( nLoop = 0; nLoop < nValidVerts; nLoop++ )
		{
			pNewMeshVertices[nLoop] = pxReducedVertices[nLoop];
		}
		pxNewMesh->UnlockVertexBuffer();

		if ( pxNewMesh->Is32BitIndexBuffer() == TRUE )
		{
			pxNewMesh->LockIndexBuffer(kLock_Normal, (byte**)( &punIndices ) );
		}
		else
		{
			pxNewMesh->LockIndexBuffer(kLock_Normal, (byte**)( &puwIndices ) );
		}

		if ( bOriginalMeshHas32BitIndexBuffer == TRUE )
		{
			pxModelData->pxBaseMesh->LockIndexBuffer( kLock_ReadOnly, (byte**)( &punOriginalIndices ) );
		}
		else
		{
			pxModelData->pxBaseMesh->LockIndexBuffer( kLock_ReadOnly, (byte**)( &puwOriginalIndices ) );
		}

		// Write out the modified index buffers
		if ( pxNewMesh->Is32BitIndexBuffer() )
		{
			for ( nLoop = 0; nLoop < pxModelData->xStats.nNumIndices; nLoop++ )
			{
				if ( bOriginalMeshHas32BitIndexBuffer == TRUE )
				{
					unOldIndex = punOriginalIndices[nLoop];
				}
				else
				{
					unOldIndex = puwOriginalIndices[nLoop];
				}
				punIndices[nLoop] = pnVertIndexList[unOldIndex];
			}
		}
		else
		{
		ushort		uwNewIndex;

			for ( nLoop = 0; nLoop < pxModelData->xStats.nNumIndices; nLoop++ )
			{
				if ( bOriginalMeshHas32BitIndexBuffer == TRUE )
				{
					unOldIndex = punOriginalIndices[nLoop];
				}
				else
				{
					unOldIndex = puwOriginalIndices[nLoop];
				}

				uwNewIndex = (ushort)pnVertIndexList[unOldIndex];
				puwIndices[nLoop] = uwNewIndex;
			}
		}
		pxNewMesh->UnlockIndexBuffer();

		DWORD*		pxOriginalMeshAttributes = NULL;
		DWORD*		pxNewMeshAttributes = NULL;

		// Copy the attribute buffer from old model to new
		nNumFaces = pxModelData->xStats.nNumIndices / 3;
		pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_ReadOnly, (BYTE**)&pxOriginalMeshAttributes );
		pxNewMesh->LockAttributeBuffer( kLock_Discard, (BYTE**)&pxNewMeshAttributes );

		if ( ( pxOriginalMeshAttributes ) &&
			 ( pxNewMeshAttributes ) )
		{
			for ( nLoop = 0; nLoop < nNumFaces; nLoop++ )
			{ 
				*pxNewMeshAttributes = *pxOriginalMeshAttributes;
				pxNewMeshAttributes++;
				pxOriginalMeshAttributes++;
			}
			pxModelData->pxBaseMesh->UnlockAttributeBuffer();
			pxNewMesh->UnlockAttributeBuffer();
		}

		pxModelData->pxBaseMesh->Release();
		pxModelData->pxBaseMesh = pxNewMesh;
		pxModelData->xStats.nNumVertices = nValidVerts;
	}


	ModelExportValidate( pxModelData );
}


void ModelExportRemoveRepeatedKeyframeTracks( MODEL_RENDER_DATA* pxModelData  )
{
CUSTOMVERTEX*	pxVertices;
int*	pnVertIndexList;
//int*	pnNewIndexList;
ushort*	puwIndices;
VECT*		pxVerts;
VECT*	pxVertBase;
VECT*	pxNormalBase;
VECT*	pxProcessVerts;
VECT*	pxProcessNormals;
int			nProcessVert;
int			nUnrepeatedVertCount = 0;
int		nLoop;
int		nLoop2;

	if ( pxModelData->pxVertexKeyframes == NULL ) return;
	if ( pxModelData->pxBaseMesh == NULL ) return;
	// This function only works when the vertex tracks and indices have previously been set up such that
	// for each face the indices are 1,2,3   4,5,6 etc etc
	if ( pxModelData->xStats.nNumVertices != pxModelData->xStats.nNumIndices ) return;

	pxVertBase = pxModelData->pxVertexKeyframes;
	pxNormalBase = pxModelData->pxNormalKeyframes;
	pnVertIndexList = (int*)( SystemMalloc( pxModelData->xStats.nNumVertices * sizeof(int) ) );
	for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
	{
		pnVertIndexList[ nLoop ] = NOTFOUND;
	}

	nProcessVert = 0;
	nUnrepeatedVertCount = 0;

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

	for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
	{
		pxProcessVerts = pxVertBase + (nLoop*pxModelData->xStats.nNumVertKeyframes);

		// If we havent identified this track yet (or its the first one)
		if ( pnVertIndexList[nLoop] == NOTFOUND )
		{
			// This is a new track
			pnVertIndexList[nLoop] = nLoop;
			nUnrepeatedVertCount++;
			for ( nLoop2 = nLoop + 1; nLoop2 < pxModelData->xStats.nNumVertices; nLoop2++ )
			{
				pxVerts = pxVertBase + (nLoop2*pxModelData->xStats.nNumVertKeyframes);
				if ( ModelProcessAreTracksIdentical( pxVerts, pxProcessVerts, pxModelData->xStats.nNumVertKeyframes ) == TRUE )
				{
					if ( ModelProcessIsVertexDataIdentical( &pxVertices[nLoop2], &pxVertices[nLoop] ) == TRUE )
					{
						pnVertIndexList[ nLoop2 ] = nLoop;
					}
				}
			}
		}
	}

	if ( nUnrepeatedVertCount < pxModelData->xStats.nNumVertices )
	{
		pxProcessVerts = (VECT*)(SystemMalloc( nUnrepeatedVertCount * pxModelData->xStats.nNumVertKeyframes * sizeof(VECT) ) );
		pxProcessNormals = (VECT*)(SystemMalloc( nUnrepeatedVertCount * pxModelData->xStats.nNumVertKeyframes * sizeof(VECT) ) );
		nProcessVert = 0;

		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
		{
			// If its a valid track to add
			if ( pnVertIndexList[ nLoop ] == nLoop )
			{
				ModelProcessCopyKeyFrameTrack( pxProcessVerts + (nProcessVert*pxModelData->xStats.nNumVertKeyframes), pxVertBase + (nLoop*pxModelData->xStats.nNumVertKeyframes), pxModelData->xStats.nNumVertKeyframes );
				ModelProcessCopyKeyFrameTrack( pxProcessNormals + (nProcessVert*pxModelData->xStats.nNumVertKeyframes), pxNormalBase + (nLoop*pxModelData->xStats.nNumVertKeyframes), pxModelData->xStats.nNumVertKeyframes );
				if ( nLoop != nProcessVert )
				{
					ModelProcessCopyVertexData( &pxVertices[ nProcessVert ], &pxVertices[ nLoop ] );
				}
				pnVertIndexList[ nLoop ] = nProcessVert;
				nProcessVert++;
				PANIC_IF( nProcessVert > nUnrepeatedVertCount, "Error in keyframe removal" );
			}
			else
			{
				// Madness!
				pnVertIndexList[ nLoop ] = pnVertIndexList[ pnVertIndexList[nLoop] ];
			}

		}
		PANIC_IF( nProcessVert != nUnrepeatedVertCount, "Error 2 in keyframe removal" );
		
		// Now modify the indices in the base mesh to match the new track info
		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
		
			for ( nLoop = 0; nLoop < pxModelData->xStats.nNumIndices; nLoop++ )
			{
				*(puwIndices) = (ushort)( pnVertIndexList[ nLoop ] );
				puwIndices++;
			}
			pxModelData->pxBaseMesh->UnlockIndexBuffer();
		}

		pxModelData->xStats.nNumVertices = nUnrepeatedVertCount;
		pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;
		SystemFree( pxModelData->pxVertexKeyframes );
		SystemFree( pxModelData->pxNormalKeyframes );
		pxModelData->pxVertexKeyframes = pxProcessVerts;
		pxModelData->pxNormalKeyframes = pxProcessNormals;
	}
	pxModelData->pxBaseMesh->UnlockVertexBuffer();

}




/***************************************************************************
 * Function    : ModelExportChunkCompressedNormalKeyframes
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkCompressedNormalKeyframes( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
int				nVertsInMesh;
VECT*	pxVertKeyframe;
byte*	pbChunkMem;
int			nMemSize;
COMPRESSED_VERTS_HEADER*		pxHeader;
SHORT_VERT*		pxCompressedVerts;
VECT		xCompressed;
int		nVertsTotal;
int		nLoop;

	if ( pxModelData->pxNormalKeyframes != NULL )
	{
		nVertsInMesh = pxModelData->xStats.nNumVertices;

		nMemSize = sizeof( COMPRESSED_VERTS_HEADER ) + (nVertsInMesh * pxModelData->xStats.nNumVertKeyframes * sizeof(SHORT_VERT));
		pbChunkMem = (byte*)( SystemMalloc( nMemSize  ) );
		pxHeader = (COMPRESSED_VERTS_HEADER*)( pbChunkMem );
		pxCompressedVerts = (SHORT_VERT*)( pxHeader + 1 );

		pxHeader->uwNumKeyframes = pxModelData->xStats.nNumVertKeyframes;
		pxHeader->uwSizeOfHeader = sizeof( COMPRESSED_VERTS_HEADER );
		pxHeader->nNumVerts = pxModelData->xStats.nNumVertices;

		nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;

		pxVertKeyframe = pxModelData->pxNormalKeyframes;
		for( nLoop = 0; nLoop < nVertsTotal; nLoop++ )
		{
			xCompressed.x = pxVertKeyframe->x + 1.0f;
			xCompressed.y = pxVertKeyframe->y + 1.0f;
			xCompressed.z = pxVertKeyframe->z + 1.0f;
			xCompressed.x = ( xCompressed.x * 0.5f ) * 65535.0f;
			xCompressed.y = ( xCompressed.y * 0.5f ) * 65535.0f;
			xCompressed.z = ( xCompressed.z * 0.5f ) * 65535.0f;
			pxCompressedVerts->uwX = (ushort)( xCompressed.x );
			pxCompressedVerts->uwY = (ushort)( xCompressed.y );
			pxCompressedVerts->uwZ = (ushort)( xCompressed.z );
			pxCompressedVerts++;
			pxVertKeyframe++;
		}

		*(pnSize) = nMemSize;
		*(pnElements) = 1;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}



/***************************************************************************
 * Function    : ModelExportChunkCompressedVertexKeyframes
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkCompressedVertexKeyframes( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
int				nVertsInMesh;
VECT*	pxVertKeyframe;
VECT	xRange;
byte*	pbChunkMem;
int			nMemSize;
COMPRESSED_VERTS_HEADER*		pxHeader;
SHORT_VERT*		pxCompressedVerts;
VECT		xCompressed;
int		nVertsTotal;
int		nLoop;

	if ( pxModelData->pxVertexKeyframes != NULL )
	{
		nVertsInMesh = pxModelData->xStats.nNumVertices;

		nMemSize = sizeof( COMPRESSED_VERTS_HEADER ) + (nVertsInMesh * pxModelData->xStats.nNumVertKeyframes * sizeof(SHORT_VERT));
		pbChunkMem = (byte*)( SystemMalloc( nMemSize  ) );
		pxHeader = (COMPRESSED_VERTS_HEADER*)( pbChunkMem );
		pxCompressedVerts = (SHORT_VERT*)( pxHeader + 1 );

		pxHeader->uwNumKeyframes = pxModelData->xStats.nNumVertKeyframes;
		pxHeader->uwSizeOfHeader = sizeof( COMPRESSED_VERTS_HEADER );
		pxHeader->nNumVerts = pxModelData->xStats.nNumVertices;

		// First get the model's min and max bounding boxes (for all frames)
		ModelExportFindAnimationExtents( pxModelData, &pxHeader->xMin, &pxHeader->xMax );
		xRange.x = pxHeader->xMax.x - pxHeader->xMin.x;
		xRange.y = pxHeader->xMax.y - pxHeader->xMin.y;
		xRange.z = pxHeader->xMax.z - pxHeader->xMin.z;

		nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;

		pxVertKeyframe = pxModelData->pxVertexKeyframes;
		for( nLoop = 0; nLoop < nVertsTotal; nLoop++ )
		{
			xCompressed.x = pxVertKeyframe->x - pxHeader->xMin.x;
			xCompressed.y = pxVertKeyframe->y - pxHeader->xMin.y;
			xCompressed.z = pxVertKeyframe->z - pxHeader->xMin.z;
			xCompressed.x = ( xCompressed.x / xRange.x ) * 65535.0f;
			xCompressed.y = ( xCompressed.y / xRange.y ) * 65535.0f;
			xCompressed.z = ( xCompressed.z / xRange.z ) * 65535.0f;
			pxCompressedVerts->uwX = (ushort)( xCompressed.x );
			pxCompressedVerts->uwY = (ushort)( xCompressed.y );
			pxCompressedVerts->uwZ = (ushort)( xCompressed.z );
			pxCompressedVerts++;
			pxVertKeyframe++;
		}

		*(pnSize) = nMemSize;
		*(pnElements) = 1;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}


/***************************************************************************
 * Function    : ModelExportChunkTexCoords
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
byte*		ModelExportChunkVertexKeyframes( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
int				nVertsInMesh;
VECT*	pxVertKeyframe;
byte*	pbChunkMem;
int			nMemSize;

	if ( pxModelData->pxVertexKeyframes != NULL )
	{
		nVertsInMesh = pxModelData->xStats.nNumVertices;

		nMemSize = nVertsInMesh * pxModelData->xStats.nNumVertKeyframes * sizeof(VECT);
		pbChunkMem = (byte*)( SystemMalloc( nMemSize  ) );
		pxVertKeyframe = (VECT*)( pbChunkMem );

		memcpy( pbChunkMem, pxModelData->pxVertexKeyframes, nMemSize );

		*(pnSize) = sizeof( VECT ) * pxModelData->xStats.nNumVertKeyframes;
		*(pnElements) = nVertsInMesh;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}

/***************************************************************************
 * Function    : ModelExportChunkGlobalProperties
 * Params      :
 * Description : 
 ***************************************************************************/
byte*		ModelExportChunkGlobalProperties( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
GLOBAL_PROPERTIES_CHUNK*	pxMatChunk;
byte*	pbChunkMem;
int			nMemSize;

	nMemSize = sizeof( GLOBAL_PROPERTIES_CHUNK );
	pbChunkMem = (byte*)( SystemMalloc( nMemSize  ) );
	pxMatChunk = (GLOBAL_PROPERTIES_CHUNK*)( pbChunkMem );

	memcpy( pbChunkMem, &pxModelData->xGlobalProperties, nMemSize );

	*(pnSize) = sizeof( GLOBAL_PROPERTIES_CHUNK);
	*(pnElements) = 1;
	return( pbChunkMem );
}


/***************************************************************************
 * Function    : ModelExportCreatorInfo
 * Params      :
 * Description : 
 ***************************************************************************/
byte*		ModelExportCreatorInfo( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
CREATOR_INFO_CHUNK*	pxChunk;
byte*	pbChunkMem;
int			nMemSize;
int		nStrLen;

	nStrLen = strlen( pxModelData->xStats.acCreatorInfo );
	if ( nStrLen > 0 )
	{
		nMemSize = nStrLen + sizeof( CREATOR_INFO_CHUNK ) + 1;
		pbChunkMem = (byte*)( SystemMalloc( nMemSize  ) );
		ZeroMemory( pbChunkMem, nMemSize );
		pxChunk = (CREATOR_INFO_CHUNK*)( pbChunkMem );

		strcpy( (char*)( pxChunk + 1 ), pxModelData->xStats.acCreatorInfo );
		*(pnSize) = nMemSize;
		*(pnElements) = 1;
		return( pbChunkMem );
	}
	return( NULL );
}

/***************************************************************************
 * Function    : ModelExportChunkKeyframeData
 * Params      :
 * Description : Writes out the chunk containing all the animation keyframe info
 ***************************************************************************/
byte*		ModelExportChunkKeyframeData( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
int				nKeyframes;
//VECT*	pxVertKeyframe;
byte*	pbChunkMem;
int			nMemSize;
MODEL_KEYFRAME_DATA*		pxKeyframeData;
int			nLoop;

	if ( pxModelData->pxVertexKeyframes != NULL )
	{
		nKeyframes = pxModelData->xStats.nNumVertKeyframes;

		nMemSize = nKeyframes * sizeof( MODEL_KEYFRAME_DATA );
		pbChunkMem = (byte*)( SystemMalloc( nMemSize ) );
		pxKeyframeData = (MODEL_KEYFRAME_DATA*)( pbChunkMem );

		for ( nLoop = 0; nLoop < nKeyframes; nLoop++ )
		{
			*(pxKeyframeData++) = pxModelData->axKeyframeData[ nLoop ];
		}

		*(pnSize) = sizeof( MODEL_KEYFRAME_DATA );
		*(pnElements) = nKeyframes;
		return( pbChunkMem );
	}
	else
	{
	}

	return( NULL );
}


/***************************************************************************
 * Function    : ModelExportBuildChunk
 * Params      :
 * Description : Called to add a single data chunk to the file
 ***************************************************************************/
byte*		ModelExportBuildChunk( MODEL_RENDER_DATA* pxModelData, int nChunkNum, int* pnSize, int* pnElements )
{
	switch( nChunkNum )
	{
	case ATM_CHUNKID_BASIC_CONTENTS:
		return( ModelExportChunkBasicContents( pxModelData, nChunkNum, pnSize, pnElements ) );
		break;

	case ATM_CHUNKID_OLD_BASIC_CONTENTS:
		break;
//		return( ModelExportChunkOldBasicContents( pxModelData, nChunkNum, pnSize, pnElements ) );
		break;
	case ATM_CHUNKID_COMPRESSED_VERTS:
		// Use this if we dont have keyframes and do want the vertices compressed
		if ( ( pxModelData->pxVertexKeyframes == NULL ) &&
			 ( (mulExportFlags & ATM_EXPORT_FLAGS_COMPRESSED_VERTICES) != 0 ) )
		{
			return( ModelExportChunkCompressedVerts( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_KEYFRAME_DATA:
		if ( pxModelData->pxVertexKeyframes != NULL )
		{
			return( ModelExportChunkKeyframeData( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_VERTS:
		// Use this if we dont have keyframes and dont want the vertices compressed
		if ( ( pxModelData->pxVertexKeyframes == NULL ) &&
			 ( (mulExportFlags & ATM_EXPORT_FLAGS_COMPRESSED_VERTICES) == 0 ) )
		{
			return( ModelExportChunkVerts( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;	
	case ATM_CHUNKID_NORMALS:
		if ( pxModelData->pxNormalKeyframes == NULL )
		{
			return( ModelExportChunkNormals( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_COLOURS:
		if ( (mulExportFlags & ATM_EXPORT_FLAGS_COLOURS) != 0 )
		{
			return( ModelExportChunkColours( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_UVS:
		if ( (mulExportFlags & ATM_EXPORT_FLAGS_VERY_COMPRESSED_UVS) == 0 )
		{
			return( ModelExportChunkTexCoords( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_VERY_COMPRESSED_UVS:
		if ( (mulExportFlags & ATM_EXPORT_FLAGS_VERY_COMPRESSED_UVS) != 0 )
		{
			return( ModelExportChunkCompressedTexCoords( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_ATTACH_DATA:
		return( ModelExportChunkAttachData( pxModelData, nChunkNum, pnSize, pnElements ) );
		break;
	case ATM_CHUNKID_LOD_DATA:
		if ( ( pxModelData->xMedLODAttachData.nModelHandle != NOTFOUND ) ||
			 ( pxModelData->xLowLODAttachData.nModelHandle != NOTFOUND ) )
		{
			return( ModelExportChunkLODData( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_TEXMATERIAL:
		return( ModelExportChunkTexMaterials( pxModelData, nChunkNum, pnSize, pnElements ) );	
		break;
	case ATM_CHUNKID_ATTRIBUTEDATA:
		return( ModelExportChunkAttributes( pxModelData, nChunkNum, pnSize, pnElements ) );	
		break;
	case ATM_CHUNKID_EFFECT_DATA:
		if ( pxModelData->xEffectAttachData.nEffectType != 0 )
		{
			return( ModelExportChunkEffectData( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_TURRET_DATA:
/*		if ( ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND ) ||
			 ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND ) ||
			 ( pxModelData->xWheel1AttachData.nModelHandle != NOTFOUND ) ||
			 ( pxModelData->xWheel2AttachData.nModelHandle != NOTFOUND ) ||
			 ( pxModelData->xWheel3AttachData.nModelHandle != NOTFOUND ) ||
			 ( pxModelData->xWheel4AttachData.nModelHandle != NOTFOUND ) )
		{
*/			return( ModelExportChunkTurretData( pxModelData, nChunkNum, pnSize, pnElements ) );
//		}
		break;
	case ATM_CHUNKID_CREATOR_INFO:
		return( ModelExportCreatorInfo( pxModelData, nChunkNum, pnSize, pnElements ) );
		break;
	case ATM_CHUNKID_GLOBALPROPERTIES:
		if ( (mulExportFlags & ATM_EXPORT_FLAGS_MATERIALS) != 0 )
		{
			return( ModelExportChunkGlobalProperties( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_COMPRESSED_NORMAL_KEYFRAMES:
		if ( pxModelData->pxNormalKeyframes != NULL )
		{
			return( ModelExportChunkCompressedNormalKeyframes( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_COMPRESSED_VERT_KEYFRAMES:
		if ( ( pxModelData->pxVertexKeyframes != NULL ) &&
			 ( (mulExportFlags & ATM_EXPORT_FLAGS_COMPRESSED_VERTICES) != 0 ) )
		{
			return( ModelExportChunkCompressedVertexKeyframes( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_VERT_KEYFRAMES:
		if ( ( pxModelData->pxVertexKeyframes != NULL ) &&
			 ( (mulExportFlags & ATM_EXPORT_FLAGS_COMPRESSED_VERTICES) == 0 ) )
		{
			return( ModelExportChunkVertexKeyframes( pxModelData, nChunkNum, pnSize, pnElements ) );
		}
		break;
	case ATM_CHUNKID_INDICES:
		return( ModelExportChunkIndices( pxModelData, nChunkNum, pnSize, pnElements ) );
		break;
	}
	return( NULL );
}


/***************************************************************************
 * Function    : ModelWriteSubModelATM
 * Params      :
 * Description : Recursed off ExportATM to save any sub models within the main atm file
 ***************************************************************************/
void		ModelWriteSubModelATM( MODEL_RENDER_DATA* pxModelData, FILE* pFile, int nLevel )
{
MODEL_RENDER_DATA* pxBaseModelData;
int		nSubModelPasses = 0;
ATM_CHUNK_HEADER	xChunkHeader;
int		nLoop;
int		nSize;
int		nElements;
byte*	pbSrcPtr;
int		nChunkSize;

	pxBaseModelData = pxModelData;

	while ( pxModelData != NULL )
	{
		for ( nLoop = 0; nLoop < ATM_MAX_CHUNKID; nLoop++ )
		{		
			// Export all the chunks
			pbSrcPtr = ModelExportBuildChunk( pxModelData, nLoop, &nSize, &nElements );
			// If a chunk was created
			if ( pbSrcPtr != NULL )
			{
				xChunkHeader.bChunkCode = ATM_CHUNK_HEADER_CODE;
				xChunkHeader.uwChunkID = (ushort)( nLoop );
				xChunkHeader.nNumElementsInChunk = nElements;
				xChunkHeader.nSizeOfElement = nSize;

				// Write the chunk header
				SysFileWrite( (unsigned char*)&xChunkHeader, sizeof( ATM_CHUNK_HEADER ), 1, pFile );

				nChunkSize = xChunkHeader.nNumElementsInChunk * xChunkHeader.nSizeOfElement;
				// Write the chunk
				SysFileWrite( pbSrcPtr, nChunkSize, 1, pFile );

				// TODO - Make sure the data is aligned to a 4 byte boundary...
				if ( ( nChunkSize % 4 ) != 0 )
				{
				int		nAlignBuffer = 4 - (nChunkSize % 4);
				unsigned char	abBuffer[4] = { 0, 0, 0, 0 };					

					SysFileWrite( &abBuffer[0], nAlignBuffer, 1, pFile );
				}

				SystemFree( pbSrcPtr );
				pbSrcPtr = NULL;
			}
		}

		switch ( nSubModelPasses )
		{
		case 0:
			// If theres a turret attached to this model
			if ( pxBaseModelData->xHorizTurretData.nModelHandle != NOTFOUND )
			{
				// Add a separator chunk at the end of the first model's data
				// Add a separator chunk at the end of the first model's data
				xChunkHeader.bChunkCode = ATM_CHUNK_HEADER_CODE;
				xChunkHeader.uwChunkID = (ushort)( ATM_CHUNKID_MODEL_SEPARATOR );
				xChunkHeader.nNumElementsInChunk = 0;
				xChunkHeader.nSizeOfElement = 0;
				xChunkHeader.bChunkDataNum = 1;
				// Write the chunk header
				SysFileWrite( (unsigned char*)&xChunkHeader, sizeof( ATM_CHUNK_HEADER ), 1, pFile );

				pxModelData = &maxModelRenderData[ pxBaseModelData->xHorizTurretData.nModelHandle ];
				break;
			}
			// no break intentional - if no turret handle, fall thru to the next stage export
		case 1:
			nSubModelPasses = 1;
			// If theres a turret attached to this model
			if ( pxBaseModelData->xVertTurretData.nModelHandle != NOTFOUND )
			{
				// Add a separator chunk at the end of the first model's data
				xChunkHeader.bChunkCode = ATM_CHUNK_HEADER_CODE;
				xChunkHeader.uwChunkID = (ushort)( ATM_CHUNKID_MODEL_SEPARATOR );
				xChunkHeader.nNumElementsInChunk = 0;
				xChunkHeader.nSizeOfElement = 0;
				xChunkHeader.bChunkDataNum = 2;
				// Write the chunk header
				SysFileWrite( (unsigned char*)&xChunkHeader, sizeof( ATM_CHUNK_HEADER ), 1, pFile );

				pxModelData = &maxModelRenderData[ pxBaseModelData->xVertTurretData.nModelHandle ];
				break;
			}
			// no break intentional - if no vert turret handle just set to null and exit the loop
		case 2:
			nSubModelPasses = 2;
			// If theres a turret attached to this model
			if ( pxBaseModelData->xWheel1AttachData.nModelHandle != NOTFOUND )
			{
				// Add a separator chunk at the end of the first model's data
				xChunkHeader.bChunkCode = ATM_CHUNK_HEADER_CODE;
				xChunkHeader.uwChunkID = (ushort)( ATM_CHUNKID_MODEL_SEPARATOR );
				xChunkHeader.nNumElementsInChunk = 0;
				xChunkHeader.nSizeOfElement = 0;
				xChunkHeader.bChunkDataNum = 3;
				// Write the chunk header
				SysFileWrite( (unsigned char*)&xChunkHeader, sizeof( ATM_CHUNK_HEADER ), 1, pFile );

				pxModelData = &maxModelRenderData[ pxBaseModelData->xWheel1AttachData.nModelHandle ];
				break;
			}
			// no break intentional - if no vert turret handle just set to null and exit the loop
		case 3:
			nSubModelPasses = 3;
			// If theres a turret attached to this model
			if ( pxBaseModelData->xWheel3AttachData.nModelHandle != NOTFOUND )
			{
				// Add a separator chunk at the end of the first model's data
				xChunkHeader.bChunkCode = ATM_CHUNK_HEADER_CODE;
				xChunkHeader.uwChunkID = (ushort)( ATM_CHUNKID_MODEL_SEPARATOR );
				xChunkHeader.nNumElementsInChunk = 0;
				xChunkHeader.nSizeOfElement = 0;
				xChunkHeader.bChunkDataNum = 4;
				// Write the chunk header
				SysFileWrite( (unsigned char*)&xChunkHeader, sizeof( ATM_CHUNK_HEADER ), 1, pFile );

				pxModelData = &maxModelRenderData[ pxBaseModelData->xWheel3AttachData.nModelHandle ];
				break;
			}
			// no break intentional - if no vert turret handle just set to null and exit the loop
		case 4:
			nSubModelPasses = 4;
			// If theres a turret attached to this model
			if ( pxBaseModelData->xMedLODAttachData.nModelHandle != NOTFOUND )
			{
				// Add a separator chunk at the end of the first model's data
				xChunkHeader.bChunkCode = ATM_CHUNK_HEADER_CODE;
				xChunkHeader.uwChunkID = (ushort)( ATM_CHUNKID_MODEL_SEPARATOR );
				xChunkHeader.nNumElementsInChunk = 0;
				xChunkHeader.nSizeOfElement = 0;
				xChunkHeader.bChunkDataNum = 5;
				// Write the chunk header
				SysFileWrite( (unsigned char*)&xChunkHeader, sizeof( ATM_CHUNK_HEADER ), 1, pFile );

				pxModelData = &maxModelRenderData[ pxBaseModelData->xMedLODAttachData.nModelHandle ];
				// Recursively export all the submodels of the medium model
				ModelWriteSubModelATM( pxModelData, pFile, nLevel + 1 );
			}
			// no break intentional - if no vert turret handle just set to null and exit the loop
		case 5:
			nSubModelPasses = 5;
			// If theres a turret attached to this model
			// (we dont want the Low LOD attached to the med model - which happens due to the 
			// natural process of recursively loading the submodels, so we only process this if we're at the top level)
			if ( ( pxBaseModelData->xLowLODAttachData.nModelHandle != NOTFOUND ) &&
				 ( nLevel == 0 ) )
			{
				// Add a separator chunk at the end of the first model's data
				xChunkHeader.bChunkCode = ATM_CHUNK_HEADER_CODE;
				xChunkHeader.uwChunkID = (ushort)( ATM_CHUNKID_MODEL_SEPARATOR );
				xChunkHeader.nNumElementsInChunk = 0;
				xChunkHeader.nSizeOfElement = 0;
				xChunkHeader.bChunkDataNum = 6;
				// Write the chunk header
				SysFileWrite( (unsigned char*)&xChunkHeader, sizeof( ATM_CHUNK_HEADER ), 1, pFile );

				pxModelData = &maxModelRenderData[ pxBaseModelData->xLowLODAttachData.nModelHandle ];
				// Recursively export all the submodels of the low LOD model
				ModelWriteSubModelATM( pxModelData, pFile, nLevel + 1 );
			}
			// no break intentional - if no vert turret handle just set to null and exit the loop
		case 6:
			nSubModelPasses = 6;
			// If theres a turret attached to this model
			if ( pxBaseModelData->xCollisionAttachData.nModelHandle != NOTFOUND )
			{
				// Add a separator chunk at the end of the first model's data
				xChunkHeader.bChunkCode = ATM_CHUNK_HEADER_CODE;
				xChunkHeader.uwChunkID = (ushort)( ATM_CHUNKID_MODEL_SEPARATOR );
				xChunkHeader.nNumElementsInChunk = 0;
				xChunkHeader.nSizeOfElement = 0;
				xChunkHeader.bChunkDataNum = 7;
				// Write the chunk header
				SysFileWrite( (unsigned char*)&xChunkHeader, sizeof( ATM_CHUNK_HEADER ), 1, pFile );

				pxModelData = &maxModelRenderData[ pxBaseModelData->xCollisionAttachData.nModelHandle ];
				break;
			}
			// no break intentional - if no vert turret handle just set to null and exit the loop
		default:
			pxModelData = NULL;
			break;
		}

		nSubModelPasses++;
	}
}

void ModelFileCompress( const char* szFilename )
{
byte*	pbMemBuffer;
byte*	pbCompressedMemBuffer;
byte*	pbMemToCompress;
int		nFileSize;
ATM_FILE_HEADER*	pxFileHeader;
uLongf	ulCompressedSize;
FILE*	pFile;
short	wResult;

	pFile = SysFileOpen( szFilename, "rb" );

	// if the file could be opened
	if ( pFile != NULL )
	{
		// Get the size of the tex
		nFileSize = SysGetFileSize( pFile );

		pbMemBuffer = (byte*)SystemMalloc( nFileSize );
		SysFileRead( pbMemBuffer, nFileSize, 1, pFile );
		SysFileClose( pFile );		

		pxFileHeader = (ATM_FILE_HEADER*)(pbMemBuffer);
		pbMemToCompress = (byte*)( pxFileHeader + 1 );

		ulCompressedSize = (int)(nFileSize * 1.2f) + 12;
		pbCompressedMemBuffer = (byte*)SystemMalloc( ulCompressedSize );
		wResult = compress(pbCompressedMemBuffer, &ulCompressedSize, pbMemToCompress, nFileSize - sizeof( ATM_FILE_HEADER ) );
		if ( ulCompressedSize < (uint32)(nFileSize) )
		{
			pxFileHeader->bCompressedFlag = 1;
			pxFileHeader->ulUncompressedSize = nFileSize - sizeof( ATM_FILE_HEADER );
			pFile = SysFileOpen( szFilename, "wb" );
			 
			// if the file could be opened
			if ( pFile != NULL )
			{
				SysFileWrite( (unsigned char*)pxFileHeader, sizeof( ATM_FILE_HEADER ), 1, pFile );
				SysFileWrite( (unsigned char*)pbCompressedMemBuffer, ulCompressedSize, 1, pFile );
				SysFileClose( pFile );
			}
		}
		else
		{	// Compression resulted in larger file.. oh hum

		}
		SystemFree( pbCompressedMemBuffer );
		SystemFree( pbMemBuffer );
	}
}


/***************************************************************************
 * Function    : ModelExportATM
 * Params      :
 * Description : Saves the specified model with the given filename
 ***************************************************************************/
void		ModelExportATM( int nModelHandle, const char* szFilename, uint32 ulSaveFlags, uint32 ulLockFlags )
{
MODEL_RENDER_DATA* pxModelData;
FILE*	pFile;
ATM_FILE_HEADER		xFileHeader;

	mulExportFlags = ulSaveFlags;
	mulExportLockFlags = ulLockFlags;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	ModelExportRemoveRepeatedKeyframeTracks( pxModelData );

	// If no anims and no attachments
	if ( ( pxModelData->pxVertexKeyframes == NULL ) &&
		 ( pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex <= 0 ) &&
		 ( pxModelData->xEffectAttachData.nAttachVertex <= 0 ) &&
		 ( pxModelData->xHorizTurretData.nModelHandle <= 0 ) &&
		 ( pxModelData->xVertTurretData.nModelHandle <= 0 ) &&
		 ( pxModelData->xWheel1AttachData.nModelHandle <= 0 ) &&
		 ( pxModelData->xWheel2AttachData.nModelHandle <= 0 ) &&
		 ( pxModelData->xWheel3AttachData.nModelHandle <= 0 ) &&
		 ( pxModelData->xWheel4AttachData.nModelHandle <= 0 ) )
	{
		// Remove duplicated vertices
		ModelExportRemoveIdenticalVertices( pxModelData );
	}

	pFile = SysFileOpen( szFilename, "wb" );

	// if the file could be opened
	if ( pFile != NULL )
	{
		xFileHeader.uwFileHeader = ATM_CHUNK_FILE_HEADER_CODE;
		xFileHeader.uwFileVersionNum = ATM_CHUNK_FILE_VERSION_NUM;
		xFileHeader.bSizeOfChunkHeader = sizeof( ATM_CHUNK_HEADER );
		xFileHeader.bSizeOfFileHeader = sizeof( ATM_FILE_HEADER );

		SysFileWrite( (unsigned char*)&xFileHeader, xFileHeader.bSizeOfFileHeader, 1, pFile );

		ModelWriteSubModelATM( pxModelData, pFile, 0 );

		SysFileClose( pFile );

		if ( (mulExportFlags & ATM_EXPORT_FLAGS_COMPRESSION) != 0 )
		{
			ModelFileCompress( szFilename );
		}
	}
	else
	{
		PANIC_IF( TRUE, "Couldnt open file for writing" );
	}
} 


/***************************************************************************
 * Function    : ModelLoadProcessChunkOldBasicContents
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkOldBasicContents( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
ATM_OLD_BASIC_CONTENTS*		pxContents;
int			nVertLoop;
ushort*		puwIndices = NULL;

	pxContents = (ATM_OLD_BASIC_CONTENTS*)( pbMem );

	pxModelData->xStats.nNumVertices = pxContents->uwNumVertices;
	pxModelData->xStats.nNumIndices = pxContents->uwNumFaces * 3;
	pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;
	pxModelData->xStats.nNumMaterials = pxContents->bNumMaterials;
	pxModelData->bModelType = pxContents->bModelType;	
	pxModelData->bWheelDisplayMode = pxContents->bWheelDisplayMode;
	if ( pxContents->ulLockID == 0xCDCDCDCD )
	{
		pxModelData->xStats.ulLockCode = 0;
		pxModelData->xStats.ulLockID = 0;
	}
	else
	{
		pxModelData->xStats.ulLockCode = pxContents->ulLockCode;
		pxModelData->xStats.ulLockID = pxContents->ulLockID;
	}
	
	if ( pxModelData->pxBaseMesh == NULL )
	{
		// Create the mesh here now we know the vertex and face counts..
//		D3DXCreateMeshFVF( pxContents->uwNumFaces, pxContents->uwNumVertices, D3DXMESH_IB_WRITEONLY|D3DXMESH_IB_DYNAMIC|D3DXMESH_VB_DYNAMIC|D3DXMESH_VB_WRITEONLY, D3DFVF_CUSTOMVERTEX, mpEngineDevice, &pxModelData->pxBaseMesh );
//		D3DXCreateMeshFVF( pxContents->uwNumFaces, pxContents->uwNumVertices, D3DXMESH_SYSTEMMEM, D3DFVF_CUSTOMVERTEX, mpEngineDevice, &pxModelData->pxBaseMesh );
#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("Creating mesh: %d faces, %d verts, %d materials\n", pxContents->uwNumFaces, pxContents->uwNumVertices, pxContents->bNumMaterials );
#endif
		// Use 32 bit index buffer if we've got more than 65,535 vertices (NOT FACES/INDICES!!)
		if ( pxContents->uwNumVertices > 65535 )
		{
		unsigned int* punIndices;

			EngineCreateMesh( pxContents->uwNumFaces, pxContents->uwNumVertices, &pxModelData->pxBaseMesh, 2 );

			// By default set the basemesh indices to 0,1,2  3,4,5 etc which the old keyframe animation expects
			pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &punIndices ) );
			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumIndices; nVertLoop++ )
			{
				*(punIndices++) = (unsigned int)(nVertLoop);
			}
			pxModelData->pxBaseMesh->UnlockIndexBuffer();
		}
		else
		{
			EngineCreateMesh( pxContents->uwNumFaces, pxContents->uwNumVertices, &pxModelData->pxBaseMesh, 0 );

			// By default set the basemesh indices to 0,1,2  3,4,5 etc which the old keyframe animation expects
			pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
			if ( puwIndices != NULL )
			{
				for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumIndices; nVertLoop++ )
				{
					*(puwIndices++) = (ushort)(nVertLoop);
				}
				pxModelData->pxBaseMesh->UnlockIndexBuffer();
			}
		}
#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("Create mesh complete\n" );
#endif
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkBasicContents
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkBasicContents( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
ATM_BASIC_CONTENTS*		pxContents;
int			nVertLoop;
ushort*		puwIndices = NULL;

	pxContents = (ATM_BASIC_CONTENTS*)( pbMem );

	pxModelData->xStats.nNumVertices = pxContents->nNumVertices;
	pxModelData->xStats.nNumIndices = pxContents->nNumFaces * 3;
	pxModelData->xStats.nNumNormals = pxContents->nNumNormals;
	pxModelData->xStats.nNumMaterials = pxContents->bNumMaterials;
	pxModelData->bModelType = pxContents->bModelType;	
	pxModelData->bWheelDisplayMode = pxContents->bWheelDisplayMode;
	if ( pxContents->ulLockID == 0xCDCDCDCD )
	{
		pxModelData->xStats.ulLockCode = 0;
		pxModelData->xStats.ulLockID = 0;
	}
	else
	{
		pxModelData->xStats.ulLockCode = pxContents->ulLockCode;
		pxModelData->xStats.ulLockID = pxContents->ulLockID;
	}
	
	if ( pxModelData->pxBaseMesh == NULL )
	{
		// Create the mesh here now we know the vertex and face counts..
//		D3DXCreateMeshFVF( pxContents->uwNumFaces, pxContents->uwNumVertices, D3DXMESH_IB_WRITEONLY|D3DXMESH_IB_DYNAMIC|D3DXMESH_VB_DYNAMIC|D3DXMESH_VB_WRITEONLY, D3DFVF_CUSTOMVERTEX, mpEngineDevice, &pxModelData->pxBaseMesh );
//		D3DXCreateMeshFVF( pxContents->uwNumFaces, pxContents->uwNumVertices, D3DXMESH_SYSTEMMEM, D3DFVF_CUSTOMVERTEX, mpEngineDevice, &pxModelData->pxBaseMesh );
#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("Creating mesh: %d faces, %d verts, %d materials\n", pxContents->nNumFaces, pxContents->nNumVertices, pxContents->bNumMaterials );
#endif
		// If we have more than 65,535 *VERTICES* (*NOT* FACES) we need a 32-bit index bufffer. (As the index buffer contains the
		// referenced vertex number).
		if ( pxContents->nNumVertices > 65535 )
		{
		unsigned int* punIndices;

			EngineCreateMesh( pxContents->nNumFaces, pxContents->nNumVertices, &pxModelData->pxBaseMesh, 2 );

			// By default set the basemesh indices to 0,1,2  3,4,5 etc which the old keyframe animation expects
			pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &punIndices ) );
			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumIndices; nVertLoop++ )
			{
				*(punIndices++) = (unsigned int)(nVertLoop);
			}
			pxModelData->pxBaseMesh->UnlockIndexBuffer();
		}
		else
		{
			EngineCreateMesh( pxContents->nNumFaces, pxContents->nNumVertices, &pxModelData->pxBaseMesh, 0 );

			// By default set the basemesh indices to 0,1,2  3,4,5 etc which the old keyframe animation expects
			pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
			if ( puwIndices != NULL )
			{
				for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumIndices; nVertLoop++ )
				{
					*(puwIndices++) = (ushort)(nVertLoop);
				}
				pxModelData->pxBaseMesh->UnlockIndexBuffer();
			}
		}
#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("Create mesh complete\n" );
#endif
	}
}


/***************************************************************************
 * Function    : ModelSetMaterialEmissive
 ***************************************************************************/
void		ModelSetMaterialEmissive( int nModelHandle, int nMaterialNum, const ENGINEMATERIAL_COLOUR* pxCol )
{
MODEL_RENDER_DATA* pxModelData;
ModelMaterialData*		pMaterialData;

	pxModelData = &maxModelRenderData[ nModelHandle ];
	pMaterialData = FindMaterial( pxModelData, nMaterialNum );
	if ( pMaterialData )
	{
		pMaterialData->SetColour( ModelMaterialData::EMISSIVE, (MATERIAL_COLOUR*)pxCol );
	}

}



/***************************************************************************
 * Function    : ModelSetVertexColours
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelSetVertexColours( int nModelHandle, uint32 ulCol )
{
CUSTOMVERTEX*	pxVertices;
MODEL_RENDER_DATA* pxModelData;
int			nVertLoop;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
		{
			pxVertices->color = ulCol;
			pxVertices++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
}

/***************************************************************************
 * Function    : ModelLoadProcessChunkVerts
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkVerts( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices = NULL;
VECT*		pxVerts;
int			nVertLoop;

	pxVerts = (VECT*)( pbMem );
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		if ( pxVertices )
		{
			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
			{
				pxVertices->position.x = pxVerts->x;
				pxVertices->position.y = pxVerts->y;
				pxVertices->position.z = pxVerts->z;
				pxVertices->color = 0xFFFFFFFF;
				pxVertices++;
				pxVerts++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkCompressedVerts
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkCompressedVerts( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices = NULL;
COMPRESSED_VERTS_HEADER*	pxHeader;
SHORT_VERT*		pxCompressed;
int			nVertLoop;
VECT		xRange;
VECT		xBuff;

	pxHeader = (COMPRESSED_VERTS_HEADER*)( pbMem );
	pxCompressed = (SHORT_VERT*)( pxHeader + 1 );
	pxModelData->xStats.nNumVertices = pxHeader->nNumVerts;
	pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;
	xRange.x = pxHeader->xMax.x - pxHeader->xMin.x;
	xRange.y = pxHeader->xMax.y - pxHeader->xMin.y;
	xRange.z = pxHeader->xMax.z - pxHeader->xMin.z;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		if ( pxVertices )
		{
			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
			{
				xBuff.x = (float)( pxCompressed->uwX ) / 65535.0f;
				xBuff.y = (float)( pxCompressed->uwY ) / 65535.0f;
				xBuff.z = (float)( pxCompressed->uwZ ) / 65535.0f;

				pxVertices->position.x = (xBuff.x * xRange.x) + pxHeader->xMin.x;
				pxVertices->position.y = (xBuff.y * xRange.y) + pxHeader->xMin.y;
				pxVertices->position.z = (xBuff.z * xRange.z) + pxHeader->xMin.z;
				pxVertices->color = 0xFFFFFFFF;
		 
				pxVertices++;
				pxCompressed++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
}

/***************************************************************************
 * Function    : ModelLoadProcessChunkTexMaterial
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkTexMaterial( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
ModelMaterialData*			pMaterialData;
TEXMATERIAL_CHUNKHEADER*	pHeader;
TEXMATERIAL_CHUNK*			pChunk;
byte*		pbRunner;
int		loop;
char*	szFilename;
byte*	pbFileData;
int		nNumEmbeddedTextures = 0;
int		texchanloop;
int		nTexFilenameSize;
int		nTexFiledataSize;

	pbRunner = pbMem;
	pHeader = (TEXMATERIAL_CHUNKHEADER*)pbRunner;
	pbRunner += sizeof( TEXMATERIAL_CHUNKHEADER );

	if ( pHeader->bTotalNumMaterials != 0 )
	{
		PANIC_IF ( pHeader->bTotalNumMaterials != pxModelData->xStats.nNumMaterials, "Mismatch in material counts?" );

		for ( loop = 0; loop < pHeader->bTotalNumMaterials; loop++ )
		{
			pChunk = (TEXMATERIAL_CHUNK*)pbRunner;
			pbRunner += sizeof( TEXMATERIAL_CHUNK );
		
			if ( pChunk->bTex1Clone != 0 )
			{
			ModelMaterialData*		pSourceMaterialData;

				pMaterialData = new ModelMaterialData;
				pMaterialData->SetAttrib( loop );

				pSourceMaterialData = FindMaterial( pxModelData, pChunk->bTex1Clone - 1 );
				pMaterialData->CloneTextureFromMaterial( DIFFUSEMAP, pSourceMaterialData );

				pMaterialData->SetColour( ModelMaterialData::DIFFUSE, &pChunk->diffuse );
				pMaterialData->SetColour( ModelMaterialData::AMBIENT, &pChunk->ambient );
				pMaterialData->SetColour( ModelMaterialData::SPECULAR, &pChunk->specular );
				pMaterialData->SetColour( ModelMaterialData::EMISSIVE, &pChunk->emissive );
				pMaterialData->SetSpecularPower( pChunk->fSpecularPower );
				pMaterialData->SetBlendType( (ModelMaterialData::eBLEND_TYPES)pChunk->bBlendType );
				if ( pChunk->bFlags == 0 )
				{
					pMaterialData->SetActiveMaterialProperties( false );
				}
				else
				{
					pMaterialData->SetActiveMaterialProperties( true );
				}
	//			pChunk->bNumTextures = 1;

				// Add to the modeldata linked list of materials
				pMaterialData->SetNext(pxModelData->pMaterialData);
				pxModelData->pMaterialData = pMaterialData;		
			}
			else
			{
				pMaterialData = new ModelMaterialData;
				pMaterialData->SetAttrib( loop );
				pMaterialData->SetColour( ModelMaterialData::DIFFUSE, &pChunk->diffuse );
				pMaterialData->SetColour( ModelMaterialData::AMBIENT, &pChunk->ambient );
				pMaterialData->SetColour( ModelMaterialData::SPECULAR, &pChunk->specular );
				pMaterialData->SetColour( ModelMaterialData::EMISSIVE, &pChunk->emissive );
				pMaterialData->SetSpecularPower( pChunk->fSpecularPower );
				pMaterialData->SetBlendType( (ModelMaterialData::eBLEND_TYPES)pChunk->bBlendType );
				if ( pChunk->bFlags == 0 )
				{
					pMaterialData->SetActiveMaterialProperties( false );
				}
				else
				{
					pMaterialData->SetActiveMaterialProperties( true );
				}

				for ( texchanloop = 0; texchanloop < MAX_NUM_TEX_CHANNELS; texchanloop++ )
				{
					switch( texchanloop )
					{
					case 0:
					default:
						nTexFilenameSize = pChunk->nSizeOfTextureFilename;
						nTexFiledataSize = pChunk->nSizeOfTextureData;
						break;
					case 1:
						nTexFilenameSize = pChunk->bSizeOfNormalMapTextureFilename;
						nTexFiledataSize = pChunk->nSizeOfNormalMapTextureData;
						break;
					case 2:
						nTexFilenameSize = pChunk->bSizeOfSpecularMapTextureFilename;
						nTexFiledataSize = pChunk->nSizeOfSpecularMapTextureData;
						break;
					}

					if ( ( nTexFilenameSize > 0 ) &&
						 ( nTexFiledataSize > 0 ) )
					{
						szFilename = (char*)pbRunner;
						pbRunner += nTexFilenameSize;
						pbFileData = pbRunner;
						pbRunner += nTexFiledataSize;
					}
					else
					{
						pbFileData = NULL;
					}
#ifdef TOOL
					if ( pbFileData != NULL )
					{
						// In the modelconv etc, store the source data so it can be re-exported if necc
						pMaterialData->SetSourceData( texchanloop, pbFileData, nTexFiledataSize );
					}
#endif
					if ( pbFileData )
					{
						pMaterialData->LoadTextureFromMem( texchanloop, pbFileData, nTexFiledataSize, szFilename );
						pMaterialData->SetIsEmbeddedTexture( texchanloop, true );
						nNumEmbeddedTextures++;
						if ( nNumEmbeddedTextures == pChunk->bNumTextures )
						{
							break;
						}
					}
				}

				// Add to the modeldata linked list of materials
				pMaterialData->SetNext(pxModelData->pMaterialData);
				pxModelData->pMaterialData = pMaterialData;		
			}
		}
		pxModelData->xStats.nNumEmbeddedTextures = nNumEmbeddedTextures;
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkAttributes
 * Params      : 
 * Description : The attributes chunk is a list of bytes, 1 per face, which defines the 
                 material used for each face. (Hence maximum of 255 materials per model)
 ***************************************************************************/
void	ModelLoadProcessChunkAttributes( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
DWORD*		pxMeshAttributes = NULL;
byte*		pxATMAttribs;
int			nFaceLoop;
int			nNumFaces = (pxModelData->xStats.nNumIndices/3);
byte		bValue;

	pxATMAttribs = (byte*)( pbMem );
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_Discard, (BYTE**)&pxMeshAttributes );

		if ( pxMeshAttributes )
		{
			for ( nFaceLoop = 0; nFaceLoop < nNumFaces; nFaceLoop++ )
			{
				bValue = *pxATMAttribs;
				*pxMeshAttributes = (DWORD)(bValue);

				pxMeshAttributes++;
				pxATMAttribs++;
			}
			pxModelData->pxBaseMesh->UnlockAttributeBuffer();
		}
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkNormals
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkNormals( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices = NULL;
VECT*		pxVerts;
int			nVertLoop;

	pxVerts = (VECT*)( pbMem );
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		if ( pxVertices )
		{
			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
			{
				pxVertices->normal.x = pxVerts->x;
				pxVertices->normal.y = pxVerts->y;
				pxVertices->normal.z = pxVerts->z;
				pxVertices++;
				pxVerts++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkCompressedUVs
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkCompressedUVs( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices = NULL;
byte*		pbUVs;
int			nVertLoop;

	pbUVs = (byte*)( pbMem );
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		if ( pxVertices )
		{
			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
			{
				pxVertices->tu = (float)( *(pbUVs++) ) / 255.0f;	
				pxVertices->tv = (float)( *(pbUVs++) ) / 255.0f;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkUVs
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkUVs( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices = NULL;
float*		pfUVs;
int			nVertLoop;

	pfUVs = (float*)( pbMem );
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		if ( pxVertices )
		{
			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
			{
				pxVertices->tu = *(pfUVs++);	
				pxVertices->tv = *(pfUVs++);
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
}

/***************************************************************************
 * Function    : ModelLoadProcessChunkIndices
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkIndices( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
ushort*		puwIndices = NULL;
ushort*		puwSrcIndices;
int			nIndexLoop;
unsigned int*		punIndices = NULL;
unsigned int*		punSrcIndices;

	if ( pxModelData->xStats.nNumVertices >= 65536 )
	{
		punSrcIndices = (unsigned int*)( pbMem );
		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &punIndices ) );

			if ( punIndices )
			{
				for ( nIndexLoop = 0; nIndexLoop < pxModelData->xStats.nNumIndices; nIndexLoop++ )
				{
					*(punIndices++) = *(punSrcIndices++);
				}
				pxModelData->pxBaseMesh->UnlockIndexBuffer();
			}
		}
	}
	else
	{
		puwSrcIndices = (ushort*)( pbMem );
		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );

			if ( puwIndices )
			{
				for ( nIndexLoop = 0; nIndexLoop < pxModelData->xStats.nNumIndices; nIndexLoop++ )
				{
					if ( *puwSrcIndices >= pxModelData->xStats.nNumVertices )
					{
						*(puwIndices++) = 0;
						puwSrcIndices++;
					}
					else
					{
						*(puwIndices++) = *(puwSrcIndices++);
					}
				}
				pxModelData->pxBaseMesh->UnlockIndexBuffer();
			}
		}
	}
}



void	ModelLoadProcessChunkCompressedNormalKeyframes( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices;
VECT*		pxVerts;
int			nVertLoop;
//ushort*		puwIndices;
int		nMemSize;
COMPRESSED_VERTS_HEADER*	pxHeader;
int		nVertsTotal;
VECT		xBuff;
SHORT_VERT*	pxCompressed;
int			nLoop;

	pxHeader = (COMPRESSED_VERTS_HEADER*)( pbMem );

	pxModelData->xStats.nNumVertKeyframes = pxHeader->uwNumKeyframes;
	pxModelData->xStats.nNumVertices = pxHeader->nNumVerts;
	pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;

	nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;
	nMemSize = nVertsTotal * sizeof(VECT);
	pxVerts = (VECT*)( SystemMalloc( nMemSize ) );
	pxModelData->pxNormalKeyframes = pxVerts;

	// Uncompress the short vert data into the keyframe buffer
	pxCompressed = (SHORT_VERT*)( pxHeader + 1 );

	for( nLoop = 0; nLoop < nVertsTotal; nLoop++ )
	{
		xBuff.x = (float)( pxCompressed->uwX ) / 65535.0f;
		xBuff.y = (float)( pxCompressed->uwY ) / 65535.0f;
		xBuff.z = (float)( pxCompressed->uwZ ) / 65535.0f;

		pxVerts->x = (xBuff.x * 2.0f) - 1.0f;
		pxVerts->y = (xBuff.y * 2.0f) - 1.0f;
		pxVerts->z = (xBuff.z * 2.0f) - 1.0f;
		 
		pxCompressed++;
		pxVerts++;
	}

	pxVerts = pxModelData->pxNormalKeyframes;
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
		{
			pxVertices->normal.x = pxVerts->x;
			pxVertices->normal.y = pxVerts->y;
			pxVertices->normal.z = pxVerts->z;
			pxVertices++;
			pxVerts += pxModelData->xStats.nNumVertKeyframes;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkCompressedVertexKeyframes
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkCompressedVertexKeyframes( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices;
VECT*		pxVerts;
int			nVertLoop;
//ushort*		puwIndices;
int		nMemSize;
COMPRESSED_VERTS_HEADER*	pxHeader;
int		nVertsTotal;
VECT		xBuff;
VECT		xRange;
SHORT_VERT*	pxCompressed;
int			nLoop;

	pxHeader = (COMPRESSED_VERTS_HEADER*)( pbMem );
	nMemSize = pxHeader->uwNumKeyframes * pxHeader->nNumVerts * sizeof(VECT);
	pxVerts = (VECT*)( SystemMalloc( nMemSize ) );

	pxModelData->pxVertexKeyframes = pxVerts;
	pxModelData->xStats.nNumVertKeyframes = pxHeader->uwNumKeyframes;
	pxModelData->xStats.nNumVertices = pxHeader->nNumVerts;
	pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;

	// Uncompress the short vert data into the keyframe buffer
	nVertsTotal = pxModelData->xStats.nNumVertKeyframes * pxModelData->xStats.nNumVertices;
	pxCompressed = (SHORT_VERT*)( pxHeader + 1 );
	xRange.x = pxHeader->xMax.x - pxHeader->xMin.x;
	xRange.y = pxHeader->xMax.y - pxHeader->xMin.y;
	xRange.z = pxHeader->xMax.z - pxHeader->xMin.z;

	for( nLoop = 0; nLoop < nVertsTotal; nLoop++ )
	{
		xBuff.x = (float)( pxCompressed->uwX ) / 65535.0f;
		xBuff.y = (float)( pxCompressed->uwY ) / 65535.0f;
		xBuff.z = (float)( pxCompressed->uwZ ) / 65535.0f;

		pxVerts->x = (xBuff.x * xRange.x) + pxHeader->xMin.x;
		pxVerts->y = (xBuff.y * xRange.y) + pxHeader->xMin.y;
		pxVerts->z = (xBuff.z * xRange.z) + pxHeader->xMin.z;
		 
		pxCompressed++;
		pxVerts++;
	}

	pxVerts = pxModelData->pxVertexKeyframes;
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
		{
			pxVertices->position.x = pxVerts->x;
			pxVertices->position.y = pxVerts->y;
			pxVertices->position.z = pxVerts->z;
			pxVertices->color = 0xFFFFFFFF;
			pxVertices++;
			pxVerts += pxModelData->xStats.nNumVertKeyframes;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();

	}
}

/***************************************************************************
 * Function    : ModelLoadProcessChunkVerts
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkVertexKeyframes( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices;
VECT*		pxVerts;
int			nVertLoop;
//ushort*		puwIndices;
int		nMemSize;

	nMemSize = pxChunkHeader->nNumElementsInChunk * pxChunkHeader->nSizeOfElement;
	pxVerts = (VECT*)( SystemMalloc( nMemSize ) );
	memcpy( pxVerts, pbMem, nMemSize );
	pxModelData->xStats.nNumVertKeyframes = pxChunkHeader->nSizeOfElement / sizeof( VECT );
	pxModelData->xStats.nNumVertices = pxChunkHeader->nNumElementsInChunk;
	pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;

	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxVertexKeyframes = pxVerts;
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
		{
			pxVertices->position.x = pxVerts->x;
			pxVertices->position.y = pxVerts->y;
			pxVertices->position.z = pxVerts->z;
			pxVertices->color = 0xFFFFFFFF;
			pxVertices++;
			pxVerts += pxModelData->xStats.nNumVertKeyframes;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
}

/***************************************************************************
 * Function    : ModelLoadProcessChunkKeyframeData
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkKeyframeData( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
MODEL_KEYFRAME_DATA*		pxKeyframeData;
int			nLoop;

	pxKeyframeData = (MODEL_KEYFRAME_DATA*)( pbMem );

	for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertKeyframes; nLoop++ )
	{
		pxModelData->axKeyframeData[ nLoop ] = *(pxKeyframeData++);
		// Make a note if the animation includes an 'ANIM_COLLISION'
		// building models use this to react differently when a player hits one
		if ( pxModelData->axKeyframeData[nLoop].bAnimationUse == ANIM_COLLISION )
		{
			pxModelData->xStats.boHasCollision = TRUE;
		}
	}
}

/***************************************************************************
 * Function    : ModelLoadProcessChunkAttachData
 * Params      : 
 * Description : 
 ***************************************************************************/
void ModelLoadProcessChunkAttachData( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
ATTACH_DATA_CHUNK*		pxAttachChunk;
int		nNumAttaches;
int		nLoop;

	pxAttachChunk = (ATTACH_DATA_CHUNK*)( pbMem );
	nNumAttaches = pxChunkHeader->nNumElementsInChunk;

	for ( nLoop = 0; nLoop < nNumAttaches; nLoop++ )
	{
		switch( pxAttachChunk->bAttachType )
		{
		case 0:		// General weapon fire attach point
			pxModelData->xAttachData.xGenericWeaponFireAttach.nAttachVertex = pxAttachChunk->nAttachVertex;
			pxModelData->xAttachData.xGenericWeaponFireAttach.xAttachOffset = pxAttachChunk->xAttachOffset;
			break;
		}
		pxAttachChunk++;
	}

}

/***************************************************************************
 * Function    : ModelLoadProcessChunkEffects
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkEffects( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
EFFECT_DATA_CHUNK*		pxEffect;

	pxEffect = (EFFECT_DATA_CHUNK*)( pbMem );

	ZeroMemory( &pxModelData->xEffectAttachData, sizeof( EFFECT_DATA_CHUNK ) );
	pxModelData->xEffectAttachData.nEffectType = pxEffect->nEffectType;
	if ( pxModelData->xEffectAttachData.nEffectType != 0 )
	{
		pxModelData->xEffectAttachData.nAttachVertex = pxEffect->nEffectAttachVertex;
		pxModelData->xEffectAttachData.xAttachOffset = pxEffect->xEffectAttachOffset;
		pxModelData->xEffectAttachData.ulEffectParam1 = pxEffect->ulEffectParam1;
		pxModelData->xEffectAttachData.ulEffectParam2 = pxEffect->ulEffectParam2;
		pxModelData->bHasEffect = 1;
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkMaterials
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkMaterials( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
GLOBAL_PROPERTIES_CHUNK*		pxMaterials;
CUSTOMVERTEX*		pxVertices;
uint32	ulAlpha;
int		nVertLoop;

	pxMaterials = (GLOBAL_PROPERTIES_CHUNK*)( pbMem );

	ZeroMemory( &pxModelData->xGlobalProperties, sizeof( GLOBAL_PROPERTIES_CHUNK ) );
	memcpy( &pxModelData->xGlobalProperties, pxMaterials, pxChunkHeader->nSizeOfElement );

	if ( pxModelData->xGlobalProperties.bBlendType != 0 )
	{
		ulAlpha = (((uint32)(pxModelData->xGlobalProperties.bOpacity) * 255)/100) << 24;

		if ( pxModelData->pxBaseMesh != NULL )
		{
			pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

			for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
			{
				pxVertices->color &= 0x00FFFFFF;
				pxVertices->color |= ulAlpha;
				pxVertices++;
			}
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
		}
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessChunkKeyframeData
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkCreatorInfo( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CREATOR_INFO_CHUNK*		pxChunk;
char*	pcInfo;
int		nStrLen;

	pxChunk = (CREATOR_INFO_CHUNK*)( pbMem );
	pcInfo = (char*)( pxChunk+1 );
	nStrLen = strlen( pcInfo ); 
	if ( ( nStrLen > 0 ) &&
		 ( nStrLen < 127 ) )
	{
		strcpy( pxModelData->xStats.acCreatorInfo, pcInfo );
	}
}

/***************************************************************************
 * Function    : ModelLoadProcessChunkKeyframeData
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkLODData( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
LOD_DATA_CHUNK*		pxLOD;

	pxLOD = (LOD_DATA_CHUNK*)( pbMem );

	pxModelData->xLodData.fHighDist = pxLOD->fHiDist;
	pxModelData->xLodData.fMedDist = pxLOD->fMedDist;
}

/***************************************************************************
 * Function    : ModelLoadProcessChunkKeyframeData
 * Params      : 
 * Description : 
 ***************************************************************************/
void	ModelLoadProcessChunkTurretData( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
TURRET_DATA_CHUNK*		pxTurret;

	pxTurret = (TURRET_DATA_CHUNK*)( pbMem );

	pxModelData->xHorizTurretData.nAttachVertex = pxTurret->nHorizAttachVertex;
	pxModelData->xHorizTurretData.xAttachOffset = pxTurret->xHorizAttachOffset;

	pxModelData->xVertTurretData.nAttachVertex = pxTurret->nVertAttachVertex;
	pxModelData->xVertTurretData.xAttachOffset = pxTurret->xVertAttachOffset;

	pxModelData->xWheel1AttachData.nAttachVertex = pxTurret->nWheel1AttachVertex;
	pxModelData->xWheel1AttachData.xAttachOffset = pxTurret->xWheel1AttachOffset;
	pxModelData->xWheel2AttachData.nAttachVertex = pxTurret->nWheel2AttachVertex;
	pxModelData->xWheel2AttachData.xAttachOffset = pxTurret->xWheel2AttachOffset;
	pxModelData->xWheel3AttachData.nAttachVertex = pxTurret->nWheel3AttachVertex;
	pxModelData->xWheel3AttachData.xAttachOffset = pxTurret->xWheel3AttachOffset;
	pxModelData->xWheel4AttachData.nAttachVertex = pxTurret->nWheel4AttachVertex;
	pxModelData->xWheel4AttachData.xAttachOffset = pxTurret->xWheel4AttachOffset;
}

void	ModelLoadProcessChunkVertColours( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
CUSTOMVERTEX*	pxVertices;
int			nVertLoop;
uint32*		pulSrcColours;

	pulSrcColours = (uint32*)( pbMem );
	if ( pxModelData->pxBaseMesh != NULL )
	{
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

		for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumVertices; nVertLoop++ )
		{
			pxVertices->color = *pulSrcColours++;
			pxVertices++;
		}
		pxModelData->pxBaseMesh->UnlockVertexBuffer();
	}
}


/***************************************************************************
 * Function    : ModelLoadProcessATMChunkPass3
 * Params      : 
 * Description : Does the second stage processing of the atm file chunks
 ***************************************************************************/
void	ModelLoadProcessATMChunkPass3( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
	switch( pxChunkHeader->uwChunkID )
	{
	case ATM_CHUNKID_COLOURS:
		ModelLoadProcessChunkVertColours( pxModelData, pxChunkHeader, pbMem );
#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("[Pass3] Vert colours chunk processed\n" );
#endif
		break;
	}
}

/***************************************************************************
 * Function    : ModelLoadProcessATMChunkPass2
 * Params      : 
 * Description : Does the second stage processing of the atm file chunks
 ***************************************************************************/
void	ModelLoadProcessATMChunkPass2( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
	switch( pxChunkHeader->uwChunkID )
	{
	case ATM_CHUNKID_COMPRESSED_VERTS:
		ModelLoadProcessChunkCompressedVerts( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_VERTS:
		ModelLoadProcessChunkVerts( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_INDICES:
		ModelLoadProcessChunkIndices( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_TEXMATERIAL:
		ModelLoadProcessChunkTexMaterial( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_ATTRIBUTEDATA:
		ModelLoadProcessChunkAttributes( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_NORMALS:
		ModelLoadProcessChunkNormals( pxModelData, pxChunkHeader, pbMem );
		break; 
	case ATM_CHUNKID_UVS:
		ModelLoadProcessChunkUVs( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_VERY_COMPRESSED_UVS:
		ModelLoadProcessChunkCompressedUVs( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_VERT_KEYFRAMES:
		ModelLoadProcessChunkVertexKeyframes( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_COMPRESSED_VERT_KEYFRAMES:
		ModelLoadProcessChunkCompressedVertexKeyframes( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_COMPRESSED_NORMAL_KEYFRAMES:
		ModelLoadProcessChunkCompressedNormalKeyframes( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_KEYFRAME_DATA:
		ModelLoadProcessChunkKeyframeData( pxModelData, pxChunkHeader, pbMem );	
		break;
	case ATM_CHUNKID_LOD_DATA:
		ModelLoadProcessChunkLODData( pxModelData, pxChunkHeader, pbMem );	
		break;
	case ATM_CHUNKID_ATTACH_DATA:
		ModelLoadProcessChunkAttachData( pxModelData, pxChunkHeader, pbMem );	
		break;
	case ATM_CHUNKID_GLOBALPROPERTIES:
		ModelLoadProcessChunkMaterials( pxModelData, pxChunkHeader, pbMem );	
		break;
	case ATM_CHUNKID_EFFECT_DATA:
		ModelLoadProcessChunkEffects( pxModelData, pxChunkHeader, pbMem );	
		break;
	case ATM_CHUNKID_CREATOR_INFO:
		ModelLoadProcessChunkCreatorInfo( pxModelData, pxChunkHeader, pbMem );	
		break;
	case ATM_CHUNKID_TURRET_DATA:
		ModelLoadProcessChunkTurretData( pxModelData, pxChunkHeader, pbMem );	
		break;
	}
}

/***************************************************************************
 * Function    : ModelLoadProcessATMChunkPass1
 * Params      : 
 * Description : Does the initial processing of the atm file chunks
 ***************************************************************************/
void	ModelLoadProcessATMChunkPass1( MODEL_RENDER_DATA* pxModelData, ATM_CHUNK_HEADER* pxChunkHeader, byte* pbMem )
{
	switch( pxChunkHeader->uwChunkID )
	{
	case ATM_CHUNKID_OLD_BASIC_CONTENTS:
#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("[Pass1] ChunkID : %d : Old Basic Contents", pxChunkHeader->uwChunkID );
#endif
		ModelLoadProcessChunkOldBasicContents( pxModelData, pxChunkHeader, pbMem );
		break;
	case ATM_CHUNKID_BASIC_CONTENTS:
#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("[Pass1] ChunkID : %d : Basic Contents", pxChunkHeader->uwChunkID );
#endif
		ModelLoadProcessChunkBasicContents( pxModelData, pxChunkHeader, pbMem );
		break;
	default:
#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("[Pass1] ChunkID : %d ignored\n", pxChunkHeader->uwChunkID );
#endif
		break;
	}
}


/***************************************************************************
 * Function    : ModelLoadUMAFile
 * Params      : Returns the modelHandle of the model loaded, or NOTFOUND if unsuccessful
 * Description : Attempts to load the specified model
 ***************************************************************************/
BOOL	ModelLoadUMAFile( MODEL_RENDER_DATA* pxModelData, const char* szFilename )
{
	pxModelData->pModelArchive = new ModelArchive;
	pxModelData->pModelArchive->Load( szFilename );
	return( TRUE );
}


void	ModelLoadVertexRemapCallback( void* pModelRenderData, void* pVertexRemapData )
{
MODEL_RENDER_DATA*		pxModelData = (MODEL_RENDER_DATA*)( pModelRenderData );
VECT*		pxVertexBuffer;
VECT*		pxNormalBuffer;
VECT*		pxVertexSource;
VECT*		pxNormalSource;
int			nLoop;
int			nLoop2;
DWORD*		pulVertexRemap = (DWORD*)pVertexRemapData;
int			nNewVertexIndex;

	if ( pxModelData->xStats.nNumVertKeyframes > 0 )
	{
		pxVertexBuffer =(VECT*)(  SystemMalloc( sizeof( VECT ) * pxModelData->xStats.nNumVertices ) );
		pxNormalBuffer =(VECT*)(  SystemMalloc( sizeof( VECT ) * pxModelData->xStats.nNumVertices ) );

		pxVertexSource = pxModelData->pxVertexKeyframes;
		if ( pxVertexSource = NULL )
		{
			return;
		}

		pxNormalSource = pxModelData->pxNormalKeyframes;
		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertKeyframes; nLoop++ )
		{
			for ( nLoop2 = 0; nLoop2 < pxModelData->xStats.nNumVertices; nLoop2++ )
			{
				nNewVertexIndex = pulVertexRemap[nLoop2];
				pxVertexBuffer[nLoop2] = pxVertexSource[(nNewVertexIndex*pxModelData->xStats.nNumVertKeyframes)+nLoop];
				if ( pxNormalSource )
				{
					pxNormalBuffer[nLoop2] = pxNormalSource[(nNewVertexIndex*pxModelData->xStats.nNumVertKeyframes)+nLoop];
				}
			}

			for ( nLoop2 = 0; nLoop2 < pxModelData->xStats.nNumVertices; nLoop2++ )
			{
				pxVertexSource[(nLoop2*pxModelData->xStats.nNumVertKeyframes)+nLoop] = pxVertexBuffer[nLoop2];
				if ( pxNormalSource )
				{
					pxNormalSource[(nLoop2*pxModelData->xStats.nNumVertKeyframes)+nLoop] = pxNormalBuffer[nLoop2];
				}
			}
//			memcpy( pxVertexSourceRunner, pxVertexBuffer, sizeof(VECT)* pxModelData->xStats.nNumVertices );
//			pxVertexSourceRunner += pxModelData->xStats.nNumVertices;
		}

		SystemFree( pxVertexBuffer );
		SystemFree( pxNormalBuffer );
	}
/*
	VECT*			pxVertexKeyframes;		// Stores the list of keyframes for a vertex-keyframe animation
	VECT*			pxNormalKeyframes;		// Stores the list of normal keyframes for a vertex-keyframe animation

	MODEL_STATS		xStats;				// Details about the model such as the number of vertices and normals etc.

	MODEL_KEYFRAME_DATA		axKeyframeData[MAX_KEYFRAMES_IN_MODEL];		// Information about each keyframe, such as the blend time and the animation it is used by
*/
}

void	ModelLoadCalcTangents( MODEL_RENDER_DATA* pxModelData )
{
CUSTOMVERTEX*		pxVertices = NULL;
int					nLoop;
ushort*				puwIndices = NULL;
CUSTOMVERTEX*		pxVert0;
CUSTOMVERTEX*		pxVert1;
CUSTOMVERTEX*		pxVert2;
VECT		xEdge1;
VECT		xEdge2;
float DeltaU1;
float DeltaV1;
float DeltaU2;
float DeltaV2;
float	f;
VECT	xTangent;

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	pxModelData->pxBaseMesh->LockIndexBuffer( kLock_ReadOnly, (byte**)( &puwIndices ) );
	if ( pxVertices != NULL )
	{
		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumIndices; nLoop += 3 )
		{
			pxVert0 = pxVertices + puwIndices[ nLoop ];
			pxVert1 = pxVertices + puwIndices[ nLoop+1 ];
			pxVert2 = pxVertices + puwIndices[ nLoop+2 ];

			VectSub( &xEdge1, &pxVert1->position, &pxVert0->position );
			VectSub( &xEdge2, &pxVert2->position, &pxVert0->position );

			DeltaU1 = pxVert1->tu - pxVert0->tu;
			DeltaV1 = pxVert1->tv - pxVert0->tv;
			DeltaU2 = pxVert2->tu - pxVert0->tu;
			DeltaV2 = pxVert2->tv - pxVert0->tv;

			f = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

		    xTangent.x = f * (DeltaV2 * xEdge1.x - DeltaV1 * xEdge2.x);
			xTangent.y = f * (DeltaV2 * xEdge1.y - DeltaV1 * xEdge2.y);
			xTangent.z = f * (DeltaV2 * xEdge1.z - DeltaV1 * xEdge2.z);

			pxVert0->tangent = xTangent;
			pxVert1->tangent = xTangent;
			pxVert2->tangent = xTangent;
		}

		for ( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
		{
			VectNormalize( &pxVertices[nLoop].tangent );
		}

		pxModelData->pxBaseMesh->UnlockVertexBuffer();
		pxModelData->pxBaseMesh->UnlockIndexBuffer();
	}
}


/***************************************************************************
 * Function    : ModelLoadATMFile
 * Params      : Returns the modelHandle of the model loaded, or NOTFOUND if unsuccessful
 * Description : Attempts to load the specified model
 ***************************************************************************/
BOOL	ModelLoadATMFile( MODEL_RENDER_DATA* pxModelData, const char* szFilename, byte* pbMem )
{
ATM_FILE_HEADER		xFileHeader;
ATM_CHUNK_HEADER*	pxChunkHeader;
ATM_CHUNK_HEADER*	pxLastChunkHeader;
byte*	pbFileInMem;
byte*	pbSrcPtr;
byte*	pbChunksBase;
int		nHandle;
MODEL_RENDER_DATA*		pxCurrentSubModelData;
BOOL	boMoreSubModels = TRUE;
int		nProcessedBytes = 0;
char	acString[256];
int		nSubModelNum = 0;
MODEL_RENDER_DATA*		pxParentModelData = pxModelData;
byte*	pbDecompressionBuffer = NULL;
int		nChunkSize;

	// if the file could be opened
	if ( pbMem != NULL )
	{
		// Get the size of the tex
		pbFileInMem = pbMem;

		xFileHeader = *( (ATM_FILE_HEADER*)( pbFileInMem ) );
		pbSrcPtr = pbFileInMem + xFileHeader.bSizeOfFileHeader;
	
		// Make sure the info in the file header is valid
		if ( xFileHeader.uwFileHeader != ATM_CHUNK_FILE_HEADER_CODE )
		{
			sprintf( acString, "*** ERROR in .atm model file %s.\n Invalid file header - aborting load", szFilename );
			SysDebugPrint( acString );

//				PANIC_IF( TRUE, acString );
			return( FALSE );
		}
			// Check the version num here etc..

		// Check if compressed..
		if ( xFileHeader.bCompressedFlag == 1 )
		{
		uLongf	nUncompressedSize;
		byte*	pbUncompressedMem;
		int		ret;

			nUncompressedSize = xFileHeader.ulUncompressedSize;
			if ( nUncompressedSize < (uint32)pxModelData->xStats.nFileSize )
			{
				nUncompressedSize = pxModelData->xStats.nFileSize;
			}
			if ( nUncompressedSize < (10*1024) )
			{
				nUncompressedSize = 10*1024;
			}

			pbUncompressedMem = (byte*)SystemMalloc( nUncompressedSize + sizeof(ATM_CHUNK_HEADER) );
			// mm.. for some reason we can get crashes if we don't clear out this entire buffer
			// might need some investigation...
			memset( pbUncompressedMem, 0, nUncompressedSize + sizeof(ATM_CHUNK_HEADER) );

			ret = uncompress(pbUncompressedMem, &nUncompressedSize, pbSrcPtr, pxModelData->xStats.nFileSize - sizeof(ATM_FILE_HEADER) );

			PANIC_IF( ret != Z_OK, "Uncompress error loading atm file");
			// If game, we should save the uncompressed model over the original here, so we dont have to keep 
			// decompressing it each time.

			pxModelData->xStats.nFileSize = nUncompressedSize + sizeof( ATM_FILE_HEADER );
			pbSrcPtr = pbUncompressedMem;
			pbDecompressionBuffer = pbUncompressedMem;
			pbFileInMem = pbUncompressedMem;
#ifdef ATM_LOADER_VERBOSE_DEBUG
			SysDebugPrint("Uncompress complete: %d\n", nUncompressedSize );
#endif
		}

		// Read the chunk header
		pbChunksBase = pbSrcPtr;
		pxCurrentSubModelData = pxModelData;

		while ( boMoreSubModels == TRUE )
		{
#ifdef ATM_LOADER_VERBOSE_DEBUG
			SysDebugPrint("----------- Start read chunks\n" );
#endif
			boMoreSubModels = FALSE;
			pbSrcPtr = pbChunksBase;
			nProcessedBytes = xFileHeader.bSizeOfFileHeader;
			// Read the chunk header
			pxChunkHeader = (ATM_CHUNK_HEADER*)( pbSrcPtr );

			// 1st pass through all the chunks
			while ( ( pxChunkHeader->bChunkCode == ATM_CHUNK_HEADER_CODE ) &&
				    ( pxChunkHeader->uwChunkID != ATM_CHUNKID_MODEL_SEPARATOR ) )
			{
				if ( nProcessedBytes + xFileHeader.bSizeOfChunkHeader > pxModelData->xStats.nFileSize )
				{
					sprintf( acString, "*** ERROR in .atm model file %s.\n File data error", szFilename );
					PANIC_IF( TRUE, acString );
					if ( pbDecompressionBuffer )
					{
						SystemFree( pbDecompressionBuffer );
					}
					return( FALSE );
				}
				pbSrcPtr += xFileHeader.bSizeOfChunkHeader;
				nProcessedBytes += xFileHeader.bSizeOfChunkHeader;
//				SysDebugPrint("Processed %d - header at %08x\n", nProcessedBytes, (uint32)( pxChunkHeader ) );
//				SysDebugPrint("Next chunk: num elements %d, sizeof %d\n", pxChunkHeader->nNumElementsInChunk, pxChunkHeader->nSizeOfElement );

				if ( nProcessedBytes + (pxChunkHeader->nNumElementsInChunk * pxChunkHeader->nSizeOfElement) > pxModelData->xStats.nFileSize )
				{
					sprintf( acString, "*** ERROR in .atm model file %s.\n File seems to be incomplete", szFilename );
					PANIC_IF( TRUE, acString );
					if ( pbDecompressionBuffer )
					{
						SystemFree( pbDecompressionBuffer );
					}
					return( FALSE );
				}
				ModelLoadProcessATMChunkPass1( pxCurrentSubModelData, pxChunkHeader, pbSrcPtr );

				if ( ( pxChunkHeader->nNumElementsInChunk < 0 ) ||
					 ( pxChunkHeader->nSizeOfElement < 0 ) )
				{
#ifdef ATM_LOADER_VERBOSE_DEBUG
					SysDebugPrint("Header error\n" );
#endif
					sprintf( acString, "*** ERROR in .atm model file %s.\n Corrupted data?", szFilename );
					PANIC_IF( TRUE, acString );
					if ( pbDecompressionBuffer )
					{
						SystemFree( pbDecompressionBuffer );
					}
					return( FALSE );
				}
				nChunkSize = (pxChunkHeader->nNumElementsInChunk * pxChunkHeader->nSizeOfElement);
				if ( xFileHeader.uwFileVersionNum > 1 )
				{
					if ( (nChunkSize % 4) != 0 )
					{
						nChunkSize += (4 - (nChunkSize%4));
					}
				}
				pbSrcPtr += nChunkSize;
				nProcessedBytes += nChunkSize;
				pxLastChunkHeader = pxChunkHeader;
				pxChunkHeader = (ATM_CHUNK_HEADER*)( pbSrcPtr );
			}

#ifdef ATM_LOADER_VERBOSE_DEBUG
			SysDebugPrint("-- Pass 1 complete\n" );
#endif

			// Check that the model isnt a newer format that we cant load
			if ( pxCurrentSubModelData->bModelType >= MODEL_TYPES_MAX )
			{
				sprintf( acString, "*** .atm model %s uses new formats not yet supported by this program.\n An upgrade may be required", szFilename );
				PANIC_IF( TRUE, acString );
				if ( pbDecompressionBuffer )
				{
					SystemFree( pbDecompressionBuffer );
				}
				return( FALSE );
			}

			nProcessedBytes = xFileHeader.bSizeOfFileHeader;
			pbSrcPtr = pbChunksBase;
			// Read the chunk header
			pxChunkHeader = (ATM_CHUNK_HEADER*)( pbSrcPtr );

			// 2nd pass through all the chunks
			while ( ( pxChunkHeader->bChunkCode == ATM_CHUNK_HEADER_CODE ) &&
			        ( pxChunkHeader->uwChunkID != ATM_CHUNKID_MODEL_SEPARATOR ) )
			{
				if ( nProcessedBytes + xFileHeader.bSizeOfChunkHeader > pxModelData->xStats.nFileSize )
				{
					sprintf( acString, "*** ERROR in .atm model file %s.\n File seems to be incomplete", szFilename );
					PANIC_IF( TRUE, acString );
					if ( pbDecompressionBuffer )
					{
						SystemFree( pbDecompressionBuffer );
					}
					return( FALSE );
				}
				pbSrcPtr += xFileHeader.bSizeOfChunkHeader;
				nProcessedBytes += xFileHeader.bSizeOfChunkHeader;
				if ( nProcessedBytes + (pxChunkHeader->nNumElementsInChunk * pxChunkHeader->nSizeOfElement) > pxModelData->xStats.nFileSize )
				{
					sprintf( acString, "*** ERROR in .atm model file %s.\n File seems to be incomplete", szFilename );
					PANIC_IF( TRUE, acString );
					if ( pbDecompressionBuffer )
					{
						SystemFree( pbDecompressionBuffer );
					}
					return( FALSE );
				}
	
				ModelLoadProcessATMChunkPass2( pxCurrentSubModelData, pxChunkHeader, pbSrcPtr );
				nChunkSize = (pxChunkHeader->nNumElementsInChunk * pxChunkHeader->nSizeOfElement);
				if ( xFileHeader.uwFileVersionNum > 1 )
				{
					if ( (nChunkSize % 4) != 0 )
					{
						nChunkSize += (4 - (nChunkSize%4));
					}
				}
				pbSrcPtr += nChunkSize;
				nProcessedBytes += nChunkSize;
				pxLastChunkHeader = pxChunkHeader;
				pxChunkHeader = (ATM_CHUNK_HEADER*)( pbSrcPtr );
			}

#ifdef ATM_LOADER_VERBOSE_DEBUG
			SysDebugPrint("-- Pass 2 complete\n" );
#endif
			nProcessedBytes = xFileHeader.bSizeOfFileHeader;
			pbSrcPtr = pbChunksBase;
			// Read the chunk header
			pxChunkHeader = (ATM_CHUNK_HEADER*)( pbSrcPtr );

			// 3rd pass through all the chunks
			while ( ( pxChunkHeader->bChunkCode == ATM_CHUNK_HEADER_CODE ) &&
			        ( pxChunkHeader->uwChunkID != ATM_CHUNKID_MODEL_SEPARATOR ) )
			{
				if ( nProcessedBytes + xFileHeader.bSizeOfChunkHeader > pxModelData->xStats.nFileSize )
				{
					sprintf( acString, "*** ERROR in .atm model file %s.\n File seems to be incomplete", szFilename );
					PANIC_IF( TRUE, acString );
					if ( pbDecompressionBuffer )
					{
						SystemFree( pbDecompressionBuffer );
					}
					return( FALSE );
				}
				pbSrcPtr += xFileHeader.bSizeOfChunkHeader;
				nProcessedBytes += xFileHeader.bSizeOfChunkHeader;
				if ( nProcessedBytes + (pxChunkHeader->nNumElementsInChunk * pxChunkHeader->nSizeOfElement) > pxModelData->xStats.nFileSize )
				{
					sprintf( acString, "*** ERROR in .atm model file %s.\n File seems to be incomplete", szFilename );
					PANIC_IF( TRUE, acString );
					if ( pbDecompressionBuffer )
					{
						SystemFree( pbDecompressionBuffer );
					}
					return( FALSE );
				}
	
				ModelLoadProcessATMChunkPass3( pxCurrentSubModelData, pxChunkHeader, pbSrcPtr );
				nChunkSize = (pxChunkHeader->nNumElementsInChunk * pxChunkHeader->nSizeOfElement);
				if ( xFileHeader.uwFileVersionNum > 1 )
				{
					if ( (nChunkSize % 4) != 0 )
					{
						nChunkSize += (4 - (nChunkSize%4));
					}
				}
				pbSrcPtr += nChunkSize;
				nProcessedBytes += nChunkSize;
				pxLastChunkHeader = pxChunkHeader;
				pxChunkHeader = (ATM_CHUNK_HEADER*)( pbSrcPtr );
			}
#ifdef ATM_LOADER_VERBOSE_DEBUG
			SysDebugPrint("-- Pass 3 complete\n" );
#endif
			// If we have materials, we need to generate an adjacency table and reorder the verts so they're
			// grouped by material. (Otherwise it renders like a dog)
			if ( pxCurrentSubModelData->xStats.nNumMaterials > 1 )
			{
				pxCurrentSubModelData->pxBaseMesh->ReorderByMaterial( ModelLoadVertexRemapCallback, pxCurrentSubModelData );
				pxCurrentSubModelData->xStats.nNumVertices = pxCurrentSubModelData->pxBaseMesh->GetNumVertices();
				pxCurrentSubModelData->xStats.nNumIndices = pxCurrentSubModelData->pxBaseMesh->GetNumFaces() * 3;
			}

			// Calculate tangents for used in normal mapping
			ModelLoadCalcTangents( pxCurrentSubModelData );

			// If the last chunk was a model separator, it indicates that a new set of 'sub-model' chunks follow
			if ( pxChunkHeader->uwChunkID == ATM_CHUNKID_MODEL_SEPARATOR )
			{
#ifdef ATM_LOADER_VERBOSE_DEBUG
				SysDebugPrint("-- SubModel separator found\n" );
#endif
				if ( nProcessedBytes + xFileHeader.bSizeOfChunkHeader > pxModelData->xStats.nFileSize )
				{
					sprintf( acString, "*** ERROR in .atm model file %s.\n File separator chunk error", szFilename );
					PANIC_IF( TRUE, acString );
					if ( pbDecompressionBuffer )
					{
						SystemFree( pbDecompressionBuffer );
					}
					return( FALSE );
				}
				pbSrcPtr += xFileHeader.bSizeOfChunkHeader;
				nProcessedBytes += xFileHeader.bSizeOfChunkHeader;

				pbChunksBase = pbSrcPtr;
				boMoreSubModels = TRUE;
				nHandle = ModelRenderGetNextHandle();
				pxCurrentSubModelData = &maxModelRenderData[ nHandle ];
				nSubModelNum = pxChunkHeader->bChunkDataNum;

				switch( pxChunkHeader->bChunkDataNum )
				{
				case 1:
				default:
					pxParentModelData->xHorizTurretData.nModelHandle = nHandle;
					pxParentModelData->xStats.bHasHorizTurret = TRUE;
					break;
				case 2:
			 		pxParentModelData->xVertTurretData.nModelHandle = nHandle;
					break;
				case 3:
					pxParentModelData->xWheel1AttachData.nModelHandle = nHandle;
					pxParentModelData->xWheel2AttachData.nModelHandle = nHandle;
					break;
				case 4:
					pxParentModelData->xWheel3AttachData.nModelHandle = nHandle;
					pxParentModelData->xWheel4AttachData.nModelHandle = nHandle;
					break;
				case 5:
			 		pxParentModelData->xMedLODAttachData.nModelHandle = nHandle;
					pxParentModelData->xStats.bNumLODs++;
					pxParentModelData = &maxModelRenderData[ nHandle ];
					break;
				case 6:
					pxParentModelData = pxModelData;
			 		pxParentModelData->xLowLODAttachData.nModelHandle = nHandle;
					pxParentModelData->xStats.bNumLODs++;
					pxParentModelData = &maxModelRenderData[ nHandle ];
					break;
				case 7:
			 		pxParentModelData->xCollisionAttachData.nModelHandle = nHandle;
					pxParentModelData->xStats.nNumCollisionMaps++;
					break;
				}
				pxParentModelData->xStats.nNumSubmodels++;
			}
		}

#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("Free decompress buffer\n" );
#endif
		if ( pbDecompressionBuffer )
		{
			SystemFree( pbDecompressionBuffer );
		}

		// Wrap up a few stats
		if ( ( pxParentModelData->xWheel1AttachData.nModelHandle != NOTFOUND ) &&
			 ( pxParentModelData->xWheel3AttachData.nModelHandle != NOTFOUND ) )
		{
			pxParentModelData->xStats.bHasWheels = TRUE;
		}

		// TODO - Count number of animations?

#ifdef ATM_LOADER_VERBOSE_DEBUG
		SysDebugPrint("modeload complete\n" );
#endif
		return( TRUE );
	}
	return( FALSE );
}



//----------------------------------------------
// ModelCreateCustomRenderer
//  A convenience function.. Game can create a model handle with a 'custom renderer'.. Nothing is loaded but the model handle can then be stored and
// used in the normal ways, same as a proper model.. when its rendered the custom function is called instead
//----------------------------------------------
int		ModelCreateCustomRenderer( const char* szName, fnCustomMeshRenderer fnCustomRenderer, uint32 ulCreateParam )
{
MODEL_RENDER_DATA*		pxModelData;
int		nHandle;

	nHandle = ModelRenderGetNextHandle();
	
	if ( nHandle != NOTFOUND )
	{
		pxModelData = &maxModelRenderData[ nHandle ];
		pxModelData->pfnCustomMeshRenderer = fnCustomRenderer;
	}
	return( nHandle );	
}

/***************************************************************************
 * Function    : ModelLoadFromMem
 * Params      : Returns the modelHandle of the model loaded, or NOTFOUND if unsuccessful
 * Description : Attempts to load the specified model
 ***************************************************************************/
int		ModelLoadFromMem( const char* szFilename, uint32 ulLoadFlags, float fScale, byte* pbMem, int nMemSize )
{
MODEL_RENDER_DATA*		pxModelData;
int		nHandle;
BOOL	bLoadSuccess = FALSE;

	nHandle = ModelRenderGetNextHandle();
	
	if ( nHandle != NOTFOUND )
	{
		pxModelData = &maxModelRenderData[ nHandle ];

		pxModelData->xStats.nFileSize = nMemSize;
		pxModelData->xGlobalProperties.bOpacity = 100;
		strncpy( pxModelData->xStats.acFilename, szFilename, 255 );
		pxModelData->xStats.acFilename[255] = 0;

		if ( ( stricmp( &szFilename[ strlen(szFilename) - 4 ], ".atm" ) == 0 ) ||
			 ( stricmp( &szFilename[ strlen(szFilename) - 4 ], ".dat" ) == 0 ) )
		{
			// Load the .atm model file
			if ( ModelLoadATMFile( pxModelData, szFilename, pbMem ) == TRUE )
			{
				bLoadSuccess = TRUE;
				if ( fScale != 1.0f )
				{
					ModelScale( nHandle, fScale, fScale, fScale );
				}
			}
		}
		else if ( stricmp( &szFilename[ strlen(szFilename) - 4 ], ".uma" ) == 0 )
		{
			// Load the .atm model file
			if ( ModelLoadUMAFile( pxModelData, szFilename ) == TRUE )
			{
				bLoadSuccess = TRUE;
			}
		}
		else if ( stricmp( &szFilename[ strlen(szFilename) - 4 ], ".obj" ) == 0 )
		{
#ifndef IW_SDK
			// Load the .obj model file
			if ( ModelLoadOBJFile( pxModelData, szFilename, pbMem ) == TRUE )
			{
				bLoadSuccess = TRUE;
			}
#endif
		}
#ifdef TOOL			// Only require the fbx lib & dll for the tool
		else if ( stricmp( &szFilename[ strlen(szFilename) - 4 ], ".fbx" ) == 0 )
		{
			// Load the .fbx model file
			if ( ModelLoadFBXFile( pxModelData, szFilename, pbMem ) == TRUE )
			{
				bLoadSuccess = TRUE;
			}
		}
#endif
		// If the filename ends with x, assume its an x model and try to load it
		else if ( ( szFilename[ strlen(szFilename) - 1 ] == 'x' ) ||
				  ( szFilename[ strlen(szFilename) - 1 ] == 'X' ) )
		{	
			// Standard scaling applied to all x and 3d imports
			if ( fScale == 1.0f ) fScale = 0.0035f;

#ifdef ENGINEDX
			bLoadSuccess = ModelCreateFromXFile( pxModelData, szFilename, fScale );

			if ( bLoadSuccess )
			{
				// If we have materials, we need to generate an adjacency table and reorder the verts so they're
				// grouped by material. (Otherwise it renders like a dog)
				if ( pxModelData->xStats.nNumMaterials > 1 )
				{
					pxModelData->pxBaseMesh->ReorderByMaterial( ModelLoadVertexRemapCallback, pxModelData );
				}
			}
#endif
		}
		// If the filename ends with 3ds
		else if ( stricmp( &szFilename[ strlen(szFilename) - 4 ], ".3ds" ) == 0 )
		{	
			// Standard scaling applied to all x and 3d imports
			if ( fScale == 1.0f ) fScale = 0.0035f;
			ModelLoad3ds( pxModelData, szFilename );

			pxModelData->xStats.nNumVertices = pxModelData->pxBaseMesh->GetNumVertices();
			pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;
			pxModelData->xStats.nNumIndices = pxModelData->pxBaseMesh->GetNumFaces() * 3;
			bLoadSuccess = TRUE;
		}
#ifdef ENGINEDX
		else if ( stricmp( &szFilename[ strlen(szFilename) - 4 ], ".bsp" ) == 0 )
		{	
			pxModelData->pBSPModel = new BSPModel;
			pxModelData->pBSPModel->Load( szFilename );
			return( nHandle );
		}
#endif
		if ( ( bLoadSuccess ) &&
			 ( ( pxModelData->pxBaseMesh != NULL ) || (pxModelData->pModelArchive != NULL) || (pxModelData->pSkinnedModel != NULL) ) )
		{
		CUSTOMVERTEX*	pxVertices = NULL;

			if ( pxModelData->xStats.nNumMaterials < 1 ) pxModelData->xStats.nNumMaterials = 1;
			strcpy( pxModelData->xStats.acFilename, szFilename );
			if ( pxModelData->pxBaseMesh )
			{
				// Calculate the bounding box for the model
				pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );

				if ( pxVertices != NULL )
				{
					RenderingComputeBoundingBox( pxVertices, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
					RenderingComputeBoundingSphere( pxVertices, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );
					pxModelData->xStats.xBoundBoxExtents.x = (pxModelData->xStats.xBoundMax.x - pxModelData->xStats.xBoundMin.x)*0.5f;
					pxModelData->xStats.xBoundBoxExtents.y = (pxModelData->xStats.xBoundMax.y - pxModelData->xStats.xBoundMin.y)*0.5f;
					pxModelData->xStats.xBoundBoxExtents.z = (pxModelData->xStats.xBoundMax.z - pxModelData->xStats.xBoundMin.z)*0.5f;
					pxModelData->xStats.xBoundBoxCentre.x = (pxModelData->xStats.xBoundBoxExtents.x) + pxModelData->xStats.xBoundMin.x;
					pxModelData->xStats.xBoundBoxCentre.y = (pxModelData->xStats.xBoundBoxExtents.y) + pxModelData->xStats.xBoundMin.y;
					pxModelData->xStats.xBoundBoxCentre.z = (pxModelData->xStats.xBoundBoxExtents.z) + pxModelData->xStats.xBoundMin.z;
					pxModelData->pxBaseMesh->UnlockVertexBuffer();
				}
			}

			if ( pxModelData->xHorizTurretData.nModelHandle != NOTFOUND )
			{
			MODEL_RENDER_DATA*		pxTurretModel;
				pxTurretModel = &maxModelRenderData[ pxModelData->xHorizTurretData.nModelHandle ];
				pxTurretModel->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
				if ( pxVertices != NULL )
				{
					RenderingComputeBoundingBox( pxVertices, pxTurretModel->xStats.nNumVertices,&pxTurretModel->xStats.xBoundMin, &pxTurretModel->xStats.xBoundMax );
					RenderingComputeBoundingSphere( pxVertices, pxTurretModel->xStats.nNumVertices,&pxTurretModel->xStats.xBoundSphereCentre, &pxTurretModel->xStats.fBoundSphereRadius );
					pxTurretModel->pxBaseMesh->UnlockVertexBuffer();
				}					
			}
			if ( pxModelData->xVertTurretData.nModelHandle != NOTFOUND )
			{
			MODEL_RENDER_DATA*		pxTurretModel;
				pxTurretModel = &maxModelRenderData[ pxModelData->xVertTurretData.nModelHandle ];
				pxTurretModel->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
				if ( pxVertices != NULL )
				{
					RenderingComputeBoundingBox( pxVertices, pxTurretModel->xStats.nNumVertices,&pxTurretModel->xStats.xBoundMin, &pxTurretModel->xStats.xBoundMax );
					RenderingComputeBoundingSphere( pxVertices, pxTurretModel->xStats.nNumVertices,&pxTurretModel->xStats.xBoundSphereCentre, &pxTurretModel->xStats.fBoundSphereRadius );
					pxTurretModel->pxBaseMesh->UnlockVertexBuffer();
				}					
			}
			return( nHandle );
		}
	}
	return( NOTFOUND );
}

void		ModelRecalcBounds( int nModelHandle )
{
MODEL_RENDER_DATA*		pxModelData;
CUSTOMVERTEX*	pxVertices = NULL;

	if ( nModelHandle != NOTFOUND )
	{
		pxModelData = &maxModelRenderData[ nModelHandle ];
		pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
		if ( pxVertices != NULL )
		{
			RenderingComputeBoundingBox( pxVertices, pxModelData->xStats.nNumVertices,&pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
			RenderingComputeBoundingSphere( pxVertices, pxModelData->xStats.nNumVertices,&pxModelData->xStats.xBoundSphereCentre, &pxModelData->xStats.fBoundSphereRadius );
			pxModelData->pxBaseMesh->UnlockVertexBuffer();
			pxModelData->xStats.xBoundBoxExtents.x = (pxModelData->xStats.xBoundMax.x - pxModelData->xStats.xBoundMin.x)*0.5f;
			pxModelData->xStats.xBoundBoxExtents.y = (pxModelData->xStats.xBoundMax.y - pxModelData->xStats.xBoundMin.y)*0.5f;
			pxModelData->xStats.xBoundBoxExtents.z = (pxModelData->xStats.xBoundMax.z - pxModelData->xStats.xBoundMin.z)*0.5f;
			pxModelData->xStats.xBoundBoxCentre.x = (pxModelData->xStats.xBoundBoxExtents.x) + pxModelData->xStats.xBoundMin.x;
			pxModelData->xStats.xBoundBoxCentre.y = (pxModelData->xStats.xBoundBoxExtents.y) + pxModelData->xStats.xBoundMin.y;
			pxModelData->xStats.xBoundBoxCentre.z = (pxModelData->xStats.xBoundBoxExtents.z) + pxModelData->xStats.xBoundMin.z;
		}					
	}
}


int		ModelCreate( int nNumFaces, int nNumVerts, int nFlags )
{
int		nHandle = ModelRenderGetNextHandle();
MODEL_RENDER_DATA*		pxModelData;

	if ( nHandle != NOTFOUND )
	{
		pxModelData = &maxModelRenderData[ nHandle ];

		EngineCreateMesh( nNumFaces, nNumVerts, &pxModelData->pxBaseMesh, nFlags );

		pxModelData->bModelType = ASSETTYPE_STATIC_MESH;
		pxModelData->xGlobalProperties.bOpacity = 100;
		pxModelData->xStats.nNumIndices = nNumFaces * 3;
		pxModelData->xStats.nNumMaterials = 0;
		pxModelData->xStats.nNumVertices = nNumVerts;
	}
	return( nHandle );
}

/***************************************************************************
 * Function    : ModelLoad
 * Params      : Returns the modelHandle of the model loaded, or NOTFOUND if unsuccessful
 * Description : Attempts to load the specified model
 ***************************************************************************/
int		ModelLoad( const char* szFilename, uint32 ulLoadFlags, float fScale )
{
int		nHandle;
FILE*	pFile;
int		nFileSize;
BOOL	bLoadSuccess = FALSE;
byte*	pbFileInMem;

	pFile = SysFileOpen( szFilename, "rb" );
	if ( pFile != NULL )
	{
		// Get the size of the tex
		nFileSize = SysGetFileSize(pFile);

		pbFileInMem = (byte*)( SystemMalloc( nFileSize + sizeof(ATM_CHUNK_HEADER) ) );
		if ( pbFileInMem != NULL )
		{
			memset( pbFileInMem + nFileSize, 0, sizeof(ATM_CHUNK_HEADER) );
			SysFileRead( pbFileInMem, nFileSize, 1, pFile );
			SysFileClose( pFile );

			nHandle = ModelLoadFromMem( szFilename, ulLoadFlags, fScale, pbFileInMem, nFileSize );
			SystemFree( pbFileInMem );
			return( nHandle );
		}
	}
	return( NOTFOUND );
}



/***************************************************************************
 * Function    : ModelLoadFromArchive
 * Params      : Returns the modelHandle of the model loaded, or NOTFOUND if unsuccessful
 * Description : Attempts to load the specified model
 ***************************************************************************/
int		ModelLoadFromArchive( const char* szFilename, uint32 ulLoadFlags, float fScale, int nArchiveHandle )
{
int		nHandle;
int		nFileHandle;
int		nFileSize;
BOOL	bLoadSuccess = FALSE;
Archive*		pArchive = ArchiveGetFromHandle( nArchiveHandle );
byte*	pbFileInMem;

	if ( pArchive == NULL )
	{
		return( NOTFOUND );
	}

	nFileHandle = pArchive->OpenFile( szFilename );

	if ( nFileHandle > 0 )
	{
		// Get the size of the tex
		nFileSize = pArchive->GetFileSize( nFileHandle );

		pbFileInMem = (byte*)( SystemMalloc( nFileSize + sizeof(ATM_CHUNK_HEADER) ) );
		if ( pbFileInMem != NULL )
		{
			memset( pbFileInMem + nFileSize, 0, sizeof(ATM_CHUNK_HEADER) );
			pArchive->ReadFile( nFileHandle, pbFileInMem, nFileSize );
			pArchive->CloseFile( nFileHandle );

			nHandle = ModelLoadFromMem( szFilename, ulLoadFlags, fScale, pbFileInMem, nFileSize );
			SystemFree( pbFileInMem );
			return( nHandle );
		}
	}
	return( NOTFOUND );
}


#endif // ndef SERVER


#endif // ndef CONSOLE_SERVER


