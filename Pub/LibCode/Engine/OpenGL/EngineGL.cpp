#include <windows.h>		// For OpenGL

#include "GL/glew.h"
#include <gl/gl.h>


#include <StandardDef.h>
#include <Engine.h>
#include <Interface.h>

#include "../LibCode/Interface/OpenGL/TextureLoader/InterfaceTextureManager.h"
#include "EngineGL.h"


void	EngineInitFromInterface( void )
{
	EngineInit();

}



void	EngineDefaultState( void )
{
 

}

void	EngineSetRenderTargetTexture( TEXTURE_HANDLE hTexture, uint32 ulClearCol, BOOL bClear )
{
	// TODO
}

void		EngineSceneShadowsSetRenderParams( float fLightCamTargetDist, float fLightNearPlane, float fLightFarPlane, float fFOVMod )
{
	// TODO
}

TEXTURE_HANDLE	EngineCreateRenderTargetTexture( int nWidth, int nHeight, int mode, const char* szTrackingName )
{
	// TODO
	return( NOTFOUND );
}

void				EngineRestoreRenderTarget( void )
{
	// TODO

}

//--------------------------------------------------
// EngineSetBlendMode
//	Sets the type of semi-transparent blending
//--------------------------------------------------
void	EngineSetBlendMode( int nBlendMode )
{

	switch( nBlendMode )
	{
	case BLEND_MODE_SRCALPHA_ADDITIVE:
	// TODO
		break;
	case BLEND_MODE_ALPHABLEND:
	// TODO
		break;
	case BLEND_MODE_COLOUR_SUBTRACTIVE:
	// TODO
		break;
	case BLEND_MODE_COLOUR_BLEND:
	// TODO
		break;
	case BLEND_MODE_COLOUR_ADDITIVE:
	// TODO
		break;
	}
}


void	EngineEnableZWrite( BOOL bFlag )
{
	glDepthMask(GL_TRUE);
}

void	EngineEnableZTest( BOOL bFlag )
{
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

}

void			EngineSetShadowMultitexture( BOOL bFlag )
{
	// todo
}

void	EngineEnableTextureAddressClamp( int nFlag )
{
	// TODO
}

void	EngineSetColourMode( int nTexLayer, int nColourMode )
{
	// TODO
}

void	EngineResetColourMode( void )
{

}

void	EngineEnableWireframe( int nFlag )
{
	// TODO

}


void	EngineSetWorldMatrix( const ENGINEMATRIX* pxWorldMatrix )
{
	// todo

}

void	EngineEnableCulling( int flag )
{
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}

void	EngineEnableSpecular( int flag )
{
	// TODO
}

void	EngineEnableBlend( int nFlag )
{
// TODO
}

extern int	TexturedOverlayCreate( int nLayer, TEXTURE_HANDLE hTexture );

int				EngineTextureCreateInterfaceOverlay( int nLayer, TEXTURE_HANDLE hTexture )
{
	if ( hTexture >= 0 )
	{
		return( TexturedOverlayCreate( nLayer, hTexture ) );
	}
	else
	{
		return( NOTFOUND );
	}
}

void	EngineSetZBias( int Value )
{
// TODO

}

void	EngineSetShadeMode( int nFlag )
{

}

BOOL	EngineTextureIsFullyLoaded( TEXTURE_HANDLE nTexHandle )
{
	return( TRUE );
}

TEXTURE_HANDLE	EngineLoadTexture( const char* szFilename, int nMode, int* pnErrorFlag )
{
	return( (TEXTURE_HANDLE)( InterfaceLoadTexture( szFilename, 0 ) ) );
}


void	EngineReleaseTexture( TEXTURE_HANDLE* pnHandle )
{
	if ( *pnHandle > 0 )
	{ 
		InterfaceReleaseTexture( *pnHandle );
		*pnHandle = 0;
	}
}

void EngineSetStandardMaterial( void )
{
	// todo
}

void	EngineSetTexture( int nTex, TEXTURE_HANDLE nTexHandle )
{
	InterfaceSetTextureAsCurrent( nTexHandle );
}

void	EngineGetRayForScreenCoord( int nScreenX, int nScreenY, VECT* pxRayStart, VECT* pxRayDir )
{
	// todo
}

void	EngineSetMaterial( ENGINEMATERIAL* pxMaterial )
{
	// todo
}

void		EngineSetMaterialColourSource( BOOL bFlag )
{
	// todo

}

void		EngineActivateLight( int nLightNum, ENGINE_LIGHT* pLight )
{
	// todo
}

void	EngineEnableLighting( int nFlag )
{
	// todo

}


TEXTURE_HANDLE	EngineLoadTextureFromFileInMem( byte* pbMem, int nMipMode, int* pnErrorFlag, int nMemSize, const char* szOptionalFilename )
{
	return( (TEXTURE_HANDLE)InterfaceGetTextureFromFileInMem( "temp.jpg", pbMem, nMemSize, 0 ) );
}


void				EngineSetPixelShader( LPGRAPHICSPIXELSHADER, const char* szShaderName )
{

}

void				EngineSetVertexShader( LPGRAPHICSVERTEXSHADER, const char* szShaderName )
{

}

void	EngineEnableFog( int nFlag )
{
	// todo

}

void	EngineEnableAlphaTest( int nFlag )
{
	// todo

}

void	EngineSetVertexFormat( int nVertexFormat )
{
	
}