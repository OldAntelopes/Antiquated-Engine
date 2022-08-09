#ifndef GAMECOMMON_UDPSESSION_CONNECTION_H
#define GAMECOMMON_UDPSESSION_CONNECTION_H

#include "UDPSessionConnectionMessages.h"
#include "../SessionConnection.h"

#define		MAX_OUT_OF_ORDER_RECEIPTS		32

class UDPSessionConnection;

typedef	void(*UDPSessionConnectionMessageReceiveHandler)( unsigned char*, UDPSessionConnection* pConnection, void* pUserData );

class UDPSessionConnection : public SessionConnection
{
public:
	UDPSessionConnection();
	virtual ~UDPSessionConnection();

	virtual void		SendGuaranteedMessage( void* pcMsg, int nMsgLen );
	virtual void		SendNonGuaranteedMessage( void* pcMsg, int nMsgLen );
	virtual BOOL		IsConnected( void );
	virtual void		Disconnect( void );

	void		InitConnectionOnHost( ulong ulClientIP, ushort uwClientPort, const char* szSessionKey, u64 ullUserUID );
	void		UpdateConnectionHost( float delta );

	BOOL		InitConnectionOnClient( ulong ulHostIP, ushort uwHostPort, const char* szSessionKey, u64 ullUserUID );
	void		UpdateConnectionClient( float delta );

	//--------------------------------------------------------------------

	BOOL		IsThisAddress( ulong ulIP, ushort uwPort );
	BOOL		IsPending( void ) { return( m_bIsVerifying ); }
	BOOL		IsVerified( void ) { return( m_bHasVerified ); }
	BOOL		HasDisconnected( void ) { return( m_bHasDisconnected ); }
	BOOL		IsDead( void );

	
	void		ResendTermination( void );
	void		OnSessionValidated( void );
	void		OnConnectAccept( UDPSESSION_CONNECT_MESSAGE* pxConnectAcceptMsg );

	void		OnReceiveGuaranteedMessage( const char* pcMsg );
	void		OnReceiveNonGuaranteedMessage( const char* pcMsg );
	void		OnClientRequestGuarUpdate( const char* pcMsg );
	void		OnReceiveTerminationMessage( const char* pcMsg );

	//--------------------------------------------------------------------------
	void		RegisterMessageHandler( UDPSessionConnectionMessageReceiveHandler fnMessageHandler, void* pUserData );

	void		InitConnectionTest( void );
	//--------------------------------------------------------------------------
	UDPSessionConnection*		GetNext( void ) { return( mpNext ); }
	void						SetNext( UDPSessionConnection* pNext ) { mpNext = pNext; }

private:
	void		OnCommunicationReceive( void );

	bool		SetOutboundMessage( const char* pcMsg, int nMsgLen );
	void		ClearOutboundMessage( void );

	class UDPSessionGuaranteedMessage
	{
	public:

		void			Send( UDPSessionConnection* pConnection );

		ushort			m_uwGuarID;
	
		char*			mpcMsg;
		int				mnMsgLen;
		ulong			mulLastSendTimestamp;
		int				mnNumRepeatSends;

		UDPSessionGuaranteedMessage*		mpNext;
	};

	class UDPSessionOutOfOrderReceipt
	{
	public:
		ushort		uwGuarID;

		UDPSessionOutOfOrderReceipt*	mpNext;
	};

	void		InitBuffers( void );

	void		SendGuaranteedReceipt( ushort uwGuarID, ushort uwTimestamp );
	void		SendTermination( void );

	void		ScrambleMessage( BYTE* pbMsg, int nLen, ushort uwTimestamp );
	void		UnscrambleMessage( BYTE* pbMsg, int nLen );
				
	void		WriteMsgHeader( UDPSESSION_MSG_HEADER* pxHeader );

	// TODO - Make static?
	UDPSessionConnectionMessageReceiveHandler		mfnMessageHandler;

	void*											mpMessageHandlerUserParam;

	// TODO - Replace all these with bitflags to save space in this struct
	BOOL					m_bHasInitialisedBuffers;
	BOOL					m_bIsVerifying;
	BOOL					m_bHasVerified;
	BOOL					m_bIsHost;
	BOOL					m_bHasDisconnected;
	ulong					m_ulIP;
	ushort					m_uwPort;

	ushort					m_uwNextSendGuarID;
	ushort					m_uwLastRecvGuarID;

	float					m_fClientTimeSinceLastGuarRequest;
	float					m_fClientTimeSinceLastGuarRepeatSend;
	
	ulong					m_ulLoginStartTime;
	ulong					m_ulLastTerminateSend;
	ulong					m_ulLastMessageReceived;

	BYTE					m_abScrambleKey[24];
	int						mnScrambleKeyLength;
 	
	char					m_aOutboundMsgBuff[128];
	int						m_nOutboundMsgLen;
	ulong					mulOutboundMsgLastSendTick;
	int						mnOutboundMsgRepeatTries;

	UDPSessionOutOfOrderReceipt*		mpOutOfOrderReceipts;
	UDPSessionOutOfOrderReceipt*		mpFreeOutOfOrderReceipts;

	ushort					m_uwGuarIDsReceivedUpTo;
	ushort					m_uwSystemValidationIdentifier;

	UDPSessionGuaranteedMessage*		mpGuaranteedMessageSendBuffer;

	UDPSessionConnection*		mpNext;

};


//---------------------------------------------------------------------

typedef	void(*UDPSendMessageFunction)( char* pcMsg, int nMsgLen, ulong ulToIP, ushort uwToPort, int nFlags );

extern void		UDPSessionRegisterMessageSendFunction( UDPSendMessageFunction fnUDPMessageSend );


#endif
