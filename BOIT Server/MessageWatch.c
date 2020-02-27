#include<Windows.h>
#include"MessageWatch.h"
#include<process.h>




unsigned __stdcall MsgWatchTimerThread(void* Args);

void __stdcall MsgWatchTimerCallback(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
);

long long MsgWatchAllocID;

HANDLE hMsgWatchTimerThread; //��Ϣ���Ӽ�ʱ���߳̾��

HANDLE hEventMsgWatchCleaning; //�����¼�

HANDLE hEventPassArgStart; //����ʱ���̴߳��ο�ʼ

HANDLE hEventPassArgEnd; //����ʱ���������

SRWLOCK PassArgLock; // ������ ***************************
long long ArgAllocID;		// In
long long ArgTimeOut;		// In
HANDLE ArgWaitableTimer;	// Out
//********************************************************



int InitializeMessageWatch()
{
	InitializeCriticalSection(&MsgWatchChainLock);
	InitializeSRWLock(&PassArgLock);
	MsgWatchAllocID = 0;

	RootMsgWatch = 0;

	hEventMsgWatchCleaning = CreateEvent(0, TRUE, 0, 0);

	hEventPassArgStart = CreateEvent(0, 0, 0, 0);
	hEventPassArgEnd = CreateEvent(0, 0, 0, 0);

	//������ʱ���߳�
	hMsgWatchTimerThread = _beginthreadex(0, 0, MsgWatchTimerThread, 0, 0, 0);
	return 0;
}


int FinalizeMessageWatch()
{
	EnterCriticalSection(&MsgWatchChainLock);
	if (RootMsgWatch)
	{
		for (pBOIT_MSGWATCH pList = RootMsgWatch; pList; pList = pList->Next)
		{
			free(pList);
		}
	}
	LeaveCriticalSection(&MsgWatchChainLock);
	RootMsgWatch = 0;

	SetEvent(hEventMsgWatchCleaning);
	WaitForSingleObject(hMsgWatchTimerThread, INFINITE);

	CloseHandle(hMsgWatchTimerThread);
	CloseHandle(hEventMsgWatchCleaning);
	CloseHandle(hEventPassArgStart);
	CloseHandle(hEventPassArgEnd);

	DeleteCriticalSection(&MsgWatchChainLock);
	return 0;
}

int RegisterMessageWatch(int WatchType,
	long long TimeOutInterval,// -1�����������
	long long GroupID,
	long long QQID,
	int SubType,
	WCHAR* AnonymousName,
	MSGWATCH_CALLBACK CallbackFunc,
	PBYTE pData)
{
	pBOIT_MSGWATCH MsgWatch = malloc(sizeof(BOIT_MSGWATCH));
	ZeroMemory(MsgWatch, sizeof(BOIT_MSGWATCH));

	MsgWatch->GroupID = GroupID;
	MsgWatch->QQID = QQID;
	MsgWatch->WatchType = WatchType;
	MsgWatch->MsgWatchID = InterlockedIncrement64(&MsgWatchAllocID);
	MsgWatch->Callback = CallbackFunc;
	MsgWatch->pData = pData;

	EnterCriticalSection(&MsgWatchChainLock);
	__try
	{
		pBOIT_MSGWATCH OrgHead = RootMsgWatch;
		RootMsgWatch = MsgWatch;
		MsgWatch->Next = OrgHead;
		if (OrgHead)
		{
			OrgHead->Last = MsgWatch;
		}
	}
	__finally
	{
		LeaveCriticalSection(&MsgWatchChainLock);
	}

	if (TimeOutInterval != -1)
	{
		AcquireSRWLockExclusive(&PassArgLock);
		ArgTimeOut = TimeOutInterval;
		ArgAllocID = MsgWatch->MsgWatchID;

		SignalObjectAndWait(hEventPassArgStart, hEventPassArgEnd, INFINITE, 0);

		MsgWatch->hTimer = ArgWaitableTimer;
		ReleaseSRWLockExclusive(&PassArgLock);
	}
	

	return 0;
}



int RemoveMessageWatchFromListUnSafe(pBOIT_MSGWATCH MsgWatch)
{
	if (MsgWatch->Last)
	{
		MsgWatch->Last->Next = MsgWatch->Next;
	}
	else
	{
		RootMsgWatch = MsgWatch->Next;
	}

	if (MsgWatch->Next)
	{
		MsgWatch->Next->Last = MsgWatch->Last;
	}
	return 0;
}


int RemoveMessageWatch(pBOIT_MSGWATCH MsgWatch)
{
	EnterCriticalSection(&MsgWatchChainLock);
	__try
	{
		RemoveMessageWatchFromListUnSafe(MsgWatch);
	}
	__finally
	{
		LeaveCriticalSection(&MsgWatchChainLock);
	}

	free(MsgWatch);
	return 0;
}


int RemoveMessageWatchByID(long long MsgWatchAllocID)
{
	EnterCriticalSection(&MsgWatchChainLock);
	__try
	{
		if (RootMsgWatch)
		{
			for (pBOIT_MSGWATCH pList = RootMsgWatch; pList; pList = pList->Next)
			{
				if (pList->MsgWatchID == MsgWatchAllocID)
				{
					RemoveMessageWatchFromListUnSafe(pList);
					if (pList->hTimer)
					{
						CloseHandle(pList->hTimer);
					}
					free(pList);
					__leave;
				}
			}
		}
	}
	__finally
	{
		LeaveCriticalSection(&MsgWatchChainLock);
	}
	
	return 0;
}


//int MarkFreeMessageWatchByID(long long MsgWatchAllocID)
//{
//	EnterCriticalSection(&MsgWatchChainLock);
//	__try
//	{
//		if (RootMsgWatch)
//		{
//			for (pBOIT_MSGWATCH pList = RootMsgWatch; pList; pList = pList->Next)
//			{
//				if (pList->MsgWatchID == MsgWatchAllocID)
//				{
//					pList->ToBeFree = TRUE;
//					__leave;
//				}
//			}
//		}
//	}
//	__finally
//	{
//		LeaveCriticalSection(&MsgWatchChainLock);
//	}
//
//	return 0;
//}


unsigned __stdcall MsgWatchTimerThread(void* Args)
{
	while (1)
	{
		HANDLE WaitHandleList[2] = { hEventMsgWatchCleaning , hEventPassArgStart };
		DWORD dwRet = WaitForMultipleObjectsEx(2, WaitHandleList, FALSE, INFINITE, TRUE);

		switch (dwRet)
		{
		case WAIT_OBJECT_0:
			return 0;
		case WAIT_OBJECT_0 + 1:
		{
			HANDLE hTimer = CreateWaitableTimer(0, 0, 0);
			LARGE_INTEGER LiTimeOut;
			LiTimeOut.QuadPart = -ArgTimeOut;
			SetWaitableTimer(hTimer, &LiTimeOut, 0, MsgWatchTimerCallback, (LPVOID)ArgAllocID, FALSE);
			ArgWaitableTimer = hTimer;
			SetEvent(hEventPassArgEnd);
		}
		break;
		}
	}
	return 0;
}


void __stdcall MsgWatchTimerCallback(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
)
{
	MSGWATCH_CALLBACK CallbackFunc = 0;
	PBYTE pData = 0;
	EnterCriticalSection(&MsgWatchChainLock);
	if (RootMsgWatch)
	{
		for (pBOIT_MSGWATCH pList = RootMsgWatch; pList; pList = pList->Next)
		{
			if (pList->MsgWatchID == lpArgToCompletionRoutine)
			{
				CallbackFunc = pList->Callback;
				pData = pList->pData;
			}
		}
	}
	//RemoveMessageWatchByID(lpArgToCompletionRoutine,);
	LeaveCriticalSection(&MsgWatchChainLock);

	if (CallbackFunc)
	{
		CallbackFunc(lpArgToCompletionRoutine, pData, BOIT_MW_EVENT_TIMEOUT, 0, 0, 0, 0, 0);
	}
	return;
}


int MessageWatchFilter(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	BOOL bPass = TRUE;
	int iRet;
	EnterCriticalSection(&MsgWatchChainLock);

	__try
	{
		if (RootMsgWatch)
		{
			for (pBOIT_MSGWATCH pList = RootMsgWatch; pList;)
			{
				pBOIT_MSGWATCH Next = pList->Next;

				switch (pList->WatchType)
				{
				case BOIT_MW_ALL:			// �����κ��ˣ��κ�;������Ϣ
					if (pList->Callback)
					{
						iRet = pList->Callback(pList->MsgWatchID, pList->pData, BOIT_MW_EVENT_MESSAGE, GroupID, QQID, SubType, AnonymousName, Msg);
						if (iRet == BOIT_MSGWATCH_BLOCK)
						{
							bPass = FALSE;
						}
					}
					break;
				case BOIT_MW_GROUP:			// ����ָ��Ⱥ��������Ϣ
					if (pList->Callback && pList->GroupID == GroupID)
					{
						iRet = pList->Callback(pList->MsgWatchID, pList->pData, BOIT_MW_EVENT_MESSAGE, GroupID, QQID, SubType, AnonymousName, Msg);
						if (iRet == BOIT_MSGWATCH_BLOCK)
						{
							bPass = FALSE;
						}
					}
					break;
				case BOIT_MW_QQ:			// ����ָ��ĳ���ˣ����������κ���������Ϣ
					if (pList->Callback && pList->QQID == QQID)
					{
						iRet = pList->Callback(pList->MsgWatchID, pList->pData, BOIT_MW_EVENT_MESSAGE, GroupID, QQID, SubType, AnonymousName, Msg);
						if (iRet == BOIT_MSGWATCH_BLOCK)
						{
							bPass = FALSE;
						}
					}
					break;
				case BOIT_MW_GROUP_QQ:		// ����ĳ��Ⱥ��ĳ���˵���Ϣ
					if (pList->Callback && pList->GroupID == GroupID && pList->QQID == QQID)
					{
						iRet = pList->Callback(pList->MsgWatchID, pList->pData, BOIT_MW_EVENT_MESSAGE, GroupID, QQID, SubType, AnonymousName, Msg);
						if (iRet == BOIT_MSGWATCH_BLOCK)
						{
							bPass = FALSE;
						}
					}
					break;
				case BOIT_MW_QQ_PRIVATE:	// ����ĳ���˵�˽����Ϣ
					if (pList->Callback && pList->GroupID == 0 && pList->QQID == QQID)
					{
						iRet = pList->Callback(pList->MsgWatchID, pList->pData, BOIT_MW_EVENT_MESSAGE, GroupID, QQID, SubType, AnonymousName, Msg);
						if (iRet == BOIT_MSGWATCH_BLOCK)
						{
							bPass = FALSE;
						}
					}
					break;
				}

				if (bPass == FALSE)
				{
					__leave;
				}

				pList = Next;
			}
		}
	}
	__finally
	{
		LeaveCriticalSection(&MsgWatchChainLock);

	}
	



	////������ͷ�
	//if (RootMsgWatch)
	//{
	//	for (pBOIT_MSGWATCH pList = RootMsgWatch; pList;)
	//	{
	//		pBOIT_MSGWATCH Next = pList->Next;
	//		if (pList->ToBeFree)
	//		{
	//			RemoveMessageWatch(pList);
	//		}
	//		pList = Next;
	//	}
	//}


	

	if (bPass)
	{
		return BOIT_MSGWATCH_PASS;
	}
	else
	{
		return BOIT_MSGWATCH_BLOCK;
	}
	
}


