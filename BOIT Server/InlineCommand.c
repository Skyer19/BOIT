#include<Windows.h>
#include"APITransfer.h"
#include"CommandManager.h"
#include"InlineCommand.h"


int RegisterInlineCommand()
{
	pBOIT_COMMAND Command_qwq = RegisterCommand(L"qwq", CmdMsg_qwq_Proc, L"����", BOIT_MATCH_FULL);
	AddCommandAlias(Command_qwq, L"pwp");
	AddCommandAlias(Command_qwq, L"qaq");
	RegisterCommand(L"èè", CmdMsg_cat_Proc, L"����èè��", BOIT_MATCH_FULL);
	RegisterCommand(L"����", CmdMsg_meow_Proc, L"����èè��", BOIT_MATCH_FULL);
	RegisterCommand(L"about", CmdMsg_about_Proc, L"����", BOIT_MATCH_FULL);
	RegisterCommand(L"boast", CmdMsg_boast_Proc, L"��ţ��(�����еĹ���)", BOIT_MATCH_FULL);
	RegisterCommandEx(L"run", CmdMsg_run_Proc, CmdEvent_run_Proc, L"���д���", BOIT_MATCH_PARAM);
	RegisterCommand(L"savecode", CmdMsg_savecode_Proc, L"�������", BOIT_MATCH_PARAM);
	RegisterCommand(L"runcode", CmdMsg_runcode_Proc, L"����֮ǰ����Ĵ���", BOIT_MATCH_FULL);
	RegisterCommand(L"help", CmdMsg_help_Proc, L"������Ϣ", BOIT_MATCH_PARAM);
	RegisterCommand(L"q&amp;a", CmdMsg_q_and_a_Proc, L"�����ʴ�", BOIT_MATCH_FULL);
	RegisterCommand(L"stop", CmdMsg_stop_Proc, L"�ر�BOIT", BOIT_MATCH_FULL);

	return 0;
}