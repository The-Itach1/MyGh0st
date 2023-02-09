#pragma once
#include "afxcmn.h"
#include "include/IOCPServer.h"
// CWindowsDlg 对话框

class CWindowsDlg : public CDialog
{
	DECLARE_DYNAMIC(CWindowsDlg)

public:
	CWindowsDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	virtual ~CWindowsDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINDOWS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list_windows;
private:
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	BOOL m_bHow;     //用来区分窗口管理和进程管理
public:
	afx_msg void OnClose();		//关闭对话框消息处理函数
	afx_msg void OnSize(UINT nType, int cx, int cy);	//对话框大小变换
	void AdjustList(void);	//调整list控件大小
	virtual BOOL OnInitDialog();	//对话框初始化函数
	void OnReceiveComplete(void);	//数据接收函数
	void ShowWindowsList(void);		//将接收到的窗口数据展示在list中。
	afx_msg void OnNMRClickListWindows(NMHDR* pNMHDR, LRESULT* pResult);	//右键弹出菜单
	afx_msg void OnWindowsClose();	//窗口关闭处理函数
	afx_msg void OnWindowsHide();	//隐藏窗口
	afx_msg void OnWindowsMax();	//最大化窗口
	afx_msg void OnWindowsMin();	//最小化窗口
	afx_msg void OnWindowsReturn();	//还原显示窗口
	afx_msg void OnWindowsReflush();	//刷新整个列表
	void GetWindowsList(void);
};
