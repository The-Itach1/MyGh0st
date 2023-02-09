#include<Windows.h>
#include<stdio.h>

int main()
{
	char strHost[] = "192.168.183.1";          //声明上线地址
	int  nPort = 7000;                     //声明上线端口
	//载入服务端dll
	//HMODULE hServerDll = LoadLibrary("..\\..\\bin\\server\\MainDll.dll");
	HMODULE hServerDll = LoadLibrary("MainDll.dll");
	//声明导出函数类型--查看上一节导出的TestRun函数
	typedef void(_cdecl* TestRunT)(char* strHost, int nPort);
	//寻找dll中导出函数
	TestRunT pTestRunT = (TestRunT)GetProcAddress(hServerDll, "TestRun");
	//判断函数是否为空
	if (pTestRunT != NULL)
	{
		pTestRunT(strHost, nPort);   //调用这个函数
	}
	while (1)
	{

	}
}