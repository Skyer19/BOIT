#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"SharedMemStruct.h"
#include<wchar.h>

#include"SendEventDispatch.h" //������ʱ�ģ���������APITransfer
int CmdMsg_cat_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	WCHAR CatMessage[128];
	
	if (GroupID)
	{
		if (QQID == 80000000)
		{
			return 0;
		}
		else
		{
			BOIT_GROUPMEMBER_INFO GroupMemberInfo;
			SendEventGetGroupMemberInfo(GroupID, QQID, TRUE, &GroupMemberInfo);
			if (QQID == 693511570)
			{
				swprintf_s(CatMessage, _countof(CatMessage), L"�ۺ��ۺ��� %ls ��Ҳ�����Ӱ�", GroupMemberInfo.NickName);
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
		SendEventGetStrangerInfo(QQID, TRUE, &StrangerInfo);

		if (QQID == 693511570)
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"�ۺ��ۺ��� %ls ��Ҳ�����Ӱ�", StrangerInfo.NickName);
		}
		else
		{
			swprintf_s(CatMessage, _countof(CatMessage), L"�������� %ls ��Ҳ��èè��", StrangerInfo.NickName);
		}
		
	}
	SendBackMessage(GroupID, QQID, CatMessage);
	return 0;
}