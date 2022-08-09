#ifndef GAMECOMMON_UDPSESSION_CONNECTION_MESSAGES_H
#define GAMECOMMON_UDPSESSION_CONNECTION_MESSAGES_H

#define		UDPSESSION_CONNECT_FIXED_SYSTEM_IDENTIFIER				0xA653

#define		UDPSESSION_GUARANTEED_FIXED_SYSTEM_IDENTIFIER			0x7519
#define		UDPSESSION_NONGUARANTEED_FIXED_SYSTEM_IDENTIFIER		0x34E6

#define		UDPSESSION_CLIENTGUARREQUEST_FIXED_SYSTEM_IDENTIFIER	0x1728

#define		UDPSESSION_TERMINATE_FIXED_SYSTEM_IDENTIFIER			0x56BB

typedef struct
{
	ushort		uwFixedSystemIdentifier;
	BYTE		bConnectionVersion;
	BYTE		bEncryptKeySet;

	BYTE		bProtocolVersion;
	BYTE		bDestination;
	ushort		uwMsgLen;

} UDPSESSION_MSG_HEADER;


typedef struct
{
	UDPSESSION_MSG_HEADER		xMsgHeader;

	ulong		ulTimestamp;
	ushort		uwSystemValidationIdentifier;
	ushort		uwPad1;

	ulong		ulPad1;
	u64			ullUserUID;
	char		acSessionKey[32];

} UDPSESSION_CONNECT_MESSAGE;


typedef struct
{
	ushort		uwSystemValidationIdentifier;
	ushort		uwTimestamp;

} UDPSESSION_SCRAMBLED_MSG_HEADER;

typedef struct
{
	ushort		uwGuarID;
	BYTE		bIsReceiptFlag;
	BYTE		bPad1;

} UDPSESSION_GUARANTEED_MSG_HEADER;



#endif
