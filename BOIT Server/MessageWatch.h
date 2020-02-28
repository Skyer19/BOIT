#include<Windows.h>
#include"SessionManage.h"

//������Ϣ����
//MW �� MessageWatch ����д
#define BOIT_MW_ALL 1 // �����κ��ˣ��κ�;������Ϣ
#define BOIT_MW_GROUP 2 // ����ָ��Ⱥ��������Ϣ
#define BOIT_MW_QQ 3 // ����ָ��ĳ���ˣ����������κ���������Ϣ
#define BOIT_MW_GROUP_QQ 4 // ����ĳ��Ⱥ��ĳ���˵���Ϣ
#define BOIT_MW_QQ_PRIVATE 5 // ����ĳ���˵�˽����Ϣ



#define BOIT_MW_EVENT_TIMEOUT 1 // �ȴ���ʱ
#define BOIT_MW_EVENT_MESSAGE 2



#define BOIT_MSGWATCH_PASS 0	// ������Ϣ
#define BOIT_MSGWATCH_BLOCK 1	// �ض���Ϣ

typedef INT(*MSGWATCH_CALLBACK)(long long MsgWatchID, PBYTE pData, UINT Event,
	long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);


typedef struct __tagMessageWatch BOIT_MSGWATCH, * pBOIT_MSGWATCH;

typedef struct __tagMessageWatch
{
	int WatchType;
	long long MsgWatchID;
	long long GroupID;
	long long QQID;
	BOOL ToBeFree;
	HANDLE hTimer;
	MSGWATCH_CALLBACK Callback;
	PBYTE pData;
	pBOIT_MSGWATCH Last;
	pBOIT_MSGWATCH Next;
}BOIT_MSGWATCH,* pBOIT_MSGWATCH;


CRITICAL_SECTION MsgWatchChainLock;

pBOIT_MSGWATCH RootMsgWatch;


int InitializeMessageWatch();

int FinalizeMessageWatch();

int RegisterMessageWatch(int WatchType,
	long long TimeOutInterval,
	pBOIT_SESSION boitSession,
	MSGWATCH_CALLBACK CallbackFunc,
	PBYTE pData);

int RemoveMessageWatch(pBOIT_MSGWATCH MsgWatch);

int RemoveMessageWatchByID(long long MsgWatchAllocID);

//int MarkFreeMessageWatchByID(long long MsgWatchAllocID);


int MessageWatchFilter(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);