#include<Windows.h>
#include<stdlib.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"

int CmdMsg_donate_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	switch (rand()%4)
	{
	case 0:
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=�ң�bot���ɰ�����Ǯ��,content=_(:�١���)_,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		SendBackMessage(boitSession, L"[CQ:image,file=8C048B6CD3FE93875DD32101364EF4C4.gif]");
		break;

	case 1:
		SendBackMessage(boitSession, L"[CQ:image,file=B7B62C976EB995D73658DDE68629EB7F.gif]");
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=Ȼ��kernel.bin�����ڹ�����,content=��ӭ�����̲����ڼ���kernel.binд����,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		break;

	case 2:
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=kernel.bin��������,content=����ing���n��,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		break;

	case 3:
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=����bot������ӵ���������Լ���ָ��,content=qwq,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		break;
	}
	return 0;
}