#include<Windows.h>

//������Ϣ����
//MW �� MessageWatch ����д
#define BOIT_MW_ALL 1 // �����κ��ˣ��κ�;������Ϣ
#define BOIT_MW_GROUP 2 // ����ָ��Ⱥ��������Ϣ
#define BOIT_MW_QQ 3 // ����ָ��ĳ���ˣ����������κ���������Ϣ
#define BOIT_MW_GROUP_QQ 4 // ����ĳ��Ⱥ��ĳ���˵���Ϣ
#define BOIT_MW_QQ_PRIVATE 5 // ����ĳ���˵�˽����Ϣ


typedef INT(*MSGWATCH_CALLBACK)(int MsgWatchID, PBYTE pData, UINT Event);


typedef struct __tagMessageWatch BOIT_MSGWATCH, * pBOIT_MSGWATCH;

typedef struct __tagMessageWatch
{
	int WatchType;
	long long MsgWatchID;
	long long GroupID;
	long long QQID;
	HANDLE hTimer;
	MSGWATCH_CALLBACK Callback;
	PBYTE pData;
	pBOIT_MSGWATCH Last;
	pBOIT_MSGWATCH Next;
}BOIT_MSGWATCH,* pBOIT_MSGWATCH;


SRWLOCK MsgWatchChainLock;

pBOIT_MSGWATCH RootMsgWatch;


int InitializeMessageWatch();

int FinalizeMessageWatch();

int RegisterMessageWatch(int WatchType,
	long long TimeOutInterval,
	long long GroupID,
	long long QQID,
	int SubType,
	WCHAR* AnonymousName,
	MSGWATCH_CALLBACK CallbackFunc,
	PBYTE pData);

int RemoveMessageWatch(pBOIT_MSGWATCH MsgWatch);

