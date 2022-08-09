
#ifndef SERVER

#include <stdio.h>
#include <string.h>

#include <StandardDef.h>

#include <System.h>
#include <Engine.h>
#include <Rendering.h>
#include <CodeUtil.h>

#include "SceneMap.h"

//-----------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------
// SceneMapElement
//
int		SceneMapElement::msnNumSelected = 0;

void	SceneMapElement::SetSelected( BOOL bFlag ) 
{
	if ( ( mbIsSelected == TRUE ) &&
		 ( bFlag == FALSE ) )
	{
		msnNumSelected--;
	}
	else if ( ( mbIsSelected == FALSE ) &&
		 ( bFlag == TRUE ) )
	{
		msnNumSelected++;
	}
	mbIsSelected = bFlag; 
}


void	SceneRoomElement::OnParseComplete( void )
{
	if ( IsModelLoaded() )
	{
	SceneMapGameInterface*		pSceneMapGameInterface = EngineGetSceneMapGameInterface();

		if ( pSceneMapGameInterface )
		{
			pSceneMapGameInterface->OnElementLoaded( GetModelHandle(), GetPos(), NULL );
		}
	}
}

void	SceneMapElement::SetModel( const char* szModelName )
{
char	szRepathed[256];

#ifdef TOOL
	strcpy( szRepathed, szModelName );
#else
	// Check to see if we have an absolute file path
	if ( szModelName[1] == ':' )
	{
		strcpy( szRepathed, szModelName );
	}
	else
	{
		sprintf( szRepathed, "%s\\%s", SysGetWritableDataFolderPath(NULL), szModelName );
	}
#endif
	mModelHandle = ModelLoad( szRepathed, 0, 1.0f );
#ifdef TOOL
	strcpy( m_szModelFilename, szModelName );
#endif

}

SceneMapElement::~SceneMapElement()
{
	if ( mTextureHandle != 0 )
	{
		EngineReleaseTexture( &mTextureHandle );
	}
	if ( mModelHandle != NOTFOUND )
	{
		ModelFree( mModelHandle );
	}
}

void	SceneMapElement::SetTexture( const char* szTextureName )
{
	if ( szTextureName )
	{
	char	szRepathed[256];
		
#ifdef TOOL
		strcpy( szRepathed, szTextureName );
#else
		if ( szTextureName[1] == ':' )
		{
			strcpy( szRepathed, szTextureName );
		}
		else
		{
			sprintf( szRepathed, "%s\\%s", SysGetWritableDataFolderPath(NULL), szTextureName );
		}
#endif
		mTextureHandle = EngineLoadTexture( szRepathed, 0, NULL );
#ifdef TOOL
		strcpy( m_szTextureFilename, szTextureName );
#endif
	}
#ifdef TOOL
	else
	{
		m_szTextureFilename[0] = 0;
	}
#endif
}

BOOL	SceneMapElement::Raycast( VECT* pxPos1, VECT* pxPos2, VECT* pxHit, VECT* pxNormal )
{
	return( ModelRayTest( mModelHandle, &mPos, &mRot, pxPos1, pxPos2, pxHit, pxNormal, NULL, 0 ) );
}

SceneMapElement*		SceneMapElement::FindParentOf( SceneMapElement* pChild )
{
SceneMapElement*		pFound;
	if ( mpChildElement )
	{
	SceneMapElement*		pChildren;

		pChildren = mpChildElement;
		while ( pChildren )
		{
			if (  pChildren == pChild )
			{
				return( this );
			}
			pChildren = pChildren->mpBrotherElement;
		}

	}
	if ( mpChildElement )
	{
		pFound = mpChildElement->FindParentOf( pChild );
		if ( pFound )
		{
			return( pFound );
		}
	}
	if ( mpBrotherElement )
	{
		pFound = mpBrotherElement->FindParentOf( pChild );
		if ( pFound )
		{
			return( pFound );
		}
	}
	return( NULL );
}


void 	SceneMapElement::Display( const VECT* pxPos, const VECT* pxRot, int nFlags )
{
VECT	vecPos;
VECT	vecRot;

	VectAdd( &vecPos, &mPos, pxPos );
	VectAdd( &vecRot, &mRot, pxRot );
	EngineSetTexture( 0, mTextureHandle );
	ModelRender( mModelHandle, &vecPos, &vecRot, nFlags );
	if ( mbIsSelected == TRUE )
	{
	ENGINEMATERIAL		xMaterial;

		memset( &xMaterial, 0, sizeof( xMaterial ) );
		xMaterial.Diffuse.r = 0.75f;
		xMaterial.Diffuse.g = 0.6f;
		xMaterial.Diffuse.b = 0.1f;
		xMaterial.Diffuse.a = 0.75f;

		EngineSetMaterial( &xMaterial );
		EngineEnableWireframe(1);
		EngineEnableBlend(TRUE);
		EngineSetMaterialColourSource( TRUE );
		EngineSetBlendMode( BLEND_MODE_COLOUR_BLEND );
		EngineSetZBias(8);
		EngineSetTexture( 0, 0 );
		ModelRender( mModelHandle, &vecPos, &vecRot, RENDER_FLAGS_DRAWMESH_ONLY );
		EngineSetZBias(0);
		EngineEnableWireframe(0);
		EngineSetMaterialColourSource( FALSE );
		EngineSetStandardMaterial();
	}
	if ( mpChildElement )
	{
		mpChildElement->Display( &vecPos, &vecRot, nFlags );
	}
	if ( mpBrotherElement )
	{
		mpBrotherElement->Display( pxPos, pxRot, nFlags );
	}

}

void	SceneMapElement::Update( const VECT* pxPos )
{
	UpdateForPlayer( pxPos );

	if ( mpChildElement )
	{
		mpChildElement->Update( pxPos );
	}

	if ( mpBrotherElement )
	{
		mpBrotherElement->Update(pxPos);
	}

}

SceneMapElement*		SceneMapElement::FindRegion( const char* szRegionType, const char* szRegionParam )
{
SceneRegionElement*		pRegion;
SceneMapElement*		pMap;

	if ( stricmp( GetTypeName(), "Region" ) == 0 )
	{
		pRegion = (SceneRegionElement*)( this );
		
		if ( stricmp( pRegion->GetRegionType(), szRegionType ) == 0 )
		{
			if ( szRegionParam )
			{
				if ( stricmp( pRegion->GetRegionParam(), szRegionParam ) == 0 )
				{
					return( this );
				}
			}
			else
			{
				return( this );
			}
		}
	}
	if ( mpChildElement )
	{
		pMap = mpChildElement->FindRegion( szRegionType, szRegionParam );
		if ( pMap )
		{
			return( pMap );
		}
	}

	if ( mpBrotherElement )
	{
		pMap = mpBrotherElement->FindRegion( szRegionType, szRegionParam );
		if ( pMap )
		{
			return( pMap );
		}
	}
	return( NULL );
}

#ifdef TOOL
void	SceneMapElement::ListFileReferences( ListFileReferencesCallback fnCallback )
{
FILE*	pFile;
int		nFilesize;

	if ( mModelHandle != NOTFOUND )
	{
		pFile = fopen( m_szModelFilename, "rb" );
		if ( pFile )
		{
			nFilesize = SysGetFileSize(pFile);
			fclose(pFile);
			fnCallback( m_szModelFilename, nFilesize );
		}
	}
	if ( mTextureHandle != 0 )
	{
		pFile = fopen( m_szTextureFilename, "rb" );
		if ( pFile )
		{
			nFilesize = SysGetFileSize(pFile);
			fclose(pFile);
			fnCallback( m_szTextureFilename, nFilesize );
		}
	}
}

void	SceneMapElement::GetFileReferences( ListFileReferencesCallback fnCallback )
{
	ListFileReferences( fnCallback );
	if ( mpChildElement )
	{
		mpChildElement->GetFileReferences(fnCallback);
	}
	if ( mpBrotherElement )
	{
		mpBrotherElement->GetFileReferences(fnCallback);
	}
}


void	SceneMapElement::Save( FILE* pFile )
{
	SaveElement(pFile );
	if ( mpChildElement )
	{
		mpChildElement->Save(pFile);
	}
	if ( mpBrotherElement )
	{
		mpBrotherElement->Save(pFile);
	}

}

void	StripPath( const char* szOriginal, char* szStripped )
{
const char*	pcRunner;
	pcRunner = szOriginal + strlen( szOriginal );
	while ( ( pcRunner != szOriginal ) &&
			( *pcRunner != '\\' ) &&
			( *pcRunner != '/' ) )
	{
		pcRunner--;
	}
	if ( ( *pcRunner == '\\' ) ||
		 ( *pcRunner == '/' ) )
	{
		pcRunner++;
	}

	strcpy( szStripped, pcRunner );
}
void	SceneRoomElement::SaveElement( FILE* pFile )
{
char	acBuff[256];
char	acModelName[256];
char	acTextureName[256];

	StripPath( m_szModelFilename, acModelName );
	StripPath( m_szTextureFilename, acTextureName );
	// simplified just to save one layer of rooms
	// other stuff will replace this lata..
	sprintf( acBuff, "room=%s\r\n", acModelName );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	sprintf( acBuff, "tex=%s\r\n", acTextureName );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	sprintf( acBuff, "pos=%f,%f,%f\r\n", mPos.x, mPos.y, mPos.z );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	sprintf( acBuff, "rot=%f,%f,%f\r\n", (mRot.x*360)/TwoPi, (mRot.y*360)/TwoPi, (mRot.z*360)/TwoPi );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );

}
void	SceneFurnitureElement::SaveElement( FILE* pFile )
{
char	acBuff[256];
char	acModelName[256];
char	acTextureName[256];

	StripPath( m_szModelFilename, acModelName );
	StripPath( m_szTextureFilename, acTextureName );
	// simplified just to save one layer of rooms
	// other stuff will replace this lata..
	sprintf( acBuff, "furn=%s\r\n", acModelName );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	sprintf( acBuff, "tex=%s\r\n", acTextureName );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	sprintf( acBuff, "pos=%f,%f,%f\r\n", mPos.x, mPos.y, mPos.z );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	sprintf( acBuff, "rot=%f,%f,%f\r\n", (mRot.x*360)/TwoPi, (mRot.y*360)/TwoPi, (mRot.z*360)/TwoPi );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );

}

void	SceneLightElement::SaveElement( FILE* pFile )
{
char	acBuff[256];
//char	acModelName[256];
//char	acTextureName[256];

	// simplified just to save one layer of rooms
	// other stuff will replace this lata..
	sprintf( acBuff, "light=0\r\n" );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	sprintf( acBuff, "pos=%f,%f,%f\r\n", mPos.x, mPos.y, mPos.z );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
//	sprintf( acBuff, "rot=%f,%f,%f\r\n", (mRot.x*360)/TwoPi, (mRot.y*360)/TwoPi, (mRot.z*360)/TwoPi );
//	fwrite( acBuff, strlen( acBuff ), 1, pFile );

}

void	SceneRegionElement::SaveElement( FILE* pFile )
{
char	acBuff[256];

	// simplified just to save one layer of rooms
	// other stuff will replace this lata..
	if ( m_szRegionType )
	{
		sprintf( acBuff, "region=%s\r\n", m_szRegionType );
	}
	else
	{
		sprintf( acBuff, "region=none\r\n" );
	}
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	if ( m_szRegionParam )
	{
		sprintf( acBuff, "param=%s\r\n", m_szRegionParam );
	}
	else
	{
		sprintf( acBuff, "param=none\r\n");
	}
	fwrite( acBuff, strlen( acBuff ), 1, pFile );

	sprintf( acBuff, "scale=%f\r\n", m_fRadius/ 0.0686367f );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );

	sprintf( acBuff, "pos=%f,%f,%f\r\n", mPos.x, mPos.y, mPos.z );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );
	sprintf( acBuff, "rot=%f,%f,%f\r\n", (mRot.x*360)/TwoPi, (mRot.y*360)/TwoPi, (mRot.z*360)/TwoPi );
	fwrite( acBuff, strlen( acBuff ), 1, pFile );

}
#endif

void 	SceneRegionElement::Display( const VECT* pxPos, const VECT* pxRot, int nFlags )
{
VECT	vecPos;
VECT	vecRot;
#ifdef TOOL
VECT	vecScale;
#endif

	VectAdd( &vecPos, &mPos, pxPos );
	VectAdd( &vecRot, &mRot, pxRot );

#ifdef TOOL
	EngineSetTexture( 0, 0 );
	EngineEnableWireframe(1);
	EngineEnableBlend(TRUE);
	EngineEnableLighting( FALSE );
	EngineSetColourMode( 0, COLOUR_MODE_DIFFUSE_ONLY );
	EngineSetBlendMode( BLEND_MODE_ALPHABLEND );

	vecScale.x = m_fRadius / 0.0686367f;		// radius of ball model
	vecScale.y = vecScale.x;
	vecScale.z = vecScale.y;

	vecScale.x *= 2.0f;
	vecScale.y *= 2.0f;
	vecScale.z *= 2.0f;

//	ModelRender( GetModelHandle(), &vecPos, &vecRot, RENDER_FLAGS_DRAWMESH_ONLY);
	ModelRenderScaled( GetModelHandle(), &vecPos, &vecRot, 0xFF, &vecScale);
	if ( mbIsSelected == TRUE )
	{
		EngineEnableBlend(TRUE);
		EngineSetBlendMode( BLEND_MODE_SRCALPHA_ADDITIVE );
		EngineSetZBias(8);
		ModelRenderScaled( GetModelHandle(), &vecPos, &vecRot, 0xFF, &vecScale);
		EngineSetZBias(0);
	}
	EngineEnableWireframe(0);
#endif
	if ( mpChildElement )
	{
		mpChildElement->Display( &vecPos, &vecRot, nFlags );
	}
	if ( mpBrotherElement )
	{
		mpBrotherElement->Display( pxPos, pxRot, nFlags );
	}

}

void	SceneRegionElement::UpdateForPlayer( const VECT* pxPos )
{
	if ( pxPos )
	{
	float	fDist = VectDist( pxPos, GetPos() );
	SceneMap*	pCurrentScene = SceneMapGet();

		if ( fDist <= m_fRadius )
		{
			if ( m_bPlayerInRegion == FALSE )
			{
				m_bPlayerInRegion = TRUE;
				pCurrentScene->TriggerRegionEvent(kEnter_Region, this );
			}
		}
		else		// Not in region
		{
			if ( m_bPlayerInRegion == TRUE )
			{
				m_bPlayerInRegion = FALSE;
				pCurrentScene->TriggerRegionEvent(kLeave_Region, this );
			}
		}
	}
}

void	SceneRegionElement::SetRegionType( const char* szRegionType )
{
	if ( m_szRegionType )
	{
		SystemFree( m_szRegionType );
	}
	m_szRegionType = (char*)SystemMalloc( strlen(szRegionType) + 1 );
	if ( m_szRegionType )
	{
		strcpy( m_szRegionType, szRegionType );
	}

	int	nModelHandle = GetModelHandle();

	if ( nModelHandle != NOTFOUND )
	{
		if ( stricmp( m_szRegionType, "InteriorLink" ) == 0 )
		{
			ModelSetVertexColours( nModelHandle, 0xA04090a0 );
		}
		else if ( stricmp( m_szRegionType, "SalesPoint" ) == 0 )
		{
			ModelSetVertexColours( nModelHandle, 0xA0702010 );
		}
		else if ( stricmp( m_szRegionType, "StocksPoint" ) == 0 )
		{
			ModelSetVertexColours( nModelHandle, 0xA0102070 );
		}
		else if ( stricmp( m_szRegionType, "BuildingAccess" ) == 0 )
		{
			ModelSetVertexColours( nModelHandle, 0xA0707070 );
		}
		else if ( stricmp( m_szRegionType, "ExitPoint" ) == 0 )
		{
			ModelSetVertexColours( nModelHandle, 0xA0a03040 );
		}
		else if ( stricmp( m_szRegionType, "SpawnPoint" ) == 0 )
		{
			ModelSetVertexColours( nModelHandle, 0xA030a040 );
		}
		else if ( stricmp( m_szRegionType, "ScriptEvent" ) == 0 )
		{
			ModelSetVertexColours( nModelHandle, 0xA0b07040 );
		}
	}


}

void	SceneRegionElement::SetRegionParam( const char* szRegionParam )
{
	if ( m_szRegionParam )
	{
		SystemFree( m_szRegionParam );
	}
	m_szRegionParam = (char*)SystemMalloc( strlen(szRegionParam) + 1 );
	if ( m_szRegionParam )
	{
		strcpy( m_szRegionParam, szRegionParam );
	}
}


SceneRegionElement::~SceneRegionElement()
{	
	if ( m_szRegionType )
	{
		SystemFree( m_szRegionType );
	}
	if ( m_szRegionParam )
	{
		SystemFree( m_szRegionParam );
	}
}

SceneRegionElement::SceneRegionElement()
{
#ifdef TOOL
int		nModelHandle;
char	acModelName[256];
	sprintf( acModelName, "Data\\ball1.atm" );
	nModelHandle = ModelLoad( acModelName, 0, 1.0f );
	if ( nModelHandle != NOTFOUND )
	{
		ModelSetVertexColours( nModelHandle, 0xA02020f0 );
	}
	SetModelHandle( nModelHandle );
	m_fRadius = ModelGetStats(nModelHandle)->fBoundSphereRadius;
#endif
	m_szRegionType = NULL;
	m_szRegionParam = NULL;
	m_fRadius = 0.0686367f;		// radius of ball model
	m_bPlayerInRegion = FALSE;
}


#endif // ndef SERVER
