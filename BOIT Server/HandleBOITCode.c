#include<Windows.h>
#include<wchar.h>
#include<WinInet.h>
#include"VBuffer.h"
#include"APITransfer.h"
#include"SessionManage.h"
#include"HandleBOITCode.h"
#include"CommandProcess.h"
#include"DirManagement.h"

WCHAR* BOITCodeStart = L"[";
WCHAR* BOITCodeEnd = L"]";
WCHAR* BOITCodeID = L"BOIT";
//TODO: ���� ']' �ַ���Ӳ���뵽�� CommandProcess.c ��� GetBOITCodeParamWithEscapeLen �� BOITCodeParamUnescape ������
//Ӧ�øĽ�


pBOITCODEINFO GetBOITCode(WCHAR* Msg, int* Len)
{
	//TODO: ����һ���ַ������жϿ�ͷ�ǲ���һ��boit�룬�Ǿͷ��������Ϣ���ṩһ�����������ͷţ����򷵻�0

	pBOITCODEINFO BOITCodeInfo = 0;
	BOOL bSuccess = FALSE;
	int Offset = 0;


	__try
	{
		if (_wcsnicmp(Msg + Offset, BOITCodeStart, wcslen(BOITCodeStart)))
		{
			__leave;
		}
		Offset += wcslen(BOITCodeStart);
		Offset += GetLineSpaceLen(Msg + Offset);

		if (_wcsnicmp(Msg + Offset, BOITCodeID, wcslen(BOITCodeID)))
		{
			__leave;
		}
		Offset += wcslen(BOITCodeID);
		Offset += GetLineSpaceLen(Msg + Offset);

		if (Msg[Offset] != L':')
		{
			__leave;
		}
		Offset++;
		Offset += GetLineSpaceLen(Msg + Offset);

		//���Խ���BOIT����


		int BOITCodeTypeLen;
		for (BOITCodeTypeLen = 0; BOITCodeTypeLen < 64; BOITCodeTypeLen++)
		{
			if (!((L'A' <= Msg[Offset + BOITCodeTypeLen] && Msg[Offset + BOITCodeTypeLen] <= L'Z') ||
				(L'a' <= Msg[Offset + BOITCodeTypeLen] && Msg[Offset + BOITCodeTypeLen] <= L'z') ||
				(L'0' <= Msg[Offset + BOITCodeTypeLen] && Msg[Offset + BOITCodeTypeLen] <= L'9')
				))
			{
				break;
			}
		}

		if (BOITCodeTypeLen == 64)
		{
			//smg?
			__leave;
		}

		BOITCodeInfo = malloc(sizeof(BOITCODEINFO));
		if (!BOITCodeInfo)
		{
			__leave;
		}
		ZeroMemory(BOITCodeInfo, sizeof(BOITCODEINFO));



		BOITCodeInfo->TypeStr = malloc((BOITCodeTypeLen + 1) * sizeof(WCHAR));
		wcsncpy_s(BOITCodeInfo->TypeStr, BOITCodeTypeLen + 1, Msg + Offset, BOITCodeTypeLen);
		BOITCodeInfo->TypeStr[BOITCodeTypeLen] = 0;


		Offset += BOITCodeTypeLen;
		Offset += GetLineSpaceLen(Msg + Offset);

		//ѭ�����������ֶΡ�
		//Ѱ�Ҷ��Ž����ֶ�����Ȼ��Ѱ�ҵȺţ����������ת�������ֱ���Ҳ����������� ```BOITCodeEnd```

		for (;;)
		{
			if (Msg[Offset] == L',')
			{
				//��һ���ֶ�
				int ParamIndex = BOITCodeInfo->ParamNum;

				BOITCodeInfo->ParamNum++;

				if (BOITCODE_MAX_PARAM == BOITCodeInfo->ParamNum)
				{
					//boom!!!
					__leave;
				}

				Offset++;
				Offset += GetLineSpaceLen(Msg + Offset);

				int BOITCodeFieldLen;
				for (BOITCodeFieldLen = 0; BOITCodeFieldLen < 64; BOITCodeFieldLen++)
				{
					if (!((L'A' <= Msg[Offset + BOITCodeFieldLen] && Msg[Offset + BOITCodeFieldLen] <= L'Z') ||
						(L'a' <= Msg[Offset + BOITCodeFieldLen] && Msg[Offset + BOITCodeFieldLen] <= L'z') ||
						(L'0' <= Msg[Offset + BOITCodeFieldLen] && Msg[Offset + BOITCodeFieldLen] <= L'9')
						))
					{
						break;
					}
				}

				if (BOITCodeFieldLen == 64)
				{
					//smg?
					__leave;
				}


				BOITCodeInfo->Key[ParamIndex] = malloc((BOITCodeFieldLen + 1) * sizeof(WCHAR));
				wcsncpy_s(BOITCodeInfo->Key[ParamIndex], BOITCodeFieldLen + 1, Msg + Offset, BOITCodeFieldLen);
				BOITCodeInfo->Key[ParamIndex][BOITCodeFieldLen] = 0;

				Offset += BOITCodeFieldLen;
				Offset += GetLineSpaceLen(Msg + Offset);

				if (Msg[Offset] != L'=')
				{
					__leave;
				}
				Offset++;
				Offset += GetLineSpaceLen(Msg + Offset);

				int ParamLen = GetBOITCodeParamWithEscapeLen(Msg + Offset);

				BOITCodeInfo->Value[ParamIndex] = malloc((ParamLen + 1) * sizeof(WCHAR));
				ZeroMemory(BOITCodeInfo->Value[ParamIndex], (ParamLen + 1) * sizeof(WCHAR));
				BOITCodeParamUnescape(Msg + Offset, BOITCodeInfo->Value[ParamIndex]);
				Offset += ParamLen;
				Offset += GetLineSpaceLen(Msg + Offset);

			}
			else if (_wcsnicmp(Msg + Offset, BOITCodeEnd, wcslen(BOITCodeEnd)) == 0)
			{
				//��������
				Offset += wcslen(BOITCodeEnd);
				break;
			}
			else
			{
				__leave; //�Ȳ���һ���ֶΣ�Ҳ���ǽ������㵽����˭������
			}
		}


		bSuccess = TRUE;
	}
	__finally
	{
		if (!bSuccess)
		{
			FreeBOITCode(BOITCodeInfo);
			return 0;
		}
		else
		{
			if (Len)
			{
				(*Len) = Offset;
			}
			return BOITCodeInfo;
		}
	}
}


BOOL FreeBOITCode(pBOITCODEINFO BOITCodeInfo)
{
	if (BOITCodeInfo)
	{
		if (BOITCodeInfo->TypeStr)
		{
			free(BOITCodeInfo->TypeStr);
		}

		for (int i = 0; i < BOITCodeInfo->ParamNum; i++)
		{
			if (BOITCodeInfo->Key[i])
			{
				free(BOITCodeInfo->Key[i]);
			}
			if (BOITCodeInfo->Value[i])
			{
				free(BOITCodeInfo->Value[i]);
			}
		}

		free(BOITCodeInfo);
	}
	return TRUE;
}

int DownloadUrlAsFile(WCHAR URL[], WCHAR FilePath[], DWORD MaxSize)
{
	HINTERNET hInternet = 0;
	HINTERNET hUrl = 0;
	BOOL bSuccess = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	__try
	{
		hInternet = InternetOpenW(L"BOIT", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (!hInternet)
		{
			__leave;
		}
		hUrl = InternetOpenUrlW(hInternet, URL, 0, 0, 0, 0);
		if (!hUrl)
		{
			__leave;
		}
		hFile = CreateFileW(FilePath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			__leave;
		}

		DWORD TotDownload = 0;
		BYTE Buffer[1024];
		DWORD BytesRead;
		DWORD BytesWritten;
		for (;;)
		{
			BOOL bResult = InternetReadFile(hUrl, Buffer, sizeof(Buffer), &BytesRead);
			if (bResult == FALSE)
			{
				// failed
				__leave;
			}
			// bResult = TRUE
			if (BytesRead == 0)
			{
				break;
			}
			// BytesRead > 0
			TotDownload += BytesRead;
			if (MaxSize && TotDownload > MaxSize) // �д�С���ƣ����ҳ�����
			{
				__leave;
			}
			bResult = WriteFile(hFile, Buffer, BytesRead, &BytesWritten, 0);
			if ((!bResult) || (BytesRead != BytesWritten))
			{
				//д��ʧ�ܣ�
				__leave;
			}
		}

		bSuccess = TRUE;
	}
	__finally
	{
		if (hUrl)
		{
			InternetCloseHandle(hUrl);
		}
		if (hInternet)
		{
			InternetCloseHandle(hInternet);
		}
		if (hFile)
		{
			CloseHandle(hFile);
			if (!bSuccess)
			{
				DeleteFileW(FilePath);
			}
		}
	}

	return bSuccess;
}

BOOL SendTextWithBOITCode(pBOIT_SESSION boitSession, WCHAR* Msg, DWORD flags)
{
	//��Handleһ��
	pVBUF SendTextBuffer;
	SendTextBuffer = AllocVBuf();

	int i = 0, j = 0;
	int OrgLen = wcslen(Msg);

	int BOITFlushMaxUse = flags >> SWBCFLAG_MAX_FLUSH_OFFSET;

	for (i = 0; i < OrgLen;)
	{
		int BOITCodeLen = 0;
		pBOITCODEINFO BOITCodeInfo = GetBOITCode(Msg + i, &BOITCodeLen);

		if (BOITCodeInfo)
		{
			BOOL bBOITCodeRecognize = FALSE;

			if (_wcsnicmp(BOITCodeInfo->TypeStr, L"flush", wcslen(L"flush")) == 0)
			{
				if (BOITFlushMaxUse-- > 0)
				{
					AdjustVBuf(SendTextBuffer, sizeof(WCHAR) * (j + 1));
					((WCHAR*)(SendTextBuffer->Data))[j] = 0;

					j = 0;
					SendBackMessage(boitSession, (WCHAR*)SendTextBuffer->Data);
				}
				bBOITCodeRecognize = TRUE;
			}

			if (_wcsnicmp(BOITCodeInfo->TypeStr, L"at", wcslen(L"at")) == 0)
			{
				for (int i = 0; i < BOITCodeInfo->ParamNum; i++)
				{
					if (_wcsnicmp(BOITCodeInfo->Key[i], L"qq", wcslen(L"qq")) == 0)
					{
						WCHAR BufferStr[32] = { 0 };

						if (_wcsnicmp(BOITCodeInfo->Value[i], L"all", wcslen(L"all")) == 0)
						{
							if (flags & SWBC_PARSE_AT_ALL)
							{
								bBOITCodeRecognize = TRUE;
								wcscpy_s(BufferStr, _countof(BufferStr), L"[CQ:at,qq=all]");
							}
						}
						else
						{
							if (flags & SWBC_PARSE_AT)
							{
								bBOITCodeRecognize = TRUE;
								long long x = _wcstoi64(BOITCodeInfo->Value[i], 0, 0);
								swprintf_s(BufferStr, _countof(BufferStr), L"[CQ:at,qq=%lld]", x);
							}
						}
						VBufferAppendStringW(SendTextBuffer, BufferStr);
						j += wcslen(BufferStr);
					}
				}
			}

			if (_wcsnicmp(BOITCodeInfo->TypeStr, L"img", wcslen(L"img")) == 0)
			{
				if (BOITCodeInfo->ParamNum == 1)
				{
					//Ŀǰֻ����һ������

					WCHAR PicFileName[MAX_PATH];
					WCHAR PicFilePath[MAX_PATH];


					if (_wcsnicmp(BOITCodeInfo->Key[0], L"url", wcslen(L"url")) == 0)
					{
						if (flags & SWBC_PARSE_IMG_URL)
						{
							bBOITCodeRecognize = TRUE;
							//������ȡͼƬ
							
							CoolQAllocPicFileName(&PicFileName);

							wcscpy_s(PicFilePath, MAX_PATH, GetCQImageDir());
							PathAppendW(PicFilePath, PicFileName);

							int bPicDownloadSuccess = DownloadUrlAsFile(BOITCodeInfo->Value[0], PicFilePath, 512 * 1024);


							WCHAR BufferStr[64] = { 0 };

							if (!bPicDownloadSuccess)
							{
								wcscpy_s(BufferStr, _countof(BufferStr), L"[ͼƬץȡʧ��]");
							}
							else
							{
								swprintf_s(BufferStr, _countof(BufferStr), L"[CQ:image,file=%ls]", PicFileName);
							}

							VBufferAppendStringW(SendTextBuffer, BufferStr);
							j += wcslen(BufferStr);
						}
					}
					else if (_wcsnicmp(BOITCodeInfo->Key[0], L"file", wcslen(L"file")) == 0)
					{
						if (flags & SWBC_PARSE_IMG_FILE)
						{
							bBOITCodeRecognize = TRUE;

							CoolQAllocPicFileName(&PicFileName);

							wcscpy_s(PicFilePath, MAX_PATH, GetCQImageDir());
							PathAppendW(PicFilePath, PicFileName);
							//copy this file
							BOOL bCopyFileSuccess = CopyFileW(BOITCodeInfo->Value[0], PicFilePath, FALSE);

							WCHAR BufferStr[64] = { 0 };

							if (!bCopyFileSuccess)
							{
								wcscpy_s(BufferStr, _countof(BufferStr), L"[����ͼƬ�ļ�����]");
							}
							else
							{
								swprintf_s(BufferStr, _countof(BufferStr), L"[CQ:image,file=%ls]", PicFileName);
							}

							VBufferAppendStringW(SendTextBuffer, BufferStr);
							j += wcslen(BufferStr);
						}
					}
				}
			}
			FreeBOITCode(BOITCodeInfo);

			if (bBOITCodeRecognize)
			{
				i += BOITCodeLen;
				continue;
			}
		}

		{
			AdjustVBuf(SendTextBuffer, sizeof(WCHAR) * (j + 1));
			((WCHAR*)(SendTextBuffer->Data))[j] = Msg[i];
			i++;
			j++;
		}

		/*if (_wcsnicmp(Msg + i, L"[BOIT:flush]", wcslen(L"[BOIT:flush]")) == 0)
		{


			i += wcslen(L"[BOIT:flush]");
		}
		else
		{

		}*/
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