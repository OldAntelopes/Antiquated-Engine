
#include "IwGx.h"
#include "IwGxFont.h"
#include "IwMaterial.h"

#include "StandardDef.h"
#include "Interface.h"

#include "../LibCode/Interface/Common/InterfaceUtil.h"
#include "../LibCode/Interface/Common/InterfaceTexturedPolyLists.h"
#include "../LibCode/Interface/Common/InterfaceTextureList.h"
#include "InterfaceInternalsMarmalade.h"
#include "InterfaceTexturesMarmalade.h"

int			mnInterfaceTexturesFrameCounter = 1;


class TextureMatBuffer
{
public:
	TextureMatBuffer()
	{
		mnTextureHandle = NOTFOUND;
		mpMaterial = NULL;
		mnLastUsedFrame = 0;
	}

	~TextureMatBuffer()
	{
		Release();
	}

	void		Release( void )
	{
		if ( mpMaterial )
		{
			delete mpMaterial;
			mpMaterial = NULL;
		}
		mnTextureHandle = NOTFOUND;
		mnLastUsedFrame = 0;
	}
	void		Update( void )
	{
		if ( mpMaterial )
		{
			if ( mnInterfaceTexturesFrameCounter > mnLastUsedFrame + 2 ) 
			{
				Release();
			}
		}
	}

	int				mnTextureHandle;
	int				mnRenderType;
	CIwMaterial*	mpMaterial;
	int				mnLastUsedFrame;
};

TextureMatBuffer		maTextureMaterialBuffer[MAX_DIFFERENT_TEXTURED_OVERLAYS];


void*				InterfacePlatformGetTexturedOverlayMaterial( int hTexture, int nRenderType )
{
CIwTexture*		pTexture = (CIwTexture*)( InterfaceTextureListGetPlatformTexture( hTexture ) );
CIwMaterial*	pNewMat;
int		nLoop;
int		nFirstEmpty = NOTFOUND;

	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		if ( ( maTextureMaterialBuffer[nLoop].mnTextureHandle == hTexture ) &&
			 ( maTextureMaterialBuffer[nLoop].mnRenderType == nRenderType ) )
		{
			maTextureMaterialBuffer[nLoop].mnLastUsedFrame = mnInterfaceTexturesFrameCounter;
			return( maTextureMaterialBuffer[nLoop].mpMaterial );
		}
		else if ( ( nFirstEmpty == NOTFOUND ) &&
				  ( maTextureMaterialBuffer[nLoop].mnTextureHandle < 0 ) )
		{
			nFirstEmpty = nLoop;
		}
	}

	if ( nFirstEmpty != NOTFOUND )
	{
		pNewMat = new CIwMaterial;
		pNewMat->SetTexture( pTexture );

		switch( nRenderType )
		{
		case RENDER_TYPE_NORMAL:
		case RENDER_TYPE_LAYER2:
		case RENDER_TYPE_ALPHATEST:
			pNewMat->SetAlphaMode(CIwMaterial::ALPHA_BLEND);
			pNewMat->SetModulateMode(CIwMaterial::MODULATE_RGB);
			break;
		case RENDER_TYPE_ADDITIVE:
			pNewMat->SetAlphaMode(CIwMaterial::ALPHA_ADD);
			pNewMat->SetModulateMode(CIwMaterial::MODULATE_RGB);
//			pNewMat->SetBlendMode(CIwMaterial::BLEND_ADD);
			break;
		case RENDER_TYPE_COLORBLEND:
			pNewMat->SetAlphaMode(CIwMaterial::ALPHA_BLEND);
			pNewMat->SetModulateMode(CIwMaterial::MODULATE_RGB);
			break;
		case RENDER_TYPE_SUBTRACTIVE:
			pNewMat->SetAlphaMode(CIwMaterial::ALPHA_SUB);
			pNewMat->SetModulateMode(CIwMaterial::MODULATE_RGB);
			break;
		case RENDER_TYPE_ALPHA_SUBTRACTIVE:
			pNewMat->SetAlphaMode(CIwMaterial::ALPHA_SUB);
			pNewMat->SetModulateMode(CIwMaterial::MODULATE_RGB);
			break;
		}

		pNewMat->SetCullMode(CIwMaterial::CULL_NONE);

		maTextureMaterialBuffer[nFirstEmpty].mnTextureHandle = hTexture;
		maTextureMaterialBuffer[nFirstEmpty].mnRenderType = nRenderType;
		maTextureMaterialBuffer[nFirstEmpty].mnLastUsedFrame = mnInterfaceTexturesFrameCounter;
		maTextureMaterialBuffer[nFirstEmpty].mpMaterial = pNewMat;

		return( (void*)( pNewMat ) );
	}
	return( NULL );
}

void	InterfacePlatformReleaseTexturedOverlayMaterial( void* pPlatformMaterial )
{
CIwMaterial*	pMat = (CIwMaterial*)( pPlatformMaterial );

	// Will probably need to morgue this.. 
//	delete pMat;
}

void*		InterfaceTextureGetRawPointer( int nTextureHandle )
{
	return( InterfaceTextureListGetPlatformTexture( nTextureHandle ) );
}

void					InterfaceReleasePlatformTexture( int hTexture, LPGRAPHICSTEXTURE pTexture )
{
int		nLoop;

	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		if ( maTextureMaterialBuffer[nLoop].mnTextureHandle == hTexture )
		{
			maTextureMaterialBuffer[nLoop].Release();
		}
	}
	delete pTexture;
}

#define CREATED_TEXTURE_FORMAT CIwImage::RGB_565
//#define CREATED_TEXTURE_FORMAT CIwImage::ARGB_8888


LPGRAPHICSTEXTURE	InterfaceGetBlankPlatformTexture( int nWidth, int nHeight, int nMode, int* pnPitch )
{
CIwTexture* pTex = new CIwTexture;
CIwImage* pImage = new CIwImage;
CIwImage::Format	format = CREATED_TEXTURE_FORMAT;

	switch( nMode )
	{
	case 1:
		format = CIwImage::RGB_565;
		break;
	case 0:
	default:
		format = CIwImage::ARGB_8888;
		break;
	}
		
	pImage->SetFormat( format );
	
	pImage->SetWidth( nWidth );
	pImage->SetHeight( nHeight );
	pImage->SetBuffers();

	pTex->SetFormatHW( format );
	pTex->SetFormatSW( format );
	pTex->SetImage( pImage );
	delete pImage;
	pTex->SetModifiable( true );
	pTex->SetMipMapping( false );

	if ( pnPitch )
	{
		*pnPitch = pTex->GetPitch();
	}

	// check actual format
	format = pTex->GetFormat();

//	pTex->Upload();
	return( pTex );
}


LPGRAPHICSTEXTURE	InterfaceGetPlatformTexture( const char* szFilename, int nFlags, int nArchiveHandle, int* pnPitch )
{
	switch( nFlags )
	{
	case 0:
	default:
		{
		CIwImage* pImage = new CIwImage;
		CIwTexture* pTex = new CIwTexture;
	
			pImage->LoadFromFile(szFilename);
			pImage->ReplaceColour( 0, 0, 0xFF, 0xFF, 0xff, 0, 0xff, 0 );
//			pImage->ReplaceAlpha( 1, 0xFF, 0, 0xFF);

			pTex->CopyFromImage( pImage );
//			pTex->LoadFromFile(szFilename);
			pTex->Upload();
			delete( pImage );
			return( pTex );
		}
	case 1:
		{
		CIwTexture* pTex = new CIwTexture;

			pTex->LoadFromFile(szFilename);
			pTex->Upload();
			return( pTex );
		}
		break;
	case 2:
		{
		CIwTexture* pTex = new CIwTexture;

			pTex->SetModifiable( true );
			pTex->LoadFromFile(szFilename);
			if ( pnPitch )
			{
				*pnPitch = pTex->GetPitch();
			}
			pTex->Upload();
			return( pTex );
		}
		break;
	case 3:
	case 4:
		{
		CIwImage* pImage = new CIwImage;
		CIwTexture* pTex = new CIwTexture;
		CIwImage* pARGBImage = new CIwImage;

			pImage->LoadFromFile(szFilename);

			pARGBImage->SetFormat( CIwImage::ARGB_8888 );
			pImage->ConvertToImage( pARGBImage );
			delete( pImage );

			pARGBImage->ReplaceColour( 0, 0, 0xFF, 0xFF, 0xff, 0, 0xff, 0 );
			pTex->CopyFromImage( pARGBImage );
			pTex->SetModifiable( true );
			if ( pnPitch )
			{
				*pnPitch = pTex->GetPitch();
			}
			pTex->Upload();
			delete( pARGBImage );
			return( pTex );
		}
		break;
	}
	return( NULL );
}

void		InterfaceTexturesMarmaladeFreeAll( void )
{
int		nLoop;

	for ( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		maTextureMaterialBuffer[nLoop].Release();
	}


}

void	InterfaceTexturesMarmaladeNewFrame( void )
{
int		nLoop;

	for( nLoop = 0; nLoop < MAX_DIFFERENT_TEXTURED_OVERLAYS; nLoop++ )
	{
		maTextureMaterialBuffer[nLoop].Update();
	}
	mnInterfaceTexturesFrameCounter++;
}


int	InterfaceGetTextureSize( int nTextureHandle, int* pnW, int* pnH )
{
CIwTexture*		pTexture = (CIwTexture*)InterfaceTextureGetRawPointer( nTextureHandle );

	if ( pTexture )
	{
		*pnW = pTexture->GetWidth();
		*pnH = pTexture->GetHeight();
	}
	else
	{
		*pnW = 0;
		*pnH = 0;
	}
	return( 1 );
}


BYTE*	InterfaceLockTexture( int nTextureHandle, int* pnPitch, int* pnFormat, int nFlags )
{
CIwTexture*		pTexture = (CIwTexture*)InterfaceTextureGetRawPointer( nTextureHandle );

	*pnPitch = InterfaceTextureListGetPitch( nTextureHandle );
	*pnFormat = (int)pTexture->GetFormat();

	return( pTexture->GetTexels() );

}

void	InterfaceTextureGetColourAtPoint( int nTextureHandle, byte* pbLockedTextureData, int nPitch, int nFormat, int x, int y, float* pfRed, float* pfGreen, float* pfBlue, float* pfAlpha )
{
byte*		pbRow = pbLockedTextureData + (y * nPitch);
int		nR = 0;
int		nG = 0;
int		nB = 0;
int		nA = 0;
ulong	ulCol;
ushort	uwColVal;
int		nPaletteNum;
CIwTexture*		pTexture = (CIwTexture*)InterfaceTextureGetRawPointer( nTextureHandle );
const BYTE*	pbPalette;


/*
        RGB_332,    //!< Unpalettised 8-bit.
        BGR_332,    //!< Unpalettised 8-bit.


        BGR_565,    //!< Unpalettised 16-bit, no alpha.


        RGBA_4444,  //!< Unpalettised 16-bit, alpha.
        ABGR_4444,  //!< Unpalettised 16-bit, alpha.
        ,  //!< Unpalettised 16-bit, alpha.


        RGB_888,    //!< Unpalettised 24-bit, no alpha.
        BGR_888,    //!< Unpalettised 24-bit, no alpha.


        RGBA_6666,  //!< Unpalettised 24-bit, alpha.
        ABGR_6666,  //!< Unpalettised 24-bit, alpha.


        RGBA_AAA2,  //!< Unpalettised 32-bit, alpha.
        ABGR_2AAA,  //!< Unpalettised 32-bit, alpha.

        A_8,                //!< Unpalettised 8-bit alpha.

        ETC,                //!< Ericsson compressed format
        ARGB_8888,          //!< Unpalettised 32-bit, alpha.

        PALETTE4_ARGB_8888, //!< 16-colour palettised.
        PALETTE8_ARGB_8888, //!< 256-colour palettised.

        DXT3,               //!< DXT3 compressed format

        PALETTE4_BGR555,        //!< 16-colour palettised.
        PALETTE8_BGR555,        //!< 16-colour palettised.
        A5_PALETTE3_BGR_555,    //!< 8BPP, of which 5 are alpha and 3 are palette index
        A3_PALETTE5_BGR_555,    //!< 8BPP, of which 3 are alpha and 5 are palette index

        PALETTE4_BGR_565,   //!< 16-colour palettised.
        PALETTE4_ABGR_8888, //!< 16-colour palettised.
        PALETTE8_BGR_565,   //!< 256-colour palettised.
        PALETTE8_ABGR_8888, //!< 256-colour palettised.

        DXT1,               //!< DXT1 compressed format
        DXT5,               //!< DXT5 compressed format
        
        ETC2,               //!< GLES 3.0 mandated formats
        ETC2_A1,            //!< 1bit alpha version
        EAC_R11,            //!< 1 channel version
        EAC_RG11,           //!< 2 channel version
*/
	switch( nFormat )
	{
	case CIwImage::RGB_888:			// 3 bytes per pixel
		nR = *( pbRow + (x*3) );
		nG = *( pbRow + (x*3) + 1 );
		nB = *( pbRow + (x*3) + 2 );
		break;
	case CIwImage::RGBA_5551:
		uwColVal = *( (ushort*)( pbRow + (x*2) ) );
		nR = ( ( uwColVal >> 11 ) & 0x1F) << 3;
		nG = ( ( uwColVal >> 6 ) & 0x1F ) << 3;
		nB = ( ( uwColVal >> 1 ) & 0x1F ) << 3;
		break;
	case CIwImage::ABGR_1555:
		uwColVal = *( (ushort*)( pbRow + (x*2) ) );
		nB = ( ( uwColVal >> 10 ) & 0x1F) << 3;
		nG = ( ( uwColVal >> 5 ) & 0x1F ) << 3;
		nR = ( uwColVal & 0x1F ) << 3;
		break;
	case CIwImage::RGB_565:
		uwColVal = *( (ushort*)( pbRow + (x*2) ) );
		nR = ( uwColVal >> 11 ) << 3;
		nG = ( ( uwColVal >> 5 ) & 0x3F ) << 2;
		nB = ( uwColVal & 0x1F ) << 3;
		break;
	case CIwImage::RGBA_8888:
		ulCol = *( (ulong*)( pbRow + (x * 4) ) );
		nR = ((ulCol >> 24) & 0xFF);
		nG = ((ulCol >> 16) & 0xFF);
		nB = ((ulCol >> 8) & 0xFF);
		nA = (ulCol & 0xFF);
		break;
	case CIwImage::PALETTE8_RGB_888:
		nPaletteNum = *( (BYTE*)( pbRow + x ) );
		pbPalette = pTexture->GetPalette();
		nR = pbPalette[nPaletteNum*3];
		nG = pbPalette[(nPaletteNum*3)+1];
		nB = pbPalette[(nPaletteNum*3)+2];
		break;
	case CIwImage::ARGB_8888:
		ulCol = *( (ulong*)( pbRow + (x * 4) ) );
		nA = ((ulCol >> 24) & 0xFF);
		nR = ((ulCol >> 16) & 0xFF);
		nG = ((ulCol >> 8) & 0xFF);
		nB = (ulCol & 0xFF);
		break;
	case CIwImage::ABGR_8888:
		ulCol = *( (ulong*)( pbRow + (x * 4) ) );
		nA = ((ulCol >> 24) & 0xFF);
		nB = ((ulCol >> 16) & 0xFF);
		nG = ((ulCol >> 8) & 0xFF);
		nR = (ulCol & 0xFF);
		break;
	}

	*pfRed = (float)(nR) / 255.0f;
	*pfGreen = (float)(nG) / 255.0f;
	*pfBlue = (float)(nB) / 255.0f;
	*pfAlpha = (float)(nA) / 255.0f;
}

void	InterfaceUnlockTexture( int nTextureHandle )
{
CIwTexture*		pTexture = (CIwTexture*)InterfaceTextureGetRawPointer( nTextureHandle );

	pTexture->Upload();
}
