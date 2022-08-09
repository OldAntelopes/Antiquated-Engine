

#include "StandardDef.h"
#include "Interface.h"

#include "Platform.h"

//#define		INMOBI_ADS
#define		AMAZON_ADS

#ifdef		INMOBI_ADS
#include "s3eInMobiAds.h"
#endif

#ifdef		AMAZON_ADS
#include "s3eAmazonAds.h"
#endif


BOOL			mbAdsInitialised = FALSE;

//-----------------------------------------------------------------------------------------------------------
// ------------ InMobi Ads
//-----------------------------------------------------------------------------------------------------------
#ifdef		INMOBI_ADS

static const char *INMOBI_PUBLISHER_ID = "298b4957ed39493281e276916b399dcb";


InMobiAd*		mspMobiAd = NULL;
BOOL			mbAdsShown = FALSE;
BOOL			mbAdsLoadRequested = FALSE;
BOOL			mbAdLoaded = FALSE;

static int ad_request_completed(InMobiAd* ad, void *systemData, void *userData) 
{
	mbAdsLoadRequested = FALSE;
	mbAdLoaded = TRUE;
   return S3E_RESULT_SUCCESS;
}

static int ad_request_failed(InMobiAd* ad, void *systemData, void *userData)
{
	mbAdsLoadRequested = FALSE;
	mbAdLoaded = FALSE;
    return S3E_RESULT_SUCCESS;
}


//const char  *AdRequestParams = "age:25,education:EDUCATION_HIGHSCHOOLORLESS";
const char  *AdRequestParams = "";

void		PlatformAdDisplay( BOOL bShow )
{
	if ( mspMobiAd )
	{
		if ( bShow != mbAdsShown )
		{
			if ( !bShow )
			{
				if ( mbAdsLoadRequested == FALSE )
				{
					inmobi_banner_load(mspMobiAd, "");
					mbAdsLoadRequested = TRUE;
				}
			}
		}

		if ( bShow )
		{
            inmobi_banner_show(mspMobiAd);
//			GameSetTestString( "Showing banner" );
        }
		else 
		{
            inmobi_banner_hide(mspMobiAd);
//			GameSetTestString( "Hiding banner" );
		}
	}
	else
	{
//		GameSetTestString( "No banner available" );
	}
}

void		PlatformAdSystemInitInMobi( const char* szAdSystemAppKey )
{
    if ( s3eInMobiAdsAvailable()) 
    {
	int		nScreenX = (InterfaceGetWidth()-468);
	int		nScreenY = 0;

		mspMobiAd = inmobi_banner_init(INMOBI_PUBLISHER_ID, INMOBIADS_BANNER_SIZE_468x60,
                                       nScreenX, nScreenY);		
		inmobi_banner_disable_hardware_acceleration(mspMobiAd); //Only applicable for Android	

		InMobiAdsRegisterAdCallback(INMOBIADS_CALLBACK_AD_REQUEST_COMPLETED, ad_request_completed, NULL, mspMobiAd);
	    InMobiAdsRegisterAdCallback(INMOBIADS_CALLBACK_AD_REQUEST_FAILED, ad_request_failed, NULL, mspMobiAd);

		//			registerAdCallbacks(pMobiAd);
		inmobi_banner_load(mspMobiAd, "");
		mbAdsLoadRequested = TRUE;

	}
}

#endif		// ifdef INMOBI_ADS



//-----------------------------------------------------------------------------------------------------------
// ------------ Amazon Ads
//-----------------------------------------------------------------------------------------------------------
#ifdef AMAZON_ADS

void		PlatformAdSystemInitAmazon( const char* szAdSystemAppKey )
{
	if ( s3eAmazonAdsAvailable() )
	{
		s3eAmazonAdsInit( szAdSystemAppKey );
	}
}



void		PlatformAdDisplay( BOOL bShow )
{
	// TODO 

}

#endif

//-----------------------------------------------------------------------------------------------------------
// ------------ Platform Ad system
//-----------------------------------------------------------------------------------------------------------


void		PlatformAdsUpdate( void )
{

}


void				PlatformAdsInitialise( const char* szAdSystemAppKey )
{
	if ( mbAdsInitialised == FALSE )
	{
#ifdef INMOBI_ADS
		PlatformAdSystemInitInMobi( szAdSystemAppKey );
#endif

#ifdef AMAZON_ADS
		PlatformAdSystemInitAmazon( szAdSystemAppKey );
#endif

		mbAdsInitialised = TRUE;
	}

}
