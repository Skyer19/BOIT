#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_boast_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	WCHAR ReplyMessage[][128] = {
		L"�ҿ��Բ�ѯcodeforce�ϵı���ʱ���أ������Բ�ѯ��Ŀ������Ͳ������ݣ�",
		L"��ѯ������Ҳ����أ�һ�Բ�ѯҲ��������Ŷ",
		L"����ϵ���Ŀ��Ҳ�����ҵ���",
		L"���Ÿ�������Ҳ��С��һ��Ŷ",
		L"��������ͷɻ���Ҳ���ҵ����ֳ���",
		L"��bot����������Դ��Ҳ��������Ŷ",
		L"ʲôʱ��ͨ��ͼ����ԣ������ͨ��ͼ��������㲻֪����"
	};
	SendBackMessage(boitSession, ReplyMessage[rand() % _countof(ReplyMessage)]);

	switch (rand() % 20)
	{
	case 0:
		SendBackMessage(boitSession, L"ƭ�ˣ�");
		SendBackMessage(boitSession, L"��ţ�Ʋ�����ƭ��.......��ţ��......bot���£��ܽ�ƭ����");
		break;
	case 1:
		SendBackMessage(boitSession, L"Ϊʲô�һ���������Щ�������ȥ��kernel.bin��");
		break;
	}
	return 0;
}