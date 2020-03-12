#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"VBuffer.h"
#include"cJSON.h"
#include<strsafe.h>
#include"EncodeConvert.h"
#include<shlwapi.h>
#include"DirManagement.h"
#include"URIEncode.h"
#include"Corpus.h"

#include"AsyncINet.h"

pASYNCINET_INFO CodeforcesInetInfo;


#define OIER_MAX_DISPLAY 3


WCHAR CFServerName[] = L"codeforces.com";

static HINTERNET hInet;
static HINTERNET hConnect;

int AsyncCFUserCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
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
		CodeforcesInetInfo = AsyncINetInit(CFServerName);
		break;

	case EC_CMDFREE:
		AsyncINetCleanup(CodeforcesInetInfo);
		break;
	}
	return 0;
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


	pBOIT_SESSION newBoitSess = DuplicateBOITSession(boitSession);
	AsyncRequestGet(CodeforcesInetInfo, UrlBuffer, newBoitSess, AsyncCFUserCallback);
	return TRUE;
}


int AsyncCFUserCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
)
{
	switch (iReason)
	{
	case ASYNCINET_REASON_SUCCESS:
		AddSizeVBuf(ReceivedBuf, 1);
		ReceivedBuf->Data[ReceivedBuf->Length - 1] = 0;

		//����json
		ParseCFUserInfoJsonAndSend((pBOIT_SESSION)ExtData, ReceivedBuf->Data);
		break;
	case ASYNCINET_REASON_FAILED:
		SendBackMessage((pBOIT_SESSION)ExtData, L"��ѽ����ѯCF�û�ʧ����");
		break;
	}
	FreeBOITSession((pBOIT_SESSION)ExtData);
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
					/*if (ProfileField[1] && ProfileField[2])
					{
						WCHAR* firstNameString = StrConvMB2WC(CP_UTF8, ProfileField[2]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, firstNameString);
						VBufferAppendStringW(ResultStr, L"  ");
						free(firstNameString);

						WCHAR* lastNameString = StrConvMB2WC(CP_UTF8, ProfileField[1]->valuestring, -1, 0);
						VBufferAppendStringW(ResultStr, lastNameString);
						VBufferAppendStringW(ResultStr, L"\n");
						free(lastNameString);
					}*/
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

