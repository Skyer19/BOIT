#pragma once
#include<Windows.h>
#include"SharedMemStruct.h"


typedef struct __tagSession
{
	//session�ļ����
	long long QQID;
	long long GroupID;
	int SubType;
	WCHAR AnonymousName[BOIT_MAX_NICKLEN];
}BOIT_SESSION, * pBOIT_SESSION;