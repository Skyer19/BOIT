#pragma once
#include<Windows.h>
#include"BOITInfoMaxDef.h"

//��ϵ�ˣ���QQ���ѣ�Ⱥ�ѣ���Ϣ�ṹ��
typedef struct __tagGroupMemberInfo
{
	long long GroupID;
	long long QQID;
	WCHAR NickName[BOIT_MAX_NICKLEN + 4];
	WCHAR CardName[BOIT_MAX_NICKLEN + 4];
	int Gender; // 0/���� 1/Ů�� 255/δ֪
	int Age;
	WCHAR Location[BOIT_MAX_LOCATION + 4]; // ����
	int EnterGroupTime;
	int LastActive;//�����ʱ��
	WCHAR LevelName[BOIT_MAX_LOCATION + 4]; // ����
	int ManageLevel; // ����Ȩ�� 1/��Ա 2/����Ա 3/Ⱥ��
	BOOL bBadRecord; //������¼
	WCHAR SpecialTitle[BOIT_MAX_NICKLEN]; //ר��ͷ��
	int SpecTitExpire;//ר��ͷ�ι���ʱ�䡣-1������
	BOOL bAllowEditCard;//�Ƿ�����༭Ⱥ��Ƭ

}BOIT_GROUPMEMBER_INFO, * pBOIT_GROUPMEMBER_INFO;


typedef struct __tagStrangerInfo
{
	long long QQID;
	WCHAR NickName[BOIT_MAX_NICKLEN + 4];
	int Gender; // 0/���� 1/Ů�� 255/δ֪
	int Age;
}BOIT_STRANGER_INFO, * pBOIT_STRANGER_INFO;


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

		struct
		{
			long long GroupID;
			long long QQID;
			BOOL NoCache;
			BOIT_GROUPMEMBER_INFO GroupMemberInfo;

			int iRet; // 1����ɹ� 0����ʧ��
		}GroupMemberInfo;


		struct
		{
			long long QQID;
			BOOL NoCache;
			BOIT_STRANGER_INFO StrangerInfo;

			int iRet; // 1����ɹ� 0����ʧ��
		}StrangerInfo;
	}u;
}EVENT_SEND,*pEVENT_SEND;


typedef struct __tagSharedProcessInfo
{
	DWORD pid[2];
}SHARED_PROCESS_INFO, *pSHARED_PROCESS_INFO;
