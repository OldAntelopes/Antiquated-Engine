
#include <StandardDef.h>

#include "IwBilling.h"

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


void PlatformCommerceRefundCallback(void* caller, IwBilling::CIwBillingRefundData* data)
{

}

void PlatformCommerceReceiptAvailableCallback(void* caller, IwBilling::CIwBillingReceiptAvailableData* data)
{
	if (data->Restored)
		s3eDebugTracePrintf("Product restore successfull");
	else
		s3eDebugTracePrintf("Product purchase successfull");
 
	// Place receipt data into a string
	char* receipt = new char[data->ReceiptLength + 1];
	memcpy(receipt, data->Receipt, data->ReceiptLength);
	receipt[data->ReceiptLength] = 0;
	s3eDebugTracePrintf("Product ID: %s", data->ProductID);
	s3eDebugTracePrintf("Receipt: %s", receipt);
	s3eDebugTracePrintf("TransactionID: %s", data->TransactionID);

	if ( stricmp( data->ProductID, mszCommerceCheckProductID ) == 0 )
	{
		MainOnPurchaseVerified( TRUE );
	}

	mbPurchaseCompleted = TRUE;
	mbPurchaseErrored = FALSE;
	mbIsPurchaseInProgress = FALSE;

	// Finalise the transaction (ths notifies the app store that the purchase has been completed)
	IwBilling::FinishTransaction(data->FinaliseData);
 
	delete [] receipt;

}



void PlatformCommerceErrorCallback(void* caller, IwBilling::CIwBillingErrorData* data)
{
	strncpy( mszCommerceLastError, IwBilling::getErrorString(data->Error), 511 );
	mszCommerceLastError[511] = 0;
	mbPurchaseErrored = TRUE;
	mbIsPurchaseInProgress = FALSE;
}


void PlatformCommerceInfoAvailableCallback(void* caller, IwBilling::CIwBillingInfoAvailableData* data)
{
	// TODO
	if ( mbPurchaseCheckInProgress == TRUE )
	{
//		GameSetTestString("Purchase Info received");

	}


}

BOOL		PlatformCommerceIsBusy( void )
{
	return( mbIsPurchaseInProgress );
}

void		PlatformCommerceCheckPurchaseState( const char* szProductID )
{
	if ( mbIsPurchaseInProgress == FALSE )
	{
		strcpy( mszCommerceCheckProductID, szProductID );
		mbPurchaseCompleted = FALSE;
		mbPurchaseErrored = FALSE;
		mszCommerceLastError[0] = 0;
	 	mbPurchaseCheckInProgress = TRUE;
		IwBilling::RestoreTransactions();
	}

}

void		PlatformCommercePurchase( const char* szProductID )
{
	strcpy( mszCommerceCheckProductID, szProductID );

	mbPurchaseCompleted = FALSE;
	mbPurchaseErrored = FALSE;
	mszCommerceLastError[0] = 0;
	mbIsPurchaseInProgress = TRUE;

#ifdef TEST_PURCHASE_PROCESS
	IwBilling::CIwBillingReceiptAvailableData		data;

	data.ProductID = new char[ strlen( szProductID ) + 1 ];
	strcpy( (char*)data.ProductID, szProductID );
	data.TransactionID = new char[ strlen( "0001Test" ) + 1 ];
	strcpy( (char*)data.TransactionID, "0001Test" );
	data.ReceiptLength = strlen( "ReceiptTest" ) + 1;
	data.Receipt = new char[ data.ReceiptLength ];
	strcpy( data.Receipt, "ReceiptTest" );

	PlatformCommerceReceiptAvailableCallback( NULL, &data );
#else
	// Purchase the product
	IwBilling::PurchaseProduct( szProductID );
#endif
}

void	PlatformCommerceFree( void )
{
	IwBilling::Terminate();
}

void PlatformCommerceReadyCallback(void* caller, void* data)
{
    // IwBilling is ready to use
}


void		PlatformCommerceInit( const char* szCommerceKey )
{
	
	if (IwBilling::isAvailable(IwBilling::BILLING_VENDOR_GOOGLE_PLAY))
	{
		// Google play billing is available
		if ( IwBilling::Init( PlatformCommerceReadyCallback, (void*)szCommerceKey, IwBilling::BILLING_VENDOR_GOOGLE_PLAY ) == 0)
		{
	//        GameSetTestString("Billing inactive");
			return;
		}

		mbCommerceAvailable = TRUE;
	
		// Set up callbacks
		IwBilling::setInfoAvailableCallback(PlatformCommerceInfoAvailableCallback);
		IwBilling::setErrorCallback(PlatformCommerceErrorCallback);
		IwBilling::setReceiptAvailableCallback(PlatformCommerceReceiptAvailableCallback);
		IwBilling::setRefundCallback(PlatformCommerceRefundCallback);
	}
	
}
