
#include "StandardDef.h"
#include "Interface.h"

#include "LandscapeCoords.h"

float		mfHeightmapTextureToHeightScale = 256.0f;

float*		mpfHeightmap = NULL;
int			mnHeightmapSizeX = 0;
int			mnHeightmapSizeY = 0;
int			mnHeightmapMapSize = 0;

int		LandscapeHeightmapGetSizeX( void )
{
	return( mnHeightmapSizeX );
}


int		LandscapeHeightmapGetSizeY( void )
{
	return( mnHeightmapSizeY );
}

float	LandscapeHeightmapGetHeight( int nMapX, int nMapY )
{
int		nMapPos = ( nMapY * mnHeightmapSizeX ) + nMapX;

	if ( ( nMapPos >= 0 ) &&
		 ( nMapPos < mnHeightmapMapSize ) )
	{
		return( mpfHeightmap[nMapPos] );
	}
	return( 0.0f );
}

float	LandscapeHeightmapGetHeightWorld( float fWorldX, float fWorldY )
{
float	fHeightA;
float	fHeightB;
float	fHeightC;
float	fHeightD;
float	fWeightA;
float	fWeightB;
float	fWeightC;
float	fWeightD;
float	fFractionAlongXLine;
float	fFractionAlongYLine;
MVECT	xBasePos;
int	nX;
int	nY;
float	fHeight;
float		fWorldToMapScale = LandscapeWorldToMapScale();
float		fMapToWorldScale = LandscapeMapToWorldScale();
//int	nIndex;

	nX = (int)( ( fWorldX * fWorldToMapScale ) );
	nY = (int)( ( fWorldY * fWorldToMapScale ) );

	xBasePos = LandscapeGetWorldPos( nX, nY );

	fFractionAlongXLine = ( fWorldX - xBasePos.fX ) / (fMapToWorldScale);
	fFractionAlongYLine = ( fWorldY - xBasePos.fY ) / (fMapToWorldScale);

	if ( fFractionAlongXLine < 0.0f ) fFractionAlongXLine = 0.0f - fFractionAlongXLine;
	if ( fFractionAlongYLine < 0.0f ) fFractionAlongYLine = 0.0f - fFractionAlongYLine;
	if ( fFractionAlongXLine >= 1.0f )
	{
		if ( fFractionAlongYLine >= 1.0f )
		{
			fHeight	= LandscapeHeightmapGetHeight( nX+1, nY+1 );
			return( fHeight ); 
		}	

		fHeight	= LandscapeHeightmapGetHeight( nX+1, nY );
		return( fHeight );
	}
	else if ( fFractionAlongYLine >= 1.0f )
	{
		fHeight = LandscapeHeightmapGetHeight( nX, nY+1 ); 
		return( fHeight ); 
	}

	if ( (fFractionAlongXLine + fFractionAlongYLine) > 1.0f )
	{
		// Need check for outside map details region
		fHeightB = LandscapeHeightmapGetHeight( nX+1, nY );
		fHeightC = LandscapeHeightmapGetHeight( nX, nY+1 );
		fHeightD = LandscapeHeightmapGetHeight( nX+1, nY+1 );
		fHeightA = fHeightD - ( (fHeightD - ((fHeightB + fHeightC)*0.5f) )*2.0f);
	}
	else
	{
	// Need check for outside map details region
		fHeightA = LandscapeHeightmapGetHeight( nX, nY );
		fHeightB = LandscapeHeightmapGetHeight( nX+1, nY );
		fHeightC = LandscapeHeightmapGetHeight( nX, nY+1 );
 		fHeightD = fHeightA - ( (fHeightA - ((fHeightB + fHeightC)*0.5f) ) *2.0f );
//		fHeightD = GetHeightAtMapCoordinate( nX+1, nY+1 );
	}

	fWeightA = (1 - fFractionAlongYLine) * (1 - fFractionAlongXLine);
	fWeightB = (1 - fFractionAlongYLine) * fFractionAlongXLine;
	fWeightC = fFractionAlongYLine * (1 - fFractionAlongXLine);
	fWeightD = fFractionAlongYLine * fFractionAlongXLine;

	fHeightA *= fWeightA;
	fHeightB *= fWeightB;
	fHeightC *= fWeightC;
	fHeightD *= fWeightD;
	fHeightA += fHeightB + fHeightC + fHeightD;

	return( fHeightA );
}

// fHeight is in range 0.0f -> 255.0f
void	LandscapeHeightmapSetHeight( int nMapX, int nMapY, float fHeight )
{
int		nMapPos = ( nMapY * mnHeightmapSizeX ) + nMapX;

	if ( ( nMapPos >= 0 ) &&
		 ( nMapPos < mnHeightmapMapSize ) )
	{
		mpfHeightmap[nMapPos] = fHeight;
	}
}


BOOL LandscapeHeightmapInit( int nMapSizeX, int nMapSizeY )
{
int		nHeightmapMemSizeBytes = nMapSizeX * nMapSizeY * sizeof( float );

	if ( mpfHeightmap != NULL )
	{
		free( mpfHeightmap );
	}
	mpfHeightmap = (float*)( malloc( nHeightmapMemSizeBytes ) );
	if ( mpfHeightmap )
	{
		mnHeightmapSizeX = nMapSizeX;
		mnHeightmapSizeY = nMapSizeY;
		mnHeightmapMapSize = mnHeightmapSizeX * mnHeightmapSizeY;

		memset( mpfHeightmap, 0, nHeightmapMemSizeBytes );
		return( TRUE );
	}
	return( FALSE );
}

void LandscapeHeightmapFree( void )
{
	if ( mpfHeightmap )
	{
		free( mpfHeightmap );
		mpfHeightmap = NULL;
	}
}


BOOL LandscapeHeightmapProcessHeightmapTexture( int hTexture, BOOL bUseTextureHeightmapSize )
{
	if ( hTexture != NOTFOUND )
	{
	int		imageW;
	int		imageH;
	byte*	pbImageData;
	int		nPitch;
	int		nFormat;
	float	fImageX = 0.0f;
	float	fImageY = 0.0f;
	float	fImageStepX = 1.0f;
	float	fImageStepY = 1.0f;
	int		nLoopX;
	int		nLoopY;

		InterfaceGetTextureSize( hTexture, &imageW, &imageH );

		if ( ( bUseTextureHeightmapSize ) &&
			 ( imageW != mnHeightmapSizeX ) )
		{
			LandscapeHeightmapInit( imageW, imageH );
		}

		if ( imageW != mnHeightmapSizeX )
		{
			fImageStepX = (float)( imageW ) / (float)( mnHeightmapSizeX );
		}
		if ( imageH != mnHeightmapSizeY )
		{
			fImageStepY = (float)( imageH ) / (float)( mnHeightmapSizeY );
		}

//		fImageX = (float)( imageW - 1 );
//		fImageStepX *= -1.0f;
//		fImageY = (float)( imageH - 1 );
//		fImageStepY *= -1.0f;

		pbImageData = InterfaceLockTexture( hTexture, &nPitch, &nFormat, 0  );

		if ( pbImageData )
		{
		float	R,G,B,A;
		float	fBrightness;

			for ( nLoopY = 0; nLoopY < mnHeightmapSizeY; nLoopY++ )
			{
				fImageX = 0.0f;//(float)( imageW - 1 );
				for ( nLoopX = 0; nLoopX < mnHeightmapSizeX; nLoopX++ )
				{
					InterfaceTextureGetColourAtPoint( hTexture, pbImageData, nPitch, nFormat, (int)(fImageX), (int)(fImageY), &R,&G,&B,&A );

					fBrightness = ( R + G + B ) / 3.0f;
					if ( fBrightness < 0.0f ) fBrightness = 0.0f;
					if ( fBrightness > 1.0f ) fBrightness = 1.0f;

					// mfHeightmapTextureToHeightScale = 256.0f so : fHeight is in range 0.0f -> 255.0f
					LandscapeHeightmapSetHeight( nLoopX, nLoopY, fBrightness * mfHeightmapTextureToHeightScale );

					fImageX += fImageStepX;
				}
				fImageY += fImageStepY;
			}
	
			InterfaceUnlockTexture( hTexture );
		}

		LandscapeCoordsInit( mnHeightmapSizeX, mnHeightmapSizeY );

		return( TRUE );
	}
	return( FALSE );
}
