#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_commingfeature_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, int SubType, WCHAR* AnonymousName, WCHAR* Msg)
{
	WCHAR ReplyMessage[][128] = {
		L"�ҿ��Բ�ѯcodeforce�ϵı���ʱ���أ��һ��ܲ�ѯtourist��profile��",
		L"��ѯ������Ҳ����أ�һ�Բ�ѯҲ��������Ŷ",
		L"����ϵ���Ŀ��Ҳ�����ҵ���",
		L"���Ÿ�������Ҳ��С��һ��Ŷ",
		L"��������ͷɻ���Ҳ���ҵ����ֳ���",
		L"��bot����������Դ��Ҳ��������Ŷ"
		L"ʲôʱ��ͨ��ͼ����ԣ������ͨ��ͼ��������㲻֪����"
	};
	SendBackMessage(GroupID, QQID, ReplyMessage[rand() % _countof(ReplyMessage)]);

	switch (rand() % 20)
	{
	case 0:
		SendBackMessage(GroupID, QQID, L"ƭ�ˣ�");
		SendBackMessage(GroupID, QQID, L"��ţ�Ʋ�����ƭ��.......��ţ��......bot���£��ܽ�ƭ����");
		break;
	case 1:
		SendBackMessage(GroupID, QQID, L"Ϊʲô�һ���������Щ�������ȥ��kernel.bin��");
		break;
	}
	return 0;
}