#include<Windows.h>

#define CP_GB18030 54936

WCHAR* StrConvMB2WC(UINT CodePage, LPCCH MbStr, int cbMbStrlen, int* cchWcLen); //ת���ַ�����Ĭ�ϲ�һ�� \0�ڽ�β�����ȴ���-1���Զ����㳤�ȣ��ڴ���Ҫ�ֶ�free

char* StrConvWC2MB(UINT CodePage, LPCWCH WcStr, int cchWcStrlen, int* cbMbLen); //ת���ַ�����Ĭ�ϲ�һ�� \0�ڽ�β�����ȴ���-1���Զ����㳤�ȣ��ڴ���Ҫ�ֶ�free
