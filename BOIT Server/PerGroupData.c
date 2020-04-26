#include<Windows.h>
#include"DirManagement.h"
#include<wchar.h>

int CheckPerGroupDataExist(long long GroupID)
{
	WCHAR PerGroupDir[MAX_PATH];
	GetPerGroupDir(PerGroupDir, GroupID);

	if (PathFileExistsW(PerGroupDir) == 0)
	{
		//�����û�Ŀ¼
		CreatePerGroupData(PerGroupDir);
	}
	return 0;
}

int CreatePerGroupData(WCHAR* Path)
{
	CreateDirectoryW(Path, 0);//�����ļ���

	return 0;
}


int GetPerGroupDir(WCHAR* Buffer, long long GroupID)
{
	wcscpy_s(Buffer, MAX_PATH, GetBOITGroupDataDir());
	WCHAR GroupIDStr[16] = { 0 };
	swprintf_s(GroupIDStr, 16, L"%lld\\", GroupID);
	PathAppendW(Buffer, GroupIDStr);
	return 0;
}


BOOL PerGroupCreateDirIfNExist(long long GroupID, WCHAR* FolderName) //���� TRUE ˵�������ˣ����� FALSE ����������
{
	WCHAR DirPath[MAX_PATH];
	GetPerGroupDir(DirPath, GroupID);
	PathAppendW(DirPath, FolderName);
	if (PathIsDirectoryW(DirPath) == 0)
	{
		CreateDirectoryW(DirPath, 0);
		return TRUE;
	}
	return FALSE;
}


BOOL PerGroupCreateFileIfNExist(long long GroupID, WCHAR* FileName) //���� TRUE ˵�������ˣ����� FALSE ����������
{
	WCHAR FilePath[MAX_PATH];
	GetPerGroupDir(FilePath, GroupID);
	PathAppendW(FilePath, FileName);
	if (PathFileExistsW(FilePath) == 0)
	{
		HANDLE hFile = CreateFile(FilePath, GENERIC_READ, 0, 0, CREATE_NEW, 0, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}



BOOL PerGroupDeleteFile(long long GroupID, WCHAR* FileName) //���� TRUE ˵�������ˣ����� FALSE ����������
{
	WCHAR FilePath[MAX_PATH];
	GetPerGroupDir(FilePath, GroupID);
	PathAppendW(FilePath, FileName);

	return DeleteFileW(FilePath);
}
