#pragma once

//���ص�ָ�����������Ҫ�ͷŵ�
//�����㷨
//char* encryptionStr(char* str)
//{
//	int len = strlen(str);
//	char * a = (char *)operator new(len + 1);
//	a[0] = len;
//	for (size_t i = 1; i <= len; i++)
//	{
//		a[i] = str[i - 1] ^ (0xCC - i);
//	}
//	return a;
//}

#define STR_CRY_LENGTH			0	//�����ַ����ĳ���

//�����㷨
char* decodeStr(char* str);



//char �����ַ���[] = { 0x07,0xbc,0xa3,0xa7,0xbb,0xb3,0xa7,0xf5 };	//winsta0
//char* ���ܳ������ַ���ָ�� = decodeStr(winsta0);					//���ܺ���

//memset(lpszWinSta, 0, winsta0[STR_CRY_LENGTH]);					//���0
//delete lpszWinSta;