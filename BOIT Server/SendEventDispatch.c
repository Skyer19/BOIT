#include<Windows.h>
#include"Global.h"
#include"EstablishConn.h"
#include"BOITEventType.h"
#include<strsafe.h>
#include "SendEventDispatch.h"
#include"SharedMemStruct.h"


int InitSendEventDispatch()
{
	InitializeSRWLock(&SendLock);
	return 0;
}

int SendEventPrivateMsg(long long QQID, WCHAR * Msg)
{
	if (GetConnState() == 0)
	{
		printf("��CoolQ֮��������Ѿ��Ͽ�����Ϣδ����");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_PRIVATE;
		StringCchCopyW(pSharedMemSend->u.PrivateMsg.Msg, BOIT_MAX_TEXTLEN, Msg);
		pSharedMemSend->u.PrivateMsg.QQID = QQID;
		SetEvent(hEventSendStart);
		
		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//��ȡ����ֵ
		printf("������ϢID��%d\n", pSharedMemSend->u.PrivateMsg.iRet);

		SetEvent(hEventSendRet);

		//�ɹ�
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}

int SendEventGroupMsg(long long GroupID, WCHAR* Msg)
{
	if (GetConnState() == 0)
	{
		printf("��CoolQ֮��������Ѿ��Ͽ�����Ϣδ����");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_GROUP;
		StringCchCopyW(pSharedMemSend->u.GroupMsg.Msg, BOIT_MAX_TEXTLEN, Msg);
		pSharedMemSend->u.GroupMsg.GroupID = GroupID;
		SetEvent(hEventSendStart);

		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//�ɹ�
		printf("������ϢID��%d\n", pSharedMemSend->u.GroupMsg.iRet);

		SetEvent(hEventSendRet);
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}

int SendEventGetGroupMemberInfo(long long GroupID, long long QQID, BOOL NoCache, pBOIT_GROUPMEMBER_INFO GroupMemberInfo)
{
	if (GetConnState() == 0)
	{
		printf("��CoolQ֮��������Ѿ��Ͽ�����Ϣδ����");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_GET_GROUPMEMBER_INFO;
		pSharedMemSend->u.GroupMemberInfo.GroupID = GroupID;
		pSharedMemSend->u.GroupMemberInfo.QQID = QQID;
		pSharedMemSend->u.GroupMemberInfo.NoCache = TRUE;
		SetEvent(hEventSendStart);

		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//�ɹ�
		GroupMemberInfo->GroupID = pSharedMemSend->u.GroupMemberInfo.GroupID;
		GroupMemberInfo->QQID = pSharedMemSend->u.GroupMemberInfo.QQID;
		wcscpy_s(GroupMemberInfo->NickName, BOIT_MAX_NICKLEN, pSharedMemSend->u.GroupMemberInfo.GroupMemberInfo.NickName);

		SetEvent(hEventSendRet);
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}



int SendEventGetStrangerInfo(long long QQID, BOOL NoCache, pBOIT_STRANGER_INFO StrangerInfo)
{
	if (GetConnState() == 0)
	{
		printf("��CoolQ֮��������Ѿ��Ͽ�����Ϣδ����");
		return 0;
	}
	AcquireSRWLockExclusive(&SendLock);
	__try
	{
		pSharedMemSend->EventType = BOIT_EVENT_SEND_GET_STRANGER_INFO;
		pSharedMemSend->u.StrangerInfo.QQID = QQID;
		pSharedMemSend->u.StrangerInfo.NoCache = TRUE;
		SetEvent(hEventSendStart);

		if (ConnWaitForObject(hEventSendEnd) == 0)
		{
			__leave;
		}
		//�ɹ�
		StrangerInfo->QQID = pSharedMemSend->u.StrangerInfo.QQID;
		wcscpy_s(StrangerInfo->NickName, BOIT_MAX_NICKLEN, pSharedMemSend->u.StrangerInfo.StrangerInfo.NickName);

		SetEvent(hEventSendRet);
	}
	__finally
	{
		ReleaseSRWLockExclusive(&SendLock);
	}
	return 0;
}