#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Global.h"
#include"DirManagement.h"

int CmdMsg_stop_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	if (CheckUserToken(QQID, L"PrivilegeBOITStop"))
	{
		SendBackMessage(GroupID, QQID, L"Goodbye~");
		SetEvent(hEventServerStop);
	}
	else
	{
		SendBackMessage(GroupID, QQID, L"Oops... ��û���ʵ���Ȩ�޽��в���");
	}
	
	return 0;
}