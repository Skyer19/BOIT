#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"DirManagement.h"


int RunCode(long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg);


int CmdMsg_runcode_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	HANDLE hSavedFile = PerUserCreateStorageFile(QQID, L"SavedCode.txt", GENERIC_READ | GENERIC_WRITE, 0, OPEN_EXISTING);
	if (hSavedFile == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			SendBackMessage(GroupID, QQID, L"���������Ҳ�������~");
		}
		else
		{
			SendBackMessage(GroupID, QQID, L"���ˣ����ļ���ʱ������ˣ��ѣ�ѣ�");
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
			SendBackMessage(GroupID, QQID, L"������������ļ���ô��ô��");
			SendBackMessage(GroupID, QQID, L"�Ҳ����� �Ұչ���qaq");
			__leave;
		}
		else if (FileSizeLow == 0)
		{
			SendBackMessage(GroupID, QQID, L"��������ļ���ô�ǿյ���");
			__leave;
		}

		FileData = malloc(FileSizeLow + 1);
		ZeroMemory(FileData, FileSizeLow + 1);

		DWORD BytesRead;
		ReadFile(hSavedFile, FileData, FileSizeLow, &BytesRead, 0);
		if (FileSizeLow != BytesRead)
		{
			SendBackMessage(GroupID, QQID, L"��ȡ�ļ���ʱ�������orz");
			__leave;
		}

		
		int cchWcLen = MultiByteToWideChar(54936, 0, FileData, FileSizeLow, 0, 0);
		WideCharStr = malloc(sizeof(WCHAR) * (cchWcLen + 1));
		ZeroMemory(WideCharStr, sizeof(WCHAR) * (cchWcLen + 1));

		MultiByteToWideChar(54936, 0, FileData, FileSizeLow, WideCharStr, cchWcLen);
		WideCharStr[cchWcLen] = 0;

		bSuccess = TRUE;
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
		RunCode(GroupID, QQID, SubType, AnonymousName, WideCharStr);
		free(WideCharStr);
	}
	return 0;
}