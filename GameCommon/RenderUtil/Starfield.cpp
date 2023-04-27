
#include "StandardDef.h"
#include "Interface.h"
#include "Engine.h"

#include "Starfield.h"


typedef struct
{
	VECT	xPos;
	uint32	ulBaseCol;

	BYTE	bType;
	BYTE	bPhase;
	ushort	uwGlowTime;

	uint32	ulGlowCol;
	float	fDotCache;

} BACKGROUND_STAR;

#define			MAX_NUM_BACKGROUND_STARS	3000
#define			MIN_STAR_DIST	60000.0f
#define			MAX_STAR_DIST	76500.0f
#define			NUM_PROCESS_PHASES		16
//#define			MIN_STAR_DIST	500.0f
//#define			MAX_STAR_DIST	850.0f

BACKGROUND_STAR*	mpxBackgroundStarfield = NULL;
int		mnBackgroundStarfieldProcessPhase = -1;
int		mnBackgroundStarsToDraw = MAX_NUM_BACKGROUND_STARS;
uint32	mulLastBackgroundCountChange = 0;



void StarfieldShutdown( void )
{
	if ( mpxBackgroundStarfield )
	{
		SystemFree( mpxBackgroundStarfield );
		mpxBackgroundStarfield = NULL;
	}

}

void StarfieldInit( eStarfieldType starfieldType )
{
	if ( !mpxBackgroundStarfield )
	{
	int		Loop;
	BACKGROUND_STAR*	pxStar;
	float	fDist;
	int		nR, nG, nB;
	int		rR, rG, rB;
	int		nBrightness;
	BYTE	bProcessPhase = 0;

		mpxBackgroundStarfield = (BACKGROUND_STAR*)SystemMalloc( sizeof(BACKGROUND_STAR) * MAX_NUM_BACKGROUND_STARS );
		SysPanicIf( mpxBackgroundStarfield == NULL, "Couldnt allocate memory for background starfield" );
		ZeroMemory( mpxBackgroundStarfield, sizeof(BACKGROUND_STAR) * MAX_NUM_BACKGROUND_STARS );

		pxStar = mpxBackgroundStarfield;
		for ( Loop = 0; Loop < MAX_NUM_BACKGROUND_STARS; Loop++ )
		{
			pxStar->xPos.x = FRand( -1.0f, 1.0f );
			pxStar->xPos.y = FRand( -1.0f, 1.0f );
			pxStar->xPos.z = FRand( -1.0f, 1.0f );
			VectNormalize( &pxStar->xPos );
			fDist = FRand( MIN_STAR_DIST, MAX_STAR_DIST );
			VectScale( &pxStar->xPos, &pxStar->xPos, fDist );
			nBrightness = 0x20 + (rand() % 0x60);
			rR = (rand() % 0x40);
			rG = (rand() % 0x40);
			rB = (rand() % 0x40);
			nR = nBrightness + rR;
			nG = nBrightness + rG;
			nB = nBrightness + rB;
			pxStar->ulBaseCol = 0xFF000000 | (nR << 16 ) | (nG << 8 ) | nB;
			nBrightness /= 2;
			nR = nBrightness + (rR/2);
			nG = nBrightness + (rG/2);
			nB = nBrightness + (rB/2);
			pxStar->ulGlowCol = 0xFF000000 | (nR << 16 ) | (nG << 8 ) | nB;
			pxStar->bType = (BYTE)( rand() % 2 );
			pxStar->bPhase = bProcessPhase++;
			bProcessPhase %= NUM_PROCESS_PHASES;
			pxStar++;
		}
	}
}



/***************************************************************************
 * Function    : StarfieldRender
 * Params      :
 * Description : 
 ***************************************************************************/
void StarfieldRender( void )
{
int		Loop;
BACKGROUND_STAR*	pxStar;
int		nX, nY;
int		nWidth = InterfaceGetWidth();
int		nHeight = InterfaceGetHeight();
VECT	xCamDir = *EngineCameraGetDirection();
VECT	xCamPos = *EngineCameraGetPos();
VECT	xVectToStar;
VECT		xVect;

	if ( mpxBackgroundStarfield )
	{
	uint32	ulTimeTakenToUpdate = SysGetTick();

		pxStar = mpxBackgroundStarfield;
		for ( Loop = 0; Loop < mnBackgroundStarsToDraw; Loop++ )
		{
			VectSub( &xVectToStar, &xCamPos, &pxStar->xPos );	
			VectNormalize( &xVectToStar );
			pxStar->fDotCache = VectDot( &xVectToStar, &xCamDir );

			if ( pxStar->fDotCache < 0.0f )
			{
				xVect.x = 0.0f;
				xVect.y = 0.0f;
				xVect.z = 0.0f;

				EngineGetScreenCoordForWorldCoord( &pxStar->xPos, &nX, &nY );

				if ( ( nX >= 0 ) &&
					 ( nX <= nWidth ) &&
					 ( nY >= 0 ) &&
					 ( nY <= nHeight ) )
				{
					if ( pxStar->bType == 0 )
					{
						InterfaceLine( 0, nX-1, nY, nX+2, nY, pxStar->ulGlowCol, pxStar->ulGlowCol );
						InterfaceLine( 0, nX, nY-1, nX, nY+1, pxStar->ulGlowCol, pxStar->ulGlowCol );
					}
					InterfaceLine( 0, nX, nY, nX + 1, nY, pxStar->ulBaseCol, pxStar->ulBaseCol );
				}
			}
			pxStar++;
		}
	}
}
