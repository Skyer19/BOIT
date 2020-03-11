#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include<WinInet.h>
#include"VBuffer.h"
#include"cJSON.h"
#include<strsafe.h>
#include"EncodeConvert.h"
#include<shlwapi.h>
#include"DirManagement.h"
#include"URIEncode.h"
#pragma comment(lib,"WinINet.lib")


#define OIER_QRY_BUFSZ 4096

#define OIER_MAX_DISPLAY 3
typedef struct __tagQueryOIerStruct
{
	pBOIT_SESSION boitSession;
	BYTE ReadBuffer[OIER_QRY_BUFSZ];
	DWORD BytesRead;

	HINTERNET hRequest;
	pVBUF vBuffer;
	BOOL bRequestComplete;
}QUERY_OIER_STRUCT, * pQUERY_OIER_STRUCT;

WCHAR OIerDBServerName[] = L"bytew.net";

static HINTERNET hInet;
static HINTERNET hConnect;

VOID CALLBACK QueryOIerCallback(
	_In_ HINTERNET hInternet,
	_In_opt_ DWORD_PTR dwContext,
	_In_ DWORD dwInternetStatus,
	_In_opt_ LPVOID lpvStatusInformation,
	_In_ DWORD dwStatusInformationLength
);


int CmdMsg_oier_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	if (boitSession->GroupID && CheckGroupToken(boitSession->GroupID, L"PrivilegeQueryOIer") == 0)
	{
		SendBackMessage(boitSession, L"��Ⱥ��ֹ�˲�ѯOIer���ܡ�����˽���в�ѯ��");
		return 0;
	}



	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	int QueryStrlen = wcslen(Msg + ParamLen + SpaceLen);
	if (QueryStrlen >= 64)
	{
		SendBackMessage(boitSession, L"�����ĸ�OIer����������ô����");
	}
	else if (QueryStrlen == 0)
	{
		SendBackMessage(boitSession, L"��Ҫ��ѯ�����ĸ�OIer����");
	}
	else
	{
		QueryOIerInfo(boitSession, Msg + ParamLen + SpaceLen);
	}

	return 0;
}

int CmdEvent_oier_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_CMDLOAD:
		hInet = InternetOpenW(L"BOIT", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
		INTERNET_STATUS_CALLBACK pOldStatusCallback = InternetSetStatusCallbackW(hInet, QueryOIerCallback);

		// For HTTP InternetConnect returns synchronously because it does not
		// actually make the connection.

		hConnect = InternetConnectW(hInet,
			OIerDBServerName, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		break;


	case EC_CMDFREE:
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInet);
		break;
	}
	return 0;
}


pQUERY_OIER_STRUCT AllocQueryOIerStruct(pBOIT_SESSION boitSession)
{
	pQUERY_OIER_STRUCT QueryStruct = malloc(sizeof(QUERY_OIER_STRUCT));
	ZeroMemory(QueryStruct, sizeof(QUERY_OIER_STRUCT));

	QueryStruct->boitSession = DuplicateBOITSession(boitSession);

	QueryStruct->vBuffer = AllocVBuf();

	return QueryStruct;
}


BOOL FreeQueryOIerStruct(pQUERY_OIER_STRUCT QueryStruct)
{
	FreeBOITSession(QueryStruct->boitSession);
	FreeVBuf(QueryStruct->vBuffer);
	free(QueryStruct);
	return TRUE;
}



BOOL QueryOIerInfo(pBOIT_SESSION boitSession, WCHAR *  ToSearchStr)
{
	WCHAR UrlBuffer[256];
	char* UTF8Search = StrConvWC2MB(CP_UTF8, ToSearchStr, -1, 0);
	char EncodedSearchStr[256];
	URLEncode(UTF8Search, strlen(UTF8Search), EncodedSearchStr, _countof(EncodedSearchStr));
	free(UTF8Search);

	WCHAR * WCEncodedSearch = StrConvMB2WC(CP_ACP, EncodedSearchStr, -1, 0);
	//http://bytew.net/OIer/search.php?method=normal&q=%E6%9D%A8%E8%B5%AB
	swprintf_s(UrlBuffer, _countof(UrlBuffer), L"/OIer/search.php?method=normal&q=%ls", WCEncodedSearch);
	free(WCEncodedSearch);
	pQUERY_OIER_STRUCT QueryStruct = AllocQueryOIerStruct(boitSession);
	WCHAR* rgpszAcceptTypes[] = { L"*/*", NULL };
	QueryStruct->hRequest = HttpOpenRequestW(hConnect, L"GET", UrlBuffer,
		NULL, NULL, rgpszAcceptTypes, INTERNET_FLAG_RELOAD, QueryStruct);

	BOOL x = HttpSendRequestW(QueryStruct->hRequest, 0, 0, 0, 0);
	return TRUE;
}



VOID CALLBACK QueryOIerCallback(
	_In_ HINTERNET hInternet,
	_In_opt_ DWORD_PTR dwContext,
	_In_ DWORD dwInternetStatus,
	_In_opt_ LPVOID lpvStatusInformation,
	_In_ DWORD dwStatusInformationLength
)
{
	pQUERY_OIER_STRUCT QueryStruct = dwContext;
	BOOL bSuccess = FALSE;
	HINTERNET * a = lpvStatusInformation;
	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_REQUEST_COMPLETE:
	{
		INTERNET_ASYNC_RESULT* AsyncResult = lpvStatusInformation;
		if (AsyncResult->dwResult == 0)
		{
			//Failed

		}
		else
		{
			if (!QueryStruct->bRequestComplete)
			{
				QueryStruct->bRequestComplete = TRUE;
			}
			else
			{
				//�ϴν�����ƽ�������
				//if (QueryStruct->InetBuf.dwBufferLength < 1000)
			ReadNow:
				{
					int OrgLen = QueryStruct->vBuffer->Length;
					AddSizeVBuf(QueryStruct->vBuffer, QueryStruct->BytesRead);
					memcpy(QueryStruct->vBuffer->Data + OrgLen, QueryStruct->ReadBuffer, QueryStruct->BytesRead);
				}
			}


			//QueryStruct->InetBuf.dwBufferLength = OIER_QRY_BUFSZ;
			ZeroMemory(QueryStruct->ReadBuffer, OIER_QRY_BUFSZ);
			//BOOL bRet = InternetReadFile(QueryStruct->hRequest, &(QueryStruct->InetBuf), IRF_ASYNC, QueryStruct);

			BOOL bRet = InternetReadFile(QueryStruct->hRequest, QueryStruct->ReadBuffer, OIER_QRY_BUFSZ, &(QueryStruct->BytesRead));
			DWORD dwErr = GetLastError();
			if (!bRet)
			{
				if (dwErr == ERROR_IO_PENDING)
				{
					//�����ERROR_IO_PENDING�Ļ���������ȥ�ȴ�ִ����ϼ���
					return;
				}
			}
			else
			{
				if (QueryStruct->BytesRead)
				{
					goto ReadNow;
				}
				//���������ˣ�ĩβ��һ��0
				AddSizeVBuf(QueryStruct->vBuffer, 1);
				QueryStruct->vBuffer->Data[QueryStruct->vBuffer->Length - 1] = 0;
				bSuccess = TRUE;
				
				//����json
				ParseOIerInfoJsonAndSend(QueryStruct->boitSession, QueryStruct->vBuffer->Data);
			}
		}

		//��������������Ĳ���ʧ���˾��ǽ����ˡ�����
		if (!bSuccess)
		{
			//ʧ��֪ͨ
			SendBackMessage(QueryStruct->boitSession, L"��ѽ����ѯOIerʧ����");
		}
		InternetCloseHandle(QueryStruct->hRequest);
		FreeQueryOIerStruct(QueryStruct);
		
	}
	}
	return;
}



BOOL ParseOIerInfoJsonAndSend(pBOIT_SESSION boitSession, char* JsonData)
{
	//����
	cJSON* JsonRoot = cJSON_Parse(JsonData);
	int TotalResult = 0;
	pVBUF PerPersonResult[OIER_MAX_DISPLAY] = { 0 };
	__try
	{
		if (JsonRoot && JsonRoot->child && JsonRoot->child->child)
		{
			for (cJSON* EnumPerson = JsonRoot->child->child; EnumPerson; EnumPerson = EnumPerson->next)
			{
				
				if (TotalResult >= OIER_MAX_DISPLAY)
				{
					TotalResult++;
					continue;
				}
				PerPersonResult[TotalResult] = AllocVBuf();
				
				char * InfoField[] = { "name", "sex","awards" };
				cJSON * JsonInfoField[_countof(InfoField)] = { 0 };
				for (cJSON* EnumField = EnumPerson->child; EnumField; EnumField = EnumField->next)
				{
					for (int i = 0; i < _countof(InfoField); i++)
					{
						if ((!JsonInfoField[i]) && _strcmpi(EnumField->string, InfoField[i]) == 0)
							JsonInfoField[i] = EnumField;
					}
				}

				if (JsonInfoField[0]) // ����
				{
					VBufferAppendStringW(PerPersonResult[TotalResult], L"������");
					WCHAR* NameString = StrConvMB2WC(CP_UTF8, JsonInfoField[0]->valuestring, -1, 0);
					VBufferAppendStringW(PerPersonResult[TotalResult], NameString);
					free(NameString);

					VBufferAppendStringW(PerPersonResult[TotalResult], L"  ");

					if (JsonInfoField[1])
					{
						WCHAR * SexNameList[] = { L"Ů",L"δ֪",L"��" };
						int SexIndex = atoi(JsonInfoField[1]->valuestring);
						if (SexIndex == 1 || SexIndex == -1)
						{
							VBufferAppendStringW(PerPersonResult[TotalResult], L"�����Ա�");
							VBufferAppendStringW(PerPersonResult[TotalResult], SexNameList[SexIndex + 1]);
							VBufferAppendStringW(PerPersonResult[TotalResult], L"  ");
						}
					}
					if (JsonInfoField[2])
					{
						char * AwardsStr = JsonInfoField[2]->valuestring;
						int AwardStrlen = strlen(AwardsStr);
						for (int i = 0; i < AwardStrlen; i++)
						{
							if (AwardsStr[i] == '\'')
							{
								AwardsStr[i] = '\"';
							}
						}
						cJSON* ParseAward = cJSON_Parse(AwardsStr);

						if (ParseAward && ParseAward->child)
						{
							for (cJSON* EnumAward = ParseAward->child; EnumAward; EnumAward = EnumAward->next)
							{
								char *AwardField[] = { "rank","province","award_type","identity","school","grade","score" };
								cJSON* JsonAwardField[_countof(AwardField)] = { 0 };

								for (cJSON* EnumAwardField = EnumAward->child; EnumAwardField; EnumAwardField = EnumAwardField->next)
								{
									for (int i = 0; i < _countof(AwardField); i++)
									{
										if ((!JsonAwardField[i]) && _strcmpi(EnumAwardField->string, AwardField[i]) == 0)
											JsonAwardField[i] = EnumAwardField;
									}
								}

								VBufferAppendStringW(PerPersonResult[TotalResult], L"\n");

								if (JsonAwardField[5])//�꼶
								{
									VBufferAppendStringW(PerPersonResult[TotalResult], L"��");
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[5]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"ʱ");
									free(Str);
								}
								VBufferAppendStringW(PerPersonResult[TotalResult], L"��");
								if (JsonAwardField[1])//ʡ��
								{
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[1]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], L" ");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L" ");
									free(Str);
								}
								if (JsonAwardField[4])//ѧУ
								{
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[4]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									free(Str);
								}
								if (JsonAwardField[3])//��������
								{
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[3]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"�μ�");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"��");
									free(Str);
								}
								if (JsonAwardField[6] && JsonAwardField[2])//���� + ����
								{
									WCHAR* Str = StrConvMB2WC(CP_UTF8, JsonAwardField[6]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"��");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"�ĳɼ�ȡ��");
									free(Str);
									Str = StrConvMB2WC(CP_UTF8, JsonAwardField[2]->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									free(Str);
								}
								else if (JsonAwardField[6] || JsonAwardField[2])//���� or ����
								{
									cJSON* ChosenField = JsonAwardField[6] ? JsonAwardField[6] : JsonAwardField[2];
									WCHAR* Str = StrConvMB2WC(CP_UTF8, ChosenField->valuestring, -1, 0);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"ȡ��");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"�ĳɼ�");
									free(Str);
								}
								if (JsonAwardField[0])//����
								{
									WCHAR Str[8];
									swprintf_s(Str, _countof(Str), L"%d", JsonAwardField[0]->valueint);
									VBufferAppendStringW(PerPersonResult[TotalResult], L"������");
									VBufferAppendStringW(PerPersonResult[TotalResult], Str);
								}
								VBufferAppendStringW(PerPersonResult[TotalResult], L"��");
							}
						}


						if(ParseAward)cJSON_Delete(ParseAward);
					}
					TotalResult++;
				}
			}
		}
	}
	__finally
	{
		if (TotalResult == 0)
		{
			SendBackMessage(boitSession, L"ʲô��û�ҵ��� �����Կ���");
		}
		else
		{
			for (int i = 0; i < OIER_MAX_DISPLAY; i++)
			{
				if (PerPersonResult[i])
				{
					AddSizeVBuf(PerPersonResult[i], sizeof(WCHAR) * 1);
					((WCHAR*)(PerPersonResult[i]->Data))[(PerPersonResult[i]->Length / 2) - 1] = 0;
					SendBackMessage(boitSession, PerPersonResult[i]->Data);
				}
			}
			if (TotalResult > OIER_MAX_DISPLAY)
			{
				SendBackMessage(boitSession, L"������Ϣ���¼ OIerDB ��վ��ϸ�鿴");
			}

			cJSON_Delete(JsonRoot);
			for (int i = 0; i < OIER_MAX_DISPLAY; i++)
			{
				if (PerPersonResult[i])FreeVBuf(PerPersonResult[i]);
			}
		}
		

		switch (rand() % 8)
		{
		case 0:
			SendBackMessage(boitSession, L"orz nocriz!!!");
			break;
		case 1:
			SendBackMessage(boitSession, L"��л nocriz ������ OIerDB!");
			break;
		}
	}

}


