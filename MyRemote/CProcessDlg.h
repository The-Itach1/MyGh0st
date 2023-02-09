#pragma once
#include "afxcmn.h"
#include "include/IOCPServer.h"
// CProcessDlg 对话框

class CProcessDlg : public CDialog
{
	DECLARE_DYNAMIC(CProcessDlg)

public:
	CProcessDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	virtual ~CProcessDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CListCtrl m_list_process;
private:
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	BOOL m_bHow;     //用来区分窗口管理和进程管理
public:
	afx_msg void OnClose();	//关闭对话框后的处理函数
	afx_msg void OnSize(UINT nType, int cx, int cy);	//让list控件的大小随对话框的变动而变动
	virtual BOOL OnInitDialog();	//初始化对话框
	void ShowProcessList(void);	//将接收到的数据显现在list中
	void AdjustList();	//调整list控件位置
	void OnReceiveComplete(void);	//接收消息的函数
private:
	void GetProcessList(void);	//想客户端发送获取进程数据的命令
public:
	afx_msg void OnNMRClickListProcess(NMHDR* pNMHDR, LRESULT* pResult);	//弹出菜单
	afx_msg void OnProcessKill();	//杀死某一进程
	afx_msg void OnProcessRefresh(); //刷新
};
