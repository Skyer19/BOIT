#include<Windows.h>
#include<stdlib.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"


int CmdMsg_luogu_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	//if (boitSession->GroupID && CheckGroupToken(boitSession->GroupID, L"PrivilegeQueryCFUser") == 0)
	//{
	//	SendBackMessage(boitSession, L"��Ⱥ��ֹ�˲�ѯCF User���ܡ�����˽���в�ѯ��");
	//	return 0;
	//}



	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	int OrderStrlen = GetCmdParamLen(Msg + ParamLen + SpaceLen);

	if (OrderStrlen >= 64)
	{
		SendBackMessage(boitSession, L"�޷�ʶ��ָ��");
		return 0;
	}
	else if (OrderStrlen == 0)
	{
		SendBackMessage(boitSession, L"����#luogu help�鿴����");
		return 0;
	}

	WCHAR* OrderStr[] = { L"help",L"profile",L"problem"};
	int iMatch;
	for (iMatch = 0; iMatch < _countof(OrderStr); iMatch++)
	{
		if (OrderStrlen == wcslen(OrderStr[iMatch]) && (_wcsnicmp(Msg + ParamLen + SpaceLen, OrderStr[iMatch], OrderStrlen) == 0))
		{
			break;
		}
	}
	switch (iMatch)
	{
	case 0:
		SendBackMessage(boitSession, L"��ѯ�й�luogu����Ϣ��\n#luogu profile [�û���] �Բ�ѯprofile\n#luogu problem �Բ�ѯ��Ŀ��Ϣ\n#luogu help �Բ鿴�ð�����Ϣ");

		break;
	case 1:
	{
		SendBackMessage(boitSession, Corpus_FunctionDeving());
	}

	break;
	case 2:
	{
		SendBackMessage(boitSession, Corpus_FunctionDeving());
	}

		break;
	default:
		SendBackMessage(boitSession, L"δ�ҵ�ָ�����#luogu help�鿴����");
		break;
	}
	/*else
	{

	}*/

	return 0;
}
