#ifndef GAMECOMMON_NETWORK_SESSION_CONNECTION_H
#define GAMECOMMON_NETWORK_SESSION_CONNECTION_H


class SessionConnection;

typedef	void(*SessionConnectionDisconnectCallback)( SessionConnection* pConnection, void* pUserData );

class SessionConnection
{
public:
	SessionConnection();
	virtual ~SessionConnection();

	virtual	void		SendGuaranteedMessage( void* pcMsg, int nMsgLen ) = 0;
	
	virtual void		SendNonGuaranteedMessage( void* pcMsg, int nMsgLen ) = 0;
		
	virtual BOOL		IsConnected( void ) = 0;

	virtual void		Disconnect( void ) = 0;


	void		AddDisconnectCallback( SessionConnectionDisconnectCallback fnCallback, void* pUserData );

	void		OnConnect( void );
	void		OnDisconnect( void );

	//---------------------------------------------------------------
	void		SetUserDataShort( ushort uwData ) { muwUserData = uwData; }
	ushort		GetUserDataShort( void ) { return( muwUserData ); }

	void		SetUserUID( u64 ullUID ) { mullUserUID = ullUID; }
	u64			GetUserUID( void ) { return( mullUserUID ); }

private:

	class DisconnectCallbackList
	{
	public:
		SessionConnectionDisconnectCallback		mfnCallback;
		void*									mpUserData;

		DisconnectCallbackList*					mpNext;
	};

	DisconnectCallbackList*		mpDisconnectCallbackList;

	ushort					muwUserData;
	u64						mullUserUID;

};





#endif