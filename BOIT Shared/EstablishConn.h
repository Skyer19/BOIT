#pragma once
#include<Windows.h>
#include"SharedMemStruct.h"


HANDLE hEventConnect;
HANDLE hEventDeconn;

HANDLE hEventRecvStart;//��ʼ���� ��Ϣ/�¼� ��
HANDLE hEventRecvEnd;//���� ��Ϣ/�¼� ������
HANDLE hEventRecvRet;//���ͽ������ֵ��ȡ���

HANDLE hEventSendStart;//��ʼ���� ��Ϣ/�¼� ��
HANDLE hEventSendEnd;//���� ��Ϣ/�¼� ������
HANDLE hEventSendRet;//���ͽ������ֵ��ȡ���

HANDLE hSharedMemProcess;//Server �� dll ��ؽ�����Ϣ
HANDLE hSharedMemRecv;
HANDLE hSharedMemSend;

HANDLE hOtherSideProcess;

pSHARED_PROCESS_INFO pSharedMemProcess;
pEVENT_RECV pSharedMemRecv;
pEVENT_SEND pSharedMemSend;

SRWLOCK ConnStateLock;
BOOL ConnState; //0��ʾû���ӣ�1��ʾ����


int InitEstablishConn();
int TryEstablishConn();
int GetConnState();
BOOL CleanConn();
BOOL InitConnVar();

BOOL ConnWaitForObject(HANDLE hObject);