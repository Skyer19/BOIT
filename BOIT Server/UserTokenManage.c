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
		//TODO: У����Ȩ��
		return TRUE;
	}

	return FALSE;
}

