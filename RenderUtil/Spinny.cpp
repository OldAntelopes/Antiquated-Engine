
#include <math.h>
#include "StandardDef.h"
#include "Interface.h"


float		msfSpinnyAngle = 0.0f;
ulong		mulLastSpinAngleUpdateTick = 0;

void		SpinnyInit( void )
{

	 
}


void		SpinnyDraw( int X, int Y, int W, int H, float fGlobalAlpha )
{
float		fRadius = (float)(W / 2);
int			nMidX = X + (W/2);
int			nMidY = Y + (W/2);
int			nNumSteps = 14;
int			nLoop;
int			nScale = 7;
int			nPosX;
int			nPosY;
float		fStartAngle = msfSpinnyAngle;
ulong		ulCurrentTick = SysGetTick();
float		fStartAlpha = 1.0f;
ulong		ulCol;

	if ( ulCurrentTick - mulLastSpinAngleUpdateTick > 50 )
	{
	float	fDelta = (float)( ulCurrentTick - mulLastSpinAngleUpdateTick ) * 0.001f;
//	float	fSpinSpeed = 3.0f;

		mulLastSpinAngleUpdateTick = ulCurrentTick;
		msfSpinnyAngle -= (A360/nNumSteps);
		if ( msfSpinnyAngle < 0.0f ) msfSpinnyAngle += A360;
	}

	for( nLoop = 0; nLoop < nNumSteps; nLoop++ )
	{
		nPosX = nMidX + (int)(fRadius * sinf( fStartAngle ) );		
		nPosY = nMidY + (int)(fRadius * cosf( fStartAngle ) );		

		ulCol = GetColWithModifiedAlpha( 0xd0f0f0f0, fStartAlpha * fGlobalAlpha );
		InterfaceRect( 1, nPosX - (nScale/2), nPosY - (nScale/2), nScale, nScale, ulCol ); 
		if ( nScale > 4 )
		{
			nScale--;
		}
		fStartAngle += (A360/nNumSteps);
		if ( fStartAngle < 0.0f ) 
		{
			fStartAngle += A360;
		}
		fStartAlpha -= 0.05f;
	}

}


void		SpinnyShutdown( void )
{


}

