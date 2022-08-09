
#include "StandardDef.h"

#define USE_REF_LOCAL_DB

#ifdef USE_REF_LOCAL_DB
#include "RefLocalDB\RefLocalDB.h"
#endif

#include "DBInterface.h"

enum eDBRequestType
{
	DB_RETRIEVE,
	DB_STORE,
};


class DBRequest
{
public:
	DBRequest()
	{
		mpcRequestData = NULL;
		mfnCallback = NULL;
		mpNext = NULL;
	}

	eDBRequestType		mRequestType;			// Retrieve or store

	char*				mpcRequestData;			
	int					mnDataLen;
	DBResponse			mfnCallback;
	void*				mvpCBParam;
	u64					mullRecordUID;
	int					mnRequestID;
	
	DBRequest*		mpNext;

};


class DBInstance
{
public:
	DBInstance()
	{
		mpDBRequestList = NULL;
		mhDBThread = NOTFOUND;
		mbDBRequestPending = FALSE;
		mbDBCallbackPending = FALSE;
		mbDBKillThread = FALSE;
		mpCurrentActiveRequest = NULL;
		mpNextDBInstance = NULL;
		mnNextRequestID = 101;
	}

	void		Initialise( int nDBInstanceID, const char* szInstanceName );
	void		Update( void );
	void		Shutdown( void );

	int			Retrieve( u64 ullRecirdUID, DBResponse fnCallback, void* vpCBParam );

	int			Store( u64 ullRecordUID, const char* pcJSONData, DBResponse fnCallback, void* vpCBParam );

	void		ResetAll( void );

	void		AddRequestToList( DBRequest* pNewRequest );
	void		ThreadPerformNextRequest( void );

	DBRequest*			mpDBRequestList;

	unsigned int		mhDBThread;
	volatile BOOL		mbDBRequestPending;
	volatile BOOL		mbDBCallbackPending;
	volatile BOOL		mbDBKillThread;
	DBRequest*			mpCurrentActiveRequest;
	int					mnDBRequestResponseCode;

	int					mnNextRequestID;

	int					mnDBInstanceID;
	char*				mszDBInstanceName;
	
	DBInstance*			mpNextDBInstance;
};

DBInstance*			mspDBInstances = NULL;


void		DBInstance::AddRequestToList( DBRequest* pNewRequest )
{
DBRequest*		pList = mpDBRequestList;

	while( pList ) 
	{
		if ( pList->mpNext == NULL )
		{
			pList->mpNext = pNewRequest;
			break;
		}
	}

	if ( !pList )
	{
		mpDBRequestList = pNewRequest;
	}
	pNewRequest->mpNext = NULL;
}

int		DBInstance::Retrieve( u64 ullRecordUID, DBResponse fnCallback, void* vpCBParam )
{
DBRequest*		pNewRequest = new DBRequest;
int				nNewRequestID = mnNextRequestID++;

	pNewRequest->mfnCallback = fnCallback;
	pNewRequest->mnDataLen = 0;
	pNewRequest->mpcRequestData = NULL;
	pNewRequest->mRequestType = DB_RETRIEVE;
	pNewRequest->mvpCBParam = vpCBParam;
	pNewRequest->mullRecordUID = ullRecordUID;
	pNewRequest->mnRequestID = nNewRequestID;

	AddRequestToList( pNewRequest );
	return( nNewRequestID );
}


int		DBInstance::Store( u64 ullRecordUID, const char* pcJSONData, DBResponse fnCallback, void* vpCBParam )
{
DBRequest*		pNewRequest = new DBRequest;
int				nNewRequestID = mnNextRequestID++;

	pNewRequest->mfnCallback = fnCallback;
	pNewRequest->mnDataLen = strlen( pcJSONData ) + 1;
	pNewRequest->mpcRequestData = (char*)pcJSONData;
	pNewRequest->mRequestType = DB_STORE;
	pNewRequest->mvpCBParam = vpCBParam;
	pNewRequest->mullRecordUID = ullRecordUID;
	pNewRequest->mnRequestID = nNewRequestID;

	AddRequestToList( pNewRequest );
	return( nNewRequestID );
}


void		DBInstance::ThreadPerformNextRequest( void )
{
	switch( mpCurrentActiveRequest->mRequestType )
	{
	case DB_RETRIEVE:
#ifdef USE_REF_LOCAL_DB
		mnDBRequestResponseCode = RefLocalDBRetrieve( mszDBInstanceName, mpCurrentActiveRequest->mullRecordUID, (BYTE**)&mpCurrentActiveRequest->mpcRequestData, &mpCurrentActiveRequest->mnDataLen );
#endif
		break;
	case DB_STORE:
#ifdef USE_REF_LOCAL_DB
		mnDBRequestResponseCode = RefLocalDBStore( mszDBInstanceName, mpCurrentActiveRequest->mullRecordUID, (BYTE*)mpCurrentActiveRequest->mpcRequestData, mpCurrentActiveRequest->mnDataLen );
#endif
		break;
	}

}


long WINAPI DBRequestThread(long lParam)
{ 
DBInstance*		pDBInstance = (DBInstance*)( lParam );

	while( pDBInstance->mbDBKillThread == FALSE )
	{
		if ( pDBInstance->mbDBRequestPending == TRUE )
		{
			pDBInstance->ThreadPerformNextRequest( );

			pDBInstance->mbDBCallbackPending = TRUE;
			pDBInstance->mbDBRequestPending = FALSE;
		}
		else
		{
			SysSleep(10);
		}
	}

	return( 0 );
}

void		DBInstance::ResetAll( void )
{
#ifdef USE_REF_LOCAL_DB
	RefLocalDBResetAll( mszDBInstanceName );
#endif

}

void		DBInstance::Initialise( int nDBInstanceID, const char* szInstanceName )
{
	RefLocalDBInit( szInstanceName );

	mnDBInstanceID = nDBInstanceID;
	mszDBInstanceName = (char*)( malloc( strlen( szInstanceName ) + 1 ) );
	strcpy( mszDBInstanceName, szInstanceName );
	
	// Create thread to handle requests..
	mhDBThread = SysCreateThread( (fnThreadFunction)DBRequestThread, (void*)this, 0, 0 );
}

void		DBInstance::Update( void )
{
	if ( mbDBCallbackPending == TRUE )
	{
		mpCurrentActiveRequest->mfnCallback( mnDBRequestResponseCode, mpCurrentActiveRequest->mnRequestID, mpCurrentActiveRequest->mullRecordUID, mpCurrentActiveRequest->mpcRequestData, mpCurrentActiveRequest->mvpCBParam );
		mbDBCallbackPending = FALSE;
		delete mpCurrentActiveRequest;
		mpCurrentActiveRequest = FALSE;
	}

	if ( mbDBRequestPending == FALSE )
	{
		if ( mpDBRequestList )
		{
			mpCurrentActiveRequest = mpDBRequestList;
			mpDBRequestList = mpDBRequestList->mpNext;
			mbDBRequestPending = TRUE;
		}
	}
}

void		DBInstance::Shutdown( void )
{
	mbDBKillThread = TRUE;
	SysSleep( 15 );
	SAFE_FREE( mszDBInstanceName );
}

//-------------------------------------------------------------------------------------------

void		DBReset( int nDBID )
{
DBInstance*		pDB = mspDBInstances;
int				nRequestID = NOTFOUND;

	while( pDB )
	{
		if ( pDB->mnDBInstanceID == nDBID )
		{
			pDB->ResetAll();
			return;
		}
		pDB = pDB->mpNextDBInstance;
	}

}

void		DBCreateInstance( int nDBID, const char* szDBName )
{
DBInstance*		pNewDB = new DBInstance;

	pNewDB->Initialise( nDBID, szDBName );

	pNewDB->mpNextDBInstance = mspDBInstances;
	mspDBInstances = pNewDB;
}

int		DBRetrieve( int nDBID, u64 ullRecordUID, DBResponse fnCallback, void* vpCBParam )
{
DBInstance*		pDB = mspDBInstances;
int				nRequestID = NOTFOUND;

	while( pDB )
	{
		if ( pDB->mnDBInstanceID == nDBID )
		{
			nRequestID = pDB->Retrieve( ullRecordUID, fnCallback, vpCBParam );
			return( nRequestID );
		}
		pDB = pDB->mpNextDBInstance;
	}

	// Assert!! - Invalid DB ID
	SysDebugPrint( "ERROR: Invalid DB ID" );
	return( nRequestID );
}

int		DBStore( int nDBID, u64 ullRecordUID, const char* pcJSON, DBResponse fnCallback, void* vpCBParam )
{
DBInstance*		pDB = mspDBInstances;
int				nRequestID = NOTFOUND;

	while( pDB )
	{
		if ( pDB->mnDBInstanceID == nDBID )
		{
			nRequestID = pDB->Store( ullRecordUID, pcJSON, fnCallback, vpCBParam );
			return( nRequestID );
		}
		pDB = pDB->mpNextDBInstance;
	}
	// Assert!! - Invalid DB ID
	return( nRequestID );
}

void		DBShutdownInstance( int nDBID )
{
DBInstance*		pDB = mspDBInstances;

	while( pDB )
	{
		if ( pDB->mnDBInstanceID == nDBID )
		{
			pDB->Shutdown();
		}
		pDB = pDB->mpNextDBInstance;
	}
}

void		DBUpdateAll( void )
{
DBInstance*		pDB = mspDBInstances;

	while( pDB )
	{
		pDB->Update();
		pDB = pDB->mpNextDBInstance;
	}
}

void		DBShutdownAll( void )
{
DBInstance*		pNewDB = mspDBInstances;
DBInstance*		pNext;

	while( pNewDB )
	{
		pNext = pNewDB->mpNextDBInstance;
		pNewDB->Shutdown();
		delete pNewDB;
		pNewDB = pNext;
	}
	// Delete all..
}
