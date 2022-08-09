#ifndef NEWCHATSERVER_MESSAGES_H
#define NEWCHATSERVER_MESSAGES_H


enum
{
	NCHATMSGID_INITIAL_LOGIN = 0x10,
	NCHATMSGID_AUTHENTICATE_RESPONSE = 0x11,
	NCHATMSGID_AUTHENTICATED_LOGIN = 0x12,
	NCHATMSGID_AUTHENTICATED_LOGIN_RESPONSE = 0x13,
	NCHATMSGID_ADMIN_COMMAND = 0x14,
	NCHATMSGID_STD_CHAT_MSG = 0x15,
	NCHATMSGID_CHAT_CMD_MSG = 0x16,
	NCHATMSGID_USER_LIST_CHAT_MSG = 0x17,
	NCHATMSGID_STATS_MSG = 0x18,
};

typedef struct
{
	BYTE	bMsgID;
	BYTE	bProtocolVersion;
	BYTE	bPad2;
	BYTE	bPad3;

	byte	acMACAddress[8];
	u64		ullUserUID;

	char	szSessionKey[48];
	char	szLoginUserName[24];

} CHATSERVICE_INITIAL_LOGIN_MSG;


typedef struct
{
	BYTE	bMsgID;
	BYTE	bResponseCode;
	BYTE	bDestinationMode;
	BYTE	bExtraTicketLen;

	ulong	ulDestinationIP;
	ushort	uwDestinationPort;
	ushort	uwPad1;

	ulong	ulDestinationUID;
	ulong	ulUserFlags;

	ulong	ulNumOnline;
	ulong	ulNumGroups;
	ulong	ulPad3;
	ulong	ulPad4;
	ulong	ulPad5;
	ulong	ulPad6;

} NCHAT_AUTHENTICATED_LOGIN_RESPONSE_MSG;


typedef struct
{
	BYTE	bMsgID;
	BYTE	bGroupNum;
	ushort	uwTextLen;

	ulong	ulTimestamp;
	ulong	ulPad1;

} NCHAT_CHAT_MSG;

typedef struct
{
	BYTE	bMsgID;
	BYTE	bResponseCode;
	BYTE	bExtraResponseLen;
	BYTE	bKeyNum;

	ushort	uwYourUpperSessionID;
	ushort	uwNewUpperSessionID;
	ushort	uwYourLowerSessionID;
	ushort	uwNewLowerSessionID;

	ulong	ulTime;

	ulong	ulYourUID;
	
	ulong	ulYourExternalIP;
	ulong	ulPad2;

} AUTHENTICATE_RESPONSE_MSG;



#endif
