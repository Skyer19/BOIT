#pragma once
#include<Windows.h>

//�汾����
#define VERSION_MAJOR 2
#define VERSION_MINOR 0

//����ֵ����
#define SETTINGS_LOADED 0			//�ҵ����ò����سɹ�
#define SETTINGS_SAVED 1
#define SETTINGS_CLEARED 2
#define SETTINGS_INITIALIZED 3

#define SETTINGS_NOT_FOUND 4			//����ע��������û�ҵ� Ӧ���ǵ�һ��ʹ��������
#define SETTINGS_OUT_OF_DATE 5				//�������ϰ汾��
#define SETTINGS_ERROR 6			//���ִ���
#define SETTINGS_NULL_BUFFER 7       //��������NULL