
#include <StandardDef.h>
#include <Engine.h>
#include <Rendering.h>

#ifdef TOOL

#include <fbxsdk.h>
//#include <fbxfilesdk/fbxio/fbxiosettings.h>

#include "../ModelFiles.h"
#include "../ModelRendering.h"
#include "../ModelMaterialData.h"
#include "../Loader.h"

#include "fbxLoader.h"

#define		ONE_METRE_IN_GAME		0.04f

int		msnFBXLoaderNumVertices = 0;
int		msnFBXLoaderNumFaces = 0;
int		msnFBXLoaderNumTriangles = 0;

int		msnFBXLoaderNumVerticesRead = 0;
int		msnFBXLoaderNumFacesRead = 0;

CUSTOMVERTEX*	mspxFBXLoadedVertexRunner = NULL;
unsigned int*	mspunFBXLoadedLargeIndicesRunner = NULL;
unsigned short*	mspuwFBXLoadedIndicesRunner = NULL;
int				msnFBXLoadedIndicesBase = 0;

int				msnFBXLoadNextVertexNum = 0;


void	FBXLoaderExtractMaterials( MODEL_RENDER_DATA* pxModelData, FbxNode* pNode )
{
int numKids = pNode->GetChildCount();
FbxNode* pChildNode = 0;
//FbxSurfaceMaterial*	pFBXMaterial; 
int		nLoop;
int		nMaterialCount;
FbxLayerElementArrayTemplate<int> *pFBXMaterialIndices = 0;
unsigned int	nNumFBXMaterialIndices;
int			i;
DWORD*		pxMeshAttributes = NULL;
int			nMaterialLoop = 0;
FbxDouble3		fbxColour;
//MATERIAL_COLOUR		matColour;
//const char*		pcTextureName;
int			nBaseMaterialNum;
int			nAttributesIndexBase = 0;

	for ( i=0 ; i<numKids ; i++)
	{	
		pChildNode = pNode->GetChild(i);
		FbxMesh*	pFBXMesh = pChildNode->GetMesh();

		nMaterialCount = pChildNode->GetMaterialCount();
		nBaseMaterialNum = pxModelData->xStats.nNumMaterials;
		pxModelData->xStats.nNumMaterials += nMaterialCount;

		for ( ; nMaterialLoop < pxModelData->xStats.nNumMaterials; nMaterialLoop++ )
		{
		ModelMaterialData*		pMaterialData;

			pMaterialData = new ModelMaterialData;
			pMaterialData->SetAttrib( nMaterialLoop );
/*
			pFBXMaterial = pChildNode->GetMaterial( nMaterialLoop );
			if (pFBXMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
            {
			FbxSurfacePhong*		pPhongMaterial = (FbxSurfacePhong*)( pFBXMaterial );

				fbxColour = pPhongMaterial->Ambient;
				matColour.fRed = (float)fbxColour[0];
				matColour.fGreen = (float)fbxColour[1];
				matColour.fBlue = (float)fbxColour[2];
				matColour.fAlpha = 1.0f;
				pMaterialData->SetColour( ModelMaterialData::AMBIENT, &matColour );

				fbxColour = pPhongMaterial->Diffuse;
				matColour.fRed = (float)fbxColour[0];
				matColour.fGreen = (float)fbxColour[1];
				matColour.fBlue = (float)fbxColour[2];
				matColour.fAlpha = 1.0f;
				pMaterialData->SetColour( ModelMaterialData::DIFFUSE, &matColour );

				fbxColour = pPhongMaterial->Specular;
				matColour.fRed = (float)fbxColour[0];
				matColour.fGreen = (float)fbxColour[1];
				matColour.fBlue = (float)fbxColour[2];
				matColour.fAlpha = 1.0f;
				pMaterialData->SetColour( ModelMaterialData::SPECULAR, &matColour );

				pcTextureName = pPhongMaterial->sDiffuse;
				pMaterialData->LoadTextureFilename( 0, pcTextureName );
//				pPhongMaterial->Shininess;
			}

//            else if(lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId) )
            {
//			FbxSurfaceLambert*		pLambertMaterial;

			pMaterialData->SetColour( ModelMaterialData::EMISSIVE, &pChunk->emissive );
			pMaterialData->SetSpecularPower( pChunk->fSpecularPower );
			pMaterialData->SetBlendType( (ModelMaterialData::eBLEND_TYPES)pChunk->bBlendType );
*/

			// Add to the modeldata linked list of materials
			pMaterialData->SetNext(pxModelData->pMaterialData);
			pxModelData->pMaterialData = pMaterialData;
		}

		if ( pFBXMesh )
		{
			pFBXMesh->GetMaterialIndices(&pFBXMaterialIndices);
			if ( pFBXMaterialIndices )
			{
				nNumFBXMaterialIndices = pFBXMaterialIndices->GetCount();

				pxModelData->pxBaseMesh->LockAttributeBuffer( kLock_Discard, (BYTE**)&pxMeshAttributes );

				pxMeshAttributes +=	nAttributesIndexBase;

				for ( nLoop = 0; nLoop < (int)nNumFBXMaterialIndices; nLoop++ )
				{
					*pxMeshAttributes = nBaseMaterialNum + pFBXMaterialIndices->GetAt( nLoop );
					pxMeshAttributes++;
				}

				pxModelData->pxBaseMesh->UnlockAttributeBuffer();

				nAttributesIndexBase += nNumFBXMaterialIndices;

			}
		}

		
	}
/*
	for ( nLoop = 0; nLoop < nMaterialCount; nLoop++)
	{
		pMaterial = pNode->GetMaterial(nLoop);
   
		for( nFaceLoop = 0; nFaceLoop < nPolysUsingThisMaterial; nFaceLoop++)
		{
		     printf("%d ",tmpArray->GetAt(i));
		}
	}
*/
/*
	if (mappingMode == FbxLayerElement::eByPolygon )
	{
	int lNbMat = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();

		if (pMaterialLayer->GetReferenceMode() == FbxLayerElement::eIndex ||
			pMaterialLayer->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
		int i;
		int lMaterialIndexCount = pMaterialLayer->GetIndexArray().GetCount();

			for (i = 0; i < lMaterialIndexCount; i++)
			{
			int matID = pMaterialLayer->GetIndexArray().GetAt(i);
        
				if ( matID == -1)
					matID = 0;     
        
				if (matID >= lNbMat)
		          matID = 0;

				spMesh->m_attributeList =  matID;
			}
		}
	}
*/
}


void		FBXLoaderExtractVertex( CUSTOMVERTEX*	pxVertex, FbxMesh*	pFBXMesh, const FbxGeometryElementUV* pFBXUVElement, int nFaceNum, int nPolyVertexNum, FbxAMatrix localTransform )
{
FbxVector4		fbxVec4Normal;
FbxVector2		fbxVec2UV;
FbxVector4		vert;
float		fFBXImportScale = 1.0f;		// (ONE_METRE_IN_GAME);
//FbxAMatrix		fbxAMatrix;
FbxMatrix		fbxMatrix;

//	pFBXMesh->GetPivot( fbxAMatrix );
	fbxMatrix = FbxMatrix( localTransform );

	vert = pFBXMesh->GetControlPointAt( pFBXMesh->GetPolygonVertex( nFaceNum, nPolyVertexNum ) );

	vert = fbxMatrix.MultNormalize( vert );
	
	pxVertex->position.x = (float)vert.mData[0] * fFBXImportScale;
	pxVertex->position.y = (float)vert.mData[1] * fFBXImportScale;
	pxVertex->position.z = (float)vert.mData[2] * fFBXImportScale;

	pFBXMesh->GetPolygonVertexNormal( nFaceNum, nPolyVertexNum, fbxVec4Normal);
	fbxVec4Normal = fbxMatrix.MultNormalize( fbxVec4Normal );

	pxVertex->normal.x = (float)(fbxVec4Normal.mData[0]);
	pxVertex->normal.y = (float)(fbxVec4Normal.mData[1]);
	pxVertex->normal.z = (float)(fbxVec4Normal.mData[2]);

	VectNormalize( &pxVertex->normal );

	if ( pFBXUVElement )
	{
	int lUVIndex = pFBXMesh->GetTextureUVIndex(nFaceNum, nPolyVertexNum);

//		vert = pFBXMesh->GetControlPointAt( pFBXMesh->GetPolygonVertex( nFaceNum, nPolyVertexNum ) );

		fbxVec2UV = pFBXUVElement->GetDirectArray().GetAt(lUVIndex);
		pxVertex->tu = (float)(fbxVec2UV.mData[0]);
		pxVertex->tv = 1.0f - (float)(fbxVec2UV.mData[1]);
	}
}

void FBXLoaderReadMesh( MODEL_RENDER_DATA* pxModelData, FbxNode* pNode )
{
int numKids = pNode->GetChildCount();
FbxNode* pChildNode = 0;
CUSTOMVERTEX*	pxVertices = mspxFBXLoadedVertexRunner;
int		i;
//int		nNumVerts = pxModelData->xStats.nNumVertices;
ushort*		puwIndices;
unsigned int*		punIndices;
FbxVector4		fbxVec4Normal;
FbxVector2		fbxVec2UV;
FbxVector4		vert;
CUSTOMVERTEX*	pxVertex;
bool		bUnmapped = false;
int		nIndexNum = 0;
BOOL	bParseUVs = FALSE;

	for ( i=0 ; i<numKids ; i++)
	{
		if ( mspunFBXLoadedLargeIndicesRunner )
		{
			punIndices = mspunFBXLoadedLargeIndicesRunner;
		}
		else
		{
			puwIndices = mspuwFBXLoadedIndicesRunner;
		}
		nIndexNum = 0;
		pxVertices = mspxFBXLoadedVertexRunner;
		
		pChildNode = pNode->GetChild(i);
		FbxMesh*	pFBXMesh = pChildNode->GetMesh();

		if ( pFBXMesh )
		{
			FbxAMatrix	localNodeTransform = pChildNode->EvaluateLocalTransform();
			int numFaces = pFBXMesh->GetPolygonCount();
			FbxVector4*		pFBXVertices = pFBXMesh->GetControlPoints();
			int		nFaceNum = 0;
			int*	pnPolygonVertices = pFBXMesh->GetPolygonVertices();


			FbxStringList UVSetNameList;
			// Get the name of each set of UV coords
			pFBXMesh->GetUVSetNames( UVSetNameList );
			const char*		pcUVSetName = UVSetNameList.GetStringAt(0);

			const FbxGeometryElementUV* pFBXUVElement = pFBXMesh->GetElementUV(pcUVSetName);

			if ( pFBXUVElement )
			{
				bParseUVs = TRUE;
	            // only support mapping mode eByPolygonVertex and eByControlPoint
				if ( ( pFBXUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex ) &&
		             ( pFBXUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint ) )
				{
					pFBXUVElement = NULL;
				}
			}

			for ( nFaceNum = 0; nFaceNum < numFaces; nFaceNum++ )
			{
				if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
				{
					punIndices[nIndexNum] = msnFBXLoadNextVertexNum;
					punIndices[nIndexNum+1] = msnFBXLoadNextVertexNum + 1;
					punIndices[nIndexNum+2] = msnFBXLoadNextVertexNum + 2;
				}
				else
				{
					puwIndices[nIndexNum] = (ushort)( msnFBXLoadNextVertexNum );
					puwIndices[nIndexNum+1] = (ushort)( msnFBXLoadNextVertexNum + 1 );
					puwIndices[nIndexNum+2] = (ushort)( msnFBXLoadNextVertexNum + 2 );
				}
				nIndexNum += 3;


				//		Get vert for index, then set its normal 
				pxVertex = pxVertices + msnFBXLoadNextVertexNum;
				FBXLoaderExtractVertex( pxVertex, pFBXMesh, pFBXUVElement, nFaceNum, 0, localNodeTransform );

				pxVertex = pxVertices + msnFBXLoadNextVertexNum + 1;
				FBXLoaderExtractVertex( pxVertex, pFBXMesh, pFBXUVElement, nFaceNum, 1, localNodeTransform );

				pxVertex = pxVertices + msnFBXLoadNextVertexNum + 2;
				FBXLoaderExtractVertex( pxVertex, pFBXMesh, pFBXUVElement, nFaceNum, 2, localNodeTransform );

				msnFBXLoadNextVertexNum += 3;
			}

			if ( mspunFBXLoadedLargeIndicesRunner )
			{
				mspunFBXLoadedLargeIndicesRunner += nIndexNum;
			}
			else
			{
				mspuwFBXLoadedIndicesRunner += nIndexNum;
			}
			nIndexNum = 0;
		}

		// Recurse children
	    FBXLoaderReadMesh(pxModelData, pChildNode);
	}
}

void FBXLoaderGetInfo( MODEL_RENDER_DATA* pxModelData, FbxNode* pNode )
{
int numKids = pNode->GetChildCount();
FbxNode*	pChildNode = 0;
int		i;
int		nLoop;

	for ( i=0; i < numKids; i++ )
	{
		pChildNode = pNode->GetChild(i);
		FbxMesh* pFBXMesh = pChildNode->GetMesh();

		if ( pFBXMesh != NULL)
		{
			int		nNumFaces = pFBXMesh->GetPolygonCount();

			msnFBXLoaderNumVertices += pFBXMesh->GetControlPointsCount();
			msnFBXLoaderNumFaces += nNumFaces;

			int numTriangles = 0;

			for ( nLoop = 0; nLoop < nNumFaces; nLoop ++ )
			{
				switch ( pFBXMesh->GetPolygonSize( nLoop ) )
				{
				case 3:
					numTriangles += 1;
					break;
				case 4:
					numTriangles += 2;
					break;
				}
			}
			msnFBXLoaderNumTriangles += numTriangles;
		}

		// Recurse children
	    FBXLoaderGetInfo(pxModelData, pChildNode);
    }
}

int			ModelLoadFBXFile( MODEL_RENDER_DATA* pxModelData, const char* szFilename, byte* pbMem )
{
FbxManager*		pFBXManager = FbxManager::Create();
FbxIOSettings*	pFBXIOSettings = FbxIOSettings::Create(pFBXManager, IOSROOT);
FbxImporter*	pFBXImporter;
FbxScene*		pScene;
FbxNode*		pRootNode;
CUSTOMVERTEX*	pxLoadedVerticesBase;

	msnFBXLoaderNumVertices = 0;
	msnFBXLoaderNumFaces = 0;
	msnFBXLoaderNumTriangles = 0;
	msnFBXLoadNextVertexNum = 0;

	pFBXManager->SetIOSettings(pFBXIOSettings);

	pFBXImporter = FbxImporter::Create(pFBXManager,"");

	pFBXImporter->Initialize( szFilename, -1, pFBXManager->GetIOSettings());
	
	pScene = FbxScene::Create( pFBXManager, "tempName" );

	pFBXImporter->Import(pScene);
	pFBXImporter->Destroy();

	FbxGeometryConverter lGeomConverter(pFBXManager);
	lGeomConverter.Triangulate(pScene, true);

	pRootNode = pScene->GetRootNode();
	if( pRootNode ) 
	{ 
		// Get the essential info (num verts, num triangles)
		FBXLoaderGetInfo( pxModelData, pRootNode ); 

		// Create our directX mesh
		msnFBXLoaderNumVertices = msnFBXLoaderNumTriangles * 3;
		if ( msnFBXLoaderNumVertices > 0 )
		{
			ModelConvInitialiseBlankModel( pxModelData,  msnFBXLoaderNumVertices,  msnFBXLoaderNumTriangles );

			pxModelData->pxBaseMesh->LockVertexBuffer( NULL, (byte**)( &mspxFBXLoadedVertexRunner ) );
			pxLoadedVerticesBase = mspxFBXLoadedVertexRunner;

			if ( pxModelData->pxBaseMesh->Is32BitIndexBuffer() == TRUE )
			{
				pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &mspunFBXLoadedLargeIndicesRunner ) );
				mspuwFBXLoadedIndicesRunner = NULL;
			}
			else
			{
				pxModelData->pxBaseMesh->LockIndexBuffer(kLock_ReadOnly, (byte**)( &mspuwFBXLoadedIndicesRunner ) );
				mspunFBXLoadedLargeIndicesRunner = NULL;
			}
			msnFBXLoadedIndicesBase = 0;

			// Now read the FBX mesh and get the vertices and index table
			FBXLoaderReadMesh( pxModelData, pRootNode );

			// Finally read in the material data/attribute buffer
			FBXLoaderExtractMaterials( pxModelData, pRootNode );

			pxModelData->pxBaseMesh->UnlockIndexBuffer();

			RenderingComputeBoundingBox( pxLoadedVerticesBase, pxModelData->xStats.nNumVertices, &pxModelData->xStats.xBoundMin, &pxModelData->xStats.xBoundMax );
			pxModelData->pxBaseMesh->UnlockVertexBuffer( );

			return( TRUE );
		}
		else
		{

		}
	}

	return( FALSE );
}


void		ModelLoaderFBXExportAddModelToScene( int nModelHandle, FbxScene* pFBXScene )
{
MODEL_RENDER_DATA* pxModelData;
FbxMesh*		pFBXMesh;
int			nNumTris;
int			nLoop;
CUSTOMVERTEX*		pxVertices;
VECT		xVect;
//int			nPointLoop;
ushort*		puwIndices;

	pxModelData = &maxModelRenderData[ nModelHandle ];

	pFBXMesh = FbxMesh::Create( pFBXScene, "" );
	pFBXMesh->Reset();

	nNumTris = pxModelData->xStats.nNumIndices / 3;
	pFBXMesh->ReservePolygonCount( pxModelData->xStats.nNumVertices / 3 );
	pFBXMesh->ReservePolygonVertexCount( pxModelData->xStats.nNumVertices );

	// Create ControlPoints in FBX
	pFBXMesh->InitControlPoints( pxModelData->xStats.nNumVertices);
	FbxVector4*		pControlPoints = pFBXMesh->GetControlPoints();

	// Create Normals layer in FBX
	FbxLayer*	pLayerZero = pFBXMesh->GetLayer(0);
	if ( pLayerZero == NULL )
	{
		pFBXMesh->CreateLayer();
		pLayerZero = pFBXMesh->GetLayer(0);
	}
	FbxLayerElementNormal* pLayerElementNormals = FbxLayerElementNormal::Create( pFBXMesh, "" );
	
	pLayerElementNormals->SetMappingMode( FbxLayerElement::eByControlPoint );
	pLayerElementNormals->SetReferenceMode( FbxLayerElement::eDirect );

	// Create UVs layer in FBX
	FbxLayer* pLayerOne = pFBXMesh->GetLayer(1);
	if (pLayerOne == NULL)
	{
		pFBXMesh->CreateLayer();
		pLayerOne = pFBXMesh->GetLayer(1);
	}
	FbxLayerElementUV* pUVDiffuseLayer = FbxLayerElementUV::Create(pFBXMesh, "DiffuseUV");
//	pUVDiffuseLayer->SetMappingMode(FbxLayerElement::eByPolygonVertex );//eByControlPoint);
	pUVDiffuseLayer->SetMappingMode(FbxLayerElement::eByControlPoint);
	pUVDiffuseLayer->SetReferenceMode(FbxLayerElement::eDirect);

	FbxLayerElementVertexColor* pVertexColorLayer = FbxLayerElementVertexColor::Create(pFBXMesh, "");
	pVertexColorLayer->SetMappingMode(FbxLayerElement::eByControlPoint);
	pVertexColorLayer->SetReferenceMode(FbxLayerElement::eDirect);
	FbxLayerElementArrayTemplate<FbxColor>& VertexColorArray = pVertexColorLayer->GetDirectArray();

	// Run through vertices data in mesh and fill in the daata in the FBX layers
	pxModelData->pxBaseMesh->LockVertexBuffer( 0, (byte**)( &pxVertices ) );
	
	for( nLoop = 0; nLoop < pxModelData->xStats.nNumVertices; nLoop++ )
	{
		xVect = pxVertices[nLoop].position;	
		pControlPoints[nLoop] = FbxVector4( 0.0f - xVect.x, xVect.y, xVect.z, 0.0f );
		xVect = pxVertices[nLoop].normal;
		pLayerElementNormals->GetDirectArray().Add( FbxVector4( 0.0f - xVect.x, xVect.y, xVect.z, 0.0f ) );
		pUVDiffuseLayer->GetDirectArray().Add( FbxVector2( pxVertices[nLoop].tu, 1.0f - pxVertices[nLoop].tv ));

//		VertexColorArray.Add( FbxColor(pxVertices[nLoop].color, VertColor.G, VertColor.B, VertColor.A ));
		VertexColorArray.Add( FbxColor( 1.0f, 1.0f, 1.0f, 1.0f ));
	}

	pxModelData->pxBaseMesh->UnlockVertexBuffer( );

	// Assign the layers on the FBX to the appropriate functions
	pLayerZero->SetNormals( pLayerElementNormals );
	pLayerZero->SetVertexColors(pVertexColorLayer);
	pLayerOne->SetUVs( pUVDiffuseLayer, FbxLayerElement::eTextureDiffuse );

	// Create the per-material polygons sets.

	// TODO - Update this to cope with multiple materials
	int			nMaterialIndex = 0;
	pxModelData->pxBaseMesh->LockIndexBuffer( nMaterialIndex, (byte**)( &pxVertices ) );
	pxModelData->pxBaseMesh->LockIndexBuffer( 0, (byte**)( &puwIndices ) );

	for( nLoop = 0; nLoop < nNumTris; nLoop++ )
	{
		pFBXMesh->BeginPolygon(nMaterialIndex);
		
		pFBXMesh->AddPolygon( puwIndices[(nLoop*3)+1] );
		pFBXMesh->AddPolygon( puwIndices[(nLoop*3)+0] );
		pFBXMesh->AddPolygon( puwIndices[(nLoop*3)+2] );
		pFBXMesh->EndPolygon();
	}

	FbxSurfaceMaterial* pFbxMaterial = FbxSurfaceLambert::Create( pFBXScene, "Default Material");//TCHAR_TO_UTF8(NewMaterialName));
	((FbxSurfaceLambert*)pFbxMaterial)->Diffuse.Set(FbxDouble3(0.72, 0.72, 0.72));

	FbxNode* pRootNode = pFBXScene->GetRootNode();

	FbxNode* pMeshNode = FbxNode::Create( pFBXScene, "Primary Mesh");
	pMeshNode->SetName("PrimaryMesh");
	pMeshNode->SetNodeAttribute( pFBXMesh );
	pMeshNode->AddMaterial(pFbxMaterial);
	pRootNode->AddChild( pMeshNode );

///  TODO - Include colours and materials..  

/*
	FbxNode* MeshNode = FbxNode::Create(Scene, TCHAR_TO_UTF8(MeshName));
	MeshNode->SetNodeAttribute(Mesh);

	// Add the materials for the mesh
	int32 MaterialCount = SkelMesh->Materials.Num();

	for(int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInterface* MatInterface = SkelMesh->Materials[MaterialIndex].MaterialInterface;

		FbxSurfaceMaterial* FbxMaterial = NULL;
		if(MatInterface && !FbxMaterials.Find(MatInterface))
		{
			FbxMaterial = ExportMaterial(MatInterface);
		}
		else
		{
			// Note: The vertex data relies on there being a set number of Materials.  
			// If you try to add the same material again it will not be added, so create a 
			// default material with a unique name to ensure the proper number of materials

			TCHAR NewMaterialName[MAX_SPRINTF]=TEXT("");
			FCString::Sprintf( NewMaterialName, TEXT("Fbx Default Material %i"), MaterialIndex );

			FbxMaterial = FbxSurfaceLambert::Create(Scene, TCHAR_TO_UTF8(NewMaterialName));
			((FbxSurfaceLambert*)FbxMaterial)->Diffuse.Set(FbxDouble3(0.72, 0.72, 0.72));
		}

		MeshNode->AddMaterial(FbxMaterial);
	}

*/

}


void		ModelLoaderExportFBX(int nModelHandle, const char* acFilename )
{

	// Create the FBX SDK manager
	FbxManager* pFBXSdkManager = FbxManager::Create();
	// Create an IOSettings object.
	FbxIOSettings * pIOSettings = FbxIOSettings::Create(pFBXSdkManager, IOSROOT );
	// set some IOSettings options 
	pIOSettings->SetBoolProp( EXP_FBX_MATERIAL, true);
	pIOSettings->SetBoolProp( EXP_FBX_MODEL,  true);
	pIOSettings->SetBoolProp( EXP_GEOMETRY,  true);
	pIOSettings->SetBoolProp( EXP_UPAXIS, true );

	pFBXSdkManager->SetIOSettings(pIOSettings);

	// Create an exporter.
	FbxExporter* pFBXExporter = FbxExporter::Create(pFBXSdkManager, "");

	// Initialize the exporter.
	bool bExportStatus = pFBXExporter->Initialize(acFilename, -1, pFBXSdkManager->GetIOSettings());

	// Create a new scene so it can be populated by the imported file.
	FbxScene* pFBXScene = FbxScene::Create(pFBXSdkManager,"exportScene");
	// Next, create a FbxMesh from our Model, and add it to the scene.
	ModelLoaderFBXExportAddModelToScene( nModelHandle, pFBXScene );

//	FbxDocumentInfo*	pDocInfo = pFBXScene->GetDocumentInfo();
	FbxNode* pRootNode = pFBXScene->GetRootNode();
	int			nNumChildren = pRootNode->GetChildCount();

	FbxAxisSystem* pAxisSystem = new FbxAxisSystem( FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded );
	pFBXScene->GetGlobalSettings().SetAxisSystem( *pAxisSystem );
	FbxSystemUnit		xSystemUnit( 100.0f );
	pFBXScene->GetGlobalSettings().SetSystemUnit( xSystemUnit );
	pFBXExporter->Export( pFBXScene );
	pFBXExporter->Destroy();
	pFBXScene->Destroy();
}

#endif			// ifdef TOOL
