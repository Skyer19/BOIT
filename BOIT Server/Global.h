#pragma once
#include<Windows.h>

SRWLOCK SendLock;

HANDLE hEventServerStop;

BOOL bBOITRemove; //�Ƿ�ж�س���

int InitServerState();
int ServerStop();

int StartSendEventHandler();

int InitSendEventDispatch();

