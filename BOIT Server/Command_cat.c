#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"SharedMemStruct.h"
#include<wchar.h>
#include"HandleBOITCode.h"


int CmdMsg_cat_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	WCHAR CatMessage[128];
	
	if (GetBOITSessionType(boitSession) == BOITSESS_TYPE_GROUP)
	{
		if (GetBOITSessionQQID(boitSession) == 80000000)
		{
			return 0;
		}
		else
		{
			BOIT_GROUPMEMBER_INFO GroupMemberInfo;
			RetrieveGroupMemberInfo(boitSession, TRUE, &GroupMemberInfo);
			if (GetBOITSessionQQID(boitSession) == 693511570)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"��ߴ��ߴ�� %ls ��Ҳ�����Ӱ�", GroupMemberInfo.NickName);
			}
			else if (GetBOITSessionQQID(boitSession) == 1976658142)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ�Ǻ��갡", GroupMemberInfo.NickName);
			}
			else
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ��èè��", GroupMemberInfo.NickName);
			}
			
		}
	}
	else
	{
		BOIT_STRANGER_INFO StrangerInfo;
		RetrieveStrangerInfo(boitSession, TRUE, &StrangerInfo);

		if (GetBOITSessionQQID(boitSession) == 693511570)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"��ߴ��ߴ�� %ls ��Ҳ�����Ӱ�", StrangerInfo.NickName);
		}
		else if (GetBOITSessionQQID(boitSession) == 1976658142)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ�Ǻ��갡", StrangerInfo.NickName);
		}
		else
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ��èè��", StrangerInfo.NickName);
		}
		
	}
	SendTextWithBOITCode(boitSession, CatMessage, SWBC_PARSE_AT | SWBC_PARSE_AT_ALL | SWBC_PARSE_IMG_URL);
	return 0;
}




int CmdMsg_meow_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	WCHAR CatMessage[128];

	if (GetBOITSessionType(boitSession) == BOITSESS_TYPE_GROUP)
	{
		if (GetBOITSessionQQID(boitSession) == 80000000)
		{
			return 0;
		}
		else
		{
			BOIT_GROUPMEMBER_INFO GroupMemberInfo;
			RetrieveGroupMemberInfo(boitSession, TRUE, &GroupMemberInfo);
			WCHAR* ChosenName;
			if (GroupMemberInfo.CardName[0])
			{
				ChosenName = GroupMemberInfo.CardName;
			}
			else
			{
				ChosenName = GroupMemberInfo.NickName;
			}
			if (GetBOITSessionQQID(boitSession) == 693511570)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"��ߴ��ߴ�� %ls ��Ҳ�����Ӱ�", ChosenName);
			}
			else if (GetBOITSessionQQID(boitSession) == 1976658142)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ�Ǻ��갡", ChosenName);
			}
			else
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ��èè��", ChosenName);
			}

		}
	}
	else
	{
		BOIT_STRANGER_INFO StrangerInfo;
		RetrieveStrangerInfo(boitSession, TRUE, &StrangerInfo);

		if (GetBOITSessionQQID(boitSession) == 693511570)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"��ߴ��ߴ�� %ls ��Ҳ�����Ӱ�", StrangerInfo.NickName);
		}
		else if (GetBOITSessionQQID(boitSession) == 1976658142)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ�Ǻ��갡", StrangerInfo.NickName);
		}
		else
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ��èè��", StrangerInfo.NickName);
		}

	}
	SendTextWithBOITCode(boitSession, CatMessage, SWBC_PARSE_AT | SWBC_PARSE_AT_ALL | SWBC_PARSE_IMG_URL);
	return 0;
}