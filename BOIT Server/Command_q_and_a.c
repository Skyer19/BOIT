#include<Windows.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"

int CmdMsg_q_and_a_Proc(pBOIT_COMMAND pCmd, long long GroupID, long long QQID, WCHAR* AnonymousName, WCHAR* Msg)
{
	WCHAR ReplyMessage[][128] = {
		L"Q:�ҿ��Զ�bot������֮����\n\nA:���� quine ���� bot ��������������ķ�ʽ���Զ� bot ���� CTF ���߹���������ġ�\n��bot �������û�����������Ŷ��\n����з��ְ�ȫ©�����ǳ���ӭ�����߽�����",
		L"Q:�ҿ��԰�bot��������Ⱥ����\n\nA:�ڵõ�Ⱥ��ͬ�������¿��ԣ�����ϵkernel.bin. ��������֤ bot ȫ�� 24h ���С�",
		L"Q:botʲôʱ����¹��ܣ�\n\nA:ȡ���� kernel.bin �ж����",
		L"Q:ΪʲôͻȻ�����˺ö๦�ܣ�\n\nA:��Ϊ�Ҷ����� bot ��������д�����������ٶȺͰ�ȫ��",
		L"Q:Ϊʲô�������� xxx ���ԣ�\n\nA:�����Ҫ�¼����ԣ�����ϵ kernel.bin",
		L"Q:bot��Դ��\n\nA:��Դ�ģ�����#about�鿴���飬��ӭ star (*�R���Q)o��",
		L"Q:���д�����ʲôԭ��\n\nA:�ڱ��ر��룬����ͨ���ܵ��ض���ʵ�ֻ�ȡ������ݣ�����ͨ��һЩ�ֶ����ƽ���Ȩ��" };

	SendBackMessage(GroupID, QQID, ReplyMessage[rand() % _countof(ReplyMessage)]);
	return 0;
}