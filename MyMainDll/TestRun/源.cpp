#include<Windows.h>
#include<stdio.h>

int main()
{
	char strHost[] = "192.168.183.1";          //�������ߵ�ַ
	int  nPort = 7000;                     //�������߶˿�
	//��������dll
	//HMODULE hServerDll = LoadLibrary("..\\..\\bin\\server\\MainDll.dll");
	HMODULE hServerDll = LoadLibrary("MainDll.dll");
	//����������������--�鿴��һ�ڵ�����TestRun����
	typedef void(_cdecl* TestRunT)(char* strHost, int nPort);
	//Ѱ��dll�е�������
	TestRunT pTestRunT = (TestRunT)GetProcAddress(hServerDll, "TestRun");
	//�жϺ����Ƿ�Ϊ��
	if (pTestRunT != NULL)
	{
		pTestRunT(strHost, nPort);   //�����������
	}
	while (1)
	{

	}
}