#include<Windows.h>
#include "RecvEventHandler.h"
#include"EstablishConn.h"
#include"Global.h"
#include"BOITEventType.h"
#include<strsafe.h>
#include"APITransfer.h"
#include<process.h>
#include<time.h>
unsigned __stdcall RecvEventThread(void* Args);


int StartRecvEventHandler()
{
	SYSTEM_INFO SysInfo;//����ȡ��CPU��������Ϣ
	GetSystemInfo(&SysInfo);

	for (unsigned int i = 0; i < SysInfo.dwNumberOfProcessors; i++)
	{
		_beginthreadex(NULL, 0, RecvEventThread, (LPVOID)i, 0, NULL);
	}
	return 0;
}


int InitailizeThreadCRT(int ThreadID)//������Щ�߳���Ҫ����ָ��ģ����������ʼ��CRT
{
	srand((unsigned int)time(0) + ThreadID);
	return 0;
}



unsigned __stdcall RecvEventThread(void *Args)
{
	InitailizeThreadCRT((int)Args);
	while (1)
	{
		while (GetConnState() == 1)
		{
			if (ConnWaitForObject(hEventRecvStart) == 0)
			{
				break;
			}
			EVENT_RECV RecvEvent;

			switch (pSharedMemRecv->EventType)
			{
			case BOIT_EVENT_RECV_PRIVATE:
				RecvEvent.EventType = BOIT_EVENT_RECV_PRIVATE;
				RecvEvent.u.PrivateMsg.QQID = pSharedMemRecv->u.PrivateMsg.QQID;
				StringCchCopyW(RecvEvent.u.PrivateMsg.Msg, BOIT_MAX_TEXTLEN, pSharedMemRecv->u.PrivateMsg.Msg);

				break;
			case BOIT_EVENT_RECV_GROUP:
				RecvEvent.EventType = BOIT_EVENT_RECV_GROUP;
				RecvEvent.u.GroupMsg.QQID = pSharedMemRecv->u.GroupMsg.QQID;
				RecvEvent.u.GroupMsg.GroupID = pSharedMemRecv->u.GroupMsg.GroupID;
				StringCchCopyW(RecvEvent.u.GroupMsg.Msg, BOIT_MAX_TEXTLEN, pSharedMemRecv->u.GroupMsg.Msg);
				StringCchCopyW(RecvEvent.u.GroupMsg.AnonymousName, BOIT_MAX_NICKLEN, pSharedMemRecv->u.GroupMsg.AnonymousName);

				break;
			default:
				break;
			}
			SetEvent(hEventRecvEnd);
			//�ַ�
			switch (RecvEvent.EventType)
			{
			case BOIT_EVENT_RECV_PRIVATE:
				RecvPrivateMessage(RecvEvent.u.PrivateMsg.QQID,
					RecvEvent.u.PrivateMsg.Msg);
				break;
			case BOIT_EVENT_RECV_GROUP:
				RecvGroupMessage(RecvEvent.u.GroupMsg.GroupID,
					RecvEvent.u.GroupMsg.QQID,
					RecvEvent.u.GroupMsg.AnonymousName,
					RecvEvent.u.GroupMsg.Msg);
				break;
			}
		}
		Sleep(1000);
	}
}



