#pragma once
#include<Windows.h>
#include"CommandEvent.h"
#include"SessionManage.h"

typedef UINT_PTR PARAMA;
typedef UINT_PTR PARAMB;

//ָ���������ݽṹ����

#define COMMAND_MAX_ALIAS 4 // �������


typedef struct __tagCommand BOIT_COMMAND, * pBOIT_COMMAND;


typedef INT(*MSGPROC)(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg);//�ص���������
typedef INT(*EVENTPROC)(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB);//�ص���������


typedef struct __tagCommand
{
	unsigned int CommandID;

	WCHAR* CommandName[COMMAND_MAX_ALIAS];//ָ������
	int AliasCount;//���ƣ�����˵�Ǳ���������
	MSGPROC MessageProc;//������Ϣר�õĻص�����
	EVENTPROC EventProc;//��������¼��Ļص�����
	WCHAR* ManualMsg;
	int MatchMode;

	pBOIT_COMMAND NextCommand;
}BOIT_COMMAND,*pBOIT_COMMAND;

pBOIT_COMMAND RootCommand;

SRWLOCK CommandChainLock;//����ָ�����õ���

int CommandAllocID;

#define BOIT_MATCH_FULL 1 //ȫ��ƥ�䣬��Сд������
#define BOIT_MATCH_FULL_CASE 2 //ȫ��ƥ�䣬��Сд����
#define BOIT_MATCH_PARAM 3 //����һ������ƥ�䣬��Сд������
#define BOIT_MATCH_PARAM_CASE 4 //����һ������ƥ�䣬��Сд����
#define BOIT_MATCH_HEAD 5 //��ǰN���ַ�ƥ�䣬��Сд������
#define BOIT_MATCH_HEAD_CASE 6 //��ǰN���ַ�ƥ�䣬��Сд����




int InitializeCommandManager();

int FinalizeCommandManager();

pBOIT_COMMAND RegisterCommandEx(WCHAR* CommandName, MSGPROC MessageProc, EVENTPROC EventProc, WCHAR* ManualMsg, int MatchMode);

pBOIT_COMMAND RegisterCommand(WCHAR* CommandName, MSGPROC MessageProc, WCHAR* ManualMsg, int MatchMode);

int RemoveCommand(pBOIT_COMMAND Command);

int FreeCommand(pBOIT_COMMAND Command);

int BroadcastCommandEvent(UINT Event, PARAMA ParamA, PARAMB ParamB);

int SendCommandEvent(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB);

int AddCommandAlias(pBOIT_COMMAND Command, WCHAR* AliasName);

BOOL CheckIsCommand(WCHAR* Msg, int* PrefixLen);

int GetCmdParamLen(WCHAR* String);

int GetCmdParamWithEscapeLen(WCHAR* String);

int CmdParamUnescape(WCHAR* String, WCHAR* UnescapeStr);

int GetCmdSpaceLen(WCHAR* String);

int CommandHandler(pBOIT_SESSION boitSession, WCHAR* Msg);



int RegisterInlineCommand();//ע�������Դ�ָ��