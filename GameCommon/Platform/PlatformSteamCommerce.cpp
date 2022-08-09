
#include <StandardDef.h>

#include "Platform.h"

extern void		MainOnPurchaseVerified( BOOL );

//#define TEST_PURCHASE_PROCESS

BOOL	mbIsPurchaseInProgress = FALSE;
BOOL	mbPurchaseCheckInProgress = FALSE;
BOOL	mbPurchaseErrored = FALSE;
BOOL	mbPurchaseCompleted = FALSE;
char	mszCommerceLastError[512] = "";
BOOL	mbCommerceAvailable = FALSE;

char	mszCommerceCheckProductID[256] = "";


void	PlatformCommerceReset( void )
{
	mbPurchaseErrored = FALSE;
	mszCommerceLastError[0] = 0;
}

void 	PlatformCommerceCancel( void )
{
	mbPurchaseErrored = TRUE;
	strcpy( mszCommerceLastError, "Transaction Cancelled" );
}

BOOL		PlatformCommerceDidError( void )
{
	return( mbPurchaseErrored );
}

const char*		PlatformCommerceGetLastErrorString( void )
{
	return( mszCommerceLastError );
}






BOOL		PlatformCommerceIsBusy( void )
{
	return( mbIsPurchaseInProgress );
}

void		PlatformCommerceCheckPurchaseState( const char* szProductID )
{
// TODO	
}

void		PlatformCommercePurchase( const char* szProductID )
{
// TODO	
}

void	PlatformCommerceFree( void )
{
// TODO	
}

void PlatformCommerceReadyCallback(void* caller, void* data)
{
// TODO	
}


void		PlatformCommerceInit( const char* szCommerceKey )
{
	
// TODO	
}
