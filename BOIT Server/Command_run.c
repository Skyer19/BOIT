#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"DirManagement.h"
#include"SimpleSandbox.h"
#include"SessionManage.h"
#include<wchar.h>
#include"VBuffer.h"
#include"RemoveCQEscapeChar.h"

#define COMPILECMD_MAXLEN 512
#define COMPILE_MAXSUFFIX 9
#define COMPILE_MAXNAME 16

#define COMPILE_TYPE_NULL 0
#define	COMPILE_TYPE_COMPILE 1
#define COMPILE_TYPE_SCRIPT 2

#define COMPILE_ENCODE_ANSI 0
#define COMPILE_ENCODE_UTF8 1
#define COMPILE_ENCODE_GB18030 2

typedef struct __tagCompileCfg
{
	int Type;
	WCHAR SourceSuffix[COMPILE_MAXSUFFIX + 1];
	WCHAR Application[MAX_PATH + 1];
	WCHAR Command[COMPILECMD_MAXLEN + 1];
	WCHAR Name[COMPILE_MAXNAME + 1];
	int SourceEncode;
	int OutputEncode;
}COMPILE_CFG, * pCOMPILE_CFG;


typedef struct __tagCompileSession
{
	BOIT_SESSION boitSession;
	pVBUF StdOutBuffer;
	pBOIT_COMMAND Command;
	pCOMPILE_CFG CompileCfg;
	LONGLONG AllocCompileID;
}COMPILE_SESSION, * pCOMPILE_SESSION;

typedef struct __tagRunSession
{
	UINT Encode;
	BOIT_SESSION boitSession;
	pVBUF StdOutBuffer;
}RUN_SESSION, * pRUN_SESSION;


BOOL FindCompileConfig(pBOIT_COMMAND pCmd, WCHAR* LanguageName, int LanguageLen, WCHAR* ConfigSuffix, pCOMPILE_CFG CompileCfg);

BOOL MatchCompileConfig(WCHAR* ConfigFileName, pCOMPILE_CFG CompileCfg, WCHAR* LanguageName, int LanguageLen);

int CompileSandboxCallback(pSANDBOX Sandbox, PBYTE pData, UINT Event, PBYTE StdOutData, DWORD DataLen);

int GetLineLen(WCHAR* String);

int GetLineFeedLen(WCHAR* String);

int GetLineSpaceLen(WCHAR* String);

LONGLONG CompileID;

int CmdMsg_run_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	if (CheckUserToken(QQID, L"PrivilegeRunCode") == 0)
	{
		SendBackMessage(GroupID, QQID, L"Opps... ��û���ʵ���Ȩ�޽��в���");
		return 0;
	}


	//����û�Ŀ¼���Ƿ�����Ӧ�ļ���
	if (PerUserCreateDirIfNExist(QQID, L"Sandbox"))
	{
		//������Ӧ�ļ�
	}
	if (PerUserCreateDirIfNExist(QQID, L"Compile"))
	{
		//������Ӧ�ļ�
	}

	//TODO:�������ȡ�û���Ϣ���Ƿ���Ȩ��ִ�г����Ƿ���Ȩ����suִ�г���


		//usage: #run language [/su] sourcecode
	int MsgLen = wcslen(Msg);
	WCHAR* lpwcParam = Msg;
	int ParamCnt = 0;
	pCOMPILE_CFG CompileCfg = malloc(sizeof(COMPILE_CFG));
	ZeroMemory(CompileCfg, sizeof(COMPILE_CFG));
	BOOL LanguageMatched = 0, bCodeFound = 0;
	WCHAR* CodeStr = 0;

	BOOL bFailed = 0, bIsSU = 0, bIsHelp = 0;

	WCHAR FailedReason[128];

	BOIT_SESSION boitSession = { 0 };
	boitSession.QQID = QQID;
	boitSession.GroupID = GroupID;
	if (AnonymousName) wcscpy_s(boitSession.AnonymousName, BOIT_MAX_NICKLEN, AnonymousName);

	__try
	{
		for (;MsgLen > 0;ParamCnt++)
		{
			int ParamLen = GetCmdParamLen(lpwcParam);


			//������� Param
			if (ParamCnt == 1 &&
				FindCompileConfig(pCmd, lpwcParam, ParamLen, L".cfg", CompileCfg) == TRUE)
			{
				LanguageMatched = TRUE;
			}
			else if (ParamCnt >= 1)
			{
				//�������ܵĲ���
				if (lpwcParam[0] == '-' || lpwcParam[0] == '/' || lpwcParam[0] == '\\')
				{
					int ParamLen = GetCmdParamLen(lpwcParam);
					if (ParamLen == 1)
					{
						swprintf_s(FailedReason, _countof(FailedReason), L"�޷��������� %lc", lpwcParam[0]);
						SendBackMessage(GroupID, QQID, FailedReason);
						bFailed = 1;
						break;
					}
					else if (ParamLen >= 16)
					{
						SendBackMessage(GroupID, QQID, L"�������ƹ���");
						bFailed = 1;
						break;
					}
					else
					{
						if ((ParamLen == wcslen(L"su") + 1) && _wcsnicmp(lpwcParam + 1, L"su", wcslen(L"su")) == 0)
						{
							bIsSU = TRUE;
						}
						if ((ParamLen == wcslen(L"help") + 1) && _wcsnicmp(lpwcParam + 1, L"help", wcslen(L"help")) == 0)
						{
							bIsHelp = TRUE;
						}
						else
						{
							WCHAR ParamBuf[16] = { 0 };
							wcsncpy_s(ParamBuf, _countof(ParamBuf), lpwcParam, ParamLen);
							swprintf_s(FailedReason, _countof(FailedReason), L"�޷��������� %ls", ParamBuf);
							SendBackMessage(GroupID, QQID, FailedReason);
							bFailed = 1;
							break;
						}
					}
				}
				else
				{
					//��������������
					bCodeFound = 1;
					CodeStr = lpwcParam;
					break;
				}
			}
			
			lpwcParam += ParamLen;
			MsgLen -= ParamLen;
			int SpaceLen = GetCmdSpaceLen(lpwcParam);
			lpwcParam += SpaceLen;
			MsgLen -= SpaceLen;
		}
	}
	__finally
	{
		if (!bFailed)
		{
			if (!bIsHelp)
			{
				if (LanguageMatched)
				{
					if (bCodeFound)
					{
						//������
					}
					else
					{
						SendBackMessage(GroupID, QQID, L"δ�ҵ�Դ����");
						bFailed = TRUE;
					}
				}
				else
				{
					SendBackMessage(GroupID, QQID, L"δ�ҵ��������ͻ�����Բ���֧��");
					bFailed = TRUE;
				}
			}
		}


		if (bFailed || bIsHelp)
		{
			if (bIsHelp)
			{
				//��ʾ��ϸ������Ϣ
				SendBackMessage(GroupID, QQID, L"usage: #run language [/su] sourcecode");

				ShowSupportLanguageInfo(pCmd, L".cfg", &boitSession);
			}
			else
			{
				SendBackMessage(GroupID, QQID, L"usage: #run language [/su] sourcecode\n ����#run /help �鿴��ϸ������Ϣ");
			}
			
			free(CompileCfg);
			return 0;
		}
	}




	//TODO:У��Ȩ��

	LONGLONG AllocCompileID = InterlockedIncrement64(&CompileID);
	//д��Դ�����ļ�

	WCHAR SourceCodeFile[MAX_PATH];

	GetPerUserDir(SourceCodeFile, QQID);
	PathAppendW(SourceCodeFile, L"Compile\\");

	WCHAR SourceFileName[16];

	swprintf_s(SourceFileName, _countof(SourceFileName), L"Temp%lld%ls", AllocCompileID, CompileCfg->SourceSuffix);

	PathAppendW(SourceCodeFile, SourceFileName);

	HANDLE hSourceFile = CreateFile(SourceCodeFile, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	PBYTE MultiByteStrCode = 0;

	BOOL bFileCreated = FALSE;

	//ȥ��ת����

	RemoveCQEscapeChar(CodeStr);
	__try
	{
		UINT CodePage = GetEncodeCodePage(CompileCfg->SourceEncode);
		
		int wcCodeLen = wcslen(CodeStr);
		int MultiByteStrLen = WideCharToMultiByte(CodePage, 0, CodeStr, wcCodeLen, 0, 0, 0, 0);
		MultiByteStrCode = malloc(MultiByteStrLen + 1);
		DWORD BytesWritten;
		ZeroMemory(MultiByteStrCode, MultiByteStrLen + 1);
		WideCharToMultiByte(CodePage, 0, CodeStr, wcCodeLen, MultiByteStrCode, MultiByteStrLen, 0, 0);

		WriteFile(hSourceFile, (LPCVOID)MultiByteStrCode, MultiByteStrLen, &BytesWritten, 0);
		if (BytesWritten != MultiByteStrLen)
		{
			__leave;
		}

		bFileCreated = TRUE;
	}
	__finally
	{
		CloseHandle(hSourceFile);
		if (MultiByteStrCode)free(MultiByteStrCode);

		if (!bFileCreated)
		{
			SendBackMessage(GroupID, QQID, L"д���ļ�ʱ���ִ���");
			free(CompileCfg);
			return 0;
		}
	}


	//����У��Ƴ�֮ǰ�Ŀ�ִ���ļ�
	switch (CompileCfg->Type)
	{
	case COMPILE_TYPE_COMPILE:
	{
		WCHAR ExecutableCodeFile[MAX_PATH];
		WCHAR ExecutableFileName[16];


		GetPerUserDir(ExecutableCodeFile, QQID);
		PathAppendW(ExecutableCodeFile, L"Compile\\");
		swprintf_s(ExecutableFileName, _countof(ExecutableFileName), L"Temp%lld.exe", AllocCompileID);
		PathAppendW(ExecutableCodeFile, ExecutableFileName);

		if (PathFileExistsW(ExecutableCodeFile))
		{
			DeleteFile(ExecutableCodeFile);
		}

		WCHAR CompileCmd[COMPILECMD_MAXLEN + 1];
		GetCompileCommand(CompileCmd, CompileCfg, AllocCompileID);

		WCHAR WorkDir[MAX_PATH];
		GetPerUserDir(WorkDir, QQID);
		PathAppendW(WorkDir, L"Compile\\");

		//TODO; ���Session����ʱϹд��
		pCOMPILE_SESSION CompileSession = malloc(sizeof(COMPILE_SESSION));
		ZeroMemory(CompileSession, sizeof(COMPILE_SESSION));
		CompileSession->boitSession.QQID = QQID;
		CompileSession->boitSession.GroupID = GroupID;
		if (AnonymousName) wcscpy_s(CompileSession->boitSession.AnonymousName, BOIT_MAX_NICKLEN, AnonymousName);
		CompileSession->StdOutBuffer = AllocVBuf();
		CompileSession->CompileCfg = CompileCfg;
		CompileSession->Command = pCmd;
		CompileSession->AllocCompileID = AllocCompileID;


		//TODO:��Щ���ƶ�������ʱд��
		if (CreateSimpleSandboxW(NULL,
			CompileCmd,
			WorkDir,
			10000000 * 10,		//10��
			256 * 1024 * 1024,	//256MB�ڴ�
			20 * 100,				//10% CPU
			FALSE,				//������Ȩ������
			(PBYTE)CompileSession,
			CompileSandboxCallback) == 0)
		{
			SendBackMessage(GroupID, QQID, L"Ϊ����������ɳ��ʱ��������");
			free(CompileCfg);
			free(CompileSession);
		}
	}
		break;
	case COMPILE_TYPE_SCRIPT:
	{
		WCHAR SandboxDir[MAX_PATH];
		WCHAR SandboxFile[MAX_PATH];
		InitSandboxDir(QQID, AllocCompileID, SourceCodeFile, SourceFileName, SandboxDir, SandboxFile);

		WCHAR CompileCmd[COMPILECMD_MAXLEN + 1];
		GetCompileCommand(CompileCmd, CompileCfg, AllocCompileID);

		
		StartRunSandbox(CompileCfg->Application[0] ? CompileCfg->Application : NULL, CompileCmd, SandboxDir, &boitSession, CompileCfg->OutputEncode);
		

	}
		
		break;
	}
	

	return 0;
}


int CompileSandboxCallback(pSANDBOX Sandbox, PBYTE pData, UINT Event, PBYTE StdOutData, DWORD DataLen)
{
	pCOMPILE_SESSION Session = (pCOMPILE_SESSION)pData;
	switch (Event)
	{
	case SANDBOX_EVTNT_PROCESS_ZERO:
	{
		DWORD CompileExitCode;
		GetExitCodeProcess(Sandbox->hProcess, &CompileExitCode);

		WCHAR ExecutableCodeFile[MAX_PATH];
		WCHAR ExecutableFileName[16];
		GetPerUserDir(ExecutableCodeFile, Session->boitSession.QQID);
		PathAppendW(ExecutableCodeFile, L"Compile\\");
		swprintf_s(ExecutableFileName, _countof(ExecutableFileName), L"Temp%lld.exe", Session->AllocCompileID);
		PathAppendW(ExecutableCodeFile, ExecutableFileName);

		if (Sandbox->ExitReason == SANDBOX_ER_EXIT && PathFileExistsW(ExecutableCodeFile)) //���Է���ֵ���м�飬�еı������ҷ���
		{
			//����ԭ�е�Ŀ¼
			//WCHAR SandboxFile[MAX_PATH] = { 0 };
			WCHAR SandboxDir[MAX_PATH];
			WCHAR SandboxFile[MAX_PATH];
			InitSandboxDir(Session->boitSession.QQID,
				Session->AllocCompileID,
				ExecutableCodeFile,
				ExecutableFileName,
				SandboxDir, SandboxFile);

			//WCHAR SandboxFile[MAX_PATH] = { 0 };
			//WCHAR SandboxDirName[16];
			//GetPerUserDir(SandboxFile, Session->boitSession.QQID);
			//PathAppendW(SandboxFile, L"Sandbox\\");
			//swprintf_s(SandboxDirName, _countof(SandboxDirName), L"Sandbox%lld", Session->AllocCompileID);

			//PathAppendW(SandboxFile, SandboxDirName);
			//SHFILEOPSTRUCTW FileOp;
			//if (PathIsDirectoryW(SandboxFile))
			//{
			//	int FileLen = wcslen(SandboxFile);
			//	SandboxFile[FileLen] = 0;
			//	SandboxFile[FileLen + 1] = 0;
			//	FileOp.fFlags = FOF_NOCONFIRMATION;
			//	FileOp.hNameMappings = NULL;
			//	FileOp.hwnd = NULL;
			//	FileOp.lpszProgressTitle = NULL;
			//	FileOp.pFrom = SandboxFile;
			//	FileOp.pTo = NULL;
			//	FileOp.wFunc = FO_DELETE;
			//	int iRet = SHFileOperationW(&FileOp);
			//}
			//CreateDirectoryW(SandboxFile, 0);
			////��Ѱ˼�������Ŀ¼����Ҳ�����ˣ��ù���ֱ������ӿ�ִ���ļ�����
			//PathAppendW(SandboxFile, ExecutableFileName);
			//
			//CopyFile(ExecutableCodeFile, SandboxFile, TRUE);

			StartRunSandbox(NULL, SandboxFile, SandboxDir, &(Session->boitSession), Session->CompileCfg->OutputEncode);
		}
		else
		{
			if (Sandbox->ExitReason == SANDBOX_ER_TIMEOUT)
			{
				SendBackMessage(Session->boitSession.GroupID, Session->boitSession.QQID, L"Oh... ���볬ʱ��");
			}
			else
			{
				SendBackMessage(Session->boitSession.GroupID, Session->boitSession.QQID, L"Opps... ����ʧ����");
			}
			WCHAR* wcStdout;

			UINT CodePage = GetEncodeCodePage(Session->CompileCfg->OutputEncode);

			int cchStdout = MultiByteToWideChar(CodePage, 0,
				Session->StdOutBuffer->Data,
				Session->StdOutBuffer->Length, 0, 0);

			wcStdout = malloc(sizeof(WCHAR) * (cchStdout + 1));
			MultiByteToWideChar(CodePage, 0,
				Session->StdOutBuffer->Data,
				Session->StdOutBuffer->Length, wcStdout, cchStdout);
			wcStdout[cchStdout] = 0;

			//ʵ�нض�
			if (cchStdout > BOIT_MAX_TEXTLEN)
			{
				wcStdout[BOIT_MAX_TEXTLEN] = 0;
			}

			WCHAR* ShowMessage = malloc(sizeof(WCHAR) * (cchStdout + 32));
			swprintf_s(ShowMessage, cchStdout + 32, L"�����������\n%ls\n����������ֵ��%ld", wcStdout, CompileExitCode);
			free(wcStdout);
			SendBackMessage(Session->boitSession.GroupID, Session->boitSession.QQID, ShowMessage);
			free(ShowMessage);
		}

		FreeSimpleSandbox(Sandbox);
		FreeVBuf(Session->StdOutBuffer);
		free(Session->CompileCfg);
		free(Session);
	}

	break;
	case SANDBOX_EVENT_STD_OUTPUT:
		if (Session->StdOutBuffer->Length + DataLen < BOIT_MAX_TEXTLEN * 4) // UTF8����������������һ���ַ�4�ֽ�
		{
			int OrgDataLen = Session->StdOutBuffer->Length;
			AdjustVBuf(Session->StdOutBuffer, Session->StdOutBuffer->Length + DataLen);
			memcpy(Session->StdOutBuffer->Data + OrgDataLen, StdOutData, DataLen);
		}
		//ʣ�µ�ֱ�Ӷ�����
		break;
	}

	return 0;
}


int RunSandboxCallback(pSANDBOX Sandbox, PBYTE pData, UINT Event, PBYTE StdOutData, DWORD DataLen)
{
	pRUN_SESSION Session = (pRUN_SESSION)pData;
	switch (Event)
	{
	case SANDBOX_EVTNT_PROCESS_ZERO:
	{
		if (Session->StdOutBuffer->Length == 0)
		{
			SendBackMessage(Session->boitSession.GroupID, Session->boitSession.QQID, L"����Ī�������");
		}
		else
		{
			UINT CodePage = GetEncodeCodePage(Session->Encode);
			WCHAR* wcStdout;
			int cchStdout = MultiByteToWideChar(CodePage, 0,
				Session->StdOutBuffer->Data,
				Session->StdOutBuffer->Length, 0, 0);
			wcStdout = malloc(sizeof(WCHAR) * (cchStdout + 1));
			MultiByteToWideChar(CodePage, 0,
				Session->StdOutBuffer->Data,
				Session->StdOutBuffer->Length, wcStdout, cchStdout);
			wcStdout[cchStdout] = 0;

			//ʵ�нض�
			if (cchStdout > BOIT_MAX_TEXTLEN)
			{
				wcStdout[BOIT_MAX_TEXTLEN] = 0;
			}

			SendBackMessage(Session->boitSession.GroupID, Session->boitSession.QQID, wcStdout);

			free(wcStdout);
		}
		FreeVBuf(Session->StdOutBuffer);
		free(Session);
	}
	break;
	case SANDBOX_EVENT_STD_OUTPUT:
	{
		if (Session->StdOutBuffer->Length + DataLen < BOIT_MAX_TEXTLEN * 4) // UTF8�����GB����������������һ���ַ�4�ֽ�
		{
			int OrgDataLen = Session->StdOutBuffer->Length;
			AdjustVBuf(Session->StdOutBuffer, Session->StdOutBuffer->Length + DataLen);
			memcpy(Session->StdOutBuffer->Data + OrgDataLen, StdOutData, DataLen);
		}
		//ʣ�µ�ֱ�Ӷ�����
		break;
	}
	break;
	}
}


BOOL InitSandboxDir(LONGLONG QQID, LONGLONG AllocCompileID, WCHAR* ToCopyFile,WCHAR * ToCopyFileName,WCHAR * SandboxDir,WCHAR * SandboxFile)
{
	//WCHAR SandboxFile[MAX_PATH] = { 0 };
	WCHAR SandboxDirName[16];
	GetPerUserDir(SandboxFile, QQID);
	PathAppendW(SandboxFile, L"Sandbox\\");
	swprintf_s(SandboxDirName, _countof(SandboxDirName), L"Sandbox%lld", AllocCompileID);

	PathAppendW(SandboxFile, SandboxDirName);

	wcscpy_s(SandboxDir, MAX_PATH, SandboxFile);

	SHFILEOPSTRUCTW FileOp;
	if (PathIsDirectoryW(SandboxFile))
	{
		int FileLen = wcslen(SandboxFile);
		SandboxFile[FileLen] = 0;
		SandboxFile[FileLen + 1] = 0;
		FileOp.fFlags = FOF_NOCONFIRMATION;
		FileOp.hNameMappings = NULL;
		FileOp.hwnd = NULL;
		FileOp.lpszProgressTitle = NULL;
		FileOp.pFrom = SandboxFile;
		FileOp.pTo = NULL;
		FileOp.wFunc = FO_DELETE;
		int iRet = SHFileOperationW(&FileOp);
	}
	CreateDirectoryW(SandboxFile, 0);
	//��Ѱ˼�������Ŀ¼����Ҳ�����ˣ��ù���ֱ������ӿ�ִ���ļ�����
	PathAppendW(SandboxFile, ToCopyFileName);

	CopyFile(ToCopyFile, SandboxFile, TRUE);


	return 0;
}


BOOL StartRunSandbox(WCHAR * Application,WCHAR* CommandLine,WCHAR * CuurentDir, pBOIT_SESSION boitSession, int Encode)
{
	pRUN_SESSION RunSession = malloc(sizeof(RUN_SESSION));
	RunSession->Encode = Encode;
	RunSession->boitSession.GroupID = boitSession->GroupID;
	RunSession->boitSession.QQID = boitSession->QQID;
	RunSession->StdOutBuffer = AllocVBuf();
	if (RunSession->boitSession.AnonymousName) wcscpy_s(RunSession->boitSession.AnonymousName, BOIT_MAX_NICKLEN, boitSession->AnonymousName);
	if (CreateSimpleSandboxW(Application, CommandLine, CuurentDir,
		10000000 * 10,		//10��
		512 * 1024 * 1024,	//512MB�ڴ�
		10 * 100,			//10% CPU
		TRUE, RunSession, RunSandboxCallback) == 0)
	{
		SendBackMessage(RunSession->boitSession.GroupID, RunSession->boitSession.QQID, L"Ϊ���򴴽�ɳ��ʱ��������");
		free(RunSession);
	}
	return 0;
}


int CmdEvent_run_Proc(pBOIT_COMMAND pCmd, UINT Event, PARAMA ParamA, PARAMB ParamB)
{
	switch (Event)
	{
	case EC_DIRINIT:
		//���ﴴ�����������ļ�
		PerCommandCfgCreateDirIfNExist(pCmd, L"Compiler\\");
		//�϶�û����
		//����˵���ļ�
		break;
	case EC_CMDLOAD:
		CompileID = 0;
		break;
	}
	return 0;
}


BOOL FindCompileConfig(pBOIT_COMMAND pCmd, WCHAR* LanguageName, int LanguageLen, WCHAR* ConfigSuffix, pCOMPILE_CFG CompileCfg)
{
	WCHAR CompilerCfgPath[MAX_PATH];
	GetPerCommandCfgDir(CompilerCfgPath, pCmd);
	PathAppendW(CompilerCfgPath, L"Compiler\\*");

	WIN32_FIND_DATAW FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	BOOL bNext = TRUE;

	BOOL bMatch = FALSE;
	for (hFind = FindFirstFileW(CompilerCfgPath, &FindData);
		hFind != INVALID_HANDLE_VALUE && wcslen(FindData.cFileName) > 0;
		bNext = FindNextFileW(hFind, &FindData))
	{
		if (!bNext)break;

		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (_wcsicmp(FindData.cFileName + wcslen(FindData.cFileName) - wcslen(ConfigSuffix), ConfigSuffix) == 0)
			{
				WCHAR CfgFilePath[MAX_PATH];
				GetPerCommandCfgDir(CfgFilePath, pCmd);
				PathAppendW(CfgFilePath, L"Compiler");
				PathAppendW(CfgFilePath, FindData.cFileName);
				if (MatchCompileConfig(CfgFilePath, CompileCfg, LanguageName, LanguageLen))
				{
					bMatch = TRUE;
					break;
				}
			}
		}

	}
	if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);


	return bMatch;
}


BOOL MatchCompileConfig(WCHAR* ConfigFileName, pCOMPILE_CFG CompileCfg, WCHAR* LanguageName, int LanguageLen)
{
	HANDLE hCfgFile = CreateFileW(ConfigFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hCfgFile == INVALID_HANDLE_VALUE) return FALSE;
	PBYTE pData = 0;
	WCHAR* pwData = 0;
	BOOL bMatch = FALSE;

	__try
	{
		DWORD FileSize = GetFileSize(hCfgFile, 0);
		pData = malloc(FileSize + 1);
		if (!pData)__leave;
		ZeroMemory(pData, FileSize + 1);
		DWORD BytesRead;
		if (ReadFile(hCfgFile, pData, FileSize, &BytesRead, NULL) == FALSE)__leave;
		if (BytesRead != FileSize)
		{
			__leave;
		}

		int wLen = MultiByteToWideChar(CP_ACP, 0, pData, FileSize, 0, 0);

		pwData = (WCHAR*)malloc(sizeof(WCHAR) * (wLen + 1));
		ZeroMemory(pwData, sizeof(WCHAR) * (wLen + 1));
		MultiByteToWideChar(CP_ACP, 0, pData, FileSize, pwData, wLen);


		//�������
		CompileCfg->Application[0] = 0;
		CompileCfg->Command[0] = 0;
		CompileCfg->OutputEncode = 0;
		CompileCfg->SourceEncode = 0;
		CompileCfg->SourceSuffix[0] = 0;
		CompileCfg->Name[0] = 0;
		CompileCfg->Type = COMPILE_TYPE_NULL;

		//�����ļ�
		WCHAR* pwParse = pwData;

		BOOL NameFound = 0, TypeFound = 0, SuffixFound = 0, CommandFound = 0;

		while (wLen > 0)
		{
			int LineFeedLen = GetLineFeedLen(pwParse);
			wLen -= LineFeedLen;
			pwParse += LineFeedLen;

			int LineLen = GetLineLen(pwParse);
			//������
			if (LineLen)
			{
				if (pwParse[0] != L'#')//ע��
				{
					WCHAR FieldNameList[][16] = { L"Name",L"Type",L"Suffix",L"Command",L"SourceEncode",L"OutputEncode", L"Application" };

					WCHAR* LineParse = pwParse;
					for (int i = 0; i < _countof(FieldNameList); i++)
					{
						if (_wcsnicmp(FieldNameList[i], LineParse, wcslen(FieldNameList[i])) == 0)
						{
							LineParse += wcslen(FieldNameList[i]);
							LineParse += GetLineSpaceLen(LineParse);
							if (LineParse[0] != L'=')
							{
								break;
							}
							LineParse++;
							LineParse += GetLineSpaceLen(LineParse);
							switch (i)
							{
							case 0:
								//����Name
							{
								int LangLen = GetCmdParamLen(LineParse);
								if ((!LangLen) || LangLen>COMPILE_MAXNAME)break;
								wcsncpy_s(CompileCfg->Name, COMPILE_MAXNAME, LineParse, LangLen);
							}
							if (LanguageName && LanguageLen)
							{
								while (1)
								{
									int LangLen = GetCmdParamLen(LineParse);
									if ((!LangLen) || LangLen > COMPILE_MAXNAME)break;

									if (LangLen == LanguageLen && (_wcsnicmp(LineParse, LanguageName, LangLen) == 0))
									{
										NameFound = TRUE; //ƥ��ɹ�
									}
									LineParse += LangLen;
									LineParse += GetLineSpaceLen(LineParse);
								}
							}
							break;
							case 1:
								//����Type
								if (_wcsnicmp(LineParse, L"Compile", wcslen(L"Compile")) == 0)
								{
									CompileCfg->Type = COMPILE_TYPE_COMPILE;
									TypeFound = TRUE;
								}
								else if (_wcsnicmp(LineParse, L"Script", wcslen(L"Script")) == 0)
								{
									CompileCfg->Type = COMPILE_TYPE_SCRIPT;
									TypeFound = TRUE;
								}
								break;
							case 2:
								//����Suffix
							{
								int SuffixLen = GetCmdParamLen(LineParse);
								if (SuffixLen && SuffixLen < COMPILE_MAXSUFFIX)
								{
									wcsncpy_s(CompileCfg->SourceSuffix, COMPILE_MAXSUFFIX, LineParse, SuffixLen);
									SuffixFound = TRUE;
								}
							}
							break;
							case 3:
							{
								//����Command
								int CommandLen = GetLineLen(LineParse);
								if (CommandLen && CommandLen < COMPILECMD_MAXLEN)
								{
									wcsncpy_s(CompileCfg->Command, COMPILECMD_MAXLEN, LineParse, CommandLen);
									CommandFound = TRUE;
								}
							}
							break;
							case 4:
							{
								if (_wcsnicmp(LineParse, L"GB18030", wcslen(L"GB18030")) == 0)
								{
									CompileCfg->SourceEncode = COMPILE_ENCODE_GB18030;
								}
								else if (_wcsnicmp(LineParse, L"UTF8", wcslen(L"UTF8")) == 0)
								{
									CompileCfg->SourceEncode = COMPILE_ENCODE_UTF8;
								}
								else if (_wcsnicmp(LineParse, L"ANSI", wcslen(L"ANSI")) == 0)
								{
									CompileCfg->SourceEncode = COMPILE_ENCODE_ANSI;
								}
								break;
							}
							break;
							case 5:
							{
								if (_wcsnicmp(LineParse, L"GB18030", wcslen(L"GB18030")) == 0)
								{
									CompileCfg->OutputEncode = COMPILE_ENCODE_GB18030;
								}
								else if (_wcsnicmp(LineParse, L"UTF8", wcslen(L"UTF8")) == 0)
								{
									CompileCfg->OutputEncode = COMPILE_ENCODE_UTF8;
								}
								else if (_wcsnicmp(LineParse, L"ANSI", wcslen(L"ANSI")) == 0)
								{
									CompileCfg->OutputEncode = COMPILE_ENCODE_ANSI;
								}
								break;
							}
							break;
							case 6:
							{
								//����Command
								int ApplicationLen = GetLineLen(LineParse);
								if (ApplicationLen && ApplicationLen < MAX_PATH)
								{
									wcsncpy_s(CompileCfg->Application, MAX_PATH, LineParse, ApplicationLen);
								}
							}
							break;
							}

						}
					}


				}
			}

			pwParse += LineLen;
			wLen -= LineLen;
		}


		if (TypeFound && SuffixFound && CommandFound)
		{
			if (LanguageName)
			{
				if (NameFound)
				{
					bMatch = TRUE;
				}
			}
			else
			{
				if (CompileCfg->Name[0])
				{
					bMatch = TRUE;
				}
			}
		}

	}
	__finally
	{
		if (hCfgFile)
		{
			CloseHandle(hCfgFile);
		}
		if (pData)
		{
			free(pData);
		}
		if (pwData)
		{
			free(pwData);
		}
	}
	return bMatch;
}


BOOL ShowSupportLanguageInfo(pBOIT_COMMAND pCmd, WCHAR* ConfigSuffix,pBOIT_SESSION boitSession)
{
	COMPILE_CFG CompileCfg;
	WCHAR CompilerCfgPath[MAX_PATH];
	GetPerCommandCfgDir(CompilerCfgPath, pCmd);
	PathAppendW(CompilerCfgPath, L"Compiler\\*");

	WIN32_FIND_DATAW FindData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	BOOL bNext = TRUE;

	BOOL bMatch = FALSE;

	WCHAR ReplyMessage[256] = L"֧���������£�\n";

	for (hFind = FindFirstFileW(CompilerCfgPath, &FindData);
		hFind != INVALID_HANDLE_VALUE && wcslen(FindData.cFileName) > 0;
		bNext = FindNextFileW(hFind, &FindData))
	{
		if (!bNext)break;

		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (_wcsicmp(FindData.cFileName + wcslen(FindData.cFileName) - wcslen(ConfigSuffix), ConfigSuffix) == 0)
			{
				WCHAR CfgFilePath[MAX_PATH];
				GetPerCommandCfgDir(CfgFilePath, pCmd);
				PathAppendW(CfgFilePath, L"Compiler");
				PathAppendW(CfgFilePath, FindData.cFileName);
				if (MatchCompileConfig(CfgFilePath, &CompileCfg, 0, 0))
				{
					wcscat_s(ReplyMessage, _countof(ReplyMessage), CompileCfg.Name);
					wcscat_s(ReplyMessage, _countof(ReplyMessage), L"  ");
					
				}
			}
		}

	}
	if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind);

	SendBackMessage(boitSession->GroupID, boitSession->QQID, ReplyMessage);
	return 0;
}


int GetLineLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] == L'\r' ||
			String[i] == L'\n' ||
			String[i] == 0)
		{
			break;
		}
	}
	return i;
}


int GetLineSpaceLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] != L' ' &&
			String[i] != L'\t')
		{
			break;
		}
	}
	return i;
}


int GetLineFeedLen(WCHAR* String)
{
	int i;
	for (i = 0;; i++)
	{
		if (String[i] != L' ' &&
			String[i] != L'\r' &&
			String[i] != L'\n')
		{
			break;
		}
	}
	return i;
}


BOOL GetCompileCommand(WCHAR* CommandBuffer, pCOMPILE_CFG CompileCfg, LONGLONG AllocCompileID)
{
	int len = wcslen(CompileCfg->Command);
	WCHAR SourceFile[16];
	WCHAR ExecutableFile[16];

	swprintf_s(SourceFile, _countof(SourceFile), L"Temp%lld%ls", AllocCompileID, CompileCfg->SourceSuffix);
	swprintf_s(ExecutableFile, _countof(ExecutableFile), L"Temp%lld.exe", AllocCompileID);
	int j = 0;
	for (int i = 0; i < len; )
	{
		if (j >= COMPILECMD_MAXLEN - 1)
		{
			break;
		}
		if (_wcsnicmp(CompileCfg->Command + i, L"%In", wcslen(L"%In")) == 0)
		{
			wcscpy_s(CommandBuffer + j, COMPILECMD_MAXLEN - j, SourceFile);
			i += 3;
			j += wcslen(SourceFile);
		}
		else if (_wcsnicmp(CompileCfg->Command + i, L"%Out", wcslen(L"%Out")) == 0)
		{
			wcscpy_s(CommandBuffer + j, COMPILECMD_MAXLEN - j, ExecutableFile);
			i += 4;
			j += wcslen(ExecutableFile);
		}
		else if (_wcsnicmp(CompileCfg->Command + i, L"%%", wcslen("%%")) == 0)
		{
			CommandBuffer[j++] = CompileCfg->Command[i++];
			i++;
		}
		else
		{
			CommandBuffer[j++] = CompileCfg->Command[i++];
		}
	}
	CommandBuffer[j++] = 0;
	return 0;
}


UINT GetEncodeCodePage(int Compile_Encode)
{
	UINT CodePage = 0;
	switch (Compile_Encode)
	{
	case COMPILE_ENCODE_UTF8:
		CodePage = CP_UTF8;
		break;
	case COMPILE_ENCODE_GB18030:
		CodePage = 54936; //���  https://docs.microsoft.com/zh-cn/windows/win32/intl/code-page-identifiers
		break;
	case COMPILE_ENCODE_ANSI:
	default:
		CodePage = CP_ACP;
		break;
	}
	return CodePage;
}