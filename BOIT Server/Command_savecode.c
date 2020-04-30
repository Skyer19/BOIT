#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"DirManagement.h"
#include"EncodeConvert.h"

int CmdMsg_savecode_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);
	
	if (ParamLen + SpaceLen == wcslen(Msg))
	{
		SendBackMessage(boitSession, L"������Ĵ����أ�qwq");
		return 0;
	}

	Msg += ParamLen + SpaceLen;
	//д���ļ�
	HANDLE hSavedFile = PerUserCreateStorageFile(GetBOITSessionQQID(boitSession), L"SavedCode.txt", GENERIC_READ | GENERIC_WRITE, 0, CREATE_ALWAYS);
	if (hSavedFile == INVALID_HANDLE_VALUE)
	{
		SendBackMessage(boitSession, L"д���ļ���ʱ�������qaq");
		return 0;
	}

	char* UTF8Text = 0;
	BOOL bSuccessSave = FALSE;
	__try
	{
		//Ϊ�˴򿪶�ȡ���㣬����ת���CP_GB18030
		int cbUTF8Len;
		UTF8Text = StrConvWC2MB(CP_GB18030, Msg, -1, &cbUTF8Len);

		if (!UTF8Text)
		{
			__leave;
		}
		
		DWORD BytesWritten;
		WriteFile(hSavedFile, UTF8Text, cbUTF8Len, &BytesWritten, 0);
		if (BytesWritten != cbUTF8Len)
		{
			__leave;
		}
		bSuccessSave = TRUE;
	}
	__finally
	{
		if (UTF8Text) free(UTF8Text);
		CloseHandle(hSavedFile);
	}

	if (bSuccessSave == TRUE)
	{
		SendBackMessage(boitSession, L"�������ɹ���");
		SendBackMessage(boitSession, L"�ǳ���Ǹbot���ڿ��������У����Ĵ��벻һ���ᱻʼ�ձ��棡");
	}
	else
	{
		SendBackMessage(boitSession, L"���ˣ�д���ļ���ʱ��ʧ���ˣ�");
	}
	return 0;
}