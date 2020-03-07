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
#include"Corpus.h"
#pragma comment(lib,"WinINet.lib")


#define CFUSER_QRY_BUFSZ 4096

#define OIER_MAX_DISPLAY 3
typedef struct __tagQueryCFUser
{
	pBOIT_SESSION boitSession;
	BYTE ReadBuffer[CFUSER_QRY_BUFSZ];
	DWORD BytesRead;

	HINTERNET hRequest;
	pVBUF vBuffer;
	BOOL bRequestComplete;
}QUERY_CFUSER_STRUCT, * pQUERY_CFUSER_STRUCT;

WCHAR CFServerName[] = L"codeforces.com";

HINTERNET hInet;
HINTERNET hConnect;

VOID CALLBACK QueryCFUserCallback(
	_In_ HINTERNET hInternet,
	_In_opt_ DWORD_PTR dwContext,
	_In_ DWORD dwInternetStatus,
	_In_opt_ LPVOID lpvStatusInformation,
	_In_ DWORD dwStatusInformationLength
);


int CmdMsg_codeforces_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	//if (boitSession->GroupID && CheckGroupToken(boitSession->GroupID, L"PrivilegeQueryCFUser") == 0)
	//{
	//	SendBackMessage(boitSession, L"��Ⱥ��ֹ�˲�ѯCF User���ܡ�����˽���в�ѯ��");
	//	return 0;
	//}



	int ParamLen = GetCmdParamLen(Msg);
	int SpaceLen = GetCmdSpaceLen(Msg + ParamLen);

	int OrderStrlen = GetCmdParamLen(Msg + ParamLen + SpaceLen);
	 
	if (OrderStrlen >= 64)
	{
		SendBackMessage(boitSession, L"�޷�ʶ��ָ��");
	}
	else if (OrderStrlen == 0)
	{
		SendBackMessage(boitSession, L"����#cf help�鿴����");
	}

	WCHAR* OrderStr[] = { L"help",L"profile",L"contests" };
	int iMatch;
	for (iMatch = 0; iMatch < _countof(OrderStr); iMatch++)
	{
		if (OrderStrlen == wcslen(OrderStr[iMatch]) && (_wcsnicmp(Msg + ParamLen + SpaceLen, OrderStr[iMatch], OrderStrlen) == 0))
		{
			break;
		}
	}
	switch (iMatch)
	{
	case 0:
		SendBackMessage(boitSession, L"��ѯ�й�codeforces����Ϣ��\n#cf profile [�û���] �Բ�ѯprofile\n#cf contests �Բ�ѯ������Ϣ\n#cf help �Բ鿴�ð�����Ϣ");
		
		break;
	case 1:
	{
		WCHAR * OrderBase = Msg + ParamLen + SpaceLen + OrderStrlen;
		OrderBase += GetCmdSpaceLen(OrderBase);
		int QueryStrlen = GetCmdParamLen(OrderBase);
		if (QueryStrlen >= 64)
		{
			SendBackMessage(boitSession, L"�����ĸ�CF�û�����������ô����");
		}
		else if (QueryStrlen == 0)
		{
			SendBackMessage(boitSession, L"��Ҫ��ѯ�����ĸ�CF�û�����");
		}
		else
		{
			SendBackMessage(boitSession, L"��ѯ��...�����Ҫһ��ʱ��");
			QueryCFUserInfo(boitSession, OrderBase);
		}
	}
		
		break;
	case 2:
		SendBackMessage(boitSession, Corpus_FunctionDeving());
		
		break;
	default:
		SendBackMessage(boitSession, L"δ�ҵ�ָ�����#cf help�鿴����");
		break;
	}
	/*else
	{
		
	}*/

	return 0;
}

int CmdEvent_codeforces_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_CMDLOAD:
		hInet = InternetOpenW(L"BOIT", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
		INTERNET_STATUS_CALLBACK pOldStatusCallback = InternetSetStatusCallbackW(hInet, QueryCFUserCallback);

		// For HTTP InternetConnect returns synchronously because it does not
		// actually make the connection.

		hConnect = InternetConnectW(hInet,
			CFServerName, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		break;


	case EC_CMDFREE:
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInet);
		break;
	}
	return 0;
}


pQUERY_CFUSER_STRUCT AllocQueryCFUserStruct(pBOIT_SESSION boitSession)
{
	pQUERY_CFUSER_STRUCT QueryStruct = malloc(sizeof(QUERY_CFUSER_STRUCT));
	ZeroMemory(QueryStruct, sizeof(QUERY_CFUSER_STRUCT));

	QueryStruct->boitSession = DuplicateBOITSession(boitSession);

	QueryStruct->vBuffer = AllocVBuf();

	return QueryStruct;
}


BOOL FreeQueryCFUserStruct(pQUERY_CFUSER_STRUCT QueryStruct)
{
	FreeBOITSession(QueryStruct->boitSession);
	FreeVBuf(QueryStruct->vBuffer);
	free(QueryStruct);
	return TRUE;
}



BOOL QueryCFUserInfo(pBOIT_SESSION boitSession, WCHAR* ToSearchStr)
{
	WCHAR UrlBuffer[256];
	char* UTF8Search = StrConvWC2MB(CP_UTF8, ToSearchStr, -1, 0);
	char EncodedSearchStr[256];
	URLEncode(UTF8Search, strlen(UTF8Search), EncodedSearchStr, _countof(EncodedSearchStr));
	free(UTF8Search);

	WCHAR* WCEncodedSearch = StrConvMB2WC(CP_ACP, EncodedSearchStr, -1, 0);
	swprintf_s(UrlBuffer, _countof(UrlBuffer), L"/api/user.info?handles=%s", WCEncodedSearch);
	free(WCEncodedSearch);
	pQUERY_CFUSER_STRUCT QueryStruct = AllocQueryCFUserStruct(boitSession);
	WCHAR* rgpszAcceptTypes[] = { L"*/*", NULL };
	QueryStruct->hRequest = HttpOpenRequestW(hConnect, L"GET", UrlBuffer,
		NULL, NULL, rgpszAcceptTypes, INTERNET_FLAG_RELOAD, QueryStruct);

	BOOL x = HttpSendRequestW(QueryStruct->hRequest, 0, 0, 0, 0);
	return TRUE;
}



VOID CALLBACK QueryCFUserCallback(
	_In_ HINTERNET hInternet,
	_In_opt_ DWORD_PTR dwContext,
	_In_ DWORD dwInternetStatus,
	_In_opt_ LPVOID lpvStatusInformation,
	_In_ DWORD dwStatusInformationLength
)
{
	pQUERY_CFUSER_STRUCT QueryStruct = dwContext;
	BOOL bSuccess = FALSE;
	HINTERNET* a = lpvStatusInformation;
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


			//QueryStruct->InetBuf.dwBufferLength = CFUSER_QRY_BUFSZ;
			ZeroMemory(QueryStruct->ReadBuffer, CFUSER_QRY_BUFSZ);
			//BOOL bRet = InternetReadFile(QueryStruct->hRequest, &(QueryStruct->InetBuf), IRF_ASYNC, QueryStruct);

			BOOL bRet = InternetReadFile(QueryStruct->hRequest, QueryStruct->ReadBuffer, CFUSER_QRY_BUFSZ, &(QueryStruct->BytesRead));
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
				ParseCFUserInfoJsonAndSend(QueryStruct->boitSession, QueryStruct->vBuffer->Data);
			}
		}

		//��������������Ĳ���ʧ���˾��ǽ����ˡ�����
		if (!bSuccess)
		{
			//ʧ��֪ͨ
			SendBackMessage(QueryStruct->boitSession, L"��ѽ����ѯCF�û�ʧ����");
		}
		InternetCloseHandle(QueryStruct->hRequest);
		FreeQueryCFUserStruct(QueryStruct);

	}
	}
	return;
}



BOOL ParseCFUserInfoJsonAndSend(pBOIT_SESSION boitSession, char* JsonData)
{
	cJSON* JsonRoot = cJSON_Parse(JsonData);

	if (!JsonRoot)
	{
		SendBackMessage(boitSession, L"����ȥ���˵�С����...");
		return 0;
	}
	pVBUF ResultStr = AllocVBuf();

	__try
	{
		char* FieldName[] = { "status", "comment","result" };
		cJSON* JsonInfoField[_countof(FieldName)] = { 0 };
		for (cJSON* EnumField = JsonRoot->child; EnumField; EnumField = EnumField->next)
		{
			for (int i = 0; i < _countof(FieldName); i++)
			{
				if ((!JsonInfoField[i]) && _strcmpi(EnumField->string, FieldName[i]) == 0)
					JsonInfoField[i] = EnumField;
			}
		}

		if (JsonInfoField[0])
		{
			if (_strcmpi(JsonInfoField[0]->valuestring, "OK") == 0)
			{
				if (JsonInfoField[2] && JsonInfoField[2]->child && JsonInfoField[2]->child->child)
				{
					char* ProfileFieldName[] = { "handle" ,"lastName", "firstName",
						"country",// 3
						"rating",// 4
						"maxRating",// 5
						"rank",// 6
						"maxRank" // 7
					};
					cJSON* ProfileField[_countof(ProfileFieldName)] = { 0 };
					for (cJSON* ProfileFieldEnum = JsonInfoField[2]->child->child; ProfileFieldEnum; ProfileFieldEnum = ProfileFieldEnum->next)
					{
						for (int i = 0; i < _countof(ProfileFieldName); i++)
						{
							if ((!ProfileField[i]) && _strcmpi(ProfileFieldEnum->string, ProfileFieldName[i]) == 0)
								ProfileField[i] = ProfileFieldEnum;
						}
					}

					if (ProfileField[0])
					{
						WCHAR* NickString = StrConvMB2WC(CP_UTF8, ProfileField[0]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, L"�ǳƣ�");
						VBufferAppendStringW(ResultStr, NickString);
						VBufferAppendStringW(ResultStr, L"\n");
						free(NickString);
					}
					if (ProfileField[1] && ProfileField[2])
					{
						WCHAR* firstNameString = StrConvMB2WC(CP_UTF8, ProfileField[2]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, firstNameString);
						VBufferAppendStringW(ResultStr, L"  ");
						free(firstNameString);

						WCHAR* lastNameString = StrConvMB2WC(CP_UTF8, ProfileField[1]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, lastNameString);
						VBufferAppendStringW(ResultStr, L"\n");
						free(lastNameString);
					}
					if (ProfileField[3])
					{
						WCHAR* CountryStr = StrConvMB2WC(CP_UTF8, ProfileField[3]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, CountryStr);
						VBufferAppendStringW(ResultStr, L"\n");
						free(CountryStr);
					}
					if (ProfileField[4] && ProfileField[5])
					{
						WCHAR Str[32];
						swprintf_s(Str, _countof(Str), L"rating: %d(max: %d)\n", ProfileField[4]->valueint, ProfileField[5]->valueint);
						VBufferAppendStringW(ResultStr, Str);
					}
					if (ProfileField[6] && ProfileField[7])
					{
						WCHAR* RankStr = StrConvMB2WC(CP_UTF8, ProfileField[6]->valuestring, -1, 0);
						WCHAR* MaxRankStr = StrConvMB2WC(CP_UTF8, ProfileField[7]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, L"rank: ");
						VBufferAppendStringW(ResultStr, RankStr);
						VBufferAppendStringW(ResultStr, L"(max: ");
						VBufferAppendStringW(ResultStr, MaxRankStr);
						VBufferAppendStringW(ResultStr, L")\n");
						free(RankStr);
						free(MaxRankStr);
					}
				}
				else
				{
					SendBackMessage(boitSession, L"����ȥ���˵�С����...");
				}
			}
			else if (_strcmpi(JsonInfoField[0]->valuestring, "FAILED") == 0)
			{
				SendBackMessage(boitSession, L"ʲô��û�ҵ��� �����Կ���");
			}
			else
			{
				SendBackMessage(boitSession, L"����ȥ���˵�С����...");
				__leave;
			}
		}
	}
	__finally
	{
		if (ResultStr->Length)
		{
			AddSizeVBuf(ResultStr, sizeof(WCHAR) * 1);
			((WCHAR*)(ResultStr->Data))[(ResultStr->Length / 2) - 1] = 0;
			SendBackMessage(boitSession, ResultStr->Data);
		}

		cJSON_Delete(JsonRoot);
		FreeVBuf(ResultStr);
	}
	return 0;
}

