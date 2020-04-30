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
#include"AsyncINet.h"



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

pASYNCINET_INFO OIerDBInetInfo;

int AsyncOIerInfoCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
);

int CmdMsg_oier_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	if ((GetBOITSessionType(boitSession) == BOITSESS_TYPE_GROUP) && CheckGroupToken(boitSession->GroupID, L"PrivilegeQueryOIer") == 0)
	{
		SendBackMessage(boitSession, L"��Ⱥ��ֹ�˲�ѯOIer���ܡ�����˽���в�ѯ����ϵ����Ա���Ź��ܡ�");
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
		SendBackMessage(boitSession, L"��Ҫ��ѯ�����ĸ�OIer���������� #oier ����/ƴ����");
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
		OIerDBInetInfo = AsyncINetInit(OIerDBServerName);
		break;

	case EC_CMDFREE:
		AsyncINetCleanup(OIerDBInetInfo);
		break;
	}
	return 0;
}







BOOL QueryOIerInfo(pBOIT_SESSION boitSession, WCHAR* ToSearchStr)
{
	WCHAR* UrlBuffer;
	int UTF8Len;
	char* UTF8Search = StrConvWC2MB(CP_UTF8, ToSearchStr, -1, &UTF8Len);
	char* EncodedSearchStr;
	EncodedSearchStr = malloc((UTF8Len + 1) * 3); //������ÿ���ַ���ת��
	ZeroMemory(EncodedSearchStr, (UTF8Len + 1) * 3);
	URLEncode(UTF8Search, strlen(UTF8Search), EncodedSearchStr, (UTF8Len + 1) * 3);
	free(UTF8Search);

	int WCSLen;
	WCHAR* WCEncodedSearch = StrConvMB2WC(CP_ACP, EncodedSearchStr, -1, &WCSLen);
	free(EncodedSearchStr);

	UrlBuffer = malloc((wcslen(L"/OIer/search.php?method=normal&q=") + WCSLen + 1) * sizeof(WCHAR));
	swprintf_s(UrlBuffer, (wcslen(L"/OIer/search.php?method=normal&q=") + WCSLen + 1), L"/OIer/search.php?method=normal&q=%ls", WCEncodedSearch);
	free(WCEncodedSearch);


	pBOIT_SESSION newBoitSess = DuplicateBOITSession(boitSession);
	AsyncRequestGet(OIerDBInetInfo, UrlBuffer, newBoitSess, AsyncOIerInfoCallback);


	/*WCHAR UrlBuffer[256 + 32] = { 0 };
	char* UTF8Search = StrConvWC2MB(CP_UTF8, ToSearchStr, -1, 0);
	char EncodedSearchStr[256 + 1] = { 0 };
	URLEncode(UTF8Search, strlen(UTF8Search), EncodedSearchStr, _countof(EncodedSearchStr));
	free(UTF8Search);

	WCHAR* WCEncodedSearch = StrConvMB2WC(CP_ACP, EncodedSearchStr, -1, 0);
	swprintf_s(UrlBuffer, _countof(UrlBuffer), L"/OIer/search.php?method=normal&q=%ls", WCEncodedSearch);
	free(WCEncodedSearch);
	
	pBOIT_SESSION newBoitSess = DuplicateBOITSession(boitSession);
	AsyncRequestGet(OIerDBInetInfo, UrlBuffer, newBoitSess, AsyncOIerInfoCallback);*/
	return TRUE;
}


int AsyncOIerInfoCallback(
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
		ParseOIerInfoJsonAndSend((pBOIT_SESSION)ExtData, ReceivedBuf->Data);
		break;
	case ASYNCINET_REASON_FAILED:
		SendBackMessage((pBOIT_SESSION)ExtData, L"��ѽ����ѯOIerʧ����");
		break;
	}
	FreeBOITSession((pBOIT_SESSION)ExtData);
	return 0;
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

				char* InfoField[] = { "name", "sex","awards" };
				cJSON* JsonInfoField[_countof(InfoField)] = { 0 };
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
						WCHAR* SexNameList[] = { L"Ů",L"δ֪",L"��" };
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
						char* AwardsStr = JsonInfoField[2]->valuestring;
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
								char* AwardField[] = { "rank","province","award_type","identity","school","grade","score" };
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


						if (ParseAward)cJSON_Delete(ParseAward);
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


