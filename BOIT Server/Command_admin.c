#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"
#include"DirManagement.h"

WCHAR* AvaliableFunc[] = { L"RunCode", L"QueryOIer", L"QueryLuoguProblem" };

int HavePermissionToAdmin(pBOIT_SESSION boitSession)
{
	BOIT_GROUPMEMBER_INFO GroupMemberInfo;
	RetrieveGroupMemberInfo(boitSession, TRUE, &GroupMemberInfo);
	switch (GroupMemberInfo.ManageLevel)
	{
	case 2:
	case 3:
		return TRUE;
	default:
		return FALSE;
	}
}

int AdminSendHelp(pBOIT_SESSION boitSession)
{

	return 0;
}

BOOL GroupEnableDisableFunc(long long GroupID, WCHAR FuncName[], BOOL bEnable)
{
	for (int i = 0; i < _countof(AvaliableFunc); i++)
	{
		if (_wcsicmp(FuncName, AvaliableFunc[i]) == 0)
		{
			WCHAR FullPrivFileName[MAX_PATH + 2] = L"Token\\Privilege";
			wcscat_s(FullPrivFileName, MAX_PATH, FuncName);
			if (bEnable)
			{
				PerGroupCreateFileIfNExist(GroupID, FullPrivFileName);
				//TODO: �����顣��������������ܻ����ļ��Ѿ����ڵ�����·��������������ļ����ڷ���ɾ����ʱ�򷵻�ʧ�ܡ����иĽ���
			}
			else
			{
				PerGroupDeleteFile(GroupID, FullPrivFileName);
				//TODO: �����顣��������������ܻ����ļ��Ѿ����ڵ�����·��������������ļ����ڷ���ɾ����ʱ�򷵻�ʧ�ܡ����иĽ���
			}
			return TRUE;
		}
	}
	return FALSE;
}


int CmdMsg_admin_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	if (GetBOITSessionType(boitSession) != BOITSESS_TYPE_GROUP)
	{
		SendBackMessage(boitSession, L"����Ⱥ����ʹ�ø�ָ��");
		return 0;
	}

	if (!HavePermissionToAdmin(boitSession))
	{
		SendBackMessage(boitSession, L"�����Ǳ�Ⱥ��Ⱥ�������Ա����Ȩʹ�ø�ָ��");
		return 0;
	}


	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	int OrderStrlen = GetCmdParamLen(Msg + ParamLen + SpaceLen);

	if (OrderStrlen >= 64)
	{
		SendBackMessage(boitSession, L"�޷�ʶ��ָ�����#admin help�鿴����");
		return 0;
	}
	else if (OrderStrlen == 0)
	{
		SendBackMessage(boitSession, L"����#admin help�鿴����");
		return 0;
	}

	WCHAR* OrderStr[] = { L"enable",L"disable",L"list", L"help" };
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
	case 0: // enable
	case 1: // disable
	{
		WCHAR* OrderBase = Msg + ParamLen + SpaceLen + OrderStrlen;
		OrderBase += GetCmdSpaceLen(OrderBase);
		int QueryStrlen = GetCmdParamLen(OrderBase);
		if (QueryStrlen >= 64)
		{
			SendBackMessage(boitSession, L"δ�ҵ�ָ������");
		}
		else if (QueryStrlen == 0)
		{
			SendBackMessage(boitSession, L"��Ҫ���û��ǽ����ĸ����ܣ�(�÷� #admin enable/disable ���������� #admin list �鿴����)");
		}
		else
		{
			if (GroupEnableDisableFunc(GetBOITSessionGroupID(boitSession), OrderBase, iMatch == 0))
			{
				SendBackMessage(boitSession, (iMatch == 0) ? L"�ѳɹ����øù���" : L"�ѳɹ��رոù���");
			}
			else
			{
				SendBackMessage(boitSession, L"δ�ҵ��ù��ܡ�#admin list�Բ鿴�ɿ��ƵĹ����б�");
			}

		}
	}
	break;
	case 2: // list
		//TODO: ��Ϊ���Ǹ�����������
		SendBackMessage(boitSession, L"Ŀǰ���Կ��ƵĹ��ܣ�\nRunCode\nQueryOIer\nQueryLuoguProblem");
		break;
	case 3: // help
		SendBackMessage(boitSession, L"����Ⱥ����bot����\n#admin enable/disbale �������Դ򿪹رչ��ܡ�\n#admin list�Բ鿴�ɿ��ƵĹ����б�\n#admin help�Բ鿴�ð�����Ϣ��");
		break;
	default:
		SendBackMessage(boitSession, L"δ�ҵ�ָ����� #admin help�鿴����");
	}


	//���
	//	���°�������ָ��ϲ���һ��
	//	#admin
	//	���ǻ����õ�ģ�ֱ�۵�����֣� manage? control? ctrl? 
	//	#admin enable xxx
	//	#admin disable xxx
	//	#admin help
	//	#admin list
	//	#admin set ĳ��
	//	....

	return 0;
}
