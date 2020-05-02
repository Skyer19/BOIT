#include<Windows.h>
#include"VBuffer.h"
#include"APITransfer.h"
#include"SessionManage.h"
#include"HandleBOITCode.h"


WCHAR* BOITCodeStart[] = L"[";
WCHAR* BOITCodeEnd[] = L"]";
WCHAR* BOITCodeID[] = L"BOIT";


pBOITCODEINFO GetBOITCode(WCHAR* Msg, int* Len)
{
	//TODO: ����һ���ַ������жϿ�ͷ�ǲ���һ��boit�룬�Ǿͷ��������Ϣ���ṩһ�����������ͷţ����򷵻�0

	int Offset = 0;
	if (_wcsnicmp(Msg + Offset, BOITCodeStart, wcslen(BOITCodeStart)))
	{
		return 0;
	}
	Offset += wcslen(BOITCodeStart);

	if (_wcsnicmp(Msg + Offset, BOITCodeID, wcslen(BOITCodeID)))
	{
		return 0;
	}
	Offset += wcslen(BOITCodeID);

	if (Msg[Offset] != L':')
	{
		return 0;
	}
	Offset++;



	return 0;
}


BOOL SendTextWithBOITCode(pBOIT_SESSION boitSession, WCHAR* Msg)
{
	//��Handleһ��
	pVBUF SendTextBuffer;
	SendTextBuffer = AllocVBuf();

	int i = 0, j = 0;
	int OrgLen = wcslen(Msg);

	int BOITFlushMaxUse = 2;
	for (i = 0; i < OrgLen;)
	{
		if (_wcsnicmp(Msg + i, L"[BOIT:flush]", wcslen(L"[BOIT:flush]")) == 0)
		{
			if (BOITFlushMaxUse-- > 0)
			{
				AdjustVBuf(SendTextBuffer, sizeof(WCHAR) * (j + 1));
				((WCHAR*)(SendTextBuffer->Data))[j] = 0;

				j = 0;
				SendBackMessage(boitSession, (WCHAR*)SendTextBuffer->Data);
			}

			i += wcslen(L"[BOIT:flush]");
		}
		else
		{
			AdjustVBuf(SendTextBuffer, sizeof(WCHAR) * (j + 1));
			((WCHAR*)(SendTextBuffer->Data))[j] = Msg[i];
			i++;
			j++;
		}
	}

	if (j)
	{
		AdjustVBuf(SendTextBuffer, sizeof(WCHAR) * (j + 1));
		((WCHAR*)(SendTextBuffer->Data))[j] = 0;
		SendBackMessage(boitSession, (WCHAR*)SendTextBuffer->Data);
	}

	FreeVBuf(SendTextBuffer);
	return 0;
}