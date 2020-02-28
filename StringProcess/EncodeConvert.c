#include<Windows.h>

WCHAR * StrConvMB2WC(UINT CodePage,LPCCH MbStr,int cbMbStrlen, int *cchWcLen) //ת���ַ�����Ĭ�ϲ�һ�� \0�ڽ�β�����ȴ���-1���Զ����㳤�ȣ��ڴ���Ҫ�ֶ�free
{
	if (cbMbStrlen == -1)
	{
		cbMbStrlen = strlen(MbStr);
	}
	int _cchWcLen = MultiByteToWideChar(CodePage, 0, MbStr, cbMbStrlen, 0, 0);
	
	WCHAR * wcStr = malloc(sizeof(WCHAR) * (_cchWcLen + 1));
	if (!wcStr)
	{
		if (cchWcLen)
		{
			(*cchWcLen) = 0;
		}
		return 0;
	}
	MultiByteToWideChar(CodePage, 0, MbStr, cbMbStrlen, wcStr, _cchWcLen + 1);
	wcStr[_cchWcLen] = 0;
	if (cchWcLen)
	{
		(*cchWcLen) = _cchWcLen;
	}
	return wcStr;
}


char* StrConvWC2MB(UINT CodePage, LPCWCH WcStr, int cchWcStrlen, int* cbMbLen) //ת���ַ�����Ĭ�ϲ�һ�� \0�ڽ�β�����ȴ���-1���Զ����㳤�ȣ��ڴ���Ҫ�ֶ�free
{
	if (cchWcStrlen == -1)
	{
		cchWcStrlen = strlen(WcStr);
	}
	int _cbMbLen = WideCharToMultiByte(CodePage, 0, WcStr, cchWcStrlen, 0, 0, 0, 0);
	char* mbStr = malloc(_cbMbLen + 1);
	if (!mbStr)
	{
		if (cbMbLen)
		{
			(*cbMbLen) = 0;
		}
		return 0;
	}
	WideCharToMultiByte(CodePage, 0, WcStr, cchWcStrlen, mbStr, _cbMbLen + 1, 0, 0);
	mbStr[_cbMbLen] = 0;
	if (cbMbLen)
	{
		(*cbMbLen) = _cbMbLen;
	}
	return mbStr;
}