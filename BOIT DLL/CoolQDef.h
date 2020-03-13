//�й� CoolQ �ĸ��ֶ���

#define CQAPIVER 9
#define CQAPIVERTEXT "9"

#define CQAPPID "com.kernelbin.boit" 
#define CQAPPINFO CQAPIVERTEXT "," CQAPPID

#ifndef CQAPI
#define CQAPI(ReturnType) __declspec(dllimport) ReturnType __stdcall
#endif

#define CQEVENT(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size))\
__declspec(dllexport) ReturnType __stdcall Name

#define FUNC(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size))\
__declspec(dllexport) ReturnType __stdcall Name



/*
* ����˽����Ϣ, �ɹ�������ϢID
* QQID Ŀ��QQ��
* msg ��Ϣ����
*/

CQAPI(int) CQ_sendPrivateMsg(int AuthCode, long long QQID, const char* msg);

/*
* ����Ⱥ��Ϣ, �ɹ�������ϢID
* groupid Ⱥ��
* msg ��Ϣ����
*/
CQAPI(int) CQ_sendGroupMsg(int AuthCode, long long groupid, const char* msg);

/*
* ������������Ϣ, �ɹ�������ϢID
* discussid �������
* msg ��Ϣ����
*/
//CQAPI(int) CQ_sendDiscussMsg(int AuthCode, long long discussid, const char* msg);

/*
* ������Ϣ
* msgid ��ϢID
*/
//CQAPI(int) CQ_deleteMsg(int AuthCode, long long msgid);

/*
* ������ �����ֻ���
* QQID QQ��
*/
//CQAPI(int) CQ_sendLike(int AuthCode, long long QQID);

/*
* ��ȺԱ�Ƴ�
* groupid Ŀ��Ⱥ
* QQID QQ��
* rejectaddrequest ���ٽ��մ��˼�Ⱥ���룬������
*/
//CQAPI(int) CQ_setGroupKick(int AuthCode, long long groupid, long long QQID, int rejectaddrequest);

/*
* ��ȺԱ����
* groupid Ŀ��Ⱥ
* QQID QQ��
* duration ���Ե�ʱ�䣬��λΪ�롣���Ҫ�����������д0��
*/
//CQAPI(int) CQ_setGroupBan(int AuthCode, long long groupid, long long QQID, long long duration);

/*
* ��Ⱥ����Ա
* groupid Ŀ��Ⱥ
* QQID QQ��
* setadmin true:���ù���Ա false:ȡ������Ա
*/
//CQAPI(int) CQ_setGroupAdmin(int AuthCode, long long groupid, long long QQID, int setadmin);

/*
* ��ȫȺ����
* groupid Ŀ��Ⱥ
* enableban true:���� false:�ر�
*/
//CQAPI(int) CQ_setGroupWholeBan(int AuthCode, long long groupid, int enableban);

/*
* ������ȺԱ����
* groupid Ŀ��Ⱥ
* anomymous Ⱥ��Ϣ�¼��յ��� anomymous ����
* duration ���Ե�ʱ�䣬��λΪ�롣��֧�ֽ����
*/
//CQAPI(int) CQ_setGroupAnonymousBan(int AuthCode, long long groupid, const char* anomymous, long long duration);

/*
* ��Ⱥ��������
* groupid Ŀ��Ⱥ
* enableanomymous true:���� false:�ر�
*/
//CQAPI(int) CQ_setGroupAnonymous(int AuthCode, long long groupid, int enableanomymous);

/*
* ��Ⱥ��Ա��Ƭ
* groupid Ŀ��Ⱥ
* QQID Ŀ��QQ
* newcard ����Ƭ(�ǳ�)
*/
//CQAPI(int) CQ_setGroupCard(int AuthCode, long long groupid, long long QQID, const char* newcard);

/*
* ��Ⱥ�˳� ����, �˽ӿ���Ҫ�ϸ���Ȩ
* groupid Ŀ��Ⱥ
* isdismiss �Ƿ��ɢ true:��ɢ��Ⱥ(Ⱥ��) false:�˳���Ⱥ(����Ⱥ��Ա)
*/
//CQAPI(int) CQ_setGroupLeave(int AuthCode, long long groupid, int isdismiss);

/*
* ��Ⱥ��Աר��ͷ�� ��Ⱥ��Ȩ��
* groupid Ŀ��Ⱥ
* QQID Ŀ��QQ
* newspecialtitle ͷ�Σ����Ҫɾ����������գ�
* duration ר��ͷ����Ч�ڣ���λΪ�롣���������Ч��������д-1��
*/
//CQAPI(int) CQ_setGroupSpecialTitle(int AuthCode, long long groupid, long long QQID, const char* newspecialtitle, long long duration);

/*
* ���������˳�
* discussid Ŀ���������
*/
//CQAPI(int) CQ_setDiscussLeave(int AuthCode, long long discussid);

/*
* �ú����������
* responseflag �����¼��յ��� responseflag ����
* responseoperation REQUEST_ALLOW �� REQUEST_DENY
* remark ��Ӻ�ĺ��ѱ�ע
*/
//CQAPI(int) CQ_setFriendAddRequest(int AuthCode, const char* responseflag, int responseoperation, const char* remark);

/*
* ��Ⱥ�������
* responseflag �����¼��յ��� responseflag ����
* requesttype���������¼������������� REQUEST_GROUPADD �� REQUEST_GROUPINVITE
* responseoperation  REQUEST_ALLOW �� REQUEST_DENY
* reason �������ɣ��� REQUEST_GROUPADD �� REQUEST_DENY ʱ����
*/
//CQAPI(int) CQ_setGroupAddRequestV2(int AuthCode, const char* responseflag, int requesttype, int responseoperation, const char* reason);

/*
* ȡȺ��Ա��Ϣ
* groupid Ŀ��QQ����Ⱥ
* QQID Ŀ��QQ��
* nocache ��ʹ�û���
*/
CQAPI(const char*) CQ_getGroupMemberInfoV2(int AuthCode, long long groupid, long long QQID, int nocache);

/*
* ȡİ������Ϣ
* QQID Ŀ��QQ
* nocache ��ʹ�û���
*/
CQAPI(const char*) CQ_getStrangerInfo(int AuthCode, long long QQID, int nocache);

/*
* ��־
* priority ���ȼ���CQLOG ��ͷ�ĳ���
* category ����
* content ����
*/
CQAPI(int) CQ_addLog(int AuthCode, int priority, const char* category, const char* content);

/*
* ȡCookies ����, �˽ӿ���Ҫ�ϸ���Ȩ
*/
CQAPI(const char*) CQ_getCookies(int AuthCode);

/*
* ȡCsrfToken ����, �˽ӿ���Ҫ�ϸ���Ȩ
*/
//CQAPI(int) CQ_getCsrfToken(int AuthCode);

/*
* ȡ��¼QQ
*/
//CQAPI(long long) CQ_getLoginQQ(int AuthCode);

/*
* ȡ��¼QQ�ǳ�
*/
//CQAPI(const char*) CQ_getLoginNick(int AuthCode);

/*
* ȡӦ��Ŀ¼�����ص�·��ĩβ��"\"
*/
CQAPI(const char*) CQ_getAppDirectory(int AuthCode);

/*
* ������������ʾ
* errorinfo ������Ϣ
*/
//CQAPI(int) CQ_setFatal(int AuthCode, const char* errorinfo);

/*
* ����������������Ϣ�е�����(record),���ر����� \data\record\ Ŀ¼�µ��ļ���
* file �յ���Ϣ�е������ļ���(file)
* outformat Ӧ������������ļ���ʽ��Ŀǰ֧�� mp3 amr wma m4a spx ogg wav flac
*/
//CQAPI(const char*) CQ_getRecord(int AuthCode, const char* file, const char* outformat);


#define EVENT_IGNORE 0          //�¼�_����
#define EVENT_BLOCK 1           //�¼�_����

#define REQUEST_ALLOW 1         //����_ͨ��
#define REQUEST_DENY 2          //����_�ܾ�

#define REQUEST_GROUPADD 1      //����_Ⱥ���
#define REQUEST_GROUPINVITE 2   //����_Ⱥ����

#define CQLOG_DEBUG 0           //���� ��ɫ
#define CQLOG_INFO 10           //��Ϣ ��ɫ
#define CQLOG_INFOSUCCESS 11    //��Ϣ(�ɹ�) ��ɫ
#define CQLOG_INFORECV 12       //��Ϣ(����) ��ɫ
#define CQLOG_INFOSEND 13       //��Ϣ(����) ��ɫ
#define CQLOG_WARNING 20        //���� ��ɫ
#define CQLOG_ERROR 30          //���� ��ɫ
#define CQLOG_FATAL 40          //�������� ���
