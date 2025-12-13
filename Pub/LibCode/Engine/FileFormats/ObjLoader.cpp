
#include <stdio.h>

#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>

#include "../ModelRendering.h"
#include "../ModelMaterialData.h"
#include "../Loader.h"

#include "ObjLoader.h"

class ObjMaterial
{
public:
	ObjMaterial()
	{
		mszMaterialLibName = NULL;
		mnMaterialNum = 0;
		mpModelMaterialData = NULL;
		mpNext = NULL;
	}

	void		InitObjMaterial( int nMaterialNum, const char* szName );
	
	char*					mszMaterialLibName;
	int						mnMaterialNum;
	ModelMaterialData*		mpModelMaterialData;

	ObjMaterial*			mpNext;
};


void	ObjMaterial::InitObjMaterial( int nMaterialNum, const char* szName )
{
	mnMaterialNum = nMaterialNum;
	mszMaterialLibName = (char*)( malloc( strlen( szName ) + 1 ) );
	if ( mszMaterialLibName )
	{
		strcpy( mszMaterialLibName, szName );
	}
	mpModelMaterialData = new ModelMaterialData;
	mpModelMaterialData->SetAttrib( nMaterialNum );

}
	

char	mszObjLoaderMtlLibName[128] = "";

ObjMaterial*		mpObjMaterialList = NULL;

//--------------------------------------------------------------------------------------------
//  .OBJ file loader
int	ModelLoadCountWords( char* pcRunner )
{
int		nNumWords = 0;
char*	pcLastWord = NULL;

	while( ( *pcRunner != 0 ) &&
		   ( *pcRunner != '\r' ) &&
		   ( *pcRunner != '\n' ) )
	{
		if ( *pcRunner == ' ' ) 
		{
			nNumWords++;
			pcLastWord = pcRunner;
		}
		pcRunner++;
	}
	if ( ( pcLastWord ) &&
		 ( pcLastWord + 1 < pcRunner ) )
	{
		nNumWords++;
	}
	return( nNumWords );
}

int	ModelLoadGetNextTextWord( char* pcRunner, char* szOutput, int nMaxSize )
{
int		nNumChars = 0;

	while( ( *pcRunner != 0 ) &&
		   ( *pcRunner != '\r' ) &&
		   ( *pcRunner != '\n' ) &&
		   ( *pcRunner != ' ' ) &&
		   ( nNumChars < (nMaxSize-1) ) )
	{
		*szOutput++ = *pcRunner++;
		nNumChars++;
	}
	*szOutput = 0;
	return( nNumChars );
}

int	ModelLoadGetNextTextLine( char* pcRunner, char* szOutput, int nMaxSize )
{
int		nNumChars = 0;

	while( ( *pcRunner != 0 ) &&
		   ( *pcRunner != '\r' ) &&
		   ( *pcRunner != '\n' ) &&
		   ( nNumChars < (nMaxSize-1) ) )
	{
		*szOutput++ = *pcRunner++;
		nNumChars++;
	}
	*szOutput = 0;
	return( nNumChars );
}


BYTE	ModelLoadObjFindMaterial( const char* pcMaterialName )
{
ObjMaterial*	pObjMaterial = mpObjMaterialList;

	while( pObjMaterial )
	{
		if ( stricmp( pObjMaterial->mszMaterialLibName, pcMaterialName ) == 0 )
		{
			return( (BYTE)pObjMaterial->mnMaterialNum );
		}
		pObjMaterial = pObjMaterial->mpNext;
	}
	return( 0 );
}

					//  TODO - Fill in all the vertices, normals, texture coords
					//      .. then fill in the indices
/*					pxModelData->pxBaseMesh->LockIndexBuffer( NULL, (byte**)( &puwIndices ) );
					for ( nVertLoop = 0; nVertLoop < pxModelData->xStats.nNumIndices; nVertLoop++ )
					{
						*(puwIndices++) = (ushort)(nVertLoop);
					}
					pxModelData->pxBaseMesh->UnlockIndexBuffer();
*/
void	ModelLoadOBJGetFaceIndices( char* pcLine, int* pnVertIndex, int* pnUVIndex, int* pnNormalIndex )
{
char*		pcWord1 = pcLine;
char*		pcWord2;
char*		pcWord3;

	pcWord2 = SplitStringSep( pcWord1, '/' );
	pcWord3 = SplitStringSep( pcWord2, '/' );

	*pnVertIndex = strtol( pcWord1, NULL, 10 );
	*pnUVIndex = strtol( pcWord2, NULL, 10 );
	*pnNormalIndex = strtol( pcWord3, NULL, 10 );
}

void	ModelLoadOBJGetVector3( char* pcLine, VECT* pVect )
{
char*		pcWord1 = pcLine;
char*		pcWord2;
char*		pcWord3;
char*		pcWord4;

	StripWhiteSpaceLeft( pcLine );
	pcWord2 = SplitStringSep( pcWord1, ' ' );
	pcWord3 = SplitStringSep( pcWord2, ' ' );
	pcWord4 = SplitStringSep( pcWord3, ' ' );

	pVect->x = (float)( atof( pcWord1 ) );
	pVect->y = (float)( atof( pcWord2 ) );
	pVect->z = (float)( atof( pcWord3 ) );

}

VECT*	mpxObjLoadFileVertices;
VECT*	mpxObjLoadFileNormals;
VECT*	mpxObjLoadFileUVs;
BYTE*	mpbObjLoadAttributes;

void	ModelLoadOBJFileBuildModel( MODEL_RENDER_DATA* pxModelData, char* pcTextBuff )
{
char	acLine[512];
char	acWord[512];
char*	pcRunner;
char*	pcLineRunner;
int		nCharsRead;
int		nWordCharsRead;
int		nVertexCount = 0;
int		nNormalCount = 0;
int		nUVCount = 0;
int		nFaceCount = 0;
CUSTOMVERTEX*	pxVertices = NULL;
CUSTOMVERTEX*	pxVertexBase = NULL;
DWORD*			pAttributeBuffer = NULL;
ushort*			puwIndices;
unsigned int*	punIndices;
int				nFacesInMesh;
int				nNumIndices;
int				nNumVerticesRead = 0;
int				nNumNormalsRead = 0;
int				nNumUVsRead = 0;
BYTE			bCurrentMaterialNum = 0;

	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	pxVertexBase = pxVertices;
	nFacesInMesh = pxModelData->pxBaseMesh->GetNumFaces();
	nNumIndices = nFacesInMesh * 3;

	if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
	{
		pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &punIndices ) );
	}
	else
	{
		pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );
	}

	pxModelData->pxBaseMesh->LockAttributeBuffer( 0, (byte**)&pAttributeBuffer );

	pcRunner = pcTextBuff;
	while ( *pcRunner != 0 )
	{
		nCharsRead = ModelLoadGetNextTextLine( pcRunner, acLine, 512 );

		if ( nCharsRead > 0 )
		{
			pcLineRunner = acLine;
			// Ignore comment lines
			if ( *pcLineRunner != '#' )
			{
				// Just read the first word on each line
				nWordCharsRead = ModelLoadGetNextTextWord( pcLineRunner, acWord, 512 );
				if ( nWordCharsRead > 0 )
				{
					pcLineRunner += nWordCharsRead;
					if ( *pcLineRunner == ' ' ) 
					{
						pcLineRunner++;
					}
					
					if ( stricmp( acWord, "v" ) == 0 )
					{
						nNumVerticesRead++;
					}
					else if ( stricmp( acWord, "vn" ) == 0 )
					{
						nNumNormalsRead++;
					}
					else if ( stricmp( acWord, "vt" ) == 0 )
					{
						nNumUVsRead++;
					}
					else if ( stricmp( acWord, "usemtl" ) == 0 )
					{
						bCurrentMaterialNum = ModelLoadObjFindMaterial( pcLineRunner );
					}
					else if ( stricmp( acWord, "f" ) == 0 )
					{
					int		nFaceWordLoop;
					char		acIndices1[64];
					int			nFaceCharsRead;
					int			nVertIndex;
					int			nUVIndex;
					int			nNormalIndex;
					int			nNumWords;

						nNumWords = ModelLoadCountWords( pcLineRunner );

						if ( nNumWords == 2 )
						{
							// we dont care about edges

						}
						else
						{
							if ( nNumWords > 4 )
							{
								nNumWords = 3;
							}

							if ( bCurrentMaterialNum > 1 )
							{
								*pAttributeBuffer++ = bCurrentMaterialNum;
							}
							else
							{
								*pAttributeBuffer++ = bCurrentMaterialNum;
							}

							// for each indice referenced
							for ( nFaceWordLoop = 0; nFaceWordLoop < nNumWords; nFaceWordLoop++ )
							{
								nFaceCharsRead = ModelLoadGetNextTextWord( pcLineRunner, acIndices1, 64 );
								pcLineRunner += nFaceCharsRead;
								if ( *pcLineRunner == ' ' )
								{
									pcLineRunner++;
								}

								ModelLoadOBJGetFaceIndices( acIndices1, &nVertIndex, &nUVIndex, &nNormalIndex );

								if ( nVertIndex < 1 )
								{
									nVertIndex = (nNumVerticesRead + nVertIndex) + 1;
									if ( nVertIndex < 1 )
									{
										nVertIndex = 1;
									}
								}
								if ( nUVIndex < 1 )
								{
									nUVIndex = (nNumUVsRead + nUVIndex) + 1;
									if ( nUVIndex < 1 )
									{
										nUVIndex = 1;
									}
								}
								if ( nNormalIndex < 1 )
								{
									nNormalIndex = (nNumNormalsRead + nNormalIndex) + 1;
									if ( nNormalIndex < 1 )
									{
										nNormalIndex = 1;
									}
								}
								pxVertices->position = mpxObjLoadFileVertices[nVertIndex-1];
								pxVertices->normal = mpxObjLoadFileNormals[nNormalIndex-1];
								pxVertices->tu = mpxObjLoadFileUVs[nUVIndex-1].x;
								pxVertices->tv = mpxObjLoadFileUVs[nUVIndex-1].y;
								pxVertices->color = 0xFFFFFFFF;
	 
								// If its a quad
								if ( nFaceWordLoop == 3 )
								{
									if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
									{
										*punIndices++ = nFaceCount - 3;
										*punIndices++ = nFaceCount - 1;
										*punIndices++ = nFaceCount;
									}
									else
									{
										*puwIndices++ = (ushort)( nFaceCount - 3 );
										*puwIndices++ = (ushort)( nFaceCount - 1 );
										*puwIndices++ = (ushort)( nFaceCount );
									}
									*pAttributeBuffer++ = bCurrentMaterialNum;
								}
								else  
								{
									if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() )
									{
										*punIndices++ = nFaceCount;
									}
									else
									{
										*puwIndices++ = (ushort)( nFaceCount );
									}
								}
								nFaceCount++;
								pxVertices++;
							}				
						}
					}
				}
			}
		}

		if ( *pcRunner != 0 )
		{
			pcRunner += nCharsRead+1;
		}
	}
	pxModelData->pxBaseMesh->UnlockIndexBuffer();
	pxModelData->pxBaseMesh->UnlockVertexBuffer();
	pxModelData->pxBaseMesh->UnlockAttributeBuffer();

	free( mpxObjLoadFileVertices );
	free( mpxObjLoadFileNormals );
	free( mpxObjLoadFileUVs );
}



void	ModelLoadOBJFileCountEntries( MODEL_RENDER_DATA* pxModelData, char* pcTextBuff )
{
char	acLine[512];
char	acWord[512];
char*	pcRunner;
char*	pcLineRunner;
int		nCharsRead;
int		nWordCharsRead;
int		nNumWords;
int		nFileNumVertices = 0;
int		nFileNumNormals = 0;
int		nFileNumUVs = 0;
VECT*	pxVertexRunner;
VECT*	pxNormalRunner;
VECT*	pxUVRunner;
VECT	xVect;
char	acMaterialLibName[256] = "";

	pxModelData->xStats.nNumVertices = 0;
	pxModelData->xStats.nNumNormals = 0;
	pxModelData->xStats.nNumUVChannels = 0;
	pxModelData->xStats.nNumIndices = 0;

	// First count the number of vertices, normals and texture coords..
	pcRunner = pcTextBuff;
	while ( *pcRunner != 0 )
	{
		nCharsRead = ModelLoadGetNextTextLine( pcRunner, acLine, 512 );

		if ( nCharsRead > 0 )
		{
			pcLineRunner = acLine;
			// Ignore comment lines
			if ( *pcLineRunner != '#' )
			{
				// Just read the first word on each line
				nWordCharsRead = ModelLoadGetNextTextWord( pcLineRunner, acWord, 512 );
				if ( nWordCharsRead > 0 )
				{
					if ( stricmp( acWord, "v" ) == 0 )
					{
						nFileNumVertices++;
					}
					else if ( stricmp( acWord, "vn" ) == 0 )
					{
						nFileNumNormals++;
					}
					else if ( stricmp( acWord, "vt" ) == 0 )
					{
						nFileNumUVs++;
						pxModelData->xStats.nNumUVChannels = 1;
					}
					else if ( stricmp( acWord, "f" ) == 0 )
					{
						// For faces, we'll need to count the number of entries to determine if they're tris or quads or whateva
						// (For faces its  "f v/vt/vn v/vt/vn v/vt/vn v/vt/vn" )
						pcLineRunner += (nWordCharsRead+1);
						nNumWords = ModelLoadCountWords( pcLineRunner );
						if ( nNumWords == 4 ) // Quads
						{
							pxModelData->xStats.nNumIndices += 6;
							pxModelData->xStats.nNumVertices += 4;
						}
						else // Tris or anything else
						{
							pxModelData->xStats.nNumIndices += 3;
							pxModelData->xStats.nNumVertices += 3;
						}
					}
					else if ( stricmp( acWord, "mtllib" ) == 0 )
					{
						pcLineRunner += (nWordCharsRead+1);
						strncpy( mszObjLoaderMtlLibName, pcLineRunner, 127 );
					}

				}
			}
		}

		if ( *pcRunner != 0 )
		{
			pcRunner += nCharsRead+1;
		}
	}

	if ( nFileNumVertices > pxModelData->xStats.nNumVertices )
	{
		pxModelData->xStats.nNumVertices = nFileNumVertices;
	}

	mpxObjLoadFileVertices = (VECT*)malloc( nFileNumVertices * sizeof( VECT ) );
	mpxObjLoadFileNormals = (VECT*)malloc( nFileNumNormals * sizeof( VECT ) );
	mpxObjLoadFileUVs = (VECT*)malloc( nFileNumUVs * sizeof( VECT ) );
	pxVertexRunner = mpxObjLoadFileVertices;
	pxNormalRunner = mpxObjLoadFileNormals;
	pxUVRunner = mpxObjLoadFileUVs;

	// Now read all the data in
	pcRunner = pcTextBuff;
	while ( *pcRunner != 0 )
	{
		nCharsRead = ModelLoadGetNextTextLine( pcRunner, acLine, 512 );
		if ( nCharsRead > 0 )
		{
			pcLineRunner = acLine;
			// Ignore comment lines
			if ( *pcLineRunner != '#' )
			{
				// Just read the first word on each line
				nWordCharsRead = ModelLoadGetNextTextWord( pcLineRunner, acWord, 512 );
				if ( nWordCharsRead > 0 )
				{
					pcLineRunner += nWordCharsRead;
					if ( *pcLineRunner != ' ' ) 
					{
						pcLineRunner++;
					}

					if ( stricmp( acWord, "v" ) == 0 )
					{
						ModelLoadOBJGetVector3( pcLineRunner, &xVect );
						pxVertexRunner->x = xVect.x;
						pxVertexRunner->y = xVect.y;
						pxVertexRunner->z = xVect.z;
						pxVertexRunner++;
					}
					else if ( stricmp( acWord, "vn" ) == 0 )
					{
						ModelLoadOBJGetVector3( pcLineRunner, &xVect );
						pxNormalRunner->x = xVect.x;
						pxNormalRunner->y = xVect.y;
						pxNormalRunner->z = xVect.z;
						pxNormalRunner++;
//						pxModelData->xStats.nNumNormals++;
					}
					else if ( stricmp( acWord, "vt" ) == 0 )
					{
						ModelLoadOBJGetVector3( pcLineRunner, &xVect );
						pxUVRunner->x = xVect.x;
						if ( xVect.y < 0.0f )
						{
							pxUVRunner->y = 0.0f - xVect.y;
						}
						else
						{
							pxUVRunner->y = 1.0f - xVect.y;
						}
						pxUVRunner++;
//						pxModelData->xStats.nNumUVChannels = 1;
					}
				}
			}
		}

		if ( *pcRunner != 0 )
		{
			pcRunner += nCharsRead+1;
		}
	}

}



int		ModelLoadObjMtlLibParse( char* pcMtlLibFileInMem, const char* szObjFilePath )
{
char	acLine[512];
char	acWord[512];
char*	pcRunner;
char*	pcLineRunner;
int		nCharsRead;
int		nWordCharsRead;
ObjMaterial*	pObjMaterial = NULL;
int		nNumMaterials = 0;

	pcRunner = pcMtlLibFileInMem;
	while ( *pcRunner != 0 )
	{
		nCharsRead = ModelLoadGetNextTextLine( pcRunner, acLine, 512 );
		if ( nCharsRead > 0 )
		{
			pcLineRunner = acLine;
			// Ignore comment lines
			if ( *pcLineRunner != '#' )
			{
				// Just read the first word on each line
				nWordCharsRead = ModelLoadGetNextTextWord( pcLineRunner, acWord, 512 );
				if ( nWordCharsRead > 0 )
				{
					pcLineRunner += nWordCharsRead;
					if ( *pcLineRunner != ' ' ) 
					{
						pcLineRunner++;
					}

					StripWhiteSpaceLeft( acWord );
					StripWhiteSpaceRight( acWord );

					StripWhiteSpaceLeft( pcLineRunner );
					StripWhiteSpaceRight( pcLineRunner );

					if ( stricmp( acWord, "newmtl" ) == 0 )
					{
						pObjMaterial = new ObjMaterial;
						pObjMaterial->InitObjMaterial( nNumMaterials, pcLineRunner );

						pObjMaterial->mpNext = mpObjMaterialList;
						mpObjMaterialList = pObjMaterial;
						nNumMaterials++;
					}
					else if ( stricmp( acWord, "Ka" ) == 0 )
					{

					}
					else if ( stricmp( acWord, "Kd" ) == 0 )
					{

					}
					else if ( stricmp( acWord, "Ks" ) == 0 )
					{

					}
					else if ( stricmp( acWord, "Ns" ) == 0 )
					{

					}
					else if ( stricmp( acWord, "map_Kd" ) == 0 )
					{
					FILE*	pFile;
					BOOL	bTextureFound = FALSE;
					
						// See if the file exists (might be a full path)
						pFile = fopen( pcLineRunner, "rb" );
						if ( pFile != NULL )
						{
							fclose( pFile );
							bTextureFound = TRUE;
						}
						else
						{
						char*	pcFileNameRunner = pcLineRunner + strlen( pcLineRunner );
							// File doesnt exist at given location, try using the file name without the full path
							while( ( pcFileNameRunner > pcLineRunner ) && 
								   ( *pcFileNameRunner != '/' ) &&
								   ( *pcFileNameRunner != '\\' ) )
							{
								pcFileNameRunner--;
							}

							if ( ( *pcFileNameRunner == '\\' ) ||
								 ( *pcFileNameRunner == '/' ) )
							{
							char	acObjRelativePath[256];

								pcFileNameRunner++;
							
								strcpy( acObjRelativePath, szObjFilePath );
								strcat( acObjRelativePath, pcFileNameRunner );

								pFile = fopen( acObjRelativePath, "rb" );
								if ( pFile != NULL )
								{
									fclose( pFile );
									bTextureFound = TRUE;
									pcLineRunner = acObjRelativePath;
								}
								else
								{
									// todo - try looking in 'Textures\\[pcFileNameRunner]' (etc)  ??
								}
							}
						}
						
						if ( bTextureFound )
						{
							pObjMaterial->mpModelMaterialData->LoadTextureFilename( 0, pcLineRunner );
						}
					}
					else if ( stricmp( acWord, "d" ) == 0 )
					{

					}
					else if ( stricmp( acWord, "Tr" ) == 0 )
					{
					}
					else if ( stricmp( acWord, "illum" ) == 0 )
					{
						// not implemented
					}
					else if ( stricmp( acWord, "map_Ks" ) == 0 )
					{
						// not implemented
					}
					else if ( stricmp( acWord, "map_Ka" ) == 0 )
					{
						// not implemented
					}
					else if ( stricmp( acWord, "map_Ns" ) == 0 )
					{
						// not implemented

					}
					else if ( stricmp( acWord, "map_d" ) == 0 )
					{
						// not implemented
					}
					else if ( stricmp( acWord, "map_bump" ) == 0 )
					{
						// not implemented

					}
				}
			}
		}

		if ( *pcRunner != 0 )
		{
			pcRunner += nCharsRead+1;
		}

	}

	return( nNumMaterials );
}


void	ModelLoadObjMtlLib( MODEL_RENDER_DATA* pxModelData, const char* szFilename, const char* szObjFilePath )
{
FILE*		pFile = fopen( szFilename, "rb" );

	if ( pFile )
	{
	int		nFileSize = SysGetFileSize( pFile );
	char*	pcMtlFileMem = (char*)( malloc( nFileSize + 1 ) );

		if ( pcMtlFileMem )
		{
			fread( pcMtlFileMem, nFileSize, 1, pFile );
			fclose( pFile );
			pcMtlFileMem[nFileSize] = 0;

			pxModelData->xStats.nNumMaterials = ModelLoadObjMtlLibParse( pcMtlFileMem, szObjFilePath );

			if ( pxModelData->xStats.nNumMaterials > 0 )
			{
			ObjMaterial*	pObjMaterial = mpObjMaterialList;

				while( pObjMaterial )
				{
					pObjMaterial->mpModelMaterialData->SetNext( pxModelData->pMaterialData );
					pxModelData->pMaterialData = pObjMaterial->mpModelMaterialData;
					pObjMaterial = pObjMaterial->mpNext;
				}
			}

			free( pcMtlFileMem );
		}
	}
	
}


/***************************************************************************
 * Function    : ModelLoadOBJFile
 * Params      : Returns the modelHandle of the model loaded, or NOTFOUND if unsuccessful
 * Description : Attempts to load the specified model
 ***************************************************************************/
BOOL	ModelLoadOBJFile( MODEL_RENDER_DATA* pxModelData, const char* szFilename, byte* pbMem )
{
char*	pcTextBuff;
BOOL	bSuccess = FALSE;

	// if the file could be opened
	if ( pbMem != NULL )
	{
	char	acObjFilePath[256];
	char*	pcPathRunner;

		strcpy( acObjFilePath, szFilename );
		pcPathRunner = acObjFilePath + strlen( acObjFilePath );
		while( ( pcPathRunner != acObjFilePath ) &&
			   ( *pcPathRunner != '\\' ) &&
			   ( *pcPathRunner != '/' ) )
		{
			pcPathRunner--;
		}

		if ( ( *pcPathRunner == '\\' ) ||
			 ( *pcPathRunner == '/' ) )
		{
			pcPathRunner[1] = 0;
		}

		pcTextBuff = (char*)pbMem;	
		
		ModelLoadOBJFileCountEntries( pxModelData, pcTextBuff );

		// If we've got something to work with
		if ( ( pxModelData->xStats.nNumIndices > 0 ) &&
			 ( pxModelData->xStats.nNumVertices > 0 ) )
		{
			if ( mszObjLoaderMtlLibName[0] != 0 )
			{
			char	acFullPath[256];

				strcpy( acFullPath, szFilename );
				pcPathRunner = acFullPath + strlen( acFullPath );
				while( ( pcPathRunner != acFullPath ) &&
					   ( *pcPathRunner != '\\' ) &&
					   ( *pcPathRunner != '/' ) )
				{
					pcPathRunner--;
				}

				if ( ( *pcPathRunner == '\\' ) ||
					 ( *pcPathRunner == '/' ) )
				{
					strcpy( pcPathRunner + 1, mszObjLoaderMtlLibName );
				}
				ModelLoadObjMtlLib( pxModelData, acFullPath, acObjFilePath );
			}

			if ( pxModelData->pxBaseMesh == NULL )
			{
				// Create the mesh here now we know the vertex and face counts..
				if ( pxModelData->xStats.nNumVertices > 65535 )
				{
					EngineCreateMesh( (pxModelData->xStats.nNumIndices / 3), pxModelData->xStats.nNumVertices, &pxModelData->pxBaseMesh, 2 );
				}
				else
				{
					EngineCreateMesh( (pxModelData->xStats.nNumIndices / 3), pxModelData->xStats.nNumVertices, &pxModelData->pxBaseMesh, 1 );
				}
				pxModelData->xStats.nNumNormals = pxModelData->xStats.nNumVertices;

				// now reparse the OBJ file..
				ModelLoadOBJFileBuildModel( pxModelData, pcTextBuff );
			}

			if ( pxModelData->xStats.nNumMaterials == 0 )
			{
				pxModelData->xStats.nNumMaterials = 1;
			}
		}

		return( TRUE );	
	}
	return( FALSE );
}
