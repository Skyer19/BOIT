#include<Windows.h>
#include <stdio.h>
#include <string.h>
#include<tchar.h>
// ȫ�ֳ�������
const char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char padding_char = '=';


int Base64Encode(const unsigned char* sourcedata, char* base64)
{
	int i = 0, j = 0;
	unsigned char trans_index = 0;    // ������8λ�����Ǹ���λ��Ϊ0
	const int datalength = strlen((const char*)sourcedata);
	for (; i < datalength; i += 3) {
		// ÿ����һ�飬���б���
		// Ҫ��������ֵĵ�һ��
		trans_index = ((sourcedata[i] >> 2) & 0x3f);
		base64[j++] = base64char[(int)trans_index];
		// �ڶ���
		trans_index = ((sourcedata[i] << 4) & 0x30);
		if (i + 1 < datalength) {
			trans_index |= ((sourcedata[i + 1] >> 4) & 0x0f);
			base64[j++] = base64char[(int)trans_index];
		}
		else {
			base64[j++] = base64char[(int)trans_index];

			base64[j++] = padding_char;

			base64[j++] = padding_char;

			break;   // �����ܳ��ȣ�����ֱ��break
		}
		// ������
		trans_index = ((sourcedata[i + 1] << 2) & 0x3c);
		if (i + 2 < datalength) { // �еĻ���Ҫ����2��
			trans_index |= ((sourcedata[i + 2] >> 6) & 0x03);
			base64[j++] = base64char[(int)trans_index];

			trans_index = sourcedata[i + 2] & 0x3f;
			base64[j++] = base64char[(int)trans_index];
		}
		else {
			base64[j++] = base64char[(int)trans_index];

			base64[j++] = padding_char;

			break;
		}
	}

	base64[j] = '\0';

	return 0;
}

/** ���ַ����в�ѯ�ض��ַ�λ������
* const char *str ���ַ���
* char c��Ҫ���ҵ��ַ�
*/
inline int num_strchr(const char* str, char c) // 
{
	const char* pindex = strchr(str, c);
	if (NULL == pindex) {
		return -1;
	}
	return pindex - str;
}
/* ����
* const char * base64 ����
* unsigned char * dedata�� ����ָ�������
*/
int Base64Decode(const char* base64, unsigned char* bindata)
{
	int i, j;
	unsigned char k;
	unsigned char temp[4];
	for (i = 0, j = 0; base64[i] != '\0'; i += 4)
	{
		memset(temp, 0xFF, sizeof(temp));
		for (k = 0; k < 64; k++)
		{
			if (base64char[k] == base64[i])
				temp[0] = k;
		}
		for (k = 0; k < 64; k++)
		{
			if (base64char[k] == base64[i + 1])
				temp[1] = k;
		}
		for (k = 0; k < 64; k++)
		{
			if (base64char[k] == base64[i + 2])
				temp[2] = k;
		}
		for (k = 0; k < 64; k++)
		{
			if (base64char[k] == base64[i + 3])
				temp[3] = k;
		}

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC)) |
			((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03));
		if (base64[i + 2] == '=')
			break;

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0)) |
			((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F));
		if (base64[i + 3] == '=')
			break;

		bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) |
			((unsigned char)(temp[3] & 0x3F));
	}
	return j;
}


