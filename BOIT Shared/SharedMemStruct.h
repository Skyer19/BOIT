#pragma once
#include<Windows.h>

#define BOIT_MAX_TEXTLEN 2048 //�������ȵ�һ�ɽض�
#define BOIT_MAX_NICKLEN 64

typedef struct __tagEventRecvStruct
{
	int EventType;

	union
	{
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long GroupID;
			long long QQID;
			WCHAR AnonymousName[BOIT_MAX_NICKLEN + 4];
			int iRet;
		}GroupMsg;
		 
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long QQID;
			int iRet;
		}PrivateMsg;
	}u;
}EVENT_RECV, *pEVENT_RECV;


typedef struct __tagEventSendStruct
{
	int EventType;

	union
	{
		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long GroupID;
			int iRet;
		}GroupMsg;

		struct
		{
			WCHAR Msg[BOIT_MAX_TEXTLEN + 4];
			long long QQID;
			int iRet;
		}PrivateMsg;
	}u;
}EVENT_SEND,*pEVENT_SEND;


typedef struct __tagSharedProcessInfo
{
	DWORD pid[2];
}SHARED_PROCESS_INFO, *pSHARED_PROCESS_INFO;
