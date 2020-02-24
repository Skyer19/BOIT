#include<Windows.h>
#include"DirManagement.h"

BOOL CheckUserToken(long long QQID, WCHAR* TokenStr)
{
	if (PerUserCreateDirIfNExist(QQID, L"Token"))
	{
		//��ʼ��Ĭ��Ȩ��
		PerUserCreateFileIfNExist(QQID,L"Token\\PrivilegeRunCode");
	}

	WCHAR TokenFile[MAX_PATH];
	GetPerUserDir(TokenFile, QQID);
	PathAppendW(TokenFile, L"Token");
	PathAppendW(TokenFile, TokenStr);
	if (PathFileExistsW(TokenFile))
	{
		return TRUE;
	}

	return FALSE;
}



BOOL CheckGroupToken(long long GroupID, WCHAR* TokenStr)
{
	if (PerGroupCreateDirIfNExist(GroupID, L"Token"))
	{
		//��ʼ��Ĭ��Ȩ��
		PerGroupCreateFileIfNExist(GroupID, L"Token\\PrivilegeRunCode");
	}

	WCHAR TokenFile[MAX_PATH];
	GetPerGroupDir(TokenFile, GroupID);
	PathAppendW(TokenFile, L"Token");
	PathAppendW(TokenFile, TokenStr);
	if (PathFileExistsW(TokenFile))
	{
		return TRUE;
	}

	return FALSE;
}

