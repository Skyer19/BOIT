#include<Windows.h>
#include<process.h>
#include<sddl.h>
#include "SimpleSandbox.h"

#define COMPPORT_MAX_THREAD 16



HANDLE hJobObjPortThread[COMPPORT_MAX_THREAD];//�߳̾����
HANDLE hPipeIOPortThread[COMPPORT_MAX_THREAD];//�߳̾����

int g_JobObjCompThreadNum;
int g_PipeIOCompThreadNum;
SRWLOCK SandboxListLock;

unsigned __stdcall JobObjCompletionPort(LPVOID Args);
unsigned __stdcall PipeIOCompletionPort(LPVOID Args);
unsigned __stdcall TerminateJobTimerThread(LPVOID Args);

#define JOB_OBJECT_MSG_MY_CLEANUP 128 //Ϲд�����֣���JobObj��IOCP�����Ϣ�Ŵ��OK


HANDLE JobObjCompPort = 0;
HANDLE PipeIOCompPort = 0;


LONGLONG AllocSandboxID = 0;


//�̼߳䴫������
HANDLE TimerThread;

SRWLOCK PassArgTimerLock;//������*************
LONGLONG g_ArgTimeLimit; // In
LONGLONG g_ArgSandboxID; // In
HANDLE hWaitableTimer;   // Out
//********************************************
HANDLE EventPassArgStart;
HANDLE EventPassArgEnd;


HANDLE SandboxCleaning;


BOOL CreateLimitedProcessW(
	_In_opt_ WCHAR lpApplicationName[],
	_Inout_opt_ WCHAR lpCommandLine[],
	_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ BOOL bInheritHandles,
	_In_ DWORD dwCreationFlags,
	_In_opt_ LPVOID lpEnvironment,
	_In_opt_ WCHAR lpCurrentDirectory[],
	_In_ LPSTARTUPINFOW lpStartupInfo,
	_Out_ LPPROCESS_INFORMATION lpProcessInformation);

void __stdcall TerminateJobTimerRoutine(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
);


BOOL CreateOverlappedNamedPipePair(PHANDLE hReadPipe, PHANDLE hWritePipe, DWORD nSize);



int InitializeSandbox(int JobObjCompThreadNum, int PipeIOCompThreadNum)
{
	//����һ����ɶ˿ڣ������̴߳���
	//���������̷ֱ߳�����Pipe Read/Write
	InitializeSRWLock(&SandboxListLock);


	RootSandbox = 0;

	//ָʾ�Ƿ�ɳ�����ڹر�
	SandboxCleaning = CreateEvent(0, TRUE, 0, 0);


	if (JobObjCompThreadNum == 0 || PipeIOCompThreadNum == 0)
	{
		SYSTEM_INFO SysInfo;//����ȡ��CPU��������Ϣ
		GetSystemInfo(&SysInfo);
		JobObjCompThreadNum = (JobObjCompThreadNum == 0) ? SysInfo.dwNumberOfProcessors : JobObjCompThreadNum;
		PipeIOCompThreadNum = (PipeIOCompThreadNum == 0) ? SysInfo.dwNumberOfProcessors : PipeIOCompThreadNum;
	}
	g_JobObjCompThreadNum = JobObjCompThreadNum;
	g_PipeIOCompThreadNum = PipeIOCompThreadNum;

	if (JobObjCompThreadNum > COMPPORT_MAX_THREAD || PipeIOCompThreadNum > COMPPORT_MAX_THREAD)
	{
		return 0;
	}

	JobObjCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, JobObjCompThreadNum);

	PipeIOCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, PipeIOCompThreadNum);

	for (int i = 0; i < (int)JobObjCompThreadNum; i++)
	{
		hJobObjPortThread[i] = (HANDLE)_beginthreadex(NULL, 0, JobObjCompletionPort, (LPVOID)0, 0, NULL);
	}
	for (int i = 0; i < (int)PipeIOCompThreadNum; i++)
	{
		hPipeIOPortThread[i] = (HANDLE)_beginthreadex(NULL, 0, PipeIOCompletionPort, (LPVOID)0, 0, NULL);
	}


	//��ʼ����ʱ���߳�
	InitializeSRWLock(&PassArgTimerLock);
	EventPassArgStart = CreateEvent(0, 0, 0, 0);
	EventPassArgEnd = CreateEvent(0, 0, 0, 0);

	TimerThread = (HANDLE)_beginthreadex(NULL, 0, TerminateJobTimerThread, (LPVOID)0, 0, NULL);
	/*����Ҫ��
		ά��SESSION��Job֮��Ĺ�ϵ��SESSION��IO�߳�֮��Ĺ�ϵ
		Job����ɴ���һ��Key (��Assoc��ʱ��ָ��)
		IO�߳̿�������Overlapped�����Խ��д���
		WriteFileEx��һ���Եġ�ֻҪ����ɶ˿�У��һ���Ƿ��������ͺ��ˡ������ų���û������̾����ˣ����ԣ�
		ReadFileEx���ܱȽϷ���*/
	return 0;
}


int FinalizeSandbox()
{
	SetEvent(SandboxCleaning);//ɳ�йر�ing
	
	AcquireSRWLockExclusive(&SandboxListLock);

	if (RootSandbox)
	{
		for (pSANDBOX_CHAIN pList = RootSandbox; pList; pList = pList->NextSandbox)
		{
			TerminateJobObject(pList->hJob, 1); //ȫ���ս�
		}
	}
	ReleaseSRWLockExclusive(&SandboxListLock);

	for (int i = 0; i < g_JobObjCompThreadNum; i++)
	{
		//PostQueuedCompletionStatusҪ�󴫵�һ��Overlapped�ṹ��������Ĳ��봫������
		//����������Ǵ���һ�°���һUB���أ�����ɶ˿��Ǳ����ͷ�
		LPOVERLAPPED lpOverlapped = malloc(sizeof(OVERLAPPED));
		ZeroMemory(lpOverlapped, sizeof(OVERLAPPED));

		PostQueuedCompletionStatus(JobObjCompPort, JOB_OBJECT_MSG_MY_CLEANUP, 0, lpOverlapped);
	}

	for (int i = 0; i < g_PipeIOCompThreadNum; i++)
	{
		PipeIOSendClose(PipeIOCompPort);
	}

	WaitForMultipleObjects(g_JobObjCompThreadNum, hJobObjPortThread, 1, INFINITE);
	WaitForMultipleObjects(g_PipeIOCompThreadNum, hPipeIOPortThread, 1, INFINITE);
	WaitForSingleObject(TimerThread, INFINITE);

	CloseHandle(EventPassArgStart);
	CloseHandle(EventPassArgEnd);

	CloseHandle(JobObjCompPort);
	CloseHandle(PipeIOCompPort);

	CloseHandle(SandboxCleaning);
	return 0;
}


pSANDBOX CreateSimpleSandboxW(WCHAR* ApplicationName,
	WCHAR* CommandLine,
	WCHAR* CurrentDirectory,
	long long TotUserTimeLimit,		//����ɳ�е�ʱ�����ƣ�msdn��˵��100����Ϊ��λ������֣��Ҿ�����10000*�룩��Ϊ-1������ʱ������
	long long TotMemoryLimit,		//����ɳ�е��ڴ����ƣ�ByteΪ��λ����Ϊ-1�������ڴ�����
	int CpuRateLimit,				//CPU���ƣ�1-10000����10000��ζ�ſ�����100%��CPU����������0����Ϊ-1����������
	BOOL bLimitPrivileges,			//����Ȩ��
	PBYTE pExternalData,			//��������Ϣ
	SANDBOX_CALLBACK CallbackFunc	//�ص�����
	)
{
	BOOL bSuccess = FALSE;
	STARTUPINFOW SysInfo = { 0 };
	PROCESS_INFORMATION ProcInfo = { 0 };
	HANDLE hJob = 0;
	HANDLE hPipeInRead = 0, hPipeInWrite = 0;
	HANDLE hPipeOutRead = 0, hPipeOutWrite = 0;

	pSANDBOX Sandbox = malloc(sizeof(SANDBOX));
	if (!Sandbox) return 0;

	ZeroMemory(Sandbox, sizeof(SANDBOX));
	__try
	{
		//����ID
		Sandbox->SandboxID = InterlockedIncrement64(&AllocSandboxID);
		Sandbox->pExternalData = pExternalData;
		Sandbox->SandboxCallback = CallbackFunc;


		//������ҵ
		hJob = CreateJobObject(0, 0);
		if (!hJob)__leave;

		{
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION ExtendLimit = { 0 };
			if (TotUserTimeLimit != -1)
			{
				ExtendLimit.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = TotUserTimeLimit;
				ExtendLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
			}
			if (TotMemoryLimit != -1)
			{
				ExtendLimit.JobMemoryLimit = (SIZE_T)TotMemoryLimit;
				ExtendLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
			}
			ExtendLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			bSuccess = SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &ExtendLimit, sizeof(ExtendLimit));
			if (!bSuccess)__leave;
			bSuccess = FALSE;


			JOBOBJECT_BASIC_UI_RESTRICTIONS UIRestriction = { 0 };
			UIRestriction.UIRestrictionsClass = JOB_OBJECT_UILIMIT_NONE;
			UIRestriction.UIRestrictionsClass |= JOB_OBJECT_UILIMIT_EXITWINDOWS | //��ֹ����ע�����ػ���������Ͽ�ϵͳ��Դ
				JOB_OBJECT_UILIMIT_READCLIPBOARD | //��ֹ���̶�ȡ�������е����ݡ�
				JOB_OBJECT_UILIMIT_WRITECLIPBOARD | //��ֹ��������������е����ݡ�
				JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS | //��ֹ���̸���ϵͳ������
				JOB_OBJECT_UILIMIT_DISPLAYSETTINGS | //���̸�����ʾ���á�
				JOB_OBJECT_UILIMIT_GLOBALATOMS | //Ϊ��ҵָ����ר�е�ȫ��ԭ�ӱ����޶���ҵ�еĽ���ֻ�ܷ��ʴ���ҵ�ı�
				JOB_OBJECT_UILIMIT_DESKTOP | //��ֹ���̴������л����档
				JOB_OBJECT_UILIMIT_HANDLES; //��ֹ��ҵ�еĽ���ʹ��ͬһ����ҵ�ⲿ�Ľ������������û�����( ��HWND) ��
			bSuccess = SetInformationJobObject(hJob, JobObjectBasicUIRestrictions, &UIRestriction, sizeof(UIRestriction));
			if (!bSuccess)__leave;
			bSuccess = FALSE;

			if (CpuRateLimit != -1)
			{
				JOBOBJECT_CPU_RATE_CONTROL_INFORMATION CpuControl = { 0 };
				CpuControl.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP;
				CpuControl.CpuRate = CpuRateLimit;
				bSuccess = SetInformationJobObject(hJob, JobObjectCpuRateControlInformation, &CpuControl, sizeof(CpuControl));
				if (!bSuccess)__leave;
				bSuccess = FALSE;
			}

			JOBOBJECT_ASSOCIATE_COMPLETION_PORT JobCompletionPort = { 0 };
			JobCompletionPort.CompletionPort = JobObjCompPort;
			JobCompletionPort.CompletionKey = Sandbox;
			SetInformationJobObject(hJob, JobObjectAssociateCompletionPortInformation, &JobCompletionPort, sizeof(JobCompletionPort));
			/*�����������Job��IOCP��CreateSimpleSandbox������Ӧ���ṩһ���ص���������֪ͨ�¼�������
				Ȼ�����Sandbox���뵽�������Ա�ö�٣�����
				д FreeSimpleSandboxW�����̶�������ɳ��������Ȼ�������ֶ��ͷ�
				Terminate����job���ͷŶ��󣬴��������Ƴ�*/
		}


		SysInfo.cb = sizeof(STARTUPINFO);


		
		//�����ܵ�
		{
			
			CreateOverlappedNamedPipePair(&hPipeInRead, &hPipeInWrite, PIPEIO_BUFSIZE);
			CreateOverlappedNamedPipePair(&hPipeOutRead, &hPipeOutWrite, PIPEIO_BUFSIZE);
			SysInfo.hStdInput = hPipeInRead;
			SysInfo.hStdOutput = SysInfo.hStdError = hPipeOutWrite;
			SysInfo.dwFlags |= STARTF_USESTDHANDLES;

			CreateIoCompletionPort(hPipeInWrite, PipeIOCompPort, (ULONG_PTR)Sandbox, 0);
			CreateIoCompletionPort(hPipeOutRead, PipeIOCompPort, (ULONG_PTR)Sandbox, 0);
		}

		
		SetHandleInformation(hPipeInRead, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		BOOL bCreated;
		if (bLimitPrivileges)
		{
			bCreated = CreateLimitedProcessW(ApplicationName, CommandLine, 0, 0, TRUE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, 0, CurrentDirectory, &SysInfo, &ProcInfo);
		}
		else
		{
			bCreated = CreateProcessW(ApplicationName, CommandLine, 0, 0, TRUE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, 0, CurrentDirectory, &SysInfo, &ProcInfo);
		}
		SetHandleInformation(hPipeInRead, HANDLE_FLAG_INHERIT, 0);
		SetHandleInformation(hPipeOutWrite, HANDLE_FLAG_INHERIT, 0);

		CloseHandle(hPipeInRead);
		CloseHandle(hPipeOutWrite);

		if (!bCreated)
		{
			__leave;
		}
		Sandbox->hPipeInWrite = hPipeInWrite;
		Sandbox->hPipeOutRead = hPipeOutRead;

		if (!AssignProcessToJobObject(hJob, ProcInfo.hProcess))
		{
			__leave;
		}

		if (!bCreated)
		{
			__leave;
		}
		Sandbox->hProcess = ProcInfo.hProcess;
		Sandbox->hJob = hJob;




		//��������
		AcquireSRWLockExclusive(&SandboxListLock);

		pSANDBOX_CHAIN OrgHead = RootSandbox;
		RootSandbox = Sandbox;
		Sandbox->NextSandbox = OrgHead;
		if (OrgHead)
		{
			OrgHead->LastSandbox = Sandbox;
		}

		ReleaseSRWLockExclusive(&SandboxListLock);

		//���ü�ʱ��
		if (TotUserTimeLimit != -1)
		{
			AcquireSRWLockExclusive(&PassArgTimerLock);
			g_ArgTimeLimit = TotUserTimeLimit; // In
			g_ArgSandboxID = Sandbox->SandboxID; // In
			SignalObjectAndWait(EventPassArgStart, EventPassArgEnd, INFINITE, FALSE);
			Sandbox->hWaitableTimer = hWaitableTimer;   // Out
			ReleaseSRWLockExclusive(&PassArgTimerLock);

			if (!Sandbox->hWaitableTimer)
			{
				TerminateJobObject(Sandbox->hJob, 1);
				Sandbox->ExitReason = SANDBOX_ER_ERROR;
				//����leave�ˣ���һͷJob��IOCP��Ҫ���ꡣ����ɾ�ˡ��������ﵱ�ɹ�����
			}
		}
		

		//��ȡPipe
		PipeIORead(hPipeOutRead);

		ResumeThread(ProcInfo.hThread);
		bSuccess = TRUE;
	}
	__finally
	{
		if (!bSuccess)
		{
			if (ProcInfo.hProcess)
			{
				TerminateProcess(ProcInfo.hProcess, 1);
				CloseHandle(ProcInfo.hProcess);
			}
			if (hJob)
			{
				CloseHandle(hJob);
			}

			if (Sandbox)
			{
				free(Sandbox);
				Sandbox = NULL;
			}

		}
		if (ProcInfo.hThread)
		{
			CloseHandle(ProcInfo.hThread);
		}
	}

	return Sandbox;
}

int FreeSimpleSandbox(pSANDBOX Sandbox)
{
	//�Ƴ�
	AcquireSRWLockExclusive(&SandboxListLock);

	if (Sandbox->LastSandbox)
	{
		Sandbox->LastSandbox->NextSandbox = Sandbox->NextSandbox;
	}
	else
	{
		RootSandbox = Sandbox->NextSandbox;
	}

	if (Sandbox->NextSandbox)
	{
		Sandbox->NextSandbox->LastSandbox = Sandbox->LastSandbox;
	}
	ReleaseSRWLockExclusive(&SandboxListLock);

	TerminateJobObject(Sandbox->hJob, 1);
	if (Sandbox->hPipeInWrite) CloseHandle(Sandbox->hPipeInWrite);
	if (Sandbox->hPipeOutRead) CloseHandle(Sandbox->hPipeOutRead);
	CloseHandle(Sandbox->hProcess);
	CloseHandle(Sandbox->hJob);
	return 0;
}

unsigned __stdcall JobObjCompletionPort(LPVOID Args)
{
	while (1)
	{
		DWORD ByteTrans;
		LPOVERLAPPED lpOverlapped;
		pSANDBOX  pSandbox;
		GetQueuedCompletionStatus(JobObjCompPort, &ByteTrans, (PULONG_PTR)(&pSandbox), (LPOVERLAPPED*)&lpOverlapped, INFINITE);
		
		switch (ByteTrans)
		{
		case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
			//��������IO���������ڸ�IO�����������½������
			//�����ڼ����������������������Sandbox���󣬵���һ��������֮ʱ�����ʷǷ���ַ
			//TODO: ��ƽ������map����������Ȼ����SandboxID��У������Ƿ����
			//�����Ҳ���Ҫ�ġ�����Ȼ���������Ӧ�þ������ǽ�ȥ��
			//��������Sleep��Σ���ִ��Ȩ���������߳�
			for (int i = 0; i < 16; i++)Sleep(0);
			SANDBOX_CALLBACK Callback;
			PBYTE pData;
			AcquireSRWLockShared(&SandboxListLock);
				Callback = pSandbox->SandboxCallback;
				pData = pSandbox->pExternalData;
			ReleaseSRWLockShared(&SandboxListLock);
			if (Callback)
			{
				Callback(pSandbox, pData, SANDBOX_EVTNT_PROCESS_ZERO, 0, 0);
			}
			
			break;
		case JOB_OBJECT_MSG_EXIT_PROCESS:
			if (pSandbox->ExitReason == SANDBOX_ER_RUNNING)
			{
				pSandbox->ExitReason = SANDBOX_ER_EXIT;
			}
			break;
		case JOB_OBJECT_MSG_END_OF_JOB_TIME:
			if (pSandbox->ExitReason == SANDBOX_ER_RUNNING)
			{
				pSandbox->ExitReason = SANDBOX_ER_TIMEOUT;
			}
			break;
		case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
			if (pSandbox->ExitReason == SANDBOX_ER_RUNNING)
			{
				pSandbox->ExitReason = SANDBOX_ER_ABNORMAL;
			}
			break;
		case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
			//�ڴ泬�����ޣ���¼����
			pSandbox->bMemoryExceed = TRUE;
			break;
		case JOB_OBJECT_MSG_MY_CLEANUP:
			free(lpOverlapped);//�ͷ������Overlapped�ṹ
			return 0;
		}
	}

}


unsigned __stdcall PipeIOCompletionPort(LPVOID Args)
{
	while (1)
	{
		DWORD ByteTrans;
		pPIPEIO_PACK PipeIOPack;
		pSANDBOX  pSandbox;
		BOOL bResult = GetQueuedCompletionStatus(PipeIOCompPort, &ByteTrans, (PULONG_PTR)(&pSandbox), (LPOVERLAPPED*)&PipeIOPack, INFINITE);
		if (bResult)
		{
			//�ɹ�����
			switch (PipeIOPack->PackMode)
			{
			case PACKMODE_READ:
			{
				SANDBOX_CALLBACK Callback;
				PBYTE pData;
				AcquireSRWLockShared(&SandboxListLock);
					Callback = pSandbox->SandboxCallback;
					pData = pSandbox->pExternalData;
				ReleaseSRWLockShared(&SandboxListLock);
				if (Callback)
				{
					Callback(pSandbox, pData, SANDBOX_EVENT_STD_OUTPUT, PipeIOPack->pData, ByteTrans);
				}

				free(PipeIOPack->pData);
				free(PipeIOPack);

				PipeIORead(pSandbox->hPipeOutRead);
			}
				break;
			case PACKMODE_WRITE:
				free(PipeIOPack);
				break;
			case PACKMODE_CLOSE://����
				free(PipeIOPack);
				return 0;
			}
		}
		else
		{
			DWORD dwErr = GetLastError();
			switch (dwErr)
			{
			case ERROR_ABANDONED_WAIT_0:
				return 0;//��ɶ˿ڹر��ˣ��������ɶҲ�𲻳���
			default:
				//��������IO����
				break;
			}
		}
	}

}


unsigned __stdcall TerminateJobTimerThread(LPVOID Args)
{
	int TimerCount = 0;
	while (1)
	{
		HANDLE WaitHandleList[2] = { SandboxCleaning ,EventPassArgStart };

		DWORD dwResult = WaitForMultipleObjectsEx(2, WaitHandleList, 0, INFINITE, TRUE);

		switch (dwResult)
		{
		case WAIT_OBJECT_0:
			//Cleaning

			return 0; //�˳��߳�
		case WAIT_OBJECT_0+1:
			//���ο�ʼ
		{
			HANDLE hTimer = CreateWaitableTimer(0, 0, 0);
			LARGE_INTEGER LargeInteger;
			LargeInteger.QuadPart = -g_ArgTimeLimit;
			if (hTimer)
			{
				SetWaitableTimer(hTimer, &LargeInteger, 0, TerminateJobTimerRoutine, (LPVOID)g_ArgSandboxID, FALSE);
			}
			hWaitableTimer = hTimer;
			SetEvent(EventPassArgEnd);
		}
			break;
		}
	}
	
	return 0;
}

void __stdcall TerminateJobTimerRoutine(
	_In_opt_ LPVOID lpArgToCompletionRoutine,
	_In_     DWORD dwTimerLowValue,
	_In_     DWORD dwTimerHighValue
	)
{
	AcquireSRWLockShared(&SandboxListLock);

	if (RootSandbox)
	{
		for (pSANDBOX_CHAIN pList = RootSandbox; pList; pList = pList->NextSandbox)
		{
			if (pList->SandboxID == (LONGLONG)lpArgToCompletionRoutine)
			{
				TerminateJobObject(pList->hJob, 1); //time over!
				if (pList->ExitReason == SANDBOX_ER_RUNNING)
				{
					pList->ExitReason = SANDBOX_ER_TIMEOUT;
				}
				break;
			}
		}
	}
	
	ReleaseSRWLockShared(&SandboxListLock);
	return;
}



BOOL CreateLimitedProcessW(
	 WCHAR lpApplicationName[],
	 WCHAR lpCommandLine[],
	 LPSECURITY_ATTRIBUTES lpProcessAttributes,
	 LPSECURITY_ATTRIBUTES lpThreadAttributes,
	 BOOL bInheritHandles,
	 DWORD dwCreationFlags,
	 LPVOID lpEnvironment,
	 WCHAR lpCurrentDirectory[],
	 LPSTARTUPINFOW lpStartupInfo,
	 LPPROCESS_INFORMATION lpProcessInformation)
{
	BOOL bSuccess = FALSE;
	BOOL bRet = FALSE;
	HANDLE hToken = 0;
	HANDLE hNewToken = 0;

	PTCHAR szIntegritySid = TEXT("S-1-16-4096"); // �������� SID 
	PSID pIntegritySid = NULL;
	TOKEN_MANDATORY_LABEL TIL = { 0 };


	__try
	{
		bSuccess = OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED,
			&hToken);
		if (!bSuccess) __leave;

		bSuccess = DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
			SecurityAnonymous, TokenPrimary, &hNewToken);//AnonymousӦ������͵���
		if (!bSuccess)__leave;

		bSuccess = ConvertStringSidToSid(szIntegritySid, &pIntegritySid);
		if (!bSuccess)__leave;

		TIL.Label.Attributes = SE_GROUP_INTEGRITY;
		TIL.Label.Sid = pIntegritySid;

		// ���ý��������Լ��� 
		bSuccess = SetTokenInformation(hNewToken, TokenIntegrityLevel, &TIL,
			sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid));
		if (!bSuccess)__leave;

		// ���ý��̵� UI Ȩ�޼��� 
		//bSuccess = SetTokenInformation(hNewToken, TokenIntegrityLevelDesktop,
		//	&TIL, sizeof(TOKEN_MANDATORY_LABEL) + RtlLengthSid(pIntegritySid));

		// �Ե������Դ����½��� 
		bRet = CreateProcessAsUserW(hNewToken,
			lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation
		);
	}
	__finally
	{
		if (hToken)CloseHandle(hToken);
		if (hNewToken)CloseHandle(hNewToken);
	}

	return bRet;
}


BOOL CreateOverlappedNamedPipePair(PHANDLE hReadPipe, PHANDLE hWritePipe, DWORD nSize)
{
	WCHAR lpszPipename[128] = L"\\\\.\\pipe\\";
	for (int i = 9; i < 120; i++)
	{
		lpszPipename[i] = L'a' + rand() % 26;
	}

	(*hReadPipe) = CreateNamedPipeW(
		lpszPipename,             // pipe name 
		PIPE_ACCESS_INBOUND |      // read/write access 
		FILE_FLAG_OVERLAPPED,     // overlapped mode 
		PIPE_TYPE_BYTE |       // byte-type pipe 
		PIPE_READMODE_BYTE |   // byte read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // unlimited instances 
		nSize,    // output buffer size 
		0,    // input buffer size 
		0,             // client time-out 
		NULL);
	if ((!(*hReadPipe)) || ((*hReadPipe) == INVALID_HANDLE_VALUE))
	{
		(*hReadPipe) = (*hWritePipe) = 0;
		return FALSE;
	}
	(*hWritePipe) = CreateFileW(lpszPipename, GENERIC_WRITE,
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		FILE_FLAG_OVERLAPPED,              // default attributes 
		NULL);          // no template file 
	if ((!(*hWritePipe))||((*hWritePipe) == INVALID_HANDLE_VALUE))
	{
		CloseHandle((*hReadPipe));
		(*hReadPipe) = (*hWritePipe) = 0;
		return FALSE;
	}
	return TRUE;
}

BOOL PipeIORead(HANDLE PipeHandle)
{
	pPIPEIO_PACK PipeIOPack = malloc(sizeof(PIPEIO_PACK));
	BOOL bSuccess = FALSE;
	if (!PipeIOPack)
	{
		return FALSE;
	}
	__try
	{
		ZeroMemory(PipeIOPack, sizeof(PIPEIO_PACK));
		PipeIOPack->PackMode = PACKMODE_READ;
		PipeIOPack->pData = malloc(PIPEIO_BUFSIZE);
		if (!PipeIOPack->pData)
		{
			__leave;
		}
		ZeroMemory(PipeIOPack->pData, PIPEIO_BUFSIZE);
		DWORD BytesRead;
		BOOL bResult = ReadFile(PipeHandle, PipeIOPack->pData, PIPEIO_BUFSIZE, &BytesRead, PipeIOPack);
		if ((!bResult) && (GetLastError()!= ERROR_IO_PENDING))//ʧ���ˣ�ʧ���뻹����ERROR_IO_PENDING
		{
			__leave;
		}
		bSuccess = TRUE;
	}
	__finally
	{
		if (!bSuccess)
		{
			if (PipeIOPack)
			{
				if (PipeIOPack->pData);
				{
					free(PipeIOPack->pData);
				}
				free(PipeIOPack);
			}
		}
	}
	return bSuccess;
}

BOOL PipeIOSendClose(HANDLE hCompPort)
{
	pPIPEIO_PACK PipeIOPack = malloc(sizeof(PIPEIO_PACK));
	if (!PipeIOPack)
	{
		return FALSE;
	}
	ZeroMemory(PipeIOPack, sizeof(PIPEIO_PACK));
	PipeIOPack->PackMode = PACKMODE_CLOSE;
	PostQueuedCompletionStatus(hCompPort, 0, 0, (LPOVERLAPPED)PipeIOPack);
	return TRUE;
}