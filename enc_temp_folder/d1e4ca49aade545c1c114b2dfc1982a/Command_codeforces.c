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



typedef struct __tagCFUserInfo
{
	WCHAR* Handle;
	WCHAR* LastName;
	WCHAR* FirstName;
	WCHAR* Country;
	int Rating; // ������Ϊ -1
	int MaxRating; // ������Ϊ -1
	WCHAR* Rank;
	WCHAR* MaxRank;
	WCHAR* TitlePhotoURL;
}CFUSERINFO, * pCFUSERINFO;


typedef struct __tagQueryCFPhotoSession
{
	CFUSERINFO CFUserInfo;
	pBOIT_SESSION boitSession;
	pASYNCINET_INFO CodeforcesPhotoInetInfo;
}QUERY_CFPHOTO_SESS, * pQUERY_CFPHOTO_SESS;

pASYNCINET_INFO CodeforcesInetInfo;




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
		WCHAR* OrderBase = Msg + ParamLen + SpaceLen + OrderStrlen;
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
		//������ܻ������ѯͷ�����Բ�ɾ��boitSession
		break;
	case ASYNCINET_REASON_FAILED:
		SendBackMessage((pBOIT_SESSION)ExtData, L"��ѽ����ѯCF�û�ʧ����");
		FreeBOITSession((pBOIT_SESSION)ExtData);
		break;
	}
	
}

int AsyncCFUserPhotoCallback(
	UINT iReason,
	pVBUF ReceivedBuf,
	PBYTE ExtData
)
{
	pQUERY_CFPHOTO_SESS QueryPhotoSess = (pQUERY_CFPHOTO_SESS)ExtData;

	pVBUF ReplyBuffer = AllocVBuf();
	
	if (QueryPhotoSess->CFUserInfo.Handle[0])
	{
		VBufferAppendStringW(ReplyBuffer, L"�ǳƣ�");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.Handle);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}
	/*if (QueryPhotoSess->CFUserInfo.FirstName[0] && QueryPhotoSess->CFUserInfo.LastName)
	{
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.FirstName);
		VBufferAppendStringW(ReplyBuffer, L"  ");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.LastName);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}*/
	if (QueryPhotoSess->CFUserInfo.Country[0])
	{
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.Country);
		VBufferAppendStringW(ReplyBuffer, L"\n");
	}
	if ((QueryPhotoSess->CFUserInfo.Rating != -1)&& (QueryPhotoSess->CFUserInfo.MaxRating!=-1))
	{
		WCHAR Str[32];
		swprintf_s(Str, _countof(Str), L"rating: %d(max: %d)\n", QueryPhotoSess->CFUserInfo.Rating, QueryPhotoSess->CFUserInfo.MaxRating);
		VBufferAppendStringW(ReplyBuffer, Str);
	}
	if (QueryPhotoSess->CFUserInfo.Rank && QueryPhotoSess->CFUserInfo.MaxRank)
	{
		VBufferAppendStringW(ReplyBuffer, L"rank: ");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.Rank);
		VBufferAppendStringW(ReplyBuffer, L"(max: ");
		VBufferAppendStringW(ReplyBuffer, QueryPhotoSess->CFUserInfo.MaxRank);
		VBufferAppendStringW(ReplyBuffer, L")\n");
	}

	switch (iReason)
	{
	case ASYNCINET_REASON_SUCCESS:
	{
		WCHAR PhotoFileName[MAX_PATH];
		CoolQAllocPicFileName(PhotoFileName);

		WCHAR PhotoFilePath[MAX_PATH];
		wcscpy_s(PhotoFilePath,MAX_PATH,GetCQImageDir());
		PathAppendW(PhotoFilePath, PhotoFileName);
		HFILE hFile = CreateFileW(PhotoFilePath, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		DWORD BytesWrite;
		BOOL bSuccess = WriteFile(hFile, ReceivedBuf->Data, ReceivedBuf->Length, &BytesWrite, 0);
		CloseHandle(hFile);
		
		WCHAR TestBuffer[32];

		swprintf_s(TestBuffer, _countof(TestBuffer), L"[CQ:image,file=%ls]", PhotoFileName);
		VBufferAppendStringW(ReplyBuffer, TestBuffer);
	}
		
		break;
	case ASYNCINET_REASON_FAILED:
		VBufferAppendStringW(ReplyBuffer, L"ץȡͷ��ʧ����orz");
		break;
	}
	FreeBOITSession((pBOIT_SESSION)QueryPhotoSess->boitSession);
	AsyncINetCleanup(QueryPhotoSess->CodeforcesPhotoInetInfo);

	//����CF Profile
	
	

	if (ReplyBuffer->Length)
	{
		AddSizeVBuf(ReplyBuffer, sizeof(WCHAR) * 1);
		((WCHAR*)(ReplyBuffer->Data))[(ReplyBuffer->Length / 2) - 1] = 0;
		SendBackMessage(QueryPhotoSess->boitSession, ReplyBuffer->Data);
	}

	{
		free(QueryPhotoSess->CFUserInfo.Country);
		free(QueryPhotoSess->CFUserInfo.FirstName);
		free(QueryPhotoSess->CFUserInfo.Handle);
		free(QueryPhotoSess->CFUserInfo.LastName);
		free(QueryPhotoSess->CFUserInfo.MaxRank);
		free(QueryPhotoSess->CFUserInfo.Rank);
		free(QueryPhotoSess->CFUserInfo.TitlePhotoURL);
	}
	free(QueryPhotoSess);

	FreeVBuf(ReplyBuffer);
	return 0;
}


BOOL ParseCFUserInfoJsonAndSend(pBOIT_SESSION boitSession, char* JsonData)
{
	cJSON* JsonRoot = cJSON_Parse(JsonData);

	if (!JsonRoot)
	{
		SendBackMessage(boitSession, L"����ȥ���˵�С����...");
		return 0;
	}

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
						"maxRank", // 7
						"titlePhoto" // 8
					};
					cJSON* ProfileField[_countof(ProfileFieldName)] = { 0 };

					int iMatchProfileField = 0;
					for (cJSON* ProfileFieldEnum = JsonInfoField[2]->child->child; ProfileFieldEnum; ProfileFieldEnum = ProfileFieldEnum->next)
					{
						for (int i = 0; i < _countof(ProfileFieldName); i++)
						{
							if ((!ProfileField[i]) && _strcmpi(ProfileFieldEnum->string, ProfileFieldName[i]) == 0)
							{
								ProfileField[i] = ProfileFieldEnum;
								iMatchProfileField++;
							}
						}
					}

					if (iMatchProfileField)
					{
						pQUERY_CFPHOTO_SESS CFQueryPhotoSess = malloc(sizeof(QUERY_CFPHOTO_SESS));
						ZeroMemory(CFQueryPhotoSess, sizeof(QUERY_CFPHOTO_SESS));

						CFQueryPhotoSess->boitSession = boitSession;

						if (ProfileField[0])
						{
							CFQueryPhotoSess->CFUserInfo.Handle = StrConvMB2WC(CP_UTF8, ProfileField[0]->valuestring, -1, 0);
						}
						if (ProfileField[1])
						{
							CFQueryPhotoSess->CFUserInfo.LastName = StrConvMB2WC(CP_UTF8, ProfileField[1]->valuestring, -1, 0);
						}
						if (ProfileField[2])
						{
							CFQueryPhotoSess->CFUserInfo.FirstName = StrConvMB2WC(CP_UTF8, ProfileField[2]->valuestring, -1, 0);
						}
						if (ProfileField[3])
						{
							CFQueryPhotoSess->CFUserInfo.Country = StrConvMB2WC(CP_UTF8, ProfileField[3]->valuestring, -1, 0);
						}
						if (ProfileField[4])
						{
							CFQueryPhotoSess->CFUserInfo.Rating = ProfileField[4]->valueint;
						}
						if (ProfileField[5])
						{
							CFQueryPhotoSess->CFUserInfo.MaxRating = ProfileField[5]->valueint;
						}
						if (ProfileField[6])
						{
							CFQueryPhotoSess->CFUserInfo.Rank = StrConvMB2WC(CP_UTF8, ProfileField[6]->valuestring, -1, 0);
						}
						if (ProfileField[7])
						{
							CFQueryPhotoSess->CFUserInfo.MaxRank = StrConvMB2WC(CP_UTF8, ProfileField[7]->valuestring, -1, 0);
						}
						if (ProfileField[8])
						{
							CFQueryPhotoSess->CFUserInfo.TitlePhotoURL = StrConvMB2WC(CP_UTF8, ProfileField[8]->valuestring, -1, 0);
						}
						WCHAR FormatTitlePhotoURL[256];
						WCHAR FormatTitlePhotoHostName[256];
						WCHAR URLQueryPart[256];
						DWORD dURLPartLen;

						//ƴ��URL
						WCHAR Scheme[] = L"http:";
						swprintf(FormatTitlePhotoURL, _countof(FormatTitlePhotoURL), L"%s%ls?a=1", Scheme, CFQueryPhotoSess->CFUserInfo.TitlePhotoURL);

						dURLPartLen = _countof(FormatTitlePhotoHostName);
						UrlGetPartW(FormatTitlePhotoURL, FormatTitlePhotoHostName, &dURLPartLen, URL_PART_HOSTNAME, 0);

						dURLPartLen = _countof(URLQueryPart);
						UrlGetPartW(FormatTitlePhotoURL, URLQueryPart, &dURLPartLen, URL_PART_QUERY, 0);

						CFQueryPhotoSess->CodeforcesPhotoInetInfo = AsyncINetInit(FormatTitlePhotoHostName);
						AsyncRequestGet(CFQueryPhotoSess->CodeforcesPhotoInetInfo,
							FormatTitlePhotoURL + wcslen(Scheme) + wcslen(L"//") + wcslen(FormatTitlePhotoHostName),
							CFQueryPhotoSess, AsyncCFUserPhotoCallback);
					}
					else
					{
						SendBackMessage(boitSession, L"����ȥ���˵�С����...");
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
		cJSON_Delete(JsonRoot);
	}
	return 0;
}

