#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"DirManagement.h"
#include"EncodeConvert.h"
#include<wchar.h>
#include"Corpus.h"
int RunCode(pBOIT_SESSION orgboitSession, WCHAR* Msg);


int CmdMsg_runcode_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	long long ToRunQQID = boitSession->QQID;
	if (SpaceLen != 0)
	{
		int QQIDLen = GetCmdParamLen(Msg + ParamLen + SpaceLen);
		if (QQIDLen)
		{
			long long ParamQQID;
			int iMatch = swscanf_s(Msg + ParamLen + SpaceLen, L"%lld", &ParamQQID);
			if (iMatch == 1 && ParamQQID)
			{
				ToRunQQID = ParamQQID;
			}
		}
	}

	HANDLE hSavedFile = PerUserCreateStorageFile(ToRunQQID, L"SavedCode.txt", GENERIC_READ | GENERIC_WRITE, 0, OPEN_EXISTING);
	if (hSavedFile == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)
		{
			SendBackMessage(boitSession, Corpus_CodeNotFound());
		}
		else
		{
			SendBackMessage(boitSession, L"���ˣ����ļ���ʱ������ˣ��ѣ�ѣ�");
		}
		return 0;
	}
	

	//����Ҫ����С�ġ�����ļ��������û�д�����
	CHAR* FileData = 0;
	WCHAR* WideCharStr = 0;

	BOOL bSuccess = 0;
	__try
	{
		DWORD FileSizeHigh;
		DWORD FileSizeLow = GetFileSize(hSavedFile, &FileSizeHigh);

		if (FileSizeHigh || FileSizeLow >= 65536)
		{
			SendBackMessage(boitSession, L"������������ļ���ô��ô��");
			SendBackMessage(boitSession, L"�Ҳ����� �Ұչ���qaq");
			__leave;
		}
		else if (FileSizeLow == 0)
		{
			SendBackMessage(boitSession, L"��������ļ���ô�ǿյ���");
			__leave;
		}

		FileData = malloc(FileSizeLow + 1);
		if (!FileData)
		{
			__leave;
		}
		ZeroMemory(FileData, (unsigned long long)FileSizeLow + 1);

		DWORD BytesRead;
		if (!ReadFile(hSavedFile, FileData, FileSizeLow, &BytesRead, 0))
		{
			__leave;
		}

		if (FileSizeLow != BytesRead)
		{
			SendBackMessage(boitSession, L"��ȡ�ļ���ʱ�������orz");
			__leave;
		}

		int cchWcLen;
		WideCharStr = StrConvMB2WC(CP_GB18030, FileData, FileSizeLow, &cchWcLen);

		
		if (WideCharStr)
		{
			bSuccess = TRUE;
		}
	}
	__finally
	{
		if (bSuccess == FALSE)
		{
			if (WideCharStr)free(WideCharStr);
		}
		if (FileData)free(FileData);
		CloseHandle(hSavedFile);
	}

	if (bSuccess)
	{
		RunCode(boitSession, WideCharStr);
		free(WideCharStr);
	}
	return 0;
}